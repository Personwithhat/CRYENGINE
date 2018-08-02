// Copyright 2001-2017 Crytek GmbH / Crytek Group. All rights reserved. 

#pragma once
#include <CrySystem/ITimer.h>

// Implements all common timing routines
class CTimer : public ITimer
{
public:
	// Constructor
	CTimer();
	// Destructor
	~CTimer() {};

	bool Init();

	// TODO: Review m_time usage in System.cpp / SystemRender.cpp
	//       if it wants Game Time / UI Time or a new Render Time?


	// PERSONAL VERIFY LIST: CURRENTLY USING CE 5.5, PREVIEW 1 AS THE BASE!!!
		// 3) Need to do a last check for:
				// No accuracy loss
				// CTimeValue Math operation consistency
				// Cleanup of pointless functions/conversions
				// Variable names, e.g. m_fFrameTime is no longer a float! remove 'f'
		// 4) Compare to Timer.cpp (5.5) to verify functionality.

	// Basically UpdateOnFrameStart() with a few differences (see 'Notes')
	// No Sleep, no smoothing regardless of cvar, and cumulates frameTime instead of replacing it.
	// PERSONAL VERIFY: Perhaps move this to ITimer.h and standardize?
	 void	 UpdatePostRollback();

	// Interface ITimer ----------------------------------------------------------
		// Lifecycle
			 ITimer* CreateNewTimer();
			 void  ResetTimer();
			 void  Serialize(TSerialize ser);

			 void  UpdateOnFrameStart();
			 bool	 SetTimer(ETimer which, const CTimeValue& timeInSeconds);

			 bool	 PauseTimer(ETimer which, bool bPause);
			 bool	 IsTimerPaused(ETimer which);

			 void  EnableTimer(const bool bEnable);
			 bool  IsTimerEnabled() const;

		// Time getters
			 CTimeValue GetFrameTime(bool ignorePause = false) const;
			 CTimeValue GetRealFrameTime() const;

			 const CTimeValue& GetFrameStartTime(ETimer which = ETIMER_GAME) const { return m_CurrTime[(int)which]; }
			 const CTimeValue& GetRealStartTime()    const { return m_realStartTime; }
			 const CTimeValue& GetAverageFrameTime() const { return m_prevAvgFrameTime; }
			 const CTimeValue& GetReplicationTime()  const { return m_replicationTime; };

			 CTimeValue	GetAsyncTime() const;
			 CTimeValue GetAsyncCurTime() const;

		// Timescales
			 mpfloat     GetTimeScale() const;
			 mpfloat     GetTimeScale(uint32 channel) const;
			 void        ClearTimeScales();
			 void        SetTimeScale(const mpfloat& scale, uint32 channel = 0);

		// Other misc.
			 CTimeValue  TicksToTime(int64 ticks) const { return CTimeValue(ticks / (mpfloat)m_lTicksPerSec); }
			 int64		 GetTicksPerSecond() const { return m_lTicksPerSec; }

			 rTime       GetFrameRate();
			 mpfloat     GetProfileFrameBlending(CTimeValue* pfBlendTime = 0, int* piBlendMode = 0); // DEBUG ONLY

			 void			 SecondsToDateUTC(time_t time, struct tm& outDateUTC);
			 time_t		 DateToSecondsUTC(struct tm& timePtr);
	// ~ Interface ITimer---------------------------------------------------------------------

private: 
	// Update profile-frame-blend time (smooth time). DEBUG ONLY
	void  UpdateBlending();

	// Updates CTimeValue, param = real-time in ticks.
	void  RefreshGameTime(int64 ticks);
	void  RefreshUITime(int64 ticks);

	// Update simulation frame-time average
	void UpdateAverageFrameTime();

	// Updates the game-time offset to match the the specified time.
	// The argument is the new number of ticks since the last Reset().
	void SetOffsetToMatchGameTime(int64 ticks);

private:

