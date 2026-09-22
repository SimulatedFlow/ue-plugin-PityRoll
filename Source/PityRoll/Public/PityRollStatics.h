// Copyright 2026 Silvan Teufel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PityRollTypes.h"
#include "PityRollStatics.generated.h"

/**
 * The rules, on their own.
 *
 * No world, no actor, no random stream - the random value is handed in. The component calls exactly
 * these and so do the tests, which is the only way the chance on screen and the chance in the roll
 * cannot drift apart.
 */
UCLASS()
class PITYROLL_API UPityRollStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Rules with the impossible combinations corrected. Call it once and keep the result. */
	UFUNCTION(BlueprintPure, Category = "PityRoll|Rules")
	static FPityRules NormaliseRules(const FPityRules& Rules);

	/**
	 * The chance the NEXT attempt will use, given the misses so far.
	 *
	 * This is the number to put in a UI. It is also the number Roll uses, because Roll calls this
	 * function rather than repeating the arithmetic.
	 */
	UFUNCTION(BlueprintPure, Category = "PityRoll|Rules")
	static float EffectiveChance(float BaseChance, int32 Misses, const FPityRules& Rules);

	/**
	 * Is the next attempt the guaranteed one?
	 *
	 * With Misses failures behind it, the next attempt is number Misses + 1. HardPityAt = 40 means
	 * attempt forty is guaranteed, so this is true from thirty-nine misses onwards - and getting
	 * that boundary wrong by one is invisible until a player counts.
	 */
	UFUNCTION(BlueprintPure, Category = "PityRoll|Rules")
	static bool IsNextAttemptGuaranteed(int32 Misses, const FPityRules& Rules);

	/**
	 * One attempt. RandomValue is a number in [0,1) that the CALLER provides.
	 *
	 * Handed in rather than drawn inside, so a test, a replay and a server all get the same answer
	 * from the same inputs. Anything that owns a random stream can feed it.
	 */
	UFUNCTION(BlueprintCallable, Category = "PityRoll|Rules")
	static FPityResult Roll(const FPityState& State, float BaseChance, const FPityRules& Rules,
		float RandomValue);

	/** How many more attempts until the guarantee. Negative means there is no guarantee. */
	UFUNCTION(BlueprintPure, Category = "PityRoll|Rules")
	static int32 AttemptsUntilGuarantee(const FPityState& State, const FPityRules& Rules);

	/** Observed rate so far: grants over attempts. Zero attempts reads as zero. */
	UFUNCTION(BlueprintPure, Category = "PityRoll|Rules")
	static float ObservedRate(const FPityState& State);
};
