// Copyright 2001-2018 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include "Common/GraphicsPipelineStage.h"
#include "Common/FullscreenPass.h"
#include "Common/UtilityPasses.h"

class CMotionBlurStage : public CGraphicsPipelineStage
{
public:
	bool IsStageActive(EShaderRenderingFlags flags) const final
	{
		return CRenderer::CV_r_MotionBlur && !gRenDev->m_nDisableTemporalEffects;
	}

	void Execute();

private:
	nTime ComputeMotionScale();

private:
	CFullscreenPass    m_passPacking;
	CFullscreenPass    m_passTileGen1;
	CFullscreenPass    m_passTileGen2;
	CFullscreenPass    m_passNeighborMax;
	CStretchRectPass   m_passCopy;
	CFullscreenPass    m_passMotionBlur;
};
