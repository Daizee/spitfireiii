//
// amf3.cpp
// Project Spitfire
//
// Copyright (c) 2013 Daizee (rensiadz at gmail dot com)
//
// This file is part of Spitfire.
// 
// Spitfire is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Spitfire is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Spitfire.  If not, see <http://www.gnu.org/licenses/>.

#include "includes.h"
#include "funcs.h"

#include "amf3.h"

int amf3arraycreate3;
int amf3classdefcreate3;
int amf3encapscreate3;
int amf3objectcreate3;
int amf3reflistcreate3;

amf3classdef::amf3classdef(void)
{
	name = "";
	properties.clear();
	dynamic = true;
	externalizable = false;
	amf3classdefcreate3++;
}
amf3classdef::amf3classdef(string name, vector<string> & properties, bool dynamic, bool externalizable)
{
	amf3classdefcreate3++;
	this->name = name;
	this->properties = properties;
	this->dynamic = dynamic;
	this->externalizable = externalizable;
}
amf3classdef::amf3classdef(const amf3classdef & classdef)
{
	amf3classdefcreate3++;
	this->name = classdef.name;
	if (classdef.properties.size() > 0)
	{
		for (int i = 0; i < classdef.properties.size(); ++i)
		{
			this->properties.push_back(classdef.properties.at(i));
		}
	}
	this->dynamic = classdef.dynamic;
	this->externalizable = classdef.externalizable;
}
amf3classdef::~amf3classdef(void)
{
	amf3classdefcreate3--;
}

amf3object::amf3object() {
	amf3encapscreate3++;
	type = Null;
	memset(&_value, 0, sizeof(_value));
}

amf3object::amf3object(const amf3object &val) {
	amf3encapscreate3++;
	InternalCopy(val);
}

void amf3object::InternalCopy(const amf3object &val) {
	type = val.type;
	memset(&_value, 0, sizeof(_value));
	switch (val.type) {
		case Date:
		{
			date = val.date;
			break;
		}
		case String:
		{
			text = val.text;
			break;
		}
		case Array:
		{
			_array = val._array;
			break;
		}
		case Object:
		{
			_object = val._object;
			break;
		}
		default:
		{
			memcpy(&_value, &val._value, sizeof(_value));
			break;
		}
	}
}
void amf3object::InternalCopy(const amf3array &val) {
	type = Array;
	memset(&_value, 0, sizeof(_value));
	_array = shared_ptr<amf3array>(new amf3array(val));
	//_value._array = new amf3array3(val);
}

void amf3object::Reset(bool isUndefined) {
	switch (type) {
		case String:
		{
			text = "";
			break;
		}
		case Array:
		{
			_array.reset();
			break;
		}
		case Object:
		{
			_object.reset();
			break;
		}
		default:
		{
			break;
		}
	}
	if (isUndefined)
		type = Undefined;
	else
		type = Null;
	memset(&_value, 0, sizeof(_value));
}

amf3object::~amf3object(void)
{
	amf3encapscreate3--;

	Reset();
	return;
}

#define OPERATOR_DEF(ctype) \
	amf3object::operator ctype() {\
	switch (type) {\
	case Null:\
	case Undefined:\
{\
	return 0;\
}\
	case True:\
	case False:\
{\
	return (ctype) _value.booltest;\
}\
	case Integer:\
{\
	return (ctype) _value.integer;\
}\
	case Number:\
{\
	return (ctype) _value.number;\
}\
	default:\
{\
	return 0;\
}\
}\
}

OPERATOR_DEF(int8_t);
OPERATOR_DEF(int16_t);
OPERATOR_DEF(int32_t);
//OPERATOR_DEF(int);
OPERATOR_DEF(int64_t);
OPERATOR_DEF(uint8_t);
OPERATOR_DEF(uint16_t);
OPERATOR_DEF(uint32_t);
OPERATOR_DEF(uint64_t);
OPERATOR_DEF(double);

