//
// ptech.cpp
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

#include "ptech.h"
#include "../Server.h"
#include "../Client.h"
#include "../City.h"
#include "../Hero.h"

ptech::ptech(Server * server, request & req, amf3object & obj)
	: packet(server, req, obj)
{

}

ptech::~ptech()
{

}

void ptech::process()
{
	obj2["data"] = amf3object();
	amf3object & data2 = obj2["data"];

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
				if (city->m_mayor)
					mayorinf = pow(0.995, city->m_mayor->GetStratagem());

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
						int temp = city->GetBuildingLevel(req.id);
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
					int blevel = city->GetBuildingLevel(B_ACADEMY);//academy
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
					parent["avalevel"] = city->GetTechLevel(i);
					parent["permition"] = !city->m_researching;
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


		if (!city->m_researching)
		{
			if ((researchconfig->food > city->m_resources.food)
				|| (researchconfig->wood > city->m_resources.wood)
				|| (researchconfig->stone > city->m_resources.stone)
				|| (researchconfig->iron > city->m_resources.iron)
				|| (researchconfig->gold > city->m_resources.gold))
			{
				gserver->SendObject(client, gserver->CreateError("tech.research", -99, "Not enough resources."));
				return;
			}
			obj2["cmd"] = "tech.research";
			data2["ok"] = 1;
			data2["packageId"] = 0.0f;

			city->m_resources.food -= researchconfig->food;
			city->m_resources.wood -= researchconfig->wood;
			city->m_resources.stone -= researchconfig->stone;
			city->m_resources.iron -= researchconfig->iron;
			city->m_resources.gold -= researchconfig->gold;

			research->castleid = castleid;


			double costtime = researchconfig->time;
			double mayorinf = 1;
			if (city->m_mayor)
				mayorinf = pow(0.995, city->m_mayor->GetStratagem());

			costtime = (costtime)* (mayorinf);

			research->endtime = timestamp + floor(costtime) * 1000;

			research->starttime = timestamp;

			stResearchAction * ra = new stResearchAction;

			stTimedEvent te;
			ra->city = city;
			ra->client = client;
			ra->researchid = techid;
			te.data = ra;
			te.type = DEF_TIMEDRESEARCH;
			city->m_researching = true;

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
			parent["avalevel"] = city->GetTechLevel(techid);
			parent["upgradeing"] = (bool)(research->starttime != 0);
			parent["endTime"] = (research->endtime > 0) ? (research->endtime - client->m_lag) : (research->endtime);// HACK: attempt to fix "lag" issues
			parent["typeId"] = techid;
			parent["permition"] = !city->m_researching;

			data2["tech"] = parent;

			gserver->SendObject(client, obj2);

			city->ResourceUpdate();

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
		PlayerCity * city = client->GetCity(castleid);
		uint16_t techid = 0;

		for (int i = 0; i < 25; ++i)
		{
			if (client->m_research[i].castleid == castleid)
			{
				techid = i;
				break;
			}
		}

		if (!city || !city->m_researching || techid == 0)
		{
			gserver->SendObject(client, gserver->CreateError("tech.cancelResearch", -99, "Invalid city."));
			return;
		}

		stResearch * research;
		stBuildingConfig * researchconfig;


		research = &client->m_research[techid];
		researchconfig = &gserver->m_researchconfig[techid][research->level];


		if (city->m_researching)
		{
			obj2["cmd"] = "tech.cancelResearch";
			data2["ok"] = 1;
			data2["packageId"] = 0.0f;

			city->m_resources.food += double(researchconfig->food) / 3;
			city->m_resources.wood += double(researchconfig->wood) / 3;
			city->m_resources.stone += double(researchconfig->stone) / 3;
			city->m_resources.iron += double(researchconfig->iron) / 3;
			city->m_resources.gold += double(researchconfig->gold) / 3;

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

			city->m_researching = false;

			gserver->SendObject(client, obj2);

			city->ResourceUpdate();

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
			if (client->m_research[i].castleid == city->m_castleid)
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



