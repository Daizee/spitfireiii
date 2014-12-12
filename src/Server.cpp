//
// server.cpp
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
#include "Server.h"

#include "Map.h"
#include "Tile.h"
#include "amf3.h"
#include "Hero.h"
#include "AllianceCore.h"
#include "Alliance.h"
#include "City.h"
#include "Client.h"


#define DEF_NOMAPDATA

Server::Server()
	: io_service_(),
	signals_(io_service_),
	acceptor_(io_service_),
	socket_(io_service_),
	map(0),
	currentplayersonline(0)
{
	pFCConsole = new FormattingChannel(new PatternFormatter("%p: %t"));
	pFCConsole->setChannel(new ConsoleChannel);
	pFCConsole->open();

	pFCDefault = new FormattingChannel(new PatternFormatter("%p: %t"));
	pFCDefault->setChannel(new ConsoleChannel);
	pFCDefault->open();

	pFCFile = new FormattingChannel(new PatternFormatter("%Y-%m-%d %H:%M:%S.%c %N[%P]:%s:%q:%t"));
	pFCFile->setChannel(new FileChannel("sample.log"));
	pFCFile->open();

	consoleLogger = &Logger::create("ConsoleLogger", pFCConsole, Message::PRIO_INFORMATION);
	logger = &Logger::create("DefaultLogger", pFCFile, Message::PRIO_INFORMATION);
	fileLogger = &Logger::create("FileLogger", pFCFile, Message::PRIO_INFORMATION);

	consoleLogger->information("EPS Server starting.");
	// 	consoleLogger->error("An error message");
	// 	consoleLogger->warning("A warning message");
	// 	consoleLogger->information("An information message");
	// 	fileLogger->warning("An information message");
	// 	if ((consoleLogger)->information())
	// 		(consoleLogger)->information("Another informational message", __FILE__, __LINE__);
	//	Logger::get("ConsoleLogger").error("Another error message");


	accountpool = 0;
	serverpool = 0;

	MySQL::Connector::registerConnector();


	SaveThreadRunning = false;
	serverstatus = SERVERSTATUS_STOPPED;//offline
	servername = "";
	memset(&m_buildingconfig, 0, sizeof(m_buildingconfig));
	memset(&m_researchconfig, 0, sizeof(m_researchconfig));
	memset(&m_troopconfig, 0, sizeof(m_troopconfig));

	mapsize = 500;
	map = 0;
	m_heroid = 1000;
	m_cityid = 100000;
	m_allianceid = 100;
	m_itemcount = 0;

	this->armycounter = 0;
	this->currentplayersonline = 0;
	this->ltime = 0;
	this->m_alliances = 0;

	L = luaL_newstate();
	luaL_openlibs(L);
	TimerThreadRunning = false;
	consoleLogger->information("Lua initialized.");

	// Register to handle the signals that indicate when the server should exit.
	// It is safe to register for the same signal multiple times in a program,
	// provided all registration for the specified signal is made through Asio.
	signals_.add(SIGINT);
	signals_.add(SIGTERM);
#if defined(SIGQUIT)
	signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)
	signals_.async_wait(
		[this](boost::system::error_code /*ec*/, int /*signo*/)
	{
		// The server is stopped by cancelling all outstanding asynchronous
		// operations. Once all operations have finished the io_service::run()
		// call will exit.
		Shutdown();
	});
}
Server::~Server()
{
	// 	if (skts)
	// 		delete skts;
	if (map)
		delete map;
	delete accountpool;
	delete serverpool;
	MySQL::Connector::unregisterConnector();
	Logger::destroy("ConsoleLogger");
	Logger::destroy("DefaultLogger");
	Logger::destroy("FileLogger");
	pFCConsole->close();
	pFCConsole->close();
	pFCDefault->close();
}

