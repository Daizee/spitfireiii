//
// connection.cpp
// Project Spitfire
//
// Copyright (c) 2014 Daizee (rensiadz at gmail dot com)
//
// This file is part of Spitfire.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "includes.h"
#include "connection.h"
#include <vector>
#include <boost/bind.hpp>
#include "Server.h"
#include "request_handler.h"
#include "Client.h"

extern uint64_t unixtime();

connection::connection(boost::asio::ip::tcp::socket socket,
					   Server& gserver, request_handler& handler)
					   : socket_(std::move(socket)),
					   server(gserver),
					   request_handler_(handler),
					   uid(0),
					   client_(0)
{
	size = 0;
}

boost::asio::ip::tcp::socket& connection::socket()
{
	return socket_;
}

void connection::start()
{
	uid = rand()*rand()*rand();

	srand(unixtime());
	server.PlayerCount(1);

	boost::asio::async_read(socket_, boost::asio::buffer(buffer_, 4), boost::bind(&connection::handle_read_header, shared_from_this(),
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));
}

void connection::startpolicy()
{
	boost::asio::async_read(socket_, boost::asio::buffer(buffer_, 4), boost::bind(&connection::handle_read_policy, shared_from_this(),
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));
}

void connection::stop()
{
	socket_.close();
	if (client_)
	{
		// 		if (!client_->m_accountexists)
		// 		{
		// 			int num = client_->m_clientnumber;
		// 			delete s->m_clients[num];
		// 			s->m_clients[num] = 0;
		// 			return;
		// 		}
		client_->socket = 0;
		client_->m_socknum = 0;
		client_ = 0;
		server.PlayerCount(-1);
		//TODO: record last online time
	}
}

void connection::write(const char * data, const int32_t size)
{
	auto self(shared_from_this());
	boost::asio::async_write(socket_, boost::asio::buffer(data, size),
							 [this, self, size](boost::system::error_code ec, std::size_t written)
	{
		if (!ec)
		{
			// No socket errors on write
			if (size != written)
			{
				//sent size not matching
				server.stop(shared_from_this());
				std::cerr << "Data sent does not match size sent. Socket closed.\n";
			}
		}
		else if (ec != boost::asio::error::operation_aborted)
		{
			server.stop(shared_from_this());
			std::cerr << "boost::asio::async_write() failure\n";
		}
		else
		{
			std::cerr << "boost::asio::async_write() failure - operation_aborted\n";
		}
	});
}

void connection::handle_read_policy(const boost::system::error_code& e,
									std::size_t bytes_transferred)
{
	if (!e)
	{
		if (bytes_transferred == 4)
		{
			if (!memcmp(buffer_.data(), "<c", 2) || !memcmp(buffer_.data(), "<p", 2))
			{
				boost::asio::async_write(socket_, boost::asio::buffer("<cross-domain-policy><allow-access-from domain=\"*\" to-ports=\"21-60000\" /></cross-domain-policy>\0"),
										 boost::bind(&connection::handle_write, shared_from_this(),
										 boost::asio::placeholders::error));
				boost::system::error_code ignored_ec;
				socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
				server.stop(shared_from_this());
			}
		}
	}
	else if (e != boost::asio::error::operation_aborted)
	{
		server.stop(shared_from_this());
		return;
	}

}

void connection::handle_read_header(const boost::system::error_code& e,
									std::size_t bytes_transferred)
{
	if (!e)
	{
		if (bytes_transferred == 4)
		{
			size = *(int32_t*)buffer_.data();
			ByteSwap(size);

			if ((size < 4) || (size >= MAXPACKETSIZE))
			{
				server.consoleLogger->information(Poco::format("Did not receive proper amount of bytes : sent: %?d - ip:%s", size, address));
				server.stop(shared_from_this());
				return;
			}

			boost::asio::async_read(socket_, boost::asio::buffer(buffer_, size), boost::bind(&connection::handle_read, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		}
	}
	else if (e != boost::asio::error::operation_aborted)
	{
		server.stop(shared_from_this());
		return;
	}

}

void connection::handle_read(const boost::system::error_code& e,
							 std::size_t bytes_transferred)
{
	if (!e)
	{
		if (bytes_transferred != size)
		{
			this->client_->m_main->consoleLogger->information(Poco::format("Did not receive proper amount of bytes : rcv: %?d needed: %?d", bytes_transferred, size));
			server.stop(shared_from_this());
			return;
		}
		//printf("uid("XI64")\n", uid);
		// read object size
		if ((size > MAXPACKETSIZE) || (size <= 0))
		{
			//ERROR - object too large - close connection
			server.stop(shared_from_this());
			return;
		}

		//TODO: Decision: have socket read thread handle packets, or push into a queue
		//socket thread (easy, already done)
		// PRO: typically instant response times due to it being processed as it comes in
		// CON: a large request (legit or non) would cause the socket read thread to lag
		// 
		//process thread (complex, ideally better)
		// PRO: can alleviate lag on socket threads and create multiple thread processing queues depending on importance
		// CON: complexity and large requests would typically land in the same thread (causing even more lag for them) unless
		//		made even more complex to have multiple threads for large requests
		//
		//Option 3: Evony style
		// -- create a process thread per x amount of sockets
		// PRO: lag from one client only affects a set amount of players and not the entire server
		// CON: quite complex. is ultimately the process thread option only for x amount of sockets

		// parse packet
		request_.size = size;
		amf3parser * cparser = new amf3parser(buffer_.data());
		try
		{
			request_.object = cparser->ReadNextObject();
			delete cparser;
		}
		catch (...)
		{
			std::cerr << "uncaught handle_request()::amf3parser exception\n";
		}
		request_.conn = this;
		try {
			request_handler_.handle_request(request_, reply_);
		}
		catch (std::exception& e)
		{
			std::cerr << "handle_request() exception: " << e.what() << "\n";
		}
		// 		if (reply_.objects.size() > 0)
		// 		{
		// 			// send reply packets
		// 			try {
		// 				socket_.write_some(reply_.to_buffers());
		// 			}
		// 			catch (std::exception& e)
		// 			{
		// 				std::cerr << "asio::write_some() exception: " << e.what() << "\n";
		// 			}
		// 			reply_.objects.clear();
		// 		}

		boost::asio::async_read(socket_, boost::asio::buffer(buffer_, 4), boost::bind(&connection::handle_read_header, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}
	else if (e != boost::asio::error::operation_aborted)
	{
		server.stop(shared_from_this());
		return;
	}
}

void connection::handle_write(const boost::system::error_code& e)
{
	if (!e)
	{
		// 		// Initiate graceful connection closure.
		// 		asio::error_code ignored_ec;
		// 		socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ignored_ec);
	}

	if (e != boost::asio::error::operation_aborted)
	{
		server.stop(shared_from_this());
	}
}
