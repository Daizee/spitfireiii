//
// server.h
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

#include "includes.h"
#include "funcs.h"
#include "connection.h"
#include "structs.h"


using boost::unique_lock;
using boost::shared_lock;
using boost::shared_mutex;
using boost::lock;
using boost::defer_lock;
using boost::mutex;
using boost::upgrade_to_unique_lock;
using boost::upgrade_lock;

using Poco::Logger;
using Poco::PatternFormatter;
using Poco::FormattingChannel;
using Poco::ConsoleChannel;
using Poco::FileChannel;
using Poco::Message;
using Poco::Data::RecordSet;


using namespace Poco::Data;


class Map;
class Client;
class City;
class Alliance;
class AllianceCore;
class sockets;


class Server
{
public:
	Server();
	~Server();

	// Thread function for io service
	void io_thread();

	/// Run the server's io_service loop.
	void run();

	/// Stop the server.
	void stop();

	/// Start the first asynchronous operation for the connection.
	void start(connection_ptr c);
	void startpolicy(connection_ptr c);

	/// Stop the specified connection.
	void stop(connection_ptr c);

	/// Stop all connections.
	void stop_all();

	void do_accept();
	void do_acceptpolicy();

	/// The io_service used to perform asynchronous operations.
	boost::asio::io_service io_service_;

	/// The signal_set is used to register for process termination notifications.
	boost::asio::signal_set signals_;

	/// Acceptor used to listen for incoming connections.
	boost::asio::ip::tcp::acceptor acceptor_;

	/// The connection list which owns all live connections.
	std::set<connection_ptr> connections_;

	/// The next socket to be accepted.
	boost::asio::ip::tcp::socket socket_;

	/// The handler for all incoming requests.
	request_handler request_handler_;

//#ifdef WIN32
	boost::asio::ip::tcp::acceptor acceptorpolicy_;
	boost::asio::ip::tcp::socket socketpolicy_;
	request_handler request_handlerpolicy_;
//#endif

	uint32_t serverstatus;

	string servername;

	// Initialize Server
	bool Init();

	void PlayerCount(int8_t amount)
	{
		currentplayersonline += amount;
		if (currentplayersonline < 0)
		{
			currentplayersonline = 0;
			std::cerr << "server.currentplayersonline less than 0 error\n";
		}
	}
	int64_t PlayerCount() { return currentplayersonline; }

	bool ConnectSQL();
	bool InitSockets();
	Client * NewClient();
	Client * GetClient(int accountid);
	Client * GetClientByParent(int accountid);
	Client * GetClientByName(string name);
	Client * GetClientByCastle(int32_t castleid);
	int32_t  GetClientIndex(int32_t accountid);
	void CloseClient(int id);
	// reasonCode:
	// 1 = "Server restarting"
	// 2 = "Server is about to shut down. Logout as soon as possible"
	// 3 = "Another computer has logged into your account. Make sure you are not trying to log into the game from two computers or browser windows."
	// 4 = "Server undergoing maintenance."
	// 5 = custom message
	void CloseClient(Client * client, int typecode = 1, string message = "Connection Closed");

	City * AddPlayerCity(Client * client, int tileid, uint32_t castleid);
	City * AddNpcCity(int tileid);
	void MassMessage(string str, bool nosender = false, bool tv = false, bool all = false);
	void SendMessage(Client * client, string str, bool nosender = false, bool tv = false, bool all = false);
	void Shutdown();
	int32_t CalcTroopSpeed(PlayerCity * city, stTroops & troops, int32_t starttile, int32_t endtile);

	void SendObject(Client * c, const amf3object & object);

	void SendObject(connection * s, const amf3object & object)
	{
		if (s == 0)
			return;
		try
		{
			if (serverstatus == 0)
				return;
			char buffer[15000];
			int length = 0;
			amf3writer * writer;

			writer = new amf3writer(buffer + 4);

			writer->Write(object);

			(*(int*)buffer) = length = writer->position;
			ByteSwap(*(int*)buffer);

			s->write(buffer, length + 4);
			delete writer;
		}
		catch (std::exception& e)
		{
			std::cerr << s->address << "exception: " << __FILE__ << " @ " << __LINE__ << "\n";
			std::cerr << e.what() << "\n";
		}
	}

	// Parse chat for commands
	bool ParseChat(Client * client, string str);

	// Get Alliance based relation between two clients
	int16_t GetRelation(int32_t client1, int32_t client2);

	void TimerThread();
	void SaveThread();

	// Console and File log handles
	Logger * consoleLogger;
	Logger * logger;
	Logger * fileLogger;

	// Lua Handle
	lua_State *L;

	// MySQL
	string sqlhost, sqluser, sqlpass, bindaddress, bindport;
	string dbmaintable;
	string dbservertable;


	// Map size -- typically 500x500 or 800x800
	uint16_t mapsize;

	// Console and File log formatting
	FormattingChannel* pFCConsole;
	FormattingChannel* pFCDefault;
	FormattingChannel* pFCFile;

	// MySQL connection pools
	SessionPool * accountpool;
	SessionPool * serverpool;