void Server::SendObject(Client * c, amf3object & object)
{
	if ((!c) || (!c->m_connected) || (c->socket == 0))
		return;
	SendObject(c->socket, object);
}
void Server::run()
{
	printf("Start up procedure\n");


#pragma region data load

	//Read Item XML info
	// 	FILE * itemxml;
	// 	fopen_s(&itemxml, "itemxml.xml", "r");
	// 	fseek(itemxml, 0, SEEK_END);
	// 	int32_t filesize = ftell(itemxml);
	// 	char * itemxmlbuff = new char[filesize+1];
	// 	itemxmlbuff[filesize+1] = 0;
	// 	fseek(itemxml, 0, SEEK_SET);
	// 	if (fread_s(itemxmlbuff, filesize, 1, filesize, itemxml) != filesize)
	// 	{
	// 		fclose(itemxml);
	// 		throw(std::exception("No settings exist for server."));
	// 		//error reading
	// 	}
	// 	fclose(itemxml);
	// 
	// 	this->m_itemxml = itemxmlbuff;
	// 	delete[] itemxmlbuff;




	//good to keep on hand for future
	// 	typedef Poco::Tuple<std::string, std::string, std::string, uint32_t, std::string, uint32_t, uint32_t, std::string, uint32_t> PAccount;
	// 	PAccount account;
	//	Statement stmt = ( ses << "SELECT * FROM account WHERE `name`=? AND `password`=?", use(username), use(password), into(account), now );
	//	account.get<0>()

	int count = 0;


	// 	if (rs.rowCount() == 0)
	// 	{
	// 		//no settings exist
	// 		throw(std::exception("No settings exist for server."));
	// 		return;
	// 	}
	// 
	// 
	// 	rs.moveFirst();
	// 	//client.name = (string)rs.value("name");
	// 	client.name = rs.value("name").convert<string>();
	// 	client.password = rs.value("password").convert<string>();
	// 	client.email = rs.value("email").convert<string>();
	// 	client.message = rs.value("reason").convert<string>();
	// 	client.lastip = rs.value("lastip").convert<string>();
	// 	client.xcoins = rs.value("xcoins").convert<uint32_t>();
	// 	client.lastlogin = rs.value("lasttime").convert<uint32_t>();
	// 	client.status = rs.value("status").convert<uint32_t>();
	// 	client.online = rs.value("online").convert<uint32_t>();







	map = new Map(this, mapsize);
	m_alliances = new AllianceCore(this);

	consoleLogger->information("Loading configurations.");

	try
	{
		Session ses(accountpool->get());

		{
			Statement select(ses);
			select << "SELECT * FROM `config_building`;";
			select.execute();
			RecordSet rs(select);

			rs.moveFirst();


			for (int i = 0; i < rs.rowCount(); ++i, rs.moveNext())
			{
				char * tok;
				char * ch = 0, *cr = 0;

				int32_t cfgid = rs.value("buildingid").convert<int32_t>();
				int32_t level = rs.value("level").convert<int32_t>() - 1;
				m_buildingconfig[cfgid][level].time = rs.value("buildtime").convert<double>();
				m_buildingconfig[cfgid][level].destructtime = rs.value("buildtime").convert<double>() / 2;
				m_buildingconfig[cfgid][level].food = rs.value("food").convert<int32_t>();
				m_buildingconfig[cfgid][level].wood = rs.value("wood").convert<int32_t>();
				m_buildingconfig[cfgid][level].stone = rs.value("stone").convert<int32_t>();
				m_buildingconfig[cfgid][level].iron = rs.value("iron").convert<int32_t>();
				m_buildingconfig[cfgid][level].gold = rs.value("gold").convert<int32_t>();
				m_buildingconfig[cfgid][level].prestige = rs.value("gold").convert<int32_t>();

				string strt = rs.value("prereqbuilding").convert<string>();
				char * str = (char*)strt.c_str();

				int x = 0;

				for (tok = strtok_s(str, "|", &ch); tok != 0; tok = strtok_s(0, "|", &ch))
				{
					stPrereq prereq;
					tok = strtok_s(tok, ",", &cr); assert(tok != 0); prereq.id = atoi(tok);
					tok = strtok_s(0, ",", &cr); assert(tok != 0); prereq.level = atoi(tok);
					m_buildingconfig[cfgid][level].buildings.push_back(prereq);
					x++;
				}

				strt = rs.value("prereqtech").convert<string>();
				str = (char*)strt.c_str();
				x = 0;

				for (tok = strtok_s(str, "|", &ch); tok != 0; tok = strtok_s(0, "|", &ch))
				{
					stPrereq prereq;
					tok = strtok_s(tok, ",", &cr); assert(tok != 0); prereq.id = atoi(tok);
					tok = strtok_s(0, ",", &cr); assert(tok != 0); prereq.level = atoi(tok);
					m_buildingconfig[cfgid][level].techs.push_back(prereq);
					x++;
				}

				strt = rs.value("prereqitem").convert<string>();
				str = (char*)strt.c_str();
				x = 0;

				for (tok = strtok_s(str, "|", &ch); tok != 0; tok = strtok_s(0, "|", &ch))
				{
					stPrereq prereq;
					tok = strtok_s(tok, ",", &cr); assert(tok != 0); prereq.id = atoi(tok);
					tok = strtok_s(0, ",", &cr); assert(tok != 0); prereq.level = atoi(tok);
					m_buildingconfig[cfgid][level].items.push_back(prereq);
					x++;
				}

				m_buildingconfig[cfgid][level].limit = rs.value("limit").convert<int32_t>();
				m_buildingconfig[cfgid][level].inside = rs.value("inside").convert<int32_t>();
			}
		}

		{
			Statement select(ses);
			select << "SELECT * FROM `config_troops`;";
			select.execute();
			RecordSet rs(select);

			rs.moveFirst();


			for (int i = 0; i < rs.rowCount(); ++i, rs.moveNext())
			{
				char * tok;
				char * ch = 0, *cr = 0;

				int32_t cfgid = rs.value("troopid").convert<int32_t>();
				m_troopconfig[cfgid].time = rs.value("buildtime").convert<double>();
				m_troopconfig[cfgid].destructtime = 0.0f;
				m_troopconfig[cfgid].food = rs.value("food").convert<int32_t>();
				m_troopconfig[cfgid].wood = rs.value("wood").convert<int32_t>();
				m_troopconfig[cfgid].stone = rs.value("stone").convert<int32_t>();
				m_troopconfig[cfgid].iron = rs.value("iron").convert<int32_t>();
				m_troopconfig[cfgid].gold = rs.value("gold").convert<int32_t>();
				m_troopconfig[cfgid].inside = rs.value("inside").convert<int32_t>();
				m_troopconfig[cfgid].population = rs.value("population").convert<int32_t>();

				string strt = rs.value("prereqbuilding").convert<string>();
				char * str = (char*)strt.c_str();
				int x = 0;

				for (tok = strtok_s(str, "|", &ch); tok != 0; tok = strtok_s(0, "|", &ch))
				{
					stPrereq prereq;
					tok = strtok_s(tok, ",", &cr); assert(tok != 0); prereq.id = atoi(tok);
					tok = strtok_s(0, ",", &cr); assert(tok != 0); prereq.level = atoi(tok);
					m_troopconfig[cfgid].buildings.push_back(prereq);
					x++;
				}

				strt = rs.value("prereqtech").convert<string>();
				str = (char*)strt.c_str();
				x = 0;

				for (tok = strtok_s(str, "|", &ch); tok != 0; tok = strtok_s(0, "|", &ch))
				{
					stPrereq prereq;
					tok = strtok_s(tok, ",", &cr); assert(tok != 0); prereq.id = atoi(tok);
					tok = strtok_s(0, ",", &cr); assert(tok != 0); prereq.level = atoi(tok);
					m_troopconfig[cfgid].techs.push_back(prereq);
					x++;
				}

				strt = rs.value("prereqitem").convert<string>();
				str = (char*)strt.c_str();
				x = 0;

				for (tok = strtok_s(str, "|", &ch); tok != 0; tok = strtok_s(0, "|", &ch))
				{
					stPrereq prereq;
					tok = strtok_s(tok, ",", &cr); assert(tok != 0); prereq.id = atoi(tok);
					tok = strtok_s(0, ",", &cr); assert(tok != 0); prereq.level = atoi(tok);
					m_troopconfig[cfgid].items.push_back(prereq);
					x++;
				}
			}
		}


		{
			Statement select(ses);
			select << "SELECT * FROM `config_research`;";
			select.execute();
			RecordSet rs(select);

			rs.moveFirst();


			for (int i = 0; i < rs.rowCount(); ++i, rs.moveNext())
			{
				char * tok;
				char * ch = 0, *cr = 0;

				int32_t cfgid = rs.value("researchid").convert<int32_t>();
				int32_t level = rs.value("level").convert<int32_t>() - 1;
				m_researchconfig[cfgid][level].time = rs.value("buildtime").convert<int32_t>();
				m_researchconfig[cfgid][level].destructtime = rs.value("buildtime").convert<int32_t>() / 2;
				m_researchconfig[cfgid][level].food = rs.value("food").convert<int32_t>();
				m_researchconfig[cfgid][level].wood = rs.value("wood").convert<int32_t>();
				m_researchconfig[cfgid][level].stone = rs.value("stone").convert<int32_t>();
				m_researchconfig[cfgid][level].iron = rs.value("iron").convert<int32_t>();
				m_researchconfig[cfgid][level].gold = rs.value("gold").convert<int32_t>();

				string strt = rs.value("prereqbuilding").convert<string>();
				char * str = (char*)strt.c_str();

				int x = 0;

				for (tok = strtok_s(str, "|", &ch); tok != 0; tok = strtok_s(0, "|", &ch))
				{
					stPrereq prereq;
					tok = strtok_s(tok, ",", &cr); assert(tok != 0); prereq.id = atoi(tok);
					tok = strtok_s(0, ",", &cr); assert(tok != 0); prereq.level = atoi(tok);
					m_researchconfig[cfgid][level].buildings.push_back(prereq);
					x++;
				}

				strt = rs.value("prereqtech").convert<string>();
				str = (char*)strt.c_str();
				x = 0;

				for (tok = strtok_s(str, "|", &ch); tok != 0; tok = strtok_s(0, "|", &ch))
				{
					stPrereq prereq;
					tok = strtok_s(tok, ",", &cr); assert(tok != 0); prereq.id = atoi(tok);
					tok = strtok_s(0, ",", &cr); assert(tok != 0); prereq.level = atoi(tok);
					m_researchconfig[cfgid][level].techs.push_back(prereq);
					x++;
				}

				strt = rs.value("prereqitem").convert<string>();
				str = (char*)strt.c_str();
				x = 0;

				for (tok = strtok_s(str, "|", &ch); tok != 0; tok = strtok_s(0, "|", &ch))
				{
					stPrereq prereq;
					tok = strtok_s(tok, ",", &cr); assert(tok != 0); prereq.id = atoi(tok);
					tok = strtok_s(0, ",", &cr); assert(tok != 0); prereq.level = atoi(tok);
					m_researchconfig[cfgid][level].items.push_back(prereq);
					x++;
				}
			}
		}


		{
			Statement select(ses);
			select << "SELECT * FROM `config_items`;";
			select.execute();
			RecordSet rs(select);

			rs.moveFirst();


			for (int i = 0; i < rs.rowCount(); ++i, rs.moveNext())
			{
				int32_t id = rs.value("id").convert<int32_t>();
				m_items[id].name = rs.value("name").convert<string>();
				m_items[id].cost = rs.value("cost").convert<int32_t>();
				m_items[id].saleprice = rs.value("cost").convert<int32_t>();
				m_items[id].buyable = rs.value("buyable").convert<bool>();
				m_items[id].cangamble = rs.value("cangamble").convert<bool>();
				m_items[id].daylimit = rs.value("daylimit").convert<int32_t>();
				m_items[id].type = rs.value("itemtype").convert<int32_t>();
				m_items[id].rarity = rs.value("rarity").convert<int32_t>();
				++m_itemcount;

				if (m_items[id].cangamble)
				{
					switch (m_items[id].rarity)
					{
						default:
						case 5:
							m_gambleitems.common.push_back(&m_items[id]);
							break;
						case 4:
							m_gambleitems.special.push_back(&m_items[id]);
							break;
						case 3:
							m_gambleitems.rare.push_back(&m_items[id]);
							break;
						case 2:
							m_gambleitems.superrare.push_back(&m_items[id]);
							break;
						case 1:
							m_gambleitems.ultrarare.push_back(&m_items[id]);
							break;
					}
				}
			}
		}
	}
	SQLCATCH(return;);

	consoleLogger->information("Loading map data.");


#ifndef DEF_NOMAPDATA
	{
		Session ses2(serverpool->get());
		Statement select(ses2);
		select << "SELECT `id`,`ownerid`,`type`,`level` FROM `tiles` ORDER BY `id` ASC;";
		select.execute();
		RecordSet rs(select);

		rs.moveFirst();

		do
		{
			int64_t id = rs.value("id").convert<int64_t>();
			int64_t ownerid = rs.value("ownerid").convert<int64_t>();
			int64_t type = rs.value("type").convert<int64_t>();
			int64_t level = rs.value("level").convert<int64_t>();

			map->m_tile[id].m_id = id;
			map->m_tile[id].m_ownerid = ownerid;
			map->m_tile[id].m_type = type;
			map->m_tile[id].m_level = level;

			if (type == NPC)
			{
				NpcCity * city = (NpcCity *)AddNpcCity(id);
				city->Initialize(true, true);
				city->m_level = level;
				city->m_ownerid = ownerid;
				map->m_tile[id].m_zoneid = map->GetStateFromID(id);
			}

			if ((id + 1) % ((mapsize*mapsize) / 100) == 0)
			{
				consoleLogger->information(Poco::format("%d%%", int((double(double(id + 1) / (mapsize*mapsize)))*double(100))));
			}
		} while (rs.moveNext());
	}
#else
	//this fakes map data
	for (int x = 0; x < (mapsize*mapsize); x += 1/*(mapsize*mapsize)/10*/)
	{
		map->m_tile[x].m_id = x;
		map->m_tile[x].m_ownerid = -1;
		//make every tile an npc
		//m_map->m_tile[x].m_type = NPC;
		map->m_tile[x].m_type = rand()%9 + 1;
		map->m_tile[x].m_level = (rand()%10)+1;

		if (map->m_tile[x].m_type > 6)
			map->m_tile[x].m_type = 10;

		if (map->m_tile[x].m_type == 6)
		{
			map->m_tile[x].m_type = NPC;
			NpcCity * city = (NpcCity *)AddNpcCity(map->m_tile[x].m_id);
			city->Initialize(true, true);
			city->m_level = map->m_tile[x].m_level;
			city->m_ownerid = map->m_tile[x].m_ownerid;
			map->m_tile[map->m_tile[x].m_id].m_zoneid = map->GetStateFromID(map->m_tile[x].m_id);
		}


		if ((x+1)%((mapsize*mapsize)/100) == 0)
		{
			consoleLogger->information(Poco::format("%d%%", int((double(double(x+1)/(mapsize*mapsize)))*double(100)) ));
		}
	}
#endif

	map->CalculateOpenTiles();

	consoleLogger->information("Loading account data.");

	uint64_t accountcount = 0;
	{
		Session ses2(serverpool->get());
		Statement select(ses2);
		select << "SELECT COUNT(*) AS a FROM `accounts`;";
		select.execute();
		RecordSet rs(select);

		rs.moveFirst();
		accountcount = rs.value("a").convert<uint64_t>();
	}

	{
		Session ses2(serverpool->get());
		Statement stmt = (ses2 << "SELECT accounts.*," + dbmaintable + ".account.email," + dbmaintable + ".account.password FROM accounts LEFT JOIN " + dbmaintable + ".account ON (" + dbmaintable + ".account.id=accounts.parentid) ORDER BY accounts.accountid ASC;");
		stmt.execute();
		bool test = stmt.done();
		RecordSet rs(stmt);

		rs.moveFirst();

		for (int i = 0; i < rs.rowCount(); ++i, rs.moveNext())
		{
			count++;

			Client * client = NewClient();
			client->m_accountexists = true;
			client->m_accountid = rs.value("accountid").convert<int64_t>();
			client->masteraccountid = rs.value("parentid").convert<int64_t>();
			client->m_playername = rs.value("username").convert<string>();
			client->m_password = rs.value("password").convert<string>();
			client->m_email = rs.value("email").convert<string>();

			client->m_allianceid = rs.value("allianceid").convert<int32_t>();
			client->m_alliancerank = rs.value("alliancerank").convert<int16_t>();
			client->m_lastlogin = rs.value("lastlogin").convert<double>();
			client->m_creation = rs.value("creation").convert<double>();
			client->m_status = rs.value("status").convert<int32_t>();
			client->m_sex = rs.value("sex").convert<int32_t>();
			client->m_flag = rs.value("flag").convert<string>();
			client->m_faceurl = rs.value("faceurl").convert<string>();
			client->m_cents = rs.value("cents").convert<int32_t>();
			client->Prestige(rs.value("prestige").convert<double>());
			client->m_honor = rs.value("honor").convert<double>();

			client->ParseBuffs(rs.value("buffs").convert<string>());
			client->ParseResearch(rs.value("research").convert<string>());
			client->ParseItems(rs.value("items").convert<string>());
			client->ParseMisc(rs.value("misc").convert<string>());

			client->CheckBeginner(false);

			if (accountcount > 101)
			{
				if ((count) % ((accountcount) / 100) == 0)
				{
					consoleLogger->information(Poco::format("%d%%", int((double(double(count) / accountcount + 1))*double(100))));
				}
			}
		}
	}


	consoleLogger->information("Loading city data.");



	uint64_t citycount = 0;
	{
		Session ses2(serverpool->get());
		Statement select(ses2);
		select << "SELECT COUNT(*) AS a FROM `cities`;";
		select.execute();
		RecordSet rs(select);

		rs.moveFirst();
		citycount = rs.value("a").convert<uint64_t>();
	}

	count = 0;


	{
		Session ses2(serverpool->get());
		Statement select(ses2);
		select << "SELECT * FROM `cities`";
		select.execute();
		RecordSet rs(select);

		rs.moveFirst();

		for (int i = 0; i < rs.rowCount(); ++i, rs.moveNext())
		{
			count++;

			int64_t accountid = rs.value("accountid").convert<int64_t>();
			int64_t cityid = rs.value("id").convert<int64_t>();
			int32_t fieldid = rs.value("fieldid").convert<int32_t>();
			Client * client = GetClient(accountid);
			GETXYFROMID(fieldid);
			if (client == 0)
			{
				consoleLogger->information(Poco::format("City exists with no account attached. - accountid:%?d cityid:%?d coord:(%?d,%?d)", accountid, cityid, xfromid, yfromid));
				continue;
			}
			PlayerCity * city = (PlayerCity *)AddPlayerCity(client, fieldid, cityid);
			city->m_client = client;
			city->m_resources.food = rs.value("food").convert<double>();
			city->m_resources.wood = rs.value("wood").convert<double>();
			city->m_resources.iron = rs.value("iron").convert<double>();
			city->m_resources.stone = rs.value("stone").convert<double>();
			city->m_resources.gold = rs.value("gold").convert<double>();
			city->m_cityname = rs.value("name").convert<string>();
			city->m_logurl = rs.value("logurl").convert<string>();
			city->m_tileid = fieldid;
			//		city->m_accountid = accountid;
			//		city->m_client = client;
			city->m_creation = rs.value("creation").convert<double>();

			//		client->m_citycount++;

			// 		server->m_map->m_tile[fieldid].m_city = city;
			// 		server->m_map->m_tile[fieldid].m_npc = false;
			// 		server->m_map->m_tile[fieldid].m_ownerid = accountid;
			// 		server->m_map->m_tile[fieldid].m_type = CASTLE;
			// 
			// 		//server->m_map->m_tile[fieldid].m_zoneid = server->m_map->GetStateFromID(fieldid);
			// 
			// 		server->m_map->m_tile[fieldid].m_castleid = cityid;


			city->ParseTroops(rs.value("troop").convert<string>());
			city->ParseBuildings(rs.value("buildings").convert<string>());
			city->ParseFortifications(rs.value("fortification").convert<string>());
			city->ParseMisc(rs.value("misc").convert<string>());

			//city->ParseHeroes(msql->GetString(i, "heroes"));
			//city->ParseTrades(msql->GetString(i, "trades"));
			//city->ParseArmyMovement(msql->GetString(i, "buffs"));

			{
				Session ses2(serverpool->get());
				Statement select2(ses2);
				select2 << "SELECT * FROM `heroes` WHERE `castleid`=?", use(city->m_castleid);
				select2.execute();
				RecordSet rs2(select2);

				rs2.moveFirst();

				for (int a = 0; a < rs2.rowCount(); ++a, rs2.moveNext())
				{
					Hero * temphero;
					temphero = new Hero();
					temphero->m_id = rs2.value("id").convert<uint64_t>();
					temphero->m_status = rs2.value("status").convert<int8_t>();
					temphero->m_itemid = rs2.value("itemid").convert<int32_t>();
					temphero->m_itemamount = rs2.value("itemamount").convert<int32_t>();

					temphero->m_basestratagem = rs2.value("basestratagem").convert<int16_t>();
					temphero->m_stratagem = rs2.value("stratagem").convert<int16_t>();
					temphero->m_stratagemadded = rs2.value("stratagemadded").convert<int16_t>();
					temphero->m_stratagembuffadded = rs2.value("stratagembuffadded").convert<int16_t>();
					temphero->m_basepower = rs2.value("basepower").convert<int16_t>();
					temphero->m_power = rs2.value("power").convert<int16_t>();
					temphero->m_poweradded = rs2.value("poweradded").convert<int16_t>();
					temphero->m_powerbuffadded = rs2.value("powerbuffadded").convert<int16_t>();
					temphero->m_basemanagement = rs2.value("basemanagement").convert<int16_t>();
					temphero->m_management = rs2.value("management").convert<int16_t>();
					temphero->m_managementadded = rs2.value("managementadded").convert<int16_t>();
					temphero->m_managementbuffadded = rs2.value("managementbuffadded").convert<int16_t>();
					temphero->m_logourl = rs2.value("logurl").convert<string>();
					temphero->m_name = rs2.value("name").convert<string>();
					temphero->m_remainpoint = rs2.value("remainpoint").convert<int16_t>();
					temphero->m_level = rs2.value("level").convert<int16_t>();
					temphero->m_upgradeexp = rs2.value("upgradeexp").convert<double>();
					temphero->m_experience = rs2.value("experience").convert<double>();
					temphero->m_loyalty = rs2.value("loyalty").convert<int8_t>();
					city->m_heroes[a] = temphero;
					city->m_heroes[a]->m_client = client;
					if (temphero->m_status == 1)
					{
						city->m_mayor = temphero;
					}
					if (temphero->m_id > m_heroid)
						m_heroid = temphero->m_id + 1;
				}
			}


			if (cityid >= m_cityid)
				m_cityid = cityid + 1;


			client->CalculateResources();

			if (citycount > 101)
			{
				if ((count) % ((citycount) / 100) == 0)
				{
					consoleLogger->information(Poco::format("%d%%", int((double(double(count) / citycount + 1))*double(100))));
				}
			}
		}
	}



	std::list<stTimedEvent>::iterator iter;
	std::list<Client*>::iterator playeriter;

	for (playeriter = players.begin(); playeriter != players.end(); ++playeriter)
	{
		Client * client = *playeriter;
		if (client)
		{
			for (int a = 0; a < 25; ++a)
			{
				if (client->m_research[a].castleid > 0)
				{
					PlayerCity * pcity = client->GetCity(client->m_research[a].castleid);
					if (pcity != 0)
					{
						pcity->m_researching = true;

						if (client->m_research[a].castleid != 0)
						{
							stResearchAction * ra = new stResearchAction;

							stTimedEvent te;
							ra->city = pcity;
							ra->client = pcity->m_client;
							ra->researchid = a;
							te.data = ra;
							te.type = DEF_TIMEDRESEARCH;

							AddTimedEvent(te);
						}
					}
					else
					{
						consoleLogger->information(Poco::format("Castleid does not exist for research! research:%d castleid:%d accountid:%d", a, client->m_research[a].castleid, client->m_accountid));
					}
				}
			}
		}
	}



	consoleLogger->information("Loading alliance data.");




	uint64_t alliancecount = 0;
	{
		Session ses2(serverpool->get());
		Statement select(ses2);
		select << "SELECT COUNT(*) AS a FROM `alliances`";
		select.execute();
		RecordSet rs(select);

		rs.moveFirst();
		alliancecount = rs.value("a").convert<uint64_t>();
	}

	count = 0;


	{
		Session ses2(serverpool->get());
		Statement select(ses2);
		select << "SELECT * FROM `alliances`";
		select.execute();
		RecordSet rs(select);

		rs.moveFirst();



		Alliance * alliance;

		for (int i = 0; i < rs.rowCount(); ++i, rs.moveNext())
		{
			count++;
			alliance = m_alliances->CreateAlliance(rs.value("name").convert<string>(), rs.value("leader").convert<int64_t>(), rs.value("id").convert<int64_t>());
			//alliance->m_allianceid = msql->GetInt(i, "id");
			alliance->ParseMembers(rs.value("members").convert<string>());
			alliance->ParseRelation(&alliance->m_enemies, rs.value("enemies").convert<string>());
			alliance->ParseRelation(&alliance->m_allies, rs.value("allies").convert<string>());
			alliance->ParseRelation(&alliance->m_neutral, rs.value("neutrals").convert<string>());
			alliance->m_name = rs.value("name").convert<string>();
			alliance->m_founder = rs.value("founder").convert<string>();
			alliance->m_note = rs.value("note").convert<string>();

			if (alliance->m_allianceid >= m_allianceid)
				m_allianceid = alliance->m_allianceid + 1;


			if (alliancecount > 101)
				if ((count) % ((alliancecount) / 100) == 0)
				{
				consoleLogger->information(Poco::format("%d%%", int((double(double(count) / alliancecount + 1))*double(100))));
				}
		}
	}


	consoleLogger->information("Incrementing valleys.");


	for (int i = 0; i < mapsize*mapsize; ++i)
	{
		if (map->m_tile[i].m_type < 11 && map->m_tile[i].m_ownerid < 0)
		{
			map->m_tile[i].m_level++;
			if (map->m_tile[i].m_level > 10)
				map->m_tile[i].m_level = 1;
		}
	}
#pragma endregion

	unsigned uAddr;

	SortPlayers();
	SortHeroes();
	SortCastles();
	//m_alliances->SortAlliances();

	TimerThreadRunning = true;
	std::thread timerthread(std::bind(std::mem_fun(&Server::TimerThread), this));


	//SOCKET THREADS

	// Create a pool of threads to run all of the io_services.
	std::vector<boost::shared_ptr<boost::thread> > threads;
	for (std::size_t i = 0; i < /*thread_pool_size_*/1; ++i)
	{
		boost::shared_ptr<boost::thread> thread(new boost::thread(
			boost::bind(&Server::io_thread, this)));
		threads.push_back(thread);
	}

	// Wait for all threads in the pool to exit.
	for (std::size_t i = 0; i < threads.size(); ++i)
		threads[i]->join();
	//io_service_.run();

	timerthread.join();

	//	boost::shared_ptr<boost::thread> savethread(new boost::thread(SaveData));
	//	savethread->join();

	lua_close(L);
}

