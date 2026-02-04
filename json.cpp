#include "stdafx.h"

_NT_BEGIN
#pragma code_seg(".json")

struct JsonObject; //pre-declaration
struct JsonArray;  //pre-declaration

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

struct JsonArray  : JsonObjectOrArray
{
	PSTR DoParse(PSTR pa, PSTR pb, ULONG level);
	int ToString(PSTR buf, int len, PSTR* pbuf, int* plen);
};

PSTR SkipWS(PSTR pa, PSTR pb)
{
	if (pa < pb) 
	{
		do 
		{
			switch(*pa++)
			{
			case '\n':
			case '\r':
			case '\t':
			case ' ':
				continue;
			}

			return pa - 1;

		} while (pa < pb);
	}

	return 0;
}

PSTR DoParseString(PSTR pa, PSTR pb)
{
	if (pa < pb) 
	{
		PSTR psz = pa;
		do 
		{
			switch(char c = *pa++)
			{
			case '\"':
				*psz = 0;
				return pa;
			case '/':
			case '\f':
			case '\b':
			case '\n':
			case '\r':
			case '\t':
				return 0;
			case '\\':
				switch (c = *pa++)
				{
				case '\"':
				case '\\':
				case '/':
					*psz++ = c;
					continue;
				case 'b':
					*psz++ = '\b';
					continue;
				case 'n':
					*psz++ = '\n';
					continue;
				case 'r':
					*psz++ = '\r';
					continue;
				case 'f':
					*psz++ = '\f';
					continue;
				case 't':
					*psz++ = '\t';
					continue;
				}
				return 0;
			default:
				*psz++ = c;
			}

		} while (pa < pb);
	}

	return 0;
}

PSTR JsonValue::DoParse(PSTR pa, PSTR pb, ULONG level)
{
	ULONG_PTR LowLimit, HighLimit;
	GetCurrentThreadStackLimits(&LowLimit, &HighLimit);
	if ((ULONG_PTR)_AddressOfReturnAddress() - LowLimit < 0x4000)
	{
		return 0;
	}

	if (pa = SkipWS(pa, pb))
	{
		bool bNegative = false;

		switch (*pa++)
		{
		case '{':
			if (JsonObject* p = new JsonObject)
			{
				if (pa = p->DoParse(pa, pb, level))
				{
					pObj = p;
					vt = JsonValue::v_obj;
					return pa;
				}
				delete p;
			}
			return 0;

		case '[':
			if (JsonArray* p = new JsonArray)
			{
				if (pa = p->DoParse(pa, pb, level))
				{
					pArr = p;
					vt = JsonValue::v_arr;
					return pa;
				}
				delete p;
			}
			return 0;
		
		case '\"':
			pcstr = pa;
			vt = JsonValue::v_str;
			if (pa = DoParseString(pa, pb))
			{
				return pa;
			}
			return 0;

		case 't':
			if ('r' == *pa++ &&
				'u' == *pa++ &&
				'e' == *pa++)
			{
				bVal = true;
				vt = JsonValue::v_bool;
				return pa;
			}
			return 0;

		case 'f':
			if ('a' == *pa++ &&
				'l' == *pa++ &&
				's' == *pa++ &&
				'e' == *pa++)
			{
				bVal = false;
				vt = JsonValue::v_bool;
				return pa;
			}
			return 0;

		case 'n':
			if ('u' == *pa++ &&
				'l' == *pa++ &&
				'l' == *pa++)
			{
				bVal = true;
				vt = JsonValue::v_null;
				return pa;
			}
			return 0;
		
		case '-':
			bNegative = true;
			pa++;
		default:
			pb = --pa;
			if (0 > (iVal = _strtoui64(pa, &pa, 10)) || pb == pa)
			{
				return 0;
			}
			switch (*pa)
			{
			case ' ':
			case '\t':
			case '\r':
			case '\n':
			case ',':
			case ']':
			case '}':
				vt = JsonValue::v_int;
				if (bNegative)
				{
					iVal = -iVal;
				}
				return pa;
			case '.':
				pb = ++pa;
				ULONG64 LowPart = _strtoui64(pa, &pa, 10);
				if (99999999 < LowPart || MAXLONG < iVal || pb == pa)
				{
					return 0;
				}
				switch (*pa)
				{
				case ' ':
				case '\t':
				case '\r':
				case '\n':
				case ',':
				case ']':
				case '}':
					vt = JsonValue::v_float;
					fVal.neg = bNegative;
					fVal.xxx = iVal;
					fVal.yyy = LowPart;
					fVal.len = pa - pb;
					return pa;
				}
				break;
			}

			break;
		}
	}

	return 0;
}

