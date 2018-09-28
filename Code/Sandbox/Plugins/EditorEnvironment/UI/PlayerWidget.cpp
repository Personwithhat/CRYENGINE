// Copyright 2001-2019 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "PlayerWidget.h"

#include "Controller.h"
#include "EnvironmentEditor.h"

#include <TimeEditControl.h>

#include <QBoxLayout>
#include <QLabel>
#include <QTime>
#include <QToolButton>

CPlayerWidget::CPlayerWidget(QWidget* pParent, CController& controller)
	: QWidget(pParent)
	, m_controller(controller)
{
	CreateControls();
	ConnectSignals();
	ResetState();
}

CPlayerWidget::~CPlayerWidget()
{
	m_controller.signalAssetOpened.DisconnectObject(this);
	m_controller.signalAssetClosed.DisconnectObject(this);
	m_controller.signalCurrentTimeChanged.DisconnectObject(this);
	m_controller.signalPlaybackModeChanged.DisconnectObject(this);
}

void CPlayerWidget::CreateControls()
{
	QHBoxLayout* pMainLayout = new QHBoxLayout;
	setLayout(pMainLayout);

	pMainLayout->setContentsMargins(QMargins(0, 0, 0, 0));

	QBoxLayout* pLayoutLeft = new QHBoxLayout;
	QBoxLayout* pLayoutCenter = new QHBoxLayout;
	QBoxLayout* pLayoutRight = new QHBoxLayout;

	pMainLayout->addLayout(pLayoutLeft);
	pMainLayout->addLayout(pLayoutCenter);
	pMainLayout->addLayout(pLayoutRight);

	QLabel* pStartTimeLabel = new QLabel("Start:");
	pStartTimeLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	m_pStartTimeEdit = new CTimeEditControl(this);
	m_pStartTimeEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	pLayoutLeft->addWidget(pStartTimeLabel);
	pLayoutLeft->addWidget(m_pStartTimeEdit);
	pLayoutLeft->addStretch();

	QLabel* pCurrentTimeLabel = new QLabel("Current:");
	pCurrentTimeLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	m_pCurrentTimeEdit = new CTimeEditControl(this);
	m_pCurrentTimeEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	m_pStopButton = new QToolButton(this);
	m_pStopButton->setIcon(CryIcon("icons:Trackview/Stop_Sequence.ico"));
	m_pStopButton->setCheckable(true);
	m_pStopButton->setAutoExclusive(true);

	m_pPlayButton = new QToolButton(this);
	m_pPlayButton->setIcon(CryIcon("icons:Trackview/Play_Sequence.ico"));
	m_pPlayButton->setCheckable(true);
	m_pPlayButton->setAutoExclusive(true);

	QLabel* pPlaySpeedLabel = new QLabel("Speed: ");
	pPlaySpeedLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	m_pPlaySpeedEdit = new QLineEdit(this);
	m_pPlaySpeedEdit->setFixedWidth(37);
	m_pPlaySpeedEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	QDoubleValidator* validator = new QDoubleValidator(0.0, 99.999, 3, m_pPlaySpeedEdit);
	validator->setNotation(QDoubleValidator::StandardNotation);
	m_pPlaySpeedEdit->setValidator(validator);
	m_pPlaySpeedEdit->setToolTip("TimeOfDay play speed\n Valid range:[0.000 - 99.999]");

	pLayoutCenter->setSpacing(0);
	pLayoutCenter->setMargin(0);

	pLayoutCenter->addWidget(pCurrentTimeLabel);
	pLayoutCenter->addWidget(m_pCurrentTimeEdit);
	pLayoutCenter->addSpacing(5);
	pLayoutCenter->addWidget(m_pStopButton);
	pLayoutCenter->addWidget(m_pPlayButton);
	pLayoutCenter->addSpacing(5);
	pLayoutCenter->addWidget(pPlaySpeedLabel);
	pLayoutCenter->addWidget(m_pPlaySpeedEdit);
	pLayoutCenter->addSpacing(15);

	QLabel* pEndTimeLabel = new QLabel("End:");
	pEndTimeLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	m_pEndTimeEdit = new CTimeEditControl(this);
	m_pEndTimeEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	pLayoutRight->addStretch();
	pLayoutRight->addWidget(pEndTimeLabel, Qt::AlignRight);
	pLayoutRight->addWidget(m_pEndTimeEdit, Qt::AlignRight);
}

