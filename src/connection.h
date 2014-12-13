//
// connection.h
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

#pragma once

#define MAXPACKETSIZE 32768/10

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "reply.h"
#include "request.h"
#include "request_handler.h"

class Server;
class Client;

/// Represents a single connection from a client.
class connection : public std::enable_shared_from_this<connection>
{
public:
	connection(const connection&) = delete;
	connection& operator=(const connection&) = delete;

	/// Construct a connection with the given io_service.
	explicit connection(boost::asio::ip::tcp::socket socket,
						Server& gserver, request_handler& handler);

	/// Get the socket associated with the connection.
	boost::asio::ip::tcp::socket& socket();

	/// Start the first asynchronous operation for the connection.
	void start();
	void startpolicy();

	/// Stop all asynchronous operations associated with the connection.
	void stop();

	void write(const char * data, const int32_t size);

private:
	/// Handle completion of a read operation.
	void handle_read_policy(const boost::system::error_code& e,
							std::size_t bytes_transferred);
	void handle_read_header(const boost::system::error_code& e,
							std::size_t bytes_transferred);
	void handle_read(const boost::system::error_code& e,
					 std::size_t bytes_transferred);
	/// Handle completion of a write operation.
	void handle_write(const boost::system::error_code& e);

	/// Socket for the connection.
	boost::asio::ip::tcp::socket socket_;

	/// The manager for this connection.
	Server& server;

	/// The handler used to process the incoming request.
	request_handler& request_handler_;

	/// Buffer for incoming data.
	std::array<char, MAXPACKETSIZE> buffer_;

	/// The incoming request.
	request request_;

	/// The reply to be sent back to the client.
	reply reply_;

	int32_t size;

public:
	uint64_t uid;

	Client * client_;

	string address;
};

typedef std::shared_ptr<connection> connection_ptr;

