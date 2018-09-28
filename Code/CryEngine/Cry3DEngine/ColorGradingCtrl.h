// Copyright 2001-2019 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include <Cry3DEngine/IColorGradingCtrl.h>
#include <vector>

struct IRenderView;
struct ITexture;

class CColorGradingTextureLoader
{
public:
	struct STextureToLoad
	{
		ITexture*  pTexture;
		CTimeValue timeToFade;
		string     texturePath;
	};

	// Command to wait till next frame
	void LoadTexture(const string& colorGradingTexturePath, const CTimeValue& timeToFade);
	void Reset();

	// Get valid textures from last frame request
	std::vector<STextureToLoad> Update();

private:
	// Double buffering: one frame delay requires to process texture load
	std::vector<STextureToLoad> m_thisFrameRequest;
	std::vector<STextureToLoad> m_prevFrameRequest;
};

class CColorGradingCtrl : public IColorGradingCtrl
{
public:
	void Init();
	void Reset();

	// Prepares RenderView for rendering
	void UpdateRenderView(IRenderView& renderView, const CTimeValue& timeSinceLastFrame);

	void Serialize(TSerialize& ar) override;

private:
	virtual void SetColorGradingLut(const char* szTexture, const CTimeValue& timeToFade) override;

	void ProcessNewCharts();
	void Update(const CTimeValue& timeSinceLastFrame);
	void FadeOutOtherLayers();
	void RemoveFadedOutLayers();
	void FillRenderView(IRenderView& renderView);

	struct SColorChart
	{
		SColorChart(int texID, float blendAmount, const CTimeValue& timeToFadeInInSeconds, const string& texturePath);

		void FadeIn(const CTimeValue& timeSinceLastFrame);
		void FadeOut(float blendAmountOfFadingInGradient);

		int    texID;
		float  blendAmount;
		CTimeValue  timeToFade;
		CTimeValue  elapsedTime;
		float  maximumBlendAmount;
		string texturePath;
	};

	CColorGradingTextureLoader m_textureLoader;
	std::vector<SColorChart>   m_charts;
};