#define CONSTRUCTORINTEGER_DEF(ctype) \
	amf3object::amf3object(const ctype &val) {\
	amf3objectcreate3++;\
	type = Integer;\
	memset(&_value, 0, sizeof (_value));\
	_value.integer = val;\
}
#define CONSTRUCTORNUMBER_DEF(ctype) \
	amf3object::amf3object(const ctype &val) {\
	amf3objectcreate3++;\
	type = Number;\
	memset(&_value, 0, sizeof (_value));\
	_value.number = val;\
}

#define OPERATORSETINTEGER_DEF(ctype) \
	amf3object& amf3object::operator=(const ctype &val) {\
	Reset();\
	type = Integer;\
	_value.integer = val;\
	return *this;\
}
#define OPERATORSETNUMBER_DEF(ctype) \
	amf3object& amf3object::operator=(const ctype &val) {\
	Reset();\
	type = Number;\
	_value.number = val;\
	return *this;\
}

CONSTRUCTORINTEGER_DEF(int8_t);
CONSTRUCTORINTEGER_DEF(int16_t);
CONSTRUCTORINTEGER_DEF(int32_t);
CONSTRUCTORNUMBER_DEF(int64_t);
CONSTRUCTORINTEGER_DEF(uint8_t);
CONSTRUCTORINTEGER_DEF(uint16_t);
CONSTRUCTORINTEGER_DEF(uint32_t);
CONSTRUCTORNUMBER_DEF(uint64_t);
CONSTRUCTORNUMBER_DEF(double);

OPERATORSETINTEGER_DEF(int8_t);
OPERATORSETINTEGER_DEF(int16_t);
OPERATORSETINTEGER_DEF(int32_t);
OPERATORSETNUMBER_DEF(int64_t);
OPERATORSETINTEGER_DEF(uint8_t);
OPERATORSETINTEGER_DEF(uint16_t);
OPERATORSETINTEGER_DEF(uint32_t);
OPERATORSETNUMBER_DEF(uint64_t);
OPERATORSETNUMBER_DEF(double);

amf3object& amf3object::operator=(const bool &val) {
	Reset();
	if (val)
	{
		type = True;
		_value.booltest = true;
	}
	else
	{
		type = False;
		_value.booltest = false;
	}
	return *this;
}

amf3object& amf3object::operator=(const amf3object &val) {
	Reset();
	InternalCopy(val);
	return *this;
}

amf3object& amf3object::operator=(const amf3array &val) {
	Reset();
	InternalCopy(val);
	return *this;
}

amf3object::amf3object(const bool &val) {
	amf3objectcreate3++;
	memset(&_value, 0, sizeof(_value));
	if (val)
	{
		type = True;
		_value.booltest = true;
	}
	else
	{
		type = False;
		_value.booltest = false;
	}
}

amf3object& amf3object::operator=(const char *pVal) {
	Reset();
	type = String;
	this->text = pVal;
	return *this;
}

amf3object& amf3object::operator=(const string &val) {
	Reset();
	type = String;
	this->text = val;
	return *this;
}

amf3object::operator Amf3TypeCode() {
	return type;
}