void Server::io_thread()
{
	// Ensures rand is seeded for io threads.
	srand(unixtime());
	io_service_.run();
}

void Server::stop()
{
	// Post a call to the stop function so that server::stop() is safe to call
	// from any thread.
	io_service_.post([this](){
		Shutdown();
	});
}

void Server::start(connection_ptr c)
{
	boost::asio::ip::tcp::endpoint endp = c->socket().remote_endpoint();
	boost::asio::ip::address address = endp.address();
	c->address = address.to_string();
	consoleLogger->information(Poco::format("Client connected %s", c->address));

	connections_.insert(c);
	c->start();
}

void Server::stop(connection_ptr c)
{
	try
	{
		connections_.erase(c);
		c->stop();
	}
	catch (std::exception& e)
	{
		std::cerr << "exception: " << e.what() << "\n";
	}
}

void Server::stop_all()
{
	std::for_each(connections_.begin(), connections_.end(),
				  boost::bind(&connection::stop, _1));
	connections_.clear();
}

bool Server::Init()
{
	consoleLogger->information("Loading Config.");
	if (luaL_dofile(L, "config.lua") != 0)
	{
		consoleLogger->information(Poco::format("%s", lua_tostring(L, -1)));
		return false;
	}
	lua_getglobal(L, "config");

	char * temp;

	//ip address
	{
		lua_getfield(L, -1, "bindip");
		temp = (char*)lua_tostring(L, -1);
		if (temp == 0) { consoleLogger->information("Invalid bindip setting."); return false; }
		bindaddress = temp;
		if (bindaddress.length() == 0 || bindaddress.length() > 15)
		{
			consoleLogger->information("Invalid bindip setting.");
			return false;
		}
		lua_pop(L, 1);
	}

	//player connect port
	{
		uint16_t tbp = 0;
		lua_getfield(L, -1, "bindport");
		temp = (char*)lua_tostring(L, -1);
		if (temp == 0) { consoleLogger->information("Invalid bindport setting."); return false; }
		tbp = atoi(temp);
		if (tbp < 1 || tbp > 65534)
		{
			consoleLogger->information("Invalid bindport setting.");
			return false;
		}
		bindport = temp;
		lua_pop(L, 1);
	}

	//maxplayers
	{
		lua_getfield(L, -1, "maxplayers");
		temp = (char*)lua_tostring(L, -1);
		if (temp == 0) { consoleLogger->information("Invalid maxplayers setting."); return false; }
		maxplayers = atoi(temp);
		lua_pop(L, 1);
	}

	//mapsize
	{
		lua_getfield(L, -1, "mapsize");
		temp = (char*)lua_tostring(L, -1);
		if (temp == 0) { consoleLogger->information("Invalid mapsize setting."); return false; }
		mapsize = atoi(temp);
		lua_pop(L, 1);
//		map = new Map(this, mapsize);
	}


	//main table name
	{
		lua_getfield(L, -1, "dbmaintable");
		temp = (char*)lua_tostring(L, -1);
		if (temp == 0) { consoleLogger->information("Invalid dbmaintable setting."); return false; }
		dbmaintable = temp;
		lua_pop(L, 1);
	}

	//server table name
	{
		lua_getfield(L, -1, "dbservertable");
		temp = (char*)lua_tostring(L, -1);
		if (temp == 0) { consoleLogger->information("Invalid dbservertable setting."); return false; }
		dbservertable = temp;
		lua_pop(L, 1);
	}

	//sql host
	{
		lua_getfield(L, -1, "sqlhost");
		temp = (char*)lua_tostring(L, -1);
		if (temp == 0) { consoleLogger->information("Invalid sqlhost setting."); return false; }
		sqlhost = temp;
		lua_pop(L, 1);
	}

	//sql user
	{
		lua_getfield(L, -1, "sqluser");
		temp = (char*)lua_tostring(L, -1);
		if (temp == 0) { consoleLogger->information("Invalid sqluser setting."); return false; }
		sqluser = temp;
		lua_pop(L, 1);
	}

	//sql pass
	{
		lua_getfield(L, -1, "sqlpass");
		temp = (char*)lua_tostring(L, -1);
		if (temp == 0) { consoleLogger->information("Invalid sqlpass setting."); return false; }
		sqlpass = temp;
		lua_pop(L, 1);
	}

	//server name
	{
		lua_getfield(L, -1, "servername");
		temp = (char*)lua_tostring(L, -1);
		if (temp == 0) { consoleLogger->information("Invalid servername setting."); return false; }
		servername = temp;
		lua_pop(L, 1);
	}

	lua_pop(L, 1);

	return true;
}

void Server::Shutdown()
{
	stop();

	//stop timer thread so the save
	//thread gets an accurate snapshot
	serverstatus = SERVERSTATUS_SHUTDOWN;

	//disconnect everyone
	MassDisconnect();

	acceptor_.close();
	stop_all();

	//shutdown complete
	//do anything else that might need to be done
	io_service_.stop();
}

bool Server::ConnectSQL()
{
	try
	{
		accountpool = new SessionPool("MySQL", "host=" + sqlhost + ";port=3306;db=" + dbmaintable + ";user=" + sqluser + ";password=" + sqlpass + ";compress=true;auto-reconnect=true");
		serverpool = new SessionPool("MySQL", "host=" + sqlhost + ";port=3306;db=" + dbservertable + ";user=" + sqluser + ";password=" + sqlpass + ";compress=true;auto-reconnect=true");
	}
	catch (Poco::Exception& exc)
	{
		std::cerr << exc.displayText() << std::endl;
		return false;
	}
	return true;
}

bool Server::InitSockets()
{
	// Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
	boost::asio::ip::tcp::resolver resolver(io_service_);
	boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve({ bindaddress, bindport });
	acceptor_.open(endpoint.protocol());
	acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	bool test = true;
	try {
		acceptor_.bind(endpoint);
	}
	catch (std::exception& e)
	{
		test = false;
	}
	if (test == false)
	{
		throw std::runtime_error("Invalid bind address or port 443 already in use! Exiting.");
	}

	// Finally listen on the socket and start accepting connections
	acceptor_.listen();
	do_accept();
	return true;
}

void Server::do_accept()
{
	acceptor_.async_accept(socket_,
						   [this](boost::system::error_code ec)
	{
		// Check whether the server was stopped by a signal before this
		// completion handler had a chance to run.
		if (!acceptor_.is_open())
		{
			return;
		}

		if (!ec)
		{
			start(std::make_shared<connection>(
				std::move(socket_), *this, request_handler_));
		}

		do_accept();
	});
}

// Timers

