#if _MSC_VER >= 1800

#pragma once

#include <tuple>
#include <functional>
#include <vector>

#include "CVariantEx.h"
#include "CArgument.h"

namespace helper
{
	template <std::size_t... Ts>
	struct index {};

	template <std::size_t N, std::size_t... Ts>
	struct gen_seq : gen_seq<N - 1, N - 1, Ts...> {};

	template <std::size_t... Ts>
	struct gen_seq<0, Ts...> : index<Ts...>{};

	template<int... Indices>
	struct indices
	{
		typedef indices<Indices..., sizeof...(Indices)> next;
	};

	template<int N>
	struct build_indices
	{
		typedef typename build_indices<N - 1>::type::next type;
	};

	template<>
	struct build_indices<0>
	{
		typedef indices<> type;
	};
}

class CProcessBase
{
public:
	CProcessBase(){}
	virtual ~CProcessBase(){}

	virtual CString GetName(int i) = 0;
	virtual CString GetDescr(int i) = 0;
	virtual const VARIANT_WC& GetValue(int i) = 0;
	virtual VARIANT_WC& GetValueNC(CString szName) = 0;
	virtual void Run() = 0;
	
	virtual size_t Arity() = 0;

protected:
	virtual CArgument* CreateArgument(size_t index) { return NULL; }
};

template <typename T,typename... Ts>
class CProcess : public CProcessBase
{

public:
	CProcess(std::function<T(Ts...)> func, CStringArray* pNames, CStringArray* pDescr, Ts&&... Args)
	{
		m_pfn = func;
		m_tArgs = std::make_tuple(std::forward<Ts>(Args)...);

		if (!CreateArguments(pNames, pDescr))
			AfxThrowInvalidArgException();
	}

	~CProcess()
	{
		for (INT_PTR i = m_arArgs.GetSize() - 1; i >= 0; i--)
			delete m_arArgs.GetAt(i);
	}

	template <typename... Args, std::size_t... Is>
	void func(std::tuple<Args...>& tup, helper::index<Is...>)
	{
		m_pfn(std::get<Is>(tup)...);
	}

	template <typename... Args>
	void func(std::tuple<Args...>& tup)
	{
		func(tup, helper::gen_seq<sizeof...(Args)>{});
	}

	CString GetName(int i)
	{	
		CArgument *pArg = GetArgument(i);
		return pArg ? pArg->GetName() : CString(_T("#Error Name#"));
	}

	CString GetDescr(int i)
	{
		CArgument *pArg = GetArgument(i);
		return pArg ? pArg->GetDescr() : CString(_T("#Error Description#"));
	}

	const VARIANT_WC& GetValue(int i)
	{
		CArgument *pArg = GetArgument(i);

		return pArg->GetValue();
	}

	VARIANT_WC& GetValueNC(CString szName)
	{
		int i = FindArgIndex(szName);
		
		CArgument *pArg = GetArgument(i);

		return pArg->GetValueNC();
	}

	size_t Arity()
	{
		return std::tuple_size<std::tuple<Ts...>>::value;
	}

	void Run()
	{
		func(m_tArgs);
	}

protected:

	CArgument *GetArgument(int i)
	{
		if (i < 0 || i >= m_arArgs.GetSize())
			return NULL;

		return m_arArgs.GetAt(i);
	}

	int FindArgIndex(CString szName)
	{
		int iFound = -1;
		BOOL bFound = FALSE;

		for (int i = 0; i < m_arArgs.GetSize() && !bFound; i++)
		{
			if (GetArgument(i)->GetName().CompareNoCase(szName) == 0)
			{
				iFound = i;
				bFound = TRUE;
			}
				
		}

		return iFound;
	}

	// Helper functions
	template<size_t... Indices>
	void CreateArguments(CStringArray *pNames, CStringArray *pDescr, helper::indices<Indices...>)
	{
		const auto size = std::tuple_size<std::tuple<Ts...>>::value;
		
		CArgument* table[] =
		{
			CArgument::CreateNewArg<std::tuple_element<Indices, std::tuple<Ts...>>::type>(&std::get<Indices>(m_tArgs), pNames->GetAt(Indices), pDescr->GetAt(Indices))
			...
		};

		for (size_t i = 0; i < size; i++)
		{
			m_arArgs.Add(table[i]);
		}
	}

	BOOL CreateArguments(CStringArray *pNames, CStringArray *pDescr)
	{
		if (pNames->GetSize() != pDescr->GetSize() || pNames->GetSize() != Arity())
			return FALSE;
		
		typedef typename std::decay<std::tuple<Ts...>>::type decay_type;
		const auto tuple_size = std::tuple_size<decay_type>::value;
		typedef typename helper::build_indices<tuple_size>::type indices_type;

		CreateArguments(pNames, pDescr, indices_type{});

		return TRUE;
	}

protected:
	// Attributes
	std::function<T(Ts...)> m_pfn;
	std::tuple<Ts...> m_tArgs;
	CArray<CArgument*> m_arArgs;
};

#endif