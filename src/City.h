//
// City.h
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

#include "funcs.h"
#include "structs.h"
#include <boost/format.hpp>

class Client;

char * GetBuildingName(int id);


class City
{
public:
	City();
	~City(void);

	bool SetBuilding(int16_t type, int16_t level, int16_t position, int16_t status = 0, double starttime = 0, double endtime = 0);
	void SetResources(double gold, double food, double wood, double stone, double iron);
	void SetMaxResources(double gold, double food, double wood, double stone, double iron);
	void SetForts(int32_t traps, int32_t abatis, int32_t towers, int32_t logs, int32_t trebs);
	int32_t GetForts(int32_t type);

	string m_cityname;
	int8_t m_status;
	int32_t m_tileid;
	int8_t m_type;
	int8_t m_level;

	int8_t m_loyalty;
	int8_t m_grievance;

	stResources m_resources;
	stResources m_maxresources;

	stForts m_forts;

	stBuilding m_innerbuildings[35]; // 735 bytes
	stBuilding m_outerbuildings[41]; // 840 bytes
	// 
	// 	public bool m_npc = true;
	// 	public Client m_client = null;
	// 	public int m_herocount = 0;
	// 	public ArrayList m_heroes = new ArrayList();
	// 	public bool m_allowAlliance = false;
	// 	//public ArrayList m_buildings = new ArrayList();
	// 	public Building[] m_buildings;
};

class NpcCity: public City
{
public:
	NpcCity();
	~NpcCity(void);
	void Initialize(bool resources, bool troops);
	void SetTroops(int32_t warrior, int32_t pike, int32_t sword, int32_t archer, int32_t cavalry);
	void SetupBuildings();
	void CalculateStats(bool resources, bool troops);

	struct stTroops
	{
		stTroops() { warrior = pike = sword = archer = cavalry = 0; }
		int32_t warrior;
		int32_t pike;
		int32_t sword;
		int32_t archer;
		int32_t cavalry;
	} m_troops, m_maxtroops;

	stForts m_maxforts;

	Hero * m_temphero;

	uint64_t m_calculatestuff;


	int32_t m_ownerid;
};

class PlayerCity: public City
{
public:
	PlayerCity();
	~PlayerCity(void);

	Client * m_client;
	uint32_t m_castleid;
	uint64_t m_castleinternalid;
	int64_t m_accountid;
	string m_logurl;
	int32_t m_population;
	uint32_t m_availablepopulation;
	uint32_t m_maxpopulation;
	bool m_allowalliance;
	bool m_gooutforbattle;
	bool m_hasenemy;
	double m_creation;
	bool m_researching;

	double m_lastcomfort;
	double m_lastlevy;

	stTroops m_troops, m_injuredtroops; // size 44 bytes

	stResources m_production;
	stResources m_workpopulation;
	stResources m_workrate;
	stResources m_storepercent;

	double m_productionefficiency;
	int32_t m_resourcebaseproduction;
	double m_resourcemanagement;
	int32_t m_resourcetech;
	int32_t m_resourcevalley;

	double m_troopconsume;

	struct stTimers
	{
		double updateresources;
	} m_timers;

	Hero * m_heroes[10]; // 75 bytes * 10
	Hero * m_innheroes[10]; // 75 bytes * 10

	Hero * m_mayor;

	string DBFortifications();
	string DBBuildings();
	string DBTransingtrades();
	string DBTrades();
	string DBTroops();
	string DBMisc();
	bool SaveToDB();

	amf3object ToObject();
	amf3array Buildings();
	amf3object Troops();
	amf3object InjuredTroops();
	amf3object Resources();
	amf3array HeroArray();
	amf3object Fortifications();

	void SetTroops(int8_t type, int64_t amount);
	void SetForts(int32_t type, int32_t count);
	int64_t GetTroops(int8_t type);
	bool HasTroops(stTroops & troops);

	void ParseBuildings(string str);
	void ParseTroops(string str);
	void ParseFortifications(string str);
	void ParseMisc(string str);

	bool CheckBuildingPrereqs(int16_t type, int16_t level);


	void CalculateStats();
	void CalculateResources();
	void RecalculateCityStats();
	void CalculateResourceStats();


	void CastleUpdate();
	void ResourceUpdate();
	void HeroUpdate(Hero * hero, int16_t updatetype);
	void TroopUpdate();
	void FortUpdate();

	int16_t GetReliefMultiplier();


	amf3array ResourceProduceData();



	std::vector<stTroopQueue> m_troopqueue;

	std::vector<stArmyMovement*> armymovement;

	int16_t HeroCount();
	Hero * GetHero(uint64_t id);
	stTroopQueue * GetBarracksQueue(int16_t position);
	int8_t AddToBarracksQueue(int8_t position, int16_t troopid, int32_t count, bool isshare, bool isidle);


	int16_t GetBuildingLevel(int16_t id);
	stBuilding * GetBuilding(int16_t position);
	int16_t GetTechLevel(int16_t id);
	int16_t GetBuildingCount(int16_t id);
}; // 3,448 + base

