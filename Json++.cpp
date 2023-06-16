#include "stdafx.h"

_NT_BEGIN

#include "json++.h"

Json Json::_G_defjs;

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

PSTR Json::DoParse(PCSTR name, PSTR pa, PSTR pb)
{
	if (pa = SkipWS(pa, pb))
	{
		Json* pObj;

		switch (*pa++)
		{
		case '{':
			return OnObject(name, &pObj) ? pObj->DoParseObject(pa, pb, this) : 0;
		case '[':
			return OnArray(name, &pObj) ? pObj->DoParseArray(pa, pb, this) : 0;
		case '\"':
			return (pb = DoParseString(pa, pb)) ? (OnString(name, pa) ? pb : 0) : 0;
		default:
			--pa;

			if (!memcmp(pa, _S_true, sizeof(_S_true) - 1))
			{
				return OnBOOL(name, true) ? pa + sizeof(_S_true) - 1 : 0;
			}

			if (!memcmp(pa, _S_false, sizeof(_S_false) - 1))
			{
				return OnBOOL(name, false) ? pa + sizeof(_S_false) - 1 : 0;
			}

			return OnNumber(name, _strtoui64(pa, &pa, 10)) ? pa : 0;
		}
	}

	return 0;
}

PSTR Json::DoParseObject(PSTR pa, PSTR pb, Json* pObj)
{
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
			return pObj->OnEnd(FALSE) ? pa : 0;
		case '\"':
			if (bWaitComma)
			{
				return 0;
			}
			break;
		case ',':
			if (bWaitComma && Separator())
			{
				bWaitComma = false;
				continue;
			}
		default: return 0;
		}

		PSTR name = pa;

		if (!(pa = DoParseString(pa, pb)))
		{
			return 0;
		}

		if (!(pa = SkipWS(pa, pb)) || *pa++ != ':' || !(pa = SkipWS(pa, pb)))
		{
			return 0;
		}

		if (!(pa = DoParse(name, pa, pb)))
		{
			return 0;
		}

		bWaitComma = true;

	} while (pa < pb);

	return 0;
}

PSTR Json::DoParseArray(PSTR pa, PSTR pb, Json* pObj)
{
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
			return pObj->OnEnd(TRUE) ? pa : 0;
		case ',':
			if (bWaitComma && Separator())
			{
				bWaitComma = false;
				continue;
			}
			return 0;
		default: 
			--pa;
		}

		if (!(pa = DoParse(0, pa, pb)))
		{
			return 0;
		}

		bWaitComma = true;

	} while (pa < pb);

	return 0;
}

_NT_END