PSTR JsonObject::DoParse(PSTR pa, PSTR pb, ULONG level)
{
	if (!--level)
	{
		return 0;
	}

	bool bWaitComma = false;
	do 
	{
		if (pa = SkipWS(pa, pb))
		{
			switch (*pa++)
			{
			case '}':
				return pa;

			case '\"':
				if (bWaitComma)
				{
					return 0;
				}
				bWaitComma = true;
				if (JsonValue* pval = new JsonValue(pa))
				{
					if ((pa = DoParseString(pa, pb)) &&
						(pa = SkipWS(pa, pb)) && ':' == *pa++ &&
						(pa = pval->DoParse(pa, pb, level)))
					{
						insert(pval);
						continue;
					}
					delete pval;
				}
				return 0;

			case ',':
				if (!bWaitComma)
				{
					return 0;
				}
				bWaitComma = false;
				continue;
			}
		}
		else
		{
			return 0;
		}

	} while (pa < pb);

	return 0;
}

PSTR JsonArray::DoParse(PSTR pa, PSTR pb, ULONG level)
{
	if (!--level)
	{
		return 0;
	}

	bool bWaitComma = false;
	do 
	{
		if (pa = SkipWS(pa, pb))
		{
			switch (*pa++)
			{
			case ']':
				return pa;

			case ',':
				if (!bWaitComma)
				{
					return 0;
				}
				bWaitComma = false;
				continue;

			default:
				if (bWaitComma)
				{
					return 0;
				}
				bWaitComma = true;
				if (JsonValue* pval = new JsonValue)
				{
					if (pa = pval->DoParse(pa - 1, pb, level))
					{
						insert(pval);
						continue;
					}
					delete pval;
				}
				return 0;
			}
		}
		else
		{
			return 0;
		}

	} while (pa < pb);

	return 0;
}

JsonValue::~JsonValue()
{
	switch (vt)
	{
	case v_obj:
		if (pObj) delete pObj;
		break;
	case v_arr:
		if (pArr) delete pArr;
		break;
	}

	vt = v_null;
}

JsonObjectOrArray::~JsonObjectOrArray()
{
	if (JsonValue* next = first)
	{
		do 
		{
			JsonValue* cur = next;
			next = next->next;
			delete cur;
		} while (next);
	}
}

JsonValue* JsonObject::operator[](PCSTR name)
{
	if (JsonValue* next = first)
	{
		do 
		{
			if (!strcmp(next->name, name))
			{
				return next;
			}
		} while (next = next->next);
	}

	return 0;
}

int StringToString(PCSTR str, PSTR buf, int len, PSTR* pbuf, int* plen)
{
	int s = 1;
	if (len)
	{
		*buf++ = '\"', --len;
	}

	char c;
	do 
	{
		bool b = true;

		switch (c = *str++)
		{
		case '\"':
		case '\\':
		case '/':
			break;
		case '\b':
			c = 'b';
			break;
		case '\n':
			c = 'n';
			break;
		case '\r':
			c = 'r';
			break;
		case '\f':
			c = 'f';
			break;
		case '\t':
			c = 't';
			break;
		default:
			b = false;
		}

		if (b)
		{
			s++;
			if (len)
			{
				*buf++ = '\\', --len;
			}
		}
		s++;
		if (len)
		{
			*buf++ = c, --len;
		}

	} while (c);

	if (buf)
	{
		buf[-1] = '\"';
	}

	*pbuf = buf, *plen = len;

	return s;
}

int JsonValue::ToString(PSTR buf, int len, PSTR* pbuf, int* plen)
{
	int s;

	switch (vt)
	{
	case v_str:
		return StringToString(pcstr, buf, len, pbuf, plen);

	case v_int:
		s = _snprintf(buf, len, "%I64d", iVal);
		break;

	case v_obj:
		return pObj->ToString(buf, len, pbuf, plen);

	case v_arr:
		return pArr->ToString(buf, len, pbuf, plen);

	case v_bool:
		s = _snprintf(buf, len, bVal ? "true" : "false");
		break;

	case v_float:
		s = _snprintf(buf, len, "%c%u.%0*u", fVal.neg ? '-' : ' ', (ULONG)fVal.xxx, (ULONG)fVal.len, (ULONG)fVal.yyy);
		break;

	case v_null:
		s = _snprintf(buf, len, "null");
		break;

	default: return -1;
	}

	if (0 <= s)
	{
		if (len)
		{
			len -= s, buf += s;
		}
		*pbuf = buf, *plen = len;
	}
	return s;
}

int JsonArray::ToString(PSTR buf, int len, PSTR* pbuf, int* plen)
{
	int s = 1;
	if (len)
	{
		*buf++ = '[', --len;
	}

	if (JsonValue* next = first)
	{
		do 
		{
			int l = next->ToString(buf, len, &buf, &len);
			if (0 > l)
			{
				return l;
			}
			s += l + 1;
			if (len)
			{
				*buf++ = ',', --len;
			}
		} while (next = next->next);
	}
	else
	{
		++s;
		if (len)
		{
			buf++, --len;
		}
	}

	if (buf)
	{
		buf[-1] = ']';
	}

	*pbuf = buf, *plen = len;

	return s;
}

int JsonObject::ToString(PSTR buf, int len, PSTR* pbuf, int* plen)
{
	int s = 1;
	if (len)
	{
		*buf++ = '{', --len;
	}

	if (JsonValue* next = first)
	{
		do 
		{
			int l = StringToString(next->name, buf, len, &buf, &len);
			if (0 > l)
			{
				return l;
			}
			s += l + 1;
			if (len)
			{
				*buf++ = ':', --len;
			}
			l = next->ToString(buf, len, &buf, &len);
			if (0 > l)
			{
				return l;
			}
			s += l + 1;
			if (len)
			{
				*buf++ = ',', --len;
			}
		} while (next = next->next);
	}
	else
	{
		++s;
		if (len)
		{
			buf++, --len;
		}
	}

	if (buf)
	{
		buf[-1] = '}';
	}

	*pbuf = buf, *plen = len;

	return s;
}

//////////////////////////////////////////////////////////////////////////
// tests

void print(JsonValue* value, PSTR str = 0)
{
	int len = 0;
	PSTR buf = 0, psz = 0;
	while (0 < (len = value->ToString(psz, len, &psz, &len)))
	{
		if (buf)
		{
			DbgPrint("%.*hs\r\n", len, buf);

			if (str)
			{
				if (strcmp(str, buf))
				{
					__debugbreak();
				}
			}
			else
			{
				memcpy(str = (PSTR)alloca(len + 1), buf, len + 1);
				JsonValue val;
				if (val.DoParse(buf, psz))
				{
					print(&val, str);
				}
				else
				{
					__debugbreak();
				}
			}

			break;
		}

		psz = buf = (PSTR)alloca(++len);
		buf[len - 1] = 0;
	}
}

void print(JsonObject* pObj)
{
	JsonValue value(0, pObj);
	print(&value);
	value.pObj = 0;
}

HRESULT ReadFromFile(_In_ PCWSTR lpFileName, _Out_ PVOID* ppb, _Out_ ULONG* pcb, _In_ ULONG _cb = 0, _In_ ULONG cb_ = 0);

void jtest()
{
	if (JsonObject* pObj = new JsonObject)
	{
		if (JsonArray* pArr = new JsonArray)
		{
			pArr->insert(new JsonValue(0, "JavaScript"));
			pArr->insert(new JsonValue(0, "HTML"));
			
			if (JsonValue* value = new JsonValue("skills", pArr))
			{
				pObj->insert(value);
			}
			else
			{
				delete pArr;
			}
		}

		pObj->insert(new JsonValue("name\r\n\test\"/\\\"[]", -581LL));
		pObj->insert(new JsonValue("key", "Jane \"(())\" Doe"));

		print(pObj);

		if (JsonValue* value = (*pObj)["key"])
		{
			print(value);
		}

		delete pObj;
	}

	PVOID pb;
	ULONG cb;
	int i = 6;
	WCHAR buf[16];
	do 
	{
		if (0 < swprintf_s(buf, _countof(buf), L"%02u.json", i))
		{
			if (!ReadFromFile(buf, &pb, &cb, 0, 1))
			{
				if (cb)
				{
					reinterpret_cast<PSTR>(pb)[cb] = 0;
					PSTR pa = (PSTR)pb, end = pa + cb;
					do 
					{
						JsonValue value;
						if (pa = value.DoParse(pa, end))
						{
							print(&value);
						}
						else
						{
							break;
						}
					} while (pa < end);
				}
				LocalFree(pb);
			}
		}
	} while (--i);
}

_NT_END