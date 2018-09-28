// Copyright 2001-2019 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "PuppetRateOfDeathConsoleVariables.h"

void SAIConsoleVarsLegacyPuppetROD::Init()
{
	REGISTER_CVAR2("ai_RODAliveTime", &RODAliveTime, CTimeValue(3), VF_NULL,
		"The base level time the player can survive under fire.");
	REGISTER_CVAR2("ai_RODMoveInc", &RODMoveInc, CTimeValue(3), VF_NULL,
		"Increment how the speed of the target affects the alive time (the value is doubled for supersprint). 0=disable");
	REGISTER_CVAR2("ai_RODStanceInc", &RODStanceInc, CTimeValue(2), VF_NULL,
		"Increment how the stance of the target affects the alive time, 0=disable.\n"
		"The base value is for crouch, and it is doubled for prone.\n"
		"The crouch inc is disable in kill-zone and prone in kill and combat-near -zones");
	REGISTER_CVAR2("ai_RODDirInc", &RODDirInc, CTimeValue(0), VF_NULL,
		"Increment how the orientation of the target affects the alive time. 0=disable");
	REGISTER_CVAR2("ai_RODKillZoneInc", &RODKillZoneInc, CTimeValue(-4), VF_NULL,
		"Increment how the target is within the kill-zone of the target.");
	REGISTER_CVAR2("ai_RODFakeHitChance", &RODFakeHitChance, 0.2f, VF_NULL,
		"Percentage of the missed hits that will instead be hits dealing very little damage.");
	REGISTER_CVAR2("ai_RODAmbientFireInc", &RODAmbientFireInc, CTimeValue(3), VF_NULL,
		"Increment for the alive time when the target is within the kill-zone of the target.");

	REGISTER_CVAR2("ai_RODKillRangeMod", &RODKillRangeMod, 0.15f, VF_NULL,
		"Kill-zone distance = attackRange * killRangeMod.");
	REGISTER_CVAR2("ai_RODCombatRangeMod", &RODCombatRangeMod, 0.55f, VF_NULL,
		"Combat-zone distance = attackRange * combatRangeMod.");

	REGISTER_CVAR2("ai_RODCoverFireTimeMod", &RODCoverFireTimeMod, mpfloat(1), VF_NULL,
		"Multiplier for cover fire times set in weapon descriptor.");

	REGISTER_CVAR2("ai_RODReactionTime", &RODReactionTime, CTimeValue(1), VF_NULL,
		"Uses rate of death as damage control method.");
	REGISTER_CVAR2("ai_RODReactionDistInc", &RODReactionDistInc, CTimeValue("0.1"), VF_NULL,
		"Increase for the reaction time when the target is in combat-far-zone or warn-zone.\n"
		"In warn-zone the increase is doubled.");
	REGISTER_CVAR2("ai_RODReactionDirInc", &RODReactionDirInc, CTimeValue(2), VF_NULL,
		"Increase for the reaction time when the enemy is outside the players FOV or near the edge of the FOV.\n"
		"The increment is doubled when the target is behind the player.");
	REGISTER_CVAR2("ai_RODReactionLeanInc", &RODReactionLeanInc, CTimeValue("0.2"), VF_NULL,
		"Increase to the reaction to when the target is leaning.");
	REGISTER_CVAR2("ai_RODReactionSuperDarkIllumInc", &RODReactionSuperDarkIllumInc, CTimeValue("0.4"), VF_NULL,
		"Increase for reaction time when the target is in super dark light condition.");
	REGISTER_CVAR2("ai_RODReactionDarkIllumInc", &RODReactionDarkIllumInc, CTimeValue("0.3"), VF_NULL,
		"Increase for reaction time when the target is in dark light condition.");
	REGISTER_CVAR2("ai_RODReactionMediumIllumInc", &RODReactionMediumIllumInc, CTimeValue("0.2"), VF_NULL,
		"Increase for reaction time when the target is in medium light condition.");

	REGISTER_CVAR2("ai_RODLowHealthMercyTime", &RODLowHealthMercyTime, CTimeValue("1.5"), VF_NULL,
		"The amount of time the AI will not hit the target when the target crosses the low health threshold.");
}