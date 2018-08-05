// Copyright 2001-2018 Crytek GmbH / Crytek Group. All rights reserved.

#ifndef __animtime_h__
#define __animtime_h__

#include <CrySystem/XML/IXml.h>
#include <CrySerialization/IArchive.h>
#include <CryMath/Cry_Math.h>

/*
PERSONAL NOTE:
	SAnimTime conflicts with CTimeValue() in usage, and has questionable accuracy/precision.
	Hence deprecated and some associated functionality moved to CTimeValue() 
*/
#ifdef USE_ANIM_TIME
struct STimeCode
{
	int32 hours;
	int32 minutes;
	int32 seconds;
	int32 frames;

	STimeCode(int32 _fps)
		: hours(0)
		, minutes(0)
		, seconds(0)
		, frames(0)
		, fps(_fps)
	{}

	void  FromTicks(int32 ticks);
	int32 ToTicks();

	void  Serialize(Serialization::IArchive& ar);

private:
	const int32 fps;
};

struct SAnimTime
{
	// NOTE: This number doesn't matter, might as well call it 'precision'.
	// Only used to save/load from m_ticks value and to archive (save/load) as ticks.....
	static const int32 numTicksPerSecond = 6000;

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

	struct Settings
	{
		EDisplayMode displayMode;
		EFrameRate   fps;

		Settings()
			: displayMode(EDisplayMode::Ticks)
			, fps(EFrameRate::eFrameRate_30fps)
		{}
	};

	inline static mpfloat do_lround(mpfloat v)
	{
		if (-0.5 < v && v < 0.5)
		{
			return 0;
		}
		else if (v > 0)
		{
			mpfloat c = ceil(v);
			return 0.5 < c - v ? c - 1 : c;
		}
		else
		{
			// see former branch
			mpfloat f = ceil(v);
			return 0.5 < v - f ? f + 1 : f;
		}
	}

	SAnimTime() : m_ticks(0) {}
	explicit SAnimTime(int32 ticks) : m_ticks(ticks) {}
	explicit SAnimTime(mpfloat time) { SetSeconds(time); }

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

	operator mpfloat()   const { return GetSeconds(); }
	mpfloat GetSeconds() const { return mpfloat(m_ticks) / numTicksPerSecond; }

	bool  Serialize(Serialization::IArchive& ar, const char* szName, const char* szLabel);

	//! Helper to serialize from ticks or old float time.
	void Serialize(XmlNodeRef keyNode, bool bLoading, const char* pName, const char* pLegacyName)
	{
		if (bLoading)
		{
			int32 ticks = 0;
			if (!keyNode->getAttr(pName, ticks))
			{
				// Backwards compatibility
				float time = 0.0f;
				keyNode->getAttr(pLegacyName, time);
				*this = SAnimTime(BADMP(time));
			}
			else
			{
				m_ticks = ticks;
			}
		}
		else if (m_ticks > 0)
		{
			keyNode->setAttr(pName, m_ticks);
		}
	}

	int32            GetTicks() const              { return m_ticks; }
	void             SetTicks(int32 ticks)         { m_ticks = ticks; }

	void             SetSeconds(mpfloat time)      { m_ticks = static_cast<int32>(do_lround(time * numTicksPerSecond)); }

	static SAnimTime Min()                         { SAnimTime minTime; minTime.m_ticks = std::numeric_limits<int32>::lowest(); return minTime; }
	static SAnimTime Max()                         { SAnimTime maxTime; maxTime.m_ticks = (std::numeric_limits<int32>::max)(); return maxTime; }

	SAnimTime        operator-() const             { return SAnimTime(-m_ticks); }
	SAnimTime        operator-(SAnimTime r) const  { SAnimTime temp = *this; temp.m_ticks -= r.m_ticks; return temp; }
	SAnimTime        operator+(SAnimTime r) const  { SAnimTime temp = *this; temp.m_ticks += r.m_ticks; return temp; }
	SAnimTime        operator*(SAnimTime r) const  { SAnimTime temp = *this; temp.m_ticks *= r.m_ticks; return temp; }
	SAnimTime        operator/(SAnimTime r) const  { SAnimTime temp; temp.m_ticks = static_cast<int32>(m_ticks * mpfloat(numTicksPerSecond) / r.m_ticks); return temp; }
	SAnimTime        operator%(SAnimTime r) const  { SAnimTime temp = *this; temp.m_ticks %= r.m_ticks; return temp; }
	SAnimTime        operator*(mpfloat r) const    { SAnimTime temp; temp.m_ticks = static_cast<int32>(do_lround(m_ticks * r)); return temp; }
	SAnimTime        operator/(mpfloat r) const    { SAnimTime temp; temp.m_ticks = static_cast<int32>(do_lround(m_ticks / r)); return temp; }
	SAnimTime        operator*(int r)	  const	  { SAnimTime temp; temp.m_ticks = static_cast<int32>(do_lround(mpfloat(m_ticks) * r)); return temp; }
	SAnimTime        operator/(int r)	  const	  { SAnimTime temp; temp.m_ticks = static_cast<int32>(do_lround(mpfloat(m_ticks) / r)); return temp; }
	SAnimTime&       operator+=(SAnimTime r)       { *this = *this + r; return *this; }
	SAnimTime&       operator-=(SAnimTime r)       { *this = *this - r; return *this; }
	SAnimTime&       operator*=(SAnimTime r)       { *this = *this * r; return *this; }
	SAnimTime&       operator/=(SAnimTime r)       { *this = *this / r; return *this; }
	SAnimTime&       operator%=(SAnimTime r)       { *this = *this % r; return *this; }
	SAnimTime&       operator*=(mpfloat r)         { *this = *this * r; return *this; }
	SAnimTime&       operator/=(mpfloat r)         { *this = *this / r; return *this; }
	SAnimTime&       operator*=(int r)				  { *this = *this * r; return *this; }
	SAnimTime&       operator/=(int r)				  { *this = *this / r; return *this; }

	bool             operator<(SAnimTime r) const  { return m_ticks < r.m_ticks;  }
	bool             operator>(SAnimTime r) const  { return m_ticks > r.m_ticks;  }
	bool             operator<=(SAnimTime r) const { return m_ticks <= r.m_ticks; }
	bool             operator>=(SAnimTime r) const { return m_ticks >= r.m_ticks; }
	bool             operator==(SAnimTime r) const { return m_ticks == r.m_ticks; }
	bool             operator!=(SAnimTime r) const { return m_ticks != r.m_ticks; }

	//! Assignment operator.
	ILINE SAnimTime& operator=(const CTimeValue& inRhs)
	{
		SetSeconds(inRhs.GetSeconds());
		return *this;
	};

	//! Copy constructor
	ILINE SAnimTime(const CTimeValue& inRhs)
	{
		SetSeconds(inRhs.GetSeconds());
	};

	//! Prevent impercise value multiplication/division, e.g. float's, double's, and so on.
	//! Use mpfloat() or integers instead.
	template<typename T> SAnimTime& operator/=(const T& inRhs) { static_assert(false, "SAnimTime /= impercise value"); }
	template<typename T> SAnimTime& operator*=(const T& inRhs) { static_assert(false, "SAnimTime *= impercise value"); }
	template<typename T> SAnimTime& operator/(const T& inRhs)  const { static_assert(false, "SAnimTime / impercise value"); }
	template<typename T> SAnimTime& operator*(const T& inRhs)  const { static_assert(false, "SAnimTime * impercise value"); }
	template<typename T> friend SAnimTime operator/(const T& inLhs, const SAnimTime& inRhs) { static_assert(false, "SAnimTime / impercise value"); }
	template<typename T> friend SAnimTime operator*(const T& inLhs, const SAnimTime& inRhs) { static_assert(false, "SAnimTime * impercise value"); }

	/**
	 * Return time snapped to nearest multiple of given frame rate
	 */
	SAnimTime SnapToNearest(const EFrameRate frameRate) const
	{
		const int sign = crymath::sign(m_ticks);
		const int32 absTicks = abs(m_ticks);

		const int framesMod = numTicksPerSecond / GetFrameRateValue(frameRate);
		const int32 remainder = absTicks % framesMod;
		const bool bNextMultiple = remainder >= (framesMod / 2);
		return SAnimTime(sign * ((absTicks - remainder) + (bNextMultiple ? framesMod : 0)));
	}

	/**
	 * Return time snapped to next multiple of given frame rate
	 * (will not change when already at a multiple)
	 */
	SAnimTime SnapToNext(const EFrameRate frameRate) const
	{
		const int framesMod = numTicksPerSecond / GetFrameRateValue(frameRate);
		const int32 remainder = m_ticks % framesMod;
		const bool bNextMultiple = (m_ticks >= 0 && remainder != 0);
		return SAnimTime((m_ticks - remainder) + (bNextMultiple ? framesMod : 0));
	}

	/**
	 * Return time snapped to previous multiple of given frame rate
	 * (will not change when already at a multiple)
	 */
	SAnimTime SnapToPrev(const EFrameRate frameRate) const
	{
		const int framesMod = numTicksPerSecond / GetFrameRateValue(frameRate);
		const int32 remainder = m_ticks % framesMod;
		const bool bPrevMultiple = (m_ticks < 0 && remainder != 0);
		return SAnimTime((m_ticks - remainder) - (bPrevMultiple ? framesMod : 0));
	}

	/**
	 * Return time after stepping to next multiple of given frame rate
	 * (will change to next multiple when already at a multiple)
	 */
	SAnimTime StepToNext(const EFrameRate frameRate) const
	{
		return SAnimTime(m_ticks + 1).SnapToNext(frameRate);
	}

	/**
	 * Return time after stepping to previous multiple of given frame rate
	 * (will change to previous multiple when already at a multiple)
	 */
	SAnimTime StepToPrev(const EFrameRate frameRate) const
	{
		return SAnimTime(m_ticks - 1).SnapToPrev(frameRate);
	}

private:
	int32 m_ticks;

	friend bool         Serialize(Serialization::IArchive& ar, SAnimTime& animTime, const char* name, const char* label);

	//static inline int64 round(const double v)
	//{
	//	return v < 0.0 ? static_cast<int64>(ceil(v - 0.5)) : static_cast<int64>(floor(v + 0.5));
	//}
};

// Copy/Assignment operators SAnimTime <=> CTimeValue
ILINE CTimeValue& CTimeValue::operator=(const SAnimTime& inRhs) {  SetSeconds(inRhs.GetSeconds()); return *this; };
ILINE CTimeValue::CTimeValue(const SAnimTime& inValue) { SetSeconds(inValue.GetSeconds()); }

inline void STimeCode::FromTicks(int32 ticks)
{
	const int32 ticksPerFrame = SAnimTime::numTicksPerSecond / fps;

	seconds = ticks / SAnimTime::numTicksPerSecond;
	frames = (ticks - SAnimTime::numTicksPerSecond * seconds) / ticksPerFrame;
	minutes = seconds / 60;
	seconds -= minutes * 60;
}

inline int32 STimeCode::ToTicks()
{
	const int32 tps = SAnimTime::numTicksPerSecond;
	const int32 ticksPerFrame = SAnimTime::numTicksPerSecond / fps;
	return minutes * 60 * tps + seconds * tps + frames * ticksPerFrame;
}

inline void STimeCode::Serialize(Serialization::IArchive& ar)
{
	ar(minutes, "m", "^");
	ar.doc("Minutes");
	ar(seconds, "s", "^");
	ar.doc("Seconds");
	ar(frames, "f", "^");
	ar.doc("Frames");
}

inline bool SAnimTime::Serialize(Serialization::IArchive& ar, const char* szName, const char* szLabel)
{
	if (!ar.isEdit())
	{
		return ar(m_ticks, szName, szLabel);
	}
	else
	{
		Settings context;
		if (const Settings* pContext = ar.context<const Settings>())
		{
			context = *pContext;
		}

		const int32 fps = GetFrameRateValue(context.fps);
		if (ar.isInput())
		{
			switch (context.displayMode)
			{
			default:
			case EDisplayMode::Ticks:
				{
					ar(m_ticks, "tick", szLabel);
				}
				break;
			case EDisplayMode::Timecode:
				{
					STimeCode timeCode(fps);
					ar(timeCode, "timecode", szLabel);
					m_ticks = timeCode.ToTicks();
				}
				break;
			case EDisplayMode::Time:
				{
					mpfloat seconds = 0;
					ar(seconds, "sec", szLabel);
					SetSeconds(seconds);
				}
				break;
			case EDisplayMode::Frames:
				{
					const int32 ticksPerFrame = numTicksPerSecond / fps;
					int32 frames = 0;
					ar(frames, "frames", szLabel);
					SetTicks(frames * ticksPerFrame);
				}
				break;
			}
		}
		else
		{
			string str;
			switch (context.displayMode)
			{
			default:
			case EDisplayMode::Ticks:
				{
					ar(m_ticks, "tick", szLabel);
				}
				break;
			case EDisplayMode::Timecode:
				{
					STimeCode timeCode(fps);
					timeCode.FromTicks(m_ticks);
					ar(timeCode, "timecode", szLabel);
				}
				break;
			case EDisplayMode::Time:
				{
					mpfloat seconds = GetSeconds();
					ar(seconds, "sec", szLabel);
				}
				break;
			case EDisplayMode::Frames:
				{
					const int32 ticksPerFrame = numTicksPerSecond / fps;
					int32 frames = GetTicks() / ticksPerFrame;
					ar(frames, "frames", szLabel);
				}
				break;
			}
		}

		return true;
	}
}

inline bool Serialize(Serialization::IArchive& ar, SAnimTime& animTime, const char* szName, const char* szLabel)
{
	return animTime.Serialize(ar, szName, szLabel);
}

inline SAnimTime abs(SAnimTime time)
{
	return (time >= SAnimTime(0)) ? time : -time;
}


#endif

#endif
