//
// Client.h
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

#include "amf3.h"
#include "City.h"
#include "connection.h"
#include "includes.h"
#include "structs.h"

class Server;
using boost::shared_mutex;

#pragma once
class Client
{
public:
	Client(Server * core);
	~Client(void);

	connection * socket;

	amf3array Packages();
	amf3object ToObject();
	amf3array Items();
	amf3array SaleTypeItems();
	amf3array SaleItems();
	amf3array CastleArray();
	amf3array BuffsArray();
	amf3object PlayerInfo();

	bool SaveToDB();

	double Prestige()
	{
		return m_prestige;
	}
	void Prestige(double pres)
	{
		m_prestige += pres;
		if (m_prestige < 0)
			m_prestige = 0;
		if (m_prestige > 2100000000)
			m_prestige = 2100000000;
	}

	void ParseBuffs(char * str);
	void ParseResearch(char * str);
	void ParseItems(char * str);
	void ParseMisc(char * str);

	void ParseBuffs(string str);
	void ParseResearch(string str);
	void ParseItems(string str);
	void ParseMisc(string str);

	string DBBuffs();
	string DBResearch();
	string DBItems();
	string DBMisc();

	void SetBuff(string type, string desc, int64_t endtime, int8_t param = 0);
	void RemoveBuff(string type);
	void SetItem(string type, int16_t dir);
	void SetResearch(uint16_t id, int16_t level, int32_t castleid, double starttime, double endtime);

	int16_t GetResearchLevel(int16_t id);
	int16_t GetItemCount(string type);
	int16_t GetItemCount(int16_t type);

	uint32_t m_mailpid;

	int64_t m_accountid;
	int64_t masteraccountid;

	bool m_accountexists;

	Server * m_main;
	int32_t m_playerid;// = 0;
	int32_t PACKETSIZE;// = 1024 * 150;
	uint64_t m_socknum;
	bool m_loggedin;// = false;
	uint32_t m_clientnumber;
	string m_playername;// = "Testname";
	string m_flag;// = "Evony";
	string m_faceurl;
	string m_alliancename;
	int16_t m_alliancerank;
	string m_office;
	string m_ipaddress;
	int32_t m_allianceid;
	string m_email;
	string m_password;
	double m_prestige;
	double m_honor;
	bool m_beginner;
	int32_t m_status;
	int32_t m_rank;
	int32_t m_title;
	double m_connected;
	double m_lastlogin;
	double m_creation;
	//char * readytoprocess = new char[PACKETSIZE];
	int32_t processoffset;
	int32_t m_citycount;
	int32_t m_sex;
	int32_t m_population;
	uint32_t m_prestigerank;
	uint32_t m_honorrank;
	std::vector<PlayerCity*> m_city;// = new ArrayList();
	int m_bdenyotherplayer;
	int m_tempvar;
	string m_allianceapply;
	int64_t m_allianceapplytime;
	bool m_changedface;
	uint16_t changeface;//new castle designs

	int8_t m_icon;

	int32_t m_cents;

	uint32_t m_currentcityindex;
	uint32_t m_currentcityid;

	double m_lag;


	std::vector<stPackage> m_packages;
	std::vector<stBuff> m_buffs;
	std::list<stMail> m_mail;
	stItem m_items[DEF_MAXITEMS];
	stResearch m_research[25];

	stBuff * GetBuff(string type)
	{
		for (int32_t i = 0; i < m_buffs.size(); ++i)
		{
			if (m_buffs[i].id == type)
				return &m_buffs[i];
		}
		return 0;
	}

	shared_mutex lists;

	std::vector<stArmyMovement*> armymovement;
	std::vector<stArmyMovement*> friendarmymovement;
	std::vector<stArmyMovement*> enemyarmymovement;

	void CalculateResources();
	void PlayerUpdate();
	void ItemUpdate(char * itemname);
	void BuffUpdate(string name, string desc, int64_t endtime, int8_t type = 0);
	void HeroUpdate(int heroid, int castleid);
	void MailUpdate();
	void ReportUpdate();
	void EnemyArmyUpdate();
	void FriendArmyUpdate();
	void SelfArmyUpdate();
	bool HasAlliance()
	{
		if (m_allianceid > 0)
			return true;
		return false;
	}
	Alliance * GetAlliance();

	PlayerCity * GetCity(int32_t castleid);
	PlayerCity * GetFocusCity();
	bool Beginner() { return m_beginner; }
	void Beginner(bool set, bool update = true) {
		if (m_beginner != set)
		{
			m_beginner = set;
			if (update)
				PlayerUpdate();
		}
	}
	inline void CheckBeginner(bool update = true)
	{
		if (Beginner())
		{
			if ((unixtime() - m_creation) > 1000 * 60 * 60 * 24 * 7)
			{
				Beginner(false, update);
				return;
			}
			for (int i = 0; i < this->m_city.size(); ++i)
			{
				if (m_city[i]->GetBuildingLevel(B_TOWNHALL) >= 5)
				{
					Beginner(false, update);
					return;
				}
			}
		}
	}
};

