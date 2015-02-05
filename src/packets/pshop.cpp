//
// pshop.cpp
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

#include "pshop.h"
#include "../Server.h"
#include "../Client.h"

pshop::pshop(Server * server, request & req, amf3object & obj)
	: packet(server, req, obj)
{

}

pshop::~pshop()
{

}

void pshop::process()
{
	obj2["data"] = amf3object();
	amf3object & data2 = obj2["data"];

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
		city->m_resources.food += fooduse * 100000;
		city->m_resources.wood += wooduse * 100000;
		city->m_resources.stone += stoneuse * 50000;
		city->m_resources.iron += ironuse * 40000;

		obj2["cmd"] = "shop.buyResource";
		data2["packageId"] = 0.0f;
		data2["ok"] = 1;

		client->PlayerUpdate();
		city->ResourceUpdate();

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

void pshop::ShopUseGoods(amf3object & data, Client * client)
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
/*
	["data"] Type: Object - Value : Object
		["alliance"] Type : Boolean - Value : False
		["tV"] Type : Boolean - Value : True
		["msg"] Type : String - Value : Lord <font color = '#FF0000'><b><u>LEGION57< / u>< / b>< / font> gained <b> < font color = '#00A2FF'>1 Ivory Horn< / font>< / b>(worth <b> < font color = '#FF0000'>60 < / font > < / b> Cents) from <b><font color = '#FF0000'>Amulet< / font>< / b>!
		["noSenderSystemInfo"] Type : Boolean - Value : False


	["data"] Type : Object - Value : Object
		["alliance"] Type : Boolean - Value : False
		["tV"] Type : Boolean - Value : False
		["msg"] Type : String - Value : Lord <font color = '#FF0000'><b><u>LEGION57< / u>< / b>< / font> gained <b> < font color = '#00A2FF'>1 Ivory Horn< / font>< / b>(worth <b> < font color = '#FF0000'>60 < / font > < / b> Cents) from <b><font color = '#FF0000'>Amulet< / font>< / b>!
		["noSenderSystemInfo"] Type : Boolean - Value : True*/


}
int32_t pshop::GetGambleCount(string item)
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
amf3object pshop::GenerateGamble()
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

