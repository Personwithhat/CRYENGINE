// Copyright 2001-2017 Crytek GmbH / Crytek Group. All rights reserved. 

#pragma once

#include "MPFloat.h"	 //! MPfloat definitions
#include "XML\IXml.h" //! For serialization

/* PERSONAL VERIFY: 
		2) Add ILINE after profiling/testing etc.
		3) Increase/decrease standard mpfloat precision (ATM 50 digits)
			
	Edit Fixes:
		1) Double check any int64() round/conversions in edits.
			If done in mpfloat mod would cause errors, e.g. 3/2 => int64(1.5) = 2 1.5 - 2 = -.5 which is wrong!
		2) Make sure all 'bad' casts use BADF etc.
		3) Fix any abs() usage, abs(CTimeValue) works as expected now.
		4) Function params should take and return by const-reference both mpfloats & CTimeValue's. (Where applicable/possible)
		5) Normalized time, and other 'time' values that can't be viewed in Seconds/etc., should be stored as the strong-typedef'd mpfloats.
			See below, rTime, nTime, kTime etc.
		6) Does min(max(dt, 0), "0.08") differ from CLAMP(dt, 0, "0.08")?
		7) All CTimeValue(int) need to have units verified as in 'seconds'. Before rewrite, you could do CTimeValue(int milliseconds).
			Also: GetMilliSeconds() usage is wrong typically, causes inaccurate results etc.!!!
			Used a lot in AI to sync events....which, for AI simulation (especially when it can run several steps/frame too) is very inaccurate.
			E.g. pathfinding, aggro change/accuracy, ugh. Same with GetMicroseconds/etc., atm it's used as a 'more accurate value' when MPFloat is better.
				Heck, review all the .Get() and .Set() usage in all the code. Most of it should be self-explanatory.....
		8) Need to update comparisons from CTimeValue > CTimeValue("0.2") to CTimeValue > "0.2" (for readability)
		9) cry_random should use the below CTimeValue() setup, if accuracy is needed later on this can be changed but abstraction is better.
		10) FPS, X per Seconds etc. should be an rTime value!!!
		11) Don't forget to update CrySleep/etc. usage everywhere!!!! nearly missed some in AI!!!!
		12) Cleanup const CTimeValue& t = CTimeValue() in default function param's, not needed! can just do = 0 etc.

	Tests:
		Need operator/conversion tests for CTimeValue (as with mpfloat). Need to also update mpfloat tests.
		Every operator/function needs to have associated tests alogn with stuff that 'should not be allowed'.
		Verify accuracy of all CTimeValue/mpfloat math funcs, e.g. trunc() proper result and so on.
*/

// Any precision-losing casts should be done like this.
class CTimeValue;
#define BADTIME(x) CTimeValue(BADMP(x))
#define BADrT(x)   rTime().lossy(x)
#define BADnT(x)   nTime().lossy(x)
#define TV_EPSILON (CTimeValue(MP_EPSILON))

// Otherwise rTime * mpfloat would auto-convert to rTime * CTimeValue.....so instead of rTime it would return nTime result.........
#define limit2(x, y) template <class T, class T2, typename boost::enable_if_c<boost::is_same<T, x>::value && boost::is_same<T2, y>::value>::type* = 0>

/*
	Animation time data for frames and what not.
*/
struct SAnimData {
public:
	SAnimData() { assert(false && "This is a static data/helper struct, do not build!"); }

	// ?????????
	enum class EDisplayMode
	{
		Ticks = 0,
		Time,
		Timecode,
		Frames
	};

	//! List of possible frame rates (dividers of 6000). Most commonly used ones first.
	enum EFrameRate
	{
		// Common
		eFrameRate_30fps, eFrameRate_60fps, eFrameRate_120fps,

		// Possible
		eFrameRate_10fps, eFrameRate_12fps, eFrameRate_15fps, eFrameRate_24fps,
		eFrameRate_25fps, eFrameRate_40fps, eFrameRate_48fps, eFrameRate_50fps,
		eFrameRate_75fps, eFrameRate_80fps, eFrameRate_100fps, eFrameRate_125fps,
		eFrameRate_150fps, eFrameRate_200fps, eFrameRate_240fps, eFrameRate_250fps,
		eFrameRate_300fps, eFrameRate_375fps, eFrameRate_400fps, eFrameRate_500fps,
		eFrameRate_600fps, eFrameRate_750fps, eFrameRate_1000fps, eFrameRate_1200fps,
		eFrameRate_1500fps, eFrameRate_2000fps, eFrameRate_3000fps, eFrameRate_6000fps,

		eFrameRate_Num
	};
	
	// ?????????
	struct Settings
	{
		EDisplayMode displayMode;
		EFrameRate   fps;

		Settings()
			: displayMode(EDisplayMode::Ticks)
			, fps(EFrameRate::eFrameRate_30fps)
		{}
	};

public:
	// Harcoded animation update-value. PERSONAL VERIFY: Does this HAVE to be 6000??
	// Why not use CPU ticks per second here from CTimer???
	static const int32 numTicksPerSecond = 6000;

	// Enum to int or string value
	static uint GetFrameRateValue(EFrameRate frameRate)
	{
		const uint frameRateValues[eFrameRate_Num] =
		{
			// Common
			30,   60,   120,

			// Possible
			10,   12,   15,  24,    25,   40,  48,  50,  75,  80,  100, 125,
			150,  200,  240, 250,   300,  375, 400, 500, 600, 750,
			1000, 1200, 1500,2000,  3000, 6000
		};

		return frameRateValues[frameRate];
	}
	static const char* GetFrameRateName(EFrameRate frameRate)
	{
		const char* frameRateNames[eFrameRate_Num] =
		{
			// Common
			"30 fps",   "60 fps",   "120 fps",

			// Possible
			"10 fps",   "12 fps",   "15 fps",  "24 fps",
			"25 fps",   "40 fps",   "48 fps",  "50 fps",
			"75 fps",   "80 fps",   "100 fps", "125 fps",
			"150 fps",  "200 fps",  "240 fps", "250 fps",
			"300 fps",  "375 fps",  "400 fps", "500 fps",
			"600 fps",  "750 fps",  "1000 fps","1200 fps",
			"1500 fps", "2000 fps", "3000 fps","6000 fps"
		};

		return frameRateNames[frameRate];
	}
};

/*
	Main time-value class. Default storage is in 'Seconds', the least accurate get/set.
	Uses GMP's mpfloat for storage, preserves up to 50 digit precision accuracy.
*/
class CTimeValue
{
public:
//**
//** Misc. constructors
//** 
	ILINE constexpr CTimeValue() {};
	ILINE constexpr CTimeValue(const CTimeValue& inValue) : m_lValue(inValue.m_lValue) {};
	~CTimeValue() {};

	// Accepts anything mpfloat accepts.
	template <class T, typename boost::enable_if_c<
		boost::is_convertible<T, mpfloat>::value
	>::type* = 0>
	ILINE CTimeValue(const T& seconds) { m_lValue = seconds; }

//**
//** Setters
//** 
	//#define rMilli mpfloat("0.001")	  // 1/1'000 
	//#define rMicro mpfloat("0.000001")  // 1/1'000'000

	CTimeValue& SetSeconds(const mpfloat& infSec) { m_lValue = infSec; return *this; }
	CTimeValue& SetMilliSeconds(const mpfloat& indwMilliSec) { m_lValue = indwMilliSec * mpfloat("0.001");    return *this; }
	CTimeValue& SetMicroSeconds(const mpfloat& indwMicroSec) { m_lValue = indwMicroSec * mpfloat("0.000001"); return *this; }

	// PERSONAL VERIFY: Unsure how to implement multi-threaded timevalues....
	ILINE void AddValueThreadSafe(const CTimeValue& val) { /*CryInterlockedAdd(&m_value, val);*/ }

//**
//** Getters
//** 
	// Ideally, anything with precision loss uses this to fetch seconds. (Or other 'BAD' nomenclature)
	float BADGetSeconds() const { return (float)m_lValue; }

	mpfloat GetSeconds()		  const { return m_lValue; }
	mpfloat GetMilliSeconds() const { return m_lValue * 1'000; }
	mpfloat GetMicroSeconds() const { return m_lValue * 1'000'000; }

	string str()				  const { return m_lValue.str(); }

	/* PERSONAL VERIFY: 
			1) Need to review Max()/Min() setups, might have misplaced them!
			2) Perhaps convert this to constexpr or whatever....find the right way.
			3) If this min/max setup is correct, from SAnimTime max/min's.....leave as is or change limits?
	*/
	static CTimeValue Min() { return CTimeValue(std::numeric_limits<int32>::min()); }
	static CTimeValue Max() { return CTimeValue(std::numeric_limits<int32>::max()); }

//**
//** Math Operations - CTimeValue & CTimeValue
//** 
	//! Assignment operator.
	CTimeValue& operator=(const CTimeValue& inRhs) { m_lValue = inRhs.m_lValue; return *this; }

	//! Prevent assignment, CTimeValue X = nonTimeValue;
	//! Use .SetSeconds() etc. instead to maintain clear units (milliseconds etc.)
	template<typename T> CTimeValue& operator=(const T& inRhs) = delete;

	CTimeValue operator-()								 const { CTimeValue ret; ret.m_lValue = -m_lValue; return ret; }

	CTimeValue operator-(const CTimeValue& inRhs) const { CTimeValue ret; ret.m_lValue = m_lValue - inRhs.m_lValue; return ret; }
	CTimeValue operator+(const CTimeValue& inRhs) const { CTimeValue ret; ret.m_lValue = m_lValue + inRhs.m_lValue; return ret;  }
	CTimeValue operator%(const CTimeValue& inRhs) const { CTimeValue ret; ret.m_lValue = mod(m_lValue, inRhs.m_lValue); return ret; }

	CTimeValue& operator+=(const CTimeValue& inRhs) { m_lValue += inRhs.m_lValue; return *this; }
	CTimeValue& operator-=(const CTimeValue& inRhs) { m_lValue -= inRhs.m_lValue; return *this; }

	TVOnly mpfloat operator*(const T& inRhs)  const { return m_lValue * inRhs.m_lValue; }					   //! Time * Time = float (For convenience)
	TVOnly nTime   operator/(const T& inRhs)  const { return (m_lValue / inRhs.m_lValue).conv<nTime>(); }	//! Time / Time = normalized time

	// Time vs Time comparisons ----------------------------
	bool operator<(const CTimeValue& inRhs)  const { return m_lValue < inRhs.m_lValue;  }
	bool operator>(const CTimeValue& inRhs)  const { return m_lValue > inRhs.m_lValue;  }
	bool operator>=(const CTimeValue& inRhs) const { return m_lValue >= inRhs.m_lValue; }
	bool operator<=(const CTimeValue& inRhs) const { return m_lValue <= inRhs.m_lValue; }
	bool operator==(const CTimeValue& inRhs) const { return m_lValue == inRhs.m_lValue; }
	bool operator!=(const CTimeValue& inRhs) const { return m_lValue != inRhs.m_lValue; }

//**
//** Math Operations - CTimeValue & T
//** 
	// Note: 1/CTimeValue = rTime (strong type-def'd mpfloat)
	CTimeValue& operator/=(const mpfloat& inRhs) { m_lValue /= inRhs; return *this; }
	CTimeValue& operator*=(const mpfloat& inRhs) { m_lValue *= inRhs; return *this; }
	CTimeValue  operator/(const mpfloat& inRhs)  const { CTimeValue ret; ret.m_lValue = m_lValue / inRhs; return ret; }
	CTimeValue  operator*(const mpfloat& inRhs)  const { CTimeValue ret; ret.m_lValue = m_lValue * inRhs; return ret; }
	friend rTime operator/(const mpfloat& inLhs, const CTimeValue& inRhs)		{ return (inLhs / inRhs.m_lValue).conv<rTime>(); }
	friend CTimeValue operator*(const mpfloat& inLhs, const CTimeValue& inRhs) { CTimeValue ret; ret.m_lValue = inLhs * inRhs.m_lValue; return ret; }

	// rTime * Time = Time/Time = mpfloat, not ntime. Typically rTime is used for a rate, e.g. frames/second which would translate to 'mpfloat of Frames' rather than 'nTime of Frames'
	AcceptOnly(rTime) mpfloat operator*(const T& inRhs)  const { mpfloat ret; ret = m_lValue * inRhs.conv<mpfloat>(); return ret; }
	limit2(rTime, CTimeValue) friend mpfloat operator*(const T& inLhs, const T2& inRhs) { mpfloat ret; ret = inLhs.conv<mpfloat>() * inRhs.m_lValue; return ret; }

	// nTime * Time = Time
	AcceptOnly(nTime) CTimeValue  operator*(const T& inRhs)  const { CTimeValue ret; ret.m_lValue = m_lValue * inRhs.conv<mpfloat>(); return ret; }
	limit2(nTime, CTimeValue) friend CTimeValue operator*(const T& inLhs, const T2& inRhs) { CTimeValue ret; ret.m_lValue = inLhs.conv<mpfloat>() * inRhs.m_lValue; return ret; }

//**
//** Snapping time to frame multiple	PERSONAL TODO: Messy location, find a way to make this all look neater.
//** 
	// Return time snapped to nearest multiple of given frame rate
	CTimeValue SnapToNearest(const SAnimData::EFrameRate frameRate) const { return SnapToNearest(SAnimData::GetFrameRateValue(frameRate)); }
	CTimeValue SnapToNearest(const uint frameRate)						 const;

	// Return time snapped to next multiple of given frame rate		 
	// (Will NOT change when already at a multiple)
	CTimeValue SnapToNext(const SAnimData::EFrameRate frameRate)	 const { return SnapToNext(SAnimData::GetFrameRateValue(frameRate)); }
	CTimeValue SnapToNext(const uint frameRate)							 const;

	// Return time snapped to previous multiple of given frame rate 
	// (Will NOT change when already at a multiple) 
	CTimeValue SnapToPrev(const SAnimData::EFrameRate frameRate)	 const { return SnapToPrev(SAnimData::GetFrameRateValue(frameRate)); }
	CTimeValue SnapToPrev(const uint frameRate)							 const;

	// Return time after stepping to next multiple of given frame rate 
	// (Will change to next multiple when already at a multiple)
	CTimeValue StepToNext(const SAnimData::EFrameRate frameRate)	 const { return StepToNext(SAnimData::GetFrameRateValue(frameRate)); }
	CTimeValue StepToNext(const uint frameRate)							 const { return (*this + "0.0001").SnapToNext(frameRate); }

	// Return time after stepping to previous multiple of given frame rate 
	// (Will change to previous multiple when already at a multiple)
	CTimeValue StepToPrev(const SAnimData::EFrameRate frameRate)	 const { return StepToPrev(SAnimData::GetFrameRateValue(frameRate));  }
	CTimeValue StepToPrev(const uint frameRate)							 const { return (*this - "0.0001").SnapToPrev(frameRate); }

//**
//** Miscellaneous other functions. (Perhaps move to getters/setters? Xd)
//**
	// PERSONAL VERIFY: Memory usage should probably be tracked for optimizing mpfloat size/etc.
	void GetMemoryUsage(class ICrySizer* pSizer)		  const { /*Nothing*/ }
	void GetMemoryStatistics(class ICrySizer* pSizer) const { /*Nothing*/ }

	// See mpfloat func description
	void memHACK(){ m_lValue.memHACK(); }

	// PERSONAL VERIFY: That this is correct!
	//! Useful for periodic events (e.g. water wave, blinking).
	//! Changing TimePeriod can results in heavy changes in the returned value.
	//! \return [0..1]
	mpfloat GetPeriodicFraction(const CTimeValue& TimePeriod) const { return (*this % TimePeriod).GetSeconds(); }

	// SAnimTime-style serialization, for XMLNodeRef's.
	void Serialize(XmlNodeRef keyNode, bool bLoading, const char* pName, const char* pLegacyName = "")
	{
		if (bLoading)
		{
			if (!keyNode->getAttr(pName, m_lValue))
			{
				/*
				// Backwards compatibility
				float fTime = 0.0f;
				keyNode->getAttr(pLegacyName, time);
				m_lValue = BADMP(fTime);
				*/
				assert(false && "Should not be using legacy names anymore!");
			}
		}
		else {
			keyNode->setAttr(pName, m_lValue);
		}
	}

	// PERSONAL VERIFY: Where should this go again? >.>
	AUTO_STRUCT_INFO;

// PERSONAL VERIFY: Setting up Volatile CTimeValue() => no def's for mpfloat volatile = and == , so this fails.
// Tricky to implement....postponed for now but threading won't work properly otherwise!
/*public: //! For GeomCacheRenderNode.h/cpp 
	CTimeValue(const volatile CTimeValue& inValue)
	{
		m_lValue = inValue.m_lValue;
	}

	mpfloat GetSeconds() const volatile
	{
		return *const_cast<const mpfloat*>(&m_lValue);
	}

	//! Volatile assignment
	volatile CTimeValue& SetVol(const CTimeValue& inRhs) volatile
	{
		m_lValue = inRhs.m_lValue;
		return *this;
	};
	volatile CTimeValue& SetVol(const volatile CTimeValue& inRhs) volatile
	{
		m_lValue = inRhs.m_lValue;
		return *this;
	};

	bool operator==(const volatile CTimeValue& inRhs) const volatile { return m_lValue == inRhs.m_lValue; };
*/

public:
	mpfloat m_lValue;     //!< Time in Seconds. Storage limited to 'least-accurate number', in this case 'Seconds'.
};

//** 
//** Other operators
//** 
	// PERSONAL TODO: Will currently only work with mpfloat/rTime. int/rTime will have ambiguous overloads!!
	// 1/rTime == CTimeValue() 
	AcceptOnly(rTime) ILINE CTimeValue operator/(const mpfloat& inLhs, const T& inRhs) { CTimeValue ret; ret = CTimeValue(inLhs / inRhs.conv<mpfloat>()); return ret; }

	// PERSONAL TODO:
	// Actually, mpfloat * any other strong-type should have the other strong-type convert to mpfloat (to avoid weird operators e.g. rTime*rTime), 
	// do the operation, then convert back. e.g. Mpfloat is a mere ratio/etc. on other values.......
	// Should also apply to some other types like nTime? Ofc don't allow interchangable implicit conversions for function calls. Just for * and / etc. ??

	// mpfloat * rT == rT  SAME APPLIES FOR nTime
	AcceptOnly(rTime) ILINE rTime operator*(const mpfloat& inLhs, const T& inRhs) { mpfloat ret; ret = inLhs * inRhs.conv<mpfloat>(); return ret.conv<rTime>(); }
	AcceptOnly(mpfloat) ILINE rTime operator*(const rTime& inLhs, const T& inRhs) { mpfloat ret; ret = inLhs.conv<mpfloat>() * inRhs; return ret.conv<rTime>(); }

	AcceptOnly(rTime) ILINE rTime operator*(const nTime& inLhs, const T& inRhs) { nTime ret; ret = inLhs * inRhs.conv<nTime>(); return ret.conv<rTime>(); }
	AcceptOnly(nTime) ILINE rTime operator*(const rTime& inLhs, const T& inRhs) { nTime ret; ret = inLhs.conv<nTime>() * inRhs; return ret.conv<rTime>(); }

	// limit2(nTime, CTimeValue) friend CTimeValue operator*(const T& inLhs, const T2& inRhs) { CTimeValue ret; ret.m_lValue = inLhs.conv<mpfloat>() * inRhs.m_lValue; return ret; }

//**
//**	Various math functions
//**
	ILINE CTimeValue ceil(const CTimeValue& time) { return CTimeValue(int_ceil(time.GetSeconds())); }

	ILINE CTimeValue abs(const CTimeValue& time) { return (time >= CTimeValue(0)) ? time : -time; }

	// PERSONAL VERIFY: This is ofc not optimized....used during inrange() Fix this etc.
	ILINE int32 isneg(const CTimeValue& time) { return (time < 0) ? 1 : 0; }
	ILINE int32 sgn(const CTimeValue& time)   { return time.GetSeconds().sign(); } // sign returns -1 if negative, 0 if zero, 1 if positive

	// ILINE float div_min(float n, float d, float m) { return n * d < m * d * d ? n / d : m; }
	ILINE CTimeValue div_min(const CTimeValue& n, const CTimeValue& d, const CTimeValue& m) { 
			return div_min(n.GetSeconds(), d.GetSeconds(), m.GetSeconds());
	}

	// Check if CTimeValue is in a valid range etc.
	template<> inline bool IsValid(const CTimeValue& val) { return true; }

//**
//**	Snapping, defined after abs etc. functions .-.
//**
	ILINE CTimeValue CTimeValue::SnapToNearest(const uint frameRate) const
	{
		const int sign = sgn(*this);
		const CTimeValue absT = abs(*this);

		const CTimeValue framesMod = CTimeValue(1) / frameRate;
		const CTimeValue remainder = absT % framesMod;
		const bool bNextMultiple = remainder >= (framesMod / 2);
		return sign * ((absT - remainder) + (bNextMultiple ? framesMod : 0));
	}
	ILINE CTimeValue CTimeValue::SnapToNext(const uint frameRate) const
	{
		const CTimeValue framesMod = CTimeValue(1) / frameRate;
		const CTimeValue remainder = *this % framesMod;
		const bool bNextMultiple = (*this >= 0 && remainder != 0);
		return (*this - remainder) + (bNextMultiple ? framesMod : 0);
	}
	ILINE CTimeValue CTimeValue::SnapToPrev(const uint frameRate) const
	{
		const CTimeValue framesMod = CTimeValue(1) / frameRate;
		const CTimeValue remainder = *this % framesMod;
		const bool bPrevMultiple = (*this < 0 && remainder != 0);
		return (*this - remainder) - (bPrevMultiple ? framesMod : 0);
	}

#undef limit2