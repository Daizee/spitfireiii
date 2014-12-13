//
// pfurlough.cpp
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

#include "pfurlough.h"
#include "../Server.h"
#include "../Client.h"


pfurlough::pfurlough(Server * server, request & req, amf3object & obj)
	: packet(server, req, obj)
{

}

pfurlough::~pfurlough()
{

}

void pfurlough::process()
{
	if ((command == "isFurlought"))
	{
		int32_t playerid = data["playerId"];
		string password = data["password"];
		bool autofurlough = data["isAutoFurlough"];
		int32_t day = data["day"];

		if (client->m_password != password)
		{
			gserver->SendObject(client, gserver->CreateError("furlough.isFurlought", -50, "Incorrect account or password."));
			return;
		}

		if (client->m_cents < day * 10)
		{
			gserver->SendObject(client, gserver->CreateError("furlough.isFurlought", -99, "Not enough cents."));
			return;
		}
		client->m_cents -= day * 10;

		client->SetBuff("FurloughBuff", "", timestamp + (day * 24 * 60 * 60 * 1000));

		obj2["cmd"] = "furlough.isFurlought";
		data2["packageId"] = 0.0f;
		data2["ok"] = 1;
		data2["playerBean"] = client->ToObject();

		gserver->SendObject(client, obj2);
		return;
	}
	if ((command == "cancelFurlought"))
	{
		client->SetBuff("FurloughBuff", "", 0);

		obj2["cmd"] = "furlough.cancelFurlought";
		data2["packageId"] = 0.0f;
		data2["ok"] = 1;

		gserver->SendObject(client, obj2);

		client->SaveToDB();

		return;
	}
}



