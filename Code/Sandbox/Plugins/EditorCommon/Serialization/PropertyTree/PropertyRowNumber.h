/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "PropertyTree.h"
#include <CrySerialization/yasli/MemoryWriter.h>
#include <CrySerialization/yasli/decorators/Range.h>
#include "PropertyRowNumberField.h"

#include <limits>
#include <float.h>
#include <math.h>

template<class T>
yasli::string numberAsString(T value)
{
	yasli::MemoryWriter buf;
	buf << value;
	return buf.c_str();
}

inline long long stringToSignedInteger(const char* str)
{
	long long value;
#ifdef _MSC_VER
	value = _atoi64(str);
#else
    char* endptr = (char*)str;
    value = strtoll(str, &endptr, 10);
#endif
	return value;
}

inline unsigned long long stringToUnsignedInteger(const char* str)
{
	unsigned long long value;
	if (*str == '-') {
		value = 0;
	}
	else {
#ifdef _MSC_VER
		char* endptr = (char*)str;
		value = _strtoui64(str, &endptr, 10);
#else
        char* endptr = (char*)str;
		value = strtoull(str, &endptr, 10);
#endif
	}
	return value;
}

template<class Output, class Input>
Output clamp(Input value, Output min, Output max)
{
	if (value < Input(min))
		return min;
	if (value > Input(max))
		return max;
	return Output(value);
}

template<class T> T limit_min(T) { return std::numeric_limits<T>::min(); }
template<class T> T limit_max(T) { return std::numeric_limits<T>::max(); }
template<class Out, class In> void clampToType(Out* out, In value) { *out = clamp(value, limit_min((Out)value), limit_max((Out)value)); }

bool isDigit(int ch);

double parseFloat(const char* s);

inline void clampedNumberFromString(char* value, const char* str)					{ clampToType(value, stringToSignedInteger(str)); }
inline void clampedNumberFromString(signed char* value, const char* str)		{ clampToType(value, stringToSignedInteger(str)); }
inline void clampedNumberFromString(short* value, const char* str)				{ clampToType(value, stringToSignedInteger(str)); }
inline void clampedNumberFromString(int* value, const char* str)					{ clampToType(value, stringToSignedInteger(str)); }
inline void clampedNumberFromString(long* value, const char* str)					{ clampToType(value, stringToSignedInteger(str)); }
inline void clampedNumberFromString(long long* value, const char* str)			{ clampToType(value, stringToSignedInteger(str)); }
inline void clampedNumberFromString(unsigned char* value, const char* str)			{ clampToType(value, stringToUnsignedInteger(str)); }
inline void clampedNumberFromString(unsigned short* value, const char* str)		{ clampToType(value, stringToUnsignedInteger(str)); }
inline void clampedNumberFromString(unsigned int* value, const char* str)			{ clampToType(value, stringToUnsignedInteger(str)); }
inline void clampedNumberFromString(unsigned long* value, const char* str)			{ clampToType(value, stringToUnsignedInteger(str)); }
inline void clampedNumberFromString(unsigned long long* value, const char* str)	{ clampToType(value, stringToUnsignedInteger(str)); }
inline void clampedNumberFromString(float* value, const char* str)
{
	double v = parseFloat(str);
	if (v > FLT_MAX)
		v = FLT_MAX;
	if (v < -FLT_MAX)
		v = -FLT_MAX;
	*value = float(v);
}
inline void clampedNumberFromString(double* value, const char* str)
{
	*value = parseFloat(str);
}


inline void clampedNumberFromString(CTimeValue* value, const char* str)
{
	*value = CTimeValue(str);
}
#define MP_FUNCTION(T)\
inline void clampedNumberFromString(T* value, const char* str)\
{\
	*value = T(str);\
}
#include <CrySystem\mpfloat.types>
#undef MP_FUNCTION

/*
	PERSONAL TODO: 
		All these edits in this file, really messy.
		Good starting point for rewrites & multi-type compatibility with floats/mpfloat/etc. as the underlying basis t-t
*/
template<class Type>
class PropertyRowNumber : public PropertyRowNumberField{
public:
	PropertyRowNumber()
	{
		hardLimit_.min = std::numeric_limits<Type>::min();
		hardLimit_.max = std::numeric_limits<Type>::max();
		softLimit_ = hardLimit_;
		singleStep_ = yasli::DefaultSinglestep<Type>();
	}

	void setValue(Type value, const void* handle, const yasli::TypeID& type) {
		value_ = value;
		serializer_.setPointer((void*)handle);
		serializer_.setType(type);
	}
	bool setValueFromString(const char* str) override{
        Type value = value_;
		  clampedNumberFromString(&value_, str);
        return value_ != value;
	}
	yasli::string valueAsString() const override{ 
        return numberAsString(value_);
	}

	bool assignToPrimitive(void* object, size_t size) const override{
		*reinterpret_cast<Type*>(object) = value_;
		return true;
	}

	void setValueAndContext(const yasli::Serializer& ser, yasli::Archive& ar) override {
		yasli::RangeDecorator<Type>* range = (yasli::RangeDecorator<Type>*)ser.pointer();
		serializer_.setPointer((void*)range->value);
		serializer_.setType(yasli::TypeID::get<Type>());
		value_ = *range->value;
		hardLimit_.min = range->hardMin;
		hardLimit_.max = range->hardMax;
		softLimit_.min = range->softMin;
		softLimit_.max = range->softMax;
		singleStep_ = range->singleStep;
	}

  bool assignTo(const yasli::Serializer& ser) const override 
	{
		if (ser.type() == yasli::TypeID::get<yasli::RangeDecorator<Type>>()) {
			yasli::RangeDecorator<Type>* range = (yasli::RangeDecorator<Type>*)ser.pointer();
			*range->value = value_;
		}
		else if (ser.type() == yasli::TypeID::get<Type>()) {
			*(Type*)ser.pointer() = value_;
		}			
    return true;
	}

	void serializeValue(yasli::Archive& ar) override{
		ar(value_, "value", "Value");
		ar(hardLimit_.min, "hardMin", "HardMin");
		ar(hardLimit_.max, "hardMax", "HardMax");
		ar(softLimit_.min, "softMin", "SoftMin");
		ar(softLimit_.max, "softMax", "SoftMax");
	}


public: // Incrementation
	void startIncrement() override
	{
		incrementStartValue_ = value_;
	}
	void endIncrement(PropertyTree* tree) override
	{
		if (value_ != incrementStartValue_) {
			Type value = value_;
			value_ = incrementStartValue_;
			value_ = value;
			tree->model()->rowChanged(this, true);
		}
	}

	// Convert from mpfloat TO lossy/other-strong types.
	template <class T = Type, typename boost::enable_if_c<isMP>::type* = 0>
	T convType(const mpfloat & val){
		return val.conv<T>();
	}
	template <class T = Type, typename boost::enable_if_c<!isMP>::type* = 0>
	T convType(const mpfloat & val) {
		return (T)val;
	}

	// Convert to mpfloat FROM lossy/other-strong types.
	template <class T, typename boost::enable_if_c<isMP>::type* = 0>
	mpfloat convTo(const T& val) {
		return val.conv<mpfloat>();
	}
	template <class T, typename boost::enable_if_c<!isMP>::type* = 0>
	mpfloat convTo(const T& val) {
		return mpfloat().lossy(val);
	}

	void incrementLog(const mpfloat& screenFraction, const mpfloat& valueFieldFraction) override
	{
		mpfloat screenFractionMultiplier = 1000;
		if (yasli::TypeID::get<Type>() == yasli::TypeID::get<float>() ||
			 	yasli::TypeID::get<Type>() == yasli::TypeID::get<double>()) 
			screenFractionMultiplier = 10;

		mpfloat incValue = convTo(incrementStartValue_);

		mpfloat startPower = log10(abs(incValue) + 1) - 3;
		mpfloat power = startPower + abs(screenFraction) * 10;
		mpfloat delta = pow<mpfloat>(10, power) - pow<mpfloat>(10, startPower) + screenFractionMultiplier * abs(screenFraction);
		mpfloat newValue;
		if (screenFraction > 0)
			newValue = incValue + delta;
		else
			newValue = incValue - delta;

		/*if (!IsValid(newValue)) {
			if (screenFraction > 0)
				newValue = std::numeric_limits<Type>::max();
			else
				newValue = std::numeric_limits<Type>::mmin();
		}*/

		value_ = CLAMP(convType(newValue), softLimit_.min, softLimit_.max);
	}

	void increment(PropertyTree* tree, int mouseDiff, Modifier modifier) override
	{
		mpfloat add = 0;
		if (softLimit_.min != std::numeric_limits<Type>::min() && softLimit_.max != std::numeric_limits<Type>::max())
		{
			add = convTo(softLimit_.max - softLimit_.min) * mouseDiff / widgetRect(tree).width();
		}
		else
		{
			add = mpfloat("0.1") * mouseDiff;
		}
		
		if (MODIFIER_CONTROL == modifier)
		{
			add *= "0.1";
		}
		else if (MODIFIER_SHIFT == modifier)
		{
			add *= 100;
		}

		value_ = CLAMP(value_ + convType(add), softLimit_.min, softLimit_.max);
	}

public: // More incrementation
	void add(PropertyTree* tree) override { addValue(tree, singleStep_);  }
	void sub(PropertyTree* tree) override { subValue(tree, singleStep_); }

	// PERSONAL NOTE: Has to be separate, can't pass in negative value since property might be Unsigned e.g. u64!
	void subValue(PropertyTree* tree, const Type& value)
	{
		tree->model()->rowAboutToBeChanged(this);
		value_ = CLAMP(value_ - value, hardLimit_.min, hardLimit_.max);
		tree->model()->rowChanged(this, true);
	}
	void addValue(PropertyTree* tree, const Type& value)
	{
		tree->model()->rowAboutToBeChanged(this);
		value_ = CLAMP(value_ + value, hardLimit_.min, hardLimit_.max);
		tree->model()->rowChanged(this, true);
	}

public: // Limit settings
	bool hasMinMax() const {
		return hardLimit_.min != std::numeric_limits<Type>::min() && hardLimit_.max != std::numeric_limits<Type>::max();
	}

	Type minValue() const { return hasMinMax() ? hardLimit_.min : std::numeric_limits<Type>::min(); }
	Type maxValue() const { return hasMinMax() ? hardLimit_.max : std::numeric_limits<Type>::max(); }

	// PERSONAL TODO: Until Q-related stuff has increased accuracy, limited to doubles......
	double minValueD() const override { return (double)minValue(); } 
	double maxValueD() const override { return (double)maxValue(); }

protected:
	Type incrementStartValue_; 
	Type value_; 
	struct SLimits { Type min, max; };

	SLimits hardLimit_; 	// Enforced limit that the property can absolutely not exceed
	SLimits softLimit_; 	// Soft limit enforced by UI sliders etc, should be lower than the hard limit

	Type singleStep_;	// Increment/de-increment step
};

