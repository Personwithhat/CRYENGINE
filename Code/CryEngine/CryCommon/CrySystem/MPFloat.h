// Copyright 2001-2017 Crytek GmbH / Crytek Group. All rights reserved. 

#pragma once

// PERSONAL TODO: Unreadable without this + maybe some other edits/options....syntaxing is off
#ifndef ILINE
//	#define ILINE inline
#endif

// PERSONAL VERIFY: Include guards still needed, even with pragma?
#ifndef MPFLOAT
	#define MPFLOAT

// For exception handling
void CryFatalError(const char*, ...);
#include <CryCore/BoostHelpers.h>

// PERSONAL TODO: https://github.com/CRYTEK/CRYENGINE/pull/169/files was needed to get boost visible here!
// However IXml.h can't access TimeValue.h (even though they're in the same directory space)
// So maybe github wasn't needed?? Incomplete edits?? Etc.....
// Boost wrapper around GNU Multiple Precision Arithmetic Library (GMP)
// MORE TODO: Turns out I implimented the above wrong, so it crashed (cuz comment before XML line) needs review and restesting xd
#include <boost/multiprecision/gmp.hpp>

// Is-mpfloat-type test
#define isMP   std::is_base_of<boost::multiprecision::newNum<T>, T>::value

// Limit a function to mpfloat-type's e.g. mpfloat, rTime, nTime, kTime, etc.
#define MPOnly template <class T, typename boost::enable_if_c<isMP>::type* = 0>

// Limit a function to NON mpfloat types
#define MPOff template <class T, typename boost::disable_if_c<isMP>::type* = 0>

// To prevent ambiguous operators due to int => (CTimevalue or mpfloat) implicit conversions.....
// This should also be applied to anywhere where there's MPOnly => Use Limit(Ctimevalue) in case time-value starts implicitly accepting....
#define AcceptOnly(x) template <class T, typename boost::enable_if_c<boost::is_same<T, x>::value>::type* = 0>
#define TVOnly AcceptOnly(CTimeValue)

// Declare a strong-typed multiprecision value.
#define decl_mp_type(name) \
namespace boost { namespace multiprecision {\
	class name : public newNum<name> {\
	public:\
		ILINE constexpr name()					: newNum()	{}\
		ILINE constexpr name(const name& n) : newNum(n.backend())	{};\
		\
		name& operator=(const name& inRhs) { backend() = inRhs.backend(); return *this; }\
		\
		using newNum::newNum;\
		using newNum::operator=;\
		\
	   const CTypeInfo& TypeInfo() const { static MPInfo<name> Info(#name); return Info; }\
		\
		/* PERSONAL VERIFY: How should Min/Max's work here and in CTimeValue? What should the range be for each?? + constexpr format... */\
		static name Min() { return name().lossy(std::numeric_limits<float>::min()); }\
		static name Max() { return name().lossy(std::numeric_limits<float>::max()); }\
	};\
}}\
using boost::multiprecision::name;

namespace boost{
	// Does not match special std::string/etc. classes
	template <class T>
	struct is_string
		: public mpl::bool_<
			is_same<       char *, typename std::decay< T >::type >::value ||
			is_same< const char *, typename std::decay< T >::type >::value
		>
	{};
}
class ICrySizer;
namespace boost { namespace multiprecision {
/* 
	PERSONAL TODO: 
		3) std::string should be changed to CryString. 
			Review any string=>mpfloat and mpfloat=>string usage, etc,
		4) Cleanup canonical values etc.
		5) Improve readability + macro's, convert inline to ILINE etc.
		6) Review nTime * mpfloat etc. operations, might be good to do mpfloat operator*(nTime, mpfloat){} etc.
*/
// Base for any mpfloat values, accepts precise inputs only e.g. int's or stringified floats. No floats/doubles.
template<class rType>
class newNum {
public:
	typedef gmp_float<50> Backend;
	#define this static_cast<rType*>(this)

	// Current check for legitimate operations/comparisons on e.g. integers but no floats
	template <class V> using valid_check = is_compatible_arithmetic_type<V, rType>;

	// For explicit conversion across strong-types
	template <class T, typename boost::enable_if_c<
		isMP //&& !boost::is_same<rType, T>::value PERSONAL TODO: ATM disabled same type for easier template conversions....
	>::type* = 0> 
	T conv() const { return T(backend()); } // PERSONAL TODO: As usual need to go over all conv()'s for validity, improvements, etc.

	// Precision lossy set.
	template <typename T> rType& lossy(const T& inRhs) { backend() = canonical_value(inRhs); return *this; }

	// PERSONAL VERIFY: Memory usage should probably be tracked for optimizing mpfloat size/etc.
	void GetMemoryUsage(class ::ICrySizer* pSizer)		  const { /*nothing*/ };
	void GetMemoryStatistics(class ::ICrySizer* pSizer)  const { /*nothing*/ };

	// WARNINGS: Previous attempts at overloads/etc.
	/*
		// WARNING: Float implicitly converted to numBackend, so this bypasses the rest of the setup.
		// explicit rType(const numBackend& n) { *this = n; }

		// WARNING : Delete is 'declared' and matches operator overloads, so we have to do SFINAE	
		// template <typename T> rType(T) = delete; 

		// WARNING: Templated constructors with default arguments will lose the default-argument portion on inheritance.
		// In this case, losing the enable_if SFINAE, causing the implicitly defined constructors to include 'every' type.
		// See 'Which way to write the enabler....' in https://www.boost.org/doc/libs/1_67_0/libs/core/doc/html/core/enable_if.html
		
		template <class V>
		inline newNum(const V& v, typename boost::enable_if_c<
			(boost::is_integral<V>::value || is_same<std::string, V>::value || is_convertible<V, const char*>::value)
			&& !is_convertible<typename detail::canonical<V, Backend>::type, Backend>::value
		>::type* = 0)
		{
			backend() = canonical_value(v);
		}
		*/

public: // Constructors and assignment operators
		ILINE constexpr newNum() {};
		ILINE constexpr newNum(const Backend& n) : m_backend(n) {};

		// Allows implicit conversion (0 can become newNum(0) etc.) but only for matches, 
		//	e.g. newNum(float), float won't implicitly convert to int and become a 'valid' constructor.
		//	(Integer-type || a string e.g. char*) etc.
		template <class V, typename enable_if_c<
				(is_integral<V>::value || is_string<V>::value)
				&& !is_convertible<typename detail::canonical<V, Backend>::type, Backend>::value
		>::type* = 0>
		ILINE newNum(const V& v)
		{
			backend() = canonical_value(v);
		}

		// PERSONAL TODO: This needed? Why is_convertible check above and here?????? WTH is this for anyway?
		/*template <class V, typename boost::enable_if_c<
			is_convertible<typename detail::canonical<V, Backend>::type, Backend>::value
			&& !detail::is_restricted_conversion<typename detail::canonical<V, Backend>::type, Backend>::value
		>::type* = 0>
		BOOST_MP_FORCEINLINE BOOST_CONSTEXPR newNum(const V& v)
		#ifndef BOOST_INTEL
			BOOST_MP_NOEXCEPT_IF(noexcept(Backend(std::declval<typename detail::canonical<V, Backend>::type const&>())))
		#endif
			: m_backend(canonical_value(v)) {}*/

	// Assignment
		template <class V>
		ILINE typename enable_if<valid_check<V>, rType& >::type
			operator=(const V& v)
			//BOOST_MP_NOEXCEPT_IF(noexcept(std::declval<Backend&>() = std::declval<const typename detail::canonical<V, Backend>::type&>()))
		{
			backend() = canonical_value(v);
			return *this;
		}

public: // Math operations
		rType operator-() const { rType ret; ret.backend() = backend(); ret.backend().negate(); return ret; };

