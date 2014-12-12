//
// Alliance.cpp
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
#include "Alliance.h"
#include "AllianceCore.h"
#include "Server.h"
#include "Client.h"


Alliance::Alliance(Server * main, string name, int32_t ownerid)
{
	enemyactioncooldown = 0;
	m_ownerid = ownerid;
	m_name = name;
	m_members.clear();
	m_enemies.clear();
	m_allies.clear();
	m_neutral.clear();
	m_currentmembers = 0;
	m_maxmembers = 500;
	//m_members.push_back(ownerid);
	m_prestige = 0;
	m_honor = 0;
	m_prestigerank = 0;
	m_honorrank = 0;
	m_membersrank = 0;
	m_main = main;
	m_owner = m_main->GetClient(ownerid)->m_playername;
	m_citycount = 0;
}

Alliance::~Alliance()
{

}

bool Alliance::SaveToDB()
{
	typedef Poco::Tuple<string> AllianceSave;


	AllianceSave savedata(m_name);


	try
	{
		Session ses(m_main->serverpool->get());
		ses << "UPDATE `accounts` SET buffs=?,`research`=?,items=?,misc=?,`status`=?,ipaddress=?,sex=?,flag=?,faceurl=?,allianceid=?,alliancerank=?,cents=?,prestige=?,honor=?,lastlogin=?,changedface=?,icon=?,allianceapply=?,allianceapplytime=? WHERE accountid=?;", use(savedata), now;
		return true;
	}
	catch (Poco::Data::MySQL::ConnectionException& e)\
	{
		m_main->consoleLogger->error(Poco::format("ConnectionException: %s", e.displayText()));
	}
	catch (Poco::Data::MySQL::StatementException& e)
	{
		m_main->consoleLogger->error(Poco::format("StatementException: %s", e.displayText()));
	}
	catch (Poco::Data::MySQL::MySQLException& e)
	{
		m_main->consoleLogger->error(Poco::format("MySQLException: %s", e.displayText()));
	}
	catch (Poco::InvalidArgumentException& e)
	{
		m_main->consoleLogger->error(Poco::format("InvalidArgumentException: %s", e.displayText()));
	}
	return false;
}

amf3object Alliance::ToObject()
{
	amf3object obj = amf3object();

	return obj;
}

bool Alliance::IsEnemy(int64_t allianceid)
{
	for (int64_t allianceid : m_enemies)
	{
		if (allianceid == allianceid)
		{
			return true;
		}
	}
	return false;
}

bool Alliance::IsAlly(int64_t allianceid)
{
	for (int64_t allianceid : m_allies)
	{
		if (allianceid == allianceid)
		{
			return true;
		}
	}
	return false;
}

bool Alliance::IsNeutral(int64_t allianceid)
{
	for (int64_t allianceid : m_neutral)
	{
		if (allianceid == allianceid)
		{
			return true;
		}
	}
	return false;
}

bool Alliance::AddMember(uint64_t clientid, uint8_t rank)
{
	if (m_currentmembers >= m_maxmembers)
	{
		return false;
	}
	stMember member;
	member.clientid = clientid;
	member.rank = rank;
	m_members.push_back(member);
	m_currentmembers++;
	return true;
}

bool Alliance::HasMember(string username)
{
	Client * client = m_main->GetClientByName(username);
	if (!client)
		return false;
	return HasMember(client->m_accountid);
}

bool Alliance::HasMember(uint64_t clientid)
{
	std::vector<Alliance::stMember>::iterator iter;
	for ( iter = m_members.begin() ; iter != m_members.end(); ++iter)
	{
		if (iter->clientid == clientid)
		{
			return true;
		}
	}
	return false;
}

bool Alliance::RemoveMember(uint64_t clientid)
{
	std::vector<Alliance::stMember>::iterator iter;
	for ( iter = m_members.begin() ; iter != m_members.end(); )
	{
		if (iter->clientid == clientid)
		{
			m_members.erase(iter);
			m_currentmembers--;
			return true;
		}
		++iter;
	}
	return true;
}

void Alliance::RequestJoin(Client * client, uint64_t timestamp)
{
	stInviteList invite;
	invite.invitetime = timestamp;
	invite.client = client;
	m_invites.push_back(invite);
}

void Alliance::UnRequestJoin(Client * client)
{
	std::vector<stInviteList>::iterator iter;
	if (m_invites.size())
	{
		for ( iter = m_invites.begin() ; iter != m_invites.end(); )
		{
			if (iter->client->m_accountid == client->m_accountid)
			{
				m_invites.erase(iter++);
				return;
			}
			++iter;
		}
	}
}

void Alliance::UnRequestJoin(string client)
{
	std::vector<stInviteList>::iterator iter;
	if (m_invites.size())
	{
		for ( iter = m_invites.begin() ; iter < m_invites.end(); )
		{
			if (iter->client->m_playername == client)
			{
				m_invites.erase(iter++);
				return;
			}
			++iter;
		}
	}
}

