// Copyright 2026 Silvan Teufel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PityRollTypes.h"
#include "PityRollComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPityGrantedSignature,
	FName, Key, bool, bByHardPity, int32, Attempt);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPityMissedSignature,
	FName, Key, int32, Misses);

/**
 * UPityRollComponent
 *
 * Goes on whatever owns the luck - usually the player state. It keeps one counter per KEY, and a
 * key is one entry of one table: "Legendary", "BossMount", "ExoticWeapon".
 *
 * One counter per key is the point. A shared counter means picking up a common resets the
 * protection that was building towards the rare, and the rare then never arrives any sooner than
 * pure chance would have brought it.
 */
UCLASS(ClassGroup = (PityRoll), meta = (BlueprintSpawnableComponent))
class PITYROLL_API UPityRollComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPityRollComponent();

	/**
	 * One attempt at one entry. RandomValue in [0,1) comes from the caller.
	 *
	 * Handed in rather than drawn here, so a server, a client prediction and a replay all reach the
	 * same answer from the same inputs.
	 */
	UFUNCTION(BlueprintCallable, Category = "PityRoll")
	FPityResult RollFor(FName Key, float BaseChance, float RandomValue);

	/** Same, but draws the number from the engine's stream. Convenient, and not replayable. */
	UFUNCTION(BlueprintCallable, Category = "PityRoll")
	FPityResult RollForRandom(FName Key, float BaseChance);

	/**
	 * The chance the next attempt at this entry will use.
	 *
	 * This is the number for a UI. It is the same function the roll calls.
	 */
	UFUNCTION(BlueprintPure, Category = "PityRoll")
	float GetEffectiveChance(FName Key, float BaseChance) const;

	/** How many more attempts until the guarantee. Negative when there is none. */
	UFUNCTION(BlueprintPure, Category = "PityRoll")
	int32 GetAttemptsUntilGuarantee(FName Key) const;

	UFUNCTION(BlueprintPure, Category = "PityRoll")
	FPityState GetState(FName Key) const;

	/** Per-entry rules. Without one, the project defaults apply. */
	UFUNCTION(BlueprintCallable, Category = "PityRoll")
	void SetRulesFor(FName Key, const FPityRules& Rules);

	UFUNCTION(BlueprintPure, Category = "PityRoll")
	FPityRules GetRulesFor(FName Key) const;

	/** Forget one entry's history. For a new season, a reset, a fresh character. */
	UFUNCTION(BlueprintCallable, Category = "PityRoll")
	void ResetKey(FName Key);

	UFUNCTION(BlueprintCallable, Category = "PityRoll")
	void ResetAll();

	/** Every key this component has seen. For a save game and for a debug readout. */
	UFUNCTION(BlueprintPure, Category = "PityRoll")
	TArray<FName> GetKnownKeys() const;

	/** Restore a saved counter. The state is plain data on purpose, so it serialises anywhere. */
	UFUNCTION(BlueprintCallable, Category = "PityRoll")
	void RestoreState(FName Key, const FPityState& State);

	UPROPERTY(BlueprintAssignable, Category = "PityRoll")
	FPityGrantedSignature OnGranted;

	UPROPERTY(BlueprintAssignable, Category = "PityRoll")
	FPityMissedSignature OnMissed;

private:
	UPROPERTY()
	TMap<FName, FPityState> Zaehler;

	UPROPERTY()
	TMap<FName, FPityRules> Regeln;
};
