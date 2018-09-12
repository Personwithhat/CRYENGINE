// Copyright 2001-2017 Crytek GmbH / Crytek Group. All rights reserved. 

#ifndef MPFLOAT
	#define MPFLOAT
	#pragma once

// For exception handling
void CryFatalError(const char*, ...);
#include <CryCore/BoostHelpers.h>

// Boost wrapper around GNU Multiple Precision Arithmetic Library (GMP)
#include <boost/multiprecision/gmp.hpp>

// Is-mpfloat-type test
#define isMP   std::is_base_of<boost::multiprecision::newNum<T>, T>::value

// Limit a function to mpfloat-type's e.g. mpfloat, rTime, nTime, kTime, etc.
// PERSONAL IMPROVE: Get only the 'typename XXXX =0' portion into macro, to keep 'template XX' setup??
#define MPOnly template <class T, typename boost::enable_if_c<isMP>::type* = 0>

// Limit a function to NON mpfloat types
#define MPOff template <class T, typename boost::disable_if_c<isMP>::type* = 0>

// To prevent ambiguous operators due to int => (CTimevalue or mpfloat) implicit conversions.....
// This should also be applied to anywhere where there's MPOnly => Use Limit(Ctimevalue) in case time-value starts implicitly accepting....
#define AcceptOnly(x) template <class T, typename boost::enable_if_c<boost::is_same<T, x>::value>::type* = 0>
#define isTV	boost::is_same<T, CTimeValue>::value
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
		/* PERSONAL IMPROVE: ATM Using number's limits, can implement custom numeric limits later. */\
		static name Min() { return std::numeric_limits<number<name::Backend>>::lowest().backend(); }\
		static name Max() { return std::numeric_limits<number<name::Backend>>::max().backend(); }\
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
/* PERSONAL IMPROVE & PERSONAL DEBUG:
	1) To prevent ambiguous sets, at the moment mpfloat does not accept 'string' as input, only const char*. Try it out, maybe it'll work?
	2) Improve readability + macro's, cleanup canonical values, convert inline to ILINE etc.
*/

// PERSONAL TODO: Move this somewhere else, clean up this file and what not. It's messy up here!
template <class S>
void format_MP_string(S& str, boost::intmax_t my_exp, boost::intmax_t digits, std::ios_base::fmtflags f, bool iszero)
{
   typedef typename S::size_type size_type;
   bool scientific = (f & std::ios_base::scientific)  == std::ios_base::scientific;
	bool skipZero   = (f & std::ios_base::dec)			== std::ios_base::dec;
   bool fixed      = ((f & std::ios_base::fixed)		== std::ios_base::fixed || skipZero); // SkipZero implies fixed.
   bool showpoint  = (f & std::ios_base::showpoint)   == std::ios_base::showpoint;
   bool showpos    = (f & std::ios_base::showpos)     == std::ios_base::showpos;

   bool neg = str.size() && (str[0] == '-');

   if(neg)
      str.erase(0, 1);

   if(digits == 0)
   {
      digits = (std::max)(str.size(), size_type(16));
   }

   if(iszero || str.empty() || (str.find_first_not_of('0') == S::npos))
   {
      // We will be printing zero, even though the value might not
      // actually be zero (it just may have been rounded to zero).
      str = "0";
      if(scientific || fixed)
      {
         str.append(1, '.');
         if(fixed && skipZero){ str.append(1, '0'); } else { str.append(size_type(digits), '0'); }
         if(scientific)
            str.append("e+00");
      }
      else if(showpoint)
      {
         str.append(1, '.');
         if(digits > 1)
				str.append(size_type(digits - 1), '0');
      }
      if(neg)
         str.insert(static_cast<string::size_type>(0), 1, '-');
      else if(showpos)
         str.insert(static_cast<string::size_type>(0), 1, '+');
      return;
   }

   if((!fixed || skipZero) && !scientific && !showpoint)
   {
      //
      // Suppress trailing zeros:
      //
		int strLen = str.length();
		int pos = strLen;
		while(pos != 0 && str.at(--pos) == '0'){}
      if(pos != strLen)
         ++pos;
      str.erase(pos, strLen);
      if(str.empty())
         str = "0";
   }
   else if(!fixed || (my_exp >= 0))
   {
      //
      // Pad out the end with zero's if we need to:
      //
      boost::intmax_t chars = str.size();
      chars = digits - chars;
      if(scientific)
         ++chars;
      if(chars > 0)
      {
         str.append(static_cast<string::size_type>(chars), '0');
      }
   }

   if(fixed || (!scientific && (my_exp >= -4) && (my_exp < digits)))
   {
      if(1 + my_exp > static_cast<boost::intmax_t>(str.size()))
      {
         // Just pad out the end with zeros:
         str.append(static_cast<string::size_type>(1 + my_exp - str.size()), '0');
         if(showpoint || fixed)
            str.append(".");
      }
      else if(my_exp + 1 < static_cast<boost::intmax_t>(str.size()))
      {
         if(my_exp < 0)
         {
            str.insert(static_cast<string::size_type>(0), static_cast<string::size_type>(-1 - my_exp), '0');
            str.insert(static_cast<string::size_type>(0), "0.");
         }
         else
         {
            // Insert the decimal point:
            str.insert(static_cast<string::size_type>(my_exp + 1), 1, '.');
         }
      }
      else if(showpoint || fixed) // we have exactly the digits we require to left of the point
         str += ".0";

      if(fixed && !skipZero)
      {
         // We may need to add trailing zeros:
         boost::intmax_t l = str.find('.') + 1;
         l = digits - (str.size() - l);
         if(l > 0)
            str.append(size_type(l), '0');
      }
   }
   else
   {
      BOOST_MP_USING_ABS
      // Scientific format:
      if(showpoint || (str.size() > 1))
         str.insert(static_cast<string::size_type>(1u), 1, '.');
      str.append(static_cast<string::size_type>(1u), 'e');
		S e = boost::lexical_cast<array<char, 16>>(abs(my_exp)).c_array();
		if(e.size() < BOOST_MP_MIN_EXPONENT_DIGITS)
         e.insert(static_cast<string::size_type>(0), BOOST_MP_MIN_EXPONENT_DIGITS - e.size(), '0');
      if(my_exp < 0)
         e.insert(static_cast<string::size_type>(0), 1, '-');
      else
         e.insert(static_cast<string::size_type>(0), 1, '+');
      str.append(e);
   }
   if(neg)
      str.insert(static_cast<string::size_type>(0), 1, '-');
   else if(showpos)
      str.insert(static_cast<string::size_type>(0), 1, '+');
}

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
		isMP //&& !boost::is_same<rType, T>::value
	>::type* = 0> 
	T conv() const { return T(backend()); } // PERSONAL TODO: As usual need to go over all conv()'s for validity, improvements, etc.

	// Precision lossy set.
	// WARNING: A set of a double/float with the value 'INFINITY' will cause a silent death of the engine!
	// Reproduction: BADMP(INFINITY)
	template <typename T> rType& lossy(const T& inRhs) { backend() = canonical_value(inRhs); return *this; }

	// PERSONAL IMPROVE: Memory usage should probably be tracked for optimizing mpfloat size/etc.
	void GetMemoryUsage(class ::ICrySizer* pSizer)		  const { /*nothing*/ };
	void GetMemoryStatistics(class ::ICrySizer* pSizer)  const { /*nothing*/ };

	// PERSONAL NOTE: Previous attempts at overloads/etc.
	/*
		// Float implicitly converted to numBackend, so this bypasses the rest of the setup.
			// explicit rType(const numBackend& n) { *this = n; }

		// Delete is 'declared' and matches operator overloads, so we have to do SFINAE	
			// template <typename T> rType(T) = delete; 

		// Templated constructors with default arguments will lose the default-argument portion on inheritance.
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
				(is_integral<V>::value)
				//&& !is_convertible<typename detail::canonical<V, Backend>::type, Backend>::value		PERSONAL DEBUG: Need test cases!
		>::type* = 0>
		ILINE newNum(const V& v)
		{
			backend() = canonical_value(v);
		}
		template <class V, typename enable_if_c<
				(is_string<V>::value)
				//&& !is_convertible<typename detail::canonical<V, Backend>::type, Backend>::value
		>::type* = 0>
		ILINE newNum(const V& v)
		{
			// Assert here for debug help. PERSONAL TODO: Make it neater, or remove it entirely? :/ ....
			// This line would would make have 0 count as nullptr, have to do "0" instead.
			//		const CTimeValue fSmoothTime = m_playbackOptions.smoothTimelineSlider ? "0.2" : 0;
			assert(v != nullptr);
			backend() = canonical_value(v);
		}

	// Assignment
		template <class V>
		ILINE typename enable_if_c<valid_check<V>::value && !is_string<V>::value, rType& >::type
			operator=(const V& v)
		{
			backend() = canonical_value(v);
			return *this;
		}
		template <class V>
		ILINE typename enable_if_c<valid_check<V>::value && is_string<V>::value, rType& >::type
			operator=(const V& v)
		{
			assert(v != nullptr);
			backend() = canonical_value(v);
			return *this;
		}

		/*
			PERSONAL IMPROVE: HACK!!!
			This 'fix' for un-initialized mpfloat heap only works when ran before the data is accessed/compared.
			Might memory leak? etc. Trace this down and preferably remove.

			Also used to show an 'Unused' mpfloat value in physics etc.!
		*/
		void memHACK(){ backend().unsafe()[0]._mp_d = 0; }

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

		// Increment/Deincrement
		ILINE rType& operator++()   { using default_ops::eval_increment; eval_increment(m_backend); return *this; }
		ILINE rType& operator--()   { using default_ops::eval_decrement; eval_decrement(m_backend); return *this; }
		ILINE rType operator++(int) { using default_ops::eval_increment; rType temp(*this); eval_increment(m_backend); return BOOST_MP_MOVE(temp); }
		ILINE rType operator--(int) { using default_ops::eval_decrement; rType temp(*this); eval_decrement(m_backend); return BOOST_MP_MOVE(temp);}

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

	// Conversion operator
	inline explicit operator bool()			  const { return !is_zero(); }
	inline explicit operator rType()			  const { return *static_cast<const rType*>(this); }
	template <class T, typename boost::enable_if_c< 
		boost::is_arithmetic<T>::value || is_string<T>::value // Convert only to string/float/int
	>::type* = 0> explicit operator T() const { return this->template convert_to<T>(); }

	// Misc. funcs
	inline bool is_zero() const { using default_ops::eval_is_zero; return eval_is_zero(m_backend); }
	inline int sign()	    const { using default_ops::eval_get_sign; return eval_get_sign(m_backend); }

	/* STR conversion test: PERSONAL DEBUG, ofc implement a unit test system for this with mpfloat and what not!

		// New string function
		#define TFormat(x,y) tmp.str(x, y)

		// Classic boost str conversion
		//#define TFormat(x,y) string(tmp.backend().str(x, y).c_str())
		mpfloat tmp("000654321.12345600");
		CryLog("TESTING STR: %s",   TFormat(0,0));
		CryLog("TESTING STR: %s",   TFormat(8,0));
		CryLog("TESTING STR: %s\n", TFormat(3,0));

		CryLog("TESTING STR: %s",   TFormat(0, std::ios_base::showpos));
		CryLog("TESTING STR: %s",   TFormat(0, std::ios_base::showpoint));
		CryLog("TESTING STR: %s",   TFormat(0, std::ios_base::fixed));
		CryLog("TESTING STR: %s\n", TFormat(0, std::ios_base::dec));

		CryLog("TESTING STR: %s",   TFormat(3, std::ios_base::showpos));
		CryLog("TESTING STR: %s",   TFormat(3, std::ios_base::showpoint));
		CryLog("TESTING STR: %s",   TFormat(3, std::ios_base::fixed));
		CryLog("TESTING STR: %s\n", TFormat(3, std::ios_base::dec));

		CryLog("TESTING STR: %s",   TFormat(9, std::ios_base::showpos));
		CryLog("TESTING STR: %s",   TFormat(9, std::ios_base::showpoint));
		CryLog("TESTING STR: %s",   TFormat(9, std::ios_base::fixed));
		CryLog("TESTING STR: %s\n", TFormat(9, std::ios_base::dec));
		#undef TFormat

		// RESULTS
			<11:14:37> TESTING STR: 654321.123456
			<11:14:37> TESTING STR: 654321.123456
			<11:14:37> TESTING STR: 654321.123

			<11:14:37> TESTING STR: +654321.123456
			<11:14:37> TESTING STR: 654321.1234560000
			<11:14:37> TESTING STR: 654321.0000000000000000
			<11:14:37> TESTING STR: 654321.0						<-- Instead of just .

			<11:14:37> TESTING STR: +6.54e+05
			<11:14:37> TESTING STR: 6.54e+05
			<11:14:37> TESTING STR: 654321.123
			<11:14:37> TESTING STR: 654321.123

			<11:14:37> TESTING STR: +654321.123
			<11:14:37> TESTING STR: 654321.123
			<11:14:37> TESTING STR: 654321.123456000
			<11:14:37> TESTING STR: 654321.123456			<-- No leading zeroes! more readable.
	*/

	// Near-copy of Boosts's implementation. Uses CryTek's string instead of std::string with an extra skipZero flag convenience.
	string str(std::streamsize digits = 0, std::ios_base::fmtflags f = 0) const 
	{
		auto m_data = backend().data();
		BOOST_ASSERT(m_data[0]._mp_d);

		if (digits != 0 && f == 0) { f = std::ios_base::dec; }		// Default precision is "Digits after decimal point". Don't pad zeroes.

		bool scientific = (f & std::ios_base::scientific) == std::ios_base::scientific;
		bool skipZero = (f & std::ios_base::dec) == std::ios_base::dec;
		bool fixed = ((f & std::ios_base::fixed) == std::ios_base::fixed || skipZero); // SkipZero implies fixed.
		std::streamsize org_digits(digits);

		if (scientific && digits)
			++digits;

		string result;
		mp_exp_t e;
		void *(*alloc_func_ptr) (size_t);
		void *(*realloc_func_ptr) (void *, size_t, size_t);
		void(*free_func_ptr) (void *, size_t);
		mp_get_memory_functions(&alloc_func_ptr, &realloc_func_ptr, &free_func_ptr);

		if (mpf_sgn(m_data) == 0)
		{
			e = 0;
			result = "0";
			if (fixed && digits)
				++digits;
		}
		else
		{
			char* ps = mpf_get_str(0, &e, 10, static_cast<std::size_t>(digits), m_data);
			--e;  // To match with what our formatter expects.
			if (fixed && e != -1)
			{
				// Oops we actually need a different number of digits to what we asked for:
				(*free_func_ptr)((void*)ps, std::strlen(ps) + 1);
				digits += e + 1;
				if (digits == 0)
				{
					// We need to get *all* the digits and then possibly round up,
					// we end up with either "0" or "1" as the result.
					ps = mpf_get_str(0, &e, 10, 0, m_data);
					--e;
					unsigned offset = *ps == '-' ? 1 : 0;
					if (ps[offset] > '5')
					{
						++e;
						ps[offset] = '1';
						ps[offset + 1] = 0;
					}
					else if (ps[offset] == '5')
					{
						unsigned i = offset + 1;
						bool round_up = false;
						while (ps[i] != 0)
						{
							if (ps[i] != '0')
							{
								round_up = true;
								break;
							}
						}
						if (round_up)
						{
							++e;
							ps[offset] = '1';
							ps[offset + 1] = 0;
						}
						else
						{
							ps[offset] = '0';
							ps[offset + 1] = 0;
						}
					}
					else
					{
						ps[offset] = '0';
						ps[offset + 1] = 0;
					}
				}
				else if (digits > 0)
				{
					ps = mpf_get_str(0, &e, 10, static_cast<std::size_t>(digits), m_data);
					--e;  // To match with what our formatter expects.
				}
				else
				{
					ps = mpf_get_str(0, &e, 10, 1, m_data);
					--e;
					unsigned offset = *ps == '-' ? 1 : 0;
					ps[offset] = '0';
					ps[offset + 1] = 0;
				}
			}
			result = ps;
			(*free_func_ptr)((void*)ps, std::strlen(ps) + 1);
		}
		format_MP_string(result, e, org_digits, f, mpf_sgn(m_data) == 0);
		return result;
	}

	// For Natvis special visualizer, to see mpfloat value during debug.
	ILINE std::string debugStr() const { return std::string(str(0, 0)); }

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
	void convert_to_imp(string* result) const
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

	// template<typename T> ILINE T mod(T a, T b)			{ return a - trunc(a / b) * b; }
	template <class rType> ILINE rType mod(const newNum<rType>& a, const newNum<rType>& b) { return (rType)(a - trunc(a / b) * b); }

// More, setups for pre-define ones in boost etc.
	#define UNARY_OP_FUNCTOR(func)\
		template <class rType> \
		ILINE rType func(const newNum<rType>& arg)\
		{\
			rType result;\
			using default_ops::BOOST_JOIN(eval_,func);\
			BOOST_JOIN(eval_,func)(result.backend(), arg.backend());\
			return BOOST_MP_MOVE(result);\
		};

		UNARY_OP_FUNCTOR(abs)
		UNARY_OP_FUNCTOR(trunc)
		UNARY_OP_FUNCTOR(sqrt)
		UNARY_OP_FUNCTOR(floor)
		UNARY_OP_FUNCTOR(ceil)
		UNARY_OP_FUNCTOR(sin)
		UNARY_OP_FUNCTOR(log10)
		UNARY_OP_FUNCTOR(log)
	#undef UNARY_OP_FUNCTOR

	#define BINARY_OP_FUNCTOR(func)\
		template <class rType> \
		ILINE rType func(const newNum<rType>& a, const newNum<rType>& b)\
		{\
			rType result;\
			using default_ops::BOOST_JOIN(eval_,func);\
			BOOST_JOIN(eval_,func)(result.backend(), a.backend(), b.backend());\
			return BOOST_MP_MOVE(result);\
		};
		BINARY_OP_FUNCTOR(pow)
	#undef BINARY_OP_FUNCTOR
}} // END boost::multiprecision namespaces

