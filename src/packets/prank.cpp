//
// prank.cpp
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

#include "prank.h"
#include "../Server.h"
#include "../Client.h"
#include "../City.h"
#include "../Alliance.h"
#include "../AllianceCore.h"

prank::prank(Server * server, request & req, amf3object & obj)
	: packet(server, req, obj)
{

}

prank::~prank()
{

}

void prank::process()
{
	if ((command == "getPlayerRank"))
	{
		int pagesize = data["pageSize"];
		string key = data["key"];
		int sorttype = data["sortType"];
		int pageno = data["pageNo"];

		obj2["cmd"] = "rank.getPlayerRank";
		data2["packageId"] = 0.0f;
		data2["ok"] = 1;
		std::list<stClientRank> * ranklist;
		amf3array beans = amf3array();
		LOCK(M_RANKEDLIST);
		gserver->mtxlist.ranklist.lock_shared();
		switch (sorttype)
		{
			case 1:
				ranklist = &gserver->m_titlerank;
				break;
			case 2:
				ranklist = &gserver->m_prestigerank;
				break;
			case 3:
				ranklist = &gserver->m_honorrank;
				break;
			case 4:
				ranklist = &gserver->m_citiesrank;
				break;
			case 5:
				ranklist = &gserver->m_populationrank;
				break;
			default:
				ranklist = &gserver->m_prestigerank;
				break;
		}
		std::list<stClientRank>::iterator iter;
		if (pagesize <= 0 || pagesize > 20 || pageno < 0 || pageno > 100000)
		{
			gserver->SendObject(client, gserver->CreateError("rank.getPlayerRank", -99, "Invalid data."));
			gserver->mtxlist.ranklist.unlock_shared();
			return;
		}

		if (key.length() > 0)
		{
			//search term given
			ranklist = (std::list<stClientRank>*)gserver->DoRankSearch(key, 1, ranklist, pageno, pagesize);//1 = client
		}

		if ((pageno - 1)*pagesize > ranklist->size())
		{
			gserver->SendObject(client, gserver->CreateError("rank.getPlayerRank", -99, "Invalid page."));
			gserver->mtxlist.ranklist.unlock_shared();
			UNLOCK(M_RANKEDLIST);
			return;
		}
		iter = ranklist->begin();
		for (int i = 0; i < (pageno - 1)*pagesize; ++i)
			iter++;

		int rank = (pageno - 1)*pagesize + 1;
		data2["pageNo"] = pageno;
		data2["pageSize"] = pagesize;
		if ((ranklist->size() % pagesize) == 0)
			data2["totalPage"] = (ranklist->size() / pagesize);
		else
			data2["totalPage"] = (ranklist->size() / pagesize) + 1;
		for (; iter != ranklist->end() && pagesize != 0; ++iter)
		{
			pagesize--;
			amf3object temp = amf3object();
			temp["createrTime"] = 0;
			if (iter->client->m_allianceid > 0)
			{
				temp["alliance"] = iter->client->GetAlliance()->m_name;
				temp["allianceLevel"] = AllianceCore::GetAllianceRank(iter->client->m_alliancerank);
				temp["levelId"] = iter->client->m_alliancerank;
			}
			temp["office"] = iter->client->m_office;
			temp["sex"] = iter->client->m_sex;
			temp["honor"] = iter->client->m_honor;
			temp["bdenyotherplayer"] = iter->client->m_bdenyotherplayer;
			temp["id"] = iter->client->m_accountid;
			temp["accountName"] = "";
			temp["prestige"] = iter->client->m_prestige;
			temp["faceUrl"] = iter->client->m_faceurl;
			temp["flag"] = iter->client->m_flag;
			temp["userId"] = iter->client->masteraccountid;
			temp["userName"] = iter->client->m_playername;
			temp["castleCount"] = iter->client->m_citycount;
			temp["titleId"] = iter->client->m_title;
			temp["medal"] = 0;
			temp["ranking"] = iter->rank;
			temp["lastLoginTime"] = 0;
			temp["population"] = iter->client->m_population;
			beans.Add(temp);
		}
		gserver->mtxlist.ranklist.unlock_shared();
		UNLOCK(M_RANKEDLIST);

		data2["beans"] = beans;
		gserver->SendObject(client, obj2);
		return;
	}
	if ((command == "getAllianceRank"))
	{
		int pagesize = data["pageSize"];
		string key = data["key"];
		int sorttype = data["sortType"];
		int pageno = data["pageNo"];

		obj2["cmd"] = "rank.getAllianceRank";
		data2["packageId"] = 0.0f;
		data2["ok"] = 1;
		std::list<stAlliance> * ranklist;
		amf3array beans = amf3array();
		LOCK(M_RANKEDLIST);
		gserver->mtxlist.ranklist.lock_shared();
		switch (sorttype)
		{
			case 1:
				ranklist = &gserver->m_alliances->m_membersrank;
				break;
			case 2:
				ranklist = &gserver->m_alliances->m_prestigerank;
				break;
			case 3:
				ranklist = &gserver->m_alliances->m_honorrank;
				break;
			default:
				ranklist = &gserver->m_alliances->m_membersrank;
				break;
		}
		std::list<stAlliance>::iterator iter;
		if (pagesize <= 0 || pagesize > 20 || pageno < 0 || pageno > 100000)
		{
			gserver->SendObject(client, gserver->CreateError("rank.getAllianceRank", -99, "Invalid data."));
			gserver->mtxlist.ranklist.unlock_shared();
			UNLOCK(M_RANKEDLIST);
			return;
		}

		if (key.length() > 0)
		{
			//search term given
			ranklist = (std::list<stAlliance>*)gserver->DoRankSearch(key, 4, ranklist, pageno, pagesize);//1 = client
		}

		if ((pageno - 1)*pagesize > ranklist->size())
		{
			gserver->SendObject(client, gserver->CreateError("rank.getAllianceRank", -99, "Invalid page."));
			gserver->mtxlist.ranklist.unlock_shared();
			UNLOCK(M_RANKEDLIST);
			return;
		}
		iter = ranklist->begin();
		for (int i = 0; i < (pageno - 1)*pagesize; ++i)
			iter++;

		int rank = (pageno - 1)*pagesize + 1;
		data2["pageNo"] = pageno;
		data2["pageSize"] = pagesize;
		if ((ranklist->size() % pagesize) == 0)
			data2["totalPage"] = (ranklist->size() / pagesize);
		else
			data2["totalPage"] = (ranklist->size() / pagesize) + 1;
		for (; iter != ranklist->end() && pagesize != 0; ++iter)
		{
			pagesize--;
			amf3object temp = amf3object();
			temp["member"] = iter->ref->m_currentmembers;
			temp["prestige"] = iter->ref->m_prestige;
			temp["rank"] = iter->rank;
			temp["playerName"] = iter->ref->m_owner;
			temp["honor"] = iter->ref->m_honor;
			temp["description"] = iter->ref->m_intro;
			temp["createrName"] = iter->ref->m_founder;
			temp["name"] = iter->ref->m_name;
			temp["city"] = iter->ref->m_citycount;
			beans.Add(temp);
		}
		gserver->mtxlist.ranklist.unlock_shared();
		UNLOCK(M_RANKEDLIST);

		data2["beans"] = beans;
		gserver->SendObject(client, obj2);
		return;
	}
	if ((command == "getHeroRank"))
	{
		int pagesize = data["pageSize"];
		string key = data["key"];
		int sorttype = data["sortType"];
		int pageno = data["pageNo"];

		obj2["cmd"] = "rank.getHeroRank";
		data2["packageId"] = 0.0f;
		data2["ok"] = 1;
		std::list<stHeroRank> * ranklist;
		amf3array beans = amf3array();
		LOCK(M_RANKEDLIST);
		switch (sorttype)
		{
			case 1:
				ranklist = &gserver->m_herorankgrade;
				break;
			case 2:
				ranklist = &gserver->m_herorankmanagement;
				break;
			case 3:
				ranklist = &gserver->m_herorankpower;
				break;
			case 4:
				ranklist = &gserver->m_herorankstratagem;
				break;
			default:
				ranklist = &gserver->m_herorankgrade;
				break;
		}
		std::list<stHeroRank>::iterator iter;
		if (pagesize <= 0 || pagesize > 20 || pageno < 0 || pageno > 100000)
		{
			gserver->SendObject(client, gserver->CreateError("rank.getHeroRank", -99, "Invalid data."));
			UNLOCK(M_RANKEDLIST);
			return;
		}

		if (key.length() > 0)
		{
			//search term given
			ranklist = (std::list<stHeroRank>*)gserver->DoRankSearch(key, 2, ranklist, pageno, pagesize);//1 = client
		}

		if ((pageno - 1)*pagesize > ranklist->size())
		{
			gserver->SendObject(client, gserver->CreateError("rank.getHeroRank", -99, "Invalid page."));
			UNLOCK(M_RANKEDLIST);
			return;
		}
		iter = ranklist->begin();
		for (int i = 0; i < (pageno - 1)*pagesize; ++i)
			iter++;

		int rank = (pageno - 1)*pagesize + 1;
		data2["pageNo"] = pageno;
		data2["pageSize"] = pagesize;
		if ((ranklist->size() % pagesize) == 0)
			data2["totalPage"] = (ranklist->size() / pagesize);
		else
			data2["totalPage"] = (ranklist->size() / pagesize) + 1;
		for (; iter != ranklist->end() && pagesize != 0; ++iter)
		{
			pagesize--;
			amf3object temp = amf3object();
			temp["rank"] = iter->rank;
			temp["stratagem"] = iter->stratagem;
			temp["name"] = iter->name;
			temp["power"] = iter->power;
			temp["grade"] = iter->grade;
			temp["management"] = iter->management;
			temp["kind"] = iter->kind;
			beans.Add(temp);
		}
		UNLOCK(M_RANKEDLIST);

		data2["beans"] = beans;
		gserver->SendObject(client, obj2);
		return;
	}
	if ((command == "getCastleRank"))
	{
		int pagesize = data["pageSize"];
		string key = data["key"];
		int sorttype = data["sortType"];
		int pageno = data["pageNo"];

		obj2["cmd"] = "rank.getCastleRank";
		data2["packageId"] = 0.0f;
		data2["ok"] = 1;
		std::list<stCastleRank> * ranklist;
		amf3array beans = amf3array();
		LOCK(M_RANKEDLIST);
		switch (sorttype)
		{
			case 1:
				ranklist = &gserver->m_castlerankpopulation;
				break;
			case 2:
				ranklist = &gserver->m_castleranklevel;
				break;
			default:
				ranklist = &gserver->m_castlerankpopulation;
				break;
		}
		std::list<stCastleRank>::iterator iter;
		if (pagesize <= 0 || pagesize > 20 || pageno < 0 || pageno > 100000)
		{
			gserver->SendObject(client, gserver->CreateError("rank.getCastleRank", -99, "Invalid data."));
			UNLOCK(M_RANKEDLIST);
			return;
		}

		if (key.length() > 0)
		{
			//search term given
			ranklist = (std::list<stCastleRank>*)gserver->DoRankSearch(key, 3, ranklist, pageno, pagesize);//1 = client
		}

		if ((pageno - 1)*pagesize > ranklist->size())
		{
			gserver->SendObject(client, gserver->CreateError("rank.getCastleRank", -99, "Invalid page."));
			UNLOCK(M_RANKEDLIST);
			return;
		}
		iter = ranklist->begin();
		for (int i = 0; i < (pageno - 1)*pagesize; ++i)
			iter++;

		int rank = (pageno - 1)*pagesize + 1;
		data2["pageNo"] = pageno;
		data2["pageSize"] = pagesize;
		if ((ranklist->size() % pagesize) == 0)
			data2["totalPage"] = (ranklist->size() / pagesize);
		else
			data2["totalPage"] = (ranklist->size() / pagesize) + 1;
		for (; iter != ranklist->end() && pagesize != 0; ++iter)
		{
			pagesize--;
			amf3object temp = amf3object();
			temp["alliance"] = iter->alliance;
			temp["rank"] = iter->rank;
			temp["level"] = iter->level;
			temp["name"] = iter->name;
			temp["grade"] = iter->grade;
			temp["kind"] = iter->kind;
			temp["population"] = iter->population;
			beans.Add(temp);
		}
		UNLOCK(M_RANKEDLIST);

		data2["beans"] = beans;
		gserver->SendObject(client, obj2);
		return;
	}
}



