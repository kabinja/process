#if _MSC_VER >= 1800

#pragma once

#include <type_traits>

#define VARIANT_WC CVariantEx<CString*, int*, bool*, const CLog**, const CBitLog**>

template<typename F, typename... Ts>
struct variant_helper
{
	static const size_t size = sizeof(F) > variant_helper<Ts...>::size ? sizeof(F) : variant_helper<Ts...>::size;

	inline static void destroy(size_t id, void * data)
	{
		if (id == typeid(F).hash_code())
			reinterpret_cast<F*>(data)->~F();
		else
			variant_helper<Ts...>::destroy(id, data);
	}

	inline static void copy(size_t old_t, const void * old_v, void * new_v)
	{
		if (old_t == typeid(F).hash_code())
		{
			if (typeid(F).hash_code() != typeid(CString).hash_code())
				new (new_v)F(*reinterpret_cast<const F*>(old_v));
		}
			
		else
			variant_helper<Ts...>::copy(old_t, old_v, new_v);
	}
};

struct variant_null_class
{
};

template<typename F>
struct variant_helper<F>
{
	static const size_t size = sizeof(F);

	inline static void destroy(size_t id, void * data)
	{
		if (id == typeid(F).hash_code())
			reinterpret_cast<F*>(data)->~F();
	}

	inline static void copy(size_t old_t, const void * old_v, void * new_v)
	{
		if (old_t == typeid(F).hash_code())
			new (new_v)F(*reinterpret_cast<const F*>(old_v));
		else
			throw std::bad_cast();
	}
};

template<size_t size>
class raw_data { char data[size]; };

template<typename... Ts>
class CVariantEx
{
private:
	typedef variant_helper<Ts...> helper;
	size_t type_id;
	raw_data<helper::size> data;

public:

	CVariantEx()
	{
		type_id = invalid_type();
	}

	~CVariantEx()
	{
		helper::destroy(type_id, &data);
	}

	CVariantEx(CVariantEx<Ts...>&& old) : type_id(old.type_id), data(old.data)
	{
		old.type_id = invalid_type();
	}

	CVariantEx(const CVariantEx<Ts...>& old) : type_id(old.type_id)
	{
		helper::copy(old.type_id, &old.data, &data);
	}

	CVariantEx<Ts...>& operator= (CVariantEx<Ts...>&& old)
	{
		data = old.data;
		old.type_id = invalid_type();

		return *this;
	}

	CVariantEx<Ts...>& operator= (CVariantEx<Ts...> old)
	{
		std::swap(data, old.data);
		std::swap(type_id, old.type_id);

		return *this;
	}

	template<typename T>
	bool is()
	{
		return (type_id == typeid(T).hash_code());
	}

	bool is_valid()
	{
		return (type_id != typeid(variant_null_class).hash_code());
	}

	template<typename T, typename... Args>
	void set(Args&&... args)
	{
		helper::destroy(type_id, &data);

		new (&data) T(std::forward<Args>(args)...);
		type_id = typeid(T).hash_code();
	}

	template<typename T>
	T& get()
	{
		if (type_id == typeid(T).hash_code())
			return *reinterpret_cast<T*>(&data);
			//return *(T*)(&data);
		else
			throw std::bad_cast();
	}

	void *GetData()
	{
		return (void*)&data;
	}

	BOOL operator==(const CVariantEx<Ts...>& rhs) const
	{
		return m_typeIndex == rhs.m_typeIndex;
	}

	BOOL operator<(const CVariantEx<Ts...>& rhs) const
	{
		return m_typeIndex <rhs.m_typeIndex;
	}

private:
	size_t invalid_type()
	{
		return typeid(variant_null_class).hash_code();
	}
};

#endif