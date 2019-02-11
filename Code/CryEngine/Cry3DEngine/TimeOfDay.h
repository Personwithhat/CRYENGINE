// Copyright 2001-2018 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include <Cry3DEngine/ITimeOfDay.h>
#include "EnvironmentPreset.h"
#include <CryAudio/IAudioInterfacesCommonData.h>

class CTimeOfDay : public ITimeOfDay
{
public:
	CTimeOfDay();

	// ITimeOfDay
	virtual int         GetPresetCount() const override { return m_presets.size(); }
	virtual bool        GetPresetsInfos(SPresetInfo* resultArray, unsigned int arraySize) const override;
	virtual bool        SetCurrentPreset(const char* szPresetName) override;
	virtual const char* GetCurrentPresetName() const override;
	virtual bool        AddNewPreset(const char* szPresetName) override;
	virtual bool        RemovePreset(const char* szPresetName) override;
	virtual bool        SavePreset(const char* szPresetName) const override;
	virtual bool        LoadPreset(const char* szFilePath) override;
	virtual void        ResetPreset(const char* szPresetName) override;

	virtual bool        ImportPreset(const char* szPresetName, const char* szFilePath) override;
	virtual bool        ExportPreset(const char* szPresetName, const char* szFilePath) const override;

	virtual int         GetVariableCount() override { return ITimeOfDay::PARAM_TOTAL; }
	virtual bool        GetVariableInfo(int nIndex, SVariableInfo& varInfo) override;
	virtual void        SetVariableValue(int nIndex, float fValue[3]) override;
	virtual bool        InterpolateVarInRange(int nIndex, const CTimeValue& fMin, const CTimeValue& fMax, unsigned int nCount, Vec3* resultArray) const override;
	virtual uint        GetSplineKeysCount(int nIndex, int nSpline) const override;
	virtual bool        GetSplineKeysForVar(int nIndex, int nSpline, SBezierKey* keysArray, unsigned int keysArraySize) const override;
	virtual bool        SetSplineKeysForVar(int nIndex, int nSpline, const SBezierKey* keysArray, unsigned int keysArraySize) override;
	virtual bool        UpdateSplineKeyForVar(int nIndex, int nSpline, const CTimeValue& fTime, float newValue) override;
	virtual int         GetAnimTimeSecondsIn24h() override;

	virtual void        ResetVariables() override;

	// Time of day is specified in hours.
	virtual void                   SetTime(const CTimeValue& fHour, bool bForceUpdate = false) override;
	virtual const CTimeValue&      GetTime() override              { return m_fTime; }

	virtual void                   SetPaused(bool paused) override { m_bPaused = paused; }

	virtual void                   SetAdvancedInfo(const SAdvancedInfo& advInfo) override;
	virtual void                   GetAdvancedInfo(SAdvancedInfo& advInfo) override;

	virtual void                   Update(bool bInterpolate = true, bool bForceUpdate = false) override;
	virtual void                   ConstantsChanged() override;

	virtual Sun&                   GetSunParams() override;
	virtual Moon&                  GetMoonParams() override;
	virtual Wind&                  GetWindParams() override;
	virtual CloudShadows&          GetCloudShadowsParams() override;
	virtual TotalIllum&            GetTotalIlluminationParams() override;
	virtual TotalIllumAdv&         GetTotalIlluminationAdvParams() override;
	virtual Serialization::SStruct GetConstantParams() override;
	virtual void                   ResetConstants(const DynArray<char>& binaryBuffer) override;

	virtual void                   Serialize(XmlNodeRef& node, bool bLoading) override;
	virtual void                   Serialize(TSerialize ser) override;

	virtual void                   NetSerialize(TSerialize ser, const CTimeValue& lag, uint32 flags) override;

	virtual void                   Tick() override;

	virtual void                   SaveInternalState(struct IDataWriteStream& writer) override;
	virtual void                   LoadInternalState(struct IDataReadStream& reader) override;

	//////////////////////////////////////////////////////////////////////////
	float GetHDRMultiplier() const { return m_fHDRMultiplier; }

	void  BeginEditMode() override { m_bEditMode = true; }
	void  EndEditMode() override   { m_bEditMode = false; }

protected:
	virtual bool RegisterListenerImpl(IListener* const pListener, const char* const szDbgName, const bool staticName) override;
	virtual void UnRegisterListenerImpl(IListener* const pListener) override;

private:
	CTimeOfDay(const CTimeOfDay&);
	CTimeOfDay(const CTimeOfDay&&);
	CTimeOfDay&    operator=(const CTimeOfDay&);
	CTimeOfDay&    operator=(const CTimeOfDay&&);

	SVariableInfo& GetVar(ETimeOfDayParamID id);
	void           UpdateEnvLighting(bool forceUpdate);
	void           NotifyOnChange(const IListener::EChangeType changeType, const char* const szPresetName);

private:
	typedef std::map<string, CEnvironmentPreset> TPresetsSet;
	typedef CListenerSet<IListener*>             TListenerSet;

private:
	TPresetsSet         m_presets;
	CEnvironmentPreset* m_pCurrentPreset;
	string              m_currentPresetName;

	SVariableInfo       m_vars[ITimeOfDay::PARAM_TOTAL];
	CTimeOfDayConstants m_consts; //Working copy

	CTimeValue          m_fTime;

	bool                m_bEditMode;
	bool                m_bPaused;

	SAdvancedInfo       m_advancedInfo;
	float               m_fHDRMultiplier;
	ICVar*              m_pTimeOfDaySpeedCVar;
	CryAudio::ControlId m_timeOfDayRtpcId;
	TListenerSet        m_listeners;
};
