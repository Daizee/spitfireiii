//
// preport.cpp
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

#include "preport.h"
#include "../Server.h"
#include "../Client.h"
#include "../City.h"
#include "../Hero.h"


preport::preport(Server * server, request & req, amf3object & obj)
	: packet(server, req, obj)
{

}

preport::~preport()
{

}

void preport::process()
{
	if (command == "deleteReport")
	{
		string reportid = data["idStr"];//always string? always a stringed number?

		amf3object obj3;
		obj3["cmd"] = "report.deleteReport";
		obj3["data"] = amf3object();
		amf3object & data3 = obj3["data"];

		data3["packageId"] = 0.0f;
		data3["ok"] = 1;
		gserver->SendObject(client, obj3);
		return;
	}
	if (command == "markAsRead")
	{

		amf3object obj3;
		obj3["cmd"] = "report.receiveReportList";
		obj3["data"] = amf3object();
		amf3object & data3 = obj3["data"];

		amf3object report;
		report["id"] = 0;//report id (based on current reports you have, or ?)
		report["selected"] = false;
		report["title"] = "Plunder Reports";
		report["startPos"] = "City Name(317,374)";
		report["back"] = false;//??
		report["attack"] = false;//??
		report["targetPos"] = "Daisy(317,371)";
		report["eventTime"] = unixtime();
		report["type"] = 1;//1 = plunder?
		report["armyType"] = 5;//5 = attack?
		report["isRead"] = 0;//0 = unread, 1 = read?
		report["content"] = "<reportData reportUrl=\"http://battleceshi3.evony.com/default.html?20140613/46/f1/46f16df3fb6ca832bc7ac1a182c99060.xml\">\
<battleReport isAttack=\"true\" isAttackSuccess=\"false\" round=\"100\">\
<attackTroop king=\"Daisy\" heroName=\"shitatt\" heroLevel=\"1\" heroUrl=\"images/icon/player/faceA21s.png\">\
<troopUnit typeId=\"2\" count=\"1\" lose=\"0\"/>\
</attackTroop>\
<defendTroop king=\"Daisy1\"/>\
<backTroop>\
<troops heroLevel=\"1\" heroName=\"shitatt\" heroUrl=\"images/icon/player/faceA21s.png\" isHeroBeSeized=\"false\">\
<troopInfo typeId=\"2\" remain=\"1\"/>\
</troops>\
</backTroop>\
</battleReport>\
</reportData>";
		data3["report"] = report;

		gserver->SendObject(client, obj3);

		client->ReportUpdate();
		return;
	}
	if (command == "readOverReport")
	{

	}
	if (command == "receiveReportList")
	{
		int32_t pageno = obj["pageNo"];
		int32_t pagesize = obj["pageSize"];
		int32_t reporttype = obj["reportType"];


		amf3object obj3;
		obj3["cmd"] = "report.receiveReportList";
		obj3["data"] = amf3object();
		amf3object & data3 = obj3["data"];

		amf3array reports;
		amf3object report;
		report["id"] = 0;//report id (based on current reports you have, or ?)
		report["selected"] = false;
		report["title"] = "Plunder Reports";
		report["startPos"] = "City Name(317,374)";
		report["back"] = false;//??
		report["attack"] = false;//??
		report["targetPos"] = "Daisy(317,371)";
		report["eventTime"] = unixtime();
		report["type"] = 1;//1 = plunder?
		report["armyType"] = 5;//5 = attack?
		report["isRead"] = 0;//0 = unread, 1 = read?
		reports.Add(report);

		data3["reports"] = reports;
		data3["pageNo"] = pageno;
		data3["packageId"] = 0.0f;
		data3["ok"] = 1;
		data3["totalPage"] = 0;
		gserver->SendObject(client, obj3);
		return;
	}
}