	// mutexes
	struct mutexes
	{
		shared_mutex tiledata;
		shared_mutex clientlist;
		shared_mutex lua;
		shared_mutex timers;
		shared_mutex market;
		shared_mutex alliance;
		shared_mutex ranklist;
		shared_mutex herocreate;
	} mtxlist;

	// Max players allowed connected
	uint32_t maxplayers;

	// Current players connected
	uint32_t currentplayersonline;

	// Whether the timer thread is running or not
	bool TimerThreadRunning;
	bool SaveThreadRunning;

	Map * map;


	// List of active cities on the server (NPC and Player)
	std::vector<City*> m_city;

	// Alliance Controller
	AllianceCore * m_alliances;


	std::list<Client*> players;

	uint64_t ltime;

	uint64_t armycounter;
	string m_itemxml;
	stItemConfig m_items[DEF_MAXITEMS];
	int m_itemcount;
	stRarityGamble m_gambleitems;



	std::list<stMarketEntry> m_marketbuy;
	std::list<stMarketEntry> m_marketsell;

	static bool comparebuy(stMarketEntry first, stMarketEntry second);
	static bool comparesell(stMarketEntry first, stMarketEntry second);
	void AddMarketEntry(stMarketEntry me, int8_t type);

#define DEF_TIMEDARMY 1
#define DEF_TIMEDBUILDING 2
#define DEF_TIMEDRESEARCH 3

	stItemConfig * GetItem(string name);

	double GetPrestigeOfAction(int8_t action, int8_t id, int8_t level, int8_t thlevel);


	void MassDisconnect();
	void AddTimedEvent(stTimedEvent & te);


	stBuildingConfig m_buildingconfig[35][10];
	stBuildingConfig m_researchconfig[25][10];
	stBuildingConfig m_troopconfig[20];

	std::list<stTimedEvent> armylist;
	std::list<stTimedEvent> buildinglist;
	std::list<stTimedEvent> researchlist;

	std::queue<stPacketOut> m_packetout;

	int64_t m_heroid;
	int64_t m_cityid;
	int32_t m_allianceid;

	std::vector<int32_t> m_deletedhero;
	std::vector<int32_t> m_deletedcity;

	int RandomStat();


	Hero * CreateRandomHero(int innlevel);



	static bool compareprestige(stClientRank first, stClientRank second);
	static bool comparehonor(stClientRank first, stClientRank second);
	static bool comparetitle(stClientRank first, stClientRank second);
	static bool comparepop(stClientRank first, stClientRank second);
	static bool comparecities(stClientRank first, stClientRank second);

	void SortPlayers();


	std::list<stClientRank> m_prestigerank;
	std::list<stClientRank> m_honorrank;
	std::list<stClientRank> m_titlerank;
	std::list<stClientRank> m_populationrank;
	std::list<stClientRank> m_citiesrank;



	std::list<stHeroRank> m_herorankstratagem;
	std::list<stHeroRank> m_herorankpower;
	std::list<stHeroRank> m_herorankmanagement;
	std::list<stHeroRank> m_herorankgrade;

	void SortHeroes();
	static bool comparestratagem(stHeroRank first, stHeroRank second);
	static bool comparepower(stHeroRank first, stHeroRank second);
	static bool comparemanagement(stHeroRank first, stHeroRank second);
	static bool comparegrade(stHeroRank first, stHeroRank second);


	std::list<stCastleRank> m_castleranklevel;
	std::list<stCastleRank> m_castlerankpopulation;

	void SortCastles();
	static bool comparepopulation(stCastleRank first, stCastleRank second);
	static bool comparelevel(stCastleRank first, stCastleRank second);


	std::list<stSearchClientRank> m_searchclientranklist;
	std::list<stSearchHeroRank> m_searchheroranklist;
	std::list<stSearchCastleRank> m_searchcastleranklist;
	std::list<stSearchAllianceRank> m_searchallianceranklist;

	void * DoRankSearch(string key, int8_t type, void * subtype, int16_t page, int16_t pagesize);
	void CheckRankSearchTimeouts(uint64_t time);


	// Construct an error message to send to the client
	amf3object CreateError(string cmd, int32_t id, string message)
	{
		amf3object obj;
		obj["cmd"] = cmd;
		obj["data"] = amf3object();
		amf3object & data = obj["data"];
		data["errorMsg"] = message;
		data["packageId"] = 0.0f;
		data["ok"] = id;
		return obj;
	}
	// Construct an error message to send to the client
	// tested and functioning 
	// gserver->SendObject(&c, *gserver->CreateError2("error.error", -1, "message").get());
	std::shared_ptr<amf3object> CreateError2(string cmd, int32_t id, string message)
	{
		std::shared_ptr<amf3object> obj(new amf3object);
		obj->operator[]("cmd") = cmd;
		obj->operator[]("data") = amf3object();
		amf3object & data = obj->operator[]("data");
		data["errorMsg"] = message;
		data["packageId"] = 0.0f;
		data["ok"] = id;
		return obj;
	}

	amf3object CreateError3(string cmd, int32_t code, string message = "")
	{
		amf3object obj;
		obj["cmd"] = cmd;
		obj["data"] = amf3object();
		amf3object & data = obj["data"];
		data["reasonCode"] = code;
		data["msg"] = message;
		return obj;
	}

	bool CreateMail(string sender, string receiver, string subject, string content, int8_t type);
};