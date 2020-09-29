#pragma once

// https://github.com/rbmm/print-buf/blob/master/wb.h
#include "wb.h"

class JSON_ELEMENT
{
public:
	enum Type { v_invalid, v_object, v_array, v_string };
private:
	JSON_ELEMENT* next;
	PCSTR name;

	union {
		JSON_ELEMENT* child;
		PCSTR str;
		ULONG num;
	};
	Type type;

	PSTR DoParseObject(PSTR pa, PSTR pb);
	PSTR DoParseArray(PSTR pa, PSTR pb);

public:
	PSTR DoParse(PSTR pa, PSTR pb);

	Type get_type() { return type; }
	PCSTR get_string() { return type == v_string ? str : 0; }
	PCSTR get_name() { return IS_INTRESOURCE(name) ? 0 : name; }
	int get_index() { return IS_INTRESOURCE(name) ? (ULONG)(ULONG_PTR)name : -1; }

	JSON_ELEMENT* operator[](PCSTR Name);
	JSON_ELEMENT* operator[](ULONG i);

	JSON_ELEMENT(PCSTR name) : name(name), next(0), child(0), type(v_invalid) {
		DbgPrint("%s<%p>\n", __FUNCTION__, this);
	}

	~JSON_ELEMENT();

	Wb& operator >>(Wb& stream);
	void Dump(PCSTR prefix);
};
