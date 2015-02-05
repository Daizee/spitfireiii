//
// parmy.cpp
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

#include "parmy.h"
#include "../Server.h"
#include "../Client.h"
#include "../City.h"
#include "../Hero.h"
#include "../Map.h"
#include "../Tile.h"
#include "../AllianceCore.h"


parmy::parmy(Server * server, request & req, amf3object & obj)
	: packet(server, req, obj)
{

}

parmy::~parmy()
{

}

void parmy::process()
{
	obj2["data"] = amf3object();
	amf3object & data2 = obj2["data"];

	if (command == "setArmyGoOut")
	{
		VERIFYCASTLEID();
		CHECKCASTLEID();

		client->GetFocusCity()->m_gooutforbattle = data["isArmyGoOut"];
		obj2["cmd"] = "army.setArmyGoOut";
		data2["packageId"] = 0.0f;
		data2["ok"] = 1;
		gserver->SendObject(client, obj2);
	}
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
		PlayerCity * city = client->GetFocusCity();
		int16_t rally = city->GetBuildingLevel(B_RALLYSPOT);
		if (rally < city->armymovement.size())
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
				if (!city->HasTroops(troops))
				{
					//not enough troops
					gserver->SendObject(client, gserver->CreateError("army.newArmy", -29, "You have insufficient troops."));
					return;
				}
				//you have enough troops, valid target, gogogo

				//check hero existence and status
				Hero * hero = city->GetHero(heroid);
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
				am->city = city;
				am->client = client;

				am->hero = hero;
				am->heroname = hero->m_name;
				am->direction = 1;
				am->resources += resources;
				am->startposname = city->m_cityname;
				am->king = client->m_playername;
				am->troops += troops;
				am->starttime = unixtime();
				am->armyid = gserver->armycounter++;
				am->reachtime = am->starttime + gserver->CalcTroopSpeed(city, troops, city->m_tileid, targettile) * 1000;
				am->herolevel = hero->m_level;
				am->resttime = resttime;
				am->missiontype = missiontype;
				am->startfieldid = city->m_tileid;
				am->targetfieldid = targettile;
				am->targetposname = gserver->map->GetTileFromID(targettile)->GetName();

				te.data = am;
				te.type = DEF_TIMEDARMY;

				gserver->AddTimedEvent(te);

				client->armymovement.push_back(am);

				//timer made, remove troops from city
				city->m_troops -= troops;
				hero->movement = am;

				city->HeroUpdate(hero, 2);
				city->TroopUpdate();
				city->ResourceUpdate();
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

				city->SaveToDB();
				hero->SaveToDB();
			}
			else
			{
				//NPC owned
				//you CAN attack this target. set everything up to pass to timers.
				if (!city->HasTroops(troops))
				{
					//not enough troops
					gserver->SendObject(client, gserver->CreateError("army.newArmy", -29, "You have insufficient troops."));
					return;
				}
				//you have enough troops, valid target, gogogo

				//check hero existence and status
				Hero * hero = city->GetHero(heroid);
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
				am->city = city;
				am->client = client;

				am->hero = hero;
				am->heroname = hero->m_name;
				am->direction = 1;
				am->resources += resources;
				am->startposname = city->m_cityname;
				am->king = client->m_playername;
				am->troops += troops;
				am->starttime = unixtime();
				am->armyid = gserver->armycounter++;
				am->reachtime = am->starttime + gserver->CalcTroopSpeed(city, troops, city->m_tileid, targettile) * 1000;
				am->herolevel = hero->m_level;
				am->resttime = resttime;
				am->missiontype = missiontype;
				am->startfieldid = city->m_tileid;
				am->targetfieldid = targettile;
				am->targetposname = gserver->map->GetTileFromID(targettile)->GetName();

				te.data = am;
				te.type = DEF_TIMEDARMY;

				gserver->AddTimedEvent(te);

				client->armymovement.push_back(am);

				//timer made, remove troops from city
				city->m_troops -= troops;
				hero->movement = am;

				hero->m_status = DEF_HEROATTACK;

				city->HeroUpdate(hero, 2);
				city->TroopUpdate();
				city->ResourceUpdate();
				client->SelfArmyUpdate();

				amf3object obj3;
				obj3["cmd"] = "army.newArmy";
				obj3["data"] = amf3object();
				amf3object & data3 = obj3["data"];
				data3["packageId"] = 0.0f;
				data3["ok"] = 1;
				//armies in embassy from other players go here
				gserver->SendObject(client, obj3);

				city->SaveToDB();
				hero->SaveToDB();
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



