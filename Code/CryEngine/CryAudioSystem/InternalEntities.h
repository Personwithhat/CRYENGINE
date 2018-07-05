// Copyright 2001-2018 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include "ATLEntities.h"

namespace CryAudio
{
class CAudioListenerManager;
class CATLAudioObject;

struct SInternalControls
{
	using SwitchState = std::pair<ControlId const, SwitchStateId const>;

	std::map<SwitchState, IAudioSwitchStateImpl const*> m_switchStates;
	std::map<ControlId, CATLTriggerImpl const*>         m_triggerConnections;
	std::map<ControlId, IParameterImpl const*>          m_parameterConnections;
};

class COcclusionObstructionState final : public IAudioSwitchStateImpl
{
public:

	explicit COcclusionObstructionState(SwitchStateId const stateId, CAudioListenerManager const& audioListenerManager);

	// IAudioSwitchStateImpl
	virtual void Set(CATLAudioObject& audioObject) const override;
	// ~IAudioSwitchStateImpl

	COcclusionObstructionState(COcclusionObstructionState const&) = delete;
	COcclusionObstructionState(COcclusionObstructionState&&) = delete;
	COcclusionObstructionState& operator=(COcclusionObstructionState const&) = delete;
	COcclusionObstructionState& operator=(COcclusionObstructionState&&) = delete;

private:

	SwitchStateId const          m_stateId;
	CAudioListenerManager const& m_audioListenerManager;
};

class CRelativeVelocityTrackingState final : public IAudioSwitchStateImpl
{
public:

	explicit CRelativeVelocityTrackingState(SwitchStateId const stateId);

	// IAudioSwitchStateImpl
	virtual void Set(CATLAudioObject& audioObject) const override;
	// ~IAudioSwitchStateImpl

	CRelativeVelocityTrackingState(CRelativeVelocityTrackingState const&) = delete;
	CRelativeVelocityTrackingState(CRelativeVelocityTrackingState&&) = delete;
	CRelativeVelocityTrackingState& operator=(CRelativeVelocityTrackingState const&) = delete;
	CRelativeVelocityTrackingState& operator=(CRelativeVelocityTrackingState&&) = delete;

private:

	SwitchStateId const m_stateId;
};

class CAbsoluteVelocityTrackingState final : public IAudioSwitchStateImpl
{
public:

	explicit CAbsoluteVelocityTrackingState(SwitchStateId const stateId);

	// IAudioSwitchStateImpl
	virtual void Set(CATLAudioObject& audioObject) const override;
	// ~IAudioSwitchStateImpl

	CAbsoluteVelocityTrackingState(CAbsoluteVelocityTrackingState const&) = delete;
	CAbsoluteVelocityTrackingState(CAbsoluteVelocityTrackingState&&) = delete;
	CAbsoluteVelocityTrackingState& operator=(CAbsoluteVelocityTrackingState const&) = delete;
	CAbsoluteVelocityTrackingState& operator=(CAbsoluteVelocityTrackingState&&) = delete;

private:

	SwitchStateId const m_stateId;
};

class CDoNothingTrigger final : public CATLTriggerImpl
{
public:

	explicit CDoNothingTrigger(TriggerImplId const id)
		: CATLTriggerImpl(id)
	{}

	CDoNothingTrigger() = delete;

	// CATLTriggerImpl
	virtual ERequestStatus Execute(Impl::IObject* const pImplObject, Impl::IEvent* const pImplEvent) const override;
	// ~CATLTriggerImpl
};

class CLoseFocusTrigger final : public CATLTriggerImpl
{
public:

	explicit CLoseFocusTrigger(TriggerImplId const id)
		: CATLTriggerImpl(id)
	{}

	CLoseFocusTrigger() = delete;

	// CATLTriggerImpl
	virtual ERequestStatus Execute(Impl::IObject* const pImplObject, Impl::IEvent* const pImplEvent) const override;
	// ~CATLTriggerImpl
};

class CGetFocusTrigger final : public CATLTriggerImpl
{
public:

	explicit CGetFocusTrigger(TriggerImplId const id)
		: CATLTriggerImpl(id)
	{}

	CGetFocusTrigger() = delete;

	// CATLTriggerImpl
	virtual ERequestStatus Execute(Impl::IObject* const pImplObject, Impl::IEvent* const pImplEvent) const override;
	// ~CATLTriggerImpl
};

class CMuteAllTrigger final : public CATLTriggerImpl
{
public:

	explicit CMuteAllTrigger(TriggerImplId const id)
		: CATLTriggerImpl(id)
	{}

	CMuteAllTrigger() = delete;

	// CATLTriggerImpl
	virtual ERequestStatus Execute(Impl::IObject* const pImplObject, Impl::IEvent* const pImplEvent) const override;
	// ~CATLTriggerImpl
};

class CUnmuteAllTrigger final : public CATLTriggerImpl
{
public:

	explicit CUnmuteAllTrigger(TriggerImplId const id)
		: CATLTriggerImpl(id)
	{}

	CUnmuteAllTrigger() = delete;

	// CATLTriggerImpl
	virtual ERequestStatus Execute(Impl::IObject* const pImplObject, Impl::IEvent* const pImplEvent) const override;
	// ~CATLTriggerImpl
};

class CPauseAllTrigger final : public CATLTriggerImpl
{
public:

	explicit CPauseAllTrigger(TriggerImplId const id)
		: CATLTriggerImpl(id)
	{}

	CPauseAllTrigger() = delete;

	// CATLTriggerImpl
	virtual ERequestStatus Execute(Impl::IObject* const pImplObject, Impl::IEvent* const pImplEvent) const override;
	// ~CATLTriggerImpl
};

class CResumeAllTrigger final : public CATLTriggerImpl
{
public:

	explicit CResumeAllTrigger(TriggerImplId const id)
		: CATLTriggerImpl(id)
	{}

	CResumeAllTrigger() = delete;

	// CATLTriggerImpl
	virtual ERequestStatus Execute(Impl::IObject* const pImplObject, Impl::IEvent* const pImplEvent) const override;
	// ~CATLTriggerImpl
};

class CAbsoluteVelocityParameter final : public IParameterImpl
{
public:

	explicit CAbsoluteVelocityParameter() = default;

	// IParameterImpl
	virtual void Set(CATLAudioObject& audioObject, float const value) const override;
	// ~IParameterImpl
};

class CRelativeVelocityParameter final : public IParameterImpl
{
public:

	explicit CRelativeVelocityParameter() = default;

	// IParameterImpl
	virtual void Set(CATLAudioObject& audioObject, float const value) const override;
	// ~IParameterImpl
};
} // namespace CryAudio