		#define Operation(op, name)\
			/*rType ?? rType*/\
			ILINE rType  operator##op (const rType& inRhs)  const\
			{ rType ret; using default_ops::eval_##name; eval_##name(ret.backend(), backend(), inRhs.backend()); return ret; }\
			/*rType ?? XX*/\
			template <class V> ILINE typename enable_if<valid_check<V>, rType>::type\
			operator##op (const V& b) const \
			{ rType result; using default_ops::eval_##name; eval_##name(result.backend(), backend(), rType::canonical_value(b)); return BOOST_MP_MOVE(result); }\
			/*XX ?? rType*/\
			template <class V> friend typename enable_if<valid_check<V>, rType>::type\
			operator##op (const V& a, const rType& b)\
			{ rType result; using default_ops::eval_##name; eval_##name(result.backend(), rType::canonical_value(a), b.backend()); return BOOST_MP_MOVE(result); }\
			/*rType ?= rType*/\
			ILINE rType&  operator##op= (const rType& inRhs)\
			{ using default_ops::eval_##name; eval_##name(backend(), inRhs.backend()); return *this; }\
			/*rType ?= XX*/\
			template <class V> ILINE typename enable_if<valid_check<V>, rType& >::type\
			operator##op= (const V& v)\
			{ using default_ops::eval_##name; eval_##name(backend(), rType::canonical_value(v)); return *this; }\
			/*newNum ?? newNum, for cross-mpfloat functions. PERSONAL TODO: Need to verify that this doesn't conflict.*/\
			ILINE newNum  operator##op (const newNum& inRhs)  const\
			{ newNum ret; using default_ops::eval_##name; eval_##name(ret.backend(), backend(), inRhs.backend()); return ret; }

		Operation(+, add);
		Operation(-, subtract);
		Operation(/, divide);
		Operation(*, multiply);
		#undef Operation
		#undef this

public: // Comparisons

		/*rType ?? rType*/
		#define check if (detail::is_unordered_comparison(*this, inRhs)) return false;
		bool operator==(const rType& inRhs) const { check return  (backend().compare(inRhs.backend()) == 0); }
		bool operator!=(const rType& inRhs) const { check return !(backend().compare(inRhs.backend()) == 0); }
		bool operator<(const rType& inRhs)  const { check return  (backend().compare(inRhs.backend()) < 0);  }
		bool operator>(const rType& inRhs)  const { check return  (backend().compare(inRhs.backend()) > 0);  }
		bool operator>=(const rType& inRhs) const { check return !(backend().compare(inRhs.backend()) < 0);  }
		bool operator<=(const rType& inRhs) const { check return !(backend().compare(inRhs.backend()) > 0);  }

		/*rType ?? XX*/
		#define boiler template <class V>  typename enable_if_c<valid_check<V>::value && !is_string<V>::value, bool>::type
		boiler ILINE operator==(const V& inRhs) const { check return  (backend().compare(canonical_value(inRhs)) == 0); }
		boiler ILINE operator!=(const V& inRhs) const { check return !(backend().compare(canonical_value(inRhs)) == 0); }
		boiler ILINE operator<(const V& inRhs)  const { check return  (backend().compare(canonical_value(inRhs)) < 0);  }
		boiler ILINE operator>(const V& inRhs)  const { check return  (backend().compare(canonical_value(inRhs)) > 0);  }
		boiler ILINE operator>=(const V& inRhs) const { check return !(backend().compare(canonical_value(inRhs)) < 0);  }
		boiler ILINE operator<=(const V& inRhs) const { check return !(backend().compare(canonical_value(inRhs)) > 0);  }
		#undef check

		/*XX ?? rType*/
		#define check if (detail::is_unordered_comparison(inLhs, inRhs)) return false;
		/*Had to flip conditionals to compare with backend().compare() e.g. 0 < rType becomes rType > 0*/
		boiler friend operator==(const V& inLhs, const rType& inRhs) { check return  (inRhs.backend().compare(canonical_value(inLhs)) == 0); }
		boiler friend operator!=(const V& inLhs, const rType& inRhs) { check return !(inRhs.backend().compare(canonical_value(inLhs)) == 0); }
		boiler friend operator<(const V& inLhs, const rType& inRhs)  { check return  (inRhs.backend().compare(canonical_value(inLhs)) > 0);  }
		boiler friend operator>(const V& inLhs, const rType& inRhs)  { check return  (inRhs.backend().compare(canonical_value(inLhs)) < 0);  }
		boiler friend operator>=(const V& inLhs, const rType& inRhs) { check return  !(inRhs.backend().compare(canonical_value(inLhs)) > 0); }
		boiler friend operator<=(const V& inLhs, const rType& inRhs) { check return  !(inRhs.backend().compare(canonical_value(inLhs)) < 0); }
		#undef check
		#undef boiler 

public: // Other re-implimented number<> functions

	//  PERSONAL TODO: Make sure rType() doesn't conflict with anything + conversions limited as expected (.conv() needed for cross-strongtype etc.)
	// Conversion operator
	inline explicit operator bool()			  const { return !is_zero(); }
	inline explicit operator rType()			  const { return *static_cast<const rType*>(this); }
	template <class T, typename boost::enable_if_c< 
		boost::is_arithmetic<T>::value || is_string<T>::value // Convert only to string/float/int
	>::type* = 0> explicit operator T() const { return this->template convert_to<T>(); }

	// Misc. funcs
	inline bool is_zero() const { using default_ops::eval_is_zero; return eval_is_zero(m_backend); }
	inline int sign()	    const { using default_ops::eval_get_sign; return eval_get_sign(m_backend); }
	string str(std::streamsize digits = 0, std::ios_base::fmtflags f = std::ios_base::fmtflags(0)) const { return string(m_backend.str(digits, f).c_str()); }

	// Backend()
	ILINE Backend& backend() noexcept							  { return m_backend; }
	ILINE constexpr const Backend& backend() const noexcept { return m_backend; }

	// Canonical_value
	static ILINE constexpr const Backend& canonical_value(const rType& v) noexcept { return v.m_backend; }
	//static ILINE typename detail::canonical<std::string, Backend>::type canonical_value(const std::string& v) noexcept { return v.c_str(); }
	template <class V>
	static ILINE constexpr typename boost::disable_if<is_same<typename detail::canonical<V, Backend>::type, V>, typename detail::canonical<V, Backend>::type>::type
		canonical_value(const V& v) noexcept { return static_cast<typename detail::canonical<V, Backend>::type>(v); }
	template <class V>
	static ILINE constexpr typename boost::enable_if<is_same<typename detail::canonical<V, Backend>::type, V>, const V&>::type
		canonical_value(const V& v) noexcept { return v; }

private:
	// Conversion implementation.
	template <class T> T convert_to()		  const { T result; convert_to_imp(&result); return result; }
	template <class T>
	void convert_to_imp(T* result)const
	{
		using default_ops::eval_convert_to;
		eval_convert_to(result, m_backend);
	}
	void convert_to_imp(std::string* result)const	// PERSONAL TODO: String stuff is wrong now!
	{
		*result = this->str();
	}

	Backend m_backend;
};

//**
//**	Various multi-precision float math functions
//**
	//ILINE int64 int_ceil(f64 f)   { int64 i = int64(f); return (f > f64(i)) ? i + 1 : i; }
	template <class rType> ILINE int64 int_ceil(const newNum<rType>& f)  { int64 i = int64(f); return (f > i) ? i + 1 : i; }
	template <class rType> ILINE int64 int_round(const newNum<rType>& f) { return f < 0 ? int64(f - "0.5") : int64(f + "0.5"); }

	// ILINE float div_min(float n, float d, float m) { return n * d < m * d * d ? n / d : m; }
	template <class rType> ILINE rType div_min(const newNum<rType>& n, const newNum<rType>& d, const newNum<rType>& m) {
		// PERSONAL TODO: < operator slightly hacky here.... can't implement newNum<> comparisons easily, ambiguous conflicts.
		return (rType)(n * d < rType(m * d * d) ? n / d : m);
	}

	// PERSONAL NOTE: No point in floor atm. (int) truncates decimal's, which is the intended result for most.
	//ILINE int64 int_floor(const newNum<rType>& f) { int64 i = int64(f); return (f < i) ? i - 1 : i; }

	// template<typename T> ILINE T mod(T a, T b)			{ return a - trunc(a / b) * b; }
	template <class rType> ILINE rType mod(const newNum<rType>& a, const newNum<rType>& b) { return (rType)(a - trunc(a / b) * b); }

	#define UNARY_OP_FUNCTOR(func)\
		template <class rType> \
		ILINE rType func(const newNum<rType>& arg)\
		{\
			rType result;\
			using default_ops::BOOST_JOIN(eval_,func);\
			BOOST_JOIN(eval_,func)(result.backend(), arg.backend());\
			return BOOST_MP_MOVE(result);\
		}

		UNARY_OP_FUNCTOR(abs);
		UNARY_OP_FUNCTOR(trunc);
		UNARY_OP_FUNCTOR(sqrt);
		UNARY_OP_FUNCTOR(floor);
		UNARY_OP_FUNCTOR(sin)
	#undef UNARY_OP_FUNCTOR

	#define BINARY_OP_FUNCTOR(func)\
		template <class rType> \
		ILINE rType func(const newNum<rType>& a, const newNum<rType>& b)\
		{\
			rType result;\
			using default_ops::BOOST_JOIN(eval_,func);\
			BOOST_JOIN(eval_,func)(result.backend(), a.backend(), b.backend());\
			return BOOST_MP_MOVE(result);\
		}
		BINARY_OP_FUNCTOR(pow)
	#undef BINARY_OP_FUNCTOR
}} // END boost::multiprecision namespaces

// TypeInfo for mpfloat
	// PERSONAL TODO: Was Split in CryTypeInfo.inl and TypeInfo_decl.h
	// Since mpfloat -> missing type specifier. Boost not loaded properly here (or visible in the main window)
	/*
		BASIC_TYPE_INFO(mpfloat);
		TYPE_INFO_BASIC(mpfloat);

		// mpfloat
		string ToString(mpfloat const& val)
		{
			return val.str();
		}

		bool FromString(mpfloat& val, const char* s)
		{
			val = mpfloat(s);
			return true;
		}
	*/

	MPOnly string ToString(T const& val)
	{
		return val.str();
	}
	MPOnly bool FromString(T& val, const char* s)
	{
		val = T(s);
		return true;
	}

	//! PERSONAL TODO: 
	// Due to 'clamp_tpl not defined' etc. issues when including CryCustomTypes....
   // Have to define a duplicate TypeInfo class that functions just like TTypeInfo....fix?
	#include "../CryCore/CryTypeInfo.h"

	//! String helper function.
	template<class T>
	inline bool HasStringMP(const T& val, FToString flags, const void* def_data = 0)
	{
		if (flags.SkipDefault())
		{
			if (val == (def_data ? *(const T*)def_data : T()))
				return false;
		}
		return true;
	}
	template<class T>
	struct MPInfo : CTypeInfo
	{
		MPInfo(cstr name)
			: CTypeInfo(name, sizeof(T), alignof(T))
		{}

		virtual bool ToValue(const void* data, void* value, const CTypeInfo& typeVal) const
		{
			if (&typeVal == this)
				return *(T*)value = *(const T*)data, true;
			return false;
		}
		virtual bool FromValue(void* data, const void* value, const CTypeInfo& typeVal) const
		{
			if (&typeVal == this)
				return *(T*)data = *(const T*)value, true;
			return false;
		}

		virtual string ToString(const void* data, FToString flags = {}, const void* def_data = 0) const
		{
			if (!HasStringMP(*(const T*)data, flags, def_data))
				return string();
			return ::ToString(*(const T*)data);
		}
		virtual bool FromString(void* data, cstr str, FFromString flags = {}) const
		{
			if (!*str)
			{
				if (!flags.SkipEmpty())
					*(T*)data = T();
				return true;
			}
			return ::FromString(*(T*)data, str);
		}
		virtual bool ValueEqual(const void* data, const void* def_data = 0) const
		{
			return *(const T*)data == (def_data ? *(const T*)def_data : T());
		}

		virtual void GetMemoryUsage(ICrySizer* pSizer, void const* data) const
		{}
	};
//~TypeInfo

// Unfortunately a large majority of CE has virtual interfaces -> Which obv won't work with templates.
// Can't think of a way to insert an intermediary class for these strong-type's (while preserving return type...etc.)
// So a standerdized macro is the solution for now(ofc use MPOnly when possible)
#define MP_FUNCTION(T) decl_mp_type(T)
	#include "mpfloat.types" // PERSONAL VERIFY: grep this and clean up access paths .-.
#undef MP_FUNCTION

// Any precision-losing casts should be done like this.
#define BADF       (float)
#define BADMP(x)   mpfloat().lossy(x)
#define MP_EPSILON BADMP(FLT_EPSILON)  // PERSONAL VERIFY: If only there was a better way for this/CTimeValue epsilon

#endif // MPFLOAT