//
// pcity.cpp
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

#include "pcity.h"
#include "../Server.h"
#include "../Client.h"
#include "../City.h"


pcity::pcity(Server * server, request & req, amf3object & obj)
	: packet(server, req, obj)
{

}

pcity::~pcity()
{

}

void pcity::process()
{
	if (command == "advMoveCastle")
	{
		VERIFYCASTLEID();
		CHECKCASTLEID();

		int32_t castleid = obj["castleId"];
		int32_t tileid = obj["targetId"];


		//check if teleported within last 12h
		gserver->SendObject(client, gserver->CreateError("city.advMoveCastle", -90, "Adv City Teleporter is in cooldown."));

		//else

		//this sends a lot of server.CastleFieldUpdate - are those city owned valleys being released due to teleport?

		PlayerCity * city = client->GetFocusCity();

		city->CastleUpdate();


		client->EnemyArmyUpdate();
		client->FriendArmyUpdate();
		client->SelfArmyUpdate();
		client->SetItem("player.more.castle.1.a", -1);
		client->ItemUpdate("player.more.castle.1.a");
		client->BuffUpdate("MoveCastleCoolDownBuff", "Adv City Teleporter in cooldown.", unixtime() + (12 * 60 * 60 * 1000));


		amf3object obj3;
		obj3["cmd"] = "city.advMoveCastle";
		obj3["data"] = amf3object();
		amf3object & data3 = obj3["data"];
		data3["packageId"] = 0.0f;
		data3["ok"] = 1;
		gserver->SendObject(client, obj3);
		return;
	}
	if ((command == "modifyCastleName"))
	{
		VERIFYCASTLEID();
		CHECKCASTLEID();

		uint32_t castleid = data["castleId"];
		string logurl = data["logUrl"];
		string name = data["name"];

		city->m_cityname = name;
		city->m_logurl = logurl;
		// TODO check valid name and error reporting - city.modifyCastleName

		obj2["cmd"] = "city.modifyCastleName";
		data2["packageId"] = 0.0f;
		data2["ok"] = 1;

		gserver->SendObject(client, obj2);

		city->SaveToDB();

		return;
	}
	if ((command == "modifyFlag"))
	{
		string flag = data["newFlag"];

		client->m_flag = flag;
		// TODO check valid name and error reporting - city.modifyFlag

		client->PlayerUpdate();

		obj2["cmd"] = "city.modifyFlag";
		data2["packageId"] = 0.0f;
		data2["ok"] = 1;

		gserver->SendObject(client, obj2);

		client->SaveToDB();

		return;
	}
	if ((command == "setStopWarState"))
	{
		string pass = data["passWord"];
		string itemid = data["ItemId"];

		if (client->m_password != pass)
		{
			gserver->SendObject(client, gserver->CreateError("city.setStopWarState", -50, "Incorrect account or password."));
			return;
		}

		if (client->GetItemCount(itemid) > 0)
		{
			client->SetItem(itemid, -1);
			client->m_status = DEF_TRUCE;
		}
		else
		{
			int32_t cost = gserver->GetItem(itemid)->cost;
			if (client->m_cents < cost)
			{
				gserver->SendObject(client, gserver->CreateError("city.setStopWarState", -99, "Not enough cents."));
				return;
			}
			client->m_cents -= cost;
		}
		if (itemid == "player.peace.1")
		{
			client->SetBuff("PlayerPeaceBuff", "Truce Agreement activated", timestamp + (12 * 60 * 60 * 1000));
		}
		else
		{
			gserver->SendObject(client, gserver->CreateError("city.setStopWarState", -99, "Invalid state."));

			gserver->SendObject(client, obj2);
			return;
		}

		obj2["cmd"] = "city.setStopWarState";
		data2["packageId"] = 0.0f;
		data2["ok"] = 1;

		gserver->SendObject(client, obj2);

		client->SaveToDB();

		return;
	}
}