amf3object::operator bool() {
	switch (type) {
		case Null:
		case Undefined:
		{
			return false;
		}
		case True:
		case False:
		{
			return _value.booltest;
		}
		case Integer:
		case Number:
		{
			bool result = false;
			result |= (_value.integer != 0);
			result |= (_value.number != 0);
			return result;
		}
		default:
		{
			return false;
		}
	}
}
amf3object::operator string()
{
	switch (type) {
		case False:
		case True:
		{
			return string(_value.booltest ? "true" : "false");
		}
		case Integer:
		{
			stringstream ss; ss << this->operator int32_t();
			return ss.str();
		}
		case Number:
		{
			stringstream ss; ss << this->operator double();
			return ss.str();
		}
		case String:
		{
			return string(text.c_str());
		}
		case Null:
		case Undefined:
		case Object:
		case Array:
		default:
		{
			//ASSERT("Cast to string failed: %s", STR(ToString()));
			return "";
		}
	}
	return "";
}
/*
amf3object::operator char*()
{
switch (type) {
case False:
case True:
{
return (char *)(_value.booltest ? "true" : "false");
}
case Integer:
{
char * str;
asprintf(&str, "%d", this->operator int32_t()), temp.append(str), free(str);
return (char*)temp.c_str();
}
case Number:
{
char * str;
asprintf(&str, "%.3f", this->operator double()), temp.append(str), free(str);
return (char*)temp.c_str();
}
case String:
{
return (char*)text.c_str();
}
case Null:
case Undefined:
case Object:
case Array:
default:
{
//ASSERT("Cast to string failed: %s", STR(ToString()));
return "";
}
}
return "";
}
amf3object::operator const char*()
{
return operator char*();
}*/
amf3object::operator amf3array*()
{
	switch (type) {
		case False:
		case True:
		case Integer:
		case Number:
		case String:
		case Null:
		case Undefined:
		case Object:
			return 0;
		case Array:
			return _array.get();
		default:
			return 0;
	}
	return 0;
}

amf3object::operator amf3array&()
{
	switch (type) {
		case False:
		case True:
		case Integer:
		case Number:
		case String:
		case Null:
		case Undefined:
		case Object:
		default:
			//really reset? or do a throw?
			this->Reset();
			_array = shared_ptr<amf3array>(new amf3array());
			return *(_array.get());
		case Array:
			return *(_array.get());
	}
}

amf3object& amf3object::operator[](const char *key) {
	if ((type != Object) &&
		(type != Array) &&
		(type != Null) &&
		(type != Undefined)) {
		//ASSERT("Subscript operator applied on a incorrect Variant type: %s",
		//STR(ToString()));
	}
	if ((type == Null) || (type == Undefined)) {
		type = Object;
		_object = shared_ptr<amf3objectmap>(new amf3objectmap);
	}
	if (_object->Exists(string(key)) < 0)
	{
		amf3object tempobj;
		_object->Add(string(key), tempobj);
	}
	return (_object->Get(string(key)));
}

amf3object& amf3object::operator[](const double &key) {
	return operator[]((uint32_t)key);
}

amf3object& amf3object::operator[](const uint32_t &key) {
	if ((type != Object) &&
		(type != Array) &&
		(type != Null) &&
		(type != Undefined)) {
		//ASSERT("Subscript operator applied on a incorrect Variant type: %s",
		//STR(ToString()));
	}
	if ((type == Null) || (type == Undefined)) {
		throw "Object not an array yet accessed like one";

		//type = Array;
		//_value._array = new amf3array3;
	}
	if (!_array->Get(key))
	{
		throw "Index out of array bounds";
		//amf3object3 temp = amf3object3();
		//_value._array->Add(temp);
	}
	return (_array->Get(key));
}

amf3object& amf3object::operator[](const int &key) {
	return operator[]((uint32_t)key);
}

bool amf3object::operator==(amf3object variant) {
	if (type == Integer)
	{
		return this->_value.integer == variant._value.integer;
	}
	if (type == Number)
	{
		return this->_value.number == variant._value.number;
	}
	if (type == String)
	{
		return this->text == variant.text;
	}
	if (type == True)
	{
		if (variant.type == True)
			return true;
	}
	if (type == False)
	{
		if (variant.type == False)
			return true;
	}
	return false;
	// TODO: Finish (add Object and Array cases? Or leave as always not equal?)
}

