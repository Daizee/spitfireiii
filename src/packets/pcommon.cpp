//
// pcommon.cpp
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

#include "pcommon.h"
#include "../Server.h"
#include "../Client.h"
#include "../Alliance.h"
#include "../Map.h"
#include "../Tile.h"


pcommon::pcommon(Server * server, request & req, amf3object & obj)
	: packet(server, req, obj)
{

}

pcommon::~pcommon()
{

}

void pcommon::process()
{
	obj2["data"] = amf3object();
	amf3object & data2 = obj2["data"];

	if (command == "worldChat")
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
	if (command == "privateChat")
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
		try
		{
			obj2["data"] = gserver->map->GetTileRangeObject(req.conn->client_->m_clientnumber, x1, x2, y1, y2);
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
<itemEum id=\"player.key.bull\" name=\"Wooden Bull Opener\" itemType=\"\xE5\xAE\x9D\xE7\xAE\xB1\" dayLimit=\"0\" userLimit=\"0\" desc=\"You can use this key to open one Hollow Wooden Bull sent to you by your friends. If you don�t have any Hollow Wooden Bull, you should ask your friends to send you some!\" itemDesc=\"You can open any Hollow Wooden Bull your friends gave you with this key once.\" iconUrl=\"images/icon/shop/WoodenBullOpener.png\" price=\"0\"/>\
<itemEum id=\"player.running.shoes\" name=\"Extra-Fast Running Shoes\" itemType=\"\xE5\xAE\x9D\xE7\xAE\xB1\" dayLimit=\"0\" userLimit=\"0\" desc=\"A gift from your friends around Run with the Bulls. Use it to get 24 hours of 50% upkeep in ALL your cities any time from July 9th through July 13th. Extra-Fast Running Shoes is stackable (meaning if you already have this buff, using it again will add an additional 24 hours). Once July 14th comes, this item will expire if you haven't used it yet.\" itemDesc=\"Get a 24 hours 50% upkeep buff during July 9th and July 13th.\" iconUrl=\"images/icon/shop/RunningShoes.png\" price=\"0\" playerItem=\"true\"/>\
<itemEum id=\"player.box.test.1\" name=\"Test Item\" itemType=\"\xE5\xAE\x9D\xE7\xAE\xB1\" dayLimit=\"0\" userLimit=\"0\" desc=\"Includes: test items.\" itemDesc=\"This package exists as a test.\" iconUrl=\"images/items/chongzhidalibao.png\" price=\"10\" playerItem=\"true\"/>\
<itemEum id=\"alliance.ritual_of_pact.ultimate\" name=\"Ritual of Pact (Ultimate)\" itemType=\"\xE5\xAE\x9D\xE7\x89\xA9\" dayLimit=\"90\" userLimit=\"0\" desc=\"Ritual of Pact (Ultimate): member limit is 1,000; effective for 90 days; leeway period is 7 days.\" itemDesc=\"It allows alliance to increase member limit to 1,000 once applied, which is effective for 90 days, while multiple applications of this item can lengthen the effective period. Once the item effect is due, 7-day leeway is given to the alliance. During this time, new members are denied to be recruited to the alliance. If no further application of the item, the alliance disbands automatically once the 7-day leeway period passes.\" iconUrl=\"images/items/Ritual_of_Pact_Ultimate.png\" price=\"75\"/>\
<itemEum id=\"player.speak.bronze_publicity_ambassador.permanent\" name=\"Bronze Publicity Ambassador (Permanent)\" itemType=\"\xE5\xAE\x9D\xE7\x89\xA9\" dayLimit=\"3650\" userLimit=\"0\" desc=\"Effect of Bronze Publicity Ambassador (Permanent) can only be replaced by Silver Publicity Ambassador (Permanent) or Gold Publicity Ambassador (Permanent).\" itemDesc=\"Once you apply this item, a special bronze icon will be displayed in front of your name when you speak in the chat box. While this item functions permanently, multiple applications make no difference to the duration of the effective period. Its effect can be replaced by Silver Publicity Ambassador (Permanent) or Gold Publicity Ambassador (Permanent).\" iconUrl=\"images/items/Bronze_Publicity_Ambassador_Permanentb.png\" price=\"75\"/>\
<itemEum id=\"player.speak.bronze_publicity_ambassador.permanent.15\" name=\"Bronze Publicity Ambassador (15-day)\" itemType=\"\xE5\xAE\x9D\xE7\x89\xA9\" dayLimit=\"15\" userLimit=\"0\" desc=\"Once you apply this item, a special bronze icon will be displayed in front of your name when you speak in the chat box. It�s effective for 15 days, while multiple applications can lengthen the effective period. Its effect can be replaced by Bronze Publicity Ambassador (Permanent), Silver Publicity Ambassador (15-day), Silver Publicity Ambassador (Permanent), Gold Publicity Ambassador (15-day) or Gold Publicity Ambassador (Permanent).\" itemDesc=\"Once you apply this item, a special bronze icon will be displayed in front of your name when you speak in the chat box. It�s effective for 15 days, while multiple applications can lengthen the effective period. Its effect can be replaced by Bronze Publicity Ambassador (Permanent), Silver Publicity Ambassador (15-day), Silver Publicity Ambassador (Permanent), Gold Publicity Ambassador (15-day) or Gold Publicity Ambassador (Permanent).\" iconUrl=\"images/items/Bronze_Publicity_Ambassador_15b.png\" price=\"75\"/>\
<itemEum id=\"player.speak.gold_publicity_ambassador.15\" name=\"Gold Publicity Ambassador (15-day)\" itemType=\"\xE5\xAE\x9D\xE7\x89\xA9\" dayLimit=\"15\" userLimit=\"0\" desc=\"Once you apply this item, a special gold icon will be displayed in front of your name when you speak in the chat box. It�s effective for 15 days, while multiple applications can lengthen the effective period. Its effect can be replaced by Gold Publicity Ambassador (Permanent).\" itemDesc=\"Effect of Gold Publicity Ambassador (15-day) can only be replaced by Gold Publicity Ambassador (Permanent).\" iconUrl=\"images/items/gold_publicity_ambassador_15b.png\" price=\"75\"/>\
<itemEum id=\"player.speak.gold_publicity_ambassador.permanent\" name=\"Gold Publicity Ambassador (Permanent)\" itemType=\"\xE5\xAE\x9D\xE7\x89\xA9\" dayLimit=\"3650\" userLimit=\"0\" desc=\"Once you apply this item, a special gold icon will be displayed in front of your name when you speak in the chat box. While this item functions permanently, multiple applications make no difference to the duration of the effective period. \" itemDesc=\"You're the highest level Publicity Ambassador now.\" iconUrl=\"images/items/Gold_Publicity_Ambassador_Permanentb.png\" price=\"75\"/>\
<itemEum id=\"player.speak.silver_publicity_ambassador.15\" name=\"Silver Publicity Ambassador (15-day)\" itemType=\"\xE5\xAE\x9D\xE7\x89\xA9\" dayLimit=\"15\" userLimit=\"0\" desc=\"Effect of Silver Publicity Ambassador (15-day) can only be replaced by Silver Publicity Ambassador (Permanent), Gold Publicity Ambassador (15-day) or Gold Publicity Ambassador (Permanent).\" itemDesc=\"Once you apply this item, a special silver icon will be displayed in front of your name when you speak in the chat box. It�s effective for 15 days, while multiple applications can lengthen the effective period. Its effect can be replaced by Silver Publicity Ambassador (Permanent), Gold Publicity Ambassador (15-day) or Gold Publicity Ambassador (Permanent).\" iconUrl=\"images/items/Silver_Publicity_Ambassador_15b.png\" price=\"75\"/>\
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


