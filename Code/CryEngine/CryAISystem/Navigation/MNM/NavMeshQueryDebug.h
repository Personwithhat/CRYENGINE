// Copyright 2001-2018 Crytek GmbH / Crytek Group. All rights reserved.
#ifdef NAV_MESH_QUERY_DEBUG

#pragma once

#include <CryAISystem/NavigationSystem/INavMeshQueryDebug.h>

namespace MNM
{
	class CNavMeshQueryDebug final : public INavMeshQueryDebug
	{
	public:
		CNavMeshQueryDebug(const INavMeshQuery& query)
			: m_query(query)
		{
			m_data.timeAtStart = GetGTimer()->GetAsyncCurTime();
		}

		virtual  const INavMeshQuery& GetQuery() const override
		{
			return m_query;
		}

		virtual  const INavMeshQueryDebug::SQueryDebugData& GetDebugData() const override
		{
			return m_data;
		}

		virtual  bool AddBatchToHistory(const INavMeshQueryDebug::SBatchData& queryBatch) override
		{
			if (!gAIEnv.CVars.StoreNavigationQueriesHistory && !gAIEnv.CVars.DebugDrawNavigationQueriesUDR)
			{
				return false;
			}

			m_data.batchHistory.push_back(queryBatch);
			m_data.trianglesCount += queryBatch.triangleDataArray.size();
			m_data.elapsedTimeRunning += queryBatch.elapsedTime;

			const CTimeValue& now = GetGTimer()->GetAsyncCurTime();
			m_data.elapsedTimeTotal = now - m_data.timeAtStart;

			return true;
		}

		virtual  bool AddInvalidationToHistory(const INavMeshQueryDebug::SInvalidationData& queryInvalidation) override
		{
			if (!gAIEnv.CVars.StoreNavigationQueriesHistory && !gAIEnv.CVars.DebugDrawNavigationQueriesUDR)
			{
				return false;
			}

			m_data.invalidationsHistory.push_back(queryInvalidation);
			return true;
		}
	
	private:
		const INavMeshQuery&   m_query;
		SQueryDebugData        m_data;
	};
} // namespace MNM

#endif // NAV_MESH_QUERY_DEBUG