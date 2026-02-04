#pragma once

struct JsonObject;
struct JsonArray;

struct JD
{
	ULONG64 yyy : 28;
	ULONG64 len : 04;
	ULONG64 xxx : 31;
	ULONG64 neg : 01;
};

struct JsonValue
{
	JsonValue* next = 0;
	PCSTR name;
	union {
		PCSTR pcstr;
		LONG64 iVal;
		JD fVal;
		bool bVal;
		JsonObject* pObj;
		JsonArray* pArr;
	};
	enum { v_str, v_int, v_float, v_bool, v_obj, v_arr, v_null } vt = v_null;

	PSTR DoParse(PSTR pa, PSTR pb, ULONG level = 64);
	void Dump(PCSTR prefix);
	int ToString(PSTR buf, int len, PSTR* pbuf, int* plen);

	~JsonValue();

	JsonValue(PCSTR name = 0) : name(name)
	{
	}

	JsonValue(PCSTR name, PCSTR pcstr) : name(name), pcstr(pcstr), vt(v_str)
	{
	}

	JsonValue(PCSTR name, LONG64 iVal) : name(name), iVal(iVal), vt(v_int)
	{
	}

	JsonValue(PCSTR name, JD fVal) : name(name), fVal(fVal), vt(v_float)
	{
	}

	JsonValue(PCSTR name, bool bVal) : name(name), bVal(bVal), vt(v_bool)
	{
	}

	JsonValue(PCSTR name, JsonObject* pObj) : name(name), pObj(pObj), vt(v_obj)
	{
	}

	JsonValue(PCSTR name, JsonArray* pArr) : name(name), pArr(pArr), vt(v_arr)
	{
	}
};

struct JsonObjectOrArray
{
	JsonValue* first = 0;
	JsonValue* last = 0;

	~JsonObjectOrArray();

	void insert(JsonValue* next)
	{
		if (next)
		{
			(last ? last->next : first) = next, last = next;
		}
	}
};

struct JsonObject : JsonObjectOrArray
{
	PSTR DoParse(PSTR pa, PSTR pb, ULONG level);
	int ToString(PSTR buf, int len, PSTR* pbuf, int* plen);
	JsonValue* operator[](PCSTR name);
};

struct JsonArray : JsonObjectOrArray
{
	PSTR DoParse(PSTR pa, PSTR pb, ULONG level);
	int ToString(PSTR buf, int len, PSTR* pbuf, int* plen);
};

