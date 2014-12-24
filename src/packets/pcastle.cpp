//
// pcastle.cpp
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

#include "pcastle.h"
#include "../Server.h"
#include "../City.h"
#include "../Client.h"
#include "../Hero.h"


pcastle::pcastle(Server * server, request & req, amf3object & obj)
	: packet(server, req, obj)
{

}

pcastle::~pcastle()
{

}

void pcastle::process()
{
	obj2["data"] = amf3object();
	amf3object & data2 = obj2["data"];

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
			if (gserver->m_buildingconfig[i][0].limit > 0 && gserver->m_buildingconfig[i][0].limit <= city->GetBuildingCount(i))
				continue;
			if (gserver->m_buildingconfig[i][0].time > 0)
			{
				amf3object parent;
				amf3object conditionbean;

				double costtime = gserver->m_buildingconfig[i][0].time * 1000;
				double mayorinf = 1;
				if (city->m_mayor)
					mayorinf = pow(0.995, city->m_mayor->GetManagement());
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
						int temp = city->GetBuildingLevel(req.id);
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
			if (gserver->m_buildingconfig[i][0].limit > 0 && gserver->m_buildingconfig[i][0].limit <= city->GetBuildingCount(i))
				continue;
			if (gserver->m_buildingconfig[i][0].time > 0)
			{
				amf3object parent;
				amf3object conditionbean;

				double costtime = gserver->m_buildingconfig[i][0].time;//*1000;
				double mayorinf = 1;
				if (city->m_mayor)
					mayorinf = pow(0.995, city->m_mayor->GetManagement());
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
						int temp = city->GetBuildingLevel(req.id);
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
			if (gserver->m_buildingconfig[i][0].limit > 0 && gserver->m_buildingconfig[i][0].limit <= city->GetBuildingCount(i))
				continue;
			if (gserver->m_buildingconfig[i][0].time > 0)
			{
				amf3object parent;
				amf3object conditionbean;

				double costtime = gserver->m_buildingconfig[i][0].time * 1000;
				double mayorinf = 1;
				if (city->m_mayor)
					mayorinf = pow(0.995, city->m_mayor->GetManagement());
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
						int temp = city->GetBuildingLevel(req.id);
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

		city->CalculateResources();
		city->CalculateStats();

		if (((positionid < -2) || (positionid > 31)) && ((positionid < 1001) || (positionid > 1040)))
		{
			gserver->SendObject(client, gserver->CreateError("castle.newBuilding", -99, "Can't build building."));
			return;
		}

		if ((buildingtype > 34 || buildingtype <= 0) || city->GetBuilding(positionid)->type || ((gserver->m_buildingconfig[buildingtype][0].limit > 0) && (gserver->m_buildingconfig[buildingtype][0].limit <= city->GetBuildingCount(buildingtype))))
		{
			gserver->SendObject(client, gserver->CreateError("castle.newBuilding", -99, "Can't build building."));
			return;
		}

		for (int i = 0; i < 35; ++i)
		{
			if (city->m_innerbuildings[i].status != 0)
			{
				// TODO: Support hammer item for multiple constructions at once
				gserver->SendObject(client, gserver->CreateError("castle.newBuilding", -48, "One building allowed to be built at a time."));
				return;
			}
		}
		for (int i = 0; i < 41; ++i)
		{
			if (city->m_outerbuildings[i].status != 0)
			{
				// TODO: Support hammer item for multiple constructions at once
				gserver->SendObject(client, gserver->CreateError("castle.newBuilding", -48, "One building allowed to be built at a time."));
				return;
			}
		}

		if (!city->CheckBuildingPrereqs(buildingtype, 0))
		{
			gserver->SendObject(client, gserver->CreateError("castle.newBuilding", -99, "Building Prerequisites not met."));
			return;
		}

		if ((gserver->m_buildingconfig[buildingtype][0].food > city->m_resources.food)
			|| (gserver->m_buildingconfig[buildingtype][0].wood > city->m_resources.wood)
			|| (gserver->m_buildingconfig[buildingtype][0].stone > city->m_resources.stone)
			|| (gserver->m_buildingconfig[buildingtype][0].iron > city->m_resources.iron)
			|| (gserver->m_buildingconfig[buildingtype][0].gold > city->m_resources.gold))
		{
			gserver->SendObject(client, gserver->CreateError("castle.newBuilding", -1, "Not enough resources."));
			return;
		}
		data2["ok"] = 1;
		data2["packageId"] = 0.0f;

		gserver->SendObject(client, obj2);

		city->m_resources.food -= gserver->m_buildingconfig[buildingtype][0].food;
		city->m_resources.wood -= gserver->m_buildingconfig[buildingtype][0].wood;
		city->m_resources.stone -= gserver->m_buildingconfig[buildingtype][0].stone;
		city->m_resources.iron -= gserver->m_buildingconfig[buildingtype][0].iron;
		city->m_resources.gold -= gserver->m_buildingconfig[buildingtype][0].gold;


		MULTILOCK(M_CASTLELIST, M_TIMEDLIST);

		stBuildingAction * ba = new stBuildingAction;

		double costtime = gserver->m_buildingconfig[buildingtype][0].time;
		double mayorinf = 1;
		if (city->m_mayor)
			mayorinf = pow(0.995, city->m_mayor->GetManagement());
		costtime = 1000 * (costtime)* (mayorinf)* (pow(0.9, client->GetResearchLevel(T_CONSTRUCTION)));

		city->SetBuilding(buildingtype, 0, positionid, 1, timestamp, timestamp + floor(costtime));

		obj2["cmd"] = "server.BuildComplate";

		data2["buildingBean"] = city->GetBuilding(positionid)->ToObject();
		data2["castleId"] = client->m_currentcityid;

		gserver->SendObject(client, obj2);

		city->ResourceUpdate();

		client->SaveToDB();
		city->SaveToDB();

		stTimedEvent te;
		ba->city = city;
		ba->client = client;
		ba->positionid = positionid;
		te.data = ba;
		te.accountid = client->m_accountid;
		te.castleid = city->m_castleid;
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
		stBuilding * bldg = city->GetBuilding(positionid);

		city->CalculateResources();
		city->CalculateStats();


		if ((bldg->type > 34 || bldg->type <= 0) || (bldg->level == 0))
		{
			gserver->SendObject(client, gserver->CreateError("castle.destructBuilding", -99, "Can't destroy building."));
			return;
		}


		for (int i = 0; i < 35; ++i)
		{
			if (city->m_innerbuildings[i].status != 0)
			{
				gserver->SendObject(client, gserver->CreateError("castle.destructBuilding", -48, "One building allowed to be built at a time."));
				return;
			}
		}
		for (int i = 0; i < 41; ++i)
		{
			if (city->m_outerbuildings[i].status != 0)
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
		ba->city = city;
		ba->client = client;
		ba->positionid = positionid;
		te.data = ba;
		te.type = DEF_TIMEDBUILDING;

		gserver->AddTimedEvent(te);

		double costtime = gserver->m_buildingconfig[bldg->type][bldg->level - 1].destructtime * 1000;
		double mayorinf = 1;
		if (city->m_mayor)
			mayorinf = pow(0.995, city->m_mayor->GetManagement());
		costtime = (costtime)* (mayorinf)* (pow(0.9, client->GetResearchLevel(T_CONSTRUCTION)));

		ba->city->SetBuilding(bldg->type, bldg->level, positionid, 2, timestamp, timestamp + floor(costtime));

		obj2["cmd"] = "server.BuildComplate";

		data2["buildingBean"] = ba->city->GetBuilding(positionid)->ToObject();
		data2["castleId"] = client->m_currentcityid;

		gserver->SendObject(client, obj2);

		city->ResourceUpdate();

		client->SaveToDB();
		city->SaveToDB();

		UNLOCK(M_CASTLELIST);
		UNLOCK(M_TIMEDLIST);
		return;
	}
	if ((command == "upgradeBuilding")) //TODO implement hammer queue system
	{
		VERIFYCASTLEID();
		CHECKCASTLEID();

		city->CalculateResources();
		city->CalculateStats();

		int positionid = data["positionId"];
		stBuilding * bldg = city->GetBuilding(positionid);
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
			if (city->m_innerbuildings[i].status != 0)
			{
				gserver->SendObject(client, gserver->CreateError("castle.upgradeBuilding", -48, "One building allowed to be built at a time."));
				return;
			}
		}
		for (int i = 0; i < 41; ++i)
		{
			if (city->m_outerbuildings[i].status != 0)
			{
				gserver->SendObject(client, gserver->CreateError("castle.upgradeBuilding", -48, "One building allowed to be built at a time."));
				return;
			}
		}

		if (!city->CheckBuildingPrereqs(buildingtype, buildinglevel))
		{
			gserver->SendObject(client, gserver->CreateError("castle.upgradeBuilding", -99, "Building Prerequisites not met."));
			return;
		}

		if ((gserver->m_buildingconfig[buildingtype][buildinglevel].food > city->m_resources.food)
			|| (gserver->m_buildingconfig[buildingtype][buildinglevel].wood > city->m_resources.wood)
			|| (gserver->m_buildingconfig[buildingtype][buildinglevel].stone > city->m_resources.stone)
			|| (gserver->m_buildingconfig[buildingtype][buildinglevel].iron > city->m_resources.iron)
			|| (gserver->m_buildingconfig[buildingtype][buildinglevel].gold > city->m_resources.gold))
		{
			gserver->SendObject(client, gserver->CreateError("castle.upgradeBuilding", -1, "Not enough resources."));
			return;
		}


		gserver->SendObject(client, obj2);

		city->m_resources.food -= gserver->m_buildingconfig[buildingtype][buildinglevel].food;
		city->m_resources.wood -= gserver->m_buildingconfig[buildingtype][buildinglevel].wood;
		city->m_resources.stone -= gserver->m_buildingconfig[buildingtype][buildinglevel].stone;
		city->m_resources.iron -= gserver->m_buildingconfig[buildingtype][buildinglevel].iron;
		city->m_resources.gold -= gserver->m_buildingconfig[buildingtype][buildinglevel].gold;

		MULTILOCK(M_CASTLELIST, M_TIMEDLIST);
		stBuildingAction * ba = new stBuildingAction;

		stTimedEvent te;
		ba->city = city;
		ba->client = client;
		ba->positionid = positionid;
		te.data = ba;
		te.type = DEF_TIMEDBUILDING;

		gserver->AddTimedEvent(te);

		double costtime = gserver->m_buildingconfig[buildingtype][buildinglevel].time;
		double mayorinf = 1;
		if (city->m_mayor)
			mayorinf = pow(0.995, city->m_mayor->GetManagement());
		costtime = (costtime)* (mayorinf)* (pow(0.9, client->GetResearchLevel(T_CONSTRUCTION)));

		city->ResourceUpdate();

		obj2["cmd"] = "server.BuildComplate";

		ba->city->SetBuilding(buildingtype, buildinglevel, positionid, 1, timestamp, timestamp + (floor(costtime) * 1000));

		data2["buildingBean"] = ba->city->GetBuilding(positionid)->ToObject();
		data2["castleId"] = client->m_currentcityid;

		gserver->SendObject(client, obj2);

		client->SaveToDB();
		city->SaveToDB();

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

		int level = city->GetBuilding(tileposition)->level;
		int id = city->GetBuilding(tileposition)->type;

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
		if (city->m_mayor)
			mayorinf = pow(0.995, city->m_mayor->GetManagement());
		costtime = (costtime)* (mayorinf)* (pow(0.9, client->GetResearchLevel(T_CONSTRUCTION)));

		double desttime = gserver->m_buildingconfig[id][level].destructtime;
		mayorinf = 1;
		if (city->m_mayor)
			mayorinf = pow(0.995, city->m_mayor->GetManagement());
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
				int temp = city->GetBuildingLevel(req.id);
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

			stBuilding * building = city->GetBuilding(positionid);

			if ((building->endtime - building->starttime) <= 5 * 60 * 1000)
			{
				//under 5 mins
				obj2["cmd"] = "castle.speedUpBuildCommand";
				data2["packageId"] = 0.0f;
				data2["ok"] = 1;

				gserver->SendObject(client, obj2);

				building->endtime -= 5 * 60 * 1000;

				client->SaveToDB();
				city->SaveToDB();
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

			stBuilding * building = city->GetBuilding(positionid);
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

			city->SetBuilding(building->type, building->level, building->id, building->status, building->starttime, building->endtime);

			obj2["cmd"] = "server.BuildComplate";

			data2["buildingBean"] = city->GetBuilding(positionid)->ToObject();
			data2["castleId"] = client->m_currentcityid;

			gserver->SendObject(client, obj2);

			client->SaveToDB();
			city->SaveToDB();

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
		{
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
							city->m_resources += res;
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
						city->SaveToDB();

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
		}
		UNLOCK(M_TIMEDLIST);
		return;
	}
}