bool amf3object::operator==(const char * str) {
	if (type == Integer)
	{
		return false;
	}
	if (type == Number)
	{
		return false;
	}
	if (type == String)
	{
		return this->text == str;
	}
	if (type == True)
	{
		return false;
	}
	if (type == False)
	{
		return false;
	}
	return false;
	// TODO: Finish (add Object and Array cases? Or leave as always not equal?)
}
bool amf3object::operator==(char * str) {
	return operator==((const char *)str);
}
bool amf3object::operator==(string str) {
	return operator==((char*)str.c_str());
}

bool amf3object::operator!=(amf3object variant) {
	return !operator==(variant);
}

bool amf3object::operator==(Amf3TypeCode type) {
	if ((type == Integer) || (type == Number))
		return type == Integer ||
		type == Number;
	else
		return this->type == type;
}

bool amf3object::operator!=(Amf3TypeCode type) {
	return !operator ==(type);
}

amf3objectmap::~amf3objectmap(void)
{
	amf3objectcreate3--;
}

amf3object & amf3objectmap::Get(string key)
{
	return properties.GetObj(key);
}
void amf3objectmap::Add(string key, amf3object & obj)
{
	properties.AddObj(key, obj);
}
int amf3objectmap::Exists(string key)
{
	return properties.Exists(key);
}

amf3array::amf3array(void)
{
	amf3arraycreate3++;
	this->type = 1;
}


amf3array::~amf3array(void)
{
	amf3arraycreate3--;
}

void amf3array::Add(amf3object & obj)
{
	this->type = 1;
	dense.push_back(obj);
}


void amf3array::Add(string key, amf3object & obj)
{
	this->type = 2;
	associative.AddObj(key, obj);
	dense.push_back(obj);
}


amf3object & amf3array::Get(int id)
{
	return dense.at(id);
}


amf3object & amf3array::Get(string key)
{
	return associative.GetObj(key);
}


inline bool amf3object::IsEqual(amf3object & obj, amf3object & obj2)
{
	if (obj2.type != obj.type)
		return false;
	switch (obj2.type)
	{
		case 2:
			if (obj._value.booltest == false)
				return true;
			break;
		case 3:
			if (obj._value.booltest == true)
				return true;
			break;
		case 4:
			if (obj._value.integer == obj2._value.integer)
				return true;
			break;
		case 5:
			if (obj._value.number == obj2._value.number)
				return true;
			break;
		case 6:
			if (obj.text == obj2.text)
				return true;
			break;
		case 7:
			return false;
		case 8:
			if (obj.date == obj2.date)
				return true;
			break;
		case 9:
			if (obj2._array->IsEqual(obj._array.get()))
				return true;
			break;
		case 10:
			if (obj2._object->IsEqual(obj._object.get()))
				return true;
			break;
		case 11:
		case 12:
			return false;
	}
	return false;
}



amf3parser::amf3parser(char * stream)
{
	this->stream = stream;
	position = 0;
	//encapslist.properties.resize(3000);
	//encapslist.propnames.resize(3000);
}


amf3parser::~amf3parser(void)
{
	for (int i = 0; i < strlist.properties.size(); ++i)
	{
		// 		delete[] (char*)strlist.properties.at(i);
	}
	//internalstrlist.properties.clear();
	//internalstrlist.propnames.clear();

	//strlist.properties.clear();
	//strlist.propnames.clear();

	// 	int asdf = encapslist.properties.size();
	// 	vector<amf3object *>::const_iterator iter;
	// 	iter = encapslist.properties.begin();
	// 	for (unsigned int i = 0; i < asdf; ++iter, ++i)
	// 		delete (amf3object*)*iter;

	//	for (int i = 0; i < encapslist.properties.size(); ++i)
	//	{
	//		//if ((*(amf3encaps*)encapslist.properties.at(i)).type != Object && (*(amf3encaps*)encapslist.properties.at(i)).type != Array)
	//			delete (amf3encaps*)encapslist.properties.at(i);
	//	}
	//encapslist.properties.clear();
	//encapslist.propnames.clear();
	//	for (int i = 0; i < objectlist.properties.size(); ++i)
	//	{
	//		delete objectlist.properties.at(i);
	//	}
	//objectlist.properties.clear();
	//objectlist.propnames.clear();
	for (int i = 0; i < deflist.properties.size(); ++i)
	{
		//delete (amf3classdef*)deflist.properties.at(i);
	}
	//deflist.properties.clear();
	//deflist.propnames.clear();
}


