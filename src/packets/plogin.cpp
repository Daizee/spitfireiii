//
// plogin.cpp
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

#include "plogin.h"
#include "../Server.h"
#include "../Client.h"

plogin::plogin(Server * server, request & req, amf3object & obj)
	: packet(server, req, obj)
{

}

plogin::~plogin()
{

}

void plogin::process()
{
	obj2["data"] = amf3object();
	amf3object & data2 = obj2["data"];

	//errors:
	//-5 = captcha
	//-99 = general error
	//-100 = holiday
	string username = data["user"];
	string password = data["pwd"];

	if (gserver->maxplayers <= gserver->currentplayersonline + 1)
	{
		gserver->SendObject(req.conn, gserver->CreateError("server.LoginResponse", -99, "Servers are currently overloaded. Please try again later."));
		return;
	}

	string newuser;
	string newpass;
	newuser = makesafe(username);
	newpass = makesafe(password);


	{
		Session ses(gserver->accountpool->get());
		Statement select(ses);
		select << "SELECT COUNT(*) AS a FROM `account` WHERE `email`=?;", use(newuser);
		select.execute();
		RecordSet rs(select);

		if (rs.value("a").convert<int32_t>() == 0)
		{
			//account does not exist - insert new row
			try
			{
				Statement insert(ses);
				insert << "INSERT INTO `account` (`name`, `email`, `ip`, `lastlogin`, `creation`, `password`, `status`, `reason`) VALUES ('null', ?, '', ?, ?, ?, 0, '');", use(newuser), use(unixtime()), use(unixtime()), use(newpass), now;
			}
			catch (Poco::Data::MySQL::StatementException * e)
			{
				gserver->consoleLogger->error(Poco::format("Account Create Exception: %s", e->displayText()));
				return;
			}
		}
	}

	{
		Session ses(gserver->accountpool->get());
		Statement select(ses);
		select << "SELECT * FROM `account` WHERE `email`=? AND `password`=?;", use(newuser), use(newpass);
		select.execute();
		RecordSet rs(select);

		if (rs.rowCount() == 0)
		{
			//account doesn't exist or password is wrong
			gserver->SendObject(req.conn, gserver->CreateError("server.LoginResponse", -2, "Incorrect account or password."));
			return;
		}
		else
		{
			int32_t masteraccountid = rs.value("id").convert<int32_t>();
			client = gserver->GetClientByParent(masteraccountid);

			bool banned = false;

			{
				//are they banned? if so, globally or for this server?
				Session ses2(gserver->serverpool->get());
				Statement select2(ses2);
				select2 << "SELECT * FROM `accounts` WHERE `parentid`=?;", use(masteraccountid);
				select2.execute();
				RecordSet rs2(select2);

				if (rs.value("status").convert<int32_t>() == -99)
					banned = true;

				if (rs2.rowCount() > 0 && rs2.value("status").convert<int32_t>() == -99)
					banned = true;

				if (banned)
				{
					string errormsg = "You are banned. Reason: ";
					errormsg += rs.value("reason").convert<string>().length() > 0 ? rs.value("reason").convert<string>() : rs2.value("reason").convert<string>();

					gserver->SendObject(req.conn, gserver->CreateError("server.LoginResponse", -99, errormsg));

					return;
				}
			}

			//client = gserver->GetClientByParent(parentid);
			if (client == 0)
			{
				client = gserver->NewClient();
				client->masteraccountid = masteraccountid;
				client->m_socknum = req.conn->uid;
				client->socket = req.conn;
				req.conn->client_ = client;
				client->m_connected = true;
			}
			else
			{
				if (client->m_connected)
				{
					//player already logged on
					gserver->CloseClient(client, 3, "");//multiple people logging into the same account
				}
				//Login is valid
				client->m_connected = true;
				double logintime = unixtime();
				if (logintime - client->m_lastlogin < 1000 * 5)
				{
					gserver->SendObject(req.conn, gserver->CreateError("server.LoginResponse", 6, "You have tried logging in too frequently. Please try again later."));
					req.conn->stop();
					return;
				}
				client->m_lastlogin = logintime;
				if (client->socket) gserver->CloseClient(client, 3, "");
				client->socket = req.conn;
				client->m_socknum = req.conn->uid;
				client->m_ipaddress = req.conn->address;
				req.conn->client_ = client;
				gserver->consoleLogger->information(Poco::format("Already established client found # %?d", (uint32_t)client->m_clientnumber));

				if (client->m_email == "Daisy")
				{
					client->m_bdenyotherplayer = true;
					client->m_icon = 7;
				}
			}

			if (client == 0)
			{
				//error creating client object
				gserver->consoleLogger->information(Poco::format("Error creating client object @ %s:%?d", (string)__FILE__, __LINE__));
				gserver->SendObject(client, gserver->CreateError("server.LoginResponse", -99, "Error with connecting. Please contact support."));
				return;
			}


			//account exists
			Session ses2(gserver->serverpool->get());
			Statement select2(ses2);
			select2 << "SELECT * FROM `accounts` WHERE `parentid`=?;", use(masteraccountid);
			select2.execute();
			RecordSet rs2(select2);

			if (rs2.rowCount() == 0)
			{
				//does not have an account on server
				gserver->SendObject(client, gserver->CreateError("server.LoginResponse", -4, "need create player"));
				client->m_loggedin = true;

				return;
			}
			else
			{
				int accountid = rs2.value("accountid").convert<int32_t>();
				client->m_accountid = accountid;

				//has an account, what about cities?
				Session ses3(gserver->serverpool->get());
				Statement select3(ses3);
				select3 << "SELECT * FROM `cities` WHERE `accountid`=?;", use(accountid);
				select3.execute();
				RecordSet rs3(select3);

				if (rs3.rowCount() == 0)
				{
					//does not have any cities on server but did have an account - this only happens if you try to "restart" your account. it saves the account info while deleting your cities
					gserver->SendObject(client, gserver->CreateError("server.LoginResponse", -4, "need create player"));
					client->m_loggedin = true;
					return;
				}
				else
				{
					//has an account and cities. process the list and send account info

					amf3object obj;
					obj["cmd"] = "server.LoginResponse";
					obj["data"] = amf3object();
					amf3object & data = obj["data"];
					data["packageId"] = 0.0f;

					double tslag = unixtime();

					if (client->GetItemCount("consume.1.a") < 10000)
						client->SetItem("consume.1.a", 10000);
					client->m_cents = 5000;

					data["player"] = client->ToObject();
					//UNLOCK(M_CLIENTLIST);

					if (client->m_city.size() == 0)
					{
						//problem
						gserver->consoleLogger->error(Poco::format("Error client has no cities @ %s:%?d", (string)__FILE__, __LINE__));
						gserver->SendObject(client, gserver->CreateError("server.LoginResponse", -99, "Error with connecting. Please contact support."));
						return;
					}
					client->m_currentcityid = ((PlayerCity*)client->m_city.at(0))->m_castleid;
					client->m_currentcityindex = 0;
					client->m_accountexists = true;


					//check for holiday status
					stBuff * holiday = client->GetBuff("FurloughBuff");
					if (holiday && holiday->endtime > tslag)
					{
						//is in holiday - send holiday info too

						string s;
						{
							int32_t hours;
							int32_t mins;
							int32_t secs = (holiday->endtime - tslag) / 1000;

							hours = secs / 60 / 60;
							mins = secs / 60 - hours * 60;
							secs = secs - mins * 60 - hours * 60 * 60;

							std::stringstream ss;
							ss << hours << "," << mins << "," << secs;

							s = ss.str();
						}

						data["ok"] = -100;
						data["msg"] = s;
						data["errorMsg"] = s;
					}
					else
					{
						data["ok"] = 1;
						data["msg"] = "success";
					}

					gserver->SendObject(client, obj);
					//SendObject(*req.connection, obj);

					client->m_lag = unixtime() - tslag;

					client->m_loggedin = true;

					gserver->currentplayersonline++;
					client->SaveToDB();

					return;
				}
			}
		}
	}
	return;
}


