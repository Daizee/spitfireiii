//
// Alliance.h
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

#pragma once

#include "funcs.h"
#include "structs.h"


class Server;
class Alliance;
class Client;

#define DEF_ALLIANCE 0
#define DEF_ALLY 1
#define DEF_NEUTRAL 2
#define DEF_ENEMY 3
#define DEF_NORELATION 4
#define DEF_SELFRELATION 5

// white = 2
// green = 0?
// teal (ally) = 1
// red = 3
// white = 4
// 

class Alliance
{
public:
	Alliance(Server * main, string name, int32_t ownerid);
	~Alliance();

	Server * m_main;

	amf3object ToObject();

	bool SaveToDB();
	bool InsertToDB();

	bool HasMember(string username);
	bool HasMember(uint64_t clientid);
	bool IsEnemy(int64_t allianceid);
	bool IsAlly(int64_t allianceid);
	bool IsNeutral(int64_t allianceid);
	void Ally(int64_t allianceid);
	void Neutral(int64_t allianceid);
	void Enemy(int64_t allianceid, bool skip = false);
	void UnAlly(int64_t allianceid);
	void UnNeutral(int64_t allianceid);
	void UnEnemy(int64_t allianceid);

	bool AddMember(uint64_t clientid, uint8_t rank);
	bool RemoveMember(uint64_t clientid);
	void ParseMembers(string str);
	void ParseRelation(std::vector<int64_t> * list, string str);

	void RequestJoin(Client * client, uint64_t timestamp);
	void UnRequestJoin(Client * client);
	void UnRequestJoin(string client);

	void SendAllianceMessage(string msg, bool tv, bool nosender);


	amf3object indexAllianceInfoBean();

	uint64_t enemyactioncooldown;

	int16_t m_currentmembers;
	int16_t m_maxmembers;

	int64_t m_prestige;
	int64_t m_honor;
	int16_t m_prestigerank;
	int16_t m_honorrank;
	int16_t m_membersrank;
	int32_t m_citycount;

	struct stMember
	{
		int32_t clientid;
		int8_t rank;
	};

	struct stInviteList
	{
		Client * client;
		string inviteperson;
		double invitetime;
	};

	std::vector<stInviteList> m_invites;

	std::vector<stMember> m_members;
	std::vector<int64_t> m_enemies;
	std::vector<int64_t> m_allies;
	std::vector<int64_t> m_neutral;

	int64_t m_ownerid;
	string m_owner;
	string m_name;
	string m_founder;
	string m_note;
	string m_intro;
	string m_motd;

	int64_t m_allianceid;
};

