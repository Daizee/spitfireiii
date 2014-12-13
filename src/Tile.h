//
// Tile.h
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

#include <string>
#include "amf3.h"

class City;

class Tile
{
public:
	Tile(void);
	~Tile(void);


	amf3object ToObject();
	string GetName();

	City * m_city;
	char m_castleicon;
	int m_castleid;
	int m_id;
	bool m_npc;
	int m_ownerid;
	char m_powerlevel;
	char m_state;
	char m_type;
	char m_status;
	char m_zoneid;
	char m_level;
/*	short x, y;*/

	/*
	["canColonize"] Type: Boolean - Value: True
	["canDeclaredWar"] Type: Boolean - Value: False
	["canLoot"] Type: Boolean - Value: False
	["canOccupy"] Type: Boolean - Value: True
	["canScout"] Type: Boolean - Value: True
	["canSend"] Type: Boolean - Value: False
	["canTrans"] Type: Boolean - Value: False
	["castleIcon"] Type: Integer - Value: 0
	["castleId"] Type: Integer - Value: 0
	["colonialRelation"] Type: Integer - Value: 0
	["colonialStatus"] Type: Integer - Value: 0
	["declaredWarStartTime"] Type: Number - Value: 0.000000
	["declaredWarStatus"] Type: Integer - Value: 0
	["freshMan"] Type: Boolean - Value: False
	["furlough"] Type: Boolean - Value: False
	["honor"] Type: Integer - Value: 0
	["id"] Type: Integer - Value: 323903
	["name"] Type: String - Value: Barbarian's city
	["npc"] Type: Boolean - Value: True
	["ownerPlayerId"] Type: Integer - Value: 0
	["powerLevel"] Type: Integer - Value: 4
	["prestige"] Type: Integer - Value: 0
	["relation"] Type: Integer - Value: 0
	["state"] Type: Integer - Value: 1
	["type"] Type: Integer - Value: 12
	["zoneName"] Type: String - Value: CARINTHIA*/
};

