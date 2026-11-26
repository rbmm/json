#include "stdafx.h"

_NT_BEGIN

#include "json.h"

unsigned __int64 strtoui(_In_z_ const char * _String, _Out_opt_ _Deref_post_z_ char ** _EndPtr)
{
	__try {
		return _strtoui64(_String, _EndPtr, 10);
	} __except(EXCEPTION_EXECUTE_HANDLER) {
	}

	*_EndPtr = (char*)MAXULONGLONG;
	return MAXULONGLONG;
}

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
			case '\f':
			case '\b':
			case '\n':
			case '\r':
			case '\t':
				return 0;
			case '\\':
				if (pa < pb)
				{
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
				}
				return 0;
			default:
				*psz++ = c;
			}

		} while (pa < pb);
	}

	return 0;
}

PSTR JNode::DoParse(PSTR pa, PSTR pb, ULONG level)
{
#ifdef _WIN32
	ULONG_PTR LowLimit, HighLimit;
	GetCurrentThreadStackLimits(&LowLimit, &HighLimit);
	if ((ULONG_PTR)_AddressOfReturnAddress() - LowLimit < 0x4000)
	{
		return 0;
	}
#endif

	if (pa = SkipWS(pa, pb))
	{
		bool bNegative = false;

		switch (*pa++)
		{
		case '{':
			vt = v_obj;
			return DoParseObject(pa, pb, level);

		case '[':
			vt = v_arr;
			return DoParseArray(pa, pb, level);

		case '\"':
			pcstr = pa;
			vt = v_str;
			if (pa = DoParseString(pa, pb))
			{
				return pa;
			}
			return 0;

		case 't':
			if (pa + 3 < pb &&
				'r' == *pa++ &&
				'u' == *pa++ &&
				'e' == *pa++)
			{
				bVal = true;
				vt = v_bool;
				return pa;
			}
			return 0;

		case 'f':
			if (pa + 4 < pb &&
				'a' == *pa++ &&
				'l' == *pa++ &&
				's' == *pa++ &&
				'e' == *pa++)
			{
				bVal = false;
				vt = v_bool;
				return pa;
			}
			return 0;

		case 'n':
			if (pa + 3 < pb &&
				'u' == *pa++ &&
				'l' == *pa++ &&
				'l' == *pa++)
			{
				bVal = true;
				vt = v_null;
				return pa;
			}
			return 0;

		case '-':
			bNegative = true;
			pa++;
		default:
			pb = --pa;
			if (0 > (iVal = strtoui(pa, &pa)) || pb == pa)
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
				vt = v_int;
				if (bNegative)
				{
					iVal = -iVal;
				}
				return pa;
			case '.':
				pb = ++pa;
				ULONG64 LowPart = strtoui(pa, &pa);
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
					vt = v_float;
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

PSTR JNode::DoParseObject(PSTR pa, PSTR pb, ULONG level)
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
				if (JNode* pval = new JNode(pa))
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

PSTR JNode::DoParseArray(PSTR pa, PSTR pb, ULONG level)
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
				if (JNode* pval = new JNode)
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

template<typename Type>
void DeleteIfNotInStack(Type* pv)
{
	if (pv)
	{
#ifdef _WIN32
		PNT_TIB tib = (PNT_TIB)NtCurrentTeb();
		if (pv < tib->StackLimit || tib->StackBase <= pv) delete pv;
#else
#error !!
#endif // _WIN32
	}
}

JNode::~JNode()
{
	switch (vt)
	{
	case v_obj:
	case v_arr:
		if (JNode* value = first)
		{
			do
			{
				JNode* cur = value;
				value = value->next;
				DeleteIfNotInStack(cur);
			} while (value);
		}
		break;
	}

	vt = v_null;
}

void JNode::insert(_In_ JNode* value)
{
	switch (vt)
	{
	case v_obj:
	case v_arr:
		break;
	default:
		__debugbreak();
		return;
	}
	JNode* Last = CONTAINING_RECORD(&first, JNode, next), *Next;

	while (Next = Last->next)
	{
		Last = Next;
	}

	Last->next = value;
}

void JNode::remove(_In_ JNode* value)
{
	switch (vt)
	{
	case v_obj:
	case v_arr:
		break;
	default:
		__debugbreak();
		return;
	}
	JNode* Prev = CONTAINING_RECORD(&first, JNode, next), *Next;

	do 
	{
		if (value == (Next = Prev->next))
		{
			Prev->next = value->next;
			DeleteIfNotInStack(value);
			return;
		}

	} while (Prev = Next);

	__debugbreak();
}

JNode* JNode::operator[](_In_ PCSTR Name)
{
	switch (vt)
	{
	case v_obj:
		break;
	default:
		__debugbreak();
		return 0;
	}

	if (JNode* value = first)
	{
		do
		{
			if (PCSTR name = value->name)
			{
				if (!strcmp(name, Name))
				{
					return value;
				}
			}
		} while (value = value->next);
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

int JNode::ToString(PSTR buf, int len, PSTR* pbuf, int* plen)
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
		return ObjectToString(buf, len, pbuf, plen);

	case v_arr:
		return ArrayToString(buf, len, pbuf, plen);

	case v_bool:
		s = _snprintf(buf, len, bVal ? "true" : "false");
		break;

	case v_float:
		s = _snprintf(buf, len, "-%u.%0*u" + !fVal.neg, (ULONG)fVal.xxx, (ULONG)fVal.len, (ULONG)fVal.yyy);
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

int JNode::ArrayToString(PSTR buf, int len, PSTR* pbuf, int* plen)
{
	int s = 1;
	if (len)
	{
		*buf++ = '[', --len;
	}

	if (JNode* value = first)
	{
		do
		{
			int l = value->ToString(buf, len, &buf, &len);
			if (0 > l)
			{
				return l;
			}
			s += l + 1;
			if (len)
			{
				*buf++ = ',', --len;
			}
		} while (value = value->next);
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

	if (len)
	{
		*buf = 0;
	}

	*pbuf = buf, *plen = len;

	return s;
}

int JNode::ObjectToString(PSTR buf, int len, PSTR* pbuf, int* plen)
{
	int s = 1;
	if (len)
	{
		*buf++ = '{', --len;
	}

	if (JNode* value = first)
	{
		do
		{
			int l = StringToString(value->name, buf, len, &buf, &len);
			if (0 > l)
			{
				return l;
			}
			s += l + 1;
			if (len)
			{
				*buf++ = ':', --len;
			}
			l = value->ToString(buf, len, &buf, &len);
			if (0 > l)
			{
				return l;
			}
			s += l + 1;
			if (len)
			{
				*buf++ = ',', --len;
			}
		} while (value = value->next);
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

	if (len)
	{
		*buf = 0;
	}

	*pbuf = buf, *plen = len;

	return s;
}

#ifdef _WIN32

#include "../inc/rtlframe.h"

void* JNode::operator new(size_t s)
{
	if (JHeapPtr* heap = RTL_FRAME<JHeapPtr>::get())
	{
		if (PVOID pv = heap->heap->Alloc(s))
		{
			return pv;
		}
	}
	
	return LocalAlloc(LMEM_FIXED, s);
}

void JNode::operator delete(void* pv)
{
	if (JHeapPtr* heap = RTL_FRAME<JHeapPtr>::get())
	{
		if (heap->heap->Free(pv))
		{
			return;
		}
	}

	LocalFree(pv);
}

#endif // _WIN32

_NT_END