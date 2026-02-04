#include "stdafx.h"
#include "json.h"

_NT_BEGIN

#include "print.h"

void print(ULONG len, PCSTR buf)
{
	if (len < 0x100)
	{
		DbgPrint("%.*hs\r\n\r\n", len, buf);
		return;
	}

	ULONG cb;

	do 
	{
		cb = __min(0x100, len);
		DbgPrint("%.*hs", cb, buf);

	} while (buf += cb, len -= cb);
	
	DbgPrint("\r\n\r\n");
}

void print(JsonValue* value, PSTR str = 0)
{
	int len = 0;
	PSTR buf = 0, psz = 0;
	while (0 < (len = value->ToString(psz, len, &psz, &len)))
	{
		if (buf)
		{
			if (str)
			{
				if (strcmp(str, buf))
				{
					__debugbreak();
				}
			}
			else
			{
				print(len, buf);

				if (str = (PSTR)_malloca(len + 1))
				{
					memcpy(str, buf, len + 1);

					JsonValue val;
					if (val.DoParse(buf, psz))
					{
						print(&val, str);
					}
					else
					{
						__debugbreak();
					}

					_freea(str);
				}
			}

			break;
		}

		++len;
		if (psz = buf = (PSTR)_malloca(len))
		{
			buf[len - 1] = 0;
		}
		else
		{
			break;
		}
	}

	if (buf) _freea(buf);
}

void print(JsonObject* pObj)
{
	JsonValue value(0, pObj);
	print(&value);
	value.pObj = 0;
}

NTSTATUS ReadFromFile(_In_ PCWSTR lpFileName, _Out_ PVOID* ppb, _Out_ ULONG* pcb, _In_ ULONG _cb = 0, _In_ ULONG cb_ = 0);

void TestJson(PCWSTR lpFileName)
{
	PVOID pb;
	ULONG cb = 0;

	NTSTATUS status = ReadFromFile(lpFileName, &pb, &cb, 0, 1);

	DbgPrint("read(%ws)=%x [%u bytes]\r\n", lpFileName, status, cb);

	if (0 <= status)
	{
		if (cb)
		{
			reinterpret_cast<PSTR>(pb)[cb] = 0;

			PSTR pa = (PSTR)pb, end = pa + cb;
			do
			{
				DbgPrint("\r\n**********************************\r\n\r\n");
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

		delete [] pb;
	}
}

void simplydemo()
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
}

_NT_END