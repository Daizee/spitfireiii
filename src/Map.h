//
// Map.h
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

#include "amf3.h"
#include "structs.h"

class Tile;
class Server;

class Map
{
public:
	Map(Server * sptr, uint32_t size);
	~Map(void);

	uint32_t mapsize;
	Server * m_main;
	string * states;

	void CalculateOpenTiles();
	int GetStateFromXY(int x, int y);
	int GetStateFromID(int id);
	int GetRandomOpenTile(int zone);
	amf3object GetTileRangeObject(int32_t clientid, int x1, int x2, int y1, int y2);
	amf3object GetMapCastle(int32_t fieldid, int32_t clientid);
	Tile * GetTileFromID(int id);

	bool AddCity(int id, City * city);

	Tile * m_tile;
	int32_t m_totalflats[DEF_STATES];
	int32_t m_openflats[DEF_STATES];
	int32_t m_npcs[DEF_STATES];
	int32_t m_cities[DEF_STATES];
	int32_t m_occupiedtiles[DEF_STATES];
	int32_t m_occupiabletiles[DEF_STATES];
	struct mapstats
	{
		int players;
		int numbercities;
		int playerrate;
	} m_stats[DEF_STATES];

	std::vector<int32_t> m_openflatlist[DEF_STATES];
};