amf3object amf3parser::ReadNextObject(void)
{
	int b = this->stream[position++];

	Amf3TypeCode type = (Amf3TypeCode)b;

	amf3object obj = amf3object();

	obj.type = type;

	char * pstr = 0;
	int len = 0;

	string str;

	switch (type)
	{
		case Undefined:
		case Null:
			return obj;
		case False:
			obj._value.booltest = false;
			return obj;
		case True:
			obj._value.booltest = true;
			return obj;
		case Integer:
			obj._value.integer = ReadInteger();
			return obj;
		case Number:
			obj._value.number = ReadNumber();
			return obj;
		case String:
			obj.text = ReadString();
			return obj;
		case Date:
			obj.date = ReadDate();
			return obj;
		case Array:
			obj._array = shared_ptr<amf3array>(ReadArray());
			return obj;
		case Object:
			obj._object = shared_ptr<amf3objectmap>(ReadAMF3Object());
			return obj;
			//default:
			//Log("Invalid object type (%d)", type);
			//delete obj;
	}
	return 0;

}

int amf3parser::ReadInteger(void)
{
	int64_t integer = 0;
	int seen = 0;
	int b = 0;

	for (;;)
	{
		b = (unsigned char)stream[position++];

		if (seen == 3)
		{
			integer = (integer << 8) | b;
			break;
		}

		integer = (integer << 7) | (b & 0x7f);

		if ((b & 0x80) == 0x80)
		{
			seen++;
		}
		else {
			break;
		}
	}

	if (integer > (0x7FFFFFFF >> 3))
		integer -= (1 << 29);

	return (int)integer;
}
double amf3parser::ReadNumber(void)
{
	int integer = 0;
	int seen = 0;
	int b = 0;

	unsigned char num[8];

	num[0] = stream[position++];
	num[1] = stream[position++];
	num[2] = stream[position++];
	num[3] = stream[position++];
	num[4] = stream[position++];
	num[5] = stream[position++];
	num[6] = stream[position++];
	num[7] = stream[position++];
	register int i = 0;
	register int j = 7;
	while (i<j)
	{
		a_swap(&num[i], &num[j]);
		i++, j--;
	}

	return *(double*)num;
}
string amf3parser::ReadString(void)
{
	int num = ReadInteger();

	int len = (num >> 1);
	if ((num & 1) == 0)
	{
		return strlist.GetObj(num >> 1);
	}

	string str;
	str.insert(0, stream + position, num >> 1);
	position += (num >> 1);
	if (str.length() > 0)
		strlist.AddObj(str);
	return str;
}

string amf3parser::ReadDate(void)
{
	int num = ReadInteger();

	if ((num & 1) == 0)
	{
		return 0;//reflist.Get(num >> 1);
	}

	double val = ReadNumber();
	//int len = (num >> 1);
	//char * string = new char[len+1];
	//memcpy(string, stream+position, (num>>1));
	//position += (num>>1);
	//reflist.Add(string);
	return "datetime";
}

