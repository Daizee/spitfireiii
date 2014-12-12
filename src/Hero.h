//
// Hero.h
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
#include "funcs.h"
#include "structs.h"
#include "amf3.h"

class Client;

class Hero
{
public:
	Hero();
	~Hero();
	amf3object ToObject();

	double m_experience;
	Client * m_client;
	int64_t m_castleid;
	int64_t m_ownerid;
	uint64_t m_id;
	int32_t m_itemamount; //?
	int32_t m_itemid; //?
	int16_t m_level;
	string m_logourl;
	int8_t m_loyalty;
	int16_t m_basemanagement;
	int16_t m_basepower;
	int16_t m_basestratagem;
	int16_t m_management;
	int16_t m_managementadded;
	int16_t m_managementbuffadded;
	string m_name;
	int16_t m_power;
	int16_t m_poweradded;
	int16_t m_powerbuffadded;
	int16_t m_remainpoint;
	int8_t m_status;
	int16_t m_stratagem;
	int16_t m_stratagemadded;
	int16_t m_stratagembuffadded;
	double m_upgradeexp;
	stArmyMovement * movement;

	bool SaveToDB();
	bool InsertToDB();
	bool DeleteFromDB();

	inline int16_t GetManagement()
	{
		return m_management + m_managementadded + m_managementbuffadded;
	}
	inline int16_t GetPower()
	{
		return m_power + m_poweradded + m_powerbuffadded;
	}
	inline int16_t GetStratagem()
	{
		return m_stratagem + m_stratagemadded + m_stratagembuffadded;
	}
	//array of buffs
}; // 75 bytes