// TypeInfo for mpfloat
	/* Was Split in CryTypeInfo.inl and TypeInfo_decl.h. Didn't work.
		BASIC_TYPE_INFO(mpfloat);
		TYPE_INFO_BASIC(mpfloat);

		// mpfloat
		string ToString(mpfloat const& val)				{ return val.str(); }
		bool FromString(mpfloat& val, const char* s) { val = mpfloat(s); return true; }
	*/

	MPOnly string ToString(T const& val)			 { return val.str(); }
	MPOnly bool FromString(T& val, const char* s) { val = T(s); return true; }

	//! PERSONAL IMPROVE: 
	// Due to 'clamp_tpl not defined' etc. issues when including CryCustomTypes with all of its math setups......
   // Have to define a duplicate TypeInfo class that functions just like TTypeInfo to avoid header inclusion loops.
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
// So a standerdized include-macro-header is the solution for now.
#define MP_FUNCTION(T) decl_mp_type(T)
	#include "mpfloat.types"
#undef MP_FUNCTION

// Just to get debugStr() accessible in debugger memory regardless of whether it was called in the scope or not!
template<class T> std::string IgnoreThis_MPSTUFF(const boost::multiprecision::newNum<T>& in) { return in.debugStr(); };

// String buffer size for MPFloats. PERSONAL DEBUG & PERSONAL IMPROVE!!!
#define MP_SIZE 256u

// Any precision-losing casts should be done like this.
#define BADF       (float)
#define BADMP(x)   mpfloat().lossy(x)
#define MP_EPSILON BADMP(FLT_EPSILON)  // PERSONAL IMPROVE: Epsilons in relation to mpfloat/ctimevalue are probably not needed in most cases. Hard to remove for now without debugging.

#endif // MPFLOAT