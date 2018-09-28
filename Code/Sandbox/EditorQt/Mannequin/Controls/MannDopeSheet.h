// Copyright 2001-2018 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include "../SequencerDopeSheet.h"

class CMannDopeSheet : public CSequencerDopeSheet
{
	DECLARE_DYNAMIC(CMannDopeSheet)

public:
	bool IsDraggingTime() const
	{
		//--- Dammit
		return m_mouseMode == 4;
	}

	const CTimeValue& GetTime() const
	{
		return m_currentTime;
	}
};
