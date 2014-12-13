//
// palliance.cpp
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

#include "palliance.h"
#include "../Client.h"
#include "../City.h"
#include "../Server.h"
#include "../Alliance.h"
#include "../AllianceCore.h"

palliance::palliance(Server * server, request & req, amf3object & obj)
	: packet(server, req, obj)
{

}

palliance::~palliance()
{

}

void palliance::process()
{
	if ((command == "sayByetoAlliance"))
	{
		if (client->HasAlliance())
		{
			Alliance * alliance = client->GetAlliance();
			alliance->RemoveMember(client->m_accountid);
			alliance->SendAllianceMessage(client->m_playername + " has left the alliance", false, true);
			client->m_allianceid = -1;
			client->m_alliancename = "";
			client->m_alliancerank = 0;
			client->m_allianceapply = "";
			client->m_allianceapplytime = 0;
		}
		return;
	}
	if ((command == "isHasAlliance"))
	{
		obj2["cmd"] = "alliance.isHasAlliance";


		if (client->HasAlliance())
		{
			data2["ok"] = 1;
			data2["packageId"] = 0.0f;


			LOCK(M_ALLIANCELIST);
			Alliance * alliance = client->GetAlliance();
			Alliance * tempalliance;

			data2["serialVersionUID"] = 1.0f;

			amf3array middleList = amf3array();
			amf3array enemyList = amf3array();
			amf3array friendlyList = amf3array();

			std::vector<int64_t>::iterator iter;
			if (alliance->m_neutral.size() > 0)
			{
				for (iter = alliance->m_neutral.begin(); iter != alliance->m_neutral.end(); ++iter)
				{
					tempalliance = client->m_main->m_alliances->AllianceById(*iter);
					if (tempalliance != (Alliance*)-1)
					{
						amf3object ta = amf3object();
						ta["rank"] = tempalliance->m_prestigerank;
						ta["honor"] = tempalliance->m_honor;// HACK: might be 0 -- alliance.isHasAlliance
						ta["allianceName"] = tempalliance->m_name.c_str();
						ta["memberCount"] = tempalliance->m_currentmembers;
						ta["aPrestigeCount"] = tempalliance->m_prestige;// HACK: might be 0 -- alliance.isHasAlliance
						middleList.Add(ta);
					}
				}
			}
			if (alliance->m_enemies.size() > 0)
			{
				for (iter = alliance->m_enemies.begin(); iter != alliance->m_enemies.end(); ++iter)
				{
					tempalliance = client->m_main->m_alliances->AllianceById(*iter);
					if (tempalliance != (Alliance*)-1)
					{
						amf3object ta = amf3object();
						ta["rank"] = tempalliance->m_prestigerank;
						ta["honor"] = tempalliance->m_honor;// HACK: might be 0 -- alliance.isHasAlliance
						ta["allianceName"] = tempalliance->m_name.c_str();
						ta["memberCount"] = tempalliance->m_currentmembers;
						ta["aPrestigeCount"] = tempalliance->m_prestige;// HACK: might be 0 -- alliance.isHasAlliance
						enemyList.Add(ta);
					}
				}
			}
			if (alliance->m_allies.size() > 0)
			{
				for (iter = alliance->m_allies.begin(); iter != alliance->m_allies.end(); ++iter)
				{
					tempalliance = client->m_main->m_alliances->AllianceById(*iter);
					if (tempalliance != (Alliance*)-1)
					{
						amf3object ta = amf3object();
						ta["rank"] = tempalliance->m_prestigerank;
						ta["honor"] = tempalliance->m_honor;// HACK: might be 0 -- alliance.isHasAlliance
						ta["allianceName"] = tempalliance->m_name.c_str();
						ta["memberCount"] = tempalliance->m_currentmembers;
						ta["aPrestigeCount"] = tempalliance->m_prestige;// HACK: might be 0 -- alliance.isHasAlliance
						friendlyList.Add(ta);
					}
				}
			}
			data2["middleList"] = middleList;
			data2["enemyList"] = enemyList;
			data2["friendlyList"] = friendlyList;
			UNLOCK(M_ALLIANCELIST);


			data2["indexAllianceInfoBean"] = alliance->indexAllianceInfoBean();
		}
		else
		{
			data2["ok"] = -99;
			data2["packageId"] = 0.0f;
			data2["serialVersionUID"] = 0.0f;
			data2["errorMsg"] = "You are not in any alliance.";
		}

		gserver->SendObject(client, obj2);
		return;
	}
	if ((command == "leaderWantUserInAllianceList"))
	{
		obj2["cmd"] = "alliance.leaderWantUserInAllianceList";
		data2["ok"] = 1;
		data2["packageId"] = 0.0f;

		LOCK(M_ALLIANCELIST);
		data2["allianceAddPlayerByLeaderInfoBeanList"] = amf3array();

		//TODO: this is your list of alliances that have invited you?-- alliance.leaderWantUserInAllianceList

		amf3array & allianceinfo = *(amf3array*)data2["allianceAddPlayerByLeaderInfoBeanList"];

		for (int i = 0; i < DEF_MAXALLIANCES; ++i)
		{
			if (gserver->m_alliances->m_alliances[i])
			{
				Alliance * alliance = gserver->m_alliances->m_alliances[i];
				if (alliance->m_invites.size())
				{
					std::vector<Alliance::stInviteList>::iterator iter;
					for (iter = alliance->m_invites.begin(); iter != alliance->m_invites.end(); ++iter)
					{
						if (iter->client->m_accountid == client->m_accountid)
						{
							amf3object invite = amf3object();
							invite["prestige"] = alliance->m_prestige;
							invite["rank"] = alliance->m_prestigerank;
							invite["allianceName"] = alliance->m_name;
							invite["memberCount"] = alliance->m_currentmembers;
							allianceinfo.Add(invite);
						}
					}
				}
			}
		}


		UNLOCK(M_ALLIANCELIST);
		gserver->SendObject(client, obj2);
		return;
	}
	if ((command == "rejectComeinAlliance"))
	{
		obj2["cmd"] = "alliance.rejectComeinAlliance";
		data2["packageId"] = 0.0f;

		string alliancename = data["allianceName"];

		Alliance * alliance = gserver->m_alliances->AllianceByName(alliancename);
		if (alliance == (Alliance*)-1)
		{
			gserver->SendObject(client, gserver->CreateError("alliance.rejectComeinAlliance", -99, "Alliance no longer exists."));
			return;
		}

		LOCK(M_ALLIANCELIST);
		if (alliance->m_invites.size())
		{
			std::vector<Alliance::stInviteList>::iterator iter;
			for (iter = alliance->m_invites.begin(); iter != alliance->m_invites.end(); ++iter)
			{
				if (iter->client->m_accountid == client->m_accountid)
				{
					alliance->m_invites.erase(iter);
					data2["ok"] = 1;

					gserver->SendObject(client, obj2);
					return;
				}
			}
		}
		else
		{
			gserver->SendObject(client, gserver->CreateError("alliance.rejectComeinAlliance", -99, "Invite no longer exists."));
			return;
		}
		UNLOCK(M_ALLIANCELIST);
		gserver->SendObject(client, gserver->CreateError("alliance.rejectComeinAlliance", -99, "An unknown error occurred."));
		return;
	}
	if ((command == "agreeComeinAllianceList"))
	{
		obj2["cmd"] = "alliance.agreeComeinAllianceList";
		data2["packageId"] = 0.0f;

		if ((client->m_allianceid == 0) || (client->m_alliancerank > DEF_ALLIANCEPRES))
		{
			gserver->SendObject(client, gserver->CreateError("alliance.agreeComeinAllianceList", -99, "Not enough rank."));
			return;
		}

		LOCK(M_ALLIANCELIST);
		Alliance * alliance = gserver->m_alliances->AllianceById(client->m_allianceid);

		if (alliance == (Alliance*)-1)
		{
			gserver->SendObject(client, gserver->CreateError("alliance.agreeComeinAllianceList", -99, "Alliance does not exist."));
			return;
		}

		amf3array allianceinfo = amf3array();

		for (int i = 0; i < alliance->m_invites.size(); ++i)
		{
			if (alliance->m_invites[i].inviteperson.length() == 0)
			{
				Client * pcl = alliance->m_invites[i].client;
				amf3object iobj = amf3object();
				iobj["prestige"] = pcl->Prestige();
				iobj["rank"] = pcl->m_prestigerank;
				iobj["userName"] = pcl->m_playername;
				iobj["inviteTime"] = alliance->m_invites[i].invitetime;
				iobj["castleCount"] = pcl->m_citycount;
				allianceinfo.Add(iobj);
			}
		}
		UNLOCK(M_ALLIANCELIST);
		data2["ok"] = 1;

		data2["allianceAddPlayerByUserInfoBeanList"] = allianceinfo;

		gserver->SendObject(client, obj2);
		return;
	}
	if ((command == "resignForAlliance"))
	{
		obj2["cmd"] = "alliance.resignForAlliance";
		data2["packageId"] = 0.0f;

		if (!client->HasAlliance())
		{
			gserver->SendObject(client, gserver->CreateError("alliance.resignForAlliance", -99, "You are not in an alliance."));
			return;
		}
		Alliance * alliance = client->GetAlliance();
		if ((client->m_alliancerank == DEF_ALLIANCEHOST) && (alliance->m_currentmembers > 1))
		{
			gserver->SendObject(client, gserver->CreateError("alliance.resignForAlliance", -99, "Resignation refused. Please transfer your host title to other before you resign."));
			return;
		}
		else
		{
			//Only person left is host - disband alliance
			if (!gserver->m_alliances->RemoveFromAlliance(client->m_allianceid, client))
			{
				gserver->SendObject(client, gserver->CreateError("alliance.resignForAlliance", -99, "Unable to leave alliance. Please contact support."));
				return;
			}
			gserver->m_alliances->DeleteAlliance(alliance);
			return;
		}

		LOCK(M_ALLIANCELIST);
		if (!gserver->m_alliances->RemoveFromAlliance(client->m_allianceid, client))
		{
			gserver->SendObject(client, gserver->CreateError("alliance.resignForAlliance", -99, "Unable to leave alliance. Please contact support."));
			UNLOCK(M_ALLIANCELIST);
			return;
		}
		UNLOCK(M_ALLIANCELIST);

		data2["ok"] = 1;

		client->SaveToDB();

		gserver->SendObject(client, obj2);
		return;
	}
	if ((command == "getAllianceMembers"))
	{
		obj2["cmd"] = "alliance.getAllianceMembers";
		data2["packageId"] = 0.0f;

		if (client->m_allianceid < 0)
		{
			gserver->SendObject(client, gserver->CreateError("alliance.getAllianceMembers", -99, "Not a member of an alliance."));
			return;
		}
		MULTILOCK(M_ALLIANCELIST, M_CLIENTLIST);
		Alliance * alliance = gserver->m_alliances->AllianceById(client->m_allianceid);

		if (alliance == (Alliance*)-1)
		{
			gserver->SendObject(client, gserver->CreateError("alliance.getAllianceMembers", -99, "Invalid Alliance."));
			UNLOCK(M_CLIENTLIST);
			UNLOCK(M_ALLIANCELIST);
			return;
		}

		amf3array members = amf3array();

		for (int k = 0; k < alliance->m_members.size(); ++k)
		{
			Client * client = gserver->GetClient(alliance->m_members[k].clientid);
			if (client)
			{
				amf3object temp = amf3object();
				temp["createrTime"] = 0;
				temp["alliance"] = alliance->m_name;
				temp["office"] = client->m_office;
				temp["allianceLevel"] = AllianceCore::GetAllianceRank(client->m_alliancerank);
				temp["sex"] = client->m_sex;
				temp["levelId"] = client->m_alliancerank;
				temp["honor"] = client->m_honor;
				temp["bdenyotherplayer"] = client->m_bdenyotherplayer;
				temp["id"] = client->m_accountid;
				temp["accountName"] = "";
				temp["prestige"] = client->Prestige();
				temp["faceUrl"] = client->m_faceurl;
				temp["flag"] = client->m_flag;
				temp["userId"] = client->masteraccountid;
				temp["userName"] = client->m_playername;
				temp["castleCount"] = client->m_citycount;
				temp["titleId"] = client->m_title;
				temp["medal"] = 0;
				temp["ranking"] = client->m_prestigerank;
				temp["lastLoginTime"] = client->m_lastlogin;
				temp["population"] = client->m_population;
				members.Add(temp);
			}
		}

		data2["ok"] = 1;
		data2["members"] = members;

		UNLOCK(M_CLIENTLIST);
		UNLOCK(M_ALLIANCELIST);
		gserver->SendObject(client, obj2);
		return;
	}
	if ((command == "getAllianceWanted"))
	{
		obj2["cmd"] = "alliance.getAllianceWanted";
		data2["ok"] = 1;
		data2["packageId"] = 0.0f;

		if (client->m_allianceapply.length() > 0)
			data2["allianceName"] = client->m_allianceapply;

		gserver->SendObject(client, obj2);
		return;
	}
	if ((command == "getAllianceInfo"))
	{
		obj2["cmd"] = "alliance.getAllianceInfo";
		data2["ok"] = 1;
		data2["packageId"] = 0.0f;

		string alliancename = data["allianceName"];

		LOCK(M_ALLIANCELIST);
		Alliance * alliance = gserver->m_alliances->AllianceByName(alliancename);

		if (alliance == (Alliance*)-1)
		{
			gserver->SendObject(client, gserver->CreateError("alliance.getAllianceInfo", -99, "Alliance does not exist."));
			return;
		}

		data2["leader"] = alliance->m_owner;
		data2["prestigeCount"] = alliance->m_prestige;
		data2["ranking"] = alliance->m_prestigerank;
		data2["memberCount"] = alliance->m_currentmembers;
		data2["allinaceInfo"] = alliance->m_intro;
		data2["creator"] = alliance->m_founder;
		UNLOCK(M_ALLIANCELIST);


		data2["ok"] = 1;
		data2["packageId"] = 0.0f;

		gserver->SendObject(client, obj2);
		return;
	}
	if ((command == "userWantInAlliance"))
	{
		obj2["cmd"] = "alliance.userWantInAlliance";
		data2["ok"] = 1;
		data2["packageId"] = 0.0f;

		string alliancename = data["allianceName"];

		Alliance * alliance = gserver->m_alliances->AllianceByName(alliancename);

		if (alliance == (Alliance*)-1)
		{
			gserver->SendObject(client, gserver->CreateError("alliance.userWantInAlliance", -99, "Alliance does not exist."));
			return;
		}

		if (client->m_allianceapply.length() != 0)
		{
			gserver->m_alliances->AllianceByName(client->m_allianceapply)->UnRequestJoin(client);
		}

		alliance->RequestJoin(client, timestamp);


		client->m_allianceapply = alliancename;
		client->m_allianceapplytime = timestamp;

		alliance->SendAllianceMessage(client->m_playername + " has applied to join your alliance", false, false);

		data2["ok"] = 1;
		data2["packageId"] = 0.0f;

		client->SaveToDB();

		gserver->SendObject(client, obj2);
		return;
	}
	if ((command == "cancelUserWantInAlliance"))
	{
		obj2["cmd"] = "alliance.cancelUserWantInAlliance";
		data2["ok"] = 1;
		data2["packageId"] = 0.0f;

		string alliancename = data["allianceName"];

		if (client->m_allianceapply.length() == 0)
		{
			gserver->SendObject(client, gserver->CreateError("alliance.cancelUserWantInAlliance", -99, "Not applied."));
			return;
		}

		LOCK(M_ALLIANCELIST);
		Alliance * alliance = gserver->m_alliances->AllianceByName(alliancename);

		if (alliance == (Alliance*)-1)
		{
			gserver->SendObject(client, gserver->CreateError("alliance.cancelUserWantInAlliance", -99, "Alliance does not exist."));
			return;
		}

		alliance->UnRequestJoin(client);
		UNLOCK(M_ALLIANCELIST);


		client->m_allianceapply = "";
		client->m_allianceapplytime = 0;

		client->SaveToDB();

		data2["ok"] = 1;
		data2["packageId"] = 0.0f;

		gserver->SendObject(client, obj2);
		return;
	}
	if ((command == "getMilitarySituationList"))
	{
		int pagesize = data["pageSize"];
		int pageno = data["pageNo"];

		obj2["cmd"] = "alliance.getMilitarySituationList";
		data2["packageId"] = 0.0f;
		data2["ok"] = 1;

		gserver->SendObject(client, obj2);
		return;
	}
	if ((command == "addUsertoAllianceList"))
	{
		obj2["cmd"] = "alliance.addUsertoAllianceList";
		data2["packageId"] = 0.0f;

		if (!client->HasAlliance())
		{
			gserver->SendObject(client, gserver->CreateError("alliance.addUsertoAllianceList", -99, "Not a member of an alliance."));
			return;
		}

		data2["ok"] = 1;

		LOCK(M_ALLIANCELIST);
		amf3array listarray = amf3array();

		Alliance * alliance = client->GetAlliance();

		for (int i = 0; i < alliance->m_invites.size(); ++i)
		{
			if (alliance->m_invites[i].inviteperson.length() != 0)
			{
				Client * pcl = alliance->m_invites[i].client;
				amf3object iobj = amf3object();
				iobj["prestige"] = pcl->m_prestige;
				iobj["rank"] = pcl->m_prestigerank;
				iobj["invitePerson"] = alliance->m_invites[i].inviteperson;
				iobj["userName"] = pcl->m_playername;
				iobj["inviteTime"] = alliance->m_invites[i].invitetime;
				listarray.Add(iobj);
			}
		}
		UNLOCK(M_ALLIANCELIST);

		data2["allianceAddPlayerInfoBeanList"] = listarray;

		gserver->SendObject(client, obj2);
		return;
	}
	if ((command == "addUsertoAlliance"))
	{
		//invite player
		obj2["cmd"] = "alliance.addUsertoAlliance";
		data2["packageId"] = 0.0f;

		if (!client->HasAlliance())
		{
			gserver->SendObject(client, gserver->CreateError("alliance.addUsertoAlliance", -99, "Not a member of an alliance."));
			return;
		}
		if (client->m_alliancerank > DEF_ALLIANCEPRES)
		{
			gserver->SendObject(client, gserver->CreateError("alliance.addUsertoAlliance", -99, "Not enough rank."));
			return;
		}

		//TODO: copy 24h alliance join cooldown?
		/*
		["data"] Type: Object - Value: Object
		["packageId"] Type: Number - Value: 0.000000
		["ok"] Type: Integer - Value: -301
		["errorMsg"] Type: String - Value: Can not join the same Alliance again in 23 hours.
		*/

		MULTILOCK(M_ALLIANCELIST, M_CLIENTLIST);
		string username = data["userName"];
		Client * invitee = gserver->GetClientByName(username);

		if (invitee == 0)
		{
			string msg;
			msg = "Player ";
			msg += username;
			msg += " doesn't exist.";
			gserver->SendObject(client, gserver->CreateError("alliance.addUsertoAlliance", -41, msg));
			UNLOCK(M_ALLIANCELIST);
			UNLOCK(M_CLIENTLIST);
			return;
		}
		if (invitee->HasAlliance())
		{
			string msg;
			msg = username;
			msg += "is already a member of other alliance.";
			gserver->SendObject(client, gserver->CreateError("alliance.addUsertoAlliance", -99, msg));
			UNLOCK(M_ALLIANCELIST);
			UNLOCK(M_CLIENTLIST);
			return;
		}
		data2["ok"] = 1;

		Alliance * alliance = client->GetAlliance();

		Alliance::stInviteList invite;
		invite.inviteperson = client->m_playername;
		invite.invitetime = timestamp;
		invite.client = invitee;

		alliance->m_invites.push_back(invite);
		UNLOCK(M_ALLIANCELIST);
		UNLOCK(M_CLIENTLIST);

		gserver->SendMessage(client, "You have been invited to the alliance " + alliance->m_name);

		gserver->SendObject(client, obj2);

		alliance->SaveToDB();

		return;
	}
	if ((command == "canceladdUsertoAlliance"))
	{
		//TODO: permission check
		obj2["cmd"] = "alliance.canceladdUsertoAlliance";
		data2["packageId"] = 0.0f;

		if (!client->HasAlliance())
		{
			gserver->SendObject(client, gserver->CreateError("alliance.canceladdUsertoAlliance", -99, "Not a member of an alliance."));
			return;
		}

		MULTILOCK(M_ALLIANCELIST, M_CLIENTLIST);
		string username = data["userName"];

		data2["ok"] = 1;

		Alliance * alliance = client->GetAlliance();

		alliance->UnRequestJoin(username);

		UNLOCK(M_ALLIANCELIST);
		UNLOCK(M_CLIENTLIST);

		gserver->SendObject(client, obj2);

		alliance->SaveToDB();

		return;
	}
	if ((command == "cancelagreeComeinAlliance"))
	{
		obj2["cmd"] = "alliance.cancelagreeComeinAlliance";
		data2["packageId"] = 0.0f;

		if (!client->HasAlliance())
		{
			gserver->SendObject(client, gserver->CreateError("alliance.cancelagreeComeinAlliance", -99, "Not a member of an alliance."));
			return;
		}

		MULTILOCK(M_ALLIANCELIST, M_CLIENTLIST);
		string username = data["userName"];

		data2["ok"] = 1;

		Alliance * alliance = client->GetAlliance();

		alliance->UnRequestJoin(username);

		UNLOCK(M_ALLIANCELIST);
		UNLOCK(M_CLIENTLIST);

		gserver->SendObject(client, obj2);

		alliance->SaveToDB();

		return;
	}
	if ((command == "setAllianceFriendship"))
	{
		//TODO: permission check
		obj2["cmd"] = "alliance.setAllianceFriendship";
		data2["packageId"] = 0.0f;

		if (!client->HasAlliance())
		{
			gserver->SendObject(client, gserver->CreateError("alliance.setAllianceFriendship", -99, "Not a member of an alliance."));
			return;
		}

		//1 friendly
		//2 neutral
		//3 enemy

		int alliancetype = data["type"];
		string otheralliancename = data["targetAllianceName"];

		Alliance * otheralliance = gserver->m_alliances->AllianceByName(otheralliancename);
		if (otheralliance == (Alliance*)-1)
		{
			//doesn't exist
			gserver->SendObject(client, gserver->CreateError("alliance.setAllianceFriendship", -99, "Alliance does not exist."));
			return;
		}

		Alliance * alliance = client->GetAlliance();

		if (alliancetype == 1)
		{
			if (alliance->IsAlly(otheralliance->m_allianceid))
			{
				//already allied
				gserver->SendObject(client, gserver->CreateError("alliance.setAllianceFriendship", -99, "Alliance is already an ally."));
				return;
			}
			alliance->Ally(otheralliance->m_allianceid);
		}
		else if (alliancetype == 2)
		{
			if (alliance->IsNeutral(otheralliance->m_allianceid))
			{
				//already neutral
				gserver->SendObject(client, gserver->CreateError("alliance.setAllianceFriendship", -99, "Alliance is already neutral."));
				return;
			}
			alliance->Neutral(otheralliance->m_allianceid);
		}
		else //alliancetype = 3
		{
			if (alliance->IsEnemy(otheralliance->m_allianceid))
			{
				//already enemy
				gserver->SendObject(client, gserver->CreateError("alliance.setAllianceFriendship", -99, "Alliance is already an enemy."));
				return;
			}
			// TODO copy declare war cooldown?
			if (unixtime() < alliance->enemyactioncooldown)
			{
				//Declared war too soon
				gserver->SendObject(client, gserver->CreateError("alliance.setAllianceFriendship", -99, "You have already declared war recently."));
				return;
			}

			alliance->Enemy(otheralliance->m_allianceid);
		}
		data2["ok"] = 1;
		gserver->SendObject(client, obj2);

		alliance->SaveToDB();
		return;
	}
	if ((command == "getAllianceFriendshipList"))
	{
		// TODO War reports -- alliance.getMilitarySituationList
		obj2["cmd"] = "alliance.getAllianceFriendshipList";
		data2["packageId"] = 0.0f;

		if (!client->HasAlliance())
		{
			gserver->SendObject(client, gserver->CreateError("alliance.getAllianceFriendshipList", -99, "Not a member of an alliance."));
			return;
		}

		data2["ok"] = 1;

		LOCK(M_ALLIANCELIST);
		Alliance * tempalliance = (Alliance*)-1;
		Alliance * alliance = client->GetAlliance();

		amf3array middleList = amf3array();
		amf3array enemyList = amf3array();
		amf3array friendlyList = amf3array();

		std::vector<int64_t>::iterator iter;
		if (alliance->m_neutral.size() > 0)
		{
			for (iter = alliance->m_neutral.begin(); iter != alliance->m_neutral.end(); ++iter)
			{
				tempalliance = client->m_main->m_alliances->AllianceById(*iter);
				if (tempalliance != (Alliance*)-1)
				{
					amf3object ta = amf3object();
					ta["rank"] = tempalliance->m_prestigerank;
					ta["honor"] = tempalliance->m_honor;// HACK might be 0 -- alliance.isHasAlliance
					ta["allianceName"] = tempalliance->m_name.c_str();
					ta["memberCount"] = tempalliance->m_currentmembers;
					ta["aPrestigeCount"] = tempalliance->m_prestige;// HACK might be 0 -- alliance.isHasAlliance
					middleList.Add(ta);
				}
			}
		}
		if (alliance->m_enemies.size() > 0)
		{
			for (iter = alliance->m_enemies.begin(); iter != alliance->m_enemies.end(); ++iter)
			{
				tempalliance = client->m_main->m_alliances->AllianceById(*iter);
				if (tempalliance != (Alliance*)-1)
				{
					amf3object ta = amf3object();
					ta["rank"] = tempalliance->m_prestigerank;
					ta["honor"] = tempalliance->m_honor;// HACK might be 0 -- alliance.isHasAlliance
					ta["allianceName"] = tempalliance->m_name.c_str();
					ta["memberCount"] = tempalliance->m_currentmembers;
					ta["aPrestigeCount"] = tempalliance->m_prestige;// HACK might be 0 -- alliance.isHasAlliance
					enemyList.Add(ta);
				}
			}
		}
		if (alliance->m_allies.size() > 0)
		{
			for (iter = alliance->m_allies.begin(); iter != alliance->m_allies.end(); ++iter)
			{
				tempalliance = client->m_main->m_alliances->AllianceById(*iter);
				if (tempalliance != (Alliance*)-1)
				{
					amf3object ta = amf3object();
					ta["rank"] = tempalliance->m_prestigerank;
					ta["honor"] = tempalliance->m_honor;// HACK might be 0 -- alliance.isHasAlliance
					ta["allianceName"] = tempalliance->m_name.c_str();
					ta["memberCount"] = tempalliance->m_currentmembers;
					ta["aPrestigeCount"] = tempalliance->m_prestige;// HACK might be 0 -- alliance.isHasAlliance
					friendlyList.Add(ta);
				}
			}
		}
		data2["middleList"] = middleList;
		data2["enemyList"] = enemyList;
		data2["friendlyList"] = friendlyList;

		UNLOCK(M_ALLIANCELIST);
		gserver->SendObject(client, obj2);
		return;
	}
	if ((command == "createAlliance"))
	{
		VERIFYCASTLEID();
		CHECKCASTLEID();

		obj2["cmd"] = "alliance.createAlliance";
		data2["packageId"] = 0.0f;

		string alliancename = data["allianceName"];

		if (city->m_resources.gold < 10000)
		{
			gserver->SendObject(client, gserver->CreateError("alliance.createAlliance", -99, "Not enough gold."));
			return;
		}

		if (!gserver->m_alliances->CheckName(alliancename) || alliancename.size() < 2)
		{
			gserver->SendObject(client, gserver->CreateError("alliance.createAlliance", -99, "Illegal naming, please choose another name."));
			return;
		}

		LOCK(M_ALLIANCELIST);
		gserver->mtxlist.ranklist.lock_shared();
		if (gserver->m_alliances->AllianceByName(alliancename) != (Alliance*)-1)
		{
			string error = "Alliance already existed: ";
			error += alliancename;
			error += ".";

			gserver->SendObject(client, gserver->CreateError("alliance.createAlliance", -12, error));
			gserver->mtxlist.ranklist.unlock_shared();
			UNLOCK(M_ALLIANCELIST);
			return;
		}
		gserver->mtxlist.ranklist.unlock_shared();
		Alliance * alliance = 0;
		gserver->mtxlist.ranklist.lock();
		if (alliance = gserver->m_alliances->CreateAlliance(alliancename, client->m_accountid))
		{
			data2["ok"] = 1;

			string error = "Establish alliance ";
			error += alliancename;
			error += " successfully.";
			data2["msg"] = error;

			alliance->m_founder = client->m_playername;

			city->m_resources.gold -= 10000;

			if (!gserver->m_alliances->JoinAlliance(alliance->m_allianceid, client))
			{
				client->PlayerUpdate();
				city->ResourceUpdate();

				gserver->SendObject(client, gserver->CreateError("alliance.createAlliance", -99, "Alliance created but cannot join. Please contact support."));
				gserver->mtxlist.ranklist.unlock();
				UNLOCK(M_ALLIANCELIST);
				return;
			}
			if (!gserver->m_alliances->SetRank(alliance->m_allianceid, client, DEF_ALLIANCEHOST))
			{
				client->PlayerUpdate();
				city->ResourceUpdate();

				gserver->SendObject(client, gserver->CreateError("alliance.createAlliance", -99, "Alliance created but cannot set rank to host. Please contact support."));
				gserver->mtxlist.ranklist.unlock();
				UNLOCK(M_ALLIANCELIST);
				return;
			}

			client->PlayerUpdate();
			city->ResourceUpdate();



			gserver->m_alliances->SortAlliances();

			gserver->SendObject(client, obj2);
			gserver->mtxlist.ranklist.unlock();
			UNLOCK(M_ALLIANCELIST);
			client->SaveToDB();
			alliance->SaveToDB();
			return;
		}
		else
		{
			gserver->SendObject(client, gserver->CreateError("alliance.createAlliance", -99, "Cannot create alliance. Please contact support."));
			UNLOCK(M_ALLIANCELIST);
			return;
		}
	}
	if ((command == "setAllInfoForAlliance"))
	{
		//TODO: permission check
		string notetext = data["noteText"];
		string infotext = data["infoText"];
		string alliancename = data["allianceName"];

		obj2["cmd"] = "alliance.setAllInfoForAlliance";
		data2["packageId"] = 0.0f;

		if (!client->HasAlliance())
		{
			gserver->SendObject(client, gserver->CreateError("alliance.setAllInfoForAlliance", -99, "Not a member of an alliance."));
			return;
		}
		LOCK(M_ALLIANCELIST);
		Alliance * alliance = client->GetAlliance();
		if (alliance->m_name != alliancename)
		{
			gserver->SendObject(client, gserver->CreateError("alliance.setAllInfoForAlliance", -99, "Error."));
			return;
		}

		alliance->m_note = makesafe(notetext);
		alliance->m_intro = makesafe(infotext);

		UNLOCK(M_ALLIANCELIST);

		data2["ok"] = 1;

		gserver->SendObject(client, obj2);

		alliance->SaveToDB();

		return;
	}
	if (command == "getPowerFromAlliance")
	{
		obj2["cmd"] = "alliance.getPowerFromAlliance";
		data2["packageId"] = 0.0f;

		if (!client->HasAlliance())
		{
			gserver->SendObject(client, gserver->CreateError("alliance.getPowerFromAlliance", -99, "Not a member of an alliance."));
			return;
		}
		data2["ok"] = 1;
		data2["level"] = client->m_alliancerank;

		gserver->SendObject(client, obj2);
		return;
	}
	if (command == "resetTopPowerForAlliance")
	{
		obj2["cmd"] = "alliance.resetTopPowerForAlliance";
		data2["packageId"] = 0.0f;

		if (!client->HasAlliance())
		{
			gserver->SendObject(client, gserver->CreateError("alliance.resetTopPowerForAlliance", -99, "Not a member of an alliance."));
			return;
		}

		if (client->m_alliancerank != DEF_ALLIANCEHOST)
		{
			//you're not the host.. what are you doing?
			gserver->SendObject(client, gserver->CreateError("alliance.resetTopPowerForAlliance", -42, "You are not entitled to operate."));
			return;
		}

		string passtoname = data["userName"];

		Alliance * alliance = client->GetAlliance();
		if (alliance->HasMember(passtoname))
		{
			//member found
			Client * tclient = gserver->GetClientByName(passtoname);
			if (tclient->m_alliancerank != DEF_ALLIANCEVICEHOST)
			{
				gserver->SendObject(client, gserver->CreateError("alliance.resetTopPowerForAlliance", -88, "The Host title of the Alliance can only be transferred to Vice Host. You need to promote this player first."));
				return;
			}
			//everything checks out. target is vice host and you are host
			tclient->m_alliancerank = DEF_ALLIANCEHOST;
			client->m_alliancerank = DEF_ALLIANCEVICEHOST;
			client->PlayerUpdate();
			tclient->PlayerUpdate();
			alliance->m_owner = tclient->m_playername;

			data2["ok"] = 1;

			gserver->SendObject(client, obj2);

			alliance->SaveToDB();
			client->SaveToDB();

			return;
		}
		else
		{
			gserver->SendObject(client, gserver->CreateError("alliance.resetTopPowerForAlliance", -41, "Player " + passtoname + " doesn't exist."));
			return;
		}
	}
	if ((command == "agreeComeinAllianceByUser"))//TODO: alliance invites player, player accepts via embassy
	{
		gserver->SendObject(client, gserver->CreateError("alliance.agreeComeinAllianceByUser", -99, "Not a member of an alliance."));

		//			data["castleId"];
		//			data["allianceName"];

		gserver->SendObject(client, obj2);

		//alliance->SaveToDB();
		//client->SaveToDB();

		return;
	}
	if ((command == "agreeComeinAllianceByLeader"))//player applies to alliance, leader accepts
	{
		//TODO: permission check?
		obj2["cmd"] = "alliance.agreeComeinAllianceByLeader";
		data2["packageId"] = 0.0f;

		if (!client->HasAlliance())
		{
			gserver->SendObject(client, gserver->CreateError("alliance.agreeComeinAllianceByLeader", -99, "Not a member of an alliance."));
			return;
		}

		MULTILOCK(M_ALLIANCELIST, M_CLIENTLIST);
		string username = data["userName"];
		Client * invitee = gserver->GetClientByName(username);

		if (invitee == 0)
		{
			gserver->SendObject(client, gserver->CreateError("alliance.agreeComeinAllianceByLeader", -41, "Player " + username + " doesn't exist."));
			UNLOCK(M_ALLIANCELIST);
			UNLOCK(M_CLIENTLIST);
			return;
		}
		if (invitee->HasAlliance())
		{
			gserver->SendObject(client, gserver->CreateError("alliance.agreeComeinAllianceByLeader", -99, username + " is already a member of other alliance."));
			UNLOCK(M_ALLIANCELIST);
			UNLOCK(M_CLIENTLIST);
			return;
		}
		data2["ok"] = 1;

		Alliance * alliance = client->GetAlliance();

		alliance->UnRequestJoin(username);
		gserver->m_alliances->JoinAlliance(alliance->m_allianceid, invitee);

		UNLOCK(M_ALLIANCELIST);
		UNLOCK(M_CLIENTLIST);

		gserver->SendObject(client, obj2);

		alliance->SaveToDB();
		invitee->SaveToDB();

		return;
	}
	if ((command == "setPowerForUserByAlliance"))
	{
		obj2["cmd"] = "alliance.setPowerForUserByAlliance";
		data2["packageId"] = 0.0f;
		data2["ok"] = 1;

		int8_t type = data["typeId"];
		string username = data["userName"];

		if (!client->HasAlliance())
		{
			gserver->SendObject(client, gserver->CreateError("alliance.setPowerForUserByAlliance", -99, "Not a member of an alliance."));
			return;
		}

		Alliance * alliance = client->GetAlliance();

		MULTILOCK(M_ALLIANCELIST, M_CLIENTLIST);
		Client * tar = gserver->GetClientByName(username);

		if (!tar || !alliance->HasMember(username))
		{
			gserver->SendObject(client, gserver->CreateError("alliance.setPowerForUserByAlliance", -99, "Member does not exist."));
			UNLOCK(M_ALLIANCELIST);
			UNLOCK(M_CLIENTLIST);
			return;
		}
		//TODO: Set limits to rank counts? (aka, only x amount of vice hosts, etc)
		gserver->m_alliances->SetRank(client->m_allianceid, tar, type);

		UNLOCK(M_ALLIANCELIST);
		UNLOCK(M_CLIENTLIST);

		gserver->SendObject(client, obj2);

		//send alliance message
		string msg = client->m_playername + " promotes " + tar->m_playername + " to " + AllianceCore::GetAllianceRank(type) + ".";
		alliance->SendAllianceMessage(msg, false, false);

		alliance->SaveToDB();
		client->SaveToDB();

		return;
	}
	if ((command == "kickOutMemberfromAlliance"))
	{
		//TODO: permission check
		obj2["cmd"] = "alliance.kickOutMemberfromAlliance";
		data2["packageId"] = 0.0f;

		if (!client->HasAlliance())
		{
			gserver->SendObject(client, gserver->CreateError("alliance.kickOutMemberfromAlliance", -99, "Not a member of an alliance."));
			return;
		}

		string username = data["userName"];

		Alliance * alliance = client->GetAlliance();

		MULTILOCK(M_ALLIANCELIST, M_CLIENTLIST);
		Client * tar = gserver->GetClientByName(username);

		if (!tar || !alliance->HasMember(username))
		{
			gserver->SendObject(client, gserver->CreateError("alliance.kickOutMemberfromAlliance", -99, "Member does not exist."));
			UNLOCK(M_ALLIANCELIST);
			UNLOCK(M_CLIENTLIST);
			return;
		}

		if (username == client->m_playername)
		{
			gserver->SendObject(client, gserver->CreateError("alliance.kickOutMemberfromAlliance", -99, "Cannot kick yourself."));
			UNLOCK(M_ALLIANCELIST);
			UNLOCK(M_CLIENTLIST);
			return;
		}

		if (client->m_alliancerank >= tar->m_alliancerank)
		{
			gserver->SendObject(client, gserver->CreateError("alliance.kickOutMemberfromAlliance", -99, "Cannot kick someone higher ranking than you."));
			UNLOCK(M_ALLIANCELIST);
			UNLOCK(M_CLIENTLIST);
			return;
		}

		if (!gserver->m_alliances->RemoveFromAlliance(client->m_allianceid, tar))
		{
			gserver->SendObject(client, gserver->CreateError("alliance.kickOutMemberfromAlliance", -99, "Could not kick out member."));
			UNLOCK(M_ALLIANCELIST);
			UNLOCK(M_CLIENTLIST);
			return;
		}

		UNLOCK(M_ALLIANCELIST);
		UNLOCK(M_CLIENTLIST);

		gserver->SendObject(client, obj2);

		alliance->SaveToDB();
		tar->SaveToDB();

		return;
	}
}



