//
// funcs.h
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

#ifdef WIN32
#include <SDKDDKVer.h>
#include <tchar.h>
#include <direct.h>
#include <process.h>
//#else
//#include <thread>
#endif

#include <stdarg.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <queue>

#include <stdio.h>

#include <memory>
#include <memory.h>

#include "amf3.h"

#include <math.h>

using namespace std;

extern char itoh(int num);
extern int htoi(char hex);
extern void a_swap(unsigned char * a, unsigned char * b);
extern void ByteSwap5(unsigned char * b, int n);
extern uint64_t unixtime();
extern char * GetBuildingName(int id);

extern char * strtolower(char * x);
extern string makesafe(string in);
extern size_t ci_find(const string& str1, const string& str2);
extern bool ci_equal(char ch1, char ch2);

#define SERVERSTATUS_STOPPED 1
#define SERVERSTATUS_STARTING 2
#define SERVERSTATUS_ONLINE 3
#define SERVERSTATUS_SHUTDOWN 4


#define ByteSwap(x) ByteSwap5((unsigned char *) &x, sizeof(x))

#ifdef WIN32

#ifndef VA_COPY
# ifdef HAVE_VA_COPY
#  define VA_COPY(dest, src) va_copy(dest, src)
# else
#  ifdef HAVE___VA_COPY
#   define VA_COPY(dest, src) __va_copy(dest, src)
#  else
#   define VA_COPY(dest, src) (dest) = (src)
#  endif
# endif
#endif

#define INIT_SZ 1024

extern int vasprintf(char **str, const char *fmt, va_list ap);
extern int asprintf(char **str, const char *fmt, ...);

#endif

#define DEF_SOCKETTHREADS 2

#define DEF_STATES 16

#define DEF_STATE1 "FRIESLAND"
#define DEF_STATE2 "SAXONY"
#define DEF_STATE3 "NORTH MARCH"
#define DEF_STATE4 "BOHEMIA"
#define DEF_STATE5 "LOWER LORRAINE"
#define DEF_STATE6 "FRANCONIA"
#define DEF_STATE7 "THURINGIA"
#define DEF_STATE8 "MORAVIA"
#define DEF_STATE9 "UPPER LORRAINE"
#define DEF_STATE10 "SWABIA"
#define DEF_STATE11 "BAVARIA"
#define DEF_STATE12 "CARINTHIA"
#define DEF_STATE13 "BURGUNDY"
#define DEF_STATE14 "LOMBARDY"
#define DEF_STATE15 "TUSCANY"
#define DEF_STATE16 "ROMAGNA"

//begin mutexes
#define M_CLIENTLIST 1
#define M_HEROLIST 2
#define M_CASTLELIST 3
#define M_ALLIANCELIST 4
#define M_TIMEDLIST 5
#define M_MAP 6
#define M_DELETELIST 7
#define M_RANKEDLIST 8

// #define MULTILOCK(a,b) while (!server->m_mutex.multilock(a,b,0, __LINE__)) { SLEEP(10); }
// #define MULTILOCK2(a,b,c) while (!server->m_mutex.multilock(a,b,c, __LINE__)) { SLEEP(10); }
// #define LOCK(a)	while (!server->m_mutex.lock(a, __LINE__)) { SLEEP(10); }
// #define UNLOCK(a) server->m_mutex.unlock(a, __LINE__)
#define MULTILOCK(a,b)
#define MULTILOCK2(a,b,c)
#define LOCK(a)
#define UNLOCK(a)

//end mutexes

#define DEF_RESEARCH 1
#define DEF_BUILDING 2
#define DEF_TRAIN 3

#define DEF_NORMAL 0
#define DEF_PEACETIME 1
#define DEF_TRUCE 2
#define DEF_BEGINNER 3
#define DEF_HOLIDAY 5

#define FOREST 1
#define DESERT 2
#define HILL 3
#define SWAMP 4
#define GRASS 5
#define LAKE 6
#define FLAT 10
#define CASTLE 11
#define NPC 12


#define WORKER 1
#define WARRIOR 2
#define SCOUT 3
#define PIKE 4
#define SWORDS 5
#define ARCHER 6
#define CAVALRY 7
#define CATAPHRACT 8
#define TRANSPORTER 9
#define RAM 10
#define CATAPULT 11

#define DEF_ALLIANCE 0
#define DEF_ALLY 1
#define DEF_NEUTRAL 2
#define DEF_ENEMY 3
#define DEF_NORELATION 4
#define DEF_SELFRELATION 5

#define DEF_ALLIANCEHOST 4
#define DEF_ALLIANCEVICEHOST 5
#define DEF_ALLIANCEPRES 6
#define DEF_ALLIANCEOFFICER 7
#define DEF_ALLIANCEMEMBER 8


#define DEF_HEROIDLE 0
#define DEF_HEROMAYOR 1
#define DEF_HEROREINFORCE //exists?
#define DEF_HEROATTACK 3
#define DEF_HEROSCOUT //exists?
#define DEF_HERORETURN 5

#define DEF_MAXITEMS 400

#define GETX xfromid
#define GETY yfromid
#define GETID idfromxy

