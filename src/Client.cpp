//
// Client.cpp
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
#include "Client.h"
#include "Hero.h"
#include "funcs.h"
#include "Map.h"

#include "Server.h"
#include "Tile.h"
#include "City.h"
#include "AllianceCore.h"
#include "Alliance.h"


#ifndef WIN32
#define _ASSERT(x)
#endif

extern Server * gserver;

Client::Client(Server * core)
{
	m_mailpid = 1;
	m_accountexists = false;
	m_socknum = 0;
	m_accountid = 0;
	m_main = core;
	m_playerid = 0;
	PACKETSIZE = 1024 * 150;
	socket = 0;
	m_loggedin = false;
	m_clientnumber = -1;
	m_playername = "Testname";
	m_flag = "Evony";
	m_alliancename = "";
	m_email = "";
	m_password = "";
	m_prestige = 0;
	m_honor = 0;
	m_status = 0;
	m_rank = 0;
	m_prestigerank = 0;
	m_title = 0;
	m_connected = 0;
	//char * readytoprocess = new char[PACKETSIZE];
	processoffset = 0;
	m_citycount = 0;
	m_lastlogin = 0;
	m_creation = 0;
	m_sex = 0;
	m_allianceid = -1;
	m_alliancerank = 0;
	m_currentcityindex = -1;
	m_currentcityid = -1;
	m_population = 0;
	m_cents = 0;
	m_office = "Civilian";
	m_bdenyotherplayer = false;
	masteraccountid = 0;
	m_icon = 0;
	m_changedface = false;
	m_beginner = false;
	changeface = 0;//1 = xmas, 2 = halloween maybe?, more?
	m_allianceapplytime = 0;

	m_currentcityid = m_currentcityindex = -1;
	//m_city;// = new ArrayList();

	//memset(&m_buffs, 0, sizeof(m_buffs));
	memset(&m_research, 0, sizeof(m_research));
	//memset(&m_items, 0, sizeof(m_items));

	for (int i = 0; i < DEF_MAXITEMS; ++i)
	{
		m_items[i].count = 0;
		m_items[i].id = "";
		m_items[i].maxcount = 0;
		m_items[i].mincount = 0;
	}
	// 	for (int i = 0; i < 30; ++i)
	// 	{
	// 		m_buffs[i].endtime = 0.0;
	// 		m_buffs[i].id = "";
	// 	}
}

Client::~Client(void)
{
	//	for (int i = 0; i < m_city.size(); ++i)
	//		delete (City*)m_city.at(i);
}

Alliance * Client::GetAlliance()
{
	return m_main->m_alliances->AllianceById(m_allianceid);
	//	return 0;//TODO: FIX
};

string Client::DBBuffs()
{
	string res;
	std::stringstream ss;
	std::vector<Poco::Any> args;

	for (stBuff buff : m_buffs)
	{
		args.push_back(buff.id);
		args.push_back(buff.desc);
		args.push_back(buff.endtime);
		ss << "%?d,%?d,%?f|";
	}
	Poco::format(res, ss.str(), args);
	return res;
}

string Client::DBResearch()
{
	string res;
	std::stringstream ss;
	std::vector<Poco::Any> args;

	for (int i = 0; i < 25; ++i)
	{
		args.push_back(i);
		args.push_back(m_research[i].level);
		args.push_back(m_research[i].castleid);
		args.push_back(m_research[i].starttime);
		args.push_back(m_research[i].endtime);
		ss << "%?d,%?d,%?d,%?f,%?f|";
	}
	Poco::format(res, ss.str(), args);
	return res;
}

string Client::DBItems()
{
	string res;
	std::stringstream ss;
	std::vector<Poco::Any> args;

	for (int i = 0; i < DEF_MAXITEMS; ++i)
	{
		args.push_back(m_items[i].id);
		args.push_back(m_items[i].count);
		ss << "%?d,%?d|";
	}
	Poco::format(res, ss.str(), args);
	return res;
}

string Client::DBMisc()
{
	std::stringstream ss;
	ss << m_icon << "," << (m_changedface) ? 1 : 0;
	return ss.str();
}