void Server::TimerThread()
{
#ifndef WIN32
	struct timespec req = { 0 };
	req.tv_sec = 0;
	req.tv_nsec = 1000000L;//1ms
#else
	_tzset();
#endif

	std::shared_ptr<std::thread> savethread;

	uint64_t t1htimer;
	uint64_t t30mintimer;
	uint64_t t6mintimer;
	uint64_t t5mintimer;
	uint64_t t3mintimer;
	uint64_t t1mintimer;
	uint64_t t5sectimer;
	uint64_t t1sectimer;
	uint64_t t100msectimer;
	uint64_t ltime;

	std::list<stTimedEvent>::iterator iter;
	std::list<Client*>::iterator playeriter;

	t1htimer = t30mintimer = t6mintimer = t5mintimer = t3mintimer = t1mintimer = t5sectimer = t1sectimer = t100msectimer = unixtime();

 	while (serverstatus == SERVERSTATUS_ONLINE)
 	{
 		try {
 			ltime = ltime = unixtime();
 
			if (t1sectimer < ltime)
			{
				t1sectimer += 1000;
			}
			if (t100msectimer < ltime)
			{
				for (playeriter = players.begin(); playeriter != players.end(); ++playeriter)
				{
					Client * client = *playeriter;
					client->lists.lock();
					for (int j = 0; j < client->m_city.size(); ++j)
					{
						PlayerCity * city = client->m_city[j];
						std::vector<stTroopQueue>::iterator tqiter;
						for (tqiter = city->m_troopqueue.begin(); tqiter != city->m_troopqueue.end();)
						{
							std::list<stTroopTrain>::iterator iter;
							iter = tqiter->queue.begin();
							if (iter != tqiter->queue.end() && iter->endtime <= ltime)
							{
								//troops done training
								double gain = iter->count * GetPrestigeOfAction(DEF_TRAIN, iter->troopid, 1, city->m_level);
								client->Prestige(gain);
								client->PlayerUpdate();
								if (city->m_mayor)
								{
									city->m_mayor->m_experience += gain;
									city->HeroUpdate(city->m_mayor, 2);
								}

								if (tqiter->positionid == -2)
								{
									city->SetForts(iter->troopid, iter->count);
								}
								else
									city->SetTroops(iter->troopid, iter->count);
								tqiter->queue.erase(iter++);
								iter->endtime = ltime + iter->costtime;
							}
							++tqiter;
						}
					}
					client->lists.unlock();
				}

				if (armylist.size() > 0)
				{
					for (iter = armylist.begin(); iter != armylist.end();)
					{
						armylist.erase(iter++);
						// 						stArmyMovement * am = (stArmyMovement *)iter->data;
						// 						PlayerCity * fcity = (PlayerCity *)am->city;
						// 						Client * fclient = am->client;
						// 						Hero * fhero = am->hero;
						// 
						// 						fclient->SelfArmyUpdate();
						// 
						// 						stTimedEvent te;
						// 						te.data = am;
						// 						te.type = DEF_TIMEDARMY;
						// 
						// 						gserver->AddTimedEvent(te);

						// recv: server.HeroUpdate
						// recv: server.InjuredTroopUpdate
						// recv: server.SelfArmysUpdate
						// recv: server.SelfArmysUpdate
						// recv: server.HeroUpdate
						// recv: server.SelfArmysUpdate
						// recv: server.HeroUpdate
						// recv: server.NewReport
						// recv: server.SelfArmysUpdate
						// recv: server.HeroUpdate
						// recv: server.TroopUpdate
						// recv: server.SelfArmysUpdate
						// recv: server.HeroUpdate
						// recv: server.SystemInfoMsg
						// recv: server.SelfArmysUpdate
						// recv: server.ResourceUpdate
						// recv: server.SelfArmysUpdate
					}
				}

				//TODO: some buildings not being set to notupgrading properly. add a new check for all buildings under construction to see if they are due to finish? maybe?
				if (buildinglist.size() > 0)
				{
					for (iter = buildinglist.begin(); iter != buildinglist.end();)
					{
						stBuildingAction * ba = (stBuildingAction *)iter->data;
						Client * client = ba->client;
						PlayerCity * city = ba->city;
						stBuilding * bldg = ba->city->GetBuilding(ba->positionid);
						if (bldg->endtime < ltime)
						{
							if (bldg->status == 1)
							{
								//build/upgrade
								bldg->status = 0;
								bldg->level++;
								ba->city->SetBuilding(bldg->type, bldg->level, ba->positionid, 0, 0.0, 0.0);

								if (bldg->type == 21)
								{
									for (int i = 0; i < 10; ++i)
									{
										if (city->m_innheroes[i])
										{
											delete city->m_innheroes[i];
											city->m_innheroes[i] = 0;
										}
									}
								}

								amf3object obj = amf3object();
								obj["cmd"] = "server.BuildComplate";
								obj["data"] = amf3object();

								amf3object & data = obj["data"];

								data["buildingBean"] = bldg->ToObject();
								data["castleId"] = city->m_castleid;

								buildinglist.erase(iter++);

								double gain = GetPrestigeOfAction(DEF_BUILDING, bldg->type, bldg->level, city->m_level);
								client->Prestige(gain);

								delete ba;

								client->CalculateResources();
								city->CalculateStats();
								if (city->m_mayor)
								{
									city->m_mayor->m_experience += gain;
									city->HeroUpdate(city->m_mayor, 2);
								}
								//city->CastleUpdate();
								client->PlayerUpdate();
								city->ResourceUpdate();

								SendObject(client, obj);

								continue;
							}
							else if (bldg->status == 2)
							{
								//destruct
								bldg->status = 0;
								bldg->level--;

								stResources res;
								res.food = m_buildingconfig[bldg->type][bldg->level].food / 3;
								res.wood = m_buildingconfig[bldg->type][bldg->level].wood / 3;
								res.stone = m_buildingconfig[bldg->type][bldg->level].stone / 3;
								res.iron = m_buildingconfig[bldg->type][bldg->level].iron / 3;
								res.gold = m_buildingconfig[bldg->type][bldg->level].gold / 3;
								ba->city->m_resources += res;

								if (bldg->level == 0)
									ba->city->SetBuilding(0, 0, ba->positionid, 0, 0.0, 0.0);
								else
									ba->city->SetBuilding(bldg->type, bldg->level, ba->positionid, 0, 0.0, 0.0);

								delete ba;

								client->CalculateResources();
								city->CalculateStats();


								amf3object obj = amf3object();
								obj["cmd"] = "server.BuildComplate";
								obj["data"] = amf3object();

								amf3object & data = obj["data"];

								data["buildingBean"] = bldg->ToObject();
								data["castleId"] = city->m_castleid;

								SendObject(client, obj);

								buildinglist.erase(iter++);

								client->CalculateResources();
								city->CalculateStats();
								city->ResourceUpdate();


								continue;
							}
						}
						++iter;
					}
				}

				if (researchlist.size() > 0)
				{
					for (iter = researchlist.begin(); iter != researchlist.end();)
					{
						stResearchAction * ra = (stResearchAction *)iter->data;
						Client * client = ra->client;
						PlayerCity * city = ra->city;
						if (ra->researchid != 0)
						{
							if (client->m_research[ra->researchid].endtime < ltime)
							{
								city->m_researching = false;
								client->m_research[ra->researchid].level++;
								client->m_research[ra->researchid].endtime = 0;
								client->m_research[ra->researchid].starttime = 0;
								client->m_research[ra->researchid].castleid = 0;

								amf3object obj = amf3object();
								obj["cmd"] = "server.ResearchCompleteUpdate";
								obj["data"] = amf3object();

								amf3object & data = obj["data"];

								data["castleId"] = city->m_castleid;


								researchlist.erase(iter++);

								double gain = GetPrestigeOfAction(DEF_RESEARCH, ra->researchid, client->m_research[ra->researchid].level, city->m_level);
								client->Prestige(gain);


								client->CalculateResources();
								city->CalculateStats();
								if (city->m_mayor)
								{
									city->m_mayor->m_experience += gain;
									city->HeroUpdate(city->m_mayor, 2);
								}
								//city->CastleUpdate();
								client->PlayerUpdate();
								city->ResourceUpdate();

								SendObject(client, obj);

								delete ra;

								continue;
							}
						}
						else
						{
							researchlist.erase(iter++);
							continue;
						}
						++iter;
					}
				}
				t100msectimer += 100;
			}
			if (t5sectimer < ltime)
			{
				SortPlayers();
				SortHeroes();
				SortCastles();
				m_alliances->SortAlliances();
				for (playeriter = players.begin(); playeriter != players.end(); ++playeriter)
				{
					Client * client = *playeriter;
					for (int j = 0; j < client->m_buffs.size(); ++j)
					{
						if (client->m_buffs[j].id.length() != 0)
						{
							if (ltime > client->m_buffs[j].endtime)
							{
								if (client->m_buffs[j].id == "PlayerPeaceBuff")
								{
									client->SetBuff("PlayerPeaceCoolDownBuff", "Truce Agreement in cooldown.", ltime + (12 * 60 * 60 * 1000));
								}
								client->RemoveBuff(client->m_buffs[j].id);
							}
						}
					}
				}

				t5sectimer += 5000;
			}
			if (t1mintimer < ltime)
			{
				for (playeriter = players.begin(); playeriter != players.end(); ++playeriter)
				{
					Client * client = *playeriter;
					client->CalculateResources();

					if (client->m_socknum > 0 && (client->m_currentcityindex != -1) && client->m_city[client->m_currentcityindex])
					{
						client->m_city[client->m_currentcityindex]->ResourceUpdate();
					}
				}

				CheckRankSearchTimeouts(ltime);

				t1mintimer += 60000;
			}
			if (t3mintimer < ltime)
			{
				//				if (!savethreadrunning)
				//					savethread = shared_ptr<thread>(new thread(std::bind(std::mem_fun(&Server::SaveData), this)));
				//				std::thread timerthread(std::bind(std::mem_fun(&Server::TimerThread), gserver));
				// 				//hSaveThread = (HANDLE)_beginthreadex(0, 0, SaveData, 0, 0, &uAddr);
				//t3mintimer += 180000;
				t3mintimer += 180000;
			}
			// 			if (t5mintimer < ltime)
			// 			{
			// #ifndef WIN32
			// 			if (pthread_create(&hSaveThread, NULL, SaveData, 0))
			// 			{
			// 				SFERROR("pthread_create");
			// 			}
			// #else
			// 				//hSaveThread = (HANDLE)_beginthreadex(0, 0, SaveData, 0, 0, &uAddr);
			// #endif
			// 				t5mintimer += 300000;
			// 			}
			if (t6mintimer < ltime)
			{
				try
				{
					for (int i = 0; i < m_city.size(); ++i)
					{
						if (m_city.at(i)->m_type == NPC)
						{
							((NpcCity*)m_city.at(i))->CalculateStats(true, true);
						}
						else if (m_city.at(i)->m_type == CASTLE)
						{
							((PlayerCity*)m_city.at(i))->RecalculateCityStats();
						}
					}
				}
				catch (...)
				{
					consoleLogger->information(Poco::format("exception: %s : %?d", (string)__FILE__, __LINE__));
					//error?
				}
				t6mintimer += 360000;
			}
			// 			if (t30mintimer < ltime)
			// 			{
			// 				LOCK(M_RANKEDLIST);
			// 				gserver->SortPlayers();
			// 				gserver->SortHeroes();
			// 				gserver->SortCastles();
			// 				gserver->m_alliances->SortAlliances();
			// 				UNLOCK(M_RANKEDLIST);
			// 				t30mintimer += 1800000;
			// 			}
			if (t1htimer < ltime)
			{

				t1htimer += 3600000;
			}
#ifdef WIN32
			Sleep(1);
#else
			nanosleep(&req, NULL);
#endif
		}
		catch (...)
		{
			consoleLogger->information("uncaught TimeThread() exception");
			TimerThreadRunning = false;
		}
	}
	TimerThreadRunning = false;
	return;
}

