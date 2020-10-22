#include "stdafx.h"

_NT_BEGIN
#include "json++.h"

void JSON_ELEMENT::Dump(PCSTR prefix)
{
	if (*prefix == -1)
	{
		return;
	}

	char c = next ? ',' : ' ', a, b;

	switch (type)
	{
	case v_string:
		if (IS_INTRESOURCE(name))
		{
			DbgPrint("%s\"%s\"%c\n", prefix, str, c);
		}
		else
		{
			DbgPrint("%s\"%s\" : \"%s\"%c\n", prefix, name, str, c);
		}
		return;
	case v_object:
		a = '{', b = '}';
		break;
	case v_array:
		a = '[', b = ']';
		break;
	default: return;
	}
	
	if (IS_INTRESOURCE(name))
	{
		DbgPrint("%s%c\n", prefix, a);
	}
	else
	{
		DbgPrint("%s\"%s\" : %c\n", prefix, name, a);
	}

	--prefix;

	if (JSON_ELEMENT* value = child)
	{
		do 
		{
			value->Dump(prefix);

		} while (value = value->next);
	}

	++prefix;

	DbgPrint("%s%c%c\n", prefix, b, c);
}

Wb& JSON_ELEMENT::operator >>(Wb& stream)
{
	char a, b;
	PCSTR c = next ? "," : "";

	switch (type)
	{
	case v_string:
		if (IS_INTRESOURCE(name))
		{
			stream("\"%s\"%s", str, c);
		}
		else
		{
			stream("\"%s\":\"%s\"%s", name, str, c);
		}
		return stream;
	case v_object:
		a = '{', b = '}';
		break;
	case v_array:
		a = '[', b = ']';
		break;
	default: return stream;
	}

	if (IS_INTRESOURCE(name))
	{
		stream("%c", a);
	}
	else
	{
		stream("\"%s\":%c", name, a);
	}

	if (JSON_ELEMENT* value = child)
	{
		do 
		{
			(*value) >> stream;

		} while (value = value->next);
	}

	stream("%c%s", b, c);

	return stream;
}

JSON_ELEMENT* JSON_ELEMENT::operator[](PCSTR Name)
{
	if (type == v_object)
	{
		if (JSON_ELEMENT* cur = child)
		{
			do 
			{
				if (!_stricmp(cur->name, Name))
				{
					return cur;
				}
			} while (cur = cur->next);
		}
	}

	return 0;
}

JSON_ELEMENT* JSON_ELEMENT::operator[](ULONG i)
{
	if (type == v_array)
	{
		if (JSON_ELEMENT* cur = child)
		{
			do 
			{
				if (cur->name == (PCSTR)(ULONG_PTR)i)
				{
					return cur;
				}
			} while (cur = cur->next);
		}
	}

	return 0;
}

JSON_ELEMENT::~JSON_ELEMENT()
{
	DbgPrint("%s<%p>\n", __FUNCTION__, this);

	switch (type)
	{
	case v_object:
	case v_array:
		if (JSON_ELEMENT* value = child)
		{
			do 
			{
				JSON_ELEMENT* cur = value;
				value = value->next;
				delete cur;
			} while (value);
		}
	}
}

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
		do 
		{
			switch(*pa++)
			{
			case '\"':
				pa[-1] = 0;
				return pa;
			case '\\':
				switch (*pa++)
				{
				case '\\':
				case '\"':
				case '/':
				case 'b':
				case 'n':
				case 'r':
				case 'f':
				case 't':
				case 'u':
					continue;
				}
				return 0;
			}

		} while (pa < pb);
	}

	return 0;
}

PSTR JSON_ELEMENT::DoParse(PSTR pa, PSTR pb)
{
	if (pa = SkipWS(pa, pb))
	{
		switch (*pa++)
		{
		case '{':
			return DoParseObject(pa, pb);
		case '[':
			return DoParseArray(pa, pb);
		case '\"':
			if (pb = DoParseString(pa, pb))
			{
				type = v_string;
				str = pa;
				return pb;
			}
			break;
		default:
			PSTR pc = --pa;
			do 
			{
				char c = *pa;
				if ((ULONG)(c - '0') > (ULONG)('9' - '0') && c != '.')
				{
					if (pc != pa)
					{
						type = v_string;
						memcpy(pc - 1, pc, pa - pc);
						str = pc - 1;
						pa[-1] = 0;
						return pa;
					}
					return 0;
				}
			} while (++pa < pb);
		}
	}

	return 0;
}

PSTR JSON_ELEMENT::DoParseObject(PSTR pa, PSTR pb)
{
	type = v_object;

	JSON_ELEMENT** pnext = &child;

	bool bWaitComma = false;
	do 
	{
		if (!(pa = SkipWS(pa, pb)))
		{
			return 0;
		}

		switch (*pa++)
		{
		case '}':
			return pa;
		case '\"':
			if (bWaitComma)
			{
				return 0;
			}
			break;
		case ',':
			if (bWaitComma)
			{
				bWaitComma = false;
				continue;
			}
		default: return 0;
		}

		PSTR ValueName = pa;

		if (!(pa = DoParseString(pa, pb)))
		{
			return 0;
		}

		if (!(pa = SkipWS(pa, pb)) || *pa++ != ':' || !(pa = SkipWS(pa, pb)))
		{
			return 0;
		}

		if (JSON_ELEMENT* value = new JSON_ELEMENT(ValueName))
		{
			*pnext = value, pnext = &value->next;

			if (!(pa = value->DoParse(pa, pb)))
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}

		bWaitComma = true;

	} while (pa < pb);

	return 0;
}

PSTR JSON_ELEMENT::DoParseArray(PSTR pa, PSTR pb)
{
	type = v_array;

	JSON_ELEMENT** pnext = &child;
	ULONG n = 0;
	bool bWaitComma = false;

	do 
	{
		if (!(pa = SkipWS(pa, pb)))
		{
			return 0;
		}

		switch (*pa++)
		{
		case ']':
			return pa;
		case ',':
			if (bWaitComma)
			{
				bWaitComma = false;
				continue;
			}
			else
			{
				return 0;
			}
		default: 
			--pa;
		}

		if (JSON_ELEMENT* value = new JSON_ELEMENT(MAKEINTRESOURCEA(n++)))
		{
			*pnext = value, pnext = &value->next;

			if (!(pa = value->DoParse(pa, pb)))
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}

		bWaitComma = true;

	} while (pa < pb);

	return 0;
}

_NT_END
