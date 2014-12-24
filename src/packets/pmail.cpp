//
// pmail.cpp
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

#include "pmail.h"
#include "../Server.h"
#include "../Client.h"


pmail::pmail(Server * server, request & req, amf3object & obj)
	: packet(server, req, obj)
{

}

pmail::~pmail()
{

}

void pmail::process()
{
	obj2["data"] = amf3object();
	amf3object & data2 = obj2["data"];

	if (command == "receiveMailList")
	{
		int32_t pageno = obj["pageNo"];
		int32_t pagesize = obj["pageSize"];
		int32_t type = obj["type"];


		amf3object obj3;
		obj3["cmd"] = "mail.receiveMailList";
		obj3["data"] = amf3object();
		amf3object & data3 = obj3["data"];

		amf3array mails;
		amf3object mail;
		mail["sender"] = "Daisy1";
		mail["mailid"] = 1;
		mail["selected"] = false;
		mail["title"] = "title";
		mail["receiver"] = "Daisy";
		mail["playerId"] = client->m_playerid;
		mail["receiveTime"] = unixtime();
		mail["targetId"] = client->m_playerid;
		mail["type_id"] = 1;//??
		mail["isRead"] = 0;//0 = unread, 1 = read?
		mails.Add(mail);

		data3["reports"] = mails;
		data3["pageNo"] = pageno;
		data3["packageId"] = 0.0f;
		data3["ok"] = 1;
		data3["totalPage"] = 0;
		gserver->SendObject(client, obj3);
		return;
	}
	if (command == "sendMail")
	{
		string username = data["username"];
		string title = data["title"];
		string content = data["content"];

		Client * tclient = gserver->GetClientByName(username);
		if (!tclient)
		{
			gserver->SendObject(client, gserver->CreateError("mail.sendMail", -41, "Player " + username + " doesn't exist."));
			return;
		}
		amf3object obj3;
		obj3["cmd"] = "server.NewMail";
		obj3["data"] = amf3object();
		amf3object & data3 = obj3["data"];

		data3["count_system"] = 0;
		data3["count"] = 0;
		data3["count_inbox"] = 0;
		gserver->SendObject(tclient, obj3);

		amf3object obj4;
		obj4["cmd"] = "server.NewMail";
		obj4["data"] = amf3object();
		amf3object & data4 = obj4["data"];

		data4["count_system"] = 0;
		data4["count"] = 0;
		data4["count_inbox"] = 0;
		gserver->SendObject(tclient, obj4);
		return;
	}
}



