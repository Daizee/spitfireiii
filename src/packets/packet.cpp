//
// packet.cpp
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

#include "packet.h"
#include "../Server.h"
#include "../Client.h"
#include "../City.h"
#include "../connection.h"


packet::packet(Server * server, request & req, amf3object & obj)
	: req(req),
	gserver(server),
	obj(obj),
	client(req.conn->client_),
	data(obj["data"]),
	obj2(amf3object()),
	data2(obj2["data"])
{
	string cmd = obj["cmd"];
	std::vector<string> tokens;
	boost::split(tokens, cmd, boost::is_any_of("."));

	if (tokens.size() == 2)
	{
		cmdtype = tokens[0];
		command = tokens[1];
	}
	else if (tokens.size() == 1)
	{
		cmdtype = tokens[0];
	}

	timestamp = unixtime();

	obj2["cmd"] = "";
	data2 = amf3object();

	city = 0;
	if (client && client->m_currentcityindex != -1)
		city = client->m_city[client->m_currentcityindex];
}

packet::~packet()
{

}


void packet::CHECKCASTLEID()
{
	if (IsObject(data) && KeyExists(data, "castleId") && (int)data["castleId"] != client->m_currentcityid)
	{
		gserver->consoleLogger->information(Poco::format("castleId does not match castle focus! gave: %?d is:%?d - cmd: %s.%s - accountid:%?d - playername: %s", (int)data["castleId"], client->m_currentcityid, cmdtype, command, client->m_accountid, (char*)client->m_playername.c_str()));
		throw(0);
	}
}

void packet::VERIFYCASTLEID()
{
	if (!IsObject(data) || !KeyExists(data, "castleId"))
	{
		gserver->consoleLogger->information(Poco::format("castleId not received! - cmd: %s.%s - accountid:%?d - playername: %s", cmdtype, command, client->m_accountid, (char*)client->m_playername.c_str()));
		throw(1);
	}
}
