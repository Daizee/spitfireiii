//
// structs.h
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

#pragma once

#include "amf3.h"
#include <stdint.h>

extern char * GetBuildingName(int id);

class Hero;
class City;
class PlayerCity;
class NpcCity;
class Client;
class Alliance;

#pragma region structs and stuff

struct stResources
{
	double gold;
	double food;
	double wood;
	double stone;
	double iron;
	stResources(double gold, double food, double wood, double stone, double iron)
	{
		this->food = food;
		this->gold = gold;
		this->wood = wood;
		this->iron = iron;
		this->stone = stone;
	}
	stResources() { gold = food = wood = stone = iron = 0; }
	stResources& operator -=(const stResources& b)
	{
		this->gold -= b.gold;
		this->food -= b.food;
		this->wood -= b.wood;
		this->stone -= b.stone;
		this->iron -= b.iron;
		return *this;
	}
	stResources& operator +=(const stResources& b)
	{
		this->gold += b.gold;
		this->food += b.food;
		this->wood += b.wood;
		this->stone += b.stone;
		this->iron += b.iron;
		return *this;
	}
	amf3object ToObject()
	{
		amf3object obj = amf3object();
		obj["wood"] = wood;
		obj["food"] = food;
		obj["stone"] = stone;
		obj["gold"] = gold;
		obj["iron"] = iron;
		return obj;
	}
}; // 40 bytes
struct stForts
{
	stForts() { traps = abatis = towers = logs = trebs = 0; }
	int32_t traps;
	int32_t abatis;
	int32_t towers;
	int32_t logs;
	int32_t trebs;
}; // 20 bytes
struct stBuilding
{
	stBuilding() { id = level = type = status = 0; endtime = starttime = 0.0f; }
	int16_t id;
	int16_t level;
	//short appearance; //age2
	int16_t type;
	int16_t status;
	double endtime;
	double starttime;
	//short help; //age2
	//string name;
	amf3object ToObject()
	{
		amf3object obj;
		obj["startTime"] = starttime;
		obj["endTime"] = endtime;//(endtime > 0)?(endtime-1000):(0);// HACK: Attempt to correct "lag" issues.
		obj["level"] = level;
		obj["status"] = status;
		obj["typeId"] = type;
		obj["positionId"] = id;
		obj["name"] = GetBuildingName(type);
		return obj;
	};
}; // 21 bytes
struct stPrereq
{
	int32_t id;
	int32_t level;
};
struct stItemConfig
{
	stItemConfig() { cangamble = buyable = false; cost = saleprice = daylimit = type = rarity = 0; }
	string name;
	int32_t cost;
	int32_t saleprice;
	int32_t daylimit;
	int32_t type;
	bool cangamble;
	bool buyable;
	int32_t rarity;
};
struct stRarityGamble
{
	std::vector<stItemConfig*> common;
	std::vector<stItemConfig*> special;
	std::vector<stItemConfig*> rare;
	std::vector<stItemConfig*> superrare;
	std::vector<stItemConfig*> ultrarare;
};
struct stBuildingConfig
{
	int32_t food;
	int32_t wood;
	int32_t iron;
	int32_t stone;
	int32_t gold;
	int32_t population;
	double time;
	double destructtime;
	std::vector<stPrereq> buildings;
	std::vector<stPrereq> techs;
	std::vector<stPrereq> items;
	int32_t limit;
	int32_t inside;
	int32_t prestige;
};
struct stMarketEntry
{
	double amount;
	double price;
	uint32_t clientid;
	uint32_t cityid;
};
struct stTroop
{
	stTroop(uint64_t count) { this->count = startcount = count; }
	uint64_t startcount;
	uint64_t count;
	uint64_t maxlife;
	uint64_t life;
	uint64_t attack;
	uint64_t DamageDealt()
	{
		return uint64_t((double(life) / double(maxlife))*count);
	}
};
struct stTroops
{
	int64_t worker;
	int64_t warrior;
	int64_t scout;
	int64_t pike;
	int64_t sword;
	int64_t archer;
	int64_t cavalry;
	int64_t cataphract;
	int64_t transporter;
	int64_t ballista;
	int64_t ram;
	int64_t catapult;
	stTroops() { memset(&worker, 0, 8 * 12); }
	stTroops& operator -=(const stTroops& b)
	{
		this->worker -= b.worker;
		this->warrior -= b.warrior;
		this->scout -= b.scout;
		this->pike -= b.pike;
		this->sword -= b.sword;
		this->archer -= b.archer;
		this->cavalry -= b.cavalry;
		this->cataphract -= b.cataphract;
		this->transporter -= b.transporter;
		this->ballista -= b.ballista;
		this->ram -= b.ram;
		this->catapult -= b.catapult;
		return *this;
	}
	stTroops& operator +=(const stTroops& b)
	{
		this->worker += b.worker;
		this->warrior += b.warrior;
		this->scout += b.scout;
		this->pike += b.pike;
		this->sword += b.sword;
		this->archer += b.archer;
		this->cavalry += b.cavalry;
		this->cataphract += b.cataphract;
		this->transporter += b.transporter;
		this->ballista += b.ballista;
		this->ram += b.ram;
		this->catapult += b.catapult;
		return *this;
	}
	amf3object ToObject()
	{
		amf3object obj = amf3object();
		obj["peasants"] = worker;
		obj["catapult"] = catapult;
		obj["archer"] = archer;
		obj["ballista"] = ballista;
		obj["scouter"] = scout;
		obj["carriage"] = transporter;
		obj["heavyCavalry"] = cataphract;
		obj["militia"] = warrior;
		obj["lightCavalry"] = cavalry;
		obj["swordsmen"] = sword;
		obj["pikemen"] = pike;
		obj["batteringRam"] = ram;
		return obj;
	}
};
struct stAlliance
{
	int32_t id;
	union
	{
		int32_t members;
		double honor;
		double prestige;
	};
	uint32_t rank;
	Alliance * ref;
};
struct stArmyMovement
{
	stArmyMovement() { memset(&resources, 0, sizeof(stResources)); memset(&troops, 0, sizeof(stTroops)); }
	Hero * hero;
	string heroname;
	int16_t direction;//1 from city - 2 back to city
	stResources resources;
	string startposname;
	string king;
	stTroops troops;
	uint64_t starttime;
	uint64_t armyid;
	uint64_t reachtime;
	uint32_t herolevel;
	uint64_t resttime;
	uint32_t missiontype;
	uint32_t startfieldid;
	uint32_t targetfieldid;
	string targetposname;
	City * city;
	Client * client;
	amf3object ToObject()
	{
		amf3object obj = amf3object();
		obj["hero"] = heroname;
		obj["direction"] = direction;
		obj["resource"] = resources.ToObject();
		obj["startPosName"] = startposname;
		obj["king"] = king;
		obj["troop"] = troops.ToObject();
		obj["startTime"] = starttime;
		obj["armyId"] = armyid;
		obj["reachTime"] = reachtime;
		obj["heroLevel"] = herolevel;
		obj["restTime"] = resttime;
		obj["missionType"] = missiontype;
		obj["startFieldId"] = startfieldid;
		obj["targetFieldId"] = targetfieldid;
		obj["targetPosName"] = targetposname;
		return obj;
	}
};
struct stTrainingAction
{
	int32_t troopcount;
	int8_t trooptype;
	int8_t buildingid;
	Client * client;
};
struct stBuildingAction
{
	PlayerCity * city;
	Client * client;
	int16_t positionid;
};
struct stResearchAction
{
	PlayerCity * city;
	Client * client;
	int16_t researchid;
};
struct stTimedEvent
{
	int8_t type;
	int64_t id;
	int64_t accountid;
	int64_t castleid;
	void * data;
};
struct stIntRank
{
	uint32_t value;
	uint32_t id;
};
struct stClientRank
{
	Client * client;
	int32_t rank;
};
struct stHeroRank
{
	int16_t stratagem;
	int16_t power;
	int16_t management;
	int16_t grade;
	string name;
	string kind;
	int32_t rank;
};
struct stCastleRank
{
	int16_t level;
	int32_t population;
	string name;
	string grade;
	string alliance;
	string kind;
	int32_t rank;
};
struct stSearchClientRank
{
	std::list<stClientRank> ranklist;
	std::list<stClientRank> * rlist;
	string key;
	uint64_t lastaccess;
};
struct stSearchHeroRank
{
	std::list<stHeroRank> ranklist;
	std::list<stHeroRank> * rlist;
	string key;
	uint64_t lastaccess;
};
struct stSearchCastleRank
{
	std::list<stCastleRank> ranklist;
	std::list<stCastleRank> * rlist;
	string key;
	uint64_t lastaccess;
};
struct stSearchAllianceRank
{
	std::list<stAlliance> ranklist;
	std::list<stAlliance> * rlist;
	string key;
	uint64_t lastaccess;
};
struct stPacketOut
{
	int32_t client;
	amf3object obj;
};
struct stTroopTrain
{
	int16_t troopid;
	int32_t count;
	int32_t queueid;
	double starttime;
	double endtime;
	double costtime;
};
struct stTroopQueue
{
	int8_t status;
	int16_t positionid;
	int32_t nextqueueid;
	std::list<stTroopTrain> queue;
};
struct stBuff
{
	string id;
	string desc;
	double endtime;
};
struct stItem
{
	string id;
	string name;
	int16_t count;
	int16_t mincount;
	int16_t maxcount;
};
struct stResearch
{
	int16_t level;
	uint32_t castleid;
	double endtime;
	double starttime;
};

struct stPackage
{
	string desc;
	string name;
	int32_t status;//4 = unclaimed? 2 = claimed?
	int32_t type;//5 = ?
	int32_t minmedal;
	int32_t maxmedal;
	int32_t id;
	int32_t isusenow;
	int32_t scores;
	int64_t providetime;
	int64_t expiredtime;
	string claimedtime;
	std::vector<stItem> items;
};

struct stCastleSign
{
	uint64_t tileid;
	uint64_t id;//castle db id
	string name;
	int32_t x;
	int32_t y;
};
#pragma endregion



