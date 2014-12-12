//
// Tile.cpp
// Project Spitfire
//
// Copyright (c) 2013 Daizee (rensiadz at gmail dot com)
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
// 
#include "includes.h"
#include "Tile.h"
#include "Client.h"
#include "City.h"
#include "funcs.h"


Tile::Tile(void)
{
	m_city = 0;
	m_castleicon = 0;
	m_castleid = -1;
	m_id = -1;
	m_npc = false;
	m_ownerid = -1;
	m_powerlevel = -1;
	m_state = -1;
	m_type = FLAT;
	m_status = -1;
	m_zoneid = -1;
	m_level = -1;
	/*	x = y = -1;*/
}


Tile::~Tile(void)
{
}


amf3object Tile::ToObject()
{
	PlayerCity * city = (PlayerCity*)m_city;
	amf3object obj = amf3object();
	obj["id"] = m_id;
	obj["name"] = city->m_cityname;
	obj["npc"] = m_npc;
	obj["prestige"] = city->m_client->m_prestige;
	obj["honor"] = city->m_client->m_honor;
	obj["state"] = city->m_client->m_status;
	obj["userName"] = city->m_client->m_playername;
	obj["flag"] = city->m_client->m_flag;
	obj["allianceName"] = city->m_client->m_alliancename;
	return obj;
}

string Tile::GetName()
{
	//if (m_npc)
	switch (m_type)
	{
		case FOREST:
			return "Forest";
		case DESERT:
			return "Desert";
		case HILL:
			return "Hill";
		case SWAMP:
			return "Swamp";
		case GRASS:
			return "Grass";
		case LAKE:
			return "Lake";
		case FLAT:
			return "Flat";
		case CASTLE:
			return (m_city)?m_city->m_cityname:"Invalid City";
		case NPC:
			return "Barbarian's City";
		default:
			return "null";
	}
}

