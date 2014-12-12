//
// request_handler.cpp
// Project Spitfire
//
// Copyright (c) 2013 Daizee (rensiadz at gmail dot com)
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
#include "funcs.h"
#include "request_handler.h"
#include <fstream>
#include <sstream>
#include <string>
#include "reply.h"
#include "request.h"
#include "Client.h"
#include "Server.h"
#include "Alliance.h"
#include "AllianceCore.h"
#include "Map.h"
#include "City.h"
#include "Hero.h"
#include "Tile.h"

void ShopUseGoods(amf3object & data, Client * client);
extern Server * gserver;

request_handler::request_handler()
{
}

void request_handler::handle_request(request& req, reply& rep)
{
	//req.object
	//object received - process
	// 	asio::async_write(socket_, reply_.to_buffers(),
	// 		boost::bind(&connection::handle_write, shared_from_this(),
	// 		asio::placeholders::error));
	//amf3object obj = req.object;
	//rep.objects.push_back(amf3object());

	uint64_t timestamp = unixtime();

	amf3object & obj = req.object;
	amf3object & data = obj["data"];
	string cmd = static_cast<string>(obj["cmd"]);

	amf3object obj2 = amf3object();
	obj2["cmd"] = "";
	amf3object & data2 = obj2["data"];
	data2 = amf3object();

	gserver->consoleLogger->information(Poco::format("packet: size: %5.0?d - Command: %s", req.size, cmd));

	char * ctx;

	char * temp = new char[cmd.length() + 2];
	memset(temp, 0, cmd.length() + 2);
	memcpy(temp, cmd.c_str(), cmd.length());
	temp[cmd.length() + 1] = 0;

	string cmdtype, command;

	cmdtype = strtok_s(temp, ".", &ctx);
	if (*ctx != 0)
		command = strtok_s(NULL, ".", &ctx);

	delete[] temp;


	connection & c = *req.conn;
	Client * client = c.client_;

	if (cmdtype != "login")
		if (cmdtype == "" || command == "")
		{
		if (c.client_)
			gserver->consoleLogger->information(Poco::format("0 length command sent clientid: %?d", c.client_->m_clientnumber));
		else
			gserver->consoleLogger->information("0 length command sent from nonexistent client");
		return;
		}

	CHECKCASTLEID();


	PlayerCity * pcity = 0;
	if (client && client->m_currentcityindex != -1)
		pcity = client->m_city[client->m_currentcityindex];


	try
	{
#pragma region common
		if ((cmdtype == "common"))
		{
			if ((command == "worldChat"))
			{
				obj2["cmd"] = "common.channelChat";
				data2["msg"] = data["msg"];
				data2["ok"] = 1;
				data2["channel"] = "world";

				gserver->SendObject(client, obj2);

				amf3object obj3;
				obj3["cmd"] = "server.ChannelChatMsg";
				obj3["data"] = amf3object();
				amf3object & data3 = obj3["data"];
				data3["msg"] = data["msg"];
				data3["languageType"] = 0;
				data3["ownitemid"] = client->m_icon;
				data3["fromUser"] = client->m_playername;
				data3["channel"] = "world";

				std::list<Client*>::iterator playeriter;

				if (gserver->ParseChat(client, data["msg"]))
				{
					for (playeriter = gserver->players.begin(); playeriter != gserver->players.end(); ++playeriter)
					{
						Client * client = *playeriter;
						if (client)
							gserver->SendObject(client, obj3);
					}
				}
				return;
			}
			if ((command == "privateChat"))
			{
				LOCK(M_CLIENTLIST);
				Client * clnt = gserver->GetClientByName(data["targetName"]);
				if (!clnt)
				{
					gserver->SendObject(client, gserver->CreateError("common.privateChat", -41, "Player " + (string)data["targetName"] + " doesn't exist."));
					UNLOCK(M_CLIENTLIST);
					return;
				}
				UNLOCK(M_CLIENTLIST);

				obj2["cmd"] = "common.privateChat";
				data2["msg"] = data["msg"];
				data2["packageId"] = 0.0f;
				data2["ok"] = 1;
				gserver->SendObject(client, obj2);

				amf3object obj3;
				obj3["cmd"] = "server.PrivateChatMessage";
				obj3["data"] = amf3object();
				amf3object & data3 = obj3["data"];
				data3["msg"] = data["msg"];
				data3["chatType"] = 1;
				data3["ownitemid"] = client->m_icon;
				data3["fromUser"] = client->m_playername;
				gserver->SendObject(clnt, obj3);
				return;
			}
			if ((command == "channelChat"))
			{
				obj2["cmd"] = "common.channelChat";
				data2["msg"] = data["msg"];
				data2["ok"] = 1;
				data2["channel"] = "beginner";

				gserver->SendObject(client, obj2);

				amf3object obj3;
				obj3["cmd"] = "server.ChannelChatMsg";
				obj3["data"] = amf3object();
				amf3object & data3 = obj3["data"];
				data3["msg"] = data["sendMsg"];
				data3["languageType"] = 0;
				data3["ownitemid"] = client->m_icon;
				data3["fromUser"] = client->m_playername;
				data3["channel"] = "beginner";


				std::list<Client*>::iterator playeriter;

				if (gserver->ParseChat(client, data["sendMsg"]))
				{
					for (playeriter = gserver->players.begin(); playeriter != gserver->players.end(); ++playeriter)
					{
						Client * client = *playeriter;
						if (client)
							gserver->SendObject(client, obj3);
					}
				}
				return;
			}
			if ((command == "allianceChat"))
			{

				if (client->m_allianceid <= 0)
				{
					gserver->SendObject(client, gserver->CreateError("common.allianceChat", -99, "To send an alliance message, you must be a member of an alliance"));
					return;
				}

				obj2["cmd"] = "common.allianceChat";
				data2["msg"] = data["msg"];
				data2["ok"] = 1;
				data2["channel"] = "alliance";

				gserver->SendObject(client, obj2);

				amf3object obj3;
				obj3["cmd"] = "server.AllianceChatMsg";
				obj3["data"] = amf3object();
				amf3object & data3 = obj3["data"];
				data3["msg"] = data["msg"];
				data3["languageType"] = 0;
				data3["ownitemid"] = client->m_icon;
				data3["fromUser"] = client->m_playername;
				data3["channel"] = "alliance";

				Alliance * alliance = client->GetAlliance();

				LOCK(M_CLIENTLIST);
				if (gserver->ParseChat(client, data["msg"]))
					for (int i = 0; i < alliance->m_members.size(); ++i)
					{
					gserver->SendObject(gserver->GetClient(alliance->m_members[i].clientid), obj3);
					}
				UNLOCK(M_CLIENTLIST);

				return;
			}
			if ((command == "mapInfoSimple"))
			{

				int x1 = data["x1"];
				int x2 = data["x2"];
				int y1 = data["y1"];
				int y2 = data["y2"];
				uint32_t castleId = data["castleId"];


				obj2["cmd"] = "common.mapInfoSimple";
				MULTILOCK(M_CLIENTLIST, M_MAP);
				try {
					obj2["data"] = gserver->map->GetTileRangeObject(c.client_->m_clientnumber, x1, x2, y1, y2);
					UNLOCK(M_CLIENTLIST);
					UNLOCK(M_MAP);
				}
				catch (...)
				{
					UNLOCK(M_CLIENTLIST);
					UNLOCK(M_MAP);
				}

				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "zoneInfo"))
			{
				obj2["cmd"] = "common.zoneInfo";
				data2["packageId"] = 0.0f;
				data2["ok"] = 1;
				amf3array amfarray = amf3array();
				amf3object zone2;
				for (int i = 0; i < 16; ++i)
				{
					amf3object zone = amf3object();
					zone["id"] = i;
					zone["rate"] = gserver->map->m_stats[i].playerrate;
					zone["name"] = gserver->map->states[i];
					zone["playerCount"] = gserver->map->m_stats[i].players;
					zone["castleCount"] = gserver->map->m_stats[i].numbercities;
					amfarray.Add(zone);
				}
				data2["zones"] = amfarray;

				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "getPackage")) //TODO
			{
				obj["ruleId"];//package id to claim
				obj["serial"];//unsure of what this does - is sent as a null string (0 length)

				//on success
				obj2["cmd"] = "common.getPackage";
				data2["packageId"] = 0.0f;
				data2["ok"] = 1;

				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "getPackageList")) //TODO
			{
				obj2["cmd"] = "common.getPackageList";
				data2["packages"] = client->Packages();
				data2["packageId"] = 0.0f;
				data2["ok"] = 1;

				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "getPackageNumber")) //TODO
			{
				obj2["cmd"] = "common.getPackageNumber";
				data2["packageId"] = 0.0f;
				data2["ok"] = 1;
				data2["number"] = 0;//unclaimed package counts

				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "changeUserFace")) //TODO check for valid faceurl (currently can send any link, even offsite)
			{
				obj2["cmd"] = "common.changeUserFace";
				data2["packageId"] = 0.0f;

				string faceurl = data["faceUrl"];
				int sex = data["sex"];

				if (client->m_changedface || faceurl.length() > 30 || faceurl.length() < 0 || sex < 0 || sex > 1)
				{
					//TODO not a valid error message. need to obtain official response for invalid request
					gserver->SendObject(client, gserver->CreateError("common.changeUserFace", -99, "Invalid setting."));
					return;
				}
				data2["ok"] = 1;
				data2["msg"] = "\xE5\xA4\xB4\xE5\x83\x8F\xE8\xAE\xBE\xE7\xBD\xAE\xE6\x88\x90\xE5\x8A\x9F";

				client->m_faceurl = faceurl;
				client->m_sex = sex;

				client->m_changedface = true;

				client->SaveToDB();

				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "delUniteServerPeaceStatus")) //TODO is this correct?
			{
				obj2["cmd"] = "common.delUniteServerPeaceStatus";
				data2["packageId"] = 0.0f;
				data2["ok"] = 1;
				data2["number"] = 0;

				client->m_status = DEF_NORMAL;

				client->SaveToDB();

				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "getItemDefXml")) //TODO lots of things to fix here.
			{
				/*
				Items =		itemType=\"\xE5\xAE\x9D\xE7\x89\xA9\"
				Hot Sale =
				Speed Up =
				Produce =
				Chest =		itemType=\"\xE5\xAE\x9D\xE7\xAE\xB1\"
				*/
				obj2["cmd"] = "common.getItemDefXml";

				data2["ok"] = 1;
				data2["packageId"] = 0.0f;
				string s = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\
<itemdef>\
<items>\
<itemEum id=\"player.box.compensation.e\" name=\"Compensation Package\" itemType=\"\xE5\xAE\x9D\xE7\xAE\xB1\" dayLimit=\"0\" userLimit=\"1\" desc=\"Includes: 10 amulets, 100 cents.\" itemDesc=\"This package was sent to every member of your server to apologize for extended downtime.\" iconUrl=\"images/items/chongzhidalibao.png\" price=\"0\" playerItem=\"true\"/>\
<itemEum id=\"player.box.present.money.44\" name=\"Pamplona Prize Pack\" itemType=\"\xE5\xAE\x9D\xE7\xAE\xB1\" dayLimit=\"0\" userLimit=\"1\" desc=\"Includes: Wooden Bull Opener, Lion Medal, Rose Medal, Cross Medal, Primary Guidelines, Intermediate Guidelines, War Horn, Corselet, Holy Water, Hero Hunting, Truce Agreement, City Teleporter, Amulet.\" itemDesc=\"These packages are delivered as gifts to players for every $30 worth of purchases made during our Run with the Bulls promotion.\" iconUrl=\"images/icon/shop/PamplonaPrizePack.png\" price=\"0\" playerItem=\"true\"/>\
<itemEum id=\"player.box.present.money.45\" name=\"Hollow Wooden Bull\" itemType=\"\xE5\xAE\x9D\xE7\xAE\xB1\" dayLimit=\"0\" userLimit=\"1\" desc=\"Includes: Chest A (Freedom Medal, Justice Medal, Nation Medal, Michelangelo's Script, Plowshares, Arch Saw, Quarrying Tools, Blower, War Ensign, Excalibur, The Wealth of Nations, Amulet) or Chest B (Primary Guidelines, Intermediate Guidelines, Hero Hunting, Merchant Fleet, Plowshares, Double Saw, Quarrying Tools, Blower, Michelangelo's Script, Tax Policy, The Wealth of Nations) or Chest C (Excalibur, War Horn, Corselet, Truce Agreement, War Ensign, Adv City Teleporter, Michelangelo's Script)\" itemDesc=\"These chests are sent to you as Run with the Bulls gifts from your friends in the game. They require a Wooden Bull Opener to open. You can obtain a Wooden Bull Opener for every $30 worth of purchases made during the Run with the Bulls promotion. When opened, you will receive the contents of Hollow Wooden Bull A, B or C at random.\" iconUrl=\"images/icon/shop/HollowWoodenBull.png\" price=\"300\" playerItem=\"true\"/>\
<itemEum id=\"player.key.bull\" name=\"Wooden Bull Opener\" itemType=\"\xE5\xAE\x9D\xE7\xAE\xB1\" dayLimit=\"0\" userLimit=\"0\" desc=\"You can use this key to open one Hollow Wooden Bull sent to you by your friends. If you don’t have any Hollow Wooden Bull, you should ask your friends to send you some!\" itemDesc=\"You can open any Hollow Wooden Bull your friends gave you with this key once.\" iconUrl=\"images/icon/shop/WoodenBullOpener.png\" price=\"0\"/>\
<itemEum id=\"player.running.shoes\" name=\"Extra-Fast Running Shoes\" itemType=\"\xE5\xAE\x9D\xE7\xAE\xB1\" dayLimit=\"0\" userLimit=\"0\" desc=\"A gift from your friends around Run with the Bulls. Use it to get 24 hours of 50% upkeep in ALL your cities any time from July 9th through July 13th. Extra-Fast Running Shoes is stackable (meaning if you already have this buff, using it again will add an additional 24 hours). Once July 14th comes, this item will expire if you haven't used it yet.\" itemDesc=\"Get a 24 hours 50% upkeep buff during July 9th and July 13th.\" iconUrl=\"images/icon/shop/RunningShoes.png\" price=\"0\" playerItem=\"true\"/>\
<itemEum id=\"player.box.test.1\" name=\"Test Item\" itemType=\"\xE5\xAE\x9D\xE7\xAE\xB1\" dayLimit=\"0\" userLimit=\"0\" desc=\"Includes: test items.\" itemDesc=\"This package exists as a test.\" iconUrl=\"images/items/chongzhidalibao.png\" price=\"10\" playerItem=\"true\"/>\
<itemEum id=\"alliance.ritual_of_pact.ultimate\" name=\"Ritual of Pact (Ultimate)\" itemType=\"\xE5\xAE\x9D\xE7\x89\xA9\" dayLimit=\"90\" userLimit=\"0\" desc=\"Ritual of Pact (Ultimate): member limit is 1,000; effective for 90 days; leeway period is 7 days.\" itemDesc=\"It allows alliance to increase member limit to 1,000 once applied, which is effective for 90 days, while multiple applications of this item can lengthen the effective period. Once the item effect is due, 7-day leeway is given to the alliance. During this time, new members are denied to be recruited to the alliance. If no further application of the item, the alliance disbands automatically once the 7-day leeway period passes.\" iconUrl=\"images/items/Ritual_of_Pact_Ultimate.png\" price=\"75\"/>\
<itemEum id=\"player.speak.bronze_publicity_ambassador.permanent\" name=\"Bronze Publicity Ambassador (Permanent)\" itemType=\"\xE5\xAE\x9D\xE7\x89\xA9\" dayLimit=\"3650\" userLimit=\"0\" desc=\"Effect of Bronze Publicity Ambassador (Permanent) can only be replaced by Silver Publicity Ambassador (Permanent) or Gold Publicity Ambassador (Permanent).\" itemDesc=\"Once you apply this item, a special bronze icon will be displayed in front of your name when you speak in the chat box. While this item functions permanently, multiple applications make no difference to the duration of the effective period. Its effect can be replaced by Silver Publicity Ambassador (Permanent) or Gold Publicity Ambassador (Permanent).\" iconUrl=\"images/items/Bronze_Publicity_Ambassador_Permanentb.png\" price=\"75\"/>\
<itemEum id=\"player.speak.bronze_publicity_ambassador.permanent.15\" name=\"Bronze Publicity Ambassador (15-day)\" itemType=\"\xE5\xAE\x9D\xE7\x89\xA9\" dayLimit=\"15\" userLimit=\"0\" desc=\"Once you apply this item, a special bronze icon will be displayed in front of your name when you speak in the chat box. It’s effective for 15 days, while multiple applications can lengthen the effective period. Its effect can be replaced by Bronze Publicity Ambassador (Permanent), Silver Publicity Ambassador (15-day), Silver Publicity Ambassador (Permanent), Gold Publicity Ambassador (15-day) or Gold Publicity Ambassador (Permanent).\" itemDesc=\"Once you apply this item, a special bronze icon will be displayed in front of your name when you speak in the chat box. It’s effective for 15 days, while multiple applications can lengthen the effective period. Its effect can be replaced by Bronze Publicity Ambassador (Permanent), Silver Publicity Ambassador (15-day), Silver Publicity Ambassador (Permanent), Gold Publicity Ambassador (15-day) or Gold Publicity Ambassador (Permanent).\" iconUrl=\"images/items/Bronze_Publicity_Ambassador_15b.png\" price=\"75\"/>\
<itemEum id=\"player.speak.gold_publicity_ambassador.15\" name=\"Gold Publicity Ambassador (15-day)\" itemType=\"\xE5\xAE\x9D\xE7\x89\xA9\" dayLimit=\"15\" userLimit=\"0\" desc=\"Once you apply this item, a special gold icon will be displayed in front of your name when you speak in the chat box. It’s effective for 15 days, while multiple applications can lengthen the effective period. Its effect can be replaced by Gold Publicity Ambassador (Permanent).\" itemDesc=\"Effect of Gold Publicity Ambassador (15-day) can only be replaced by Gold Publicity Ambassador (Permanent).\" iconUrl=\"images/items/gold_publicity_ambassador_15b.png\" price=\"75\"/>\
<itemEum id=\"player.speak.gold_publicity_ambassador.permanent\" name=\"Gold Publicity Ambassador (Permanent)\" itemType=\"\xE5\xAE\x9D\xE7\x89\xA9\" dayLimit=\"3650\" userLimit=\"0\" desc=\"Once you apply this item, a special gold icon will be displayed in front of your name when you speak in the chat box. While this item functions permanently, multiple applications make no difference to the duration of the effective period. \" itemDesc=\"You're the highest level Publicity Ambassador now.\" iconUrl=\"images/items/Gold_Publicity_Ambassador_Permanentb.png\" price=\"75\"/>\
<itemEum id=\"player.speak.silver_publicity_ambassador.15\" name=\"Silver Publicity Ambassador (15-day)\" itemType=\"\xE5\xAE\x9D\xE7\x89\xA9\" dayLimit=\"15\" userLimit=\"0\" desc=\"Effect of Silver Publicity Ambassador (15-day) can only be replaced by Silver Publicity Ambassador (Permanent), Gold Publicity Ambassador (15-day) or Gold Publicity Ambassador (Permanent).\" itemDesc=\"Once you apply this item, a special silver icon will be displayed in front of your name when you speak in the chat box. It’s effective for 15 days, while multiple applications can lengthen the effective period. Its effect can be replaced by Silver Publicity Ambassador (Permanent), Gold Publicity Ambassador (15-day) or Gold Publicity Ambassador (Permanent).\" iconUrl=\"images/items/Silver_Publicity_Ambassador_15b.png\" price=\"75\"/>\
<itemEum id=\"player.speak.silver_publicity_ambassador.permanent\" name=\"Silver Publicity Ambassador (Permanent)\" itemType=\"\xE5\xAE\x9D\xE7\x89\xA9\" dayLimit=\"3650\" userLimit=\"0\" desc=\"Once you apply this item, a special silver icon will be displayed in front of your name when you speak in the chat box. While this item functions permanently, multiple applications make no difference to the duration of the effective period. Its effect can be replaced by Silver Publicity Ambassador (Permanent) or Gold Publicity Ambassador (Permanent).\" itemDesc=\"Effect of Silver Publicity Ambassador (Permanent) can only be replaced by Gold Publicity Ambassador (Permanent).\" iconUrl=\"images/items/silver_publicity_ambassador_permanentb.png\" price=\"75\"/>\
<itemEum id=\"alliance.ritual_of_pact.advanced\" name=\"Ritual of Pact (Advanced)\" itemType=\"\xE5\xAE\x9D\xE7\x89\xA9\" dayLimit=\"15\" userLimit=\"0\" desc=\"Ritual of Pact(Advanced):member limit is 1,000;effective for 15 days; leeway perod is 7 days.\" itemDesc=\"It allows alliance to increase member limit to 1,000 once applied, which is effective for 15 days, while multiple applications of this item can lengthen the effective period. Once the item effect is due, 7-day leeway is given to the alliance. During this time, new members are denied to be recruited to the alliance. If no further application of the item, the alliance disbands automatically once the 7-day leeway period passes.\" iconUrl=\"images/items/Ritual_of_Pact_Advanced.png\" price=\"75\"/>\
<itemEum id=\"alliance.ritual_of_pact.premium\" name=\"Ritual of Pact (Premium)\" itemType=\"\xE5\xAE\x9D\xE7\x89\xA9\" dayLimit=\"30\" userLimit=\"0\" desc=\"Ritual of Pact (Premium): member limit is 1,000; effective for 30 days; leeway period is 7 days.\" itemDesc=\"It allows alliance to increase member limit to 1,000 once applied, which is effective for 30 days, while multiple applications of this item can lengthen the effective period. Once the item effect is due, 7-day leeway is given to the alliance. During this time, new members are denied to be recruited to the alliance. If no further application of the item, the alliance disbands automatically once the 7-day leeway period passes.\" iconUrl=\"images/items/Ritual_of_Pact_Premium.png\" price=\"75\"/>\
<itemEum id=\"consume.1.c\" name=\"Speaker (100 pieces package)\" itemType=\"\xE5\xAE\x9D\xE7\x89\xA9\" dayLimit=\"0\" userLimit=\"0\" desc=\"Used while speaking in World channel and sending group message.\" itemDesc=\"It includes 100 Speakers. It costs one Speaker per sentence when chatting in World channel, while sending a group message costs two. Unpack automatically when purchased.\" iconUrl=\"images/items/biglaba.png\" price=\"200\" playerItem=\"true\"/>\
</items>\
<special>\
<pack id=\"Special Christmas Chest\"/>\
<pack id=\"Special New Year Chest\"/>\
<pack id=\"Special Easter Chest\"/>\
<pack id=\"Special Evony Happiness Chest\"/>\
<pack id=\"Halloween Chest O'Treats\"/>\
<pack id=\"Special Thanksgiving Package\"/>\
<pack id=\"Secret Santa Chest\"/>\
<pack id=\"Valentine's Day Chest \"/>\
<pack id=\"St Patrick's Day Chest\"/>\
<pack id=\"Special Easter Chest\"/>\
<pack id=\"Hollow Wooden Bull\"/>\
</special>\
</itemdef>";
				data2["itemXml"] = s;


				//gserver->m_itemxml;

				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "createNewPlayer"))
			{
				string captcha = data["captcha"];
				string faceUrl = data["faceUrl"];
				string castleName = data["castleName"];
				string flag = data["flag"];
				int sex = data["sex"];
				string userName = data["userName"];
				int zone = data["zone"];

				if ((client->m_accountid > 0) && (client->m_citycount > 0))
				{
					// already has a city
					gserver->SendObject(client, gserver->CreateError("common.createNewPlayer", -86, "City/Account exists."));
					return;
				}


				//check for data error
				if (zone < 0 || zone > 15 || userName.length() < 1 || userName.length() > 20 || flag.length() < 1 || flag.length() > 5 || castleName.length() < 1 || castleName.length() > 10
					|| faceUrl.length() < 1 || faceUrl.length() > 30 || sex < 0 || sex > 1)
				{
					gserver->SendObject(client, gserver->CreateError("common.createNewPlayer", -87, "Invalid data sent."));
					return;
				}

				if (client->m_accountid == 0)
				{
					gserver->consoleLogger->information("Account doesn't exist");
					Session ses(gserver->serverpool->get());
					Statement select(ses);
					select << "SELECT * FROM `accounts` WHERE `username`=?;", use(userName);
					select.execute();
					RecordSet rs(select);

					if (rs.rowCount() > 0)
					{
						//player name exists
						gserver->SendObject(client, gserver->CreateError("common.createNewPlayer", -88, "Player name taken"));
						return;
					}
				}
				//else
			{
				//see if state can support a new city

				if (gserver->map->m_openflats[zone] > 0)
				{
					gserver->map->m_openflats[zone]--;
					//create new account, create new city, then send account details

					char tempc[50];
					int randomid = gserver->map->GetRandomOpenTile(zone);
					int mapsize = gserver->mapsize;
					GETXYFROMID(randomid);
					int x = xfromid;
					int y = yfromid;
					if (gserver->map->m_tile[randomid].m_type != FLAT || gserver->map->m_tile[randomid].m_ownerid != -1)
					{
						gserver->consoleLogger->information("Error. Flat not empty!");
						gserver->SendObject(client, gserver->CreateError("common.createNewPlayer", -25, "Error with account creation. #-25"));
						return;
					}

					string user;
					string flag2;
					string faceUrl2;
					string castlename2;
					user = makesafe(userName);
					castlename2 = makesafe(castleName);
					faceUrl2 = makesafe(faceUrl);
					flag2 = makesafe(flag);
					if (client->m_accountid == 0)
					{

						Session ses(gserver->serverpool->get());
						Statement select(ses);
						select << "SELECT * FROM `accounts` WHERE `username`=?;", use(userName);
						select.execute();
						RecordSet rs(select);
						uint64_t nixtime = unixtime();
						int32_t zero = 0;
						string empty = "";
						Statement stmt = (ses << "INSERT INTO `accounts` (`parentid`, `username`, `lastlogin`, `creation`, `ipaddress`, `status`, `reason`, `sex`, `flag`, `faceurl`, `buffs`, `research`, `items`, `misc`) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, '', '', '', '');", use(client->masteraccountid), use(user), use(nixtime), use(nixtime), use(client->m_ipaddress), use(zero), use(empty), use(sex), use(flag2), use(faceUrl2));
						stmt.execute();
						if (!stmt.done())
						{
							gserver->consoleLogger->information("Unable to create account.");
							gserver->SendObject(client, gserver->CreateError("common.createNewPlayer", -26, "Error with account creation. #-26"));
							return;
						}
						Statement lastinsert = (ses << "SELECT LAST_INSERT_ID()");
						lastinsert.execute();
						RecordSet lsi(lastinsert);
						lsi.moveFirst();
						int32_t lsiv = lsi.value("LAST_INSERT_ID()").convert<int32_t>();
						if (lsiv > 0)
						{
							client->m_accountid = lsiv;
						}
						else
						{
							gserver->consoleLogger->information("Unable to create account.");
							gserver->SendObject(client, gserver->CreateError("common.createNewPlayer", -27, "Error with account creation. #-27"));
							return;
						}

						//res2 = gserver->sql2->QueryRes("SELECT LAST_INSERT_ID()");
						//res2->next();
						//client->m_accountid = res2->getInt(1);
						//delete res2;
					}
					client->m_accountexists = true;

					string temp = "50,10.000000,100.000000,100.000000,100.000000,100.000000,90,0,";
					char temp2[200];
					sprintf_s(temp2, 200, DBL, (double)unixtime());
					temp += temp2;

					// 					if (!gserver->sql2->Query("INSERT INTO `cities` (`accountid`,`misc`,`fieldid`,`name`,`buildings`,`gold`,`food`,`wood`,`iron`,`stone`,`creation`,`transingtrades`,`troop`,`fortification`,`trades`) \
										// 								 VALUES ("XI64", '%s',%d, '%s', '%s',100000,100000,100000,100000,100000,"DBL",'','','','');",
					// 								 client->m_accountid, (char*)temp.c_str(), randomid, castleName, "31,1,-1,0,0.000000,0.000000", (double)unixtime()))
					Session ses(gserver->serverpool->get());
					uint64_t nixtime = unixtime();
					int32_t zero = 0;
					string empty = "";
					string defaultbuildings = "31,1,-1,0,0.000000,0.000000";
					Statement stmt = (ses << "INSERT INTO `cities` (`accountid`,`misc`,`fieldid`,`name`,`buildings`,`gold`,`food`,`wood`,`iron`,`stone`,`creation`,`transingtrades`,`troop`,`fortification`,`trades`) VALUES (?, ?, ?, ?, ?,100000,100000,100000,100000,100000,?,'','','','');",
									  use(client->m_accountid), use(temp), use(randomid), use(castleName), use(defaultbuildings), use(nixtime));
					stmt.execute();
					if (!stmt.done())
					{
						//gserver->FileLog()->Log("Unable to create city.");
						//error making city
						gserver->consoleLogger->information("Error. Unable to insert new city row.");
						gserver->SendObject(client, gserver->CreateError("common.createNewPlayer", -28, "Error with account creation. #-28"));
						return;
					}

					client->m_playername = user;
					client->m_flag = flag2;
					client->m_faceurl = faceUrl2;
					client->m_sex = sex;

					LOCK(M_CASTLELIST);

					PlayerCity * city;

					Statement lastinsert = (ses << "SELECT LAST_INSERT_ID()");
					lastinsert.execute();
					RecordSet lsi(lastinsert);
					lsi.moveFirst();
					uint32_t lsiv = lsi.value("LAST_INSERT_ID()").convert<uint32_t>();
					if (lsiv > 0)
					{
						client->m_accountid = lsiv;
						city = (PlayerCity*)gserver->AddPlayerCity(client, randomid, lsiv);
					}
					else
					{
						gserver->consoleLogger->information("Unable to create account.");
						gserver->SendObject(client, gserver->CreateError("common.createNewPlayer", -29, "Error with account creation. #-29"));
						return;
					}

					//res = gserver->sql2->QueryRes("SELECT LAST_INSERT_ID()");
					//res->next();
					UNLOCK(M_CASTLELIST);

					city->ParseBuildings("31,1,-1,0,0.000000,0.000000");
					city->m_logurl = "images/icon/cityLogo/citylogo_01.png";
					//city->m_accountid = client->m_accountid;
					city->m_cityname = castlename2;
					//city->m_tileid = randomid;
					client->m_currentcityid = city->m_castleid;
					city->m_creation = unixtime();
					client->m_currentcityindex = 0;
					city->SetResources(100000, 100000, 100000, 100000, 100000);
					city->CalculateResources();
					city->CalculateStats();

					/* Send new account data */

					if (client->GetItemCount("consume.1.a") < 10000)
						client->SetItem("consume.1.a", 10000);
					client->m_cents = 5000;


					gserver->SortPlayers();
					gserver->SortHeroes();
					gserver->SortCastles();


					amf3object obj3;
					obj3["cmd"] = "common.createNewPlayer";
					obj3["data"] = amf3object();
					amf3object & data3 = obj3["data"];
					data3["packageId"] = 0.0f;
					data3["ok"] = 1;
					data3["msg"] = "success";
					data3["player"] = client->ToObject();

					gserver->SendObject(client, obj3);

					client->m_connected = true;

					if (client->GetItemCount("consume.1.a") < 10000)
						client->SetItem("consume.1.a", 10000);


					gserver->map->CalculateOpenTiles();

					client->SaveToDB();
					city->SaveToDB();


					return;
				}
				else
				{
					//state is full
					gserver->SendObject(client, gserver->CreateError("common.createNewPlayer", -25, "No open flats exist."));
					return;
				}

			}
				return;
			}
			if (command == "setSecurityCode")
			{
				string code = data["code"];
			}
		}
#pragma endregion
#pragma region city
		if (cmdtype == "city")
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
		}
#pragma endregion
#pragma region report
		if (cmdtype == "report")
		{
			if (command == "deleteReport")
			{
				string reportid = data["idStr"];//always string? always a stringed number?

				amf3object obj3;
				obj3["cmd"] = "report.deleteReport";
				obj3["data"] = amf3object();
				amf3object & data3 = obj3["data"];

				data3["packageId"] = 0.0f;
				data3["ok"] = 1;
				gserver->SendObject(client, obj3);
				return;
			}
			if (command == "markAsRead")
			{

				amf3object obj3;
				obj3["cmd"] = "report.receiveReportList";
				obj3["data"] = amf3object();
				amf3object & data3 = obj3["data"];

				amf3object report;
				report["id"] = 0;//report id (based on current reports you have, or ?)
				report["selected"] = false;
				report["title"] = "Plunder Reports";
				report["startPos"] = "City Name(317,374)";
				report["back"] = false;//??
				report["attack"] = false;//??
				report["targetPos"] = "Daisy(317,371)";
				report["eventTime"] = unixtime();
				report["type"] = 1;//1 = plunder?
				report["armyType"] = 5;//5 = attack?
				report["isRead"] = 0;//0 = unread, 1 = read?
				report["content"] = "<reportData reportUrl=\"http://battleceshi3.evony.com/default.html?20140613/46/f1/46f16df3fb6ca832bc7ac1a182c99060.xml\">\
<battleReport isAttack=\"true\" isAttackSuccess=\"false\" round=\"100\">\
<attackTroop king=\"Daisy\" heroName=\"shitatt\" heroLevel=\"1\" heroUrl=\"images/icon/player/faceA21s.png\">\
<troopUnit typeId=\"2\" count=\"1\" lose=\"0\"/>\
</attackTroop>\
<defendTroop king=\"Daisy1\"/>\
<backTroop>\
<troops heroLevel=\"1\" heroName=\"shitatt\" heroUrl=\"images/icon/player/faceA21s.png\" isHeroBeSeized=\"false\">\
<troopInfo typeId=\"2\" remain=\"1\"/>\
</troops>\
</backTroop>\
</battleReport>\
</reportData>";
				data3["report"] = report;

				gserver->SendObject(client, obj3);

				client->ReportUpdate();
				return;
			}
			if (command == "readOverReport")
			{

			}
			if (command == "receiveReportList")
			{
				int32_t pageno = obj["pageNo"];
				int32_t pagesize = obj["pageSize"];
				int32_t reporttype = obj["reportType"];


				amf3object obj3;
				obj3["cmd"] = "report.receiveReportList";
				obj3["data"] = amf3object();
				amf3object & data3 = obj3["data"];

				amf3array reports;
				amf3object report;
				report["id"] = 0;//report id (based on current reports you have, or ?)
				report["selected"] = false;
				report["title"] = "Plunder Reports";
				report["startPos"] = "City Name(317,374)";
				report["back"] = false;//??
				report["attack"] = false;//??
				report["targetPos"] = "Daisy(317,371)";
				report["eventTime"] = unixtime();
				report["type"] = 1;//1 = plunder?
				report["armyType"] = 5;//5 = attack?
				report["isRead"] = 0;//0 = unread, 1 = read?
				reports.Add(report);

				data3["reports"] = reports;
				data3["pageNo"] = pageno;
				data3["packageId"] = 0.0f;
				data3["ok"] = 1;
				data3["totalPage"] = 0;
				gserver->SendObject(client, obj3);
				return;
			}
		}
#pragma endregion
#pragma region server
		// 	if ((cmdtype == "server"))
		// 	{
		// 		//shouldn't have anything
		// 	}
#pragma endregion
#pragma region castle
		if ((cmdtype == "castle"))
		{
			if ((command == "getAvailableBuildingBean"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				obj2["cmd"] = "castle.getAvailableBuildingBean";
				amf3array buildinglist = amf3array();


				for (int i = 0; i < 35; ++i)
				{
					if (gserver->m_buildingconfig[i][0].inside != 2)
						continue;
					if (gserver->m_buildingconfig[i][0].limit > 0 && gserver->m_buildingconfig[i][0].limit <= pcity->GetBuildingCount(i))
						continue;
					if (gserver->m_buildingconfig[i][0].time > 0)
					{
						amf3object parent;
						amf3object conditionbean;

						double costtime = gserver->m_buildingconfig[i][0].time * 1000;
						double mayorinf = 1;
						if (pcity->m_mayor)
							mayorinf = pow(0.995, pcity->m_mayor->GetManagement());
						costtime = (costtime)* (mayorinf)* (pow(0.9, client->GetResearchLevel(T_CONSTRUCTION)));


						conditionbean["time"] = floor(costtime);
						conditionbean["destructTime"] = gserver->m_buildingconfig[i][0].destructtime;
						conditionbean["wood"] = gserver->m_buildingconfig[i][0].wood;
						conditionbean["food"] = gserver->m_buildingconfig[i][0].food;
						conditionbean["iron"] = gserver->m_buildingconfig[i][0].iron;
						conditionbean["gold"] = 0;
						conditionbean["stone"] = gserver->m_buildingconfig[i][0].stone;

						amf3array buildings = amf3array();
						amf3array items = amf3array();
						amf3array techs = amf3array();

						for_each(gserver->m_buildingconfig[i][0].buildings.begin(), gserver->m_buildingconfig[i][0].buildings.end(), [&](stPrereq & req)
						{
							if (req.id > 0)
							{
								amf3object ta = amf3object();
								ta["level"] = req.level;
								int temp = pcity->GetBuildingLevel(req.id);
								ta["curLevel"] = temp;
								ta["successFlag"] = temp >= req.level ? true : false;
								ta["typeId"] = req.id;
								buildings.Add(ta);
							}
						});
						for_each(gserver->m_buildingconfig[i][0].items.begin(), gserver->m_buildingconfig[i][0].items.end(), [&](stPrereq & req)
						{
							if (req.id > 0)
							{
								amf3object ta = amf3object();
								int temp = client->m_items[req.id].count;
								ta["curNum"] = temp;
								ta["num"] = req.level;
								ta["successFlag"] = temp >= req.level ? true : false;
								ta["id"] = gserver->m_items[req.id].name;
								items.Add(ta);
							}
						});
						for_each(gserver->m_buildingconfig[i][0].techs.begin(), gserver->m_buildingconfig[i][0].techs.end(), [&](stPrereq & req)
						{
							if (req.id > 0)
							{
								amf3object ta = amf3object();
								ta["level"] = req.level;
								int temp = client->GetResearchLevel(req.id);
								ta["curLevel"] = temp;
								ta["successFlag"] = temp >= req.level ? true : false;
								ta["typeId"] = req.id;
								buildings.Add(ta);
							}
						});

						conditionbean["buildings"] = buildings;
						conditionbean["items"] = items;
						conditionbean["techs"] = techs;
						conditionbean["population"] = gserver->m_buildingconfig[i][0].population;
						parent["conditionBean"] = conditionbean;
						parent["typeId"] = i;
						buildinglist.Add(parent);
					}
				}


				data2["builingList"] = buildinglist;
				data2["ok"] = 1;
				data2["packageId"] = 0.0f;

				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "getAvailableBuildingListInside"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				obj2["cmd"] = "castle.getAvailableBuildingListInside";
				amf3array buildinglist = amf3array();

				for (int i = 0; i < 35; ++i)
				{
					if (gserver->m_buildingconfig[i][0].inside != 1)
						continue;
					if (gserver->m_buildingconfig[i][0].limit > 0 && gserver->m_buildingconfig[i][0].limit <= pcity->GetBuildingCount(i))
						continue;
					if (gserver->m_buildingconfig[i][0].time > 0)
					{
						amf3object parent;
						amf3object conditionbean;

						double costtime = gserver->m_buildingconfig[i][0].time;//*1000;
						double mayorinf = 1;
						if (pcity->m_mayor)
							mayorinf = pow(0.995, pcity->m_mayor->GetManagement());
						costtime = (costtime)* (mayorinf)* (pow(0.9, client->GetResearchLevel(T_CONSTRUCTION)));


						conditionbean["time"] = floor(costtime);
						conditionbean["destructTime"] = gserver->m_buildingconfig[i][0].destructtime;
						conditionbean["wood"] = gserver->m_buildingconfig[i][0].wood;
						conditionbean["food"] = gserver->m_buildingconfig[i][0].food;
						conditionbean["iron"] = gserver->m_buildingconfig[i][0].iron;
						conditionbean["gold"] = 0;
						conditionbean["stone"] = gserver->m_buildingconfig[i][0].stone;

						amf3array buildings = amf3array();
						amf3array items = amf3array();
						amf3array techs = amf3array();

						for_each(gserver->m_buildingconfig[i][0].buildings.begin(), gserver->m_buildingconfig[i][0].buildings.end(), [&](stPrereq & req)
						{
							if (req.id > 0)
							{
								amf3object ta = amf3object();
								ta["level"] = req.level;
								int temp = pcity->GetBuildingLevel(req.id);
								ta["curLevel"] = temp;
								ta["successFlag"] = temp >= req.level ? true : false;
								ta["typeId"] = req.id;
								buildings.Add(ta);
							}
						});
						for_each(gserver->m_buildingconfig[i][0].items.begin(), gserver->m_buildingconfig[i][0].items.end(), [&](stPrereq & req)
						{
							if (req.id > 0)
							{
								amf3object ta = amf3object();
								int temp = client->m_items[req.id].count;
								ta["curNum"] = temp;
								ta["num"] = req.level;
								ta["successFlag"] = temp >= req.level ? true : false;
								ta["id"] = gserver->m_items[req.id].name;
								items.Add(ta);
							}
						});
						for_each(gserver->m_buildingconfig[i][0].techs.begin(), gserver->m_buildingconfig[i][0].techs.end(), [&](stPrereq & req)
						{
							if (req.id > 0)
							{
								amf3object ta = amf3object();
								ta["level"] = req.level;
								int temp = client->GetResearchLevel(req.id);
								ta["curLevel"] = temp;
								ta["successFlag"] = temp >= req.level ? true : false;
								ta["typeId"] = req.id;
								buildings.Add(ta);
							}
						});

						conditionbean["buildings"] = buildings;
						conditionbean["items"] = items;
						conditionbean["techs"] = techs;
						conditionbean["population"] = gserver->m_buildingconfig[i][0].population;
						parent["conditionBean"] = conditionbean;
						parent["typeId"] = i;
						buildinglist.Add(parent);
					}
				}


				data2["builingList"] = buildinglist;
				data2["ok"] = 1;
				data2["packageId"] = 0.0f;

				gserver->SendObject(client, obj2);




				/*amf3object obj2;
				obj2["cmd"] = "castle.getAvailableBuildingListInside";
				obj2["data"] = amf3object();
				amf3object & data2 = obj2["data"];
				data2["errorMsg"] = "com.evony.entity.tech.impl.FortificationTech";
				data2["ok"] = -99;
				data2["packageId"] = 0.0f;

				gserver->SendObject(client, obj2);*/

				return;
			}
			if ((command == "getAvailableBuildingListOutside"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				obj2["cmd"] = "castle.getAvailableBuildingListOutside";
				amf3array buildinglist = amf3array();

				for (int i = 0; i < 35; ++i)
				{
					if (gserver->m_buildingconfig[i][0].inside != 0)
						continue;
					if (gserver->m_buildingconfig[i][0].limit > 0 && gserver->m_buildingconfig[i][0].limit <= pcity->GetBuildingCount(i))
						continue;
					if (gserver->m_buildingconfig[i][0].time > 0)
					{
						amf3object parent;
						amf3object conditionbean;

						double costtime = gserver->m_buildingconfig[i][0].time * 1000;
						double mayorinf = 1;
						if (pcity->m_mayor)
							mayorinf = pow(0.995, pcity->m_mayor->GetManagement());
						costtime = (costtime)* (mayorinf)* (pow(0.9, client->GetResearchLevel(T_CONSTRUCTION)));


						conditionbean["time"] = floor(costtime);
						conditionbean["destructTime"] = gserver->m_buildingconfig[i][0].destructtime;
						conditionbean["wood"] = gserver->m_buildingconfig[i][0].wood;
						conditionbean["food"] = gserver->m_buildingconfig[i][0].food;
						conditionbean["iron"] = gserver->m_buildingconfig[i][0].iron;
						conditionbean["gold"] = 0;
						conditionbean["stone"] = gserver->m_buildingconfig[i][0].stone;

						amf3array buildings = amf3array();
						amf3array items = amf3array();
						amf3array techs = amf3array();

						for (stPrereq & req : gserver->m_buildingconfig[i][0].buildings)
						{
							if (req.id > 0)
							{
								amf3object ta = amf3object();
								ta["level"] = req.level;
								int temp = pcity->GetBuildingLevel(req.id);
								ta["curLevel"] = temp;
								ta["successFlag"] = req.level ? true : false;
								ta["typeId"] = req.id;
								buildings.Add(ta);
							}
						}
						for (stPrereq & req : gserver->m_buildingconfig[i][0].items)
						{
							if (req.id > 0)
							{
								amf3object ta = amf3object();
								int temp = client->m_items[req.id].count;
								ta["curNum"] = temp;
								ta["num"] = req.level;
								ta["successFlag"] = req.level ? true : false;
								ta["id"] = gserver->m_items[req.id].name;
								items.Add(ta);
							}
						}
						for (stPrereq & req : gserver->m_buildingconfig[i][0].techs)
						{
							if (req.id > 0)
							{
								amf3object ta = amf3object();
								ta["level"] = req.level;
								int temp = client->GetResearchLevel(req.id);
								ta["curLevel"] = temp;
								ta["successFlag"] = req.level ? true : false;
								ta["typeId"] = req.id;
								buildings.Add(ta);
							}
						}

						conditionbean["buildings"] = buildings;
						conditionbean["items"] = items;
						conditionbean["techs"] = techs;
						conditionbean["population"] = gserver->m_buildingconfig[i][0].population;
						parent["conditionBean"] = conditionbean;
						parent["typeId"] = i;
						buildinglist.Add(parent);
					}
				}


				data2["builingList"] = buildinglist;
				data2["ok"] = 1;
				data2["packageId"] = 0.0f;

				gserver->SendObject(client, obj2);




				/*amf3object obj2;
				obj2["cmd"] = "castle.getAvailableBuildingListInside";
				obj2["data"] = amf3object();
				amf3object & data2 = obj2["data"];
				data2["errorMsg"] = "com.evony.entity.tech.impl.FortificationTech";
				data2["ok"] = -99;
				data2["packageId"] = 0.0f;

				gserver->SendObject(client, obj2);*/

				return;
			}
			if ((command == "newBuilding")) //TODO implement hammer queue system
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				obj2["cmd"] = "castle.newBuilding";

				int buildingtype = data["buildingType"];
				int positionid = data["positionId"];

				pcity->CalculateResources();
				pcity->CalculateStats();

				if (((positionid < -2) || (positionid > 31)) && ((positionid < 1001) || (positionid > 1040)))
				{
					gserver->SendObject(client, gserver->CreateError("castle.newBuilding", -99, "Can't build building."));
					return;
				}

				if ((buildingtype > 34 || buildingtype <= 0) || pcity->GetBuilding(positionid)->type || ((gserver->m_buildingconfig[buildingtype][0].limit > 0) && (gserver->m_buildingconfig[buildingtype][0].limit <= pcity->GetBuildingCount(buildingtype))))
				{
					gserver->SendObject(client, gserver->CreateError("castle.newBuilding", -99, "Can't build building."));
					return;
				}

				for (int i = 0; i < 35; ++i)
				{
					if (pcity->m_innerbuildings[i].status != 0)
					{
						// TODO: Support hammer item for multiple constructions at once
						gserver->SendObject(client, gserver->CreateError("castle.newBuilding", -48, "One building allowed to be built at a time."));
						return;
					}
				}
				for (int i = 0; i < 41; ++i)
				{
					if (pcity->m_outerbuildings[i].status != 0)
					{
						// TODO: Support hammer item for multiple constructions at once
						gserver->SendObject(client, gserver->CreateError("castle.newBuilding", -48, "One building allowed to be built at a time."));
						return;
					}
				}

				if (!pcity->CheckBuildingPrereqs(buildingtype, 0))
				{
					gserver->SendObject(client, gserver->CreateError("castle.newBuilding", -99, "Building Prerequisites not met."));
					return;
				}

				if ((gserver->m_buildingconfig[buildingtype][0].food > pcity->m_resources.food)
					|| (gserver->m_buildingconfig[buildingtype][0].wood > pcity->m_resources.wood)
					|| (gserver->m_buildingconfig[buildingtype][0].stone > pcity->m_resources.stone)
					|| (gserver->m_buildingconfig[buildingtype][0].iron > pcity->m_resources.iron)
					|| (gserver->m_buildingconfig[buildingtype][0].gold > pcity->m_resources.gold))
				{
					gserver->SendObject(client, gserver->CreateError("castle.newBuilding", -1, "Not enough resources."));
					return;
				}
				data2["ok"] = 1;
				data2["packageId"] = 0.0f;

				gserver->SendObject(client, obj2);

				pcity->m_resources.food -= gserver->m_buildingconfig[buildingtype][0].food;
				pcity->m_resources.wood -= gserver->m_buildingconfig[buildingtype][0].wood;
				pcity->m_resources.stone -= gserver->m_buildingconfig[buildingtype][0].stone;
				pcity->m_resources.iron -= gserver->m_buildingconfig[buildingtype][0].iron;
				pcity->m_resources.gold -= gserver->m_buildingconfig[buildingtype][0].gold;


				MULTILOCK(M_CASTLELIST, M_TIMEDLIST);

				stBuildingAction * ba = new stBuildingAction;

				double costtime = gserver->m_buildingconfig[buildingtype][0].time;
				double mayorinf = 1;
				if (pcity->m_mayor)
					mayorinf = pow(0.995, pcity->m_mayor->GetManagement());
				costtime = 1000 * (costtime)* (mayorinf)* (pow(0.9, client->GetResearchLevel(T_CONSTRUCTION)));

				pcity->SetBuilding(buildingtype, 0, positionid, 1, timestamp, timestamp + floor(costtime));

				obj2["cmd"] = "server.BuildComplate";

				data2["buildingBean"] = pcity->GetBuilding(positionid)->ToObject();
				data2["castleId"] = client->m_currentcityid;

				gserver->SendObject(client, obj2);

				pcity->ResourceUpdate();

				client->SaveToDB();
				pcity->SaveToDB();

				stTimedEvent te;
				ba->city = pcity;
				ba->client = client;
				ba->positionid = positionid;
				te.data = ba;
				te.accountid = client->m_accountid;
				te.castleid = pcity->m_castleid;
				te.type = DEF_TIMEDBUILDING;

				gserver->AddTimedEvent(te);


				UNLOCK(M_CASTLELIST);
				UNLOCK(M_TIMEDLIST);
				return;
			}
			if ((command == "destructBuilding")) //TODO implement hammer queue system
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				obj2["cmd"] = "castle.destructBuilding";

				int buildingtype = data["buildingType"];
				int positionid = data["positionId"];
				stBuilding * bldg = pcity->GetBuilding(positionid);

				pcity->CalculateResources();
				pcity->CalculateStats();


				if ((bldg->type > 34 || bldg->type <= 0) || (bldg->level == 0))
				{
					gserver->SendObject(client, gserver->CreateError("castle.destructBuilding", -99, "Can't destroy building."));
					return;
				}


				for (int i = 0; i < 35; ++i)
				{
					if (pcity->m_innerbuildings[i].status != 0)
					{
						gserver->SendObject(client, gserver->CreateError("castle.destructBuilding", -48, "One building allowed to be built at a time."));
						return;
					}
				}
				for (int i = 0; i < 41; ++i)
				{
					if (pcity->m_outerbuildings[i].status != 0)
					{
						gserver->SendObject(client, gserver->CreateError("castle.destructBuilding", -48, "One building allowed to be built at a time."));
						return;
					}
				}

				data2["ok"] = 1;
				data2["packageId"] = 0.0f;

				gserver->SendObject(client, obj2);

				MULTILOCK(M_CASTLELIST, M_TIMEDLIST);

				stBuildingAction * ba = new stBuildingAction;

				stTimedEvent te;
				ba->city = pcity;
				ba->client = client;
				ba->positionid = positionid;
				te.data = ba;
				te.type = DEF_TIMEDBUILDING;

				gserver->AddTimedEvent(te);

				double costtime = gserver->m_buildingconfig[bldg->type][bldg->level - 1].destructtime * 1000;
				double mayorinf = 1;
				if (pcity->m_mayor)
					mayorinf = pow(0.995, pcity->m_mayor->GetManagement());
				costtime = (costtime)* (mayorinf)* (pow(0.9, client->GetResearchLevel(T_CONSTRUCTION)));

				ba->city->SetBuilding(bldg->type, bldg->level, positionid, 2, timestamp, timestamp + floor(costtime));

				obj2["cmd"] = "server.BuildComplate";

				data2["buildingBean"] = ba->city->GetBuilding(positionid)->ToObject();
				data2["castleId"] = client->m_currentcityid;

				gserver->SendObject(client, obj2);

				pcity->ResourceUpdate();

				client->SaveToDB();
				pcity->SaveToDB();

				UNLOCK(M_CASTLELIST);
				UNLOCK(M_TIMEDLIST);
				return;
			}
			if ((command == "upgradeBuilding")) //TODO implement hammer queue system
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				pcity->CalculateResources();
				pcity->CalculateStats();

				int positionid = data["positionId"];
				stBuilding * bldg = pcity->GetBuilding(positionid);
				int buildingtype = bldg->type;
				int buildinglevel = bldg->level;

				obj2["cmd"] = "castle.upgradeBuilding";
				data2["ok"] = 1;
				data2["packageId"] = 0.0f;

				if (bldg->status != 0)
				{
					gserver->SendObject(client, gserver->CreateError("castle.upgradeBuilding", -45, "Invalid building status: Your network connection might have experienced delay or congestion. If this error message persists, please refresh or reload this page to login to the game again."));
					return;
				}

				for (int i = 0; i < 35; ++i)
				{
					if (pcity->m_innerbuildings[i].status != 0)
					{
						gserver->SendObject(client, gserver->CreateError("castle.upgradeBuilding", -48, "One building allowed to be built at a time."));
						return;
					}
				}
				for (int i = 0; i < 41; ++i)
				{
					if (pcity->m_outerbuildings[i].status != 0)
					{
						gserver->SendObject(client, gserver->CreateError("castle.upgradeBuilding", -48, "One building allowed to be built at a time."));
						return;
					}
				}

				if (!pcity->CheckBuildingPrereqs(buildingtype, buildinglevel))
				{
					gserver->SendObject(client, gserver->CreateError("castle.upgradeBuilding", -99, "Building Prerequisites not met."));
					return;
				}

				if ((gserver->m_buildingconfig[buildingtype][buildinglevel].food > pcity->m_resources.food)
					|| (gserver->m_buildingconfig[buildingtype][buildinglevel].wood > pcity->m_resources.wood)
					|| (gserver->m_buildingconfig[buildingtype][buildinglevel].stone > pcity->m_resources.stone)
					|| (gserver->m_buildingconfig[buildingtype][buildinglevel].iron > pcity->m_resources.iron)
					|| (gserver->m_buildingconfig[buildingtype][buildinglevel].gold > pcity->m_resources.gold))
				{
					gserver->SendObject(client, gserver->CreateError("castle.upgradeBuilding", -1, "Not enough resources."));
					return;
				}


				gserver->SendObject(client, obj2);

				pcity->m_resources.food -= gserver->m_buildingconfig[buildingtype][buildinglevel].food;
				pcity->m_resources.wood -= gserver->m_buildingconfig[buildingtype][buildinglevel].wood;
				pcity->m_resources.stone -= gserver->m_buildingconfig[buildingtype][buildinglevel].stone;
				pcity->m_resources.iron -= gserver->m_buildingconfig[buildingtype][buildinglevel].iron;
				pcity->m_resources.gold -= gserver->m_buildingconfig[buildingtype][buildinglevel].gold;

				MULTILOCK(M_CASTLELIST, M_TIMEDLIST);
				stBuildingAction * ba = new stBuildingAction;

				stTimedEvent te;
				ba->city = pcity;
				ba->client = client;
				ba->positionid = positionid;
				te.data = ba;
				te.type = DEF_TIMEDBUILDING;

				gserver->AddTimedEvent(te);

				double costtime = gserver->m_buildingconfig[buildingtype][buildinglevel].time;
				double mayorinf = 1;
				if (pcity->m_mayor)
					mayorinf = pow(0.995, pcity->m_mayor->GetManagement());
				costtime = (costtime)* (mayorinf)* (pow(0.9, client->GetResearchLevel(T_CONSTRUCTION)));

				pcity->ResourceUpdate();

				obj2["cmd"] = "server.BuildComplate";

				ba->city->SetBuilding(buildingtype, buildinglevel, positionid, 1, timestamp, timestamp + (floor(costtime) * 1000));

				data2["buildingBean"] = ba->city->GetBuilding(positionid)->ToObject();
				data2["castleId"] = client->m_currentcityid;

				gserver->SendObject(client, obj2);

				client->SaveToDB();
				pcity->SaveToDB();

				UNLOCK(M_CASTLELIST);
				UNLOCK(M_TIMEDLIST);
				return;
			}
			if ((command == "checkOutUpgrade"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				obj2["cmd"] = "castle.checkOutUpgrade";

				int tileposition = data["positionId"];

				// 		if (tileposition == -1)
				// 			tileposition = 31;
				// 		if (tileposition == -2)
				// 			tileposition = 32;

				int level = pcity->GetBuilding(tileposition)->level;
				int id = pcity->GetBuilding(tileposition)->type;

				if (level >= 10)
				{
					data2["ok"] = 1;
					data2["packageId"] = 0.0f;

					amf3object conditionbean;

					conditionbean["time"] = 31536000;
					conditionbean["destructTime"] = gserver->m_buildingconfig[id][level].destructtime;
					conditionbean["wood"] = 50000000;
					conditionbean["food"] = 50000000;
					conditionbean["iron"] = 50000000;
					conditionbean["gold"] = 0;
					conditionbean["stone"] = 50000000;

					data2["conditionBean"] = conditionbean;

					gserver->SendObject(client, obj2);
					return;
				}

				amf3object conditionbean;

				double costtime = gserver->m_buildingconfig[id][level].time;

				double mayorinf = 1;
				if (pcity->m_mayor)
					mayorinf = pow(0.995, pcity->m_mayor->GetManagement());
				costtime = (costtime)* (mayorinf)* (pow(0.9, client->GetResearchLevel(T_CONSTRUCTION)));

				double desttime = gserver->m_buildingconfig[id][level].destructtime;
				mayorinf = 1;
				if (pcity->m_mayor)
					mayorinf = pow(0.995, pcity->m_mayor->GetManagement());
				desttime = (desttime)* (mayorinf)* (pow(0.9, client->GetResearchLevel(T_CONSTRUCTION)));

				conditionbean["time"] = floor(costtime);
				conditionbean["destructTime"] = desttime;
				conditionbean["wood"] = gserver->m_buildingconfig[id][level].wood;
				conditionbean["food"] = gserver->m_buildingconfig[id][level].food;
				conditionbean["iron"] = gserver->m_buildingconfig[id][level].iron;
				conditionbean["gold"] = 0;
				conditionbean["stone"] = gserver->m_buildingconfig[id][level].stone;

				amf3array buildings = amf3array();
				amf3array items = amf3array();
				amf3array techs = amf3array();

				for_each(gserver->m_buildingconfig[id][level].buildings.begin(), gserver->m_buildingconfig[id][level].buildings.end(), [&](stPrereq & req)
				{
					if (req.id > 0)
					{
						amf3object ta = amf3object();
						ta["level"] = req.level;
						int temp = pcity->GetBuildingLevel(req.id);
						ta["curLevel"] = temp;
						ta["successFlag"] = temp >= req.level ? true : false;
						ta["typeId"] = req.id;
						buildings.Add(ta);
					}
				});
				for_each(gserver->m_buildingconfig[id][level].items.begin(), gserver->m_buildingconfig[id][level].items.end(), [&](stPrereq & req)
				{
					if (req.id > 0)
					{
						amf3object ta = amf3object();
						int temp = client->m_items[req.id].count;
						ta["curNum"] = temp;
						ta["num"] = req.level;
						ta["successFlag"] = temp >= req.level ? true : false;
						ta["id"] = gserver->m_items[req.id].name;
						items.Add(ta);
					}
				});
				for_each(gserver->m_buildingconfig[id][level].techs.begin(), gserver->m_buildingconfig[id][level].techs.end(), [&](stPrereq & req)
				{
					if (req.id > 0)
					{
						amf3object ta = amf3object();
						ta["level"] = req.level;
						int temp = client->GetResearchLevel(req.id);
						ta["curLevel"] = temp;
						ta["successFlag"] = temp >= req.level ? true : false;
						ta["typeId"] = req.id;
						buildings.Add(ta);
					}
				});

				conditionbean["buildings"] = buildings;
				conditionbean["items"] = items;
				conditionbean["techs"] = techs;
				conditionbean["population"] = gserver->m_buildingconfig[id][level].population;


				data2["conditionBean"] = conditionbean;
				data2["ok"] = 1;
				data2["packageId"] = 0.0f;

				gserver->SendObject(client, obj2);


				return;
			}
			if ((command == "speedUpBuildCommand"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				string speeditemid;
				int positionid = data["positionId"];
				uint32_t castleid = data["castleId"];

				speeditemid = static_cast<string>(data["itemId"]);

				if (speeditemid == "free.speed")
				{
					//check if under 5 mins

					stBuilding * building = pcity->GetBuilding(positionid);

					if ((building->endtime - building->starttime) <= 5 * 60 * 1000)
					{
						//under 5 mins
						obj2["cmd"] = "castle.speedUpBuildCommand";
						data2["packageId"] = 0.0f;
						data2["ok"] = 1;

						gserver->SendObject(client, obj2);

						building->endtime -= 5 * 60 * 1000;

						client->SaveToDB();
						pcity->SaveToDB();
					}
					else
					{
						//over 5 mins
						gserver->SendObject(client, gserver->CreateError("castle.speedUpBuildCommand", -99, "Invalid speed up."));// TODO get 5 min speed up error - castle.speedUpBuildCommand
					}
				}
				else
				{
					//is not under 5 mins, apply an item
					int itemcount = client->GetItemCount((string)speeditemid);

					int cents = 0;
					int reducetime = 0;

					obj2["cmd"] = "castle.speedUpBuildCommand";
					data2["packageId"] = 0.0f;
					data2["ok"] = 1;

					gserver->SendObject(client, obj2);

					// TODO reduce time on building being sped up -- castle.speedUpBuildCommand

					stBuilding * building = pcity->GetBuilding(positionid);
					if (speeditemid == "consume.2.a")
					{
						cents = 5;
						reducetime = 15 * 60 * 1000;
					}
					else if (speeditemid == "consume.2.b")
					{
						cents = 10;
						reducetime = 60 * 60 * 1000;
					}
					else if (speeditemid == "consume.2.b.1")
					{
						cents = 20;
						reducetime = 150 * 60 * 1000;
					}
					else if (speeditemid == "consume.2.c")
					{
						cents = 50;
						reducetime = 8 * 60 * 60 * 1000;
					}
					else if (speeditemid == "consume.2.c.1")
					{
						cents = 80;
						reducetime = ((rand() % 21) + 10) * 60 * 60 * 1000;
					}
					else if (speeditemid == "consume.2.d")
					{
						cents = 120;
						reducetime = ((building->endtime - building->starttime)*0.30);
					}
					else if (speeditemid == "coins.speed")
					{
						cents = 200;
						reducetime = (building->endtime - building->starttime);
					}

					if (itemcount <= 0)
					{
						if (client->m_cents < cents)
						{
							// TODO find error value -- castle.speedUpBuildCommand
							gserver->SendObject(client, gserver->CreateError("castle.speedUpBuildCommand", -99, "Not enough cents."));// not enough item and not enough cents
							return;
						}
						//not enough item, but can buy with cents
						client->m_cents -= cents;
						client->PlayerUpdate();
					}
					else
					{ //has item
						client->SetItem((string)speeditemid, -1);
					}

					building->endtime -= reducetime;

					pcity->SetBuilding(building->type, building->level, building->id, building->status, building->starttime, building->endtime);

					obj2["cmd"] = "server.BuildComplate";

					data2["buildingBean"] = pcity->GetBuilding(positionid)->ToObject();
					data2["castleId"] = client->m_currentcityid;

					gserver->SendObject(client, obj2);

					client->SaveToDB();
					pcity->SaveToDB();

					return;
				}
				return;
			}
			if ((command == "getCoinsNeed"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				int positionid = data["positionId"];
				uint32_t castleid = data["castleId"];

				obj2["cmd"] = "castle.getCoinsNeed";
				data2["packageId"] = 0.0f;
				data2["ok"] = 1;
				data2["coinsNeed"] = 200;//TODO calculate correct cents cost based on time

				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "cancleBuildCommand"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				uint32_t castleid = data["castleId"];
				int positionid = data["positionId"];

				obj2["cmd"] = "castle.cancleBuildCommand";
				data2["ok"] = 1;
				data2["packageId"] = 0.0f;

				LOCK(M_TIMEDLIST);
				std::list<stTimedEvent>::iterator iter;
				if (gserver->buildinglist.size() > 0)
					for (iter = gserver->buildinglist.begin(); iter != gserver->buildinglist.end();)
					{
					stBuildingAction * ba = (stBuildingAction *)iter->data;
					if (ba->positionid == positionid)
					{
						Client * client = ba->client;
						PlayerCity * city = ba->city;
						stBuilding * bldg = ba->city->GetBuilding(ba->positionid);
						if (bldg->status != 0)
						{
							bldg->status = 0;

							if (bldg->status == 1)
							{
								stResources res;
								res.food = gserver->m_buildingconfig[bldg->type][bldg->level].food;
								res.wood = gserver->m_buildingconfig[bldg->type][bldg->level].wood;
								res.stone = gserver->m_buildingconfig[bldg->type][bldg->level].stone;
								res.iron = gserver->m_buildingconfig[bldg->type][bldg->level].iron;
								res.gold = gserver->m_buildingconfig[bldg->type][bldg->level].gold;
								pcity->m_resources += res;
							}

							gserver->buildinglist.erase(iter++);

							if (bldg->level == 0)
								ba->city->SetBuilding(0, 0, ba->positionid, 0, 0.0, 0.0);
							else
								ba->city->SetBuilding(bldg->type, bldg->level, ba->positionid, 0, 0.0, 0.0);

							client->CalculateResources();
							city->CalculateStats();

							//gserver->SendObject(client->socket, obj);

							obj2["cmd"] = "server.BuildComplate";

							data2["buildingBean"] = ba->city->GetBuilding(positionid)->ToObject();
							data2["castleId"] = client->m_currentcityid;

							gserver->SendObject(client, obj2);

							client->SaveToDB();
							pcity->SaveToDB();

							delete ba;

							UNLOCK(M_TIMEDLIST);

							return;
						}
						else
						{
							gserver->SendObject(client, gserver->CreateError("castle.cancleBuildCommand", -99, "Not being constructed."));
							UNLOCK(M_TIMEDLIST);
							return;
						}
					}
					++iter;
					}
				UNLOCK(M_TIMEDLIST);
				return;
			}
		}
#pragma endregion
#pragma region field
		if ((cmdtype == "field"))
		{
			if ((command == "getOtherFieldInfo"))
			{
				int fieldid = data["fieldId"];

				obj2["cmd"] = "field.getOtherFieldInfo";

				amf3object bean;

				MULTILOCK2(M_MAP, M_CLIENTLIST, M_ALLIANCELIST);
				data2["bean"] = gserver->map->GetMapCastle(fieldid, c.client_->m_clientnumber);
				UNLOCK(M_ALLIANCELIST);
				UNLOCK(M_CLIENTLIST);
				UNLOCK(M_MAP);

				//data2["errorMsg"] = "";
				data2["ok"] = 1;
				data2["packageId"] = 0.0f;

				gserver->SendObject(client, obj2);
				return;
			}
		}
#pragma endregion
#pragma region quest
		if ((cmdtype == "quest"))
		{
			if ((command == "getQuestType")) //quest info requested every 3 mins
			{
				VERIFYCASTLEID();

				//TODO create or copy quests

				uint32_t castleid = data["castleId"];
				int questtype = data["type"];

				if (questtype == 1)
				{
					obj2["cmd"] = "quest.getQuestType";
					data2["ok"] = 1;
					data2["packageId"] = 0.0f;
					amf3array types = amf3array();

					amf3object dailygift = amf3object();
					dailygift["description"] = "Rebuild";
					dailygift["mainId"] = 1;
					dailygift["isFinish"] = false;
					dailygift["name"] = "Rebuild";
					dailygift["typeId"] = 66;
					types.Add(dailygift);

					dailygift["description"] = "Domain Expansion";
					dailygift["mainId"] = 1;
					dailygift["isFinish"] = false;
					dailygift["name"] = "Domain Expansion";
					dailygift["typeId"] = 72;
					types.Add(dailygift);

					data2["types"] = types;

					gserver->SendObject(client, obj2);
				}
				else if (questtype == 3)//dailies
				{
					obj2["cmd"] = "quest.getQuestType";
					data2["ok"] = 1;
					data2["packageId"] = 0.0f;
					amf3array types = amf3array();

					amf3object dailygift = amf3object();
					dailygift["description"] = "Daily Gift";
					dailygift["mainId"] = 3;
					dailygift["isFinish"] = false;
					dailygift["name"] = "Daily Gift";
					dailygift["typeId"] = 94;
					types.Add(dailygift);
					data2["types"] = types;

					gserver->SendObject(client, obj2);
				}

				return;
			}
			if ((command == "getQuestList"))
			{
				VERIFYCASTLEID();

				uint32_t castleid = data["castleId"];

				obj2["cmd"] = "quest.getQuestList";
				data2["ok"] = 1;
				data2["packageId"] = 0.0f;

				amf3array quests = amf3array();

				amf3object questobject = amf3object();
				questobject["description"] = "quest.getQuestList DESCRIPTION";
				questobject["manual"] = "WIN THE GAME LA";
				questobject["isCard"] = false;
				questobject["isFinish"] = false;
				questobject["name"] = "CHAMPION OF SERVERS";
				questobject["award"] = "Gold 187,438,274,822,314";

				amf3array targets = amf3array();
				amf3object objective = amf3object();
				objective["name"] = "Winning the game.";
				objective["finished"] = false;
				targets.Add(objective);

				questobject["targets"] = targets;
				questobject["questId"] = 63;


				quests.Add(questobject);



				data2["quests"] = quests;



				gserver->SendObject(client, obj2);
				return;
			}
		}
#pragma endregion
#pragma region alliance
		if ((cmdtype == "alliance"))
		{
			if ((command == "isHasAlliance"))
			{
				obj2["cmd"] = "alliance.isHasAlliance";


				if (client->HasAlliance())
				{
					data2["ok"] = 1;
					data2["packageId"] = 0.0f;


					LOCK(M_ALLIANCELIST);
					Alliance * alliance = client->GetAlliance();
					Alliance * tempalliance;

					data2["serialVersionUID"] = 1.0f;

					amf3array middleList = amf3array();
					amf3array enemyList = amf3array();
					amf3array friendlyList = amf3array();

					std::vector<int64_t>::iterator iter;
					if (alliance->m_neutral.size() > 0)
						for (iter = alliance->m_neutral.begin(); iter != alliance->m_neutral.end(); ++iter)
						{
						tempalliance = client->m_main->m_alliances->AllianceById(*iter);
						if (tempalliance != (Alliance*)-1)
						{
							amf3object ta = amf3object();
							ta["rank"] = tempalliance->m_prestigerank;
							ta["honor"] = tempalliance->m_honor;// HACK: might be 0 -- alliance.isHasAlliance
							ta["allianceName"] = tempalliance->m_name.c_str();
							ta["memberCount"] = tempalliance->m_currentmembers;
							ta["aPrestigeCount"] = tempalliance->m_prestige;// HACK: might be 0 -- alliance.isHasAlliance
							middleList.Add(ta);
						}
						}
					if (alliance->m_enemies.size() > 0)
						for (iter = alliance->m_enemies.begin(); iter != alliance->m_enemies.end(); ++iter)
						{
						tempalliance = client->m_main->m_alliances->AllianceById(*iter);
						if (tempalliance != (Alliance*)-1)
						{
							amf3object ta = amf3object();
							ta["rank"] = tempalliance->m_prestigerank;
							ta["honor"] = tempalliance->m_honor;// HACK: might be 0 -- alliance.isHasAlliance
							ta["allianceName"] = tempalliance->m_name.c_str();
							ta["memberCount"] = tempalliance->m_currentmembers;
							ta["aPrestigeCount"] = tempalliance->m_prestige;// HACK: might be 0 -- alliance.isHasAlliance
							enemyList.Add(ta);
						}
						}
					if (alliance->m_allies.size() > 0)
						for (iter = alliance->m_allies.begin(); iter != alliance->m_allies.end(); ++iter)
						{
						tempalliance = client->m_main->m_alliances->AllianceById(*iter);
						if (tempalliance != (Alliance*)-1)
						{
							amf3object ta = amf3object();
							ta["rank"] = tempalliance->m_prestigerank;
							ta["honor"] = tempalliance->m_honor;// HACK: might be 0 -- alliance.isHasAlliance
							ta["allianceName"] = tempalliance->m_name.c_str();
							ta["memberCount"] = tempalliance->m_currentmembers;
							ta["aPrestigeCount"] = tempalliance->m_prestige;// HACK: might be 0 -- alliance.isHasAlliance
							friendlyList.Add(ta);
						}
						}
					data2["middleList"] = middleList;
					data2["enemyList"] = enemyList;
					data2["friendlyList"] = friendlyList;
					UNLOCK(M_ALLIANCELIST);


					data2["indexAllianceInfoBean"] = alliance->indexAllianceInfoBean();
				}
				else
				{
					data2["ok"] = -99;
					data2["packageId"] = 0.0f;
					data2["serialVersionUID"] = 0.0f;
					data2["errorMsg"] = "You are not in any alliance.";
				}

				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "leaderWantUserInAllianceList"))
			{
				obj2["cmd"] = "alliance.leaderWantUserInAllianceList";
				data2["ok"] = 1;
				data2["packageId"] = 0.0f;

				LOCK(M_ALLIANCELIST);
				data2["allianceAddPlayerByLeaderInfoBeanList"] = amf3array();

				//TODO: this is your list of alliances that have invited you?-- alliance.leaderWantUserInAllianceList

				amf3array & allianceinfo = *(amf3array*)data2["allianceAddPlayerByLeaderInfoBeanList"];

				for (int i = 0; i < DEF_MAXALLIANCES; ++i)
				{
					if (gserver->m_alliances->m_alliances[i])
					{
						Alliance * alliance = gserver->m_alliances->m_alliances[i];
						if (alliance->m_invites.size())
						{
							std::vector<Alliance::stInviteList>::iterator iter;
							for (iter = alliance->m_invites.begin(); iter != alliance->m_invites.end(); ++iter)
							{
								if (iter->client->m_accountid == client->m_accountid)
								{
									amf3object invite = amf3object();
									invite["prestige"] = alliance->m_prestige;
									invite["rank"] = alliance->m_prestigerank;
									invite["allianceName"] = alliance->m_name;
									invite["memberCount"] = alliance->m_currentmembers;
									allianceinfo.Add(invite);
								}
							}
						}
					}
				}


				UNLOCK(M_ALLIANCELIST);
				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "rejectComeinAlliance"))
			{
				obj2["cmd"] = "alliance.rejectComeinAlliance";
				data2["packageId"] = 0.0f;

				string alliancename = data["allianceName"];

				Alliance * alliance = gserver->m_alliances->AllianceByName(alliancename);
				if (alliance == (Alliance*)-1)
				{
					gserver->SendObject(client, gserver->CreateError("alliance.rejectComeinAlliance", -99, "Alliance no longer exists."));
					return;
				}

				LOCK(M_ALLIANCELIST);
				if (alliance->m_invites.size())
				{
					std::vector<Alliance::stInviteList>::iterator iter;
					for (iter = alliance->m_invites.begin(); iter != alliance->m_invites.end(); ++iter)
					{
						if (iter->client->m_accountid == client->m_accountid)
						{
							alliance->m_invites.erase(iter);
							data2["ok"] = 1;

							gserver->SendObject(client, obj2);
							return;
						}
					}
				}
				else
				{
					gserver->SendObject(client, gserver->CreateError("alliance.rejectComeinAlliance", -99, "Invite no longer exists."));
					return;
				}
				UNLOCK(M_ALLIANCELIST);
				gserver->SendObject(client, gserver->CreateError("alliance.rejectComeinAlliance", -99, "An unknown error occurred."));
				return;
			}
			if ((command == "agreeComeinAllianceList"))
			{
				obj2["cmd"] = "alliance.agreeComeinAllianceList";
				data2["packageId"] = 0.0f;

				if ((client->m_allianceid == 0) || (client->m_alliancerank > DEF_ALLIANCEPRES))
				{
					gserver->SendObject(client, gserver->CreateError("alliance.agreeComeinAllianceList", -99, "Not enough rank."));
					return;
				}

				LOCK(M_ALLIANCELIST);
				Alliance * alliance = gserver->m_alliances->AllianceById(client->m_allianceid);

				if (alliance == (Alliance*)-1)
				{
					gserver->SendObject(client, gserver->CreateError("alliance.agreeComeinAllianceList", -99, "Alliance does not exist."));
					return;
				}

				amf3array allianceinfo = amf3array();

				for (int i = 0; i < alliance->m_invites.size(); ++i)
				{
					if (alliance->m_invites[i].inviteperson.length() == 0)
					{
						Client * pcl = alliance->m_invites[i].client;
						amf3object iobj = amf3object();
						iobj["prestige"] = pcl->Prestige();
						iobj["rank"] = pcl->m_prestigerank;
						iobj["userName"] = pcl->m_playername;
						iobj["inviteTime"] = alliance->m_invites[i].invitetime;
						iobj["castleCount"] = pcl->m_citycount;
						allianceinfo.Add(iobj);
					}
				}
				UNLOCK(M_ALLIANCELIST);
				data2["ok"] = 1;

				data2["allianceAddPlayerByUserInfoBeanList"] = allianceinfo;

				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "resignForAlliance"))
			{
				obj2["cmd"] = "alliance.resignForAlliance";
				data2["packageId"] = 0.0f;

				if (!client->HasAlliance())
				{
					gserver->SendObject(client, gserver->CreateError("alliance.resignForAlliance", -99, "You are not in an alliance."));
					return;
				}
				Alliance * alliance = client->GetAlliance();
				if ((client->m_alliancerank == DEF_ALLIANCEHOST) && (alliance->m_currentmembers > 1))
				{
					gserver->SendObject(client, gserver->CreateError("alliance.resignForAlliance", -99, "Resignation refused. Please transfer your host title to other before you resign."));
					return;
				}
				else
				{
					//Only person left is host - disband alliance
					if (!gserver->m_alliances->RemoveFromAlliance(client->m_allianceid, client))
					{
						gserver->SendObject(client, gserver->CreateError("alliance.resignForAlliance", -99, "Unable to leave alliance. Please contact support."));
						return;
					}
					gserver->m_alliances->DeleteAlliance(alliance);
					return;
				}

				LOCK(M_ALLIANCELIST);
				if (!gserver->m_alliances->RemoveFromAlliance(client->m_allianceid, client))
				{
					gserver->SendObject(client, gserver->CreateError("alliance.resignForAlliance", -99, "Unable to leave alliance. Please contact support."));
					UNLOCK(M_ALLIANCELIST);
					return;
				}
				UNLOCK(M_ALLIANCELIST);

				data2["ok"] = 1;

				client->SaveToDB();

				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "getAllianceMembers"))
			{
				obj2["cmd"] = "alliance.getAllianceMembers";
				data2["packageId"] = 0.0f;

				if (client->m_allianceid < 0)
				{
					gserver->SendObject(client, gserver->CreateError("alliance.getAllianceMembers", -99, "Not a member of an alliance."));
					return;
				}
				MULTILOCK(M_ALLIANCELIST, M_CLIENTLIST);
				Alliance * alliance = gserver->m_alliances->AllianceById(client->m_allianceid);

				if (alliance == (Alliance*)-1)
				{
					gserver->SendObject(client, gserver->CreateError("alliance.getAllianceMembers", -99, "Invalid Alliance."));
					UNLOCK(M_CLIENTLIST);
					UNLOCK(M_ALLIANCELIST);
					return;
				}

				amf3array members = amf3array();

				for (int k = 0; k < alliance->m_members.size(); ++k)
				{
					Client * client = gserver->GetClient(alliance->m_members[k].clientid);
					if (client)
					{
						amf3object temp = amf3object();
						temp["createrTime"] = 0;
						temp["alliance"] = alliance->m_name;
						temp["office"] = client->m_office;
						temp["allianceLevel"] = AllianceCore::GetAllianceRank(client->m_alliancerank);
						temp["sex"] = client->m_sex;
						temp["levelId"] = client->m_alliancerank;
						temp["honor"] = client->m_honor;
						temp["bdenyotherplayer"] = client->m_bdenyotherplayer;
						temp["id"] = client->m_accountid;
						temp["accountName"] = "";
						temp["prestige"] = client->Prestige();
						temp["faceUrl"] = client->m_faceurl;
						temp["flag"] = client->m_flag;
						temp["userId"] = client->masteraccountid;
						temp["userName"] = client->m_playername;
						temp["castleCount"] = client->m_citycount;
						temp["titleId"] = client->m_title;
						temp["medal"] = 0;
						temp["ranking"] = client->m_prestigerank;
						temp["lastLoginTime"] = client->m_lastlogin;
						temp["population"] = client->m_population;
						members.Add(temp);
					}
				}

				data2["ok"] = 1;
				data2["members"] = members;

				UNLOCK(M_CLIENTLIST);
				UNLOCK(M_ALLIANCELIST);
				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "getAllianceWanted"))
			{
				obj2["cmd"] = "alliance.getAllianceWanted";
				data2["ok"] = 1;
				data2["packageId"] = 0.0f;

				if (client->m_allianceapply.length() > 0)
					data2["allianceName"] = client->m_allianceapply;

				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "getAllianceInfo"))
			{
				obj2["cmd"] = "alliance.getAllianceInfo";
				data2["ok"] = 1;
				data2["packageId"] = 0.0f;

				string alliancename = data["allianceName"];

				LOCK(M_ALLIANCELIST);
				Alliance * alliance = gserver->m_alliances->AllianceByName(alliancename);

				if (alliance == (Alliance*)-1)
				{
					gserver->SendObject(client, gserver->CreateError("alliance.getAllianceInfo", -99, "Alliance does not exist."));
					return;
				}

				data2["leader"] = alliance->m_owner;
				data2["prestigeCount"] = alliance->m_prestige;
				data2["ranking"] = alliance->m_prestigerank;
				data2["memberCount"] = alliance->m_currentmembers;
				data2["allinaceInfo"] = alliance->m_intro;
				data2["creator"] = alliance->m_founder;
				UNLOCK(M_ALLIANCELIST);


				data2["ok"] = 1;
				data2["packageId"] = 0.0f;

				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "userWantInAlliance"))
			{
				obj2["cmd"] = "alliance.userWantInAlliance";
				data2["ok"] = 1;
				data2["packageId"] = 0.0f;

				string alliancename = data["allianceName"];

				Alliance * alliance = gserver->m_alliances->AllianceByName(alliancename);

				if (alliance == (Alliance*)-1)
				{
					gserver->SendObject(client, gserver->CreateError("alliance.userWantInAlliance", -99, "Alliance does not exist."));
					return;
				}

				if (client->m_allianceapply.length() != 0)
				{
					gserver->m_alliances->AllianceByName(client->m_allianceapply)->UnRequestJoin(client);
				}

				alliance->RequestJoin(client, timestamp);


				client->m_allianceapply = alliancename;
				client->m_allianceapplytime = timestamp;

				alliance->SendAllianceMessage(client->m_playername + " has applied to join your alliance", false, false);

				data2["ok"] = 1;
				data2["packageId"] = 0.0f;

				client->SaveToDB();

				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "cancelUserWantInAlliance"))
			{
				obj2["cmd"] = "alliance.cancelUserWantInAlliance";
				data2["ok"] = 1;
				data2["packageId"] = 0.0f;

				string alliancename = data["allianceName"];

				if (client->m_allianceapply.length() == 0)
				{
					gserver->SendObject(client, gserver->CreateError("alliance.cancelUserWantInAlliance", -99, "Not applied."));
					return;
				}

				LOCK(M_ALLIANCELIST);
				Alliance * alliance = gserver->m_alliances->AllianceByName(alliancename);

				if (alliance == (Alliance*)-1)
				{
					gserver->SendObject(client, gserver->CreateError("alliance.cancelUserWantInAlliance", -99, "Alliance does not exist."));
					return;
				}

				alliance->UnRequestJoin(client);
				UNLOCK(M_ALLIANCELIST);


				client->m_allianceapply = "";
				client->m_allianceapplytime = 0;

				client->SaveToDB();

				data2["ok"] = 1;
				data2["packageId"] = 0.0f;

				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "getMilitarySituationList"))
			{
				int pagesize = data["pageSize"];
				int pageno = data["pageNo"];

				obj2["cmd"] = "alliance.getMilitarySituationList";
				data2["packageId"] = 0.0f;
				data2["ok"] = 1;

				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "addUsertoAllianceList"))
			{
				obj2["cmd"] = "alliance.addUsertoAllianceList";
				data2["packageId"] = 0.0f;

				if (!client->HasAlliance())
				{
					gserver->SendObject(client, gserver->CreateError("alliance.addUsertoAllianceList", -99, "Not a member of an alliance."));
					return;
				}

				data2["ok"] = 1;

				LOCK(M_ALLIANCELIST);
				amf3array listarray = amf3array();

				Alliance * alliance = client->GetAlliance();

				for (int i = 0; i < alliance->m_invites.size(); ++i)
				{
					if (alliance->m_invites[i].inviteperson.length() != 0)
					{
						Client * pcl = alliance->m_invites[i].client;
						amf3object iobj = amf3object();
						iobj["prestige"] = pcl->m_prestige;
						iobj["rank"] = pcl->m_prestigerank;
						iobj["invitePerson"] = alliance->m_invites[i].inviteperson;
						iobj["userName"] = pcl->m_playername;
						iobj["inviteTime"] = alliance->m_invites[i].invitetime;
						listarray.Add(iobj);
					}
				}
				UNLOCK(M_ALLIANCELIST);

				data2["allianceAddPlayerInfoBeanList"] = listarray;

				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "addUsertoAlliance"))
			{
				//invite player
				obj2["cmd"] = "alliance.addUsertoAlliance";
				data2["packageId"] = 0.0f;

				if (!client->HasAlliance())
				{
					gserver->SendObject(client, gserver->CreateError("alliance.addUsertoAlliance", -99, "Not a member of an alliance."));
					return;
				}
				if (client->m_alliancerank > DEF_ALLIANCEPRES)
				{
					gserver->SendObject(client, gserver->CreateError("alliance.addUsertoAlliance", -99, "Not enough rank."));
					return;
				}

				//TODO: copy 24h alliance join cooldown?
				/*
				["data"] Type: Object - Value: Object
				["packageId"] Type: Number - Value: 0.000000
				["ok"] Type: Integer - Value: -301
				["errorMsg"] Type: String - Value: Can not join the same Alliance again in 23 hours.
				*/

				MULTILOCK(M_ALLIANCELIST, M_CLIENTLIST);
				string username = data["userName"];
				Client * invitee = gserver->GetClientByName(username);

				if (invitee == 0)
				{
					string msg;
					msg = "Player ";
					msg += username;
					msg += " doesn't exist.";
					gserver->SendObject(client, gserver->CreateError("alliance.addUsertoAlliance", -41, msg));
					UNLOCK(M_ALLIANCELIST);
					UNLOCK(M_CLIENTLIST);
					return;
				}
				if (invitee->HasAlliance())
				{
					string msg;
					msg = username;
					msg += "is already a member of other alliance.";
					gserver->SendObject(client, gserver->CreateError("alliance.addUsertoAlliance", -99, msg));
					UNLOCK(M_ALLIANCELIST);
					UNLOCK(M_CLIENTLIST);
					return;
				}
				data2["ok"] = 1;

				Alliance * alliance = client->GetAlliance();

				Alliance::stInviteList invite;
				invite.inviteperson = client->m_playername;
				invite.invitetime = timestamp;
				invite.client = invitee;

				alliance->m_invites.push_back(invite);
				UNLOCK(M_ALLIANCELIST);
				UNLOCK(M_CLIENTLIST);

				gserver->SendMessage(client, "You have been invited to the alliance " + alliance->m_name);

				gserver->SendObject(client, obj2);

				alliance->SaveToDB();

				return;
			}
			if ((command == "canceladdUsertoAlliance"))
			{
				//TODO: permission check
				obj2["cmd"] = "alliance.canceladdUsertoAlliance";
				data2["packageId"] = 0.0f;

				if (!client->HasAlliance())
				{
					gserver->SendObject(client, gserver->CreateError("alliance.canceladdUsertoAlliance", -99, "Not a member of an alliance."));
					return;
				}

				MULTILOCK(M_ALLIANCELIST, M_CLIENTLIST);
				string username = data["userName"];

				data2["ok"] = 1;

				Alliance * alliance = client->GetAlliance();

				alliance->UnRequestJoin(username);

				UNLOCK(M_ALLIANCELIST);
				UNLOCK(M_CLIENTLIST);

				gserver->SendObject(client, obj2);

				alliance->SaveToDB();

				return;
			}
			if ((command == "cancelagreeComeinAlliance"))
			{
				obj2["cmd"] = "alliance.cancelagreeComeinAlliance";
				data2["packageId"] = 0.0f;

				if (!client->HasAlliance())
				{
					gserver->SendObject(client, gserver->CreateError("alliance.cancelagreeComeinAlliance", -99, "Not a member of an alliance."));
					return;
				}

				MULTILOCK(M_ALLIANCELIST, M_CLIENTLIST);
				string username = data["userName"];

				data2["ok"] = 1;

				Alliance * alliance = client->GetAlliance();

				alliance->UnRequestJoin(username);

				UNLOCK(M_ALLIANCELIST);
				UNLOCK(M_CLIENTLIST);

				gserver->SendObject(client, obj2);

				alliance->SaveToDB();

				return;
			}
			if ((command == "setAllianceFriendship"))
			{
				//TODO: permission check
				obj2["cmd"] = "alliance.setAllianceFriendship";
				data2["packageId"] = 0.0f;

				if (!client->HasAlliance())
				{
					gserver->SendObject(client, gserver->CreateError("alliance.setAllianceFriendship", -99, "Not a member of an alliance."));
					return;
				}

				//1 friendly
				//2 neutral
				//3 enemy

				int alliancetype = data["type"];
				string otheralliancename = data["targetAllianceName"];

				Alliance * otheralliance = gserver->m_alliances->AllianceByName(otheralliancename);
				if (otheralliance == (Alliance*)-1)
				{
					//doesn't exist
					gserver->SendObject(client, gserver->CreateError("alliance.setAllianceFriendship", -99, "Alliance does not exist."));
					return;
				}

				Alliance * alliance = client->GetAlliance();

				if (alliancetype == 1)
				{
					if (alliance->IsAlly(otheralliance->m_allianceid))
					{
						//already allied
						gserver->SendObject(client, gserver->CreateError("alliance.setAllianceFriendship", -99, "Alliance is already an ally."));
						return;
					}
					alliance->Ally(otheralliance->m_allianceid);
				}
				else if (alliancetype == 2)
				{
					if (alliance->IsNeutral(otheralliance->m_allianceid))
					{
						//already neutral
						gserver->SendObject(client, gserver->CreateError("alliance.setAllianceFriendship", -99, "Alliance is already neutral."));
						return;
					}
					alliance->Neutral(otheralliance->m_allianceid);
				}
				else //alliancetype = 3
				{
					if (alliance->IsEnemy(otheralliance->m_allianceid))
					{
						//already enemy
						gserver->SendObject(client, gserver->CreateError("alliance.setAllianceFriendship", -99, "Alliance is already an enemy."));
						return;
					}
					// TODO copy declare war cooldown?
					if (unixtime() < alliance->enemyactioncooldown)
					{
						//Declared war too soon
						gserver->SendObject(client, gserver->CreateError("alliance.setAllianceFriendship", -99, "You have already declared war recently."));
						return;
					}

					alliance->Enemy(otheralliance->m_allianceid);
				}
				data2["ok"] = 1;
				gserver->SendObject(client, obj2);

				alliance->SaveToDB();
				return;
			}
			if ((command == "getAllianceFriendshipList"))
			{
				// TODO War reports -- alliance.getMilitarySituationList
				obj2["cmd"] = "alliance.getAllianceFriendshipList";
				data2["packageId"] = 0.0f;

				if (!client->HasAlliance())
				{
					gserver->SendObject(client, gserver->CreateError("alliance.getAllianceFriendshipList", -99, "Not a member of an alliance."));
					return;
				}

				data2["ok"] = 1;

				LOCK(M_ALLIANCELIST);
				Alliance * tempalliance = (Alliance*)-1;
				Alliance * alliance = client->GetAlliance();

				amf3array middleList = amf3array();
				amf3array enemyList = amf3array();
				amf3array friendlyList = amf3array();

				std::vector<int64_t>::iterator iter;
				if (alliance->m_neutral.size() > 0)
					for (iter = alliance->m_neutral.begin(); iter != alliance->m_neutral.end(); ++iter)
					{
					tempalliance = client->m_main->m_alliances->AllianceById(*iter);
					if (tempalliance != (Alliance*)-1)
					{
						amf3object ta = amf3object();
						ta["rank"] = tempalliance->m_prestigerank;
						ta["honor"] = tempalliance->m_honor;// HACK might be 0 -- alliance.isHasAlliance
						ta["allianceName"] = tempalliance->m_name.c_str();
						ta["memberCount"] = tempalliance->m_currentmembers;
						ta["aPrestigeCount"] = tempalliance->m_prestige;// HACK might be 0 -- alliance.isHasAlliance
						middleList.Add(ta);
					}
					}
				if (alliance->m_enemies.size() > 0)
					for (iter = alliance->m_enemies.begin(); iter != alliance->m_enemies.end(); ++iter)
					{
					tempalliance = client->m_main->m_alliances->AllianceById(*iter);
					if (tempalliance != (Alliance*)-1)
					{
						amf3object ta = amf3object();
						ta["rank"] = tempalliance->m_prestigerank;
						ta["honor"] = tempalliance->m_honor;// HACK might be 0 -- alliance.isHasAlliance
						ta["allianceName"] = tempalliance->m_name.c_str();
						ta["memberCount"] = tempalliance->m_currentmembers;
						ta["aPrestigeCount"] = tempalliance->m_prestige;// HACK might be 0 -- alliance.isHasAlliance
						enemyList.Add(ta);
					}
					}
				if (alliance->m_allies.size() > 0)
					for (iter = alliance->m_allies.begin(); iter != alliance->m_allies.end(); ++iter)
					{
					tempalliance = client->m_main->m_alliances->AllianceById(*iter);
					if (tempalliance != (Alliance*)-1)
					{
						amf3object ta = amf3object();
						ta["rank"] = tempalliance->m_prestigerank;
						ta["honor"] = tempalliance->m_honor;// HACK might be 0 -- alliance.isHasAlliance
						ta["allianceName"] = tempalliance->m_name.c_str();
						ta["memberCount"] = tempalliance->m_currentmembers;
						ta["aPrestigeCount"] = tempalliance->m_prestige;// HACK might be 0 -- alliance.isHasAlliance
						friendlyList.Add(ta);
					}
					}
				data2["middleList"] = middleList;
				data2["enemyList"] = enemyList;
				data2["friendlyList"] = friendlyList;

				UNLOCK(M_ALLIANCELIST);
				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "createAlliance"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				obj2["cmd"] = "alliance.createAlliance";
				data2["packageId"] = 0.0f;

				string alliancename = data["allianceName"];

				if (pcity->m_resources.gold < 10000)
				{
					gserver->SendObject(client, gserver->CreateError("alliance.createAlliance", -99, "Not enough gold."));
					return;
				}

				if (!gserver->m_alliances->CheckName(alliancename) || alliancename.size() < 2)
				{
					gserver->SendObject(client, gserver->CreateError("alliance.createAlliance", -99, "Illegal naming, please choose another name."));
					return;
				}

				LOCK(M_ALLIANCELIST);
				if (gserver->m_alliances->AllianceByName(alliancename) != (Alliance*)-1)
				{
					string error = "Alliance already existed: ";
					error += alliancename;
					error += ".";

					gserver->SendObject(client, gserver->CreateError("alliance.createAlliance", -12, error));
					UNLOCK(M_ALLIANCELIST);
					return;
				}
				Alliance * alliance = 0;
				if (alliance = gserver->m_alliances->CreateAlliance(alliancename, client->m_accountid))
				{
					data2["ok"] = 1;

					string error = "Establish alliance ";
					error += alliancename;
					error += " successfully.";
					data2["msg"] = error;

					alliance->m_founder = client->m_playername;

					pcity->m_resources.gold -= 10000;

					if (!gserver->m_alliances->JoinAlliance(alliance->m_allianceid, client))
					{
						client->PlayerUpdate();
						pcity->ResourceUpdate();

						gserver->SendObject(client, gserver->CreateError("alliance.createAlliance", -99, "Alliance created but cannot join. Please contact support."));
						UNLOCK(M_ALLIANCELIST);
						return;
					}
					if (!gserver->m_alliances->SetRank(alliance->m_allianceid, client, DEF_ALLIANCEHOST))
					{
						client->PlayerUpdate();
						pcity->ResourceUpdate();

						gserver->SendObject(client, gserver->CreateError("alliance.createAlliance", -99, "Alliance created but cannot set rank to host. Please contact support."));
						UNLOCK(M_ALLIANCELIST);
						return;
					}

					client->PlayerUpdate();
					pcity->ResourceUpdate();



					gserver->m_alliances->SortAlliances();

					gserver->SendObject(client, obj2);
					UNLOCK(M_ALLIANCELIST);
					client->SaveToDB();
					alliance->SaveToDB();
					return;
				}
				else
				{
					gserver->SendObject(client, gserver->CreateError("alliance.createAlliance", -99, "Cannot create alliance. Please contact support."));
					UNLOCK(M_ALLIANCELIST);
					return;
				}
			}
			if ((command == "setAllInfoForAlliance"))
			{
				//TODO: permission check
				string notetext = data["noteText"];
				string infotext = data["infoText"];
				string alliancename = data["allianceName"];

				obj2["cmd"] = "alliance.setAllInfoForAlliance";
				data2["packageId"] = 0.0f;

				if (!client->HasAlliance())
				{
					gserver->SendObject(client, gserver->CreateError("alliance.setAllInfoForAlliance", -99, "Not a member of an alliance."));
					return;
				}
				LOCK(M_ALLIANCELIST);
				Alliance * alliance = client->GetAlliance();
				if (alliance->m_name != alliancename)
				{
					gserver->SendObject(client, gserver->CreateError("alliance.setAllInfoForAlliance", -99, "Error."));
					return;
				}

				alliance->m_note = makesafe(notetext);
				alliance->m_intro = makesafe(infotext);

				UNLOCK(M_ALLIANCELIST);

				data2["ok"] = 1;

				gserver->SendObject(client, obj2);

				alliance->SaveToDB();

				return;
			}
			if (command == "getPowerFromAlliance")
			{
				obj2["cmd"] = "alliance.getPowerFromAlliance";
				data2["packageId"] = 0.0f;

				if (!client->HasAlliance())
				{
					gserver->SendObject(client, gserver->CreateError("alliance.getPowerFromAlliance", -99, "Not a member of an alliance."));
					return;
				}
				data2["ok"] = 1;
				data2["level"] = client->m_alliancerank;

				gserver->SendObject(client, obj2);
				return;
			}
			if (command == "resetTopPowerForAlliance")
			{
				obj2["cmd"] = "alliance.resetTopPowerForAlliance";
				data2["packageId"] = 0.0f;

				if (!client->HasAlliance())
				{
					gserver->SendObject(client, gserver->CreateError("alliance.resetTopPowerForAlliance", -99, "Not a member of an alliance."));
					return;
				}

				if (client->m_alliancerank != DEF_ALLIANCEHOST)
				{
					//you're not the host.. what are you doing?
					gserver->SendObject(client, gserver->CreateError("alliance.resetTopPowerForAlliance", -42, "You are not entitled to operate."));
					return;
				}

				string passtoname = data["userName"];

				Alliance * alliance = client->GetAlliance();
				if (alliance->HasMember(passtoname))
				{
					//member found
					Client * tclient = gserver->GetClientByName(passtoname);
					if (tclient->m_alliancerank != DEF_ALLIANCEVICEHOST)
					{
						gserver->SendObject(client, gserver->CreateError("alliance.resetTopPowerForAlliance", -88, "The Host title of the Alliance can only be transferred to Vice Host. You need to promote this player first."));
						return;
					}
					//everything checks out. target is vice host and you are host
					tclient->m_alliancerank = DEF_ALLIANCEHOST;
					client->m_alliancerank = DEF_ALLIANCEVICEHOST;
					client->PlayerUpdate();
					tclient->PlayerUpdate();
					alliance->m_owner = tclient->m_playername;

					data2["ok"] = 1;

					gserver->SendObject(client, obj2);

					alliance->SaveToDB();
					client->SaveToDB();

					return;
				}
				else
				{
					gserver->SendObject(client, gserver->CreateError("alliance.resetTopPowerForAlliance", -41, "Player " + passtoname + " doesn't exist."));
					return;
				}
			}
			if ((command == "agreeComeinAllianceByUser"))//TODO: alliance invites player, player accepts via embassy
			{
				gserver->SendObject(client, gserver->CreateError("alliance.agreeComeinAllianceByUser", -99, "Not a member of an alliance."));

				//			data["castleId"];
				//			data["allianceName"];

				gserver->SendObject(client, obj2);

				//alliance->SaveToDB();
				//client->SaveToDB();

				return;
			}
			if ((command == "agreeComeinAllianceByLeader"))//player applies to alliance, leader accepts
			{
				//TODO: permission check?
				obj2["cmd"] = "alliance.agreeComeinAllianceByLeader";
				data2["packageId"] = 0.0f;

				if (!client->HasAlliance())
				{
					gserver->SendObject(client, gserver->CreateError("alliance.agreeComeinAllianceByLeader", -99, "Not a member of an alliance."));
					return;
				}

				MULTILOCK(M_ALLIANCELIST, M_CLIENTLIST);
				string username = data["userName"];
				Client * invitee = gserver->GetClientByName(username);

				if (invitee == 0)
				{
					gserver->SendObject(client, gserver->CreateError("alliance.agreeComeinAllianceByLeader", -41, "Player " + username + " doesn't exist."));
					UNLOCK(M_ALLIANCELIST);
					UNLOCK(M_CLIENTLIST);
					return;
				}
				if (invitee->HasAlliance())
				{
					gserver->SendObject(client, gserver->CreateError("alliance.agreeComeinAllianceByLeader", -99, username + " is already a member of other alliance."));
					UNLOCK(M_ALLIANCELIST);
					UNLOCK(M_CLIENTLIST);
					return;
				}
				data2["ok"] = 1;

				Alliance * alliance = client->GetAlliance();

				alliance->UnRequestJoin(username);
				gserver->m_alliances->JoinAlliance(alliance->m_allianceid, invitee);

				UNLOCK(M_ALLIANCELIST);
				UNLOCK(M_CLIENTLIST);

				gserver->SendObject(client, obj2);

				alliance->SaveToDB();
				invitee->SaveToDB();

				return;
			}
			if ((command == "setPowerForUserByAlliance"))
			{
				obj2["cmd"] = "alliance.setPowerForUserByAlliance";
				data2["packageId"] = 0.0f;
				data2["ok"] = 1;

				int8_t type = data["typeId"];
				string username = data["userName"];

				if (!client->HasAlliance())
				{
					gserver->SendObject(client, gserver->CreateError("alliance.setPowerForUserByAlliance", -99, "Not a member of an alliance."));
					return;
				}

				Alliance * alliance = client->GetAlliance();

				MULTILOCK(M_ALLIANCELIST, M_CLIENTLIST);
				Client * tar = gserver->GetClientByName(username);

				if (!tar || !alliance->HasMember(username))
				{
					gserver->SendObject(client, gserver->CreateError("alliance.setPowerForUserByAlliance", -99, "Member does not exist."));
					UNLOCK(M_ALLIANCELIST);
					UNLOCK(M_CLIENTLIST);
					return;
				}
				//TODO: Set limits to rank counts? (aka, only x amount of vice hosts, etc)
				gserver->m_alliances->SetRank(client->m_allianceid, tar, type);

				UNLOCK(M_ALLIANCELIST);
				UNLOCK(M_CLIENTLIST);

				gserver->SendObject(client, obj2);

				//send alliance message
				string msg = client->m_playername + " promotes " + tar->m_playername + " to " + AllianceCore::GetAllianceRank(type) + ".";
				alliance->SendAllianceMessage(msg, false, false);

				alliance->SaveToDB();
				client->SaveToDB();

				return;
			}
			if ((command == "kickOutMemberfromAlliance"))
			{
				//TODO: permission check
				obj2["cmd"] = "alliance.kickOutMemberfromAlliance";
				data2["packageId"] = 0.0f;

				if (!client->HasAlliance())
				{
					gserver->SendObject(client, gserver->CreateError("alliance.kickOutMemberfromAlliance", -99, "Not a member of an alliance."));
					return;
				}

				string username = data["userName"];

				Alliance * alliance = client->GetAlliance();

				MULTILOCK(M_ALLIANCELIST, M_CLIENTLIST);
				Client * tar = gserver->GetClientByName(username);

				if (!tar || !alliance->HasMember(username))
				{
					gserver->SendObject(client, gserver->CreateError("alliance.kickOutMemberfromAlliance", -99, "Member does not exist."));
					UNLOCK(M_ALLIANCELIST);
					UNLOCK(M_CLIENTLIST);
					return;
				}

				if (username == client->m_playername)
				{
					gserver->SendObject(client, gserver->CreateError("alliance.kickOutMemberfromAlliance", -99, "Cannot kick yourself."));
					UNLOCK(M_ALLIANCELIST);
					UNLOCK(M_CLIENTLIST);
					return;
				}

				if (client->m_alliancerank >= tar->m_alliancerank)
				{
					gserver->SendObject(client, gserver->CreateError("alliance.kickOutMemberfromAlliance", -99, "Cannot kick someone higher ranking than you."));
					UNLOCK(M_ALLIANCELIST);
					UNLOCK(M_CLIENTLIST);
					return;
				}

				if (!gserver->m_alliances->RemoveFromAlliance(client->m_allianceid, tar))
				{
					gserver->SendObject(client, gserver->CreateError("alliance.kickOutMemberfromAlliance", -99, "Could not kick out member."));
					UNLOCK(M_ALLIANCELIST);
					UNLOCK(M_CLIENTLIST);
					return;
				}

				UNLOCK(M_ALLIANCELIST);
				UNLOCK(M_CLIENTLIST);

				gserver->SendObject(client, obj2);

				alliance->SaveToDB();
				tar->SaveToDB();

				return;
			}
		}
#pragma endregion
#pragma region tech
		if ((cmdtype == "tech"))
		{
			if ((command == "getResearchList"))
			{
				VERIFYCASTLEID();

				uint32_t castleid = data["castleId"];

				obj2["cmd"] = "tech.getResearchList";
				data2["ok"] = 1;
				data2["packageId"] = 0.0f;

				amf3array researchbeans = amf3array();

				for (int i = 0; i < 25; ++i)
				{
					if (gserver->m_researchconfig[i][0].time > 0)
					{
						int level = client->GetResearchLevel(i);


						amf3object parent;
						amf3object conditionbean;

						double costtime = gserver->m_researchconfig[i][level].time;
						double mayorinf = 1;
						if (pcity->m_mayor)
							mayorinf = pow(0.995, pcity->m_mayor->GetStratagem());

						costtime = (costtime)* (mayorinf);

						conditionbean["time"] = floor(costtime);
						conditionbean["destructTime"] = 0;
						conditionbean["wood"] = gserver->m_researchconfig[i][level].wood;
						conditionbean["food"] = gserver->m_researchconfig[i][level].food;
						conditionbean["iron"] = gserver->m_researchconfig[i][level].iron;
						conditionbean["gold"] = gserver->m_researchconfig[i][level].gold;
						conditionbean["stone"] = gserver->m_researchconfig[i][level].stone;

						amf3array buildings = amf3array();
						amf3array items = amf3array();
						amf3array techs = amf3array();

						for (stPrereq & req : gserver->m_researchconfig[i][0].buildings)
						{
							if (req.id > 0)
							{
								amf3object ta = amf3object();
								ta["level"] = req.level;
								int temp = pcity->GetBuildingLevel(req.id);
								ta["curLevel"] = temp;
								ta["successFlag"] = temp >= req.level ? true : false;
								ta["typeId"] = req.id;
								buildings.Add(ta);
							}
						}
						for (stPrereq & req : gserver->m_researchconfig[i][0].items)
						{
							if (req.id > 0)
							{
								amf3object ta = amf3object();
								int temp = client->m_items[req.id].count;
								ta["curNum"] = temp;
								ta["num"] = req.level;
								ta["successFlag"] = temp >= req.level ? true : false;
								ta["id"] = gserver->m_items[req.id].name;
								items.Add(ta);
							}
						}
						for (stPrereq & req : gserver->m_researchconfig[i][0].techs)
						{
							if (req.id > 0)
							{
								amf3object ta = amf3object();
								ta["level"] = req.level;
								int temp = client->GetResearchLevel(req.id);
								ta["curLevel"] = temp;
								ta["successFlag"] = temp >= req.level ? true : false;
								ta["typeId"] = req.id;
								buildings.Add(ta);
							}
						}

						conditionbean["buildings"] = buildings;
						conditionbean["items"] = items;
						conditionbean["techs"] = techs;
						conditionbean["population"] = gserver->m_researchconfig[i][level].population;
						parent["startTime"] = (double)client->m_research[i].starttime;
						parent["castleId"] = (double)client->m_research[i].castleid;
						//TODO: verify if works with multiple academies
						if ((client->m_research[i].endtime != 0) && (client->m_research[i].endtime < timestamp))
						{
							//if research was sped up and isn't processed through the timer thread yet, emulate it being completed
							int research = level + 1;
							parent["level"] = research;
							parent["upgradeing"] = false;
							parent["endTime"] = 0;
							int blevel = pcity->GetBuildingLevel(B_ACADEMY);//academy
							if (blevel != 0)
							{
								parent["avalevel"] = blevel <= research ? blevel : research;
							}
							parent["permition"] = true;
						}
						else
						{
							parent["level"] = level;
							parent["upgradeing"] = ((client->m_research[i].endtime != 0) && (client->m_research[i].endtime > timestamp));
							parent["endTime"] = (client->m_research[i].endtime > 0) ? (client->m_research[i].endtime - client->m_lag) : (client->m_research[i].endtime);// HACK: attempt to fix "lag" issues
							parent["avalevel"] = pcity->GetTechLevel(i);
							parent["permition"] = !pcity->m_researching;
						}
						parent["conditionBean"] = conditionbean;
						parent["typeId"] = i;
						researchbeans.Add(parent);
					}
				}


				data2["acailableResearchBeans"] = researchbeans;
				data2["ok"] = 1;
				data2["packageId"] = 0.0f;


				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "research"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				uint32_t castleid = data["castleId"];
				int techid = data["techId"];

				if (techid < 0 || techid > 25 || client->m_research[techid].level >= 10 || gserver->m_researchconfig[techid][client->m_research[techid].level].time == 0)
				{
					gserver->SendObject(client, gserver->CreateError("tech.research", -99, "Invalid technology."));
					return;
				}

				stResearch * research;
				stBuildingConfig * researchconfig;

				research = &client->m_research[techid];
				researchconfig = &gserver->m_researchconfig[techid][research->level];


				if (!pcity->m_researching)
				{
					if ((researchconfig->food > pcity->m_resources.food)
						|| (researchconfig->wood > pcity->m_resources.wood)
						|| (researchconfig->stone > pcity->m_resources.stone)
						|| (researchconfig->iron > pcity->m_resources.iron)
						|| (researchconfig->gold > pcity->m_resources.gold))
					{
						gserver->SendObject(client, gserver->CreateError("tech.research", -99, "Not enough resources."));
						return;
					}
					obj2["cmd"] = "tech.research";
					data2["ok"] = 1;
					data2["packageId"] = 0.0f;

					pcity->m_resources.food -= researchconfig->food;
					pcity->m_resources.wood -= researchconfig->wood;
					pcity->m_resources.stone -= researchconfig->stone;
					pcity->m_resources.iron -= researchconfig->iron;
					pcity->m_resources.gold -= researchconfig->gold;

					research->castleid = castleid;


					double costtime = researchconfig->time;
					double mayorinf = 1;
					if (pcity->m_mayor)
						mayorinf = pow(0.995, pcity->m_mayor->GetStratagem());

					costtime = (costtime)* (mayorinf);

					research->endtime = timestamp + floor(costtime) * 1000;

					research->starttime = timestamp;

					stResearchAction * ra = new stResearchAction;

					stTimedEvent te;
					ra->city = pcity;
					ra->client = client;
					ra->researchid = techid;
					te.data = ra;
					te.type = DEF_TIMEDRESEARCH;
					pcity->m_researching = true;

					gserver->AddTimedEvent(te);

					amf3object parent;
					amf3object conditionbean;

					conditionbean["time"] = floor(costtime);
					conditionbean["destructTime"] = 0.0f;
					conditionbean["wood"] = researchconfig->wood;
					conditionbean["food"] = researchconfig->food;
					conditionbean["iron"] = researchconfig->iron;
					conditionbean["gold"] = researchconfig->gold;
					conditionbean["stone"] = researchconfig->stone;

					amf3array buildings = amf3array();
					amf3array items = amf3array();
					amf3array techs = amf3array();


					for (stPrereq & req : researchconfig->buildings)
					{
						if (req.id > 0)
						{
							amf3object ta = amf3object();
							ta["level"] = req.level;
							int temp = ((PlayerCity*)client->m_city.at(client->m_currentcityindex))->GetBuildingLevel(req.id);
							ta["curLevel"] = temp;
							ta["successFlag"] = temp >= req.level ? true : false;
							ta["typeId"] = req.id;
							buildings.Add(ta);
						}
					}
					for (stPrereq & req : researchconfig->items)
					{
						if (req.id > 0)
						{
							amf3object ta = amf3object();
							int temp = client->m_items[req.id].count;
							ta["curNum"] = temp;
							ta["num"] = req.level;
							ta["successFlag"] = temp >= req.level ? true : false;
							ta["id"] = gserver->m_items[req.id].name;
							items.Add(ta);
						}
					}
					for (stPrereq & req : researchconfig->techs)
					{
						if (req.id > 0)
						{
							amf3object ta = amf3object();
							ta["level"] = req.level;
							int temp = client->GetResearchLevel(req.id);
							ta["curLevel"] = temp;
							ta["successFlag"] = temp >= req.level ? true : false;
							ta["typeId"] = req.id;
							buildings.Add(ta);
						}
					}

					conditionbean["buildings"] = buildings;
					conditionbean["items"] = items;
					conditionbean["techs"] = techs;
					conditionbean["population"] = researchconfig->population;
					parent["startTime"] = (double)research->starttime;
					parent["castleId"] = (double)research->castleid;
					parent["level"] = client->GetResearchLevel(techid);
					parent["conditionBean"] = conditionbean;
					parent["avalevel"] = pcity->GetTechLevel(techid);
					parent["upgradeing"] = (bool)(research->starttime != 0);
					parent["endTime"] = (research->endtime > 0) ? (research->endtime - client->m_lag) : (research->endtime);// HACK: attempt to fix "lag" issues
					parent["typeId"] = techid;
					parent["permition"] = !pcity->m_researching;

					data2["tech"] = parent;

					gserver->SendObject(client, obj2);

					pcity->ResourceUpdate();

					client->SaveToDB();

					return;
				}
				else
				{
					gserver->SendObject(client, gserver->CreateError("tech.research", -99, "Research already in progress."));
					return;
				}
			}
			if ((command == "cancelResearch"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				uint32_t castleid = data["castleId"];
				PlayerCity * pcity = client->GetCity(castleid);
				uint16_t techid = 0;

				for (int i = 0; i < 25; ++i)
				{
					if (client->m_research[i].castleid == castleid)
					{
						techid = i;
						break;
					}
				}

				if (!pcity || !pcity->m_researching || techid == 0)
				{
					gserver->SendObject(client, gserver->CreateError("tech.cancelResearch", -99, "Invalid city."));
					return;
				}

				stResearch * research;
				stBuildingConfig * researchconfig;


				research = &client->m_research[techid];
				researchconfig = &gserver->m_researchconfig[techid][research->level];


				if (pcity->m_researching)
				{
					obj2["cmd"] = "tech.cancelResearch";
					data2["ok"] = 1;
					data2["packageId"] = 0.0f;

					pcity->m_resources.food += double(researchconfig->food) / 3;
					pcity->m_resources.wood += double(researchconfig->wood) / 3;
					pcity->m_resources.stone += double(researchconfig->stone) / 3;
					pcity->m_resources.iron += double(researchconfig->iron) / 3;
					pcity->m_resources.gold += double(researchconfig->gold) / 3;

					std::list<stTimedEvent>::iterator iter;


					LOCK(M_TIMEDLIST);
					for (iter = gserver->researchlist.begin(); iter != gserver->researchlist.end();)
					{
						stResearchAction * ra = (stResearchAction *)iter->data;
						PlayerCity * city = ra->city;
						if (city->m_castleid == castleid)
						{
							ra->researchid = 0;
							break;
						}
						++iter;
					}
					UNLOCK(M_TIMEDLIST);

					research->castleid = 0;
					research->endtime = 0.0f;
					research->starttime = 0.0f;

					pcity->m_researching = false;

					gserver->SendObject(client, obj2);

					pcity->ResourceUpdate();

					client->SaveToDB();

					return;
				}
			}
			if ((command == "speedUpResearch"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				string speeditemid;
				uint32_t castleid = data["castleId"];

				speeditemid = static_cast<string>(data["itemId"]);

				stResearch * research = 0;
				for (int i = 0; i < 25; ++i)
				{
					if (client->m_research[i].castleid == pcity->m_castleid)
					{
						research = &client->m_research[i];
						break;
					}
				}
				if (research == 0)
				{
					//TODO: get error message for city not having research -- tech.speedUpResearch
					gserver->SendObject(client, gserver->CreateError("tech.speedUpResearch", -99, "Invalid tech."));
					return;
				}

				if (speeditemid == "free.speed")
				{
					//check if under 5 mins

					if ((research->endtime - research->starttime) <= 5 * 60 * 1000)
					{
						//under 5 mins
						obj2["cmd"] = "tech.speedUpResearch";
						data2["packageId"] = 0.0f;
						data2["ok"] = 1;

						gserver->SendObject(client, obj2);

						research->endtime -= 5 * 60 * 1000;

						client->SaveToDB();
					}
					else
					{
						//over 5 mins
						//TODO: get 5 min speed up error - tech.speedUpResearch
						gserver->SendObject(client, gserver->CreateError("tech.speedUpResearch", -99, "Invalid speed up."));
						return;
					}
				}
				else
				{
					//is not under 5 mins, apply an item
					int itemcount = client->GetItemCount(speeditemid);

					int cents = 0;
					int reducetime = 0;

					obj2["cmd"] = "tech.speedUpResearch";
					data2["packageId"] = 0.0f;
					data2["ok"] = 1;

					gserver->SendObject(client, obj2);

					//TODO: reduce time on building being sped up -- tech.speedUpResearch

					if (speeditemid == "consume.2.a")
					{
						cents = 5;
						reducetime = 15 * 60 * 1000;
					}
					else if (speeditemid == "consume.2.b")
					{
						cents = 10;
						reducetime = 60 * 60 * 1000;
					}
					else if (speeditemid == "consume.2.b.1")
					{
						cents = 20;
						reducetime = 150 * 60 * 1000;
					}
					else if (speeditemid == "consume.2.c")
					{
						cents = 50;
						reducetime = 8 * 60 * 60 * 1000;
					}
					else if (speeditemid == "consume.2.c.1")
					{
						cents = 80;
						reducetime = ((rand() % 21) + 10) * 60 * 60 * 1000;
					}
					else if (speeditemid == "consume.2.d")
					{
						cents = 120;
						reducetime = ((research->endtime - research->starttime)*0.30);
					}
					else if (speeditemid == "coins.speed")
					{
						cents = 200;
						reducetime = (research->endtime - research->starttime);
					}

					if (itemcount <= 0)
					{
						if (client->m_cents < cents)
						{
							// not enough item and not enough cents
							//TODO: find error value -- tech.speedUpResearch
							gserver->SendObject(client, gserver->CreateError("tech.speedUpResearch", -99, "Not enough cents."));
							return;
						}
						//not enough item, but can buy with cents
						client->m_cents -= cents;
						client->PlayerUpdate();
					}
					else
					{ //has item
						client->SetItem((string)speeditemid, -1);
					}

					research->endtime -= reducetime;

					client->SaveToDB();

					return;
				}
				return;
			}
			if ((command == "getCoinsNeed"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				int positionid = data["positionId"];
				uint32_t castleid = data["castleId"];

				obj2["cmd"] = "castle.getCoinsNeed";
				data2["packageId"] = 0.0f;
				data2["ok"] = 1;
				data2["coinsNeed"] = 200;//TODO: calculate correct cents cost based on time

				gserver->SendObject(client, obj2);
				return;
			}
		}
#pragma endregion
#pragma region login
		if (cmdtype == "login")
		{
			//errors:
			//-5 = captcha
			//-99 = general error
			//-100 = holiday
			string username = data["user"];
			string password = data["pwd"];

			if (gserver->maxplayers <= gserver->currentplayersonline + 1)
			{
				gserver->SendObject(&c, gserver->CreateError("server.LoginResponse", -99, "Servers are currently overloaded. Please try again later."));
				return;
			}

			string newuser;
			string newpass;
			newuser = makesafe(username);
			newpass = makesafe(password);


			{
				Session ses(gserver->accountpool->get());
				Statement select(ses);
				select << "SELECT COUNT(*) AS a FROM `account` WHERE `email`=?;", use(newuser);
				select.execute();
				RecordSet rs(select);

				if (rs.value("a").convert<int32_t>() == 0)
				{
					//account does not exist - insert new row
					try
					{
						Statement insert(ses);
						insert << "INSERT INTO `account` (`name`, `email`, `ip`, `lastlogin`, `creation`, `password`, `status`, `reason`) VALUES ('null', ?, '', ?, ?, ?, 0, '');", use(newuser), use(unixtime()), use(unixtime()), use(newpass), now;
					}
					catch (Poco::Data::MySQL::StatementException * e)
					{
						gserver->consoleLogger->error(Poco::format("Account Create Exception: %s", e->displayText()));
						return;
					}
				}
			}

		{
			Session ses(gserver->accountpool->get());
			Statement select(ses);
			select << "SELECT * FROM `account` WHERE `email`=? AND `password`=?;", use(newuser), use(newpass);
			select.execute();
			RecordSet rs(select);

			if (rs.rowCount() == 0)
			{
				//account doesn't exist or password is wrong
				gserver->SendObject(&c, gserver->CreateError("server.LoginResponse", -2, "Incorrect account or password."));
				return;
			}
			else
			{
				int32_t masteraccountid = rs.value("id").convert<int32_t>();
				client = gserver->GetClientByParent(masteraccountid);

				bool banned = false;

				{
					//are they banned? if so, globally or for this server?
					Session ses2(gserver->serverpool->get());
					Statement select2(ses2);
					select2 << "SELECT * FROM `accounts` WHERE `parentid`=?;", use(masteraccountid);
					select2.execute();
					RecordSet rs2(select2);

					if (rs.value("status").convert<int32_t>() == -99)
						banned = true;

					if (rs2.rowCount() > 0 && rs2.value("status").convert<int32_t>() == -99)
						banned = true;

					if (banned)
					{
						string errormsg = "You are banned. Reason: ";
						errormsg += rs.value("reason").convert<string>().length()>0 ? rs.value("reason").convert<string>() : rs2.value("reason").convert<string>();

						gserver->SendObject(&c, gserver->CreateError("server.LoginResponse", -99, errormsg));

						return;
					}
				}

				//client = gserver->GetClientByParent(parentid);
				if (client == 0)
				{
					client = gserver->NewClient();
					client->masteraccountid = masteraccountid;
					client->m_socknum = req.conn->uid;
					client->socket = req.conn;
					req.conn->client_ = client;
					client->m_connected = true;
				}
				else
				{
					if (client->m_connected)
					{
						//player already logged on
						gserver->CloseClient(client, 3, "");//multiple people logging into the same account
					}
					//Login is valid
					client->m_connected = true;
					double logintime = unixtime();
					if (logintime - client->m_lastlogin < 1000 * 5)
					{
						gserver->SendObject(req.conn, gserver->CreateError("server.LoginResponse", 6, "You have tried logging in too frequently. Please try again later."));
						req.conn->stop();
						return;
					}
					client->m_lastlogin = logintime;
					if (client->socket) gserver->CloseClient(client, 3, "");
					client->socket = req.conn;
					client->m_socknum = req.conn->uid;
					client->m_ipaddress = req.conn->address;
					req.conn->client_ = client;
					gserver->consoleLogger->information(Poco::format("Already established client found # %?d", (uint32_t)client->m_clientnumber));

					if (client->m_email == "Daisy")
					{
						client->m_bdenyotherplayer = true;
						client->m_icon = 7;
					}
				}

				if (client == 0)
				{
					//error creating client object
					gserver->consoleLogger->information(Poco::format("Error creating client object @ %s:%?d", (string)__FILE__, __LINE__));
					gserver->SendObject(client, gserver->CreateError("server.LoginResponse", -99, "Error with connecting. Please contact support."));
					return;
				}


				//account exists
				Session ses2(gserver->serverpool->get());
				Statement select2(ses2);
				select2 << "SELECT * FROM `accounts` WHERE `parentid`=?;", use(masteraccountid);
				select2.execute();
				RecordSet rs2(select2);

				if (rs2.rowCount() == 0)
				{
					//does not have an account on server
					gserver->SendObject(client, gserver->CreateError("server.LoginResponse", -4, "need create player"));
					client->m_loggedin = true;

					return;
				}
				else
				{
					int accountid = rs2.value("accountid").convert<int32_t>();
					client->m_accountid = accountid;

					//has an account, what about cities?
					Session ses3(gserver->serverpool->get());
					Statement select3(ses3);
					select3 << "SELECT * FROM `cities` WHERE `accountid`=?;", use(accountid);
					select3.execute();
					RecordSet rs3(select3);

					if (rs3.rowCount() == 0)
					{
						//does not have any cities on server but did have an account - this only happens if you try to "restart" your account. it saves the account info while deleting your cities
						gserver->SendObject(client, gserver->CreateError("server.LoginResponse", -4, "need create player"));
						client->m_loggedin = true;
						return;
					}
					else
					{
						//has an account and cities. process the list and send account info

						amf3object obj;
						obj["cmd"] = "server.LoginResponse";
						obj["data"] = amf3object();
						amf3object & data = obj["data"];
						data["packageId"] = 0.0f;

						double tslag = unixtime();

						if (client->GetItemCount("consume.1.a") < 10000)
							client->SetItem("consume.1.a", 10000);
						client->m_cents = 5000;

						data["player"] = client->ToObject();
						//UNLOCK(M_CLIENTLIST);

						if (client->m_city.size() == 0)
						{
							//problem
							gserver->consoleLogger->error(Poco::format("Error client has no cities @ %s:%?d", (string)__FILE__, __LINE__));
							gserver->SendObject(client, gserver->CreateError("server.LoginResponse", -99, "Error with connecting. Please contact support."));
							return;
						}
						client->m_currentcityid = ((PlayerCity*)client->m_city.at(0))->m_castleid;
						client->m_currentcityindex = 0;
						client->m_accountexists = true;


						//check for holiday status
						stBuff * holiday = client->GetBuff("FurloughBuff");
						if (holiday && holiday->endtime > tslag)
						{
							//is in holiday - send holiday info too

							string s;
							{
								int32_t hours;
								int32_t mins;
								int32_t secs = (holiday->endtime - tslag) / 1000;

								hours = secs / 60 / 60;
								mins = secs / 60 - hours * 60;
								secs = secs - mins * 60 - hours * 60 * 60;

								std::stringstream ss;
								ss << hours << "," << mins << "," << secs;

								s = ss.str();
							}

							data["ok"] = -100;
							data["msg"] = s;
							data["errorMsg"] = s;
						}
						else
						{
							data["ok"] = 1;
							data["msg"] = "success";
						}

						gserver->SendObject(client, obj);
						//SendObject(*req.connection, obj);

						client->m_lag = unixtime() - tslag;

						client->m_loggedin = true;

						gserver->currentplayersonline++;
						client->SaveToDB();

						return;
					}
				}
			}
		}
			return;
		}
#pragma endregion
#pragma region rank
		if ((cmdtype == "rank"))
		{
			if ((command == "getPlayerRank"))
			{
				int pagesize = data["pageSize"];
				string key = data["key"];
				int sorttype = data["sortType"];
				int pageno = data["pageNo"];

				obj2["cmd"] = "rank.getPlayerRank";
				data2["packageId"] = 0.0f;
				data2["ok"] = 1;
				std::list<stClientRank> * ranklist;
				amf3array beans = amf3array();
				LOCK(M_RANKEDLIST);
				switch (sorttype)
				{
					case 1:
						ranklist = &gserver->m_titlerank;
						break;
					case 2:
						ranklist = &gserver->m_prestigerank;
						break;
					case 3:
						ranklist = &gserver->m_honorrank;
						break;
					case 4:
						ranklist = &gserver->m_citiesrank;
						break;
					case 5:
						ranklist = &gserver->m_populationrank;
						break;
					default:
						ranklist = &gserver->m_prestigerank;
						break;
				}
				std::list<stClientRank>::iterator iter;
				if (pagesize <= 0 || pagesize > 20 || pageno < 0 || pageno > 100000)
				{
					gserver->SendObject(client, gserver->CreateError("rank.getPlayerRank", -99, "Invalid data."));
					return;
				}

				if (key.length() > 0)
				{
					//search term given
					ranklist = (std::list<stClientRank>*)gserver->DoRankSearch(key, 1, ranklist, pageno, pagesize);//1 = client
				}

				if ((pageno - 1)*pagesize > ranklist->size())
				{
					gserver->SendObject(client, gserver->CreateError("rank.getPlayerRank", -99, "Invalid page."));
					UNLOCK(M_RANKEDLIST);
					return;
				}
				iter = ranklist->begin();
				for (int i = 0; i < (pageno - 1)*pagesize; ++i)
					iter++;

				int rank = (pageno - 1)*pagesize + 1;
				data2["pageNo"] = pageno;
				data2["pageSize"] = pagesize;
				if ((ranklist->size() % pagesize) == 0)
					data2["totalPage"] = (ranklist->size() / pagesize);
				else
					data2["totalPage"] = (ranklist->size() / pagesize) + 1;
				for (; iter != ranklist->end() && pagesize != 0; ++iter)
				{
					pagesize--;
					amf3object temp = amf3object();
					temp["createrTime"] = 0;
					if (iter->client->m_allianceid > 0)
					{
						temp["alliance"] = iter->client->GetAlliance()->m_name;
						temp["allianceLevel"] = AllianceCore::GetAllianceRank(iter->client->m_alliancerank);
						temp["levelId"] = iter->client->m_alliancerank;
					}
					temp["office"] = iter->client->m_office;
					temp["sex"] = iter->client->m_sex;
					temp["honor"] = iter->client->m_honor;
					temp["bdenyotherplayer"] = iter->client->m_bdenyotherplayer;
					temp["id"] = iter->client->m_accountid;
					temp["accountName"] = "";
					temp["prestige"] = iter->client->m_prestige;
					temp["faceUrl"] = iter->client->m_faceurl;
					temp["flag"] = iter->client->m_flag;
					temp["userId"] = iter->client->masteraccountid;
					temp["userName"] = iter->client->m_playername;
					temp["castleCount"] = iter->client->m_citycount;
					temp["titleId"] = iter->client->m_title;
					temp["medal"] = 0;
					temp["ranking"] = iter->rank;
					temp["lastLoginTime"] = 0;
					temp["population"] = iter->client->m_population;
					beans.Add(temp);
				}
				UNLOCK(M_RANKEDLIST);

				data2["beans"] = beans;
				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "getAllianceRank"))
			{
				int pagesize = data["pageSize"];
				string key = data["key"];
				int sorttype = data["sortType"];
				int pageno = data["pageNo"];

				obj2["cmd"] = "rank.getAllianceRank";
				data2["packageId"] = 0.0f;
				data2["ok"] = 1;
				std::list<stAlliance> * ranklist;
				amf3array beans = amf3array();
				LOCK(M_RANKEDLIST);
				switch (sorttype)
				{
					case 1:
						ranklist = &gserver->m_alliances->m_membersrank;
						break;
					case 2:
						ranklist = &gserver->m_alliances->m_prestigerank;
						break;
					case 3:
						ranklist = &gserver->m_alliances->m_honorrank;
						break;
					default:
						ranklist = &gserver->m_alliances->m_membersrank;
						break;
				}
				std::list<stAlliance>::iterator iter;
				if (pagesize <= 0 || pagesize > 20 || pageno < 0 || pageno > 100000)
				{
					gserver->SendObject(client, gserver->CreateError("rank.getAllianceRank", -99, "Invalid data."));
					UNLOCK(M_RANKEDLIST);
					return;
				}

				if (key.length() > 0)
				{
					//search term given
					ranklist = (std::list<stAlliance>*)gserver->DoRankSearch(key, 4, ranklist, pageno, pagesize);//1 = client
				}

				if ((pageno - 1)*pagesize > ranklist->size())
				{
					gserver->SendObject(client, gserver->CreateError("rank.getAllianceRank", -99, "Invalid page."));
					UNLOCK(M_RANKEDLIST);
					return;
				}
				iter = ranklist->begin();
				for (int i = 0; i < (pageno - 1)*pagesize; ++i)
					iter++;

				int rank = (pageno - 1)*pagesize + 1;
				data2["pageNo"] = pageno;
				data2["pageSize"] = pagesize;
				if ((ranklist->size() % pagesize) == 0)
					data2["totalPage"] = (ranklist->size() / pagesize);
				else
					data2["totalPage"] = (ranklist->size() / pagesize) + 1;
				for (; iter != ranklist->end() && pagesize != 0; ++iter)
				{
					pagesize--;
					amf3object temp = amf3object();
					temp["member"] = iter->ref->m_currentmembers;
					temp["prestige"] = iter->ref->m_prestige;
					temp["rank"] = iter->rank;
					temp["playerName"] = iter->ref->m_owner;
					temp["honor"] = iter->ref->m_honor;
					temp["description"] = iter->ref->m_intro;
					temp["createrName"] = iter->ref->m_founder;
					temp["name"] = iter->ref->m_name;
					temp["city"] = iter->ref->m_citycount;
					beans.Add(temp);
				}
				UNLOCK(M_RANKEDLIST);

				data2["beans"] = beans;
				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "getHeroRank"))
			{
				int pagesize = data["pageSize"];
				string key = data["key"];
				int sorttype = data["sortType"];
				int pageno = data["pageNo"];

				obj2["cmd"] = "rank.getHeroRank";
				data2["packageId"] = 0.0f;
				data2["ok"] = 1;
				std::list<stHeroRank> * ranklist;
				amf3array beans = amf3array();
				LOCK(M_RANKEDLIST);
				switch (sorttype)
				{
					case 1:
						ranklist = &gserver->m_herorankgrade;
						break;
					case 2:
						ranklist = &gserver->m_herorankmanagement;
						break;
					case 3:
						ranklist = &gserver->m_herorankpower;
						break;
					case 4:
						ranklist = &gserver->m_herorankstratagem;
						break;
					default:
						ranklist = &gserver->m_herorankgrade;
						break;
				}
				std::list<stHeroRank>::iterator iter;
				if (pagesize <= 0 || pagesize > 20 || pageno < 0 || pageno > 100000)
				{
					gserver->SendObject(client, gserver->CreateError("rank.getHeroRank", -99, "Invalid data."));
					UNLOCK(M_RANKEDLIST);
					return;
				}

				if (key.length() > 0)
				{
					//search term given
					ranklist = (std::list<stHeroRank>*)gserver->DoRankSearch(key, 2, ranklist, pageno, pagesize);//1 = client
				}

				if ((pageno - 1)*pagesize > ranklist->size())
				{
					gserver->SendObject(client, gserver->CreateError("rank.getHeroRank", -99, "Invalid page."));
					UNLOCK(M_RANKEDLIST);
					return;
				}
				iter = ranklist->begin();
				for (int i = 0; i < (pageno - 1)*pagesize; ++i)
					iter++;

				int rank = (pageno - 1)*pagesize + 1;
				data2["pageNo"] = pageno;
				data2["pageSize"] = pagesize;
				if ((ranklist->size() % pagesize) == 0)
					data2["totalPage"] = (ranklist->size() / pagesize);
				else
					data2["totalPage"] = (ranklist->size() / pagesize) + 1;
				for (; iter != ranklist->end() && pagesize != 0; ++iter)
				{
					pagesize--;
					amf3object temp = amf3object();
					temp["rank"] = iter->rank;
					temp["stratagem"] = iter->stratagem;
					temp["name"] = iter->name;
					temp["power"] = iter->power;
					temp["grade"] = iter->grade;
					temp["management"] = iter->management;
					temp["kind"] = iter->kind;
					beans.Add(temp);
				}
				UNLOCK(M_RANKEDLIST);

				data2["beans"] = beans;
				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "getCastleRank"))
			{
				int pagesize = data["pageSize"];
				string key = data["key"];
				int sorttype = data["sortType"];
				int pageno = data["pageNo"];

				obj2["cmd"] = "rank.getCastleRank";
				data2["packageId"] = 0.0f;
				data2["ok"] = 1;
				std::list<stCastleRank> * ranklist;
				amf3array beans = amf3array();
				LOCK(M_RANKEDLIST);
				switch (sorttype)
				{
					case 1:
						ranklist = &gserver->m_castlerankpopulation;
						break;
					case 2:
						ranklist = &gserver->m_castleranklevel;
						break;
					default:
						ranklist = &gserver->m_castlerankpopulation;
						break;
				}
				std::list<stCastleRank>::iterator iter;
				if (pagesize <= 0 || pagesize > 20 || pageno < 0 || pageno > 100000)
				{
					gserver->SendObject(client, gserver->CreateError("rank.getCastleRank", -99, "Invalid data."));
					UNLOCK(M_RANKEDLIST);
					return;
				}

				if (key.length() > 0)
				{
					//search term given
					ranklist = (std::list<stCastleRank>*)gserver->DoRankSearch(key, 3, ranklist, pageno, pagesize);//1 = client
				}

				if ((pageno - 1)*pagesize > ranklist->size())
				{
					gserver->SendObject(client, gserver->CreateError("rank.getCastleRank", -99, "Invalid page."));
					UNLOCK(M_RANKEDLIST);
					return;
				}
				iter = ranklist->begin();
				for (int i = 0; i < (pageno - 1)*pagesize; ++i)
					iter++;

				int rank = (pageno - 1)*pagesize + 1;
				data2["pageNo"] = pageno;
				data2["pageSize"] = pagesize;
				if ((ranklist->size() % pagesize) == 0)
					data2["totalPage"] = (ranklist->size() / pagesize);
				else
					data2["totalPage"] = (ranklist->size() / pagesize) + 1;
				for (; iter != ranklist->end() && pagesize != 0; ++iter)
				{
					pagesize--;
					amf3object temp = amf3object();
					temp["alliance"] = iter->alliance;
					temp["rank"] = iter->rank;
					temp["level"] = iter->level;
					temp["name"] = iter->name;
					temp["grade"] = iter->grade;
					temp["kind"] = iter->kind;
					temp["population"] = iter->population;
					beans.Add(temp);
				}
				UNLOCK(M_RANKEDLIST);

				data2["beans"] = beans;
				gserver->SendObject(client, obj2);
				return;
			}
		}
#pragma endregion
#pragma region trade
		if ((cmdtype == "trade"))//TODO: finish market code
		{
			if ((command == "searchTrades"))
			{
				int restype = data["resType"];

				if (restype < 0 || restype > 3)
				{
					gserver->SendObject(client, gserver->CreateError("trade.searchTrades", -99, "Not a valid resource type."));
					return;
				}


				obj2["cmd"] = "trade.searchTrades";
				data2["packageId"] = 0.0f;
				data2["ok"] = 1;

				gserver->SendObject(client, obj2);
				return;
			}
		}
#pragma endregion
#pragma region shop
		if ((cmdtype == "shop"))
		{
			if ((command == "buy"))
			{
				int amount = data["amount"];
				string itemid = static_cast<string>(data["itemId"]);

				for (int i = 0; i < DEF_MAXITEMS; ++i)
				{
					if (itemid == gserver->m_items[i].name)
					{
						if (client->m_cents < gserver->m_items[i].cost*amount)
						{
							gserver->SendObject(client, gserver->CreateError("shop.buy", -28, "Insufficient game coins."));
							return;
						}
						client->m_cents -= gserver->m_items[i].cost*amount;

						client->SetItem(itemid, amount);

						client->PlayerUpdate();

						obj2["cmd"] = "shop.buy";
						data2["packageId"] = 0.0f;
						data2["ok"] = 1;

						gserver->SendObject(client, obj2);

						client->SaveToDB();

						return;
					}
				}
				gserver->SendObject(client, gserver->CreateError("shop.buy", -99, "Item does not exist."));
				return;
			}
			if ((command == "getBuyResourceInfo"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();


				int amount = data["amount"];
				//string itemid = static_cast<string>(data["itemId"]);

				obj2["cmd"] = "shop.getBuyResourceInfo";
				data2["packageId"] = 0.0f;
				data2["ok"] = 1;

				amf3object buyresource = amf3object();
				buyresource["woodRemain"] = 10000000;
				buyresource["forWood"] = 100000;
				buyresource["stoneRemain"] = 10000000;
				buyresource["forStone"] = 50000;
				buyresource["ironRemain"] = 10000000;
				buyresource["forIron"] = 40000;
				buyresource["foodRemain"] = 10000000;
				buyresource["forFood"] = 100000;

				data2["buyResourceBean"] = buyresource;

				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "buyResource"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				int fooduse = data["foodUse"];
				int wooduse = data["woodUse"];
				int stoneuse = data["stoneUse"];
				int ironuse = data["ironUse"];

				if (fooduse + wooduse + stoneuse + ironuse > client->m_cents)
				{
					gserver->SendObject(client, gserver->CreateError("shop.buyResource", -24, "Insufficient game coins."));
					return;
				}

				client->m_cents -= fooduse + wooduse + stoneuse + ironuse;
				pcity->m_resources.food += fooduse * 100000;
				pcity->m_resources.wood += wooduse * 100000;
				pcity->m_resources.stone += stoneuse * 50000;
				pcity->m_resources.iron += ironuse * 40000;

				obj2["cmd"] = "shop.buyResource";
				data2["packageId"] = 0.0f;
				data2["ok"] = 1;

				client->PlayerUpdate();
				pcity->ResourceUpdate();

				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "useGoods"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				uint32_t num = data["num"];
				string itemid = static_cast<string>(data["itemId"]);

				if (client->GetItemCount(itemid) < num)
				{
					gserver->SendObject(client, gserver->CreateError("shop.useGoods", -24, "Insufficient items."));
					return;
				}

				//other errors?
				//timed items
				//{
				//	gserver->SendObject(client, gserver->CreateError("shop.useGoods", -132, "You can only use this item between Wed Feb 06 00:00:00 CST 2013 and Thu Feb 21 00:00:00 CST 2013"));
				//}
				ShopUseGoods(data, client);

				client->SaveToDB();

				return;
			}
		}
#pragma endregion
#pragma region mail
		if (cmdtype == "mail")//TODO: mail code
		{
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
#pragma endregion
#pragma region fortifications
		if ((cmdtype == "fortifications"))
		{
			if ((command == "getProduceQueue"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				uint32_t castleid = data["castleId"];

				obj2["cmd"] = "fortifications.getProduceQueue";
				data2["packageId"] = 0.0f;
				data2["ok"] = 1;

				amf3array producequeue = amf3array();
				stTroopQueue * train = pcity->GetBarracksQueue(-2);
				amf3object producequeueobj = amf3object();
				amf3array producequeueinner = amf3array();
				amf3object producequeueinnerobj = amf3object();

				std::list<stTroopTrain>::iterator iter;

				client->lists.lock();
				if (train->queue.size() > 0)
				{
					for (iter = train->queue.begin(); iter != train->queue.end(); ++iter)
					{
						producequeueinnerobj["num"] = iter->count;
						producequeueinnerobj["queueId"] = iter->queueid;
						producequeueinnerobj["endTime"] = (iter->endtime > 0) ? (iter->endtime - client->m_lag) : (iter->endtime);// HACK: attempt to fix "lag" issues
						producequeueinnerobj["type"] = iter->troopid;
						producequeueinnerobj["costTime"] = iter->costtime / 1000;
						producequeueinner.Add(producequeueinnerobj);
					}
				}
				producequeueobj["allProduceQueue"] = producequeueinner;
				producequeueobj["positionId"] = -2;
				producequeue.Add(producequeueobj);

				client->lists.unlock();

				data2["allProduceQueue"] = producequeue;

				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "getFortificationsProduceList"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				uint32_t castleid = data["castleId"];

				obj2["cmd"] = "fortifications.getFortificationsProduceList";
				amf3array fortlist = amf3array();


				for (int i = 0; i < 20; ++i)
				{
					if (gserver->m_troopconfig[i].inside != 2)
						continue;
					if (gserver->m_troopconfig[i].time > 0)
					{
						amf3object parent;
						amf3object conditionbean;

						double costtime = gserver->m_troopconfig[i].time;
						double mayorinf = 1;
						if (pcity->m_mayor)
							mayorinf = pow(0.995, pcity->m_mayor->GetManagement());

						costtime = (costtime)* (mayorinf)* (pow(0.9, client->GetResearchLevel(T_CONSTRUCTION)));

						conditionbean["time"] = floor(costtime);
						conditionbean["destructTime"] = 0;
						conditionbean["wood"] = gserver->m_troopconfig[i].wood;
						conditionbean["food"] = gserver->m_troopconfig[i].food;
						conditionbean["iron"] = gserver->m_troopconfig[i].iron;
						conditionbean["gold"] = gserver->m_troopconfig[i].gold;
						conditionbean["stone"] = gserver->m_troopconfig[i].stone;

						amf3array buildings = amf3array();
						amf3array items = amf3array();
						amf3array techs = amf3array();

						for_each(gserver->m_troopconfig[i].buildings.begin(), gserver->m_troopconfig[i].buildings.end(), [&](stPrereq & req)
						{
							if (req.id > 0)
							{
								amf3object ta = amf3object();
								ta["level"] = req.level;
								int temp = pcity->GetBuildingLevel(req.id);
								ta["curLevel"] = temp;
								ta["successFlag"] = temp >= req.level ? true : false;
								ta["typeId"] = req.id;
								buildings.Add(ta);
							}
						});
						for_each(gserver->m_troopconfig[i].items.begin(), gserver->m_troopconfig[i].items.end(), [&](stPrereq & req)
						{
							if (req.id > 0)
							{
								amf3object ta = amf3object();
								int temp = client->m_items[req.id].count;
								ta["curNum"] = temp;
								ta["num"] = req.level;
								ta["successFlag"] = temp >= req.level ? true : false;
								ta["id"] = gserver->m_items[req.id].name;
								items.Add(ta);
							}
						});
						for_each(gserver->m_troopconfig[i].techs.begin(), gserver->m_troopconfig[i].techs.end(), [&](stPrereq & req)
						{
							if (req.id > 0)
							{
								amf3object ta = amf3object();
								ta["level"] = req.level;
								int temp = client->GetResearchLevel(req.id);
								ta["curLevel"] = temp;
								ta["successFlag"] = temp >= req.level ? true : false;
								ta["typeId"] = req.id;
								buildings.Add(ta);
							}
						});

						conditionbean["buildings"] = buildings;
						conditionbean["items"] = items;
						conditionbean["techs"] = techs;
						conditionbean["population"] = 0;
						parent["conditionBean"] = conditionbean;
						parent["permition"] = false;
						parent["typeId"] = i;
						fortlist.Add(parent);
					}
				}


				data2["fortList"] = fortlist;
				data2["ok"] = 1;
				data2["packageId"] = 0.0f;

				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "produceWallProtect"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				uint32_t castleid = data["castleId"];


				obj2["cmd"] = "fortifications.produceWallProtect";
				data2["packageId"] = 0.0f;
				data2["ok"] = 1;

				int trooptype = data["wallProtectType"];
				int num = data["num"];

				stResources res;
				res.food = gserver->m_troopconfig[trooptype].food * num;
				res.wood = gserver->m_troopconfig[trooptype].wood * num;
				res.stone = gserver->m_troopconfig[trooptype].stone * num;
				res.iron = gserver->m_troopconfig[trooptype].iron * num;
				res.gold = gserver->m_troopconfig[trooptype].gold * num;


				if ((res.food > pcity->m_resources.food)
					|| (res.wood > pcity->m_resources.wood)
					|| (res.stone > pcity->m_resources.stone)
					|| (res.iron > pcity->m_resources.iron)
					|| (res.gold > pcity->m_resources.gold))
				{
					gserver->SendObject(client, gserver->CreateError("fortifications.produceWallProtect", -1, "Not enough resources."));
					return;
				}

				pcity->m_resources -= res;
				pcity->ResourceUpdate();

				if (pcity->AddToBarracksQueue(-2, trooptype, num, false, false) == -1)
				{
					gserver->SendObject(client, gserver->CreateError("fortifications.produceWallProtect", -99, "Troops could not be trained."));
					return;
				}

				gserver->SendObject(client, obj2);

				client->SaveToDB();
				pcity->SaveToDB();

				return;
			}
			if ((command == "cancelFortificationProduce"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				uint32_t castleid = data["castleId"];
				int positionid = data["positionId"];
				int queueid = data["queueId"];


				obj2["cmd"] = "fortifications.cancelFortificationProduce";
				data2["packageId"] = 0.0f;
				data2["ok"] = 1;


				client->lists.lock();
				stTroopQueue * tq = pcity->GetBarracksQueue(-2);
				std::list<stTroopTrain>::iterator iter;

				for (iter = tq->queue.begin(); iter != tq->queue.end();)
				{
					if (iter->queueid == queueid)
					{
						if (iter->endtime > 0)
						{
							//in progress
							//refund 1/3 resources and set next queue to run
							stResources res;
							res.food = (gserver->m_troopconfig[iter->troopid].food * iter->count) / 3;
							res.wood = (gserver->m_troopconfig[iter->troopid].wood * iter->count) / 3;
							res.stone = (gserver->m_troopconfig[iter->troopid].stone * iter->count) / 3;
							res.iron = (gserver->m_troopconfig[iter->troopid].iron * iter->count) / 3;
							res.gold = (gserver->m_troopconfig[iter->troopid].gold * iter->count) / 3;

							pcity->m_resources += res;
							pcity->ResourceUpdate();
							tq->queue.erase(iter++);

							iter->endtime = unixtime() + iter->costtime;
						}
						else
						{
							//not in progress
							//refund all resources
							stResources res;
							res.food = gserver->m_troopconfig[iter->troopid].food * iter->count;
							res.wood = gserver->m_troopconfig[iter->troopid].wood * iter->count;
							res.stone = gserver->m_troopconfig[iter->troopid].stone * iter->count;
							res.iron = gserver->m_troopconfig[iter->troopid].iron * iter->count;
							res.gold = gserver->m_troopconfig[iter->troopid].gold * iter->count;

							pcity->m_resources += res;
							tq->queue.erase(iter++);
							pcity->ResourceUpdate();
						}

						gserver->SendObject(client, obj2);
						client->lists.unlock();

						client->SaveToDB();
						pcity->SaveToDB();

						return;
					}
					++iter;
				}
				client->lists.unlock();
			}
		}
#pragma endregion
#pragma region troop
		if ((cmdtype == "troop"))
		{
			if ((command == "getProduceQueue"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				uint32_t castleid = data["castleId"];

				obj2["cmd"] = "troop.getProduceQueue";
				data2["packageId"] = 0.0f;
				data2["ok"] = 1;

				client->lists.lock();
				amf3array producequeue = amf3array();
				for (int i = 0; i < 35; ++i)
				{
					stBuilding * building = pcity->GetBuilding(i);
					if ((building) && (building->type == B_BARRACKS))
					{
						stTroopQueue * train = pcity->GetBarracksQueue(i);

						if (train == 0)
						{
							gserver->consoleLogger->information(Poco::format("Crash! PlayerCity::stTroopQueue * train = pcity->GetBarracksQueue(%?d); - %?d", i, __LINE__));
							client->lists.unlock();
							return;
						}

						amf3object producequeueobj = amf3object();
						amf3array producequeueinner = amf3array();
						amf3object producequeueinnerobj = amf3object();

						std::list<stTroopTrain>::iterator iter;

						if (train->queue.size() > 0)
							for (iter = train->queue.begin(); iter != train->queue.end(); ++iter)
							{
							producequeueinnerobj["num"] = iter->count;
							producequeueinnerobj["queueId"] = iter->queueid;
							producequeueinnerobj["endTime"] = (iter->endtime > 0) ? (iter->endtime - client->m_lag) : (iter->endtime);// HACK: attempt to fix "lag" issues
							producequeueinnerobj["type"] = iter->troopid;
							producequeueinnerobj["costTime"] = iter->costtime / 1000;
							producequeueinner.Add(producequeueinnerobj);
							}
						producequeueobj["allProduceQueue"] = producequeueinner;
						producequeueobj["positionId"] = building->id;
						producequeue.Add(producequeueobj);
					}
				}

				client->lists.unlock();

				data2["allProduceQueue"] = producequeue;

				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "getTroopProduceList"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				uint32_t castleid = data["castleId"];

				obj2["cmd"] = "troop.getTroopProduceList";
				amf3array trooplist = amf3array();


				for (int i = 0; i < 20; ++i)
				{
					if (gserver->m_troopconfig[i].inside != 1)
						continue;
					if (gserver->m_troopconfig[i].time > 0)
					{
						amf3object parent;
						amf3object conditionbean;

						double costtime = gserver->m_troopconfig[i].time;
						double mayorinf = 1;
						if (pcity->m_mayor)
							mayorinf = pow(0.995, pcity->m_mayor->GetPower());

						switch (i)
						{
							case TR_CATAPULT:
							case TR_RAM:
							case TR_TRANSPORTER:
							case TR_BALLISTA:
								costtime = (costtime)* (mayorinf)* (pow(0.9, client->GetResearchLevel(T_METALCASTING)));
								break;
							default:
								costtime = (costtime)* (mayorinf)* (pow(0.9, client->GetResearchLevel(T_MILITARYSCIENCE)));
								break;
						}

						conditionbean["time"] = floor(costtime);
						conditionbean["destructTime"] = 0;
						conditionbean["wood"] = gserver->m_troopconfig[i].wood;
						conditionbean["food"] = gserver->m_troopconfig[i].food;
						conditionbean["iron"] = gserver->m_troopconfig[i].iron;
						conditionbean["gold"] = gserver->m_troopconfig[i].gold;
						conditionbean["stone"] = gserver->m_troopconfig[i].stone;

						amf3array buildings = amf3array();
						amf3array items = amf3array();
						amf3array techs = amf3array();

						for_each(gserver->m_troopconfig[i].buildings.begin(), gserver->m_troopconfig[i].buildings.end(), [&](stPrereq & req)
						{
							if (req.id > 0)
							{
								amf3object ta = amf3object();
								ta["level"] = req.level;
								int temp = pcity->GetBuildingLevel(req.id);
								ta["curLevel"] = temp;
								ta["successFlag"] = temp >= req.level ? true : false;
								ta["typeId"] = req.id;
								buildings.Add(ta);
							}
						});
						for_each(gserver->m_troopconfig[i].items.begin(), gserver->m_troopconfig[i].items.end(), [&](stPrereq & req)
						{
							if (req.id > 0)
							{
								amf3object ta = amf3object();
								int temp = client->m_items[req.id].count;
								ta["curNum"] = temp;
								ta["num"] = req.level;
								ta["successFlag"] = temp >= req.level ? true : false;
								ta["id"] = gserver->m_items[req.id].name;
								items.Add(ta);
							}
						});
						for_each(gserver->m_troopconfig[i].techs.begin(), gserver->m_troopconfig[i].techs.end(), [&](stPrereq & req)
						{
							if (req.id > 0)
							{
								amf3object ta = amf3object();
								ta["level"] = req.level;
								int temp = client->GetResearchLevel(req.id);
								ta["curLevel"] = temp;
								ta["successFlag"] = temp >= req.level ? true : false;
								ta["typeId"] = req.id;
								buildings.Add(ta);
							}
						});


						conditionbean["buildings"] = buildings;
						conditionbean["items"] = items;
						conditionbean["techs"] = techs;
						conditionbean["population"] = gserver->m_troopconfig[i].population;
						parent["conditionBean"] = conditionbean;
						parent["permition"] = false;
						parent["typeId"] = i;
						trooplist.Add(parent);
					}
				}


				data2["troopList"] = trooplist;
				data2["ok"] = 1;
				data2["packageId"] = 0.0f;

				gserver->SendObject(client, obj2);
				return;
			}
			if ((command == "produceTroop"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				uint32_t castleid = data["castleId"];


				obj2["cmd"] = "troop.produceTroop";
				data2["packageId"] = 0.0f;
				data2["ok"] = 1;

				int trooptype = data["troopType"];
				bool isshare = data["isShare"];
				bool toidle = data["toIdle"];
				int positionid = data["positionId"];
				int num = data["num"];


				stResources res;
				res.food = gserver->m_troopconfig[trooptype].food * num;
				res.wood = gserver->m_troopconfig[trooptype].wood * num;
				res.stone = gserver->m_troopconfig[trooptype].stone * num;
				res.iron = gserver->m_troopconfig[trooptype].iron * num;
				res.gold = gserver->m_troopconfig[trooptype].gold * num;


				if ((res.food > pcity->m_resources.food)
					|| (res.wood > pcity->m_resources.wood)
					|| (res.stone > pcity->m_resources.stone)
					|| (res.iron > pcity->m_resources.iron)
					|| (res.gold > pcity->m_resources.gold))
				{
					gserver->SendObject(client, gserver->CreateError("troop.produceTroop", -1, "Not enough resources."));
					return;
				}

				if (isshare || toidle)
				{
					gserver->SendObject(client, gserver->CreateError("troop.produceTroop", -99, "Not supported action."));
					return;
				}

				pcity->m_resources -= res;
				pcity->ResourceUpdate();

				LOCK(M_TIMEDLIST);
				if (pcity->AddToBarracksQueue(positionid, trooptype, num, isshare, toidle) == -1)
				{
					UNLOCK(M_TIMEDLIST);
					gserver->SendObject(client, gserver->CreateError("troop.produceTroop", -99, "Troops could not be trained."));
					return;
				}
				UNLOCK(M_TIMEDLIST);




				gserver->SendObject(client, obj2);

				client->SaveToDB();
				pcity->SaveToDB();

				return;
			}
			if ((command == "cancelTroopProduce"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				uint32_t castleid = data["castleId"];
				int positionid = data["positionId"];
				int queueid = data["queueId"];


				obj2["cmd"] = "troop.cancelTroopProduce";
				data2["packageId"] = 0.0f;
				data2["ok"] = 1;


				client->lists.lock();
				stTroopQueue * tq = pcity->GetBarracksQueue(positionid);
				std::list<stTroopTrain>::iterator iter;

				for (iter = tq->queue.begin(); iter != tq->queue.end();)
				{
					if (iter->queueid == queueid)
					{
						if (iter->endtime > 0)
						{
							//in progress
							//refund 1/3 resources and set next queue to run
							stResources res;
							res.food = (gserver->m_troopconfig[iter->troopid].food * iter->count) / 3;
							res.wood = (gserver->m_troopconfig[iter->troopid].wood * iter->count) / 3;
							res.stone = (gserver->m_troopconfig[iter->troopid].stone * iter->count) / 3;
							res.iron = (gserver->m_troopconfig[iter->troopid].iron * iter->count) / 3;
							res.gold = (gserver->m_troopconfig[iter->troopid].gold * iter->count) / 3;

							pcity->m_resources += res;
							pcity->ResourceUpdate();
							tq->queue.erase(iter++);

							iter->endtime = unixtime() + iter->costtime;
						}
						else
						{
							//not in progress
							//refund all resources
							stResources res;
							res.food = gserver->m_troopconfig[iter->troopid].food * iter->count;
							res.wood = gserver->m_troopconfig[iter->troopid].wood * iter->count;
							res.stone = gserver->m_troopconfig[iter->troopid].stone * iter->count;
							res.iron = gserver->m_troopconfig[iter->troopid].iron * iter->count;
							res.gold = gserver->m_troopconfig[iter->troopid].gold * iter->count;

							pcity->m_resources += res;
							tq->queue.erase(iter++);
							pcity->ResourceUpdate();
						}

						gserver->SendObject(client, obj2);

						client->lists.unlock();

						client->SaveToDB();
						pcity->SaveToDB();

						return;
					}
					++iter;
				}
				client->lists.unlock();
			}
		}
#pragma endregion
#pragma region interior
		if ((cmdtype == "interior"))
		{
			if ((command == "modifyTaxRate"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				uint32_t castleid = data["castleId"];

				int temp = data["tax"];
				if (temp < 0 || temp > 100)
				{
					pcity->m_workrate.gold = 0;
					// TODO error reporting - interior.modifyTaxRate
				}
				else
				{
					pcity->m_workrate.gold = data["tax"];
				}

				obj2["cmd"] = "interior.modifyTaxRate";
				data2["packageId"] = 0.0f;
				data2["ok"] = 1;

				gserver->SendObject(client, obj2);

				pcity->CalculateStats();
				pcity->CalculateResources();
				pcity->ResourceUpdate();

				client->SaveToDB();
				pcity->SaveToDB();

				return;
			}
			if ((command == "pacifyPeople"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				uint32_t castleid = data["castleId"];

				if (timestamp - pcity->m_lastcomfort < 15 * 60 * 1000)
				{
					{
						std::stringstream ss;
						double timediff = pcity->m_lastcomfort + 15 * 60 * 1000 - timestamp;
						int min = timediff / 1000 / 60;
						int sec = int(((((timediff) / 1000) / 60) - min) * 60);
						ss << min << "m ";
						ss << sec << "s ";
						ss << "interval needed for next comforting.";
						gserver->SendObject(client, gserver->CreateError("interior.pacifyPeople", -34, ss.str()));
					}
					return;
				}

				int itypeid = data["typeId"];
				pcity->m_lastcomfort = timestamp;

				obj2["cmd"] = "interior.pacifyPeople";
				data2["packageId"] = 0.0f;
				data2["ok"] = -99;

				switch (itypeid)
				{
					case 1://Disaster Relief 100% pop limit in food for cost, increases loyalty by 5 reduces grievance by 15
						if (pcity->m_resources.food < pcity->m_maxpopulation)
						{
							gserver->SendObject(client, gserver->CreateError("interior.pacifyPeople", -99, "Not enough food."));
							return;
						}
						pcity->m_resources.food -= pcity->m_maxpopulation;
						pcity->m_loyalty += 5;
						pcity->m_grievance -= 15;
						if (pcity->m_loyalty > 100)
							pcity->m_loyalty = 100;
						if (pcity->m_grievance < 0)
							pcity->m_grievance = 0;
						pcity->ResourceUpdate();
						client->PlayerUpdate();
						break;
					case 2://Praying 100% pop limit in food for cost, increases loyalty by 25 reduces grievance by 5
						if (pcity->m_resources.food < pcity->m_maxpopulation)
						{
							gserver->SendObject(client, gserver->CreateError("interior.pacifyPeople", -99, "Not enough food."));
							return;
						}
						pcity->m_resources.food -= pcity->m_maxpopulation;
						pcity->m_loyalty += 25;
						pcity->m_grievance -= 5;
						if (pcity->m_loyalty > 100)
							pcity->m_loyalty = 100;
						if (pcity->m_grievance < 0)
							pcity->m_grievance = 0;
						pcity->ResourceUpdate();
						client->PlayerUpdate();
						break;
					case 3://Blessing 10% pop limit in gold for cost, increases food by 100% pop limit - chance for escaping disaster?
						if (pcity->m_resources.gold < (pcity->m_maxpopulation / 10))
						{
							gserver->SendObject(client, gserver->CreateError("interior.pacifyPeople", -99, "Not enough gold."));
							return;
						}
						pcity->m_resources.gold -= (pcity->m_maxpopulation / 10);
						pcity->m_resources.food += pcity->m_maxpopulation;
						if (rand() % 10 == 1)
						{
							pcity->m_resources.food += pcity->m_maxpopulation;
							gserver->SendObject(client, gserver->CreateError("interior.pacifyPeople", -99, "Free blessing!"));

							pcity->ResourceUpdate();
							client->PlayerUpdate();

							client->SaveToDB();
							pcity->SaveToDB();

							return;
						}
						pcity->ResourceUpdate();
						client->PlayerUpdate();
						break;
					case 4://Population Raising 500% pop limit in food for cost, increases population by 5%
						if (pcity->m_resources.food < (pcity->m_maxpopulation * 5))
						{
							gserver->SendObject(client, gserver->CreateError("interior.pacifyPeople", -99, "Not enough food."));
							return;
						}
						pcity->m_resources.food -= (pcity->m_maxpopulation * 5);
						pcity->m_population += double(pcity->m_maxpopulation) / 20;
						if (pcity->m_population >= pcity->m_maxpopulation)
							pcity->m_population = pcity->m_maxpopulation;
						pcity->ResourceUpdate();
						client->PlayerUpdate();
						break;
				}

				// TODO finish - interior.pacifyPeople
				obj2["cmd"] = "interior.pacifyPeople";
				data2["packageId"] = 0.0f;
				data2["ok"] = 1;

				gserver->SendObject(client, obj2);

				client->SaveToDB();
				pcity->SaveToDB();

				return;
			}
			if ((command == "taxation"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				uint32_t castleid = data["castleId"];

				if (timestamp - pcity->m_lastlevy < 15 * 60 * 1000)
				{
					{
						std::stringstream ss;
						double timediff = pcity->m_lastlevy + 15 * 60 * 1000 - timestamp;
						int min = timediff / 1000 / 60;
						int sec = int(((((timediff) / 1000) / 60) - min) * 60);
						ss << min << "m ";
						ss << sec << "s ";
						ss << "interval needed for next levy.";
						gserver->SendObject(client, gserver->CreateError("interior.taxation", -34, ss.str()));
					}
					return;
				}

				int itypeid = data["typeId"];
				pcity->m_lastlevy = timestamp;

				obj2["cmd"] = "interior.taxation";
				data2["packageId"] = 0.0f;
				data2["ok"] = -99;

				if (pcity->m_loyalty <= 20) //not enough loyalty to levy
				{
					gserver->SendObject(client, gserver->CreateError("interior.taxation", -99, "Loyalty too low. Please comfort first."));
					return;
				}
				pcity->m_loyalty -= 20;

				switch (itypeid)
				{
					case 1://Gold 10% current pop
						pcity->m_resources.gold += (pcity->m_population / 10);
						break;
					case 2://Food 100% current pop
						pcity->m_resources.food += pcity->m_population;
						break;
					case 3://Wood 100% current pop 
						pcity->m_resources.wood += pcity->m_population;
						break;
					case 4://Stone 50% current pop 
						pcity->m_resources.stone += (pcity->m_population / 2);
						break;
					case 5://Iron 40% current pop 
						pcity->m_resources.iron += (pcity->m_population*0.40);
						break;
				}
				pcity->ResourceUpdate();
				client->PlayerUpdate();

				// TODO finish - interior.taxation
				obj2["cmd"] = "interior.taxation";
				data2["packageId"] = 0.0f;
				data2["ok"] = 1;

				gserver->SendObject(client, obj2);

				client->SaveToDB();
				pcity->SaveToDB();

				return;
			}
			if ((command == "modifyCommenceRate"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				uint32_t castleid = data["castleId"];

				int temp = data["foodrate"];

				pcity->CalculateStats();
				pcity->CalculateResources();

				if (temp < 0 || temp > 100)
				{
					pcity->m_workrate.food = 0;
					// TODO error reporting - interior.modifyCommenceRate
				}
				else
				{
					pcity->m_workrate.food = data["foodrate"];
				}
				temp = data["woodrate"];
				if (temp < 0 || temp > 100)
				{
					pcity->m_workrate.wood = 0;
					// TODO error reporting - interior.modifyCommenceRate
				}
				else
				{
					pcity->m_workrate.wood = data["woodrate"];
				}
				temp = data["ironrate"];
				if (temp < 0 || temp > 100)
				{
					pcity->m_workrate.iron = 0;
					// TODO error reporting - interior.modifyCommenceRate
				}
				else
				{
					pcity->m_workrate.iron = data["ironrate"];
				}
				temp = data["stonerate"];
				if (temp < 0 || temp > 100)
				{
					pcity->m_workrate.stone = 0;
					// TODO error reporting - interior.modifyCommenceRate
				}
				else
				{
					pcity->m_workrate.stone = data["stonerate"];
				}

				obj2["cmd"] = "interior.modifyCommenceRate";
				data2["packageId"] = 0.0f;
				data2["ok"] = 1;

				gserver->SendObject(client, obj2);

				pcity->CalculateStats();
				pcity->ResourceUpdate();

				client->SaveToDB();
				pcity->SaveToDB();

				return;
			}
			if ((command == "getResourceProduceData"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				uint32_t castleid = data["castleId"];

				obj2["cmd"] = "interior.getResourceProduceData";
				data2["packageId"] = 0.0f;
				data2["ok"] = 1;
				data2["resourceProduceDataBean"] = pcity->ResourceProduceData();

				gserver->SendObject(client, obj2);
				return;
			}
		}
#pragma endregion
#pragma region hero
		if ((cmdtype == "hero"))
		{
			if ((command == "awardGold"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				for (int i = 0; i < 10; ++i)
				{
					if ((pcity->m_heroes[i]) && (pcity->m_heroes[i]->m_id == (uint64_t)obj["heroId"]))
					{
						Hero * hero = pcity->m_heroes[i];
						uint32_t goldcost = hero->m_level * 100;

						if (pcity->m_resources.gold < goldcost)
						{
							obj2["cmd"] = "hero.awardGold";
							data2["ok"] = -99;
							data2["errorMsg"] = "Not enough gold";
							data2["packageId"] = 0.0f;

							gserver->SendObject(client, obj2);
							return;
						}

						hero->m_loyalty += 5;
						if (hero->m_loyalty > 100)
							hero->m_loyalty = 100;

						pcity->m_resources.gold -= goldcost;

						pcity->ResourceUpdate();
						pcity->HeroUpdate(hero, 2);

						obj2["cmd"] = "hero.awardGold";
						data2["ok"] = 1;
						data2["packageId"] = 0.0f;

						gserver->SendObject(client, obj2);
						return;
					}
				}

				obj2["cmd"] = "hero.awardGold";
				data2["ok"] = -99;
				data2["errorMsg"] = "Invalid hero";
				data2["packageId"] = 0.0f;

				gserver->SendObject(client, obj2);
				return;
			}
			if (command == "changeName")
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				int64_t heroid = obj["heroId"];
				string newname = obj["newName"];

				Hero * hero = pcity->GetHero(heroid);

				if (!hero)
				{
					gserver->SendObject(client, gserver->CreateError("hero.changeName", -99, "Hero not found."));
					return;
				}

				hero->m_name = newname;

				pcity->HeroUpdate(hero, 2);

				obj2["cmd"] = "hero.changeName";
				data2["ok"] = 1;
				data2["packageId"] = 0.0f;
				gserver->SendObject(client, obj2);

				return;
			}
			if ((command == "getHerosListFromTavern"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				int innlevel = pcity->GetBuildingLevel(B_INN);

				if (innlevel > 0)
				{
					obj2["cmd"] = "hero.getHerosListFromTavern";
					data2["ok"] = 1;
					data2["packageId"] = 0.0f;
					int tempnum = 0;
					for (int i = 0; i < 10; ++i)
						if (pcity->m_heroes[i])
							tempnum++;
					data2["posCount"] = pcity->GetBuildingLevel(B_FEASTINGHALL) - tempnum;

					amf3array heroes = amf3array();

					for (int i = 0; i < innlevel; ++i)
					{
						if (!pcity->m_innheroes[i])
						{
							pcity->m_innheroes[i] = gserver->CreateRandomHero(innlevel);
						}

						amf3object temphero = pcity->m_innheroes[i]->ToObject();
						heroes.Add(temphero);
					}

					data2["heros"] = heroes;

					gserver->SendObject(client, obj2);
				}
				else
				{
					obj2["cmd"] = "hero.getHerosListFromTavern";
					data2["ok"] = -1; // TODO find error (no inn exists) - hero.getHerosListFromTavern
					data2["packageId"] = 0.0f;

					gserver->SendObject(client, obj2);
				}

				return;
			}
			if ((command == "hireHero"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();


				string heroname = data["heroName"];
				int blevel = pcity->GetBuildingLevel(B_FEASTINGHALL);

				if (blevel <= pcity->HeroCount())
				{
					gserver->SendObject(client, gserver->CreateError("hero.hireHero", -99, "Insufficient vacancies in Feasting Hall."));
					return;
				}

				for (int i = 0; i < 10; ++i)
				{
					if (pcity->m_innheroes[i]->m_name == heroname)
					{
						int32_t hirecost = pcity->m_innheroes[i]->m_level * 1000;
						if (hirecost > pcity->m_resources.gold)
						{
							// TODO Get proper not enough gold to hire error code - hero.hireHero
							gserver->SendObject(client, gserver->CreateError("hero.hireHero", -99, "Not enough gold!"));
							return;
						}

						for (int x = 0; x < 10; ++x)
						{
							if (!pcity->m_heroes[x])
							{
								pcity->m_resources.gold -= hirecost;
								pcity->CalculateResourceStats();
								pcity->CalculateStats();
								pcity->CalculateResources();
								pcity->m_innheroes[i]->m_id = gserver->m_heroid++;
								LOCK(M_HEROLIST);
								pcity->HeroUpdate(pcity->m_innheroes[i], 0);
								pcity->m_heroes[x] = pcity->m_innheroes[i];
								pcity->m_innheroes[i] = 0;
								UNLOCK(M_HEROLIST);
								pcity->ResourceUpdate();
								pcity->m_heroes[x]->m_client = client;
								pcity->m_heroes[x]->m_ownerid = client->m_accountid;
								pcity->m_heroes[x]->m_castleid = pcity->m_castleid;
								pcity->m_heroes[x]->InsertToDB();
								break;
							}
						}

						obj2["cmd"] = "hero.hireHero";
						data2["ok"] = 1;
						data2["packageId"] = 0.0f;

						gserver->SendObject(client, obj2);

						client->SaveToDB();
						pcity->SaveToDB();

						return;
					}
				}
				gserver->SendObject(client, gserver->CreateError("hero.hireHero", -99, "Hero does not exist!"));
				return;
			}
			if ((command == "fireHero"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				int heroid = data["heroId"];

				for (int i = 0; i < 10; ++i)
				{
					if (pcity->m_heroes[i] && pcity->m_heroes[i]->m_id == heroid)
					{
						if (pcity->m_heroes[i]->m_status != 0)
						{
							gserver->SendObject(client, gserver->CreateError("hero.fireHero", -80, "Status of this hero is not Idle!"));
							return;
						}
						pcity->CalculateResourceStats();
						pcity->CalculateStats();
						pcity->CalculateResources();
						pcity->ResourceUpdate();
						pcity->HeroUpdate(pcity->m_heroes[i], 1);

						MULTILOCK(M_HEROLIST, M_DELETELIST);
						gserver->m_deletedhero.push_back(pcity->m_heroes[i]->m_id);

						pcity->m_heroes[i]->DeleteFromDB();

						delete pcity->m_heroes[i];
						pcity->m_heroes[i] = 0;
						UNLOCK(M_HEROLIST);
						UNLOCK(M_DELETELIST);

						obj2["cmd"] = "hero.fireHero";
						data2["ok"] = 1;
						data2["packageId"] = 0.0f;

						gserver->SendObject(client, obj2);
						return;
					}
				}

				gserver->SendObject(client, gserver->CreateError("hero.fireHero", -99, "Hero does not exist!"));
				return;
			}
			if ((command == "promoteToChief"))
			{// BUG : Correct hero not being promoted
				VERIFYCASTLEID();
				CHECKCASTLEID();

				int heroid = data["heroId"];

				for (int i = 0; i < 10; ++i)
				{
					if (pcity->m_heroes[i] && pcity->m_heroes[i]->m_id == heroid)
					{
						pcity->CalculateResourceStats();
						pcity->CalculateStats();
						pcity->CalculateResources();
						if (pcity->m_mayor)
							pcity->m_mayor->m_status = DEF_HEROIDLE;
						pcity->m_mayor = pcity->m_heroes[i];
						pcity->m_heroes[i]->m_status = DEF_HEROMAYOR;

						pcity->HeroUpdate(pcity->m_mayor, 2);
						pcity->CalculateResourceStats();
						pcity->CalculateResources();
						pcity->ResourceUpdate();

						obj2["cmd"] = "hero.promoteToChief";
						data2["ok"] = 1;
						data2["packageId"] = 0.0f;

						gserver->SendObject(client, obj2);

						pcity->SaveToDB();

						return;
					}
				}
				gserver->SendObject(client, gserver->CreateError("hero.promoteToChief", -99, "TODO error message - hero.promoteToChief"));
				return;
				// TODO needs an error message - hero.promoteToChief
			}
			if ((command == "dischargeChief"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				if (!pcity->m_mayor)
				{
					gserver->SendObject(client, gserver->CreateError("hero.dischargeChief", -99, "Castellan is not appointed yet."));
					return;
				}

				pcity->CalculateResourceStats();
				pcity->CalculateStats();
				pcity->CalculateResources();
				pcity->m_mayor->m_status = 0;
				pcity->HeroUpdate(pcity->m_mayor, 2);
				pcity->m_mayor = 0;
				pcity->CalculateResourceStats();
				pcity->CalculateResources();
				pcity->ResourceUpdate();

				obj2["cmd"] = "hero.dischargeChief";
				data2["ok"] = 1;
				data2["packageId"] = 0.0f;

				gserver->SendObject(client, obj2);

				pcity->SaveToDB();

				return;
			}
			if ((command == "resetPoint"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				int heroid = data["heroId"];

				// TODO require holy water? - hero.resetPoint

				for (int i = 0; i < 10; ++i)
				{
					if (pcity->m_heroes[i] && pcity->m_heroes[i]->m_id == heroid)
					{
						if (pcity->m_heroes[i]->m_status > 1)
						{
							gserver->SendObject(client, gserver->CreateError("hero.resetPoint", -80, "Status of this hero is not Idle!"));
							return;
						}
						pcity->CalculateResourceStats();
						pcity->CalculateStats();
						pcity->CalculateResources();
						pcity->ResourceUpdate();

						Hero * hero = pcity->m_heroes[i];
						hero->m_power = hero->m_basepower;
						hero->m_management = hero->m_basemanagement;
						hero->m_stratagem = hero->m_basestratagem;
						hero->m_remainpoint = hero->m_level;

						pcity->HeroUpdate(pcity->m_heroes[i], 2);
						pcity->CalculateResourceStats();
						pcity->CalculateResources();

						obj2["cmd"] = "hero.resetPoint";
						data2["ok"] = 1;
						data2["packageId"] = 0.0f;

						gserver->SendObject(client, obj2);

						pcity->m_heroes[i]->SaveToDB();

						return;
					}
				}

				gserver->SendObject(client, gserver->CreateError("hero.resetPoint", -99, "Hero does not exist!"));
				return;
			}
			if ((command == "addPoint"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				int heroid = data["heroId"];
				int stratagem = data["stratagem"];
				int power = data["power"];
				int management = data["management"];

				// TODO require holy water? - hero.resetPoint

				for (int i = 0; i < 10; ++i)
				{
					if (pcity->m_heroes[i] && pcity->m_heroes[i]->m_id == heroid)
					{
						pcity->CalculateResourceStats();
						pcity->CalculateStats();
						pcity->CalculateResources();
						pcity->ResourceUpdate();

						Hero * hero = pcity->m_heroes[i];
						if (((stratagem + power + management) >(hero->m_basemanagement + hero->m_basepower + hero->m_basestratagem + hero->m_level))
							|| (stratagem < hero->m_stratagem) || (power < hero->m_power) || (management < hero->m_management))
						{
							gserver->SendObject(client, gserver->CreateError("hero.addPoint", -99, "Invalid action."));
							return;
						}
						hero->m_power = power;
						hero->m_management = management;
						hero->m_stratagem = stratagem;
						hero->m_remainpoint = (hero->m_basemanagement + hero->m_basepower + hero->m_basestratagem + hero->m_level) - (stratagem + power + management);

						pcity->HeroUpdate(pcity->m_heroes[i], 2);
						pcity->CalculateResourceStats();
						pcity->CalculateResources();
						pcity->ResourceUpdate();

						obj2["cmd"] = "hero.resetPoint";
						data2["ok"] = 1;
						data2["packageId"] = 0.0f;

						gserver->SendObject(client, obj2);

						pcity->m_heroes[i]->SaveToDB();

						return;
					}
				}

				gserver->SendObject(client, gserver->CreateError("hero.addPoint", -99, "Hero does not exist!"));
				return;
			}
			if ((command == "levelUp"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				int heroid = data["heroId"];

				obj2["cmd"] = "hero.levelUp";
				data2["packageId"] = 0.0f;

				for (int i = 0; i < 10; ++i)
				{
					if (pcity->m_heroes[i] && pcity->m_heroes[i]->m_id == heroid)
					{
						Hero * hero = pcity->m_heroes[i];
						if (hero->m_experience < hero->m_upgradeexp)
						{
							gserver->SendObject(client, gserver->CreateError("hero.levelUp", -99, "Not enough experience."));
							return;
						}

						hero->m_level++;
						hero->m_remainpoint++;

						hero->m_experience -= hero->m_upgradeexp;
						hero->m_upgradeexp = hero->m_level * hero->m_level * 100;

						pcity->HeroUpdate(hero, 2);

						obj2["cmd"] = "hero.levelUp";
						data2["ok"] = 1;

						gserver->SendObject(client, obj2);

						pcity->m_heroes[i]->SaveToDB();

						return;
					}
				}

				gserver->SendObject(client, gserver->CreateError("hero.levelUp", -99, "Hero does not exist!"));
				return;
			}
			if ((command == "refreshHerosListFromTavern"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				int innlevel = pcity->GetBuildingLevel(B_INN);

				if (innlevel > 0)
				{
					obj2["cmd"] = "hero.getHerosListFromTavern";
					data2["ok"] = 1;
					data2["packageId"] = 0.0f;
					int tempnum = 0;
					for (int i = 0; i < 10; ++i)
						if (pcity->m_heroes[i])
							tempnum++;
					data2["posCount"] = pcity->GetBuildingLevel(B_FEASTINGHALL) - tempnum;

					amf3array heroes = amf3array();

					for (int i = 0; i < innlevel; ++i)
					{
						if (pcity->m_innheroes[i])
						{
							delete pcity->m_innheroes[i];
						}

						pcity->m_innheroes[i] = gserver->CreateRandomHero(innlevel);
						amf3object temphero = pcity->m_innheroes[i]->ToObject();
						heroes.Add(temphero);
					}

					data2["heros"] = heroes;

					gserver->SendObject(client, obj2);
				}
				else
				{
					// TODO find error (not enough cents) - hero.refreshHerosListFromTavern
					gserver->SendObject(client, gserver->CreateError("hero.refreshHerosListFromTavern", -99, "Not enough cents."));
				}

				return;
			}
		}
#pragma endregion
#pragma region friend
		if ((cmdtype == "friend"))
		{

		}
#pragma endregion
#pragma region city
		if ((cmdtype == "city"))
		{
			if ((command == "modifyCastleName"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				uint32_t castleid = data["castleId"];
				string logurl = data["logUrl"];
				string name = data["name"];

				pcity->m_cityname = name;
				pcity->m_logurl = logurl;
				// TODO check valid name and error reporting - city.modifyCastleName

				obj2["cmd"] = "city.modifyCastleName";
				data2["packageId"] = 0.0f;
				data2["ok"] = 1;

				gserver->SendObject(client, obj2);

				pcity->SaveToDB();

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
#pragma endregion
#pragma region furlough
		if ((cmdtype == "furlough"))
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
#pragma endregion
#pragma region army
		if (cmdtype == "army")
		{
			if (command == "IsDropItemInCastle")
			{
				//??
				int32_t y = obj["y"];
				int32_t x = obj["x"];
				return;
			}
			if ((command == "callBackArmy"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				uint32_t armyid = data["armyId"];

				return;
			}
			if ((command == "getInjuredTroop"))
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				uint32_t castleid = data["castleId"];

				amf3object obj3;
				obj3["cmd"] = "server.InjuredTroopUpdate";
				obj3["data"] = amf3object();
				amf3object & data3 = obj3["data"];
				data3["castleId"] = 0.0f;
				data3["troop"] = client->GetFocusCity()->InjuredTroops();
				gserver->SendObject(client, obj3);
				return;
			}
			if (command == "getTroopParam")
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				uint32_t castleid = data["castleId"];

				amf3object obj3;
				obj3["cmd"] = "army.getTroopParam";
				obj3["data"] = amf3object();
				amf3object & data3 = obj3["data"];
				data3["marchSkillParam"] = client->GetResearchLevel(T_COMPASS) * 10;
				data3["loadSkillParam"] = client->GetResearchLevel(T_LOGISTICS) * 10;
				data3["packageId"] = 0.0f;
				data3["ok"] = 1;
				data3["driveSkillParam"] = client->GetResearchLevel(T_HORSEBACKRIDING) * 5;
				data3["transportStationParam"] = client->GetFocusCity()->GetReliefMultiplier();
				gserver->SendObject(client, obj3);
				return;
			}
			if (command == "newArmy")
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				//TODO: should set a mutex up to prevent the possibility of multiple threads trying to start new armies effectively duplicating troops
				//or causing troop number errors

				amf3object & armybean = data["newArmyBean"];
				uint32_t castleid = data["castleId"];
				int32_t targettile = armybean["targetPoint"];
				amf3object & otroops = armybean["troops"];
				amf3object & oresources = armybean["resource"];
				stResources resources(oresources["gold"], oresources["food"], oresources["wood"], oresources["stone"], oresources["iron"]);
				bool useflag = armybean["useFlag"];
				bool backafterconstruct = armybean["backAfterConstruct"];
				int64_t heroid = armybean["heroId"];
				int64_t resttime = armybean["restTime"];// in seconds
				int16_t missiontype = armybean["missionType"];// 1 = transport | 2 = reinforce | 3 = scout | 4 = build | 5 = attack 

				stTroops troops;
				troops.archer = otroops["archer"];
				troops.ballista = otroops["ballista"];
				troops.cataphract = otroops["heavyCavalry"];
				troops.catapult = otroops["catapult"];
				troops.cavalry = otroops["lightCavalry"];
				troops.pike = otroops["pikemen"];
				troops.ram = otroops["batteringRam"];
				troops.scout = otroops["scouter"];
				troops.sword = otroops["swordsmen"];
				troops.transporter = otroops["carriage"];
				troops.warrior = otroops["militia"];
				troops.worker = otroops["peasants"];

				Tile * tile;
				tile = gserver->map->GetTileFromID(targettile);
				Client * oclient = 0;
				PlayerCity * pcity = client->GetFocusCity();
				int16_t rally = pcity->GetBuildingLevel(B_RALLYSPOT);
				if (rally < pcity->armymovement.size())
				{
					//can't send any more armies out
					gserver->SendObject(client, gserver->CreateError("army.newArmy", -51, "No more troops are allowed to sent at current level of Rally Spot. Please upgrade the Rally Spot first."));
					return;
				}

				if (missiontype == 3 || missiontype == 5)// scout or attack
				{
					if (client->Beginner())
					{
						//gserver->SendObject(client->socket, gserver->CreateError("army.newArmy", -69, "Beginner's Protection will automatically expired 7 days after registration or Town Hall reaches level 5."));
						gserver->SendObject(client, gserver->CreateError("army.newArmy", -69, "Beginner's Protection will automatically expired 7 days after registration or Town Hall reaches level 5."));
						return;
					}
					// check if valid enemy
					if (tile->m_ownerid > 0)
					{
						//player owned
						oclient = gserver->GetClient(tile->m_ownerid);
						if (oclient == 0)
						{
							//error occurred
							gserver->SendObject(client, gserver->CreateError("army.newArmy", -13, "Problem with internal mapdata. Please report this bug."));
							return;
						}
						//TODO: this will need changed for Age2 support (individual war status) (army.newArmy alliance checking)
						int16_t relation = gserver->m_alliances->GetRelation(client->m_clientnumber, oclient->m_clientnumber);
						if (oclient->Beginner())
						{
							//attacking beginner
							gserver->SendObject(client, gserver->CreateError("army.newArmy", -70, "Operation denied. This player is under Beginner's Protection."));
							return;
						}
						if (relation == DEF_SELFRELATION || relation == DEF_ALLIANCE || relation == DEF_ALLY)
						{
							//attacking self, own alliance, or allied alliance. reject attack
							gserver->SendObject(client, gserver->CreateError("army.newArmy", -13, "You can't perform this operation against this target."));
							return;
						}
						//you CAN attack this target. set everything up to pass to timers.
						if (!pcity->HasTroops(troops))
						{
							//not enough troops
							gserver->SendObject(client, gserver->CreateError("army.newArmy", -29, "You have insufficient troops."));
							return;
						}
						//you have enough troops, valid target, gogogo

						//check hero existence and status
						Hero * hero = pcity->GetHero(heroid);
						if (hero == 0)
						{
							//hero doesn't exist
							gserver->SendObject(client, gserver->CreateError("army.newArmy", -80, "Hero does not exist."));
							return;
						}
						if (hero->m_status != DEF_HEROIDLE)
						{
							//hero busy
							gserver->SendObject(client, gserver->CreateError("army.newArmy", -80, "Status of this hero is not Idle."));
							return;
						}

						stArmyMovement * am = new stArmyMovement;

						stTimedEvent te;
						am->city = pcity;
						am->client = client;

						am->hero = hero;
						am->heroname = hero->m_name;
						am->direction = 1;
						am->resources += resources;
						am->startposname = pcity->m_cityname;
						am->king = client->m_playername;
						am->troops += troops;
						am->starttime = unixtime();
						am->armyid = gserver->armycounter++;
						am->reachtime = am->starttime + gserver->CalcTroopSpeed(pcity, troops, pcity->m_tileid, targettile) * 1000;
						am->herolevel = hero->m_level;
						am->resttime = resttime;
						am->missiontype = missiontype;
						am->startfieldid = pcity->m_tileid;
						am->targetfieldid = targettile;
						am->targetposname = gserver->map->GetTileFromID(targettile)->GetName();

						te.data = am;
						te.type = DEF_TIMEDARMY;

						gserver->AddTimedEvent(te);

						client->armymovement.push_back(am);

						//timer made, remove troops from city
						pcity->m_troops -= troops;
						hero->movement = am;

						pcity->HeroUpdate(hero, 2);
						pcity->TroopUpdate();
						pcity->ResourceUpdate();
						client->SelfArmyUpdate();
						oclient->EnemyArmyUpdate();

						amf3object obj3;
						obj3["cmd"] = "army.newArmy";
						obj3["data"] = amf3object();
						amf3object & data3 = obj3["data"];
						data3["packageId"] = 0.0f;
						data3["ok"] = 1;
						//armies in embassy from other players go here
						gserver->SendObject(client, obj3);

						pcity->SaveToDB();
					}
					else
					{
						//NPC owned
						//you CAN attack this target. set everything up to pass to timers.
						if (!pcity->HasTroops(troops))
						{
							//not enough troops
							gserver->SendObject(client, gserver->CreateError("army.newArmy", -29, "You have insufficient troops."));
							return;
						}
						//you have enough troops, valid target, gogogo

						//check hero existence and status
						Hero * hero = pcity->GetHero(heroid);
						if (hero == 0)
						{
							//hero doesn't exist
							gserver->SendObject(client, gserver->CreateError("army.newArmy", -80, "Hero does not exist."));
							return;
						}
						if (hero->m_status != DEF_HEROIDLE)
						{
							//hero busy
							gserver->SendObject(client, gserver->CreateError("army.newArmy", -80, "Status of this hero is not Idle."));
							return;
						}

						stArmyMovement * am = new stArmyMovement;

						stTimedEvent te;
						am->city = pcity;
						am->client = client;

						am->hero = hero;
						am->heroname = hero->m_name;
						am->direction = 1;
						am->resources += resources;
						am->startposname = pcity->m_cityname;
						am->king = client->m_playername;
						am->troops += troops;
						am->starttime = unixtime();
						am->armyid = gserver->armycounter++;
						am->reachtime = am->starttime + gserver->CalcTroopSpeed(pcity, troops, pcity->m_tileid, targettile) * 1000;
						am->herolevel = hero->m_level;
						am->resttime = resttime;
						am->missiontype = missiontype;
						am->startfieldid = pcity->m_tileid;
						am->targetfieldid = targettile;
						am->targetposname = gserver->map->GetTileFromID(targettile)->GetName();

						te.data = am;
						te.type = DEF_TIMEDARMY;

						gserver->AddTimedEvent(te);

						client->armymovement.push_back(am);

						//timer made, remove troops from city
						pcity->m_troops -= troops;
						hero->movement = am;

						hero->m_status = DEF_HEROATTACK;

						pcity->HeroUpdate(hero, 2);
						pcity->TroopUpdate();
						pcity->ResourceUpdate();
						client->SelfArmyUpdate();

						amf3object obj3;
						obj3["cmd"] = "army.newArmy";
						obj3["data"] = amf3object();
						amf3object & data3 = obj3["data"];
						data3["packageId"] = 0.0f;
						data3["ok"] = 1;
						//armies in embassy from other players go here
						gserver->SendObject(client, obj3);

						pcity->SaveToDB();
					}
				}
				else if (missiontype == 1 || missiontype == 2 || missiontype == 4)// not sure
				{

				}


				return;



				//Alliance * all1 = client->GetAlliance();


				// 			["data"] Type: Object - Value: Object
				// 			  ["castleId"] Type: Integer - Value: 1085204
				// 			  ["newArmyBean"] Type: Object - Value: Object
				// 				["restTime"] Type: Integer - Value: 0
				// 				["heroId"] Type: Integer - Value: 637322
				// 				["missionType"] Type: Integer - Value: 5
				// 				["troops"] Type: Object - Value: Object
				// 				  ["militia"] Type: Integer - Value: 0
				// 				  ["swordsmen"] Type: Integer - Value: 0
				// 				  ["peasants"] Type: Integer - Value: 0
				// 				  ["archer"] Type: Integer - Value: 0
				// 				  ["scouter"] Type: Integer - Value: 0
				// 				  ["heavyCavalry"] Type: Integer - Value: 0
				// 				  ["pikemen"] Type: Integer - Value: 0
				// 				  ["batteringRam"] Type: Integer - Value: 0
				// 				  ["carriage"] Type: Integer - Value: 2215
				// 				  ["ballista"] Type: Integer - Value: 1263
				// 				  ["lightCavalry"] Type: Integer - Value: 0
				// 				  ["catapult"] Type: Integer - Value: 0
				// 				["resource"] Type: Object - Value: Object
				// 				  ["stone"] Type: Integer - Value: 0
				// 				  ["food"] Type: Integer - Value: 0
				// 				  ["iron"] Type: Integer - Value: 0
				// 				  ["wood"] Type: Integer - Value: 0
				// 				  ["gold"] Type: Integer - Value: 0
				// 				["targetPoint"] Type: Integer - Value: 129249
				// 				["useFlag"] Type: Boolean - Value: False
				// 				["backAfterConstruct"] Type: Boolean - Value: False

			}
			if (command == "getStayAllianceArmys")
			{
				VERIFYCASTLEID();
				CHECKCASTLEID();

				uint32_t castleid = data["castleId"];

				amf3object obj3;
				obj3["cmd"] = "army.getStayAllianceArmys";
				obj3["data"] = amf3object();
				amf3object & data3 = obj3["data"];
				data3["packageId"] = 0.0f;
				data3["ok"] = 1;
				//armies in embassy from other players go here
				gserver->SendObject(client, obj3);
				return;
			}
		}

#pragma endregion
#pragma region gameClient
		else if (cmdtype == "gameClient")
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
					// 				obj2["cmd"] = "gameClient.kickout";
					// 				obj2["data"] = amf3object();
					// 				data2["msg"] = "You suck.";
					// 
					// 				gserver->SendObject(client, obj2);
					// 				//"other" version
					// 				return;

					amf3object obj2 = amf3object();
					obj2["cmd"] = "";
					amf3object & data2 = obj2["data"];
					data2 = amf3object();

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
#pragma endregion
#pragma region unknown
		else
		{
			amf3object obj3;
			obj3["cmd"] = "server.SystemInfoMsg";
			obj3["data"] = amf3object();
			amf3object & data3 = obj3["data"];
			data3["alliance"] = false;
			data3["tV"] = false;
			data3["noSenderSystemInfo"] = false;
			data3["msg"] = string("Unknown object - " + (string)obj["cmd"]);
			gserver->SendObject(client, obj3);
			return;
		}
	}
	catch (Poco::Data::MySQL::StatementException * e)
	{
		gserver->consoleLogger->error(Poco::format("Random SQL Exception: %s", e->displayText()));
	}
#pragma endregion
}
int32_t GetGambleCount(string item);
amf3object GenerateGamble();

void ShopUseGoods(amf3object & data, Client * client)
{
	amf3object obj2 = amf3object();
	obj2["cmd"] = "";
	amf3object & data2 = obj2["data"];

	uint32_t num = data["num"];
	string itemid = static_cast<string>(data["itemId"]);

	gserver->consoleLogger->information(Poco::format("usegoods: %s", itemid));

	//Maybe tokenize instead?
	// 	char * cmdtype, * command, * ctx;
	// 	cmdtype = strtok_s(cmd, ".", &ctx);
	// 	command = strtok_s(NULL, ".", &ctx);
	// 
	// 	if ((cmdtype == "gameClient"))
	// 	{
	// 		if ((command == "version"))
	// 		{
	// 		}
	// 	}

	/*if (itemid == "player.speak.bronze_publicity_ambassador.permanent")
	{

	}
	if (itemid == "player.speak.bronze_publicity_ambassador.permanent.15")
	{

	}
	if (itemid == "player.speak.gold_publicity_ambassador.15")
	{

	}
	if (itemid == "player.speak.gold_publicity_ambassador.permanent"){}
	if (itemid == "player.speak.silver_publicity_ambassador.15"){}
	if (itemid == "player.speak.silver_publicity_ambassador.permanent"){}
	if (itemid == "alliance.ritual_of_pact.advanced"){}
	if (itemid == "alliance.ritual_of_pact.premium"){}
	if (itemid == "alliance.ritual_of_pact.ultimate"){}
	if (itemid == "consume.1.a"){}
	if (itemid == "consume.1.b"){}
	if (itemid == "consume.1.c"){}
	if (itemid == "consume.2.a"){}
	if (itemid == "consume.2.b"){}
	if (itemid == "consume.2.b.1"){}
	if (itemid == "consume.2.c"){}
	if (itemid == "consume.2.c.1"){}
	if (itemid == "consume.2.d"){}
	if (itemid == "consume.blueprint.1"){}
	if (itemid == "consume.changeflag.1"){}
	if (itemid == "consume.hegemony.1"){}
	if (itemid == "consume.key.1"){}
	if (itemid == "consume.key.2"){}
	if (itemid == "consume.key.3"){}
	if (itemid == "consume.move.1"){}
	if (itemid == "consume.refreshtvern.1"){}
	if (itemid == "consume.transaction.1"){}
	if (itemid == "hero.intelligence.1"){}
	if (itemid == "hero.loyalty.1"){}
	if (itemid == "hero.loyalty.2"){}
	if (itemid == "hero.loyalty.3"){}
	if (itemid == "hero.loyalty.4"){}
	if (itemid == "hero.loyalty.5"){}
	if (itemid == "hero.loyalty.6"){}
	if (itemid == "hero.loyalty.7"){}
	if (itemid == "hero.loyalty.8"){}
	if (itemid == "hero.loyalty.9"){}
	if (itemid == "hero.management.1"){}
	if (itemid == "hero.power.1"){}
	if (itemid == "hero.reset.1"){}
	if (itemid == "hero.reset.1.a"){}
	if (itemid == "player.attackinc.1"){}
	if (itemid == "player.attackinc.1.b"){}
	if (itemid == "player.box.1"){}
	if (itemid == "player.box.2"){}
	if (itemid == "player.box.3"){}
	if (itemid == "player.box.currently.1"){}
	if (itemid == "player.box.gambling.1"){}
	if (itemid == "player.box.gambling.10"){}
	if (itemid == "player.box.gambling.11"){}
	if (itemid == "player.box.gambling.12"){}
	if (itemid == "player.box.gambling.2"){}
	if (itemid == "player.box.gambling.3"){}
	if (itemid == "player.box.gambling.4"){}
	if (itemid == "player.box.gambling.5"){}
	if (itemid == "player.box.gambling.6"){}
	if (itemid == "player.box.gambling.7"){}
	if (itemid == "player.box.gambling.8"){}
	if (itemid == "player.box.gambling.9"){}
	if (itemid == "player.box.gambling.food"){}
	if (itemid == "player.box.gambling.gold"){}
	if (itemid == "player.box.gambling.iron"){}
	if (itemid == "player.box.gambling.medal.10"){}
	if (itemid == "player.box.gambling.medal.300"){}
	if (itemid == "player.box.gambling.stone"){}
	if (itemid == "player.box.gambling.wood"){}
	if (itemid == "player.box.hero.a"){}
	if (itemid == "player.box.hero.b"){}
	if (itemid == "player.box.hero.c"){}
	if (itemid == "player.box.hero.d"){}
	if (itemid == "player.box.present.1"){}
	if (itemid == "player.box.present.10"){}
	if (itemid == "player.box.present.11"){}
	if (itemid == "player.box.present.2"){}
	if (itemid == "player.box.present.3"){}
	if (itemid == "player.box.present.4"){}
	if (itemid == "player.box.present.5"){}
	if (itemid == "player.box.present.6"){}
	if (itemid == "player.box.present.7"){}
	if (itemid == "player.box.present.8"){}
	if (itemid == "player.box.present.9"){}
	if (itemid == "player.box.resource.1"){}
	if (itemid == "player.box.special.1"){}
	if (itemid == "player.box.special.2"){}
	if (itemid == "player.box.special.3"){}
	if (itemid == "player.box.troop.1"){}
	if (itemid == "player.box.troop.a"){}
	if (itemid == "player.box.troop.b"){}
	if (itemid == "player.box.troop.c"){}
	if (itemid == "player.box.troop.d"){}
	if (itemid == "player.box.wood.1"){}
	if (itemid == "player.defendinc.1"){}
	if (itemid == "player.defendinc.1.b"){}
	if (itemid == "player.destroy.1.a"){}
	if (itemid == "player.experience.1.a"){}
	if (itemid == "player.experience.1.b"){}
	if (itemid == "player.experience.1.c"){}
	if (itemid == "player.fort.1.c"){}
	if (itemid == "player.gold.1.a"){}
	if (itemid == "player.gold.1.b"){}
	if (itemid == "player.heart.1.a"){}
	if (itemid == "player.more.castle.1.a"){}
	if (itemid == "player.name.1.a"){}
	if (itemid == "player.peace.1"){}
	if (itemid == "player.pop.1.a"){}
	if (itemid == "player.relive.1"){}
	if (itemid == "player.resinc.1"){}
	if (itemid == "player.resinc.1.b"){}
	if (itemid == "player.resinc.2"){}
	if (itemid == "player.resinc.2.b"){}
	if (itemid == "player.resinc.3"){}
	if (itemid == "player.resinc.3.b"){}
	if (itemid == "player.resinc.4"){}
	if (itemid == "player.resinc.4.b"){}
	if (itemid == "player.troop.1.a"){}
	if (itemid == "player.troop.1.b"){}
	if (itemid == "player.box.present.medal.50"){}
	if (itemid == "player.box.present.12"){}
	if (itemid == "player.box.present.13"){}
	if (itemid == "player.box.present.14"){}
	if (itemid == "player.box.present.15"){}
	if (itemid == "player.box.present.16"){}
	if (itemid == "player.box.present.17"){}
	if (itemid == "player.box.present.18"){}
	if (itemid == "player.box.present.19"){}
	if (itemid == "player.box.present.20"){}
	if (itemid == "player.box.present.21"){}
	if (itemid == "player.box.present.22"){}
	if (itemid == "player.box.present.medal.3500"){}
	if (itemid == "player.box.present.medal.500"){}
	if (itemid == "player.box.present.23"){}
	if (itemid == "player.box.present.24"){}
	if (itemid == "player.box.present.25"){}
	if (itemid == "player.box.present.26"){}
	if (itemid == "player.box.present.27"){}
	if (itemid == "player.box.present.28"){}
	if (itemid == "player.box.present.29"){}
	if (itemid == "player.box.present.3"){}
	if (itemid == "player.box.present.30"){}
	if (itemid == "player.box.present.31"){}
	if (itemid == "player.box.present.32"){}
	if (itemid == "player.box.present.33"){}
	if (itemid == "player.box.present.34"){}
	if (itemid == "player.box.present.35"){}
	if (itemid == "player.box.present.36"){}
	if (itemid == "player.box.present.37"){}
	if (itemid == "player.box.present.38"){}
	if (itemid == "player.box.present.recall.a"){}
	if (itemid == "player.box.present.recall.b"){}
	if (itemid == "player.box.present.recall.c"){}
	if (itemid == "player.box.present.money.3"){}
	if (itemid == "player.box.present.money.4"){}
	if (itemid == "player.box.present.money.5"){}
	if (itemid == "player.box.present.money.6"){}
	if (itemid == "player.box.present.money.7"){}
	if (itemid == "player.box.present.money.8"){}
	if (itemid == "player.box.present.money.9"){}
	if (itemid == "player.box.present.money.10"){}
	if (itemid == "player.box.present.money.11"){}
	if (itemid == "player.box.present.money.12"){}
	if (itemid == "player.box.present.money.13"){}
	if (itemid == "player.box.present.money.14"){}
	if (itemid == "player.box.present.money.15"){}
	if (itemid == "player.box.present.money.16"){}
	if (itemid == "player.box.present.money.buff.17"){}
	if (itemid == "player.box.present.money.buff.18"){}
	if (itemid == "player.box.present.money.buff.19"){}
	if (itemid == "player.box.present.money.20"){}
	if (itemid == "player.box.present.money.21"){}
	if (itemid == "player.box.present.money.22"){}
	if (itemid == "player.box.present.money.23"){}
	if (itemid == "player.key.santa"){}
	if (itemid == "player.santa.stoptoopsupkeep"){}
	if (itemid == "player.box.present.christmas.a"){}
	if (itemid == "player.box.present.christmas.b"){}
	if (itemid == "player.box.present.christmas.c"){}
	if (itemid == "player.box.present.money.24"){}
	if (itemid == "player.box.present.money.25"){}
	if (itemid == "player.key.newyear"){}
	if (itemid == "player.newyear.stoptoopsupkeep"){}
	if (itemid == "player.truce.dream"){}
	if (itemid == "player.box.present.money.27"){}
	if (itemid == "player.move.castle.1.b"){}
	if (itemid == "player.box.present.40"){}
	if (itemid == "player.key.easter_package"){}
	if (itemid == "player.box.present.money.28"){}
	if (itemid == "player.box.present.money.29"){}
	if (itemid == "player.reduce.troops.upkeep.1"){}
	if (itemid == "player.key.special.chest"){}
	if (itemid == "player.box.present.money.30"){}
	if (itemid == "player.box.present.money.31"){}
	if (itemid == "player.box.merger.compensation"){}
	if (itemid == "player.box.present.money.32"){}
	if (itemid == "player.box.present.money.33"){}
	if (itemid == "player.key.halloween"){}
	if (itemid == "player.halloween.candy"){}
	if (itemid == "player.box.present.money.34"){}
	if (itemid == "player.box.present.money.35"){}
	if (itemid == "player.box.evony.birthday"){}
	if (itemid == "player.box.present.money.36"){}
	if (itemid == "player.box.present.money.37"){}
	if (itemid == "player.box.evony.subscription"){}
	if (itemid == "player.box.toolbarbonus1.greater"){}
	if (itemid == "player.box.toolbarbonus1.lesser"){}
	if (itemid == "player.box.toolbarbonus1.medium"){}
	if (itemid == "player.box.toolbarbonus1.superior"){}
	if (itemid == "player.box.present.money.38"){}
	if (itemid == "player.box.present.money.39"){}
	if (itemid == "player.key.valentine"){}
	if (itemid == "player.cupid.chocolate"){}
	if (itemid == "player.queue.building"){}
	if (itemid == "player.key.patrick"){}
	if (itemid == "player.box.present.money.40"){}
	if (itemid == "player.box.present.money.41"){}
	if (itemid == "player.irish.whiskey"){}
	if (itemid == "player.box.present.money.42"){}
	if (itemid == "player.box.present.money.43"){}
	if (itemid == "player.key.easter"){}
	if (itemid == "player.easter.egg"){}
	if (itemid == "player.box.compensation.a"){}
	if (itemid == "player.box.compensation.b"){}
	if (itemid == "player.box.compensation.c"){}
	if (itemid == "player.box.compensation.d"){}
	if (itemid == "player.box.compensation.e"){}
	if (itemid == "player.box.present.money.44"){}
	if (itemid == "player.box.present.money.45"){}
	if (itemid == "player.key.bull"){}
	if (itemid == "player.running.shoes"){}*/

	if (itemid == "player.box.gambling.3")
	{
		obj2["cmd"] = "shop.useGoods";

		amf3array itembeans = amf3array();
		amf3array gamblingbeans = amf3array();

		stItemConfig * randitem = &gserver->m_items[rand() % gserver->m_itemcount];

		amf3object item = amf3object();
		item["id"] = randitem->name;
		item["count"] = 1;
		item["minCount"] = 0;
		item["maxCount"] = 0;

		itembeans.Add(item);

		randitem = &gserver->m_items[rand() % gserver->m_itemcount];

		obj2["data"] = GenerateGamble();

		amf3object & announceitem = obj2["data"];
		announceitem["itemBeans"] = amf3array();
		amf3array & array = announceitem["itemBeans"];
		amf3object itemobj = array.Get(0);

		int worthcents = 0;

		worthcents = gserver->GetItem((string)itemobj["id"])->cost * (int)itemobj["count"];

		std::stringstream ss;
		if (client->m_sex == 1)
			ss << "Lord ";
		else
			ss << "Lady ";
		ss << "<font color='#FF0000'><b><u>" << client->m_playername << "</u></b></font> gained <b><font color='#00A2FF'>" << (string)itemobj["count"] << " " << (string)itemobj["id"] << "</font></b> (worth <b><font color='#FF0000'>" << worthcents << "</font></b> Cents) from <b><font color='#FF0000'>Amulet</font></b>!";
		gserver->MassMessage(ss.str());

		gserver->SendObject(client, obj2);
		return;
	}
	else if (itemid == "player.box.test.1")
	{

	}
}
int32_t GetGambleCount(string item)
{
	if (item == "consume.1.a")
		return (rand() % 5 == 1) ? 5 : 1;
	if (item == "consume.2.a")
		return (rand() % 5 == 1) ? 5 : 1;
	if (item == "consume.2.b")
		return (rand() % 5 == 1) ? 5 : 1;
	if (item == "consume.2.b.1")
		return (rand() % 5 == 1) ? 5 : 1;
	if (item == "consume.blueprint.1")
		return (rand() % 5 == 1) ? 5 : 1;
	if (item == "consume.refreshtavern.1")
		return (rand() % 5 == 1) ? 10 : 5;
	if (item == "consume.transaction.1")
		return (rand() % 5 == 1) ? 10 : 5;
	if (item == "hero.loyalty.1")
		return (rand() % 5 != 1) ? 1 : 5;
	if (item == "hero.loyalty.2")
		return (rand() % 5 != 1) ? 1 : 5;
	if (item == "hero.loyalty.3")
		return (rand() % 5 != 1) ? 1 : 5;
	if (item == "hero.loyalty.4")
		return (rand() % 5 != 1) ? 1 : 5;
	if (item == "hero.loyalty.5")
		return (rand() % 5 != 1) ? 1 : 5;
	if (item == "hero.loyalty.6")
		return (rand() % 5 != 1) ? 1 : 5;
	if (item == "hero.loyalty.7")
		return (rand() % 5 != 1) ? 1 : 4;
	if (item == "hero.loyalty.8")
		return (rand() % 5 != 1) ? 1 : 3;
	if (item == "hero.loyalty.9")
		return (rand() % 5 != 1) ? 1 : 2;
	if (item == "player.box.gambling.food")
		return (rand() % 2 == 1) ? 250000 : 500000;
	if (item == "player.box.gambling.wood")
		return (rand() % 2 == 1) ? 250000 : 500000;
	if (item == "player.box.gambling.stone")
		return (rand() % 2 == 1) ? 150000 : 300000;
	if (item == "player.box.gambling.iron")
		return (rand() % 2 == 1) ? 100000 : 200000;
	if (item == "player.box.gambling.gold")
		return (rand() % 2 == 1) ? 150000 : 300000;
	if (item == "player.heart.1.a")
		return (rand() % 5 == 1) ? 10 : 5;
	if (item == "player.queue.building")
		return (rand() % 5 == 1) ? 10 : 5;
	if (item == "player.gold.1.a")
		return (rand() % 5 == 1) ? 10 : 5;
	if (item == "player.gold.1.b")
		return (rand() % 5 == 1) ? 10 : 5;

	return 1;
}
amf3object GenerateGamble()
{
	amf3array itemBeans = amf3array();
	amf3array gamblingItemsBeans = amf3array();
	amf3object data = amf3object();

	//16 normals
	//4 corners
	//3 rares (left right bottom)
	//1 super rare (top middle)

	// 24 total


	// Item rarity:
	// 5 = ultra rare (chance within super rare to appear)
	// 4 = super rare
	// 3 = semi rare
	// 2 = special
	// 1 = common


	for (int i = 0; i < 16; ++i)
	{
		amf3object obj = amf3object();
		stItemConfig * item = gserver->m_gambleitems.common.at(rand() % gserver->m_gambleitems.common.size());
		obj["id"] = item->name;
		obj["count"] = GetGambleCount(item->name);
		obj["kind"] = item->rarity - 1;
		gamblingItemsBeans.Add(obj);
		//gserver->consoleLogger->information(Poco::format("Item: %s", item->name));
	}
	for (int i = 0; i < 4; ++i)
	{
		amf3object obj = amf3object();
		stItemConfig * item = gserver->m_gambleitems.special.at(rand() % gserver->m_gambleitems.special.size());
		obj["id"] = item->name;
		obj["count"] = GetGambleCount(item->name);
		obj["kind"] = item->rarity - 1;
		gamblingItemsBeans.Add(obj);
		//gserver->consoleLogger->information(Poco::format("Item: %s", item->name));
	}
	for (int i = 0; i < 3; ++i)
	{
		amf3object obj = amf3object();
		stItemConfig * item = gserver->m_gambleitems.rare.at(rand() % gserver->m_gambleitems.rare.size());
		obj["id"] = item->name;
		obj["count"] = GetGambleCount(item->name);
		obj["kind"] = item->rarity - 1;
		gamblingItemsBeans.Add(obj);
		//gserver->consoleLogger->information(Poco::format("Item: %s", item->name));
	}
	{
		amf3object obj = amf3object();
		stItemConfig * item;
		if (rand() % 100 < 95)
		{
			item = gserver->m_gambleitems.superrare.at(rand() % gserver->m_gambleitems.superrare.size());
			obj["kind"] = item->rarity - 1;
		}
		else
		{
			item = gserver->m_gambleitems.ultrarare.at(rand() % gserver->m_gambleitems.ultrarare.size());
			obj["kind"] = item->rarity;
		}

		obj["id"] = item->name;
		obj["count"] = GetGambleCount(item->name);
		gamblingItemsBeans.Add(obj);
		//gserver->consoleLogger->information(Poco::format("Item: %s", item->name));
	}
	int get = 0;
	int value = (rand() % 100000);
	if ((value >= 0) && (value < 60000))
	{
		get = rand() % 16;
	}
	else if ((value >= 60000) && (value < 85000))
	{
		get = rand() % 4 + 16;
	}
	else if ((value >= 85000) && (value < 95000))
	{
		get = rand() % 3 + 16 + 4;
	}
	else if ((value >= 95000) && (value < 100000))
	{
		get = 23;
	}

	itemBeans.Add(gamblingItemsBeans.Get(get));

	data["itemBeans"] = itemBeans;
	data["gamblingItemsBeans"] = gamblingItemsBeans;
	data["packageId"] = 0.0f;
	data["ok"] = 1;

	return data;
}


/*
Possible messages:

respMap["alliance.getAllianceArmyReport"] = _resp_alliance_getAllianceArmyReport;
respMap["alliance.getAllianceEventList"] = _resp_alliance_getAllianceEventList;
respMap["alliance.getAllianceInfo"] = _resp_alliance_getAllianceInfo;
respMap["alliance.getAllianceList"] = _resp_alliance_getAllianceList;
respMap["alliance.getAllianceMembers"] = _resp_alliance_getAllianceMembers;
respMap["alliance.getMilitarySituationList"] = _resp_alliance_getMilitarySituationList;
respMap["alliance.addUsertoAlliance"] = _resp_alliance_addUsertoAlliance;
respMap["alliance.addUsertoAllianceList"] = _resp_alliance_addUsertoAllianceList;
respMap["alliance.agreeComeinAllianceByLeader"] = _resp_alliance_agreeComeinAllianceByLeader;
respMap["alliance.agreeComeinAllianceByUser"] = _resp_alliance_agreeComeinAllianceByUser;
respMap["alliance.agreeComeinAllianceList"] = _resp_alliance_agreeComeinAllianceList;
respMap["alliance.cancelUserWantInAlliance"] = _resp_alliance_cancelUserWantInAlliance;
respMap["alliance.canceladdUsertoAlliance"] = _resp_alliance_canceladdUsertoAlliance;
respMap["alliance.cancelagreeComeinAlliance"] = _resp_alliance_cancelagreeComeinAlliance;
respMap["alliance.createAlliance"] = _resp_alliance_createAlliance;
respMap["alliance.dropAllianceFriendshipRelation"] = _resp_alliance_dropAllianceFriendshipRelation;
respMap["alliance.getAllianceFriendshipList"] = _resp_alliance_getAllianceFriendshipList;
respMap["alliance.getAllianceWanted"] = _resp_alliance_getAllianceWanted;
respMap["alliance.getPowerFromAlliance"] = _resp_alliance_getPowerFromAlliance;
respMap["alliance.isHasAlliance"] = _resp_alliance_isHasAlliance;
respMap["alliance.kickOutMemberfromAlliance"] = _resp_alliance_kickOutMemberfromAlliance;
respMap["alliance.leaderWantUserInAllianceList"] = _resp_alliance_leaderWantUserInAllianceList;
respMap["alliance.messagesForAllianceMember"] = _resp_alliance_messagesForAllianceMember;
respMap["alliance.rejectComeinAlliance"] = _resp_alliance_rejectComeinAlliance;
respMap["alliance.resetTopPowerForAlliance"] = _resp_alliance_resetTopPowerForAlliance;
respMap["alliance.resignForAlliance"] = _resp_alliance_resignForAlliance;
respMap["alliance.sayByetoAlliance"] = _resp_alliance_sayByetoAlliance;
respMap["alliance.setAllInfoForAlliance"] = _resp_alliance_setAllInfoForAlliance;
respMap["alliance.setAllianceFriendship"] = _resp_alliance_setAllianceFriendship;
respMap["alliance.setPowerForUserByAlliance"] = _resp_alliance_setPowerForUserByAlliance;
respMap["alliance.userWantInAlliance"] = _resp_alliance_userWantInAlliance;
respMap["army.callBackArmy"] = _resp_army_callBackArmy;
respMap["army.cureInjuredTroop"] = _resp_army_cureInjuredTroop;
respMap["army.disbandInjuredTroop"] = _resp_army_disbandInjuredTroop;
respMap["army.exerciseArmy"] = _resp_army_exerciseArmy;
respMap["army.getInjuredTroop"] = _resp_army_getInjuredTroop;
respMap["army.getStayAllianceArmys"] = _resp_army_getStayAllianceArmys;
respMap["army.getTroopParam"] = _resp_army_getTroopParam;
respMap["army.newArmy"] = _resp_army_newArmy;
respMap["army.setAllowAllianceArmy"] = _resp_army_setAllowAllianceArmy;
respMap["army.setArmyGoOut"] = _resp_army_setArmyGoOut;
respMap["castle.cancelBuildingQueue"] = _resp_castle_cancelBuildingQueue;
respMap["castle.cancleBuildCommand"] = _resp_castle_cancleBuildCommand;
respMap["castle.checkOutUpgrade"] = _resp_castle_checkOutUpgrade;
respMap["castle.demolishBuildingQueue"] = _resp_castle_demolishBuildingQueue;
respMap["castle.destructBuilding"] = _resp_castle_destructBuilding;
respMap["castle.getAvailableBuildingBean"] = _resp_castle_getAvailableBuildingBean;
respMap["castle.getAvailableBuildingListInside"] = _resp_castle_getAvailableBuildingListInside;
respMap["castle.getAvailableBuildingListOutside"] = _resp_castle_getAvailableBuildingListOutside;
respMap["castle.getBuildingQueueCoinsNeed"] = _resp_castle_getBuildingQueueCoinsNeed;
respMap["castle.getCoinsNeed"] = _resp_castle_getCoinsNeed;
respMap["castle.getDestructBuildBean"] = _resp_castle_getDestructBuildBean;
respMap["castle.newBuilding"] = _resp_castle_newBuilding;
respMap["castle.newBuildingQueue"] = _resp_castle_newBuildingQueue;
respMap["castle.speedUpBuildCommand"] = _resp_castle_speedUpBuildCommand;
respMap["castle.speedupBuildingQueue"] = _resp_castle_speedupBuildingQueue;
respMap["castle.upgradeBuilding"] = _resp_castle_upgradeBuilding;
respMap["castle.deleteCastleSign"] = _resp_castle_deleteCastleSign;
respMap["castle.saveCastleSignList"] = _resp_castle_saveCastleSignList;
respMap["city.advMoveCastle"] = _resp_city_advMoveCastle;
respMap["city.constructCastle"] = _resp_city_constructCastle;
respMap["city.getStoreList"] = _resp_city_getStoreList;
respMap["city.giveupCastle"] = _resp_city_giveupCastle;
respMap["city.modifyCastleName"] = _resp_city_modifyCastleName;
respMap["city.modifyFlag"] = _resp_city_modifyFlag;
respMap["city.modifyStorePercent"] = _resp_city_modifyStorePercent;
respMap["city.modifyUserName"] = _resp_city_modifyUserName;
respMap["city.moveCastle"] = _resp_city_moveCastle;
respMap["city.setStopWarState"] = _resp_city_setStopWarState;
respMap["city.uniteAdvMoveCastle"] = _resp_city_uniteAdvMoveCastle;
respMap["common.CbUD"] = _resp_common_CbUD;
respMap["common.allianceChat"] = _resp_common_allianceChat;
respMap["common.cancelOnlineBonus"] = _resp_common_cancelOnlineBonus;
respMap["common.changeName"] = _resp_common_changeName;
respMap["common.changeUserFace"] = _resp_common_changeUserFace;
respMap["common.channelChat"] = _resp_common_channelChat;
respMap["common.createNewPlayer"] = _resp_common_createNewPlayer;
respMap["common.delUniteServerPeaceStatus"] = _resp_common_delUniteServerPeaceStatus;
respMap["common.deleteUserAndRestart"] = _resp_common_deleteUserAndRestart;
respMap["common.denyPlayerSpeak"] = _resp_common_denyPlayerSpeak;
respMap["common.getItemDefXml"] = _resp_common_getItemDefXml;
respMap["common.getOnlineBonus"] = _resp_common_getOnlineBonus;
respMap["common.getPackage"] = _resp_common_getPackage;
respMap["common.getPackageList"] = _resp_common_getPackageList;
respMap["common.getPackageNumber"] = _resp_common_getPackageNumber;
respMap["common.getPlayerInfoByName"] = _resp_common_getPlayerInfoByName;
respMap["common.mapCastle"] = _resp_common_mapCastle;
respMap["common.mapInfo"] = _resp_common_mapInfo;
respMap["common.mapInfoSimple"] = _resp_common_mapInfoSimple;
respMap["common.privateChat"] = _resp_common_privateChat;
respMap["common.refreshCaptcha"] = _resp_common_refreshCaptcha;
respMap["common.step"] = _resp_common_step;
respMap["common.worldChat"] = _resp_common_worldChat;
respMap["common.zoneInfo"] = _resp_common_zoneInfo;
respMap["common.authSecurityCode"] = _resp_common_authSecurityCode;
respMap["common.cancelRemovingSecurityCodeProcess"] = _resp_common_cancelRemovingSecurityCodeProcess;
respMap["common.changeSecurityCode"] = _resp_common_changeSecurityCode;
respMap["common.getIsSecurityCodeSetted"] = _resp_common_getIsSecurityCodeSetted;
respMap["common.getProtectOption"] = _resp_common_getProtectOption;
respMap["common.removeSecurityCode"] = _resp_common_removeSecurityCode;
respMap["common.setProtectOption"] = _resp_common_setProtectOption;
respMap["common.setSecurityCode"] = _resp_common_setSecurityCode;
respMap["common.setUnlockOption"] = _resp_common_setUnlockOption;
respMap["field.getCastleFieldInfo"] = _resp_field_getCastleFieldInfo;
respMap["field.getOtherFieldInfo"] = _resp_field_getOtherFieldInfo;
respMap["field.giveUpField"] = _resp_field_giveUpField;
respMap["fortifications.accTroopProduce"] = _resp_fortifications_accTroopProduce;
respMap["fortifications.cancelFortificationProduce"] = _resp_fortifications_cancelFortificationProduce;
respMap["fortifications.destructWallProtect"] = _resp_fortifications_destructWallProtect;
respMap["fortifications.getFortificationsProduceList"] = _resp_fortifications_getFortificationsProduceList;
respMap["fortifications.getProduceQueue"] = _resp_fortifications_getProduceQueue;
respMap["fortifications.produceWallProtect"] = _resp_fortifications_produceWallProtect;
respMap["friend.addBlock"] = _resp_friend_addBlock;
respMap["friend.addFriend"] = _resp_friend_addFriend;
respMap["friend.deleteBlock"] = _resp_friend_deleteBlock;
respMap["friend.deleteFriend"] = _resp_friend_deleteFriend;
respMap["friend.isBlockMailPlayer"] = _resp_friend_isBlockMailPlayer;
respMap["furlough.cancelFurlought"] = _resp_furlough_cancelFurlought;
respMap["furlough.isFurlought"] = _resp_furlough_isFurlought;
respMap["gamemaster.addBuilding"] = _resp_gamemaster_addBuilding;
respMap["gamemaster.addHero"] = _resp_gamemaster_addHero;
respMap["gamemaster.addItems"] = _resp_gamemaster_addItems;
respMap["gamemaster.removeBuilding"] = _resp_gamemaster_removeBuilding;
respMap["gamemaster.removeHero"] = _resp_gamemaster_removeHero;
respMap["gamemaster.removeItem"] = _resp_gamemaster_removeItem;
respMap["gamemaster.setResources"] = _resp_gamemaster_setResources;
respMap["gamemaster.setTechnology"] = _resp_gamemaster_setTechnology;
respMap["hero.addPoint"] = _resp_hero_addPoint;
respMap["hero.awardGold"] = _resp_hero_awardGold;
respMap["hero.callBackHero"] = _resp_hero_callBackHero;
respMap["hero.changeName"] = _resp_hero_changeName;
respMap["hero.dischargeChief"] = _resp_hero_dischargeChief;
respMap["hero.fireHero"] = _resp_hero_fireHero;
respMap["hero.getHerosListFromTavern"] = _resp_hero_getHerosListFromTavern;
respMap["hero.hireHero"] = _resp_hero_hireHero;
respMap["hero.levelUp"] = _resp_hero_levelUp;
respMap["hero.promoteToChief"] = _resp_hero_promoteToChief;
respMap["hero.refreshHerosListFromTavern"] = _resp_hero_refreshHerosListFromTavern;
respMap["hero.releaseHero"] = _resp_hero_releaseHero;
respMap["hero.resetPoint"] = _resp_hero_resetPoint;
respMap["hero.tryGetSeizedHero"] = _resp_hero_tryGetSeizedHero;
respMap["hero.useItem"] = _resp_hero_useItem;
respMap["interior.getResourceProduceData"] = _resp_interior_getResourceProduceData;
respMap["interior.modifyCommenceRate"] = _resp_interior_modifyCommenceRate;
respMap["interior.modifyTaxRate"] = _resp_interior_modifyTaxRate;
respMap["interior.pacifyPeople"] = _resp_interior_pacifyPeople;
respMap["interior.taxation"] = _resp_interior_taxation;
respMap["mail.deleteMail"] = _resp_mail_deleteMail;
respMap["mail.getAllTVMsg"] = _resp_mail_getAllTVMsg;
respMap["mail.readMail"] = _resp_mail_readMail;
respMap["mail.readOverMailList"] = _resp_mail_readOverMailList;
respMap["mail.receiveMailList"] = _resp_mail_receiveMailList;
respMap["mail.reportBug"] = _resp_mail_reportBug;
respMap["mail.reportPlayer"] = _resp_mail_reportPlayer;
respMap["mail.sendMail"] = _resp_mail_sendMail;
respMap["quest.award"] = _resp_quest_award;
respMap["quest.awardPacket"] = _resp_quest_awardPacket;
respMap["quest.getAwardItems"] = _resp_quest_getAwardItems;
respMap["quest.getQuestList"] = _resp_quest_getQuestList;
respMap["quest.getQuestType"] = _resp_quest_getQuestType;
respMap["rank.getAllianceRank"] = _resp_rank_getAllianceRank;
respMap["rank.getCastleRank"] = _resp_rank_getCastleRank;
respMap["rank.getHeroRank"] = _resp_rank_getHeroRank;
respMap["rank.getPlayerRank"] = _resp_rank_getPlayerRank;
respMap["report.deleteReport"] = _resp_report_deleteReport;
respMap["report.markAsRead"] = _resp_report_markAsRead;
respMap["report.readOverReport"] = _resp_report_readOverReport;
respMap["report.receiveReportList"] = _resp_report_receiveReportList;
respMap["shop.buy"] = _resp_shop_buy;
respMap["shop.buyResource"] = _resp_shop_buyResource;
respMap["shop.getBuyResourceInfo"] = _resp_shop_getBuyResourceInfo;
respMap["shop.useCastleGoods"] = _resp_shop_useCastleGoods;
respMap["shop.useGoods"] = _resp_shop_useGoods;
respMap["stratagem.useStratagem"] = _resp_stratagem_useStratagem;
respMap["tech.cancelResearch"] = _resp_tech_cancelResearch;
respMap["tech.getCoinsNeed"] = _resp_tech_getCoinsNeed;
respMap["tech.getResearchList"] = _resp_tech_getResearchList;
respMap["tech.research"] = _resp_tech_research;
respMap["tech.speedUpResearch"] = _resp_tech_speedUpResearch;
respMap["trade.cancelTrade"] = _resp_trade_cancelTrade;
respMap["trade.getMyTradeList"] = _resp_trade_getMyTradeList;
respMap["trade.getTransingTradeList"] = _resp_trade_getTransingTradeList;
respMap["trade.newTrade"] = _resp_trade_newTrade;
respMap["trade.searchTrades"] = _resp_trade_searchTrades;
respMap["trade.speedUpTrans"] = _resp_trade_speedUpTrans;
respMap["troop.accTroopProduce"] = _resp_troop_accTroopProduce;
respMap["troop.cancelTroopProduce"] = _resp_troop_cancelTroopProduce;
respMap["troop.checkIdleBarrack"] = _resp_troop_checkIdleBarrack;
respMap["troop.disbandTroop"] = _resp_troop_disbandTroop;
respMap["troop.getProduceQueue"] = _resp_troop_getProduceQueue;
respMap["troop.getTroopProduceList"] = _resp_troop_getTroopProduceList;
respMap["troop.produceTroop"] = _resp_troop_produceTroop;
respMap["truce.cancelDreamTruce"] = _resp_truce_cancelDreamTruce;
respMap["truce.changeDreamTruceTime"] = _resp_truce_changeDreamTruceTime;
respMap["truce.setDreamTruce"] = _resp_truce_setDreamTruce;
respMap["server.AllianceChatMsg"] = _resp_AllianceChatMsg;
respMap["server.BuildComplate"] = _resp_BuildComplate;
respMap["server.BuildingQueueUpdate"] = _resp_BuildingQueueUpdate;
respMap["server.CastleFieldUpdate"] = _resp_CastleFieldUpdate;
respMap["server.CastleUpdate"] = _resp_CastleUpdate;
respMap["server.CbUpdate"] = _resp_CbUpdate;
respMap["server.ChangeName"] = _resp_ChangeName;
respMap["server.ChannelChatMsg"] = _resp_ChannelChatMsg;
respMap["server.ConnectionLost"] = _resp_ConnectionLost;
respMap["server.EnemyArmysUpdate"] = _resp_EnemyArmysUpdate;
respMap["server.FortificationsUpdate"] = _resp_FortificationsUpdate;
respMap["server.FriendArmysUpdate"] = _resp_FriendArmysUpdate;
respMap["server.HeroUpdate"] = _resp_HeroUpdate;
respMap["server.InjuredTroopUpdate"] = _resp_InjuredTroopUpdate;
respMap["server.ItemBuff"] = _resp_ItemBuff;
respMap["server.ItemUpdate"] = _resp_ItemUpdate;
respMap["server.KickedOut"] = _resp_KickedOut;
respMap["server.LoginResponse"] = _resp_LoginResponse;
respMap["server.NewFinishedQuest"] = _resp_NewFinishedQuest;
respMap["server.NewMail"] = _resp_NewMail;
respMap["server.NewReport"] = _resp_NewReport;
respMap["server.OnlineBonus"] = _resp_OnlineBonus;
respMap["server.PackageList"] = _resp_PackageList;
respMap["server.PlayerBuffUpdate"] = _resp_PlayerBuffUpdate;
respMap["server.PlayerInfoUpdate"] = _resp_PlayerInfoUpdate;
respMap["server.PrivateChatMessage"] = _resp_PrivateChatMessage;
respMap["server.QuestFinished"] = _resp_QuestFinished;
respMap["server.RegisterResponse"] = _resp_RegisterResponse;
respMap["server.ResearchCompleteUpdate"] = _resp_ResearchCompleteUpdate;
respMap["server.ResourceUpdate"] = _resp_ResourceUpdate;
respMap["server.SecurityCodeRemovedEvent"] = _resp_SecurityCodeRemovedEvent;
respMap["server.SelfArmysUpdate"] = _resp_SelfArmysUpdate;
respMap["server.StrategyBuff"] = _resp_StrategyBuff;
respMap["server.SystemInfoMsg"] = _resp_SystemInfoMsg;
respMap["server.TradesUpdate"] = _resp_TradesUpdate;
respMap["server.TransingTradeUpdate"] = _resp_TransingTradeUpdate;
respMap["server.TroopUpdate"] = _resp_TroopUpdate;
respMap["server.WorldChatMsg"] = _resp_WorldChatMsg;
*/
