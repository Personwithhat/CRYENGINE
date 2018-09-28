// Copyright 2001-2019 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include "Utils.h"
#include <CurveEditorContent.h>

#include <CrySandbox/CrySignal.h>

#include <memory>

class CEnvironmentEditor;

enum class PlaybackMode
{
	Edit,
	Play,
};

// Controls interaction between "Editor", and (not all the time existed) "VariablesTab", "CurveEditorTab", and "Playback" part of Environment preset
// Any of the windows, calls a method of this controller, and it emits signals.
// If the signal is handled, it will update it's window state.
// IT IS EXPECTED, while the signal is handled, nothing will be emitted again {widget.blockSignals(true); widget.setText(); widget.blockSignals(false);}
class CController : public IAutoEditorNotifyListener
{
public:
	CController(CEnvironmentEditor& editor);

	void                 OnOpenAsset();
	void                 OnCloseAsset();

	void                 CopySelectedCurveToClipboard();
	void                 PasteCurveContentFromClipboard();

	void                 OnVariableSelected(int id);
	void                 InterpolateVarTreeChanges(STodParameter& param, const Vec3& oldValue);

	void                 OnSelectedVariableStartChange();
	void                 OnCurveDragging();
	void                 OnCurveEditorEndChange();
	void                 OnVariableTreeEndChange();

	void                 UndoVariableChange(ITimeOfDay::IPreset* pPreset, int paramId, bool undo, const DynArray<char>& undoState, DynArray<char>& redoState);
	void                 RedoVariableChange(ITimeOfDay::IPreset* pPreset, int paramId, const DynArray<char>& redoState);

	void                 SetCurrentTime(QWidget* pSender, const CTimeValue& time);

	void                 GetEnginePlaybackParams(CTimeValue& startTime, CTimeValue& endTime, mpfloat& speed) const;
	CTimeValue           GetCurrentTime() const;
	// User changed AndvancedInfo. Send it to the Engine
	void                 SetEnginePlaybackParams(const CTimeValue& startTime, const CTimeValue& endTime, const mpfloat& speed) const;

	PlaybackMode         GetPlaybackMode() const;
	void                 TogglePlaybackMode();
	void                 AnimateTime();

	CEnvironmentEditor&  GetEditor()                      { return m_editor; }
	STodVariablesState&  GetVariables()                   { return m_variables; }
	int                  GetSelectedVariableIndex() const { return m_selectedVariableIndex; }
	SCurveEditorContent& GetCurveContent()                { return m_selectedVariableContent; }

	CCrySignal<void()>                             signalAssetOpened;
	CCrySignal<void()>                             signalAssetClosed;

	CCrySignal<void()>                             signalNewVariableSelected;
	CCrySignal<void()>                             signalVariableTreeChanged;
	CCrySignal<void()>                             signalCurveContentChanged;

	CCrySignal<void(QWidget* pSender, CTimeValue time)> signalCurrentTimeChanged;
	CCrySignal<void(PlaybackMode newMod)>				signalPlaybackModeChanged;
	CCrySignal<void(QEvent* event)>						signalHandleKeyEventsInVarPropertyTree;

private:
	virtual void OnEditorNotifyEvent(EEditorNotifyEvent event) override;
	void         RebuildVariableTreeFromPreset(bool newPreset = false);
	void         RebuildCurveContentFromPreset();
	void         ApplyVariableChangeToPreset(int paramId, SCurveEditorContent& content);

	CEnvironmentEditor&                   m_editor;
	PlaybackMode                          m_playbackMode;

	STodVariablesState                    m_variables;
	int                                   m_selectedVariableIndex;
	SCurveEditorContent                   m_selectedVariableContent;
	std::unique_ptr<CVariableUndoCommand> m_pUndoVarCommand;

	// Signal to all Undo's of this document, that they are obsolete (OnCloseAsset()) has been called
	std::unique_ptr<QObject> m_pUndoVarCancelSignal;
};
