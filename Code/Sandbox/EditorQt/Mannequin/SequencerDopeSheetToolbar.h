// Copyright 2001-2018 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

class CSequencerDopeSheetToolbar;

#include "Controls/DlgBars.h"
#include "SequencerDopeSheetBase.h"

class CSequencerDopeSheetToolbar : public CDlgToolBar
{

public:
	CSequencerDopeSheetToolbar();
	virtual ~CSequencerDopeSheetToolbar();
	virtual void InitToolbar();
	virtual void SetTime(const CTimeValue& fTime, const rTime& fFps);

protected:
	CStatic m_timeWindow;
	CTimeValue m_lastTime;

};
