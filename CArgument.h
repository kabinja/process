#if _MSC_VER >= 1800

#pragma once

#include "afx.h"

#include <tuple>
#include <functional>
#include <vector>

template <typename T, typename... Ts>
class CProcess;


class CArgument
{
public:
	CArgument(const VARIANT_WC& varValue, CString szName, CString szDescr)
	{
		m_varValue = varValue;
		m_szName = szName;
		m_szDescr = szDescr;
	}

	~CArgument()
	{
	}

	template<typename T>
	static CArgument* CreateNewArg(T* arg, CString szName, CString szDescr)
	{
		VARIANT_WC var;
		var.set<T*>(arg);

		return new CArgument(var, szName, szDescr);
	}

	CString GetName()
	{
		return m_szName;
	}

	CString GetDescr()
	{
		return m_szDescr;
	}

	const VARIANT_WC& GetValue()
	{
		return m_varValue;
	}

	VARIANT_WC& GetValueNC()
	{
		return m_varValue;
	}

	void SetValue(VARIANT_WC& varValue)
	{
		m_varValue = varValue;
	}

private:
	VARIANT_WC m_varValue;
	CString m_szName;
	CString m_szDescr;
};

#endif