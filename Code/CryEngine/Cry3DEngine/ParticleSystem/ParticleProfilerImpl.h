// Copyright 2001-2017 Crytek GmbH / Crytek Group. All rights reserved. 

namespace pfx2
{

#if !defined(_RELEASE)

ILINE void CParticleProfiler::AddEntry(CParticleComponentRuntime* pRuntime, EProfileStat type, uint value)
{
	if (!IsEnabled())
		return;
	const uint32 threadId = JobManager::GetWorkerThreadId();
	SEntry entry;
	entry.m_pRuntime = pRuntime;
	entry.m_type = type;
	entry.m_value = value;
	m_entries[threadId + 1].push_back(entry);
}

ILINE CTimeProfiler::CTimeProfiler(CParticleProfiler& profiler, CParticleComponentRuntime* pRuntime, EProfileStat stat)
	: m_profiler(profiler)
	, m_pRuntime(pRuntime)
	, m_stat(stat)
	, m_startTicks(CryGetTicks())
{
}

ILINE CTimeProfiler::~CTimeProfiler()
{
	const int64 endTicks = CryGetTicks();
	// Float inaccuracy is fine, debug/profiling:   Although this can perhaps be improved.
	const uint time = uint(gEnv->pTimer->TicksToTime(endTicks - m_startTicks).GetSeconds() * 1000000);
	m_profiler.AddEntry(m_pRuntime, m_stat, time);
	m_profiler.AddEntry(m_pRuntime, EPS_TotalTiming, time);
}

#else

ILINE void CParticleProfiler::AddEntry(CParticleComponentRuntime* pRuntime, EProfileStat type, uint value)  {}
ILINE CTimeProfiler::CTimeProfiler(CParticleProfiler& profiler, CParticleComponentRuntime* pRuntime, EProfileStat stat) : m_profiler(profiler) {}
ILINE CTimeProfiler::~CTimeProfiler() {}

#endif

}
