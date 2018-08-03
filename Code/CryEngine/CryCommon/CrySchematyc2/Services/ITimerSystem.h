// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

// #SchematycTODO : Use scoped connections to automatically disconnect timers?
// #SchematycTODO : Use combination of pools and seeds for more efficient look-up by id?

#pragma once

#include "CrySchematyc2/Prerequisites.h"

#include <CrySchematyc2/TemplateUtils/TemplateUtils_ArrayView.h>
#include <CrySchematyc2/TemplateUtils/TemplateUtils_Delegate.h>
#include <CrySchematyc2/TemplateUtils/TemplateUtils_PreprocessorUtils.h>

#include "CrySchematyc2/BasicTypes.h"
#include "CrySchematyc2/Utils/StringUtils.h"

namespace Schematyc2
{
	enum class ETimerUnits
	{
		Invalid,
		Frames,
		Seconds,
		Random
	};

	struct STimerDuration
	{
		// PERSONAL VERIFY: These 2 should probably be improved/cleaned up, default const/etc. is implicitly deleted after CTimeValue edits....
		~STimerDuration(){};
		STimerDuration& operator=(const STimerDuration& rhs)
		{
			units = rhs.units;
			frames = rhs.frames;
			range.min = rhs.range.min;
			range.max = rhs.range.max;
			return *this;
		}

		// PERSONAL NOTE: For same reason as in StreamEngine.cpp, mpfloat/CTimeValue can't be memset.
		inline STimerDuration()
			: units(ETimerUnits::Invalid)
		{
		}

		// PERSONAL VERIFY: Changed Frame setup to specific function call, conflicts with CTimeValue if in construction.
		// Does this conflict with how schematyc handles/uses/sets up STimerDuration?
		inline STimerDuration& Frames(uint32 _frames)
		{
			*this = STimerDuration();
			units = ETimerUnits::Frames;
			frames = _frames;
			return *this;
		}

		inline STimerDuration(const CTimeValue& _seconds)
		{
			*this = STimerDuration();
			units   = ETimerUnits::Seconds;
			seconds = _seconds;
		}

		explicit inline STimerDuration(const CTimeValue& _min, const CTimeValue& _max)
		{
			*this = STimerDuration();
			units     = ETimerUnits::Random;
			range.min = _min;
			range.max = _max;
		}

		inline STimerDuration(const STimerDuration& rhs)
		{
			memcpy(this, &rhs, sizeof(STimerDuration));
		}

		ETimerUnits units;
		union
		{
			uint32 frames;
			CTimeValue seconds;
			struct
			{
				CTimeValue min;
				CTimeValue max;
			} range;
		};
	};

	enum class ETimerFlags
	{
		None      = 0,
		AutoStart = BIT(0),
		Repeat    = BIT(1)
	};

	DECLARE_ENUM_CLASS_FLAGS(ETimerFlags)

	struct STimerParams
	{
		inline STimerParams(const STimerDuration& _duration = STimerDuration(), ETimerFlags _flags = ETimerFlags::None)
			: duration(_duration)
			, flags(_flags)
		{}

		STimerDuration duration;
		ETimerFlags    flags;
	};

	typedef TemplateUtils::CDelegate<void ()> TimerCallback;
	typedef uint32                            TimerId;

	static const TimerId s_invalidTimerId = ~0;

	struct ITimerSystem
	{
		virtual ~ITimerSystem() {}

		virtual TimerId CreateTimer(const STimerParams& params, const TimerCallback& callback) = 0;
		virtual void DestroyTimer(TimerId timerId) = 0;
		virtual bool StartTimer(TimerId timerId) = 0;
		virtual bool StopTimer(TimerId timerId) = 0;
		virtual void ResetTimer(TimerId timerId) = 0;
		virtual bool IsTimerActive(TimerId timerId) const = 0;
		virtual STimerDuration GetTimeRemaining(TimerId timerId) const = 0;
		virtual void Update() = 0;
	};

	inline bool ToString(const STimerDuration& duration, const CharArrayView& output)
	{
		switch(duration.units)
		{
		case ETimerUnits::Frames:
			{
				return StringUtils::UInt32ToString(duration.frames, output) && StringUtils::Append("(frames)", output);
			}
		case ETimerUnits::Seconds:
			{
				return StringUtils::TimeToString(duration.seconds, output) && StringUtils::Append("(s)", output);
			}
		}
		return false;
	}
}