#define GETXYFROMID4(b,a,c,d) a = short(c / d); b = short(c % d);//x, y, id, mapsize

#define GETXYFROMID(a) short yfromid = short(a / gserver->mapsize); short xfromid = short(a % gserver->mapsize);

#define GETIDFROMXY(x,y) int idfromxy = y*gserver->mapsize+x;

#define DEF_RESEARCH 1
#define DEF_BUILDING 2
#define DEF_TRAIN 3




// BUILDING IDS

#define B_COTTAGE 1
#define B_BARRACKS 2
#define B_WAREHOUSE 3
#define B_SAWMILL 4
#define B_STONEMINE 5
#define B_IRONMINE 6
#define B_FARM 7
#define B_STABLE 20
#define B_INN 21
#define B_FORGE 22
#define B_MARKETPLACE 23
#define B_RELIEFSTATION 24
#define B_ACADEMY 25
#define B_WORKSHOP 26
#define B_FEASTINGHALL 27
#define B_EMBASSY 28
#define B_RALLYSPOT 29
#define B_BEACONTOWER 30
#define B_TOWNHALL 31
#define B_WALLS 32

// TROOP IDS
#define TR_WORKER 2
#define TR_WARRIOR 3
#define TR_SCOUT 4
#define TR_PIKE 5
#define TR_SWORDS 6
#define TR_ARCHER 7
#define TR_TRANSPORTER 8
#define TR_CAVALRY 9
#define TR_CATAPHRACT 10
#define TR_BALLISTA 11
#define TR_RAM 12
#define TR_CATAPULT 13

#define TR_TRAP 14
#define TR_ABATIS 15
#define TR_ARCHERTOWER 16
#define TR_ROLLINGLOG 17
#define TR_TREBUCHET 18


// RESEARCH IDS
#define T_AGRICULTURE 1
#define T_LUMBERING 2
#define T_MASONRY 3
#define T_MINING 4
#define T_METALCASTING 5
#define T_INFORMATICS 7
#define T_MILITARYSCIENCE 8
#define T_MILITARYTRADITION 9
#define T_IRONWORKING 10
#define T_LOGISTICS 11
#define T_COMPASS 12
#define T_HORSEBACKRIDING 13
#define T_ARCHERY 14
#define T_STOCKPILE 15
#define T_MEDICINE 16
#define T_CONSTRUCTION 17
#define T_ENGINEERING 18
#define T_MACHINERY 19
#define T_PRIVATEERING 20


#ifndef WIN32
#define strtok_s strtok_r
#define _atoi64 atoll
#define sprintf_s snprintf
#define strcpy_s(a,b,c) strcpy(a,c)
#endif


#define KeyExists(x,y) ((x._object->Exists(y))>=0)
#define IsString(x) (x.type==String)
#define IsObject(x) (x.type==Object)

#define CHECKCASTLEID() \
	if (IsObject(data) && KeyExists(data, "castleId") && (int)data["castleId"] != client->m_currentcityid) \
		{ \
	gserver->consoleLogger->information(Poco::format("castleId does not match castle focus! gave: %d is:%d - cmd: %s - accountid:%d - playername: %s", (int)data["castleId"], client->m_currentcityid, cmd.c_str(), client->m_accountid, (char*)client->m_playername.c_str())); \
		}

#define VERIFYCASTLEID() \
	if (!IsObject(data) || !KeyExists(data, "castleId") ) \
		{ \
	gserver->consoleLogger->information("castleId not received!"); \
	return; \
		}



#define SQLCATCH(a)	catch (Poco::Data::MySQL::ConnectionException& e)\
{\
	consoleLogger->error(Poco::format("ConnectionException: %s", e.displayText() ));\
	a; \
}\
	catch (Poco::Data::MySQL::StatementException& e)\
{\
	consoleLogger->error(Poco::format("StatementException: %s", e.displayText() ));\
	a; \
}\
	catch (Poco::Data::MySQL::MySQLException& e)\
{\
	consoleLogger->error(Poco::format("MySQLException: %s", e.displayText() ));\
	a; \
}\
	catch (Poco::InvalidArgumentException& e)\
{\
	consoleLogger->error(Poco::format("InvalidArgumentException: %s", e.displayText() ));\
	a; \
}



//#define _HAS_ITERATOR_DEBUGGING 1
//#define _ITERATOR_DEBUG_LEVEL 2


#ifdef WIN32
#define DBL "%Lf"
#define DBL2 "Lf"
#define XI64 "%I64d"
#else
#define DBL "%f"
#define DBL2 "f"
#define XI64 "%lld"
#endif



#ifdef WIN32
#define SLEEP(a) Sleep(a)
#else
#define SLEEP(a) { struct timespec req={0}; req.tv_sec = 0; req.tv_nsec = 1000000 * a; nanosleep(&req,NULL); }
#endif

#define SOCKET_SERVER 2

//#define DEF_MAXCLIENTS 1024
#define DEF_MAXPHP 100
#define DEF_MAXALLIANCES 1000
#define DEF_MAXITEMS 400

#define DEF_LISTENSOCK 1
#define DEF_NORMALSOCK 2



