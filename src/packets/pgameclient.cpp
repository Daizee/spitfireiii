//
// pgameclient.cpp
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

#include "pgameclient.h"
#include "../Server.h"

pgameclient::pgameclient(Server * server, request & req, amf3object & obj)
	: packet(server, req, obj)
{

}

pgameclient::~pgameclient()
{

}

void pgameclient::process()
{
	if (command == "version")
	{
		if (data == "091103_11")
		{
			//pass
			return;
		}
		else
		{
// 			obj2["cmd"] = "gameClient.kickout";
// 			obj2["data"] = amf3object();
// 			data2["msg"] = "You suck.";
// 
// 			gserver->SendObject(client, obj2);
// 			//"other" version
// 			return;

			obj2["cmd"] = "gameClient.errorVersion";
			data2["version"] = "091103_11";
			data2["msg"] = "Invalid Version.";

			gserver->SendObject(client, obj2);
			//"other" version
			return;
		}
	}
	return;
}