void CPlayerWidget::ConnectSignals()
{
	connect(m_pStartTimeEdit, &CTimeEditControl::timeChanged, this, &CPlayerWidget::OnPlaybackParamsChanged);
	connect(m_pEndTimeEdit, &CTimeEditControl::timeChanged, this, &CPlayerWidget::OnPlaybackParamsChanged);
	connect(m_pPlaySpeedEdit, &QLineEdit::editingFinished, this, &CPlayerWidget::OnPlaybackParamsChanged);

	connect(m_pStopButton, &QToolButton::clicked, [this]() { m_controller.TogglePlaybackMode(); });
	connect(m_pPlayButton, &QToolButton::clicked, [this]() { m_controller.TogglePlaybackMode(); });

	connect(m_pCurrentTimeEdit, &CTimeEditControl::timeChanged, this, &CPlayerWidget::CurrentTimeEdited);

	m_controller.signalAssetOpened.Connect(this, &CPlayerWidget::ResetState);
	m_controller.signalAssetClosed.Connect(this, &CPlayerWidget::ResetState);
	m_controller.signalCurrentTimeChanged.Connect(this, &CPlayerWidget::OnCurrentTimeChanged);
	m_controller.signalPlaybackModeChanged.Connect(this, &CPlayerWidget::OnPlaybackModeChanged);
}

void CPlayerWidget::ResetState()
{
	ResetPlaybackEdits();
	OnPlaybackModeChanged(m_controller.GetPlaybackMode());
}

void CPlayerWidget::ResetPlaybackEdits()
{
	CTimeValue start, end; mpfloat speed;
	m_controller.GetEnginePlaybackParams(start, end, speed);

	blockSignals(true);

	m_pStartTimeEdit->setTime(TimeToQTime(start));
	m_pEndTimeEdit->setTime(TimeToQTime(end));
	m_pPlaySpeedEdit->setText(QString::number(BADF speed));

	const CTimeValue currTime = m_controller.GetCurrentTime();
	m_pCurrentTimeEdit->setTime(TimeToQTime(currTime));

	blockSignals(false);
}

CTimeValue CPlayerWidget::GetTime(CTimeEditControl* pTimeCtrl) const
{
	const int nSeconds = QTime(0, 0, 0).secsTo(pTimeCtrl->time());
	const CTimeValue time = nSeconds / 3600;
	return time;
}

QTime CPlayerWidget::TimeToQTime(const CTimeValue& time) const
{
	mpfloat tSec = time.GetSeconds();
	unsigned int hour = (int)floor(tSec);

	static const CTimeValue nTimeLineMinutesScale = 60;
	unsigned int minute = (int)floor((time - floor(tSec)) * nTimeLineMinutesScale);
	if (hour > 23)
	{
		hour = 23;
		minute = 59;
	}

	return QTime(hour, minute);
}

void CPlayerWidget::OnPlaybackParamsChanged()
{
	const CTimeValue start = GetTime(m_pStartTimeEdit);
	const CTimeValue end = GetTime(m_pEndTimeEdit);
	const mpfloat speed = BADMP(m_pPlaySpeedEdit->text().toFloat());

	m_controller.SetEnginePlaybackParams(start, end, speed);
}

void CPlayerWidget::CurrentTimeEdited()
{
	const CTimeValue time = GetTime(m_pCurrentTimeEdit);
	m_controller.SetCurrentTime(this, time);
}

// Signals from the controller (nothing should be emitted back)
void CPlayerWidget::OnPlaybackModeChanged(PlaybackMode newMode)
{
	const bool allowEdit = (newMode == PlaybackMode::Edit) && m_controller.GetEditor().GetPreset();

	blockSignals(true);

	m_pStartTimeEdit->setEnabled(allowEdit);
	m_pCurrentTimeEdit->setEnabled(allowEdit);
	m_pEndTimeEdit->setEnabled(allowEdit);
	m_pPlaySpeedEdit->setEnabled(allowEdit);

	m_pStopButton->setChecked(allowEdit);
	m_pStopButton->setEnabled((newMode == PlaybackMode::Play) && m_controller.GetEditor().GetPreset());

	m_pPlayButton->setChecked(!allowEdit);
	m_pPlayButton->setEnabled(allowEdit);

	blockSignals(false);
}

void CPlayerWidget::OnCurrentTimeChanged(QWidget* pSender, const CTimeValue& newTime)
{
	if (this == pSender)
	{
		return;
	}

	QTime qTime = TimeToQTime(newTime);

	m_pCurrentTimeEdit->blockSignals(true);
	m_pCurrentTimeEdit->setTime(qTime);
	m_pCurrentTimeEdit->blockSignals(false);
}
