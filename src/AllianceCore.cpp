//
// AllianceCore.cpp
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

#include "includes.h"
#include "AllianceCore.h"
#include "Alliance.h"
#include "Server.h"
#include "Client.h"

AllianceCore::AllianceCore(Server * server)
{
	m_main = server;
	for (int i = 0; i < DEF_MAXALLIANCES; ++i)
	{
		m_alliances[i] = 0;
	}
}

AllianceCore::~AllianceCore()
{
	for (int i = 0; i < DEF_MAXALLIANCES; ++i)
	{
		if (m_alliances[i])
			delete m_alliances[i];
	}
}

void AllianceCore::DeleteAlliance(Alliance * alliance)
{
	int32_t found = 0;
	for (int i = 0; i < DEF_MAXALLIANCES; ++i)
	{
		if (m_alliances[i])
		{
			if (m_alliances[i]->m_allianceid != alliance->m_allianceid)
			{
				if (m_alliances[i]->IsAlly(alliance->m_allianceid))
					m_alliances[i]->UnAlly(alliance->m_allianceid);
				if (m_alliances[i]->IsNeutral(alliance->m_allianceid))
					m_alliances[i]->UnNeutral(alliance->m_allianceid);
				if (m_alliances[i]->IsEnemy(alliance->m_allianceid))
					m_alliances[i]->UnEnemy(alliance->m_allianceid);


				for (Client * client : m_main->players)
				{
					if (client)
					{
						if ((client->m_allianceapply == alliance->m_name))
						{
							client->m_allianceapply = "";
							client->m_allianceapplytime = 0;
						}
					}
				}
			}
			else
			{
				found = i;
			}
		}
	}
	alliance->m_invites.clear();
	delete m_alliances[found];
	m_alliances[found] = 0;
}

Alliance * AllianceCore::CreateAlliance(string name, int64_t ownerid, int64_t allianceid)
{
	for (int i = 0; i < DEF_MAXALLIANCES; ++i)
	{
		if (m_alliances[i] == 0)
		{
			m_alliances[i] = new Alliance(m_main, name, ownerid);
			if (allianceid == 0)
				m_alliances[i]->m_allianceid = m_main->m_allianceid++;
			else
				m_alliances[i]->m_allianceid = allianceid;
			return m_alliances[i];
		}
	}
	return 0;
}

bool AllianceCore::JoinAlliance(uint64_t allianceid, Client * client)
{
	if (client->m_allianceid > 0)
	{
		return false; //already in an alliance
	}
	Alliance * alliance = AllianceById(allianceid);
	if (alliance == (Alliance*)-1)
	{
		return false; //alliance doesn't exist
	}


	if (alliance->AddMember(client->m_accountid, DEF_ALLIANCEMEMBER))
	{
		client->m_allianceid = alliance->m_allianceid;
		client->m_alliancename = alliance->m_name;
		client->m_alliancerank = DEF_ALLIANCEMEMBER;
		client->PlayerUpdate();
		return true;
	}
	return false;
}

bool AllianceCore::RemoveFromAlliance(uint64_t allianceid, Client * client)
{
	if (client->m_allianceid <= 0)
	{
		return false; //not in an alliance
	}
	Alliance * alliance = AllianceById(allianceid);
	if (alliance == (Alliance*)-1)
	{
		return false; //alliance doesn't exist
	}

	alliance->RemoveMember(client->m_accountid);

	client->m_allianceid = 0;
	client->m_alliancename = "";
	client->m_alliancerank = 0;

	client->PlayerUpdate();
	return true;
}

bool AllianceCore::SetRank(uint64_t allianceid, Client * client, int8_t rank)
{
	if (client->m_allianceid <= 0)
	{
		return false; //no alliance id given
	}
	Alliance * alliance = AllianceById(allianceid);
	if (alliance == (Alliance*)-1)
	{
		return false; //alliance doesn't exist
	}


	for (int i = 0; i < alliance->m_members.size(); ++i)
	{
		if (alliance->m_members[i].clientid == client->m_accountid)
		{
			client->m_alliancerank = rank;
			alliance->m_members[i].rank = rank;
			return true;
		}
	}
	return false;
}

bool AllianceCore::CheckName(string name)
{
// 	char * tempname;
// 	tempname = new char[strlen(name)+1];
// 	memset(tempname, 0, strlen(name)+1);
// 	for (int i = 0; i < strlen(name); ++i)
// 		tempname[i] = tolower(name[i]);

	std::transform(name.begin(), name.end(), name.begin(), ::tolower);

	if (name.find("penis") != string::npos)
		return false;
	if (name.find("cock") != string::npos)
		return false;
	if (name.find("dick") != string::npos)
		return false;
	if (name.find("asshole") != string::npos)
		return false;
	if (name.find("fuck") != string::npos)
		return false;
	if (name.find("shit") != string::npos)
		return false;
	if (name.find("bitch") != string::npos)
		return false;
	if (name.find("tits") != string::npos)
		return false;
	if (name.find("cunt") != string::npos)
		return false;
	if (name.find("testicle") != string::npos)
		return false;
	if (name.find("queer") != string::npos)
		return false;
	if (name.find("fag") != string::npos)
		return false;
	if (name.find("gay") != string::npos)
		return false;
	if (name.find("homo") != string::npos)
		return false;
	if (name.find("whore") != string::npos)
		return false;
	if (name.find("nigger") != string::npos)
		return false;
	return true;
}

