// Copyright 2026 Silvan Teufel. All Rights Reserved.

#include "PityRollStatics.h"

FPityRules UPityRollStatics::NormaliseRules(const FPityRules& Rules)
{
	FPityRules Out = Rules;
	Out.SoftPityAfter = FMath::Max(0, Out.SoftPityAfter);
	Out.HardPityAt = FMath::Max(0, Out.HardPityAt);
	Out.RampPerMiss = FMath::Clamp(Out.RampPerMiss, 0.0f, 1.0f);
	Out.MaxChance = FMath::Clamp(Out.MaxChance, 0.0f, 1.0f);

	// A guarantee that lands before the ramp even starts makes the ramp dead code and reads, to
	// whoever wrote it, as though the ramp is broken. Push the start of the ramp inside the window.
	if (Out.HardPityAt > 0 && Out.SoftPityAfter >= Out.HardPityAt)
	{
		Out.SoftPityAfter = FMath::Max(0, Out.HardPityAt - 1);
	}

	return Out;
}

float UPityRollStatics::EffectiveChance(const float BaseChance, const int32 Misses,
	const FPityRules& Rules)
{
	const FPityRules R = NormaliseRules(Rules);
	const int32 Seen = FMath::Max(0, Misses);

	if (IsNextAttemptGuaranteed(Seen, R))
	{
		return 1.0f;
	}

	const float Basis = FMath::Clamp(BaseChance, 0.0f, 1.0f);
	if (R.RampPerMiss <= 0.0f || Seen < R.SoftPityAfter)
	{
		return FMath::Min(Basis, R.MaxChance);
	}

	// Counted from the first miss that is INSIDE the ramp, so the attempt right after the threshold
	// gets exactly one step of help rather than none or two.
	const int32 Schritte = Seen - R.SoftPityAfter + 1;
	const float Erhoeht = Basis + R.RampPerMiss * static_cast<float>(Schritte);
	return FMath::Clamp(Erhoeht, 0.0f, R.MaxChance);
}

bool UPityRollStatics::IsNextAttemptGuaranteed(const int32 Misses, const FPityRules& Rules)
{
	const FPityRules R = NormaliseRules(Rules);
	if (R.HardPityAt <= 0)
	{
		return false;
	}
	// Misses failures behind us means the next attempt is number Misses + 1.
	return (FMath::Max(0, Misses) + 1) >= R.HardPityAt;
}

FPityResult UPityRollStatics::Roll(const FPityState& State, const float BaseChance,
	const FPityRules& Rules, const float RandomValue)
{
	const FPityRules R = NormaliseRules(Rules);

	FPityResult Result;
	Result.State = State;
	Result.Attempt = State.Attempts + 1;
	Result.EffectiveChance = EffectiveChance(BaseChance, State.Misses, R);

	const bool bGarantie = IsNextAttemptGuaranteed(State.Misses, R);

	if (Result.EffectiveChance <= 0.0f && !bGarantie)
	{
		// Nothing to roll. Explicitly a non-event rather than a silent miss: a base chance of zero
		// with no pity is a configuration mistake, and counting it as a miss would slowly build a
		// guarantee for a drop that can never happen.
		Result.Outcome = EPityOutcome::NotRolled;
		return Result;
	}

	Result.State.Attempts = State.Attempts + 1;

	const bool bTreffer = bGarantie || (RandomValue < Result.EffectiveChance);

	if (!bTreffer)
	{
		Result.State.Misses = State.Misses + 1;
		Result.Outcome = EPityOutcome::Missed;
		return Result;
	}

	// The counter is consumed EXACTLY ONCE, whether the hit was natural or guaranteed. Anything
	// else lets a player collect the drop and keep the protection they had built up for it.
	Result.State.Misses = 0;
	Result.State.Grants = State.Grants + 1;
	Result.bGranted = true;

	if (bGarantie)
	{
		Result.State.HardPityGrants = State.HardPityGrants + 1;
		Result.Outcome = EPityOutcome::GrantedByHardPity;
	}
	else
	{
		Result.Outcome = EPityOutcome::Granted;
	}

	return Result;
}

int32 UPityRollStatics::AttemptsUntilGuarantee(const FPityState& State, const FPityRules& Rules)
{
	const FPityRules R = NormaliseRules(Rules);
	if (R.HardPityAt <= 0)
	{
		return -1;
	}
	return FMath::Max(0, R.HardPityAt - (State.Misses + 1));
}

float UPityRollStatics::ObservedRate(const FPityState& State)
{
	if (State.Attempts <= 0)
	{
		return 0.0f;
	}
	return static_cast<float>(State.Grants) / static_cast<float>(State.Attempts);
}
