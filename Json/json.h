#pragma once

#ifndef DbgPrint
#ifdef _WIN32
EXTERN_C
ULONG
__cdecl
DbgPrint(
	_In_z_ _Printf_format_string_ PCSTR Format,
	...
);
#endif // _WIN32
#endif

#define _JSON_

struct JD
{
	ULONG64 yyy : 28;
	ULONG64 len : 04;
	ULONG64 xxx : 31;
	ULONG64 neg : 01;
};

struct JNode
{
	JNode* next = 0;
	PCSTR name;
	union {
		JNode* first;
		PCSTR pcstr;
		LONG64 iVal;
		JD fVal;
		bool bVal;
	};

	enum JVT { v_str, v_int, v_float, v_bool, v_obj, v_arr, v_null } vt;

	PSTR DoParse(PSTR pa, PSTR pb, ULONG level = 64);
	int ToString(PSTR buf, int len, PSTR* pbuf, int* plen);

	~JNode();

	JNode(PCSTR name = 0) : name(name), first(0), vt(v_null)
	{
	}

	JNode(PCSTR name, PCSTR pcstr) : name(name), pcstr(pcstr), vt(v_str)
	{
	}

	JNode(PCSTR name, LONG64 iVal) : name(name), iVal(iVal), vt(v_int)
	{
	}

	JNode(PCSTR name, ULONG64 iVal) : name(name), iVal(iVal), vt(v_int)
	{
	}

	JNode(PCSTR name, ULONG iVal) : name(name), iVal(iVal), vt(v_int)
	{
	}

	JNode(PCSTR name, LONG iVal) : name(name), iVal(iVal), vt(v_int)
	{
	}

	JNode(PCSTR name, INT iVal) : name(name), iVal(iVal), vt(v_int)
	{
	}

	JNode(PCSTR name, UINT iVal) : name(name), iVal(iVal), vt(v_int)
	{
	}

	JNode(PCSTR name, JD fVal) : name(name), fVal(fVal), vt(v_float)
	{
	}

	JNode(PCSTR name, bool bVal) : name(name), bVal(bVal), vt(v_bool)
	{
	}

	JNode(PCSTR name, JVT vt) : name(name), vt(vt), first(0)
	{
	}

	JNode(JVT vt) : name(0), vt(vt), first(0)
	{
	}

	void* operator new(size_t s);

	void operator delete(void* pv);

	void insert(_In_ JNode* value);

	void remove(_In_ JNode* value);

	// ++ vt_obj
	PSTR DoParseObject(PSTR pa, PSTR pb, ULONG level);
	int ObjectToString(PSTR buf, int len, PSTR* pbuf, int* plen);
	JNode* operator[](_In_ PCSTR name);

	template<typename Type>
	void SetValue(PCSTR name, Type arg)
	{
		insert(new JNode(name, arg));
	}
	// -- vt_obj

	// ++ vt_arr
	PSTR DoParseArray(PSTR pa, PSTR pb, ULONG level);
	int ArrayToString(PSTR buf, int len, PSTR* pbuf, int* plen);

	template<typename Type>
	void push_back(Type arg)
	{
		insert(new JNode(0, arg));
	}
	// -- vt_arr
};

#ifdef _WIN32

struct __declspec(novtable) JHeap
{
	virtual PVOID Alloc(_In_ size_t cb) = 0;
	virtual BOOL Free(_In_ PVOID pv) = 0;
};

struct JHeapPtr 
{
	JHeap* heap;
};

struct JHeap1 : public JHeap
{
	virtual PVOID Alloc(_In_ size_t cb)
	{
		if (((cb += sizeof(PVOID) - 1) &= ~(sizeof(PVOID) - 1)) > m_cb)
		{
			return 0;
		}

		PVOID pv = (PBYTE)m_buf + m_ptr;
		m_cb -= (ULONG)cb, m_ptr += (ULONG)cb, m_nOP++;

		return pv;
	}

	virtual BOOL Free(_In_ PVOID pv)
	{
		if ((ULONG_PTR)pv - (ULONG_PTR)m_buf < m_ptr)
		{
			m_nOP--;
			return TRUE;
		}

		return FALSE;
	}

	PVOID m_buf;
	ULONG m_cb = 0;
	ULONG m_ptr = 0;
	ULONG m_nOP = 0;

	JHeap1(ULONG cb)
	{
		if (m_buf = LocalAlloc(LMEM_FIXED, cb <<= (cb < 0x400 ? 3 : 1)))
		{
			m_cb = cb;
		}
	}

	~JHeap1()
	{
		if (m_nOP)
		{
			__debugbreak();
		}
		LocalFree(m_buf);
	}
};

struct JHeap2 : public JHeap
{
	virtual PVOID Alloc(_In_ size_t cb)
	{
		if (cb > sizeof(JNode))
		{
			return 0;
		}

		if (JNode * first = m_first)
		{
			m_first = first->next;

			m_nOP++;

			return first;
		}

		return 0;
	}

	virtual BOOL Free(_In_ PVOID pv)
	{
		if ((ULONG_PTR)pv - (ULONG_PTR)m_buf < m_cb)
		{
			reinterpret_cast<JNode*>(pv)->next = m_first;
			m_first = reinterpret_cast<JNode*>(pv);
			m_nOP--;
			return TRUE;
		}

		return FALSE;
	}

	JNode* m_first = 0;
	PVOID m_buf = 0;
	ULONG m_cb = 0;
	ULONG m_nOP = 0;

	JHeap2(ULONG cb)
	{
		union {
			PVOID pv;
			JNode* node;
		};

		if (pv = LocalAlloc(LMEM_FIXED, cb <<= (cb < 0x400 ? 3 : 1)))
		{
			m_buf = pv;
			JNode* first = 0;
			m_cb = cb;
			if (cb /= sizeof(JNode))
			{
				do
				{
					node->next = first, first = node++;
				} while (--cb);
			}

			m_first = first;
		}
	}

	~JHeap2()
	{
		if (m_nOP)
		{
			__debugbreak();
		}
		LocalFree(m_buf);
	}
};

#endif // _WIN32