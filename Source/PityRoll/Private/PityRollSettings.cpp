// Copyright 2026 Silvan Teufel. All Rights Reserved.

#include "PityRollSettings.h"

UPityRollSettings::UPityRollSettings()
	: bLogHardPity(false)
{
	// No protection unless a project asks for it. A pity system switched on by accident changes a
	// drop table's whole shape, and the designer who drew that table would not be told.
	DefaultRules.SoftPityAfter = 0;
	DefaultRules.RampPerMiss = 0.0f;
	DefaultRules.HardPityAt = 0;
	DefaultRules.MaxChance = 1.0f;
}

const UPityRollSettings* UPityRollSettings::Get()
{
	const UPityRollSettings* Settings = GetDefault<UPityRollSettings>();
	check(Settings);
	return Settings;
}
