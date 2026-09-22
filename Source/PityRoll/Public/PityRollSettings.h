// Copyright 2026 Silvan Teufel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PityRollTypes.h"
#include "PityRollSettings.generated.h"

/** Project Settings > Plugins > PityRoll. The defaults an entry starts from. */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "PityRoll"))
class PITYROLL_API UPityRollSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPityRollSettings();

	virtual FName GetContainerName() const override { return TEXT("Project"); }
	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }

	static const UPityRollSettings* Get();

	/** Used for any entry that does not bring its own rules. */
	UPROPERTY(config, EditAnywhere, Category = "Rules")
	FPityRules DefaultRules;

	/** Write a line whenever the guarantee had to step in. */
	UPROPERTY(config, EditAnywhere, Category = "Diagnostics")
	bool bLogHardPity;
};
