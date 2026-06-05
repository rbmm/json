#pragma once

#ifndef DbgPrint
EXTERN_C
ULONG
__cdecl
DbgPrint(
	_In_z_ _Printf_format_string_ PCSTR Format,
	...
);
#endif

//#define JDBG

#ifdef JDBG

#pragma message(__FILE__ "(" _CRT_STRINGIZE(__LINE__) "):" " !!! JDBG !!!")

inline PVOID par(PVOID pv, char c)
{
	DbgPrint("[%c%c] 0x%p\r\n", c, c, pv);
	return pv;
}
#endif // JDBG

#define _JSON_

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
	enum JVT { v_str, v_int, v_float, v_bool, v_obj, v_arr, v_null } vt = v_null;

	PSTR DoParse(PSTR pa, PSTR pb, ULONG level = 64);
	void Dump(PCSTR prefix);
	int ToString(PSTR buf, int len, PSTR* pbuf, int* plen);

	~JsonValue();

	JsonValue(PCSTR name = 0) : name(name), pcstr(0)
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

#ifdef JDBG
	void* operator new(size_t s)
	{
		return par(LocalAlloc(LMEM_FIXED, s), '+');
	}

	void operator delete(void* pv)
	{
		LocalFree(par(pv, '-'));
	}
#endif // JDBG
};

struct JsonObjectOrArray
{
	JsonValue* first = 0;
	JsonValue* last = CONTAINING_RECORD(&first, JsonValue, next);

	~JsonObjectOrArray();

	void insert(_In_ JsonValue* value)
	{
		if (value)
		{
			last->next = value, last = value;
		}
	}

	void remove(_In_ JsonValue* value);

#ifdef JDBG
	void* operator new(size_t s)
	{
		return par(LocalAlloc(LMEM_FIXED, s), '+');
	}

	void operator delete(void* pv)
	{
		LocalFree(par(pv, '-'));
	}
#endif // JDBG
};

struct JsonObject : JsonObjectOrArray
{
	PSTR DoParse(PSTR pa, PSTR pb, ULONG level);
	int ToString(PSTR buf, int len, PSTR* pbuf, int* plen);
	JsonValue* operator[](_In_ PCSTR name);

	template<typename Type>
	void SetValue(PCSTR name, Type arg)
	{
		insert(new JsonValue(name, arg));
	}
};

struct JsonArray : JsonObjectOrArray
{
	PSTR DoParse(PSTR pa, PSTR pb, ULONG level);
	int ToString(PSTR buf, int len, PSTR* pbuf, int* plen);

	template<typename Type>
	void push_back(Type arg)
	{
		insert(new JsonValue(0, arg));
	}
};

