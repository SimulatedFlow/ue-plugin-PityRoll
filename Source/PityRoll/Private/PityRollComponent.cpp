// Copyright 2026 Silvan Teufel. All Rights Reserved.

#include "PityRollComponent.h"

#include "GameFramework/Actor.h"
#include "PityRollLog.h"
#include "PityRollSettings.h"
#include "PityRollStatics.h"

UPityRollComponent::UPityRollComponent()
{
	// Nothing to tick: pity is counted in attempts, never in seconds. A time-based guarantee
	// rewards idling, which is the opposite of what a drop guarantee is for.
	PrimaryComponentTick.bCanEverTick = false;
}

FPityRules UPityRollComponent::GetRulesFor(const FName Key) const
{
	if (const FPityRules* Eigene = Regeln.Find(Key))
	{
		return *Eigene;
	}
	return UPityRollSettings::Get()->DefaultRules;
}

void UPityRollComponent::SetRulesFor(const FName Key, const FPityRules& Rules)
{
	Regeln.Add(Key, UPityRollStatics::NormaliseRules(Rules));
}

FPityState UPityRollComponent::GetState(const FName Key) const
{
	if (const FPityState* S = Zaehler.Find(Key))
	{
		return *S;
	}
	return FPityState();
}

float UPityRollComponent::GetEffectiveChance(const FName Key, const float BaseChance) const
{
	return UPityRollStatics::EffectiveChance(BaseChance, GetState(Key).Misses, GetRulesFor(Key));
}

int32 UPityRollComponent::GetAttemptsUntilGuarantee(const FName Key) const
{
	return UPityRollStatics::AttemptsUntilGuarantee(GetState(Key), GetRulesFor(Key));
}

FPityResult UPityRollComponent::RollFor(const FName Key, const float BaseChance,
	const float RandomValue)
{
	const FPityRules R = GetRulesFor(Key);
	const FPityResult Ergebnis = UPityRollStatics::Roll(GetState(Key), BaseChance, R, RandomValue);

	if (Ergebnis.Outcome == EPityOutcome::NotRolled)
	{
		return Ergebnis;
	}

	Zaehler.Add(Key, Ergebnis.State);

	if (Ergebnis.bGranted)
	{
		const bool bGarantie = Ergebnis.Outcome == EPityOutcome::GrantedByHardPity;
		if (bGarantie && UPityRollSettings::Get()->bLogHardPity)
		{
			UE_LOG(LogPityRoll, Display,
				TEXT("[%s] %s granted by the guarantee on attempt %d (chance was %.1f%%)"),
				*GetNameSafe(GetOwner()), *Key.ToString(), Ergebnis.Attempt,
				Ergebnis.EffectiveChance * 100.0f);
		}
		OnGranted.Broadcast(Key, bGarantie, Ergebnis.Attempt);
	}
	else
	{
		OnMissed.Broadcast(Key, Ergebnis.State.Misses);
	}

	return Ergebnis;
}

FPityResult UPityRollComponent::RollForRandom(const FName Key, const float BaseChance)
{
	return RollFor(Key, BaseChance, FMath::FRand());
}

void UPityRollComponent::ResetKey(const FName Key)
{
	Zaehler.Remove(Key);
}

void UPityRollComponent::ResetAll()
{
	Zaehler.Reset();
}

TArray<FName> UPityRollComponent::GetKnownKeys() const
{
	TArray<FName> Schluessel;
	Zaehler.GetKeys(Schluessel);
	Schluessel.Sort(FNameLexicalLess());
	return Schluessel;
}

void UPityRollComponent::RestoreState(const FName Key, const FPityState& State)
{
	Zaehler.Add(Key, State);
}