void Server::SaveThread()
{
	SaveThreadRunning = true;
	consoleLogger->information("Saving data.");

	// #ifdef __WIN32__
	// 	_endthread();
	// 	return 0;
	// #endif

	Map * newmap;
	Client * client;

	// 	newmap = new Map(server);
	// 
	// 	Log("Loading old map data.");
	// 
	// 	for (int x = 0; x < (DEF_MAPSIZE*DEF_MAPSIZE); x += (DEF_MAPSIZE*DEF_MAPSIZE)/10)
	// 	{
	// 		char query[1024];
	// 		sprintf(query, "SELECT `id`,`ownerid`,`type`,`level` FROM `tiles` ORDER BY `id` ASC LIMIT %d,%d", x, ((DEF_MAPSIZE*DEF_MAPSIZE)/10));
	// 		mysql_query(msql->mySQL, query);
	// 		st_mysql_res* m_pQueryResult;
	// 		m_pQueryResult = mysql_store_result(msql->mySQL);
	// 		int m_iRows = (int)mysql_num_rows(m_pQueryResult);
	// 		int m_iFields = mysql_num_fields(m_pQueryResult);
	// 		//MYSQL_FIELD ** field = new MYSQL_FIELD*[m_iFields+1];;
	// 		MYSQL_ROW myRow;
	// 		mysql_field_seek(m_pQueryResult, 0);
	// 
	// 
	// 
	// 		for (int i = 0; i < m_iRows; ++i)
	// 		{
	// 			myRow = mysql_fetch_row(m_pQueryResult);
	// 			int64_t id = _atoi64(myRow[0]);
	// 			int64_t ownerid = _atoi64(myRow[1]);
	// 			int64_t type = _atoi64(myRow[2]);
	// 			int64_t level = _atoi64(myRow[3]);
	// 
	// 			newmap->m_tile[id].m_id = id;
	// 			newmap->m_tile[id].m_ownerid = ownerid;
	// 			newmap->m_tile[id].m_type = type;
	// 			newmap->m_tile[id].m_level = level;
	// 
	// 			if ((id+1)%((DEF_MAPSIZE*DEF_MAPSIZE)/10) == 0)
	// 			{
	// 				Log("%d%%", int((double(double(id+1)/(DEF_MAPSIZE*DEF_MAPSIZE)))*double(100)));
	// 			}
	// 		}
	// 		mysql_free_result(m_pQueryResult);
	// 	}
	// 
	// 	msql->Reset();

	consoleLogger->information("Saving tile data.");

	// 	typedef Poco::Tuple<uint8_t, uint8_t, uint32_t, uint16_t> tiledata;
	// 	typedef std::vector<tiledata> tiledatalist;
	// 
	// 	mtxlist.tiledata.lock();
	// 	tiledatalist datalist;
	// 	for (uint32_t i = 0; i < map->mapsize*map->mapsize; ++i)
	// 	{
	// 		tiledata data(map->m_tile->m_level, map->m_tile->m_type, map->m_tile->m_ownerid, map->m_tile->m_id);
	// 		datalist.push_back(data);
	// 	}
	// 	mtxlist.tiledata.unlock();
	// 
	// 	try
	// 	{
	// 		Session ses(serverpool->get());
	// 		ses << "UPDATE tiles SET level=?,`type`=?,ownerid=? WHERE id=?;", use(datalist), now;
	// 	}
	// 	SQLCATCH(void(0));

	consoleLogger->information("Tile data saved.");

#pragma region old save
	//old save
	/*string updatestr = "";
	char temp[40];
	bool btype, blevel, bowner;
	for (int i = 0; i < DEF_MAPSIZE*DEF_MAPSIZE; ++i)
	{
	btype = blevel = bowner = false;
	updatestr += "UPDATE `tiles` SET ";
	if (newmap->m_tile[i].m_type != server->m_map->m_tile[i].m_type)
	{
	sprintf_s(temp, 40, "`type`=%d", server->m_map->m_tile[i].m_type);
	updatestr += temp;
	btype = true;
	}
	if (newmap->m_tile[i].m_level != server->m_map->m_tile[i].m_level)
	{
	if (btype)
	{
	sprintf_s(temp, 40, ",`level`=%d", server->m_map->m_tile[i].m_level);
	updatestr += temp;
	}
	else
	{
	sprintf_s(temp, 40, "`level`=%d", server->m_map->m_tile[i].m_level);
	updatestr += temp;
	}
	blevel = true;
	}
	if (newmap->m_tile[i].m_ownerid != server->m_map->m_tile[i].m_ownerid)
	{
	if (blevel || btype)
	{
	sprintf_s(temp, 40, ",`ownerid`=%d", server->m_map->m_tile[i].m_ownerid);
	updatestr += temp;
	}
	else
	{
	sprintf_s(temp, 40, "`ownerid`=%d", server->m_map->m_tile[i].m_ownerid);
	updatestr += temp;
	}
	bowner = true;
	}
	if (btype || blevel || bowner)
	{
	sprintf_s(temp, 40, " WHERE `id`=%d; ", i);
	updatestr += temp;
	}
	if ((i+1)%((DEF_MAPSIZE*DEF_MAPSIZE)/10) == 0)
	{
	Log("%d%%", int((double(double(i+1)/(DEF_MAPSIZE*DEF_MAPSIZE)))*double(100)));
	}
	//if (i%10 == 9)
	if (i%5 == 4)
	{
	msql->Update((char*)updatestr.c_str());
	msql->Reset();
	updatestr = "";
	}

	}*/
#pragma endregion

	/*
	*
	*
	UPDATE `tiles`
	SET `level` = CASE `id`
	WHEN 0 THEN 8
	WHEN 1 THEN 2
	WHEN 2 THEN 4
	END
	WHERE `id` IN (0,1,2);
	**/


	/*

	vector<int32_t>::iterator iter;

	LOCK(M_DELETELIST);
	if (gserver->m_deletedhero.size() > 0)
	{
	for (iter = gserver->m_deletedhero.begin(); iter != gserver->m_deletedhero.end(); ++iter)
	{
	sql3->Query("DELETE FROM `heroes` WHERE `id`=%d LIMIT 1", *iter);
	}
	}
	if (gserver->m_deletedcity.size() > 0)
	{
	for (iter = gserver->m_deletedcity.begin(); iter != gserver->m_deletedcity.end(); ++iter)
	{
	sql3->Query("DELETE FROM `cities` WHERE `id`=%d LIMIT 1", *iter);
	}
	}
	UNLOCK(M_DELETELIST);

	try
	{
	LOCK(M_MAP);
	string fullstr = "";
	string updatestr = "UPDATE `tiles` SET ";
	string typehstr  = " `type` = CASE `id` ";
	string levelhstr = " `level` = CASE `id` ";
	string ownerhstr = " `ownerid` = CASE `id` ";
	string typestr  = "";
	string levelstr = "";
	string ownerstr = "";
	string endstr = " END, ";
	string endstr2 = " END ";
	string wherestr = "WHERE `id` IN (";
	string instr  = "";
	char temp[40];
	bool btype, blevel, bowner;
	for (int i = 0; i < map->mapsize*map->mapsize; ++i)
	{
	stringstream ss;
	ss << " WHEN " << i << " THEN " << (int32_t)map->m_tile[i].m_type;
	typestr += ss.str();
	ss.str("");
	ss << " WHEN " << i << " THEN " << (int32_t)map->m_tile[i].m_level;
	levelstr += ss.str();
	ss.str("");
	ss << " WHEN " << i << " THEN " << (int32_t)map->m_tile[i].m_ownerid;
	ownerstr += ss.str();
	ss.str("");
	ss << i;
	instr += ss.str();
	ss.str("");

	if (i%200 == 10)//199)
	{
	fullstr = updatestr + typehstr + typestr + endstr + levelhstr + levelstr + endstr + ownerhstr + ownerstr + endstr2 + wherestr + instr;
	fullstr += ");";
	sql3->Query(fullstr);
	instr = typestr = levelstr = ownerstr = fullstr = "";
	}
	else
	{
	instr += ",";
	}
	if ((i+1)%((map->mapsize*map->mapsize)/20) == 0)
	{
	Log("%d%%", int((double(double(i+1)/(map->mapsize*map->mapsize)))*double(100)));
	}
	}
	UNLOCK(M_MAP);
	}
	catch (std::exception& e)
	{
	Log("SaveData() Exception: %s", e.what());
	UNLOCK(M_MAP);
	}
	catch(...)
	{
	Log("SaveData() Exception.");
	UNLOCK(M_MAP);
	}
	//delete newmap;


	Log("Saving alliance data.");

	try
	{
	LOCK(M_ALLIANCELIST);
	for (int i = 0; i < DEF_MAXALLIANCES; ++i)
	{
	Alliance * alliance = gserver->m_alliances->m_alliances[i];
	if (!alliance)
	continue;

	ResultSet * res = sql3->QueryRes("SELECT COUNT(*) AS a FROM `alliances` WHERE `id`=%d", alliance->m_allianceid);
	res->next();
	int alliancecount = res->getInt("a");
	delete res;

	string members;
	string enemies;
	string allies;
	string neutrals;


	stringstream ss;

	for (int j = 0; j < alliance->m_members.size(); ++j)
	{
	if (j != 0)
	ss << "|";
	Client * temp = gserver->GetClient(alliance->m_members[j].clientid);
	ss << temp->m_accountid << "," << temp->m_alliancerank;
	}
	members = ss.str();
	ss.str("");
	for (int j = 0; j < alliance->m_enemies.size(); ++j)
	{
	if (j != 0)
	ss << "|";
	ss << alliance->m_enemies[j];
	}
	enemies = ss.str();
	ss.str("");
	for (int j = 0; j < alliance->m_allies.size(); ++j)
	{
	if (j != 0)
	ss << "|";
	ss << alliance->m_allies[j];
	}
	allies = ss.str();
	ss.str("");
	for (int j = 0; j < alliance->m_neutral.size(); ++j)
	{
	if (j != 0)
	ss << "|";
	ss << alliance->m_neutral[j];
	}
	neutrals = ss.str();
	ss.str("");

	members = makesafe(members);
	enemies = makesafe(enemies);
	allies = makesafe(allies);
	neutrals = makesafe(neutrals);

	string note, intro, motd;

	note = makesafe(alliance->m_note);
	intro = makesafe(alliance->m_intro);
	motd = makesafe(alliance->m_motd);


	if (alliancecount > 0)
	{
	//update
	sql3->Query("UPDATE `alliances` SET `leader`=%d,`name`='%s',`founder`='%s',`note`='%s',`intro`='%s',`motd`='%s',`members`='%s',`enemies`='%s',`allies`='%s',`neutrals`='%s' WHERE `id`=%d LIMIT 1;",
	alliance->m_ownerid, alliance->m_name.c_str(), alliance->m_founder.c_str(), note.c_str(), intro.c_str(), motd.c_str(), members.c_str(), enemies.c_str(), allies.c_str(), neutrals.c_str(), alliance->m_allianceid);
	}
	else
	{
	//insert
	sql3->Query("INSERT INTO `alliances` (`id`,`leader`,`name`,`founder`,`note`,`intro`,`motd`,`members`,`enemies`,`allies`,`neutrals`) VALUES (%d,%d,'%s','%s','%s','%s','%s','%s','%s','%s','%s');",
	alliance->m_allianceid, alliance->m_ownerid,alliance->m_name.c_str(), alliance->m_founder.c_str(), alliance->m_note.c_str(), alliance->m_intro.c_str(), alliance->m_motd.c_str(), members.c_str(), enemies.c_str(), allies.c_str(), neutrals.c_str());
	}
	}
	UNLOCK(M_ALLIANCELIST);
	}
	catch (std::exception& e)
	{
	Log("SaveData() Exception: %s", e.what());
	UNLOCK(M_ALLIANCELIST);
	}
	catch(...)
	{
	Log("SaveData() Exception.");
	UNLOCK(M_ALLIANCELIST);
	}

	Log("Saving player data.");

	char temp1[1000];

	try
	{
	LOCK(M_CLIENTLIST);
	for (int i = 0; i < gserver->maxplayers; ++i)
	{
	if (!gserver->m_clients[i])
	continue;
	Client * tempclient = gserver->m_clients[i];
	string buffs;
	string research;
	string items;
	string misc;

	memset(temp1, 0, 1000);

	stResearch * rsrch;
	for (int j = 0; j < 25; ++j)
	{
	if (j != 0)
	research += "|";
	rsrch = &gserver->m_clients[i]->m_research[j];
	sprintf_s(temp1, 1000, "%d,%d,%d,"DBL","DBL"", j, rsrch->level, rsrch->castleid, rsrch->starttime, rsrch->endtime);
	research += temp1;
	}

	for (int j = 0; j < gserver->m_clients[i]->m_buffs.size(); ++j)
	{
	if (gserver->m_clients[i]->m_buffs[j].id.length() > 0)
	{
	if (j != 0)
	buffs += "|";
	sprintf_s(temp1, 1000, "%s,%s,"DBL"", (char*)gserver->m_clients[i]->m_buffs[j].id.c_str(), (char*)gserver->m_clients[i]->m_buffs[j].desc.c_str(), gserver->m_clients[i]->m_buffs[j].endtime);
	buffs += temp1;
	}
	}

	//sprintf_s(temp1, 1000, "%d", tempclient->m_cents);
	//misc = temp1;

	{
	stringstream ss;
	for (int j = 1; j < DEF_MAXITEMS; ++j)
	{
	if (gserver->m_clients[i]->m_items[j].id.length() > 0)
	{
	if (j != 1)
	ss << "|";
	ss << gserver->m_clients[i]->m_items[j].id << "," << gserver->m_clients[i]->m_items[j].count;
	}
	}
	items = ss.str();
	}
	{
	stringstream ss;
	ss << tempclient->m_icon << "," << tempclient->m_allianceapply << "," << tempclient->m_changedface;
	misc = ss.str();
	}
	sql3->Query("UPDATE `accounts` SET `username`='%s',`lastlogin`="DBL",`ipaddress`='%s',`status`=%d,`buffs`='%s',`flag`='%s',`faceurl`='%s',`research`='%s',`items`='%s',`allianceid`=%d,`alliancerank`=%d,`misc`='%s',`cents`=%d,`prestige`="DBL",`honor`="DBL" WHERE `accountid`="XI64";",
	(char*)tempclient->m_playername.c_str(), tempclient->m_lastlogin, tempclient->m_ipaddress, tempclient->m_status, (char*)buffs.c_str(),
	(char*)tempclient->m_flag.c_str(), (char*)tempclient->m_faceurl.c_str(), (char*)research.c_str(), (char*)items.c_str(), (int)tempclient->m_allianceid, (int)tempclient->m_alliancerank, (char*)misc.c_str(), tempclient->m_cents, tempclient->Prestige(), tempclient->m_honor, tempclient->m_accountid);

	try
	{
	LOCK(M_CASTLELIST);
	if (tempclient)
	for (int k = 0; k < gserver->m_clients[i]->m_city.size(); ++k)
	{
	PlayerCity * tempcity = gserver->m_clients[i]->m_city[k];
	string misc2 = "";
	string transingtrades = "";
	string troop = "";
	string buildings = "";
	string fortification = "";
	string trades = "";

	ResultSet * res = sql3->QueryRes("SELECT COUNT(*) AS a FROM `cities` WHERE `fieldid`=%d", tempcity->m_tileid);
	res->next();
	int citycount = 0;
	if (res != 0)
	{
	citycount = res->getInt("a");
	delete res;
	}

	sprintf_s(temp1, 1000, "%d,%d,%d,%d,%d", tempcity->m_forts.traps, tempcity->m_forts.abatis, tempcity->m_forts.towers, tempcity->m_forts.logs, tempcity->m_forts.trebs);
	fortification = temp1;

	for (int j = 2; j <= 13; ++j)
	{
	if (j != 2)
	troop += "|";
	sprintf_s(temp1, 1000, XI64, tempcity->GetTroops(j));
	troop += temp1;
	}

	sprintf_s(temp1, 1000, "%d,"DBL","DBL","DBL","DBL","DBL",%d,%d,"DBL"", tempcity->m_population, tempcity->m_workrate.gold, tempcity->m_workrate.food, tempcity->m_workrate.wood, tempcity->m_workrate.iron, tempcity->m_workrate.stone, (int)tempcity->m_loyalty, (int)tempcity->m_grievance, tempcity->m_timers.updateresources);
	misc2 = temp1;

	for (int j = 0; j < 35; ++j)
	{
	if (tempcity->m_innerbuildings[j].type > 0)
	{
	if (j != 0)
	buildings += "|";
	sprintf_s(temp1, 1000, "%d,%d,%d,%d,"DBL","DBL"", tempcity->m_innerbuildings[j].type, tempcity->m_innerbuildings[j].level, tempcity->m_innerbuildings[j].id, tempcity->m_innerbuildings[j].status, tempcity->m_innerbuildings[j].starttime, tempcity->m_innerbuildings[j].endtime);
	buildings += temp1;
	}
	}
	for (int j = 0; j <= 40; ++j)
	{
	if (tempcity->m_outerbuildings[j].type > 0)
	{
	buildings += "|";
	sprintf_s(temp1, 1000, "%d,%d,%d,%d,"DBL","DBL"", tempcity->m_outerbuildings[j].type, tempcity->m_outerbuildings[j].level, tempcity->m_outerbuildings[j].id, tempcity->m_outerbuildings[j].status, tempcity->m_outerbuildings[j].starttime, tempcity->m_outerbuildings[j].endtime);
	buildings += temp1;
	}
	}

	if (citycount > 0)
	{
	//update
	sql3->Query("UPDATE `cities` SET `misc`='%s',`status`=%d,`allowalliance`=%d,`logurl`='%s',`fieldid`=%d,`transingtrades`='%s',`troop`='%s',`name`='%s',`buildings`='%s',`fortification`='%s',`trades`='%s', \
	`gooutforbattle`=%d,`hasenemy`=%d,`gold`="DBL",`food`="DBL",`wood`="DBL",`iron`="DBL",`stone`="DBL",`creation`="DBL" WHERE `fieldid`=%d LIMIT 1;",
	(char*)misc2.c_str(), (int)tempcity->m_status, (int)tempcity->m_allowalliance, (char*)tempcity->m_logurl.c_str(), tempcity->m_tileid, (char*)transingtrades.c_str(),
	(char*)troop.c_str(), (char*)tempcity->m_cityname.c_str(), (char*)buildings.c_str(), (char*)fortification.c_str(), (char*)trades.c_str(), (int)tempcity->m_gooutforbattle, (tempcity->m_hasenemy)?1:0,
	tempcity->m_resources.gold, tempcity->m_resources.food, tempcity->m_resources.wood, tempcity->m_resources.iron, tempcity->m_resources.stone, tempcity->m_creation, tempcity->m_tileid);
	}
	else
	{
	//insert
	sql3->Query("INSERT INTO `cities` (`accountid`,`misc`,`status`,`allowalliance`,`logurl`,`fieldid`,`transingtrades`,`troop`,`name`,`buildings`,`fortification`,`trades`,`gooutforbattle`,`hasenemy`,`gold`,`food`,`wood`,`iron`,`stone`,`creation`) \
	VALUES (%d, '%s',%d,%d,'%s',%d,'%s','%s','%s','%s','%s','%s',%d,%d,"DBL","DBL","DBL","DBL","DBL","DBL");",
	tempcity->m_accountid, (char*)misc2.c_str(), (int)tempcity->m_status, (int)tempcity->m_allowalliance, (char*)tempcity->m_logurl.c_str(), tempcity->m_tileid, (char*)transingtrades.c_str(), (char*)troop.c_str(), (char*)tempcity->m_cityname.c_str(), (char*)buildings.c_str(), (char*)fortification.c_str(), (char*)trades.c_str(), (int)tempcity->m_gooutforbattle,
	(tempcity->m_hasenemy)?1:0, tempcity->m_resources.gold, tempcity->m_resources.food, tempcity->m_resources.wood, tempcity->m_resources.iron, tempcity->m_resources.stone, tempcity->m_creation);
	}


	try
	{
	LOCK(M_HEROLIST);
	for (int a = 0; a < 10; ++a)
	{
	Hero * temphero = tempcity->m_heroes[a];

	if (temphero)
	{
	ResultSet * res = sql3->QueryRes("SELECT COUNT(*) AS a FROM `heroes` WHERE `id`=%d", temphero->m_id);
	res->next();
	int herocount = 0;
	if (res != 0)
	{
	herocount = res->getInt("a");
	delete res;
	}

	if (herocount > 0)
	{
	//update
	sql3->Query("UPDATE `heroes` SET `ownerid`="XI64",`castleid`=%d,`name`='%s',`status`=%d,`itemid`=%d,`itemamount`=%d,`basestratagem`=%d,`stratagem`=%d,`stratagemadded`=%d,`stratagembuffadded`=%d,\
	`basepower`=%d,`power`=%d,`poweradded`=%d,`powerbuffadded`=%d,`basemanagement`=%d,`management`=%d,`managementadded`=%d,`managementbuffadded`=%d,\
	`logurl`='%s',`remainpoint`=%d,`level`=%d,`upgradeexp`="DBL",`experience`="DBL",`loyalty`=%d WHERE `id`="XI64" LIMIT 1;",
	tempcity->m_accountid, tempcity->m_castleid, temphero->m_name.c_str(), (int)temphero->m_status, temphero->m_itemid, temphero->m_itemamount, (int)temphero->m_basestratagem, (int)temphero->m_stratagem, (int)temphero->m_stratagemadded, (int)temphero->m_stratagembuffadded,
	(int)temphero->m_basepower, (int)temphero->m_power, (int)temphero->m_poweradded, (int)temphero->m_powerbuffadded, (int)temphero->m_basemanagement, (int)temphero->m_management, (int)temphero->m_managementadded, (int)temphero->m_managementbuffadded,
	(char*)temphero->m_logourl.c_str(), (int)temphero->m_remainpoint, (int)temphero->m_level, temphero->m_upgradeexp, temphero->m_experience, (int)temphero->m_loyalty, temphero->m_id);
	}
	else
	{
	//insert
	sql3->Query("INSERT INTO `heroes` (`id`,`ownerid`,`castleid`,`name`,`status`,`itemid`,`itemamount`,`basestratagem`,`stratagem`,`stratagemadded`,`stratagembuffadded`,\
	`basepower`,`power`,`poweradded`,`powerbuffadded`,`basemanagement`,`management`,`managementadded`,`managementbuffadded`,\
	`logurl`,`remainpoint`,`level`,`upgradeexp`,`experience`,`loyalty`) VALUES ("XI64","XI64",%d,'%s',%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,'%s',%d,%d,"DBL","DBL",%d);",
	temphero->m_id, tempcity->m_accountid, tempcity->m_castleid, temphero->m_name.c_str(), (int)temphero->m_status, temphero->m_itemid, temphero->m_itemamount, (int)temphero->m_basestratagem, (int)temphero->m_stratagem, (int)temphero->m_stratagemadded, (int)temphero->m_stratagembuffadded,
	(int)temphero->m_basepower, (int)temphero->m_power, (int)temphero->m_poweradded, (int)temphero->m_powerbuffadded, (int)temphero->m_basemanagement, (int)temphero->m_management, (int)temphero->m_managementadded, (int)temphero->m_managementbuffadded,
	(char*)temphero->m_logourl.c_str(), (int)temphero->m_remainpoint, (int)temphero->m_level, temphero->m_upgradeexp, temphero->m_experience, (int)temphero->m_loyalty);
	}
	}
	}
	UNLOCK(M_HEROLIST);
	}
	catch (std::exception& e)
	{
	Log("SaveData() Exception: %s", e.what());
	UNLOCK(M_HEROLIST);
	}
	catch(...)
	{
	Log("SaveData() Exception.");
	UNLOCK(M_HEROLIST);
	}

	}
	UNLOCK(M_CASTLELIST);
	}
	catch (std::exception& e)
	{
	Log("SaveData() Exception: %s", e.what());
	UNLOCK(M_CASTLELIST);
	}
	catch(...)
	{
	Log("SaveData() Exception.");
	UNLOCK(M_CASTLELIST);
	}
	if ((i+1)%(gserver->maxplayers/10) == 0)
	{
	Log("%d%%", int((double(double(i+1)/gserver->maxplayers))*double(100)));
	}
	}
	UNLOCK(M_CLIENTLIST);
	}
	catch (std::exception& e)
	{
	Log("SaveData() Exception: %s", e.what());
	UNLOCK(M_CLIENTLIST);
	}
	catch(...)
	{
	Log("SaveData() Exception.");
	UNLOCK(M_CLIENTLIST);
	}*/

	consoleLogger->information("Save Complete.");
	SaveThreadRunning = false;
	return;
}


// Server code

int32_t Server::CalcTroopSpeed(PlayerCity * city, stTroops & troops, int32_t starttile, int32_t endtile)
{
	int32_t fx, fy, tx, ty;
	GETXYFROMID4(fx, fy, starttile, mapsize);
	GETXYFROMID4(tx, ty, endtile, mapsize);

	double line = sqrt(pow(abs(fx - tx), abs(fx - tx)) + pow(abs(fy - ty), abs(fy - ty)));

	double slowest = 10000;//either
	double mslowest = 10000;//mech
	double fslowest = 10000;//foot

	if (troops.catapult) { mslowest = 80; }
	else if (troops.ballista) { mslowest = 100; }
	else if (troops.ram) { mslowest = 120; }
	else if (troops.transporter) { mslowest = 150; }
	else if (troops.cataphract) { mslowest = 750; }
	else if (troops.cavalry) { mslowest = 1000; }

	if (troops.worker) { fslowest = 180; }
	else if (troops.warrior) { fslowest = 200; }
	else if (troops.archer) { fslowest = 250; }
	else if (troops.sword) { fslowest = 275; }
	else if (troops.pike) { fslowest = 300; }
	else if (troops.scout) { fslowest = 3000; }

	double maxtechlevel = city->GetBuildingLevel(B_ACADEMY);
	double compass = city->m_client->GetResearchLevel(T_COMPASS);//1.5x speed max
	double hbr = city->m_client->GetResearchLevel(T_HORSEBACKRIDING);//2x speed max

	hbr = (maxtechlevel >= hbr) ? hbr : maxtechlevel;
	compass = (maxtechlevel >= compass) ? compass : maxtechlevel;

	mslowest *= (hbr * 0.05) + 1;
	fslowest *= (compass * 0.10) + 1;

	mslowest /= 1000;
	fslowest /= 1000;

	slowest = (mslowest < fslowest) ? mslowest : fslowest;

	slowest /= 60;

	return line / slowest;
}
stItemConfig * Server::GetItem(string name)
{
	for (int i = 0; i < DEF_MAXITEMS; ++i)
	{
		if (m_items[i].name == name)
			return &m_items[i];
	}
	return 0;
}
bool Server::comparebuy(stMarketEntry first, stMarketEntry second)
{
	if (first.price > second.price)
		return true;
	else
		return false;
}
bool Server::comparesell(stMarketEntry first, stMarketEntry second)
{
	if (first.price < second.price)
		return true;
	else
		return false;
}
double Server::GetPrestigeOfAction(int8_t action, int8_t id, int8_t level, int8_t thlevel)
{
	double prestige = 0;
	switch (action)
	{
		case DEF_RESEARCH:
			switch (id)
			{
				case T_AGRICULTURE:
					prestige = 26 * 2;
					break;
				case T_LUMBERING:
					prestige = 31 * 2;
					break;
				case T_MASONRY:
					prestige = 41 * 2;
					break;
				case T_MINING:
					prestige = 55 * 2;
					break;
				case T_METALCASTING:
					prestige = 57 * 2;//not final
					break;
				case T_INFORMATICS:
					prestige = 59 * 2;//not final
					break;
				case T_MILITARYSCIENCE:
					prestige = 61 * 2;
					break;
				case T_MILITARYTRADITION:
					prestige = 78 * 2;
					break;
				case T_IRONWORKING:
					prestige = 26 * 2;//not final
					break;
				case T_LOGISTICS:
					prestige = 26 * 2;//not final
					break;
				case T_COMPASS:
					prestige = 26 * 2;//not final
					break;
				case T_HORSEBACKRIDING:
					prestige = 26 * 2;//not final
					break;
				case T_ARCHERY:
					prestige = 26 * 2;//not final
					break;
				case T_STOCKPILE:
					prestige = 26 * 2;//not final
					break;
				case T_MEDICINE:
					prestige = 26 * 2;//not final
					break;
				case T_CONSTRUCTION:
					prestige = 26 * 2;//not final
					break;
				case T_ENGINEERING:
					prestige = 26 * 2;//not final
					break;
				case T_MACHINERY:
					prestige = 26 * 2;//not final
					break;
				case T_PRIVATEERING:
					prestige = 26 * 2;//not final
					break;
			}
			break;
		case DEF_BUILDING:
			switch (id)
			{
				case B_COTTAGE:
					prestige = 4;
					break;
				case B_BARRACKS:
					prestige = 28;
					break;
				case B_WAREHOUSE:
					prestige = 21;
					break;
				case B_SAWMILL:
					prestige = 7;
					break;
				case B_STONEMINE:
					prestige = 9;
					break;
				case B_IRONMINE:
					prestige = 11;
					break;
				case B_FARM:
					prestige = 5;
					break;
				case B_STABLE:
					prestige = 36;
					break;
				case B_INN:
					prestige = 26;
					break;
				case B_FORGE:
					prestige = 26;
					break;
				case B_MARKETPLACE:
					prestige = 32;
					break;
				case B_RELIEFSTATION:
					prestige = 26;//not final
					break;
				case B_ACADEMY:
					prestige = 30;
					break;
				case B_WORKSHOP:
					prestige = 38;
					break;
				case B_FEASTINGHALL:
					prestige = 35;
					break;
				case B_EMBASSY:
					prestige = 18;
					break;
				case B_RALLYSPOT:
					prestige = 25;
					break;
				case B_BEACONTOWER:
					prestige = 39;
					break;
				case B_TOWNHALL:
					prestige = 112;
					break;
				case B_WALLS:
					prestige = 128;
					break;
			}
			break;
		case DEF_TRAIN:
			switch (id)
			{
				case TR_WORKER:
					prestige = 0.5;
					break;
				case TR_WARRIOR:
					prestige = 0.5;
					break;
				case TR_SCOUT:
					prestige = 1.0;//not final
					break;
				case TR_PIKE:
					prestige = 2.0;
					break;
				case TR_SWORDS:
					prestige = 3;//not final
					break;
				case TR_ARCHER:
					prestige = 4;//not final
					break;
				case TR_TRANSPORTER:
					prestige = 5;//not final
					break;
				case TR_CAVALRY:
					prestige = 6;//not final
					break;
				case TR_CATAPHRACT:
					prestige = 7;//not final
					break;
				case TR_BALLISTA:
					prestige = 8;//not final
					break;
				case TR_RAM:
					prestige = 9;//not final
					break;
				case TR_CATAPULT:
					prestige = 10;//not final
					break;
				case TR_TRAP:
					prestige = 1;//not final
					break;
				case TR_ABATIS:
					prestige = 2;//not final
					break;
				case TR_ARCHERTOWER:
					prestige = 3;//not final
					break;
				case TR_ROLLINGLOG:
					prestige = 4;//not final
					break;
				case TR_TREBUCHET:
					prestige = 5;//not final
					break;
			}
			break;
	}
	for (int i = 0; i < level; ++i)
		prestige *= 2;
	for (int i = 0; i < thlevel - 1; ++i)
		prestige /= 2;
	return prestige;
}
void Server::AddTimedEvent(stTimedEvent & te)
{
	switch (te.type)
	{
		case DEF_TIMEDARMY:
			armylist.push_back(te);
			break;
		case DEF_TIMEDBUILDING:
			buildinglist.push_back(te);
			break;
		case DEF_TIMEDRESEARCH:
			researchlist.push_back(te);
			break;
	}
}
void Server::MassDisconnect()
{

	std::list<Client*>::iterator playeriter;

	for (playeriter = players.begin(); playeriter != players.end(); ++playeriter)
	{
		Client * client = *playeriter;
		if (client->socket)
		{
			amf3object obj;
			obj["cmd"] = "server.SystemInfoMsg";
			obj["data"] = amf3object();
			amf3object & data = obj["data"];
			data["alliance"] = false;
			data["tV"] = false;
			data["noSenderSystemInfo"] = true;
			data["msg"] = "Server shutting down.";

			SendObject(client->socket, obj);

			client->socket->stop();
			//shutdown(sockets->fdsockets[i].fdsock, 0);
			//closesocket(sockets->fdsockets[i].fdsock);
		}
	}
}
void Server::MassMessage(string str, bool nosender /* = false*/, bool tv /* = false*/, bool all /* = false*/)
{
	std::list<Client*>::iterator playeriter;
	for (playeriter = players.begin(); playeriter != players.end(); ++playeriter)
	{
		Client * client = *playeriter;

		amf3object obj;
		obj["cmd"] = "server.SystemInfoMsg";
		obj["data"] = amf3object();
		amf3object & data = obj["data"];
		data["alliance"] = all;
		data["tV"] = tv;
		data["noSenderSystemInfo"] = nosender;
		data["msg"] = str;

		SendObject(client, obj);
	}
}
void Server::SendMessage(Client * client, string str, bool nosender /* = false*/, bool tv /* = false*/, bool all /* = false*/)
{
	if (client && client->socket)
	{
		amf3object obj;
		obj["cmd"] = "server.SystemInfoMsg";
		obj["data"] = amf3object();
		amf3object & data = obj["data"];
		data["alliance"] = all;
		data["tV"] = tv;
		data["noSenderSystemInfo"] = nosender;
		data["msg"] = str;

		SendObject(client, obj);
	}
}
int Server::RandomStat()
{
	int rnd = rand() % 10000;
	if ((rnd >= 0) && (rnd < 6500))
	{
		return rand() % 21 + 15; // 15-35
	}
	else if ((rnd >= 6500) && (rnd < 8500))
	{
		return rand() % 21 + 30; // 30-50
	}
	else if ((rnd >= 8500) && (rnd < 9500))
	{
		return rand() % 16 + 45; // 45-60
	}
	else if ((rnd >= 9500) && (rnd < 9900))
	{
		return rand() % 11 + 60; // 60-70
	}
	else if ((rnd >= 9900) && (rnd < 9950))
	{
		return rand() % 6 + 70; // 70-75
	}
	else if ((rnd >= 9950) && (rnd < 9975))
	{
		return rand() % 6 + 75; // 75-80
	}
	else if ((rnd >= 9975) && (rnd < 10000))
	{
		return rand() % 6 + 80; // 80-85
	}
	return 10;
}
Hero * Server::CreateRandomHero(int innlevel)
{
	Hero * hero = new Hero();

	int maxherolevel = innlevel * 5;

	hero->m_level = (rand() % maxherolevel) + 1;
	hero->m_basemanagement = RandomStat();
	hero->m_basestratagem = RandomStat();
	hero->m_basepower = RandomStat();

	int remainpoints = hero->m_level;

	hero->m_power = rand() % remainpoints;
	remainpoints -= hero->m_power;
	hero->m_power += hero->m_basepower;
	if (remainpoints > 0)
	{
		hero->m_management = rand() % remainpoints;
		remainpoints -= hero->m_management;
		hero->m_management += hero->m_basemanagement;
	}
	if (remainpoints > 0)
	{
		hero->m_stratagem = remainpoints;
		remainpoints -= hero->m_stratagem;
		hero->m_stratagem += hero->m_basestratagem;
	}


	hero->m_loyalty = 70;
	hero->m_experience = 0;
	hero->m_upgradeexp = hero->m_level * hero->m_level * 100;
	hero->m_id = 0;
	char tempstr[30];
	sprintf(tempstr, "Test Name%d%d%d", hero->m_power, hero->m_management, hero->m_stratagem);
	hero->m_name = tempstr;
	hero->m_logourl = "images/icon/player/faceA20.jpg";

	return hero;
}
bool Server::compareprestige(stClientRank first, stClientRank second)
{
	if (first.client->m_prestige > second.client->m_prestige)
		return true;
	else
		return false;
}
bool Server::comparehonor(stClientRank first, stClientRank second)
{
	if (first.client->m_honor > second.client->m_honor)
		return true;
	else
		return false;
}
bool Server::comparetitle(stClientRank first, stClientRank second)
{
	if (first.client->m_title > second.client->m_title)
		return true;
	else
		return false;
}
bool Server::comparepop(stClientRank first, stClientRank second)
{
	if (first.client->m_population > second.client->m_population)
		return true;
	else
		return false;
}
bool Server::comparecities(stClientRank first, stClientRank second)
{
	if (first.client->m_citycount > second.client->m_citycount)
		return true;
	else
		return false;
}
//TODO: correct searches. subsorts. If searching by player title, each title should be sorted by prestige
void Server::SortPlayers()
{
	m_prestigerank.clear();
	m_honorrank.clear();
	m_titlerank.clear();
	m_populationrank.clear();
	m_citiesrank.clear();


	std::list<Client*>::iterator playeriter;
	for (playeriter = players.begin(); playeriter != players.end(); ++playeriter)
	{
		Client * client = *playeriter;

		stClientRank rank = stClientRank();
		rank.client = client;
		rank.rank = 0;

		m_prestigerank.push_back(rank);
		m_honorrank.push_back(rank);
		m_titlerank.push_back(rank);
		m_populationrank.push_back(rank);
		m_citiesrank.push_back(rank);
	}

	m_prestigerank.sort(compareprestige);
	m_honorrank.sort(comparehonor);
	m_titlerank.sort(comparetitle);
	m_populationrank.sort(comparepop);
	m_citiesrank.sort(comparecities);


	std::list<stClientRank>::iterator iter;
	int num = 1;
	for (iter = m_prestigerank.begin(); iter != m_prestigerank.end(); ++iter)
	{
		iter->rank = num;
		iter->client->m_prestigerank = num++;
		if (iter->client->m_connected)
			iter->client->PlayerUpdate();
	}
	num = 1;
	for (iter = m_honorrank.begin(); iter != m_honorrank.end(); ++iter)
	{
		iter->rank = num++;
	}
	num = 1;
	for (iter = m_titlerank.begin(); iter != m_titlerank.end(); ++iter)
	{
		iter->rank = num++;
	}
	num = 1;
	for (iter = m_populationrank.begin(); iter != m_populationrank.end(); ++iter)
	{
		iter->rank = num++;
	}
	num = 1;
	for (iter = m_citiesrank.begin(); iter != m_citiesrank.end(); ++iter)
	{
		iter->rank = num++;
	}
}
bool Server::comparestratagem(stHeroRank first, stHeroRank second)
{
	if (first.stratagem > second.stratagem)
		return true;
	else
		return false;
}
bool Server::comparepower(stHeroRank first, stHeroRank second)
{
	if (first.power > second.power)
		return true;
	else
		return false;
}
bool Server::comparemanagement(stHeroRank first, stHeroRank second)
{
	if (first.management > second.management)
		return true;
	else
		return false;
}
bool Server::comparegrade(stHeroRank first, stHeroRank second)
{
	if (first.grade> second.grade)
		return true;
	else
		return false;
}
void Server::SortHeroes()
{
	m_herorankstratagem.clear();
	m_herorankpower.clear();
	m_herorankmanagement.clear();
	m_herorankgrade.clear();


	std::list<Client*>::iterator playeriter;
	for (playeriter = players.begin(); playeriter != players.end(); ++playeriter)
	{
		Client * client = *playeriter;
		for (uint32_t j = 0; j < client->m_city.size(); ++j)
		{
			if (client->m_city.at(j))
			{
				for (uint32_t k = 0; k < 10; ++k)
				{
					if (client->m_city[j]->m_heroes[k])
					{
						Hero * hero = client->m_city[j]->m_heroes[k];
						stHeroRank rank = stHeroRank();
						assert(hero->m_level > 0);
						assert(hero->m_stratagem > 0);
						assert(hero->m_management > 0);
						assert(hero->m_power > 0);
						rank.grade = hero->m_level;
						rank.stratagem = hero->m_stratagem;
						rank.management = hero->m_management;
						rank.power = hero->m_power;
						rank.name = hero->m_name;
						rank.kind = client->m_playername;
						rank.rank = 0;
						m_herorankstratagem.push_back(rank);
						m_herorankpower.push_back(rank);
						m_herorankmanagement.push_back(rank);
						m_herorankgrade.push_back(rank);
					}
				}
			}
		}
	}

	m_herorankstratagem.sort(comparestratagem);
	m_herorankpower.sort(comparepower);
	m_herorankmanagement.sort(comparemanagement);
	m_herorankgrade.sort(comparegrade);

	std::list<stHeroRank>::iterator iter;
	int num = 1;
	for (iter = m_herorankstratagem.begin(); iter != m_herorankstratagem.end(); ++iter)
	{
		iter->rank = num++;
	}
	num = 1;
	for (iter = m_herorankpower.begin(); iter != m_herorankpower.end(); ++iter)
	{
		iter->rank = num++;
	}
	num = 1;
	for (iter = m_herorankmanagement.begin(); iter != m_herorankmanagement.end(); ++iter)
	{
		iter->rank = num++;
	}
	num = 1;
	for (iter = m_herorankgrade.begin(); iter != m_herorankgrade.end(); ++iter)
	{
		iter->rank = num++;
	}
}
bool Server::comparepopulation(stCastleRank first, stCastleRank second)
{
	if (first.population > second.population)
		return true;
	else
		return false;
}
bool Server::comparelevel(stCastleRank first, stCastleRank second)
{
	if (first.level > second.level)
		return true;
	else
		return false;
}
void Server::SortCastles()
{
	m_castleranklevel.clear();
	m_castlerankpopulation.clear();

	std::list<Client*>::iterator playeriter;
	for (playeriter = players.begin(); playeriter != players.end(); ++playeriter)
	{
		Client * client = *playeriter;
		for (int j = 0; j < client->m_city.size(); ++j)
		{
			if (client->m_city.at(j))
			{
				PlayerCity * city = client->m_city[j];
				stCastleRank rank = stCastleRank();
				std::stringstream ss;
				string grade;
				int16_t level = city->GetBuildingLevel(B_TOWNHALL);
				ss << "Level " << level;
				grade = ss.str();
				if (client->HasAlliance())
					rank.alliance = client->GetAlliance()->m_name;
				else
					rank.alliance = "";
				rank.level = level;
				rank.population = city->m_population;
				rank.name = city->m_cityname;
				rank.grade = grade;
				rank.kind = client->m_playername;
				rank.rank = 0;
				m_castleranklevel.push_back(rank);
				m_castlerankpopulation.push_back(rank);
			}
		}
	}

	m_castleranklevel.sort(comparelevel);
	m_castlerankpopulation.sort(comparepopulation);

	std::list<stCastleRank>::iterator iter;
	int num = 1;
	for (iter = m_castleranklevel.begin(); iter != m_castleranklevel.end(); ++iter)
	{
		iter->rank = num++;
	}
	num = 1;
	for (iter = m_castlerankpopulation.begin(); iter != m_castlerankpopulation.end(); ++iter)
	{
		iter->rank = num++;
	}
}
int32_t  Server::GetClientIndex(int32_t accountid)
{
	int32_t i = 0;
	std::list<Client*>::iterator playeriter;
	for (playeriter = players.begin(); playeriter != players.end(); ++playeriter)
	{
		Client * client = *playeriter;
		if (client->m_accountid == accountid)
			return i;
		++i;
	}
 	return -1;
}
Client * Server::GetClientByCastle(int32_t castleid)
{
	std::list<Client*>::iterator playeriter;
	for (playeriter = players.begin(); playeriter != players.end(); ++playeriter)
	{
		Client * client = *playeriter;
		if (client->GetCity(castleid) != 0)
			return client;
	}
 	return 0;
}
Client * Server::GetClient(int32_t accountid)
{
	std::list<Client*>::iterator playeriter;
	for (playeriter = players.begin(); playeriter != players.end(); ++playeriter)
	{
		Client * client = *playeriter;
		if (client->m_accountid == accountid)
			return client;
	}
 	return 0;
}
Client * Server::GetClientByParent(int accountid)
{
	std::list<Client*>::iterator playeriter;
	for (playeriter = players.begin(); playeriter != players.end(); ++playeriter)
	{
		Client * client = *playeriter;
		if (client->masteraccountid == accountid)
			return client;
	}
 	return 0;
}
Client * Server::GetClientByName(string name)
{
	std::list<Client*>::iterator playeriter;
	for (playeriter = players.begin(); playeriter != players.end(); ++playeriter)
	{
		Client * client = *playeriter;
		if (client->m_playername == name)
			return client;
	}
 	return 0;
}
void Server::CloseClient(Client * client, int typecode, string message)
{
	if (client && client->socket)
	{
		amf3object obj;
		obj["cmd"] = "server.ConnectionLost";//gameClient.kickout";
		obj["data"] = amf3object();
		amf3object & data = obj["data"];
		data["reasonCode"] = typecode;
		data["msg"] = message;

		SendObject(client, obj);
		//client->socket->socket().shutdown(asio::socket_base::shutdown_type::shutdown_both);
		client->socket->stop();
	}
}
City * Server::AddPlayerCity(Client * client, int tileid, uint32_t castleid)
{
	PlayerCity * city = new PlayerCity();
	city->m_type = CASTLE;
	city->m_logurl = "";
	city->m_status = 0;
	city->m_tileid = tileid;
	city->m_castleid = tileid;
	city->m_castleinternalid = castleid;
	city->m_accountid = client->m_accountid;

	city->SetBuilding(31, 1, -1, 0, 0, 0);
	city->SetResources(0, 0, 0, 0, 0);
	city->m_client = client;
	string temp = "";
	std::stringstream ss;
	ss << "50,10.000000,100.000000,100.000000,100.000000,100.000000,90,0," << (double)unixtime();
	temp = ss.str();

	city->ParseMisc(temp);

	client->m_city.push_back(city);
	client->m_citycount++;

	map->m_tile[tileid].m_city = city;
	m_city.push_back(city);


	map->m_tile[tileid].m_npc = false;
	map->m_tile[tileid].m_ownerid = client->m_accountid;
	map->m_tile[tileid].m_city = city;
	map->m_tile[tileid].m_type = CASTLE;
	map->m_tile[tileid].m_castleid = castleid;

	return city;
}
City * Server::AddNpcCity(int tileid)
{
	NpcCity * city = new NpcCity();
	city->m_tileid = tileid;
	city->m_type = NPC;
	city->m_cityname = "Barbarian City";
	city->m_status = 0;
	m_city.push_back(city);
	map->m_tile[tileid].m_city = city;
	map->m_tile[tileid].m_npc = true;
	map->m_tile[tileid].m_type = NPC;
	return city;
}
Client * Server::NewClient()
{
	static uint32_t clientnum;
	if (currentplayersonline < maxplayers)
	{
		Client * client = new Client(this);
		client->m_clientnumber = clientnum++;
		players.push_back(client);
		consoleLogger->information(Poco::format("New client # %?d", players.size()));
		return client;
	}
	consoleLogger->information(Poco::format("Client list full! %?d players on server.", maxplayers));
	return 0;
}
bool Server::ParseChat(Client * client, string str)
{
	if (str.size() > 0)
	{
		char tempstr[200];
		memset(tempstr, 0, 200);
		memcpy(tempstr, str.c_str(), str.size() >= 200 ? 200 : str.size());
		if (!memcmp(tempstr, "\\", 1))
		{
			char * command, *ctx;
			command = strtok_s(tempstr, "\\ ", &ctx);

			amf3object obj;
			obj["cmd"] = "server.SystemInfoMsg";
			obj["data"] = amf3object();
			amf3object & data = obj["data"];
			data["alliance"] = false;
			data["tV"] = false;
			data["noSenderSystemInfo"] = true;

			if (command == 0)
			{
				return false;
			}

			if (!strcmp(command, "motd"))
			{
				if ((client->m_allianceid <= 0) || (client->m_alliancerank < 3))
				{
					data["msg"] = "You do not have an alliance or are not the proper rank.";
				}
				else
				{
					m_alliances->AllianceById(client->m_allianceid)->m_motd = (tempstr + strlen(command) + 2);
					string s;
					s = "Alliance MOTD set to '";
					s += m_alliances->AllianceById(client->m_allianceid)->m_motd;
					s += "'";
					data["msg"] = s;
				}
			}
			else if (!strcmp(command, "restart"))
			{
				// 				amf3object sobj;
				// 				sobj["cmd"] = "server.ConnectionLost";
				// 				sobj["data"] = amf3object();
				// 				amf3object & sdata = sobj["data"];
				// 				sdata["reasonCode"] = 1;

				std::list<Client*>::iterator playeriter;
				for (playeriter = players.begin(); playeriter != players.end(); ++playeriter)
					CloseClient(*playeriter, 1, "");
				//					SendObject((*playeriter)->socket, sobj);
			}
			else if (!strcmp(command, "massmsg"))
			{
				if (client->m_playername == "Daisy")
				{
					char * substr = tempstr + strlen(command) + 2;
					int substrlen = strlen(substr);
					for (int i = 0; i < substrlen; ++i)
					{
						if (!memcmp(((char*)substr) + i, "&lt;", 4))
						{
							//match
							*(char*)(substr + i) = '<';
							memcpy(substr + i + 1, substr + i + 4, substrlen - i);
						}
					}
					MassMessage(substr);
				}
			}
			else if (!strcmp(command, "save"))
			{
				// #ifndef WIN32
				// 				pthread_t hSaveThread;
				// 				if (pthread_create(&hSaveThread, NULL, SaveData, 0))
				// 				{
				// 					SFERROR("pthread_create");
				// 				}
				// #else
				// 				uint32_t uAddr;
				// 				HANDLE hSaveThread;
				// 				hSaveThread = (HANDLE)_beginthreadex(0, 0, SaveData, 0, 0, &uAddr);
				// #endif
				data["msg"] = "Data being saved.";
			}
			else if (!strcmp(command, "cents"))
			{
				data["msg"] = "500 Cents granted.";
				client->m_cents += 5000;
				client->PlayerUpdate();
			}
			/*			else if (!strcmp(command, "pres"))
			{
			int test = 12786431;
			stringstream ss;
			ss << test << "prestige granted.";
			data["msg"] = ss.str().c_str();
			client->m_prestige += test;
			client->PlayerUpdate();
			}*/
			else if (!strcmp(command, "resources"))
			{
				stResources res(10000000, 10000000, 10000000, 10000000, 10000000);
				client->GetFocusCity()->m_resources += res;
				data["msg"] = "Resources granted.";
				client->GetFocusCity()->ResourceUpdate();
			}
			else if (!strcmp(command, "tempvar"))
			{
				command = strtok_s(0, " ", &ctx);
				client->m_icon = atoi(command);
				string s;
				s = "Tempvar set to '<u>";
				s += command;
				s += "</u>'.";
				data["msg"] = s;
			}
			else if (!strcmp(command, "kick"))
			{
				command = strtok_s(0, " |", &ctx);
				if (!command)
				{
					data["msg"] = "Invalid syntax";
					SendObject(client, obj);
					return false;
				}

				int typecode = atoi(command);
				command = strtok_s(0, "|", &ctx);
				if (!command)
				{
					data["msg"] = "Invalid syntax";
					SendObject(client, obj);
					return false;
				}
				string msg = command;
				CloseClient(client, typecode, msg);
			}
			else if (!strcmp(command, "buff"))
			{
				command = strtok_s(0, " |", &ctx);
				if (!command)
				{
					data["msg"] = "Invalid syntax";
					SendObject(client, obj);
					return false;
				}

				string buffid = command;
				command = strtok_s(0, "| ", &ctx);
				if (!command)
				{
					data["msg"] = "Invalid syntax";
					SendObject(client, obj);
					return false;
				}
				string desc = command;
				command = strtok_s(0, "| ", &ctx);
				if (!command)
				{
					data["msg"] = "Invalid syntax";
					SendObject(client, obj);
					return false;
				}
				int64_t var = atoi(command) + unixtime();
				string s;
				s = "Buff set to '<u>";
				s += buffid;
				s += "</u>'.";
				data["msg"] = s;

				client->SetBuff(buffid, desc, var);
			}
			else if (!strcmp(command, "commands"))
			{
				data["msg"] = "Commands: motd cents resources";
			}
			else
			{
				string s;
				s = "Command '<u>";
				s += command;
				s += "</u>' does not exist.";
				data["msg"] = s;
			}
			SendObject(client, obj);
			return false;
		}
	}
	return true;
}
int16_t Server::GetRelation(int32_t client1, int32_t client2)
{
	if (client1 >= 0 && client2 >= 0)
	{
		return m_alliances->GetRelation(client1, client2);
	}
	return 0;
}
void * Server::DoRankSearch(string key, int8_t type, void * subtype, int16_t page, int16_t pagesize)
{
	if (type == 1)//client lists
	{
		std::list<stSearchClientRank>::iterator iter;
		for (iter = m_searchclientranklist.begin(); iter != m_searchclientranklist.end();)
		{
			if (iter->rlist == subtype && iter->key == key)
				return &iter->ranklist;
			++iter;
		}

		stSearchClientRank searchrank = stSearchClientRank();
		searchrank.key = key;
		searchrank.lastaccess = unixtime();
		searchrank.rlist = (std::list<stClientRank>*)subtype;

		std::list<stClientRank>::iterator iterclient;

		for (iterclient = ((std::list<stClientRank>*)subtype)->begin(); iterclient != ((std::list<stClientRank>*)subtype)->end();)
		{
			if (ci_find(iterclient->client->m_playername, key) != string::npos)
			{
				stClientRank clientrank;
				clientrank.client = iterclient->client;
				clientrank.rank = iterclient->rank;
				searchrank.ranklist.push_back(clientrank);
			}
			++iterclient;
		}
		m_searchclientranklist.push_back(searchrank);
		return &m_searchclientranklist.back();
	}
	else if (type == 2)//hero lists
	{
		std::list<stSearchHeroRank>::iterator iter;
		for (iter = m_searchheroranklist.begin(); iter != m_searchheroranklist.end();)
		{
			if (iter->rlist == subtype && iter->key == key)
				return &iter->ranklist;
			++iter;
		}

		stSearchHeroRank searchrank = stSearchHeroRank();
		searchrank.key = key;
		searchrank.lastaccess = unixtime();
		searchrank.rlist = (std::list<stHeroRank>*)subtype;

		std::list<stHeroRank>::iterator iterhero;

		for (iterhero = ((std::list<stHeroRank>*)subtype)->begin(); iterhero != ((std::list<stHeroRank>*)subtype)->end();)
		{
			if (ci_find(iterhero->name, key) != string::npos)
			{
				stHeroRank herorank;
				herorank.grade = iterhero->grade;
				herorank.kind = iterhero->kind;
				herorank.management = iterhero->management;
				herorank.power = iterhero->power;
				herorank.name = iterhero->name;
				herorank.stratagem = iterhero->stratagem;
				herorank.rank = iterhero->rank;
				searchrank.ranklist.push_back(herorank);
			}
			++iterhero;
		}
		m_searchheroranklist.push_back(searchrank);
		return &m_searchheroranklist.back();
	}
	else if (type == 3)//castle lists
	{
		std::list<stSearchCastleRank>::iterator iter;
		for (iter = m_searchcastleranklist.begin(); iter != m_searchcastleranklist.end();)
		{
			if (iter->rlist == subtype && iter->key == key)
				return &iter->ranklist;
			++iter;
		}

		stSearchCastleRank searchrank;
		searchrank.key = key;
		searchrank.lastaccess = unixtime();
		searchrank.rlist = (std::list<stCastleRank>*)subtype;

		std::list<stCastleRank>::iterator itercastle;

		for (itercastle = ((std::list<stCastleRank>*)subtype)->begin(); itercastle != ((std::list<stCastleRank>*)subtype)->end();)
		{
			if (ci_find(itercastle->name, key) != string::npos)
			{
				stCastleRank castlerank;
				castlerank.grade = itercastle->grade;
				castlerank.alliance = itercastle->alliance;
				castlerank.kind = itercastle->kind;
				castlerank.level = itercastle->level;
				castlerank.name = itercastle->name;
				castlerank.population = itercastle->population;
				castlerank.rank = itercastle->rank;
				searchrank.ranklist.push_back(castlerank);
			}
			++itercastle;
		}
		m_searchcastleranklist.push_back(searchrank);
		return &m_searchcastleranklist.back();
	}
	else// if (type == 4)//alliance lists
	{
		std::list<stSearchAllianceRank>::iterator iter;
		for (iter = m_searchallianceranklist.begin(); iter != m_searchallianceranklist.end();)
		{
			if (iter->rlist == subtype && iter->key == key)
				return &iter->ranklist;
			++iter;
		}

		stSearchAllianceRank searchrank = stSearchAllianceRank();
		searchrank.key = key;
		searchrank.lastaccess = unixtime();
		searchrank.rlist = (std::list<stAlliance>*)subtype;

		std::list<stAlliance>::iterator iteralliance;

		for (iteralliance = ((std::list<stAlliance>*)subtype)->begin(); iteralliance != ((std::list<stAlliance>*)subtype)->end();)
		{
			if (ci_find(iteralliance->ref->m_name, key) != string::npos)
			{
				stAlliance alliancerank;
				alliancerank.ref = iteralliance->ref;
				alliancerank.rank = iteralliance->rank;
				searchrank.ranklist.push_back(alliancerank);
			}
			++iteralliance;
		}
		m_searchallianceranklist.push_back(searchrank);
		return &m_searchallianceranklist.back();
	}
return 0;
}
void Server::CheckRankSearchTimeouts(uint64_t time)
{
	std::list<stSearchClientRank>::iterator iterclient;
	std::list<stSearchHeroRank>::iterator iterhero;
	std::list<stSearchCastleRank>::iterator itercastle;

	for (iterclient = m_searchclientranklist.begin(); iterclient != m_searchclientranklist.end();)
	{
		if (iterclient->lastaccess + 30000 < time)
		{
			m_searchclientranklist.erase(iterclient++);
			continue;
		}
		++iterclient;
	}

	for (iterhero = m_searchheroranklist.begin(); iterhero != m_searchheroranklist.end();)
	{
		if (iterhero->lastaccess + 30000 < time)
		{
			m_searchheroranklist.erase(iterhero++);
			continue;
		}
		++iterhero;
	}

	for (itercastle = m_searchcastleranklist.begin(); itercastle != m_searchcastleranklist.end();)
	{
		if (itercastle->lastaccess + 30000 < time)
		{
			m_searchcastleranklist.erase(itercastle++);
			continue;
		}
		++itercastle;
	}
}
void Server::AddMarketEntry(stMarketEntry me, int8_t type)
{
	if (type == 1)
	{
		m_marketbuy.push_back(me);
		m_marketbuy.sort(comparebuy);
	}
	else if (type == 2)
	{
		m_marketsell.push_back(me);
		m_marketsell.sort(comparesell);
	}
}