bool Client::SaveToDB()
{
	typedef Poco::Tuple<string, string, string, string, int32_t, string, int32_t, string, string, int32_t, int16_t, int32_t, double, double, double, bool, int8_t, string, int64_t> ClientSave;


	ClientSave savedata(DBBuffs(), DBResearch(), DBItems(), DBMisc(), m_status, m_ipaddress, m_sex, m_flag, m_faceurl, m_allianceid, m_alliancerank, m_cents, m_prestige, m_honor, m_lastlogin, m_changedface, m_icon, m_allianceapply, m_allianceapplytime);


	try
	{
		Session ses(m_main->serverpool->get());
		ses << "UPDATE `accounts` SET buffs=?,`research`=?,items=?,misc=?,`status`=?,ipaddress=?,sex=?,flag=?,faceurl=?,allianceid=?,alliancerank=?,cents=?,prestige=?,honor=?,lastlogin=?,changedface=?,icon=?,allianceapply=?,allianceapplytime=? WHERE accountid=?;", use(savedata), use(m_accountid), now;
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

amf3object  Client::ToObject()
{
	amf3object obj = amf3object();
	int newmailsys = 0;
	int newmailinbox = 0;
	int newmailall = 0;
	for (stMail mail : m_mail)
	{
		if (!mail.isread())
		{
			if (mail.type_id == 1)
				newmailinbox++;
			else if (mail.type_id == 2)
				newmailsys++;
			newmailall++;
		}
	}
	int newreporttrade = 0;
	int newreportall = 0;
	int newreportother = 0;
	for (stMail mail : m_mail)
	{
		if (!mail.isread())
		{
			if (mail.type_id == 2)
				newreporttrade++;
			else if (mail.type_id == 1)
				newreportother++;
			newreportall++;
		}
	}
	obj["newReportCount_trade"] = newreporttrade;
	obj["newMaileCount_system"] = newmailsys;
	obj["newReportCount"] = newreportall;
	obj["isSetSecurityCode"] = false;
	obj["mapSizeX"] = gserver->mapsize;
	obj["mapSizeY"] = gserver->mapsize;
	obj["newReportCount_other"] = newreportother;
	obj["buffs"] = BuffsArray();
	obj["gamblingItemIndex"] = 1;
	obj["changedFace"] = m_changedface;
	obj["castles"] = CastleArray();
	obj["playerInfo"] = PlayerInfo();
	obj["redCount"] = 0;
	obj["usePACIFY_SUCCOUR_OR_PACIFY_PRAY"] = 1;//always 1?
	obj["newMaileCount_inbox"] = newmailinbox;

	string s;
	{
		time_t ttime;
		time(&ttime);
		struct tm * timeinfo;
		timeinfo = localtime(&ttime);

		std::stringstream ss;
		ss << (timeinfo->tm_year + 1900) << ".";
		if (timeinfo->tm_mon < 9)
			ss << "0" << (timeinfo->tm_mon + 1);
		else
			ss << (timeinfo->tm_mon + 1);
		ss << ".";
		if (timeinfo->tm_mday < 10)
			ss << "0" << timeinfo->tm_mday;
		else
			ss << timeinfo->tm_mday;
		ss << " ";
		if (timeinfo->tm_hour < 10)
			ss << "0" << timeinfo->tm_hour;
		else
			ss << timeinfo->tm_hour;
		ss << ".";
		if (timeinfo->tm_min < 10)
			ss << "0" << timeinfo->tm_min;
		else
			ss << timeinfo->tm_min;
		ss << ".";
		if (timeinfo->tm_sec < 10)
			ss << "0" << timeinfo->tm_sec;
		else
			ss << timeinfo->tm_sec;

		s = ss.str();
	}

	//	obj["currentDateTime"] = "2011.07.27 03.20.32";
	obj["currentDateTime"] = s.c_str();
	obj["newReportCount_army"] = 0;
	obj["friendArmys"] = amf3array();
	obj["saleTypeBeans"] = SaleTypeItems();
	obj["autoFurlough"] = false;
	obj["castleSignBean"] = amf3array();
	obj["furloughDay"] = 0;
	obj["tutorialStepId"] = 0;//10101; -- can set any tutorial
	obj["newReportCount_army"] = 0;
	obj["newMailCount"] = newmailall;
	obj["furlough"] = false;
	obj["gameSpeed"] = 1;
	obj["enemyArmys"] = amf3array();
	obj["currentTime"] = (double)unixtime();
	obj["items"] = Items();
	obj["freshMan"] = false;
	obj["finishedQuestCount"] = 0;
	obj["selfArmys"] = amf3array();
	obj["saleItemBeans"] = SaleItems();

	return obj;
}

amf3array Client::Items()
{
	//	amf3object retobj;
	//amf3array * itemarray = new amf3array();
	amf3array itemarray = amf3array();
	//	retobj.type = Array;
	//	retobj._value._array = itemarray;

	amf3object obj = amf3object();
	//age2?
	//obj["appearance"] = 96;
	//obj["help"] = 1;

	for (int i = 0; i < DEF_MAXITEMS; ++i)
	{
		if (m_items[i].count > 0)
		{
			obj = amf3object();
			obj["id"] = m_items[i].id;
			obj["count"] = m_items[i].count;
			obj["minCount"] = m_items[i].mincount;
			obj["maxCount"] = m_items[i].maxcount;
			itemarray.Add(obj);
		}
	}
	return itemarray;
}
amf3array Client::SaleTypeItems()
{
	amf3array array = amf3array();
	amf3object obj1 = amf3object();
	amf3object obj2 = amf3object();
	amf3object obj3 = amf3object();
	amf3object obj4 = amf3object();
	obj1["typeName"] = "\xE5\x8A\xA0\xE9\x80\x9F";//加速
	obj2["typeName"] = "\xE5\xAE\x9D\xE7\xAE\xB1";//宝箱
	obj3["typeName"] = "\xE7\x94\x9F\xE4\xBA\xA7";//生产
	obj4["typeName"] = "\xE5\xAE\x9D\xE7\x89\xA9";//宝物
	std::stringstream ss1, ss2, ss3, ss4;

	bool count1, count2, count3, count4;
	count1 = count2 = count3 = count4 = false;

	for (int i = 0; i < DEF_MAXITEMS; ++i)
	{
		if (m_main->m_items[i].buyable)
		{
			if (m_main->m_items[i].type == 1)//speed up tab
			{
				if (count1)
					ss1 << "#";
				ss1 << m_main->m_items[i].name;
				count1 = true;
			}
			else if (m_main->m_items[i].type == 2)//chest tab
			{
				if (count2)
					ss2 << "#";
				ss2 << m_main->m_items[i].name;
				count2 = true;
			}
			else if (m_main->m_items[i].type == 3)//produce tab
			{
				if (count3)
					ss3 << "#";
				ss3 << m_main->m_items[i].name;
				count3 = true;
			}
			else if (m_main->m_items[i].type == 4)//items tab
			{
				if (count4)
					ss4 << "#";
				ss4 << m_main->m_items[i].name;
				count4 = true;
			}
		}
	}
	obj1["items"] = ss1.str();
	obj2["items"] = ss2.str();
	obj3["items"] = ss3.str();
	obj4["items"] = ss4.str();
	array.Add(obj1);
	array.Add(obj2);
	array.Add(obj3);
	array.Add(obj4);

	//	obj["items"] = "consume.2.a#consume.2.b#consume.2.b.1#consume.2.c#consume.2.c.1#consume.2.d#player.fort.1.c#player.troop.1.b#consume.transaction.1";

	// 	obj = amf3object();
	// 	obj["typeName"] = "\xE5\xAE\x9D\xE7\xAE\xB1";//宝箱
	// 	obj["items"] = "player.box.special.1#player.box.special.2#player.box.special.3#player.box.currently.1#player.box.gambling.1#player.box.gambling.2#player.box.gambling.3#player.box.gambling.4#player.box.gambling.5#player.box.gambling.6#player.box.gambling.7#player.box.gambling.8#player.box.gambling.9#player.box.gambling.10#player.box.gambling.11#player.box.gambling.12";
	// 	array.Add(obj);
	// 
	// 	obj = amf3object();
	// 	obj["typeName"] = "\xE7\x94\x9F\xE4\xBA\xA7";//生产
	// 	obj["items"] = "player.resinc.1#player.resinc.1.b#player.resinc.2#player.resinc.2.b#player.resinc.3#player.resinc.3.b#player.resinc.4#player.resinc.4.b#player.gold.1.a#player.gold.1.b";
	// 	array.Add(obj);
	// 
	// 	obj = amf3object();
	// 	obj["typeName"] = "\xE5\xAE\x9D\xE7\x89\xA9";//宝物
	// 	obj["items"] = "player.speak.bronze_publicity_ambassador.permanent#player.speak.bronze_publicity_ambassador.permanent.15#player.pop.1.a#hero.management.1#player.experience.1.b#player.experience.1.a#player.queue.building#player.experience.1.c#player.peace.1#player.heart.1.a#hero.intelligence.1#consume.blueprint.1#consume.refreshtavern.1#consume.move.1#player.more.castle.1.a#player.name.1.a#consume.changeflag.1#player.troop.1.a#player.attackinc.1#player.attackinc.1.b#consume.hegemony.1#player.defendinc.1#player.defendinc.1.b#player.relive.1#hero.reset.1#hero.reset.1.a#player.destroy.1.a#hero.power.1#consume.1.a#consume.1.b#consume.1.c";
	// 	array.Add(obj);
	return array;
}
amf3array Client::SaleItems()
{
	amf3array array = amf3array();
	amf3object obj = amf3object();
	obj["saleType"] = 1;
	obj["items"] = "player.resinc.1";
	array.Add(obj);

	obj = amf3object();
	obj["saleType"] = 1;
	obj["items"] = "player.resinc.2";
	array.Add(obj);

	obj = amf3object();
	obj["saleType"] = 1;
	obj["items"] = "player.resinc.3";
	array.Add(obj);

	obj = amf3object();
	obj["saleType"] = 1;
	obj["items"] = "player.resinc.4";
	array.Add(obj);

	obj = amf3object();
	obj["saleType"] = 0;
	obj["items"] = "consume.1.c";
	array.Add(obj);

	obj = amf3object();
	obj["saleType"] = 1;
	obj["items"] = "consume.2.b.1";
	array.Add(obj);

	obj = amf3object();
	obj["saleType"] = 1;
	obj["items"] = "player.gold.1.a";
	array.Add(obj);

	obj = amf3object();
	obj["saleType"] = 1;
	obj["items"] = "consume.refreshtavern.1";
	array.Add(obj);

	obj = amf3object();
	obj["saleType"] = 1;
	obj["items"] = "consume.2.b";
	array.Add(obj);

	obj = amf3object();
	obj["saleType"] = 1;
	obj["items"] = "consume.transaction.1";
	array.Add(obj);
	return array;
}
amf3array Client::CastleArray()
{
	// 	Amf3Array array = new Amf3Array();
	// 	for (int i = 0; i < m_citycount; ++i)
	// 		array.DenseArray.Add(((City)m_city[i]).ToObject());
	// 	return array;
	amf3array array = amf3array();
	for (int32_t i = 0; i < m_citycount; ++i)
	{
		amf3object temp = ((PlayerCity*)m_city.at(i))->ToObject();
		array.Add(temp);
	}
	return array;
}
amf3array Client::BuffsArray()
{
	amf3array array = amf3array();
	for (int32_t i = 0; i < m_buffs.size(); ++i)
	{
		amf3object temp = amf3object();
		temp["desc"] = m_buffs[i].desc;
		temp["endTime"] = m_buffs[i].endtime;
		temp["id"] = m_buffs[i].id;
		array.Add(temp);
	}
	// 	amf3object t = amf3object();
	// 	t["id"] = "PlayerSpeakerBuf_BPAP_15";
	// 	t["endTime"] = unixtime()+100000;
	// 	t["desc"] = 3;
	// 	array.Add(t);
	return array;
}
amf3array Client::Packages()
{
	amf3array array = amf3array();
	for (int32_t i = 0; i < m_packages.size(); ++i)
	{
		amf3object temp = amf3object();
		temp["desc"] = m_packages[i].desc;
		temp["packageName"] = m_packages[i].name;
		temp["status"] = m_packages[i].id;
		temp["type"] = m_packages[i].id;
		temp["minMedal"] = m_packages[i].id;
		temp["maxMedal"] = m_packages[i].id;
		temp["id"] = m_packages[i].id;
		temp["isUseNow"] = m_packages[i].id;
		temp["scores"] = m_packages[i].id;
		temp["provideTime"] = m_packages[i].id;
		temp["expiredTime"] = m_packages[i].id;
		temp["claimedTime"] = m_packages[i].id;
		amf3array itemarray = amf3array();
		for (int32_t a = 0; a < m_packages[i].items.size(); ++a)
		{
			amf3object tempitem = amf3object();
			tempitem["id"] = m_packages[i].items[a].id;
			tempitem["count"] = m_packages[i].items[a].count;
			tempitem["name"] = m_packages[i].items[a].name;
			tempitem["minCount"] = m_packages[i].items[a].mincount;
			tempitem["maxCount"] = m_packages[i].items[a].maxcount;
			itemarray.Add(tempitem);
		}
		temp["itemList"] = itemarray;
		array.Add(temp);
	}
	return array;
}
amf3object Client::PlayerInfo()
{
	amf3object obj = amf3object();
	if (m_allianceid > 0)
	{
		obj["alliance"] = m_main->m_alliances->AllianceById(m_allianceid)->m_name;
		obj["allianceLevel"] = AllianceCore::GetAllianceRank(m_alliancerank);
		obj["levelId"] = m_alliancerank;
	}
	obj["createrTime"] = m_creation;//-3*7*24*60*60*1000;
	obj["office"] = m_office;
	obj["sex"] = m_sex;
	obj["honor"] = m_honor;
	obj["bdenyotherplayer"] = m_bdenyotherplayer;
	obj["id"] = m_accountid;
	obj["accountName"] = m_email;
	obj["prestige"] = m_prestige;
	obj["faceUrl"] = m_faceurl;
	obj["flag"] = m_flag;
	obj["userId"] = masteraccountid;
	obj["userName"] = m_playername;
	obj["castleCount"] = m_citycount;
	obj["medal"] = m_cents;
	obj["ranking"] = m_prestigerank;
	obj["titleId"] = m_title;
	obj["lastLoginTime"] = m_lastlogin;
	m_population = 0;

	for (int i = 0; i < m_citycount; ++i)
		m_population += ((PlayerCity*)m_city.at(i))->m_population;

	obj["population"] = m_population;
	return obj;
}

void Client::ParseBuffs(string str)
{
	if (str.length() > 0)
	{
		std::vector<string> bufftokens;
		std::vector<string> tokens;
		boost::split(bufftokens, str, boost::is_any_of("|"));

		for (int i = 0; i < bufftokens.size(); ++i)
		{
			boost::split(tokens, bufftokens[i], boost::is_any_of(","));

			if (tokens.size() == 3)
				SetBuff(tokens[0], tokens[1], _atoi64(tokens[2].c_str()));
		}
	}
}
void Client::ParseResearch(string str)
{
	if (str.length() > 0)
	{
		std::vector<string> researchtokens;
		std::vector<string> tokens;
		boost::split(researchtokens, str, boost::is_any_of("|"));

		for (int i = 0; i < researchtokens.size(); ++i)
		{
			boost::split(tokens, researchtokens[i], boost::is_any_of(","));

			if (tokens.size() == 5)
			{
				SetResearch(atoi(tokens[0].c_str()), atoi(tokens[1].c_str()), atoi(tokens[2].c_str()), atof(tokens[3].c_str()), atof(tokens[4].c_str()));
			}
		}
	}
}
void Client::ParseItems(string str)
{
	if (str.length() > 0)
	{
		std::vector<string> itemtokens;
		std::vector<string> tokens;
		boost::split(itemtokens, str, boost::is_any_of("|"));

		for (int i = 0; i < itemtokens.size(); ++i)
		{
			boost::split(tokens, itemtokens[i], boost::is_any_of(","));

			if (tokens.size() == 2)
				SetItem(tokens[0], _atoi64(tokens[1].c_str()));
		}
	}
}
void Client::ParseMisc(string str)
{
	if (str.length() > 0)
	{
		std::vector<string> tokens;
		boost::split(tokens, str, boost::is_any_of(","));

		if (tokens.size() == 2)
		{
			m_icon = atoi(tokens[0].c_str());
			m_changedface = (bool)tokens[1].c_str();
		}
		else
		{
			//error in token size
			m_main->consoleLogger->error(Poco::format("Error in '%s' ParseMisc()", m_playername));
		}
	}
}

void Client::SetBuff(string type, string desc, int64_t endtime, int8_t param)
{
	for (int i = 0; i < m_buffs.size(); ++i)
	{
		if (m_buffs[i].id == type)
		{
			m_buffs[i].endtime = endtime;
			BuffUpdate(type, desc, endtime);
			return;
		}
	}
	stBuff buff;
	buff.desc = desc;
	buff.endtime = endtime;
	buff.id = type;

	m_buffs.push_back(buff);

	BuffUpdate(type, desc, endtime, param);
	return;
}

void Client::RemoveBuff(string type)
{
	std::vector<stBuff>::iterator iter;
	if (m_buffs.size() > 0)
		for (iter = m_buffs.begin(); iter != m_buffs.end();)
		{
		if (iter->id == type)
		{
			BuffUpdate(type, iter->desc, iter->endtime, 1);//1 = remove
			m_buffs.erase(iter++);
			return;
		}
		++iter;
		}
	return;
}

PlayerCity * Client::GetCity(int32_t castleid)
{
	for (int32_t i = 0; i < m_citycount; ++i)
	{
		if (m_city[i]->m_castleid == castleid)
			return m_city[i];
	}
	return 0;
};

PlayerCity * Client::GetFocusCity()
{
	return (PlayerCity*)m_city[m_currentcityindex];
};

void Client::SetResearch(uint16_t id, int16_t level, int32_t castleid, double starttime, double endtime)
{
	if (id < 0 || id > 24)
		_ASSERT(0);
	m_research[id].level = level;
	m_research[id].endtime = endtime;
	m_research[id].starttime = starttime;
	m_research[id].castleid = castleid;
}

void Client::SetItem(string type, int16_t dir)
{
	stItem * sitem = 0;
	for (int i = 1; i < 100; ++i)
	{
		//Log("item %s - %s - %d", m_items[i].id.c_str(), type.c_str(), (bool)(m_items[i].id == type));
		if (m_items[i].id == type)
		{
			m_items[i].count += dir;
			sitem = &m_items[i];
			break;
		}
	}
	if (sitem == 0)
		for (int i = 1; i < 100; ++i)
		{
		if (m_items[i].id.length() == 0)
		{
			m_items[i].id = type;
			m_items[i].count += dir;
			sitem = &m_items[i];
			break;
		}
		}

	if (sitem == 0)
	{
		gserver->consoleLogger->information(Poco::format("Error in SetItem item:%s num:%d", type.c_str(), dir));
		return;
	}

	amf3object obj = amf3object();
	obj["cmd"] = "server.ItemUpdate";
	obj["data"] = amf3object();
	amf3object & data = obj["data"];
	amf3array array = amf3array();

	amf3object item = amf3object();
	item["id"] = (char*)type.c_str();
	item["count"] = sitem->count;
	item["minCount"] = 0;
	item["maxCount"] = 0;
	array.Add(item);

	data["items"] = array;

	m_main->SendObject(this, obj);
}

int16_t Client::GetItemCount(string type)
{
	for (int i = 0; i < DEF_MAXITEMS; ++i)
	{
		if (m_items[i].id == type)
		{
			return m_items[i].count;
		}
	}
	return 0;
}

int16_t Client::GetItemCount(int16_t type)
{
	if (type < 0 || type > DEF_MAXITEMS)
		return 0;
	return m_items[type].count;
}

int16_t Client::GetResearchLevel(int16_t id)
{
	return m_research[id].level;
}

void Client::CalculateResources()
{
	m_population = 0;
	for (int i = 0; i < m_citycount; ++i)
		m_population += ((PlayerCity*)m_city.at(i))->m_population;

	for (int i = 0; i < m_city.size(); ++i)
	{
		if (m_city[i])
		{
			m_city[i]->CalculateResourceStats();
			m_city[i]->CalculateStats();
			m_city[i]->CalculateResources();
		}
	}
}

void Client::PlayerUpdate()
{
	amf3object obj = amf3object();
	obj["cmd"] = "server.PlayerInfoUpdate";
	obj["data"] = amf3object();
	amf3object & data = obj["data"];
	data["playerInfo"] = PlayerInfo();

	m_main->SendObject(this, obj);
}
void Client::EnemyArmyUpdate()
{
	if (!m_connected)
		return;
	amf3object obj = amf3object();
	obj["cmd"] = "server.EnemyArmysUpdate";
	obj["data"] = amf3object();
	amf3object & data = obj["data"];
	amf3array armylist = amf3array();

	for (stArmyMovement * movement : enemyarmymovement)
	{
		amf3object tempobj = movement->ToObject();
		armylist.Add(tempobj);
	}

	m_main->SendObject(this, obj);
}
void Client::FriendArmyUpdate()
{
	if (!m_connected)
		return;
	amf3object obj = amf3object();
	obj["cmd"] = "server.FriendArmysUpdate";
	obj["data"] = amf3object();
	amf3object & data = obj["data"];
	amf3array armylist = amf3array();

	for (stArmyMovement * movement : friendarmymovement)
	{
		amf3object tempobj = movement->ToObject();
		armylist.Add(tempobj);
	}

	m_main->SendObject(this, obj);
}
void Client::SelfArmyUpdate()
{
	if (!m_connected)
		return;
	amf3object obj = amf3object();
	obj["cmd"] = "server.SelfArmysUpdate";
	obj["data"] = amf3object();
	amf3object & data = obj["data"];
	amf3array armylist = amf3array();

	for (stArmyMovement * movement : armymovement)
	{
		amf3object tempobj = movement->ToObject();
		armylist.Add(tempobj);
	}

	data["armys"] = armylist;

	m_main->SendObject(this, obj);
}

void Client::ItemUpdate(char * itemname)
{
	amf3object obj = amf3object();
	obj["cmd"] = "server.ItemUpdate";
	obj["data"] = amf3object();
	amf3object & data = obj["data"];

	amf3array items = amf3array();

	for (int i = 0; i < DEF_MAXITEMS; ++i)
	{
		if (!m_items[i].id.compare(itemname))
		{
			amf3object ta = amf3object();
			ta["id"] = itemname;
			ta["count"] = m_items[i].count;
			ta["minCount"] = m_items[i].mincount;
			ta["maxCount"] = m_items[i].maxcount;
			items.Add(ta);
			break;
		}
	}

	data["items"] = items;

	m_main->SendObject(this, obj);
}

void Client::HeroUpdate(int heroid, int castleid)
{
	amf3object obj = amf3object();
	obj["cmd"] = "server.HeroUpdate";
	obj["data"] = amf3object();
	amf3object & data = obj["data"];

	data["castleId"] = castleid;
	for (int i = 0; i < m_citycount; ++i)
	{
		for (int k = 0; k < 10; ++k)
		{
			if (m_city[i]->m_heroes[k]->m_id == heroid)
			{
				data["hero"] = m_city[i]->m_heroes[k]->ToObject();
				m_main->SendObject(this, obj);
				return;
			}
		}
	}
	// TODO error case maybe? - void Client::HeroUpdate(int heroid, int castleid)
	return;
}

void Client::MailUpdate()
{
	amf3object obj = amf3object();
	obj["cmd"] = "server.NewMail";
	obj["data"] = amf3object();
	amf3object & data = obj["data"];

	int32_t sysmail = 0;
	int32_t totalmail = 0;
	int32_t inboxmail = 0;

	for (stMail mail : m_mail)
	{
		if (!mail.isread())
		{
			totalmail++;
			if (mail.type_id == 1)
				inboxmail++;
			else if (mail.type_id == 2)
				sysmail++;
		}
	}
	data["count_system"] = sysmail;
	data["count"] = totalmail;
	data["count_inbox"] = inboxmail;

	m_main->SendObject(this, obj);

	return;
}

void Client::ReportUpdate()
{
	amf3object obj = amf3object();
	obj["cmd"] = "server.NewReport";
	obj["data"] = amf3object();
	amf3object & data = obj["data"];

	data["count"] = 0;
	data["army_inbox"] = 0;
	data["trade_count"] = 0;
	data["other_count"] = 0;

	// TODO count reports properly - void Client::ReportUpdate()

	m_main->SendObject(this, obj);

	return;
}

void Client::BuffUpdate(string name, string desc, int64_t endtime, int8_t type)
{
	amf3object obj = amf3object();
	obj["cmd"] = "server.PlayerBuffUpdate";
	obj["data"] = amf3object();
	amf3object & data = obj["data"];

	amf3object buffbean = amf3object();

	buffbean["descName"] = desc;
	buffbean["endTime"] = endtime;
	buffbean["typeId"] = name;

	data["buffBean"] = buffbean;

	data["updateType"] = (int32_t)type;

	m_main->SendObject(this, obj);
	return;
}