amf3array * amf3parser::ReadArray(void)
{
	int num = ReadInteger();

	if ((num & 1) == 0)
	{
		return (amf3array*)objectlist.GetObj(num >> 1);
	}
	num >>= 1;

	//amf3encaps * encapsobj = new amf3encaps();
	//memset(encapsobj, 0, sizeof(amf3encaps));
	//encapsobj->type = Array;
	amf3array * amfarray = new amf3array();
	//encapsobj->_array = amfarray;
	//objectlist.AddObj(encapsobj);
	//encapslist.AddObj(encapsobj);

	string key = ReadString();
	if (key.length() != 0)
	{
		amf3object obj = ReadNextObject();
		amfarray->Add(key, obj);
		encapslist.AddObj(obj);

		key = ReadString();
	}

	while (num-- > 0)
	{
		// 		if (num == 125)
		// 			__debugbreak();
		amf3object obj = ReadNextObject();
		amfarray->Add(obj);
		encapslist.AddObj(obj);
	}
	return amfarray;
}
amf3objectmap * amf3parser::ReadAMF3Object(void)
{
	//Log("Reading new AMF3Object");
	int flags = (Flags)ReadInteger();

	if ((flags & Inline) == 0)
	{
		return objectlist.GetObj(((int)flags) >> 1)._object.get();
	}

	amf3classdef classdef;

	bool cdfound = false;


	//	amfreflist * properties = 0;
	//	amfreflist * propnames = 0;
	//	amfreflist * tempnames = 0;


	int members = 0;

	if ((flags & InlineClassDef) == 0)
	{
		classdef = deflist.GetObj((((int)flags) >> 2));
		cdfound = true;
	}
	else {
		bool externalizable = ((flags & Externalizable) != 0);
		bool dynamic = ((flags & Dynamic) != 0);
		string name = ReadString();

		if (externalizable && dynamic)
			throw "Serialized objects cannot be both dynamic and externalizable";

		// TODO FIX EXTERNALIZED CLASS CODE
		if (externalizable)
			throw "Externalized object. CATCH()";
		// TODO FIX IT

		vector<string> properties;
		//properties = new vector<char *>;
		//properties = new amfreflist(2);
		//propnames = new amfreflist();
		//tempnames = new amfreflist();


		members = ((int)flags) >> 4;

		for (int i = 0; i < members; i++)
		{
			properties.push_back(ReadString());
		}
		//tempnames = properties;
		classdef = amf3classdef(name, properties, dynamic, externalizable);
		deflist.AddObj(classdef);
	}

	//amf3encaps * encapsobj = new amf3encaps();
	//memset(encapsobj, 0, sizeof(amf3encaps));
	amf3objectmap * obj = new amf3objectmap(classdef);
	obj->selfdel = (cdfound) ? false : true;
	//encapsobj->type = Object;
	//encapsobj->_object = obj;
	//objectlist.AddObj(encapsobj);
	//encapslist.AddObj(encapsobj);
	obj->flags = flags;

	if (classdef.externalizable)
	{
		amf3object temp = ReadNextObject();
		obj->Add("inner", temp);
		return obj;
	}

	amf3object tempobject;
	string name;
	for (int i = 0; i < classdef.properties.size(); i++)
	{
		name = classdef.properties.at(i);
		tempobject = ReadNextObject();
		encapslist.AddObj(tempobject);
		obj->Add(name, tempobject);
	}
	amf3object temp;
	if (classdef.dynamic)
	{
		string key = ReadString();
		while (key.length() != 0)
		{
			temp = ReadNextObject();
			obj->Add(key, temp);
			encapslist.AddObj(temp);
			key = ReadString();
		}
	}
	return obj;

}



amf3writer::amf3writer(char * stream)
{
	if (stream == 0)
		throw "Stream error";
	this->stream = stream;
	this->position = 0;
}


amf3writer::~amf3writer(void)
{
}

void amf3writer::Write(Amf3TypeCode type)
{
	stream[position++] = type;
}

void amf3writer::Write(amf3object & obj)
{
	if (obj.type == Null)
	{
		Write(Null);
		return;
	}
	if (obj.type == True)
	{
		Write(True);
		return;
	}
	if (obj.type == False)
	{
		Write(False);
		return;
	}
	if (obj.type == Integer)
	{
		Write(obj._value.integer);
		return;
	}
	if (obj.type == Number)
	{
		Write(obj._value.number);
		return;
	}
	if (obj.type == String)
	{
		Write(obj.text);
		return;
	}
	if (obj.type == Date)
	{
		Write(Date);
		TypelessWrite(obj.date);
		return;
	}
	if (obj.type == Array)
	{
		Write(obj._array.get(), obj);
		return;
	}
	if (obj.type == Object)
	{
		Write(obj._object.get(), obj);
		return;
	}
	throw "Invalid object type";
}
void amf3writer::Write(short integer)
{
	Write((int)integer);
}
void amf3writer::Write(char integer)
{
	Write((int)integer);
}
void amf3writer::Write(unsigned short integer)
{
	Write((int)integer);
}
void amf3writer::Write(unsigned char integer)
{
	Write((int)integer);
}
void amf3writer::Write(unsigned int integer)
{
	Write((int)integer);
}
void amf3writer::Write(int integer)
{
	Write(Integer);
	TypelessWrite((int)integer);
}
void amf3writer::Write(double number)
{
	Write(Number);
	TypelessWrite(number);
}
void amf3writer::Write(string str)
{
	Write(String);
	TypelessWrite(str);
}
void amf3writer::Write(amf3array * _array, amf3object & obj)
{
	Write(Array);
	TypelessWrite(_array, obj);
}
void amf3writer::Write(amf3objectmap * _object, amf3object & obj)
{
	Write(Object);
	TypelessWrite(_object, obj);
}
void amf3writer::TypelessWrite(uint32_t integer)
{
	TypelessWrite(int(integer));
}
void amf3writer::TypelessWrite(int integer)
{
	integer &= ~(7 << 29);

	if ((integer & (0xFF << 21)) != 0)
	{
		stream[position++] = (uint8_t)(((integer >> 22) & 0x7f) | 0x80);
		stream[position++] = (uint8_t)(((integer >> 15) & 0x7f) | 0x80);
		stream[position++] = (uint8_t)(((integer >> 8) & 0x7f) | 0x80);
		stream[position++] = (uint8_t)(integer & 0xff);
		return;
	}

	bool force = false;

	if ((integer & (0x7f << 14)) != 0)
	{
		stream[position++] = (char)(((integer >> 14) & 0x7f) | 0x80);
		force = true;
	}

	if (force || (integer & (0x7f << 7)) != 0)
	{
		stream[position++] = (char)(((integer >> 7) & 0x7f) | 0x80);
	}
	stream[position++] = (char)(integer & 0x7f);
}
void amf3writer::TypelessWrite(double number)
{
	unsigned char num[8];

	*(double*)num = number;

	register int i = 0;
	register int j = 7;
	while (i<j)
	{
		a_swap(&num[i], &num[j]);
		i++, j--;
	}

	stream[position++] = num[0];
	stream[position++] = num[1];
	stream[position++] = num[2];
	stream[position++] = num[3];
	stream[position++] = num[4];
	stream[position++] = num[5];
	stream[position++] = num[6];
	stream[position++] = num[7];
}
void amf3writer::TypelessWrite(string str)
{
	if (str.length() == 0)
	{
		TypelessWrite(1);
		return;
	}


	map<int, string>::const_iterator iter;
	iter = stringTable.begin();
	for (int i = 0; i < stringTable.size(); ++iter, ++i)
	{
		if (iter->second == str)
		{
			TypelessWrite(i << 1);
			return;
		}
	}

	//Need UTF8 code here...
	TypelessWrite((int)(str.length() << 1 | 1));
	strcpy_s(stream + position, str.length() + 1, str.c_str());
	position += str.length();

	typedef pair <int32_t, string> Int_Pair;
	stringTable.insert(Int_Pair(int32_t(stringTable.size()), str));
	strlist.AddObj(str);
}
void amf3writer::WriteDictionary(amf3reflist<amf3object> * reflist)
{
	for (int i = 0; i < reflist->propnames.size(); ++i)
	{
		TypelessWrite(reflist->propnames.at(i));
		Write(reflist->properties.at(i));
	}
	TypelessWrite("");
}
void amf3writer::TypelessWrite(amf3array * _array, amf3object & obj)
{
	if (CheckObjectTable(obj))
		return;

	typedef pair <int32_t, amf3object> Int_Pair;
	objectTable.insert(Int_Pair(int32_t(objectTable.size()), obj));
	//encapslist.AddObj(obj);

	TypelessWrite(int32_t(obj._array->dense.size() << 1 | 1));

	//	if (obj._value._array->associative.propnames.size() > 0)
	{
		WriteDictionary(&obj._array->associative);
	}
	//	else
	{
		//TypelessWrite(1);
	}

	for (int i = 0; i < obj._array->dense.size(); ++i)
	{
		Write(obj._array->dense.at(i));
	}
}
void amf3writer::TypelessWrite(amf3objectmap * _object, amf3object & obj)
{
	if (CheckObjectTable(obj))
		return;

	bool found = false;
	map<int32_t, amf3classdef>::const_iterator iter;
	iter = classdefTable.begin();
	for (int32_t i = 0; i < classdefTable.size(); ++iter, ++i)
	{
		if ((iter->second).IsEqual(obj._object->classdef))
		{
			TypelessWrite((i << 2) | 1);
			found = true;
			if (obj._object->anoncd)
			{
				obj._object->selfdel = false;
				//delete obj._value._object->classdef;
				//obj._value._object->classdef = 0;
				obj._object->classdef = iter->second;
			}
			break;
		}
	}

	if (!found)
	{
		typedef pair <int32_t, amf3classdef> Int_Pair;
		classdefTable.insert(Int_Pair(int32_t(classdefTable.size()), obj._object->classdef));
		deflist.AddObj(obj._object->classdef);

		int flags = Inline | InlineClassDef;
		if (_object->classdef.externalizable)
			flags |= Externalizable;
		if (_object->classdef.dynamic)
			flags |= Dynamic;

		TypelessWrite(int32_t(flags | (_object->classdef.properties.size() << 4)));

		TypelessWrite(_object->classdef.name);

		for (int i = 0; i < _object->classdef.properties.size(); ++i)
		{
			TypelessWrite(_object->classdef.properties.at(i));
		}
	}

	typedef pair <int32_t, amf3object> Int_Pair;
	objectTable.insert(Int_Pair(int32_t(objectTable.size()), obj));
	//encapslist.AddObj(obj);

	if (_object->classdef.externalizable)
	{
		Write((amf3object&)_object->Get("inner"));
		return;
	}

	for (int i = 0; i < _object->classdef.properties.size(); ++i)
	{
		Write((amf3object&)_object->Get(_object->classdef.properties.at(i)));
	}

	bool booltest = false;
	if (_object->classdef.dynamic)
	{
		for (int i = 0; i < _object->properties.properties.size(); ++i)
		{
			/************************************************************************/
			/* This is for "true" flash applications that would know the difference */
			/************************************************************************/
			/*if (_object->classdef->properties)
			{
			booltest= false;
			for (int x = 0; x < _object->properties.propnames.size(); ++x)
			{
			if (!strcmp(_object->classdef->properties->at(i)))
			booltest = true;
			}
			if (booltest == false)
			{
			TypelessWrite((char*)_object->properties.propnames.at(i));
			Write(_object->Get(_object->properties.propnames.at(i)));
			}
			}
			else*/
			{
				if (_object->properties.propnames.at(i) == "friendArmys")
					int32_t i = 0;
				TypelessWrite(_object->properties.propnames.at(i));
				Write((amf3object&)_object->Get(_object->properties.propnames.at(i)));
			}
		}
		TypelessWrite("");
	}


}