void Alliance::ParseMembers(string str)
{
	if (str.length() > 0)
	{
		char * str2 = new char[str.length()+1];
		memset(str2, 0, str.length()+1);
		memcpy(str2, str.c_str(), str.length());

		uint32_t clientid;
		uint8_t rank;

		char * ch = 0, * cr = 0;
		char * tok;
		tok = strtok_s(str2, "|", &ch);
		do
		{
			tok = strtok_s(tok, ",", &cr);
			if (tok != 0)
				clientid = atoi(tok);
			tok = strtok_s(0, ",", &cr);
			if (tok != 0)
				rank = atoi(tok);

			AddMember(clientid, rank);
			tok = strtok_s(0, "|", &ch);
		} while (tok != 0);

		delete[] str2;
	}
}

void Alliance::ParseRelation(std::vector<int64_t> * list, string str)
{
	if (str.length() > 0)
	{
		char * str2 = new char[str.length()+1];
		memset(str2, 0, str.length()+1);
		memcpy(str2, str.c_str(), str.length());

		uint64_t allianceid;

		char * ch = 0, * cr = 0;
		char * tok;
		tok = strtok_s(str2, "|", &ch);
		do
		{
			tok = strtok_s(tok, ",", &cr);
			if (tok != 0)
				allianceid = atoi(tok);

			list->push_back(allianceid);

			tok = strtok_s(0, "|", &ch);
		} while (tok != 0);

		delete[] str2;
	}
}

amf3object Alliance::indexAllianceInfoBean()
{
	amf3object allianceinfo;
	allianceinfo["prestige"] = m_prestige;
	allianceinfo["rank"] = m_prestigerank;
	allianceinfo["creatorName"] = m_founder.c_str();
	allianceinfo["allianceNote"] = m_note.c_str();
	allianceinfo["allianceInfo"] = m_intro.c_str();
	allianceinfo["allianceName"] = m_name.c_str();
	allianceinfo["memberCount"] = m_currentmembers;
	allianceinfo["memberLimit"] = m_maxmembers;
	allianceinfo["leaderName"] = m_owner.c_str();
	return allianceinfo;
};

void Alliance::Ally(int64_t allianceid)
{
	if (IsAlly(allianceid))
		return;
	if (IsNeutral(allianceid))
		UnNeutral(allianceid);
	if (IsEnemy(allianceid))
		UnEnemy(allianceid);
	m_allies.push_back(allianceid);
	Alliance * temp = m_main->m_alliances->AllianceById(allianceid);
	temp->SendAllianceMessage("Alliance [" + temp->m_name + "] recognizes Diplomatic Relationship with us as Ally.", false, false);
}
void Alliance::Neutral(int64_t allianceid)
{
	if (IsNeutral(allianceid))
		return;
	if (IsAlly(allianceid))
		UnAlly(allianceid);
	if (IsEnemy(allianceid))
		UnEnemy(allianceid);
	m_neutral.push_back(allianceid);
	Alliance * temp = m_main->m_alliances->AllianceById(allianceid);
	temp->SendAllianceMessage("Alliance [" + temp->m_name + "] recognizes Diplomatic Relationship with us as Neutral.", false, false);
}
void Alliance::Enemy(int64_t allianceid, bool skip /* = false*/)
{
	if (IsEnemy(allianceid))
		return;
	enemyactioncooldown = unixtime() + 1000*60*60*24;
	if (IsNeutral(allianceid))
		UnNeutral(allianceid);
	if (IsAlly(allianceid))
		UnAlly(allianceid);
	m_enemies.push_back(allianceid);
	if (skip)
		return;
	//send global message
	Alliance * temp = m_main->m_alliances->AllianceById(allianceid);
	m_main->MassMessage("Alliance " + this->m_name + " declares war against alliance " + temp->m_name + ". Diplomatic Relationship between each other alters to Hostile automatically.");
	temp->Enemy(m_allianceid, true);
	temp->SendAllianceMessage("Alliance [" + m_name + "] recognizes Diplomatic Relationship with us as Enemy.", false, false);
}
void Alliance::UnAlly(int64_t allianceid)
{
	std::vector<int64_t>::iterator iter;
	if (m_allies.size())
	{
		for ( iter = m_allies.begin() ; iter < m_allies.end(); )
		{
			if (*iter == allianceid)
			{
				m_allies.erase(iter++);
				return;
			}
			++iter;
		}
	}
}
void Alliance::UnNeutral(int64_t allianceid)
{
	std::vector<int64_t>::iterator iter;
	if (m_neutral.size())
	{
		for ( iter = m_neutral.begin() ; iter < m_neutral.end(); )
		{
			if (*iter == allianceid)
			{
				m_neutral.erase(iter++);
				return;
			}
			++iter;
		}
	}
}
void Alliance::UnEnemy(int64_t allianceid)
{
	std::vector<int64_t>::iterator iter;
	if (m_enemies.size())
	{
		for ( iter = m_enemies.begin() ; iter < m_enemies.end(); )
		{
			if (*iter == allianceid)
			{
				m_enemies.erase(iter++);
				return;
			}
			++iter;
		}
	}
}

void Alliance::SendAllianceMessage(string msg, bool tv, bool nosender)
{
	amf3object obj = amf3object();
	amf3object data = amf3object();

	obj["cmd"] = "server.SystemInfoMsg";
	data["alliance"] = true;
	data["sender"] = "Alliance Msg";
	data["tV"] = tv;
	data["msg"] = msg;
	data["noSenderSystemInfo"] = nosender;

	obj["data"] = data;

	for (Alliance::stMember & client : m_members)
	{
		m_main->SendObject(m_main->GetClient(client.clientid), obj);
	}
}
