//
// reply.cpp
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
#include "reply.h"
#include <string>
#include <boost/lexical_cast.hpp>

std::vector<boost::asio::const_buffer> reply::to_buffers()
{
	std::vector<boost::asio::const_buffer> buffers;
	for (std::size_t i = 0; i < objects.size(); ++i)
	{
		amf3writer * writer;

		//TODO: use boost buffer object instead?
		char tbuff[15000];
		int length = 0;
		writer = new amf3writer(tbuff + 4);

		writer->Write(objects[i]);

		(*(int*)tbuff) = length = writer->position;
		ByteSwap(*(int*)tbuff);

		buffers.push_back(boost::asio::buffer(tbuff, length + 4));
		delete writer;
	}
	return buffers;
}
