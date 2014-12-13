//
// AllianceCore.h
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

#include <string.h>
#include "structs.h"
#include "funcs.h"


class Server;
class Alliance;
class Client;

#define DEF_MAXALLIANCES 1000

class AllianceCore
{
public:
	AllianceCore(Server * server);
	~AllianceCore();
	int16_t GetRelation(int64_t client1, int64_t client2);
	Alliance * CreateAlliance(string name, int64_t ownerid, int64_t allianceid = 0);
	void DeleteAlliance(Alliance * alliance);

	bool CheckName(string name);

	static string GetAllianceRank(int16_t rank)
	{
		if (rank == DEF_ALLIANCEHOST)
		{
			return "Host";
		}
		else if (rank == DEF_ALLIANCEVICEHOST)
		{
			return "Vice Host";
		}
		else if (rank == DEF_ALLIANCEPRES)
		{
			return "Presbyter";
		}
		else if (rank == DEF_ALLIANCEOFFICER)
		{
			return "Officer";
		}
		else if (rank == DEF_ALLIANCEMEMBER)
		{
			return "Member";
		}
		return "No Rank";
	};
	// 	string GetAllianceRank(int16_t rank)
	// 	{
	// 		if (rank == 1)
	// 		{
	// 			return "Leader";
	// 		}
	// 		else if (rank == 2)
	// 		{
	// 			return "Vice Host";
	// 		}
	// 		else if (rank == 3)
	// 		{
	// 			return "Presbyter";
	// 		}
	// 		else if (rank == 4)
	// 		{
	// 			return "Officer";
	// 		}
	// 		else if (rank == 5)
	// 		{
	// 			return "Member";
	// 		}
	// 		return "No Rank";
	// 	};

	void SortAlliances();
	bool JoinAlliance(uint64_t allianceid, Client * client);
	bool RemoveFromAlliance(uint64_t allianceid, Client * client);
	bool SetRank(uint64_t allianceid, Client * client, int8_t rank);

	Alliance * AllianceById(uint64_t id);
	Alliance * AllianceByName(string name);

	Server * m_main;

	Alliance * m_alliances[DEF_MAXALLIANCES];

	std::list<stAlliance> m_membersrank;
	std::list<stAlliance> m_prestigerank;
	std::list<stAlliance> m_honorrank;
};
