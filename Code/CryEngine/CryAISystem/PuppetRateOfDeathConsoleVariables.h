// Copyright 2001-2019 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include <CrySystem/ConsoleRegistration.h>

struct SAIConsoleVarsLegacyPuppetROD
{
	void Init();

	CTimeValue RODAliveTime;
	CTimeValue RODMoveInc;
	CTimeValue RODStanceInc;
	CTimeValue RODDirInc;
	CTimeValue RODAmbientFireInc;
	CTimeValue RODKillZoneInc;
	float RODFakeHitChance;

	float RODKillRangeMod;
	float RODCombatRangeMod;

	CTimeValue RODReactionTime;
	CTimeValue RODReactionSuperDarkIllumInc;
	CTimeValue RODReactionDarkIllumInc;
	CTimeValue RODReactionMediumIllumInc;
	CTimeValue RODReactionDistInc;
	CTimeValue RODReactionDirInc;
	CTimeValue RODReactionLeanInc;

	CTimeValue RODLowHealthMercyTime;

	mpfloat RODCoverFireTimeMod;
};