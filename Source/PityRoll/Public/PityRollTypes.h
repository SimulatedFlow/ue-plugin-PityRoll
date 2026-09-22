// Copyright 2026 Silvan Teufel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PityRollTypes.generated.h"

/** Why a roll came out the way it did. Handed back so a log line can explain a lucky streak. */
UENUM(BlueprintType)
enum class EPityOutcome : uint8
{
	/** The roll failed. The miss counter went up. */
	Missed,
	/** The roll succeeded on its own, at the effective chance. */
	Granted,
	/** The roll was guaranteed because the hard-pity attempt had been reached. */
	GrantedByHardPity,
	/** Nothing was rolled - a base chance of zero or less with no pity configured. */
	NotRolled,
};

/**
 * The rules for one entry.
 *
 * Passed to the pure functions explicitly rather than read from the settings inside them: a project
 * wants different protection on a legendary than on a common, and the tests hand in their own.
 */
USTRUCT(BlueprintType)
struct PITYROLL_API FPityRules
{
	GENERATED_BODY()

	/**
	 * Misses after which the chance starts climbing. Zero means it climbs from the first miss.
	 *
	 * Below this, the chance is exactly the base chance - which matters, because a player who
	 * datamines the table should find the number they were told.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PityRoll", meta = (ClampMin = "0"))
	int32 SoftPityAfter = 0;

	/** How much the chance climbs per miss once the soft pity has started. Zero switches it off. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PityRoll", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RampPerMiss = 0.0f;

	/**
	 * The attempt number that is guaranteed, counting from one. Zero means no guarantee.
	 *
	 * HardPityAt = 40 means the fortieth attempt in a row without the drop gets it. Not the
	 * thirty-ninth and not the forty-first: the off-by-one here is invisible until a player counts,
	 * and players do count.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PityRoll", meta = (ClampMin = "0"))
	int32 HardPityAt = 0;

	/** Never let the effective chance climb above this. One means no cap. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PityRoll", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxChance = 1.0f;
};

/** What one entry remembers. Plain data: no world, no actor, no clock. */
USTRUCT(BlueprintType)
struct PITYROLL_API FPityState
{
	GENERATED_BODY()

	/** Consecutive attempts that did NOT produce this entry. */
	UPROPERTY(BlueprintReadOnly, Category = "PityRoll")
	int32 Misses = 0;

	/** How often this entry has ever been granted. For a statistics readout. */
	UPROPERTY(BlueprintReadOnly, Category = "PityRoll")
	int32 Grants = 0;

	/** How often the guarantee had to step in. A high number means the base chance is too low. */
	UPROPERTY(BlueprintReadOnly, Category = "PityRoll")
	int32 HardPityGrants = 0;

	/** Total attempts, granted or not. */
	UPROPERTY(BlueprintReadOnly, Category = "PityRoll")
	int32 Attempts = 0;
};

/** The answer to one roll. */
USTRUCT(BlueprintType)
struct PITYROLL_API FPityResult
{
	GENERATED_BODY()

	/** The state after the roll. Assign it back; the functions never mutate what they are given. */
	UPROPERTY(BlueprintReadOnly, Category = "PityRoll")
	FPityState State;

	UPROPERTY(BlueprintReadOnly, Category = "PityRoll")
	EPityOutcome Outcome = EPityOutcome::NotRolled;

	UPROPERTY(BlueprintReadOnly, Category = "PityRoll")
	bool bGranted = false;

	/**
	 * The chance this roll actually used, after soft pity and the cap.
	 *
	 * Show THIS in a UI. A displayed chance that disagrees with the roll is worse than showing
	 * nothing, and it will disagree the moment the two are computed in different places.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "PityRoll")
	float EffectiveChance = 0.0f;

	/** Which attempt this was, counting from one. */
	UPROPERTY(BlueprintReadOnly, Category = "PityRoll")
	int32 Attempt = 0;
};
