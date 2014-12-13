//
// phero.cpp
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

#include "phero.h"
#include "../Server.h"
#include "../Hero.h"
#include "../Client.h"
#include "../City.h"


phero::phero(Server * server, request & req, amf3object & obj)
	: packet(server, req, obj)
{

}

phero::~phero()
{

}

void phero::process()
{
	VERIFYCASTLEID();
	CHECKCASTLEID();

	if ((command == "awardGold"))
	{
		for (int i = 0; i < 10; ++i)
		{
			if ((city->m_heroes[i]) && (city->m_heroes[i]->m_id == (uint64_t)obj["heroId"]))
			{
				Hero * hero = city->m_heroes[i];
				uint32_t goldcost = hero->m_level * 100;

				if (city->m_resources.gold < goldcost)
				{
					obj2["cmd"] = "hero.awardGold";
					data2["ok"] = -99;
					data2["errorMsg"] = "Not enough gold";
					data2["packageId"] = 0.0f;

					gserver->SendObject(client, obj2);
					return;
				}

				hero->m_loyalty += 5;
				if (hero->m_loyalty > 100)
					hero->m_loyalty = 100;

				city->m_resources.gold -= goldcost;

				city->ResourceUpdate();
				city->HeroUpdate(hero, 2);

				obj2["cmd"] = "hero.awardGold";
				data2["ok"] = 1;
				data2["packageId"] = 0.0f;

				gserver->SendObject(client, obj2);
				return;
			}
		}

		obj2["cmd"] = "hero.awardGold";
		data2["ok"] = -99;
		data2["errorMsg"] = "Invalid hero";
		data2["packageId"] = 0.0f;

		gserver->SendObject(client, obj2);
		return;
	}
	if (command == "changeName")
	{
		int64_t heroid = obj["heroId"];
		string newname = obj["newName"];

		Hero * hero = city->GetHero(heroid);

		if (!hero)
		{
			gserver->SendObject(client, gserver->CreateError("hero.changeName", -99, "Hero not found."));
			return;
		}

		hero->m_name = newname;

		city->HeroUpdate(hero, 2);

		obj2["cmd"] = "hero.changeName";
		data2["ok"] = 1;
		data2["packageId"] = 0.0f;
		gserver->SendObject(client, obj2);

		return;
	}
	if ((command == "getHerosListFromTavern"))
	{
		int innlevel = city->GetBuildingLevel(B_INN);

		if (innlevel > 0)
		{
			obj2["cmd"] = "hero.getHerosListFromTavern";
			data2["ok"] = 1;
			data2["packageId"] = 0.0f;
			int tempnum = 0;
			for (int i = 0; i < 10; ++i)
				if (city->m_heroes[i])
					tempnum++;
			data2["posCount"] = city->GetBuildingLevel(B_FEASTINGHALL) - tempnum;

			amf3array heroes = amf3array();

			for (int i = 0; i < innlevel; ++i)
			{
				if (!city->m_innheroes[i])
				{
					city->m_innheroes[i] = gserver->CreateRandomHero(innlevel);
				}

				amf3object temphero = city->m_innheroes[i]->ToObject();
				heroes.Add(temphero);
			}

			data2["heros"] = heroes;

			gserver->SendObject(client, obj2);
		}
		else
		{
			obj2["cmd"] = "hero.getHerosListFromTavern";
			data2["ok"] = -1; // TODO find error (no inn exists) - hero.getHerosListFromTavern
			data2["packageId"] = 0.0f;

			gserver->SendObject(client, obj2);
		}

		return;
	}
	if ((command == "hireHero"))
	{
		string heroname = data["heroName"];
		int blevel = city->GetBuildingLevel(B_FEASTINGHALL);

		if (blevel <= city->HeroCount())
		{
			gserver->SendObject(client, gserver->CreateError("hero.hireHero", -99, "Insufficient vacancies in Feasting Hall."));
			return;
		}

		for (int i = 0; i < 10; ++i)
		{
			if (city->m_innheroes[i]->m_name == heroname)
			{
				int32_t hirecost = city->m_innheroes[i]->m_level * 1000;
				if (hirecost > city->m_resources.gold)
				{
					// TODO Get proper not enough gold to hire error code - hero.hireHero
					gserver->SendObject(client, gserver->CreateError("hero.hireHero", -99, "Not enough gold!"));
					return;
				}

				for (int x = 0; x < 10; ++x)
				{
					if (!city->m_heroes[x])
					{
						city->m_resources.gold -= hirecost;
						city->CalculateResourceStats();
						city->CalculateStats();
						city->CalculateResources();
						city->m_innheroes[i]->m_id = gserver->m_heroid++;
						LOCK(M_HEROLIST);
						city->HeroUpdate(city->m_innheroes[i], 0);
						city->m_heroes[x] = city->m_innheroes[i];
						city->m_innheroes[i] = 0;
						UNLOCK(M_HEROLIST);
						city->ResourceUpdate();
						city->m_heroes[x]->m_client = client;
						city->m_heroes[x]->m_ownerid = client->m_accountid;
						city->m_heroes[x]->m_castleid = city->m_castleid;
						city->m_heroes[x]->InsertToDB();
						break;
					}
				}

				obj2["cmd"] = "hero.hireHero";
				data2["ok"] = 1;
				data2["packageId"] = 0.0f;

				gserver->SendObject(client, obj2);

				client->SaveToDB();
				city->SaveToDB();

				return;
			}
		}
		gserver->SendObject(client, gserver->CreateError("hero.hireHero", -99, "Hero does not exist!"));
		return;
	}
	if ((command == "fireHero"))
	{
		int heroid = data["heroId"];

		for (int i = 0; i < 10; ++i)
		{
			if (city->m_heroes[i] && city->m_heroes[i]->m_id == heroid)
			{
				if (city->m_heroes[i]->m_status != 0)
				{
					gserver->SendObject(client, gserver->CreateError("hero.fireHero", -80, "Status of this hero is not Idle!"));
					return;
				}
				city->CalculateResourceStats();
				city->CalculateStats();
				city->CalculateResources();
				city->ResourceUpdate();
				city->HeroUpdate(city->m_heroes[i], 1);

				MULTILOCK(M_HEROLIST, M_DELETELIST);
				gserver->m_deletedhero.push_back(city->m_heroes[i]->m_id);

				city->m_heroes[i]->DeleteFromDB();

				delete city->m_heroes[i];
				city->m_heroes[i] = 0;
				UNLOCK(M_HEROLIST);
				UNLOCK(M_DELETELIST);

				obj2["cmd"] = "hero.fireHero";
				data2["ok"] = 1;
				data2["packageId"] = 0.0f;

				gserver->SendObject(client, obj2);
				return;
			}
		}

		gserver->SendObject(client, gserver->CreateError("hero.fireHero", -99, "Hero does not exist!"));
		return;
	}
	if ((command == "promoteToChief"))
	{// BUG : Correct hero not being promoted
		int heroid = data["heroId"];

		for (int i = 0; i < 10; ++i)
		{
			if (city->m_heroes[i] && city->m_heroes[i]->m_id == heroid)
			{
				city->CalculateResourceStats();
				city->CalculateStats();
				city->CalculateResources();
				if (city->m_mayor)
					city->m_mayor->m_status = DEF_HEROIDLE;
				city->m_mayor = city->m_heroes[i];
				city->m_heroes[i]->m_status = DEF_HEROMAYOR;

				city->HeroUpdate(city->m_mayor, 2);
				city->CalculateResourceStats();
				city->CalculateResources();
				city->ResourceUpdate();

				obj2["cmd"] = "hero.promoteToChief";
				data2["ok"] = 1;
				data2["packageId"] = 0.0f;

				gserver->SendObject(client, obj2);

				city->SaveToDB();

				return;
			}
		}
		gserver->SendObject(client, gserver->CreateError("hero.promoteToChief", -99, "TODO error message - hero.promoteToChief"));
		return;
		// TODO needs an error message - hero.promoteToChief
	}
	if ((command == "dischargeChief"))
	{
		if (!city->m_mayor)
		{
			gserver->SendObject(client, gserver->CreateError("hero.dischargeChief", -99, "Castellan is not appointed yet."));
			return;
		}

		city->CalculateResourceStats();
		city->CalculateStats();
		city->CalculateResources();
		city->m_mayor->m_status = 0;
		city->HeroUpdate(city->m_mayor, 2);
		city->m_mayor = 0;
		city->CalculateResourceStats();
		city->CalculateResources();
		city->ResourceUpdate();

		obj2["cmd"] = "hero.dischargeChief";
		data2["ok"] = 1;
		data2["packageId"] = 0.0f;

		gserver->SendObject(client, obj2);

		city->SaveToDB();

		return;
	}
	if ((command == "resetPoint"))
	{
		int heroid = data["heroId"];

		// TODO require holy water? - hero.resetPoint

		for (int i = 0; i < 10; ++i)
		{
			if (city->m_heroes[i] && city->m_heroes[i]->m_id == heroid)
			{
				if (city->m_heroes[i]->m_status > 1)
				{
					gserver->SendObject(client, gserver->CreateError("hero.resetPoint", -80, "Status of this hero is not Idle!"));
					return;
				}
				city->CalculateResourceStats();
				city->CalculateStats();
				city->CalculateResources();
				city->ResourceUpdate();

				Hero * hero = city->m_heroes[i];
				hero->m_power = hero->m_basepower;
				hero->m_management = hero->m_basemanagement;
				hero->m_stratagem = hero->m_basestratagem;
				hero->m_remainpoint = hero->m_level;

				city->HeroUpdate(city->m_heroes[i], 2);
				city->CalculateResourceStats();
				city->CalculateResources();

				obj2["cmd"] = "hero.resetPoint";
				data2["ok"] = 1;
				data2["packageId"] = 0.0f;

				gserver->SendObject(client, obj2);

				city->m_heroes[i]->SaveToDB();

				return;
			}
		}

		gserver->SendObject(client, gserver->CreateError("hero.resetPoint", -99, "Hero does not exist!"));
		return;
	}
	if ((command == "addPoint"))
	{
		int heroid = data["heroId"];
		int stratagem = data["stratagem"];
		int power = data["power"];
		int management = data["management"];

		// TODO require holy water? - hero.resetPoint

		for (int i = 0; i < 10; ++i)
		{
			if (city->m_heroes[i] && city->m_heroes[i]->m_id == heroid)
			{
				city->CalculateResourceStats();
				city->CalculateStats();
				city->CalculateResources();
				city->ResourceUpdate();

				Hero * hero = city->m_heroes[i];
				if (((stratagem + power + management) >(hero->m_basemanagement + hero->m_basepower + hero->m_basestratagem + hero->m_level))
					|| (stratagem < hero->m_stratagem) || (power < hero->m_power) || (management < hero->m_management))
				{
					gserver->SendObject(client, gserver->CreateError("hero.addPoint", -99, "Invalid action."));
					return;
				}
				hero->m_power = power;
				hero->m_management = management;
				hero->m_stratagem = stratagem;
				hero->m_remainpoint = (hero->m_basemanagement + hero->m_basepower + hero->m_basestratagem + hero->m_level) - (stratagem + power + management);

				city->HeroUpdate(city->m_heroes[i], 2);
				city->CalculateResourceStats();
				city->CalculateResources();
				city->ResourceUpdate();

				obj2["cmd"] = "hero.resetPoint";
				data2["ok"] = 1;
				data2["packageId"] = 0.0f;

				gserver->SendObject(client, obj2);

				city->m_heroes[i]->SaveToDB();

				return;
			}
		}

		gserver->SendObject(client, gserver->CreateError("hero.addPoint", -99, "Hero does not exist!"));
		return;
	}
	if ((command == "levelUp"))
	{
		int heroid = data["heroId"];

		obj2["cmd"] = "hero.levelUp";
		data2["packageId"] = 0.0f;

		for (int i = 0; i < 10; ++i)
		{
			if (city->m_heroes[i] && city->m_heroes[i]->m_id == heroid)
			{
				Hero * hero = city->m_heroes[i];
				if (hero->m_experience < hero->m_upgradeexp)
				{
					gserver->SendObject(client, gserver->CreateError("hero.levelUp", -99, "Not enough experience."));
					return;
				}

				hero->m_level++;
				hero->m_remainpoint++;

				hero->m_experience -= hero->m_upgradeexp;
				hero->m_upgradeexp = hero->m_level * hero->m_level * 100;

				city->HeroUpdate(hero, 2);

				obj2["cmd"] = "hero.levelUp";
				data2["ok"] = 1;

				gserver->SendObject(client, obj2);

				city->m_heroes[i]->SaveToDB();

				return;
			}
		}

		gserver->SendObject(client, gserver->CreateError("hero.levelUp", -99, "Hero does not exist!"));
		return;
	}
	if ((command == "refreshHerosListFromTavern"))
	{
		int innlevel = city->GetBuildingLevel(B_INN);

		if (innlevel > 0)
		{
			obj2["cmd"] = "hero.getHerosListFromTavern";
			data2["ok"] = 1;
			data2["packageId"] = 0.0f;
			int tempnum = 0;
			for (int i = 0; i < 10; ++i)
				if (city->m_heroes[i])
					tempnum++;
			data2["posCount"] = city->GetBuildingLevel(B_FEASTINGHALL) - tempnum;

			amf3array heroes = amf3array();

			for (int i = 0; i < innlevel; ++i)
			{
				if (city->m_innheroes[i])
				{
					delete city->m_innheroes[i];
				}

				city->m_innheroes[i] = gserver->CreateRandomHero(innlevel);
				amf3object temphero = city->m_innheroes[i]->ToObject();
				heroes.Add(temphero);
			}

			data2["heros"] = heroes;

			gserver->SendObject(client, obj2);
		}
		else
		{
			// TODO find error (not enough cents) - hero.refreshHerosListFromTavern
			gserver->SendObject(client, gserver->CreateError("hero.refreshHerosListFromTavern", -99, "Not enough cents."));
		}

		return;
	}
}