int16_t AllianceCore::GetRelation(int64_t client1, int64_t client2)
{
	if (client1 == client2)
		return DEF_SELFRELATION;

	Client * c1 = m_main->GetClient(client1);
	Client * c2 = m_main->GetClient(client2);

	if (!c1 || !c2 || c1->m_allianceid < 0 || c2->m_allianceid < 0)
		return DEF_NORELATION;

	if (c1->m_allianceid == c2->m_allianceid)
		return DEF_ALLIANCE;

	if (c1->m_allianceid <= 0 || c2->m_allianceid <= 0)
		return DEF_NORELATION;

	Alliance * a1 = m_alliances[c1->m_allianceid];
	Alliance * a2 = m_alliances[c2->m_allianceid];

	if (a1->IsEnemy(c2->m_allianceid))
		return DEF_ENEMY;
	else if (a1->IsAlly(c2->m_allianceid))
		return DEF_ALLY;
	else if (a1->IsNeutral(c2->m_allianceid))
		return DEF_NEUTRAL;
	else
		return DEF_NORELATION;
}

bool comparemembers(stAlliance first, stAlliance second)
{
	if (first.members > second.members)
		return true;
	else
		return false;
}
bool compareprestige(stAlliance first, stAlliance second)
{
	if (first.prestige > second.prestige)
		return true;
	else
		return false;
}
bool comparehonor(stAlliance first, stAlliance second)
{
	if (first.honor > second.honor)
		return true;
	else
		return false;
}

void AllianceCore::SortAlliances()
{
	m_main->mtxlist.alliance.lock_shared();
	m_membersrank.clear();
	m_prestigerank.clear();
	m_honorrank.clear();

	stAlliance stmem;
	stAlliance stpre;
	stAlliance sthon;
	for (int i = 0; i < DEF_MAXALLIANCES; ++i)
	{
		if (m_alliances[i])
		{
			sthon.honor = stpre.prestige = 0;
			stmem.id = stpre.id = sthon.id = m_alliances[i]->m_allianceid;
			stmem.members = m_alliances[i]->m_members.size();
			m_alliances[i]->m_citycount = 0;
			for (int k = 0; k < m_alliances[i]->m_members.size(); ++k)
			{
				Client * client = m_main->GetClient(m_alliances[i]->m_members[k].clientid);
				if (client)
				{
					sthon.honor += client->m_honor;
					stpre.prestige += client->m_prestige;
					m_alliances[i]->m_citycount += client->m_citycount;
				}
			}
			stmem.ref = m_alliances[i];
			stpre.ref = m_alliances[i];
			sthon.ref = m_alliances[i];
			m_membersrank.push_back(stmem);
			m_prestigerank.push_back(stpre);
			m_honorrank.push_back(sthon);
		}
	}

	m_membersrank.sort(comparemembers);
	m_prestigerank.sort(compareprestige);
	m_honorrank.sort(comparehonor);


	int num = 1;
	for (stAlliance & alliance : m_membersrank)
	{
		alliance.rank = alliance.ref->m_membersrank = num++;
	}
	num = 1;
	for (stAlliance & alliance : m_prestigerank)
	{
		alliance.rank = alliance.ref->m_prestigerank = num++;
		alliance.ref->m_prestige = alliance.prestige;
	}
	num = 1;
	for (stAlliance & alliance : m_honorrank)
	{
		alliance.rank = alliance.ref->m_honorrank = num++;
		alliance.ref->m_honor = alliance.honor;
	}
	m_main->mtxlist.alliance.unlock_shared();
}

Alliance * AllianceCore::AllianceById(uint64_t id)
{
	for (int i = 0; i < DEF_MAXALLIANCES; ++i)
	{
		if (m_alliances[i] && m_alliances[i]->m_allianceid == id)
		{
			return m_alliances[i];
		}
	}
	return (Alliance*)-1;
}

Alliance * AllianceCore::AllianceByName(string name)
{
	for (int i = 0; i < DEF_MAXALLIANCES; ++i)
	{
		if (m_alliances[i] && m_alliances[i]->m_name == name)
		{
			return m_alliances[i];
		}
	}
	return (Alliance*)-1;
}


/*
struct stHero
{
	int32_t m_id;
	int16_t m_level;
	int8_t m_loyalty;
	int16_t m_management;
	int16_t m_managementbuffadded;
	int16_t m_power;
	int16_t m_powerbuffadded;
	int16_t m_stratagem;
	int16_t m_stratagembuffadded;
	int8_t m_status;

};
struct stResources
{
	double food;
	double stone;
	double iron;
	double wood;
	double gold;
};
struct stForts
{
	int32_t traps;
	int32_t abatis;
	int32_t towers;
	int32_t logs;
	int32_t trebs;
};
struct stTroops
{
	int32_t worker;
	int32_t warrior;
	int32_t scout;
	int32_t pike;
	int32_t sword;
	int32_t archer;
	int32_t cavalry;
	int32_t cataphract;
	int32_t transporter;
	int32_t ballista;
	int32_t ram;
	int32_t catapult;
};
struct stResult
{
	stResources resources;
	bool attackerwon;
};
struct stBuff
{
	string id;
	double endtime;
} m_buffs;
struct stResearch
{
	int16_t level;
	double endtime;
} m_research;
struct stAttacker
{
	stTroops troops;
	stBuff ** buffs;
	stResearch ** techs;
	PlayerCity * city;
	stHero hero;
};
struct stDefender
{
	stTroops troops;
	stForts forts;
	stBuff ** buffs;
	stResearch ** techs;
	PlayerCity * city;
	stHero hero;
};
stResult * CalculateBattle(stAttacker & attacker, stDefender & defender)
{
	
}*/

