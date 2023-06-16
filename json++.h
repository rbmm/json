#pragma once

class Json
{
	PSTR DoParseObject(PSTR pa, PSTR pb, Json* pObj);
	PSTR DoParseArray(PSTR pa, PSTR pb, Json* pObj);

protected:

	virtual BOOL OnObject(PCSTR /*name*/, Json** ppObj)
	{
		*ppObj = this;
		return TRUE;
	}

	virtual BOOL OnArray(PCSTR /*name*/, Json** ppObj)
	{
		*ppObj = this;
		return TRUE;
	}

	virtual BOOL OnString(PCSTR /*name*/, PCSTR /*value*/)
	{
		return TRUE;
	}

	virtual BOOL OnBOOL(PCSTR /*name*/, BOOL /*value*/)
	{
		return TRUE;
	}

	virtual BOOL OnNumber(PCSTR /*name*/, ULONG64 /*value*/)
	{
		return TRUE;
	}

	virtual BOOL OnEnd(BOOL /*bArray*/)
	{
		return TRUE;
	}

	virtual BOOL Separator()
	{
		return TRUE;
	}
public:
	PSTR DoParse(PCSTR name, PSTR pa, PSTR pb);

	inline static const char _S_true[] = "true";
	inline static const char _S_false[] = "false";

	static Json _G_defjs;
};