	enum
	{
		NUM_TIME_SCALE_CHANNELS = 8,
	};

	//////////////////////////////////////////////////////////////////////////
	// Dynamic state, reset by ResetTimer()
	//////////////////////////////////////////////////////////////////////////
	CTimeValue m_CurrTime[ETIMER_LAST]; // Real (UI) and Simulation (Game) time since last Reset(), cached during UpdateOnFrameStart()
	CTimeValue m_realStartTime;			// Absolute real-time at frame start, basically GetAsyncTime(). Cached during UpdateOnFrameStart()
	
	int64      m_lBaseTime;   // Absolute time (in ticks) when timer was last Reset()
	int64      m_lLastTime;   // Ticks since last Reset(). This is the base for UI time. UI time is monotonic, it always moves forward at a constant rate until the timer is Reset().
	int64      m_lOffsetTime; // Additional ticks for Game time (relative to UI time). Game time can be affected by loading, pausing, time smoothing and time clamping, as well as SetTimer(). PERSONAL VERIFY: NOPE! See comments in timer.cpp on ACTUAL game-time, not just game-frame-time...
	
	CTimeValue m_replicationTime; // Sum of all frame times used in replication
	CTimeValue m_fFrameTime;      // Simulation time since the last Update(), clamped/smoothed etc.
	CTimeValue m_fRealFrameTime;  // Real time since the last Update(), no clamping/smoothing/pausing etc.

	bool       m_bGameTimerPaused;     // Set if the game is paused. GetFrameTime() will return 0, GetFrameStartTime(ETIMER_GAME) will not progress.
	int64      m_lGameTimerPausedTime; // The UI time (in ticks) when the game timer was paused. On un-pause, offset will be adjusted to match.


	//////////////////////////////////////////////////////////////////////////
	// Persistant state, kept by ResetTimer()
	//////////////////////////////////////////////////////////////////////////
	bool         m_bEnabled;		//!< Timer enabled/disabled
	unsigned int m_nFrameCounter; //!< Frame counter, atm only used for TicksPerSec.
	int64			 m_lTicksPerSec;  //!< CPU ticks per second, updated occassionally (approx. every 127 frames) in case of rate changes.

	// Frame averaging
	CTimeValue m_prevAvgFrameTime;			//!< The simulation frame-time averaged over a time period. (e.g. 0.25 seconds)
	std::vector<CTimeValue> m_frameTimes;	//!< Unsmoothed simulation frame-time list. [0] = Most recent.

	// Time scales
	mpfloat m_timeScaleChannels[NUM_TIME_SCALE_CHANNELS];
	mpfloat m_totalTimeScale;


	//////////////////////////////////////////////////////////////////////////
	// Console vars, always have default value on secondary CTimer instances
	//////////////////////////////////////////////////////////////////////////
	int   m_TimeSmoothing;			// Console Variable, 0=off, otherwise on
	mpfloat m_cvar_time_scale;		// Slow down time cvar.

	CTimeValue m_fixed_time_step;	// If negative, real frame time has a minimum duration of fixed_time_step, enforced via sleep.
	CTimeValue m_max_time_step;	// Max simulation frame time, clamping.
	CTimeValue average_interval;  // Frame-averaging time period.


	/*------------== DEBUG ONLY ==------------*/
	int   m_TimeDebug;			 // Console Variable, 0=off, 1=events, 2=verbose

	// Profile time smoothing (Persistent state)
	mpfloat m_fProfileBlend;	 // Current blending amount for profile.
	CTimeValue m_fAvgFrameTime; // Used for blend weighting (UpdateBlending())
	CTimeValue m_fSmoothTime;   // Smoothing interval (up to m_profile_smooth_time).

	// Profile averaging help.
	int   m_profile_weighting;	 // Weighting mode (see RegisterVar desc).

	// Time to exponentially smooth profile results.
	CTimeValue m_profile_smooth_time;  
};
