// Copyright 2001-2018 Crytek GmbH / Crytek Group. All rights reserved.

#ifndef __SERVERTIMER_H__
#define __SERVERTIMER_H__

#pragma once

// PERSONAL VERIFY: Considering the usage of frame-time, this is a very messy class.......
// Most of these func's are empty, there's really only 3 gets + 1 update function!
// Also, edited clamping setup a bit to include CVar to clamp game systems instead of harcoded 0.25 + no minimum 0.0001 frame time anymore.
class CServerTimer : public ITimer
{
public:
	static ITimer* Get() { return &m_this; }

	// Interface ITimer ----------------------------------------------------------
		// Lifecycle
			 ITimer* CreateNewTimer()							{ return new CServerTimer(); }		//<<
			 void  ResetTimer()									{ CRY_ASSERT(false); }
			 void  Serialize(TSerialize ser)					{ CRY_ASSERT(false); }

			 // PERSONAL VERIFY: Why is this timer updated on CActionGame::Update() instead of when the other timer is updated???
			 void  UpdateOnFrameStart();																		//<<
			 bool	 SetTimer(ETimer which, const CTimeValue& timeInSeconds){ return false; }

			 bool	 PauseTimer(ETimer which, bool bPause) { assert(false); return false; }
			 bool	 IsTimerPaused(ETimer which)				{ return false; }


			 void  EnableTimer(const bool bEnable)			{ CRY_ASSERT(false); }
			 bool  IsTimerEnabled() const						{ return true; }

		// Time getters
			 CTimeValue GetFrameTime(bool ignorePause = false) const { return m_frameTime; } //<<
			 CTimeValue GetRealFrameTime() const { return gEnv->pTimer->GetRealFrameTime(); }

			 const CTimeValue& GetFrameStartTime(ETimer which = ETIMER_GAME) const { return m_remoteFrameStartTime; } //<<
			 const CTimeValue& GetRealStartTime()    const { assert(false); return bogusTime; }
			 const CTimeValue& GetAverageFrameTime() const { assert(false); return bogusTime; }
			 const CTimeValue& GetReplicationTime()  const { return m_replicationTime; };						//<<
			 const CTimeValue  GetServerTime()		  const { return gEnv->pTimer->GetServerTime(); };

			 CTimeValue	GetAsyncTime()		const { return gEnv->pTimer->GetAsyncTime();	  }
			 CTimeValue GetAsyncCurTime() const { return gEnv->pTimer->GetAsyncCurTime(); }

		// Timescales
			 mpfloat     GetTimeScale() const										{ return 1; }
			 mpfloat     GetTimeScale(uint32 channel) const						{ return 1; }
			 void        ClearTimeScales()											{};
			 void        SetTimeScale(const mpfloat& scale, uint32 channel = 0)	{};

		// Other misc.
			 CTimeValue  TicksToTime(int64 ticks) const { return gEnv->pTimer->TicksToTime(ticks); }
			 int64		 GetTicksPerSecond()		  const { return gEnv->pTimer->GetTicksPerSecond(); }

			 rTime       GetFrameRate()						{ return gEnv->pTimer->GetFrameRate(); }
			 mpfloat     GetProfileFrameBlending(CTimeValue* pfBlendTime = 0, int* piBlendMode = 0) { return 1; }

			 void			 SecondsToDateUTC(time_t time, struct tm& outDateUTC) { gEnv->pTimer->SecondsToDateUTC(time, outDateUTC); };
			 time_t		 DateToSecondsUTC(struct tm& timePtr)						{ return gEnv->pTimer->DateToSecondsUTC(timePtr); }
	// ~ Interface ITimer---------------------------------------------------------------------

private:
	CServerTimer();

	CTimeValue		bogusTime;

	CTimeValue     m_remoteFrameStartTime;
	CTimeValue     m_frameTime;
	CTimeValue		m_replicationTime;

	static CServerTimer m_this;
};

#endif
