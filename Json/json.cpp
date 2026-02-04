#include "stdafx.h"

#include "json.h"

PSTR SkipWS(PSTR pa, PSTR pb)
{
	if (pa < pb)
	{
		do
		{
			switch (*pa++)
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
			switch (char c = *pa++)
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

	*pbuf = buf, * plen = len;

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
		*pbuf = buf, * plen = len;
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

	*pbuf = buf, * plen = len;

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

	*pbuf = buf, * plen = len;

	return s;
}
