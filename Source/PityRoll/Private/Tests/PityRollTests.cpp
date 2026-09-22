// Copyright 2026 Silvan Teufel. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "PityRollStatics.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace PityRollTests
{
	constexpr EAutomationTestFlags TestFlags = EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::CommandletContext
		| EAutomationTestFlags::EngineFilter;

	/** Soft pity from the tenth miss, a point a miss, guaranteed on the fortieth attempt. */
	static FPityRules Rules()
	{
		FPityRules R;
		R.SoftPityAfter = 10;
		R.RampPerMiss = 0.01f;
		R.HardPityAt = 40;
		R.MaxChance = 1.0f;
		return R;
	}

	static FPityState Nach(const int32 Misses, const int32 Attempts = 0)
	{
		FPityState S;
		S.Misses = Misses;
		S.Attempts = Attempts > 0 ? Attempts : Misses;
		return S;
	}
}

// -------------------------------------------------------------------------------------------------
// The off-by-one that players count.
// -------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPityRollGuaranteeLandsOnTheNamedAttempt,
	"PityRoll.HardPity.LandsOnExactlyTheNamedAttempt", PityRollTests::TestFlags)

bool FPityRollGuaranteeLandsOnTheNamedAttempt::RunTest(const FString&)
{
	using namespace PityRollTests;
	const FPityRules R = Rules();   // HardPityAt = 40

	// Thirty-eight misses behind us: the next attempt is number thirty-nine. Not yet.
	TestFalse(TEXT("attempt 39 is not guaranteed"),
		UPityRollStatics::IsNextAttemptGuaranteed(38, R));

	// Thirty-nine misses: the next attempt is number forty. That is the promised one.
	TestTrue(TEXT("attempt 40 is guaranteed"),
		UPityRollStatics::IsNextAttemptGuaranteed(39, R));

	// And it really fires, even on a random value that would otherwise miss by a mile.
	const FPityResult Vierzig = UPityRollStatics::Roll(Nach(39), 0.05f, R, 0.999f);
	TestTrue(TEXT("and it is granted"), Vierzig.bGranted);
	TestTrue(TEXT("by the guarantee, and it says so"),
		Vierzig.Outcome == EPityOutcome::GrantedByHardPity);
	TestEqual(TEXT("it was attempt forty"), Vierzig.Attempt, 40);
	TestNearlyEqual(TEXT("and the chance it reports is one"), Vierzig.EffectiveChance, 1.0f, 0.0001f);

	// One attempt earlier, the same unlucky roll still misses.
	const FPityResult Neununddreissig = UPityRollStatics::Roll(Nach(38), 0.05f, R, 0.999f);
	TestFalse(TEXT("attempt 39 with the same roll misses"), Neununddreissig.bGranted);
	TestEqual(TEXT("and the counter goes to thirty-nine"), Neununddreissig.State.Misses, 39);

	// The countdown a UI would show.
	TestEqual(TEXT("from a clean slate it is thirty-nine more attempts"),
		UPityRollStatics::AttemptsUntilGuarantee(Nach(0), R), 39);
	TestEqual(TEXT("at thirty-nine misses it is none"),
		UPityRollStatics::AttemptsUntilGuarantee(Nach(39), R), 0);

	return true;
}

// -------------------------------------------------------------------------------------------------
// The counter is consumed once.
// -------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPityRollCounterIsConsumedExactlyOnce,
	"PityRoll.Counter.ConsumedExactlyOnceOnAnyGrant", PityRollTests::TestFlags)

bool FPityRollCounterIsConsumedExactlyOnce::RunTest(const FString&)
{
	using namespace PityRollTests;
	const FPityRules R = Rules();

	// A NATURAL hit deep inside the pity window. The player gets the drop - and loses the
	// protection. Keeping both is the bug: the next rare would then arrive almost immediately and
	// the whole curve the designer drew is gone.
	const FPityResult Natuerlich = UPityRollStatics::Roll(Nach(35), 0.05f, R, 0.01f);
	TestTrue(TEXT("a natural hit inside the window is granted"), Natuerlich.bGranted);
	TestTrue(TEXT("and reports itself as natural, not as the guarantee"),
		Natuerlich.Outcome == EPityOutcome::Granted);
	TestEqual(TEXT("the counter is back to zero"), Natuerlich.State.Misses, 0);
	TestEqual(TEXT("the guarantee was not used"), Natuerlich.State.HardPityGrants, 0);

	// The guaranteed hit resets it exactly the same way, and is counted separately so a designer
	// can see how often the floor had to catch the player.
	const FPityResult Garantiert = UPityRollStatics::Roll(Nach(39), 0.05f, R, 0.999f);
	TestEqual(TEXT("the counter is back to zero here too"), Garantiert.State.Misses, 0);
	TestEqual(TEXT("and the guarantee is counted"), Garantiert.State.HardPityGrants, 1);

	// A miss only ever adds one, however unlucky.
	const FPityResult Daneben = UPityRollStatics::Roll(Nach(3), 0.05f, R, 0.99f);
	TestEqual(TEXT("a miss adds exactly one"), Daneben.State.Misses, 4);
	TestEqual(TEXT("and one attempt"), Daneben.State.Attempts, 4);

	return true;
}

// -------------------------------------------------------------------------------------------------
// The number on screen is the number in the roll.
// -------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPityRollDisplayedChanceIsTheRolledChance,
	"PityRoll.Chance.WhatIsShownIsWhatIsRolled", PityRollTests::TestFlags)

bool FPityRollDisplayedChanceIsTheRolledChance::RunTest(const FString&)
{
	using namespace PityRollTests;
	const FPityRules R = Rules();   // soft pity after 10, +1 point per miss

	// Below the threshold the player gets exactly the advertised number. Somebody who reads the
	// patch notes and then measures must find what they were told.
	TestNearlyEqual(TEXT("a fresh attempt is the base chance"),
		UPityRollStatics::EffectiveChance(0.05f, 0, R), 0.05f, 0.0001f);
	TestNearlyEqual(TEXT("and so is the tenth"),
		UPityRollStatics::EffectiveChance(0.05f, 9, R), 0.05f, 0.0001f);

	// The first miss inside the ramp buys exactly one step - not none, not two.
	TestNearlyEqual(TEXT("one step past the threshold is one step of help"),
		UPityRollStatics::EffectiveChance(0.05f, 10, R), 0.06f, 0.0001f);
	TestNearlyEqual(TEXT("ten steps past it, ten"),
		UPityRollStatics::EffectiveChance(0.05f, 19, R), 0.15f, 0.0001f);

	// THE POINT: a roll at exactly the boundary of the effective chance decides the way the
	// displayed number says it should. If Roll repeated the arithmetic instead of calling
	// EffectiveChance, these two could disagree and nobody would notice for months.
	const float Chance = UPityRollStatics::EffectiveChance(0.05f, 15, R);
	const FPityResult KnappDrunter = UPityRollStatics::Roll(Nach(15), 0.05f, R, Chance - 0.0001f);
	const FPityResult KnappDrueber = UPityRollStatics::Roll(Nach(15), 0.05f, R, Chance + 0.0001f);
	TestTrue(TEXT("just under the shown chance is a hit"), KnappDrunter.bGranted);
	TestFalse(TEXT("just over it is a miss"), KnappDrueber.bGranted);
	TestNearlyEqual(TEXT("and the roll reports the same number"),
		KnappDrunter.EffectiveChance, Chance, 0.0001f);

	// The cap holds, and the ramp never runs past one.
	FPityRules Steil = R;
	Steil.RampPerMiss = 0.5f;
	Steil.MaxChance = 0.8f;
	TestNearlyEqual(TEXT("the cap holds"),
		UPityRollStatics::EffectiveChance(0.05f, 30, Steil), 0.8f, 0.0001f);

	return true;
}

// -------------------------------------------------------------------------------------------------
// Configuration that cannot mean what it says.
// -------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPityRollRulesAreNormalised,
	"PityRoll.Rules.ImpossibleCombinationsAreCorrected", PityRollTests::TestFlags)

bool FPityRollRulesAreNormalised::RunTest(const FString&)
{
	using namespace PityRollTests;

	// A ramp that starts after the guarantee has already fired is dead code, and it reads to
	// whoever wrote it as though the ramp is broken.
	FPityRules Widerspruch;
	Widerspruch.SoftPityAfter = 50;
	Widerspruch.RampPerMiss = 0.02f;
	Widerspruch.HardPityAt = 20;

	const FPityRules Fest = UPityRollStatics::NormaliseRules(Widerspruch);
	TestTrue(TEXT("the ramp is pulled inside the window"), Fest.SoftPityAfter < Fest.HardPityAt);
	TestEqual(TEXT("to the attempt before the guarantee"), Fest.SoftPityAfter, 19);

	// Negative and out-of-range numbers are clamped rather than trusted.
	FPityRules Unsinn;
	Unsinn.SoftPityAfter = -5;
	Unsinn.RampPerMiss = 2.0f;
	Unsinn.HardPityAt = -1;
	Unsinn.MaxChance = 5.0f;
	const FPityRules Sauber = UPityRollStatics::NormaliseRules(Unsinn);
	TestEqual(TEXT("no negative threshold"), Sauber.SoftPityAfter, 0);
	TestNearlyEqual(TEXT("no ramp above one"), Sauber.RampPerMiss, 1.0f, 0.0001f);
	TestEqual(TEXT("no negative guarantee"), Sauber.HardPityAt, 0);
	TestNearlyEqual(TEXT("no cap above one"), Sauber.MaxChance, 1.0f, 0.0001f);

	// A base chance of zero with no pity is a non-event, not a miss. Counting it as a miss would
	// slowly build a guarantee for a drop that can never happen.
	FPityRules Ohne;
	const FPityResult Nichts = UPityRollStatics::Roll(Nach(5), 0.0f, Ohne, 0.5f);
	TestTrue(TEXT("nothing is rolled"), Nichts.Outcome == EPityOutcome::NotRolled);
	TestEqual(TEXT("and nothing is counted"), Nichts.State.Misses, 5);
	TestEqual(TEXT("not even an attempt"), Nichts.State.Attempts, Nach(5).Attempts);

	return true;
}

// -------------------------------------------------------------------------------------------------
// A whole sequence, so the curve is the one that was drawn.
// -------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPityRollNobodyEverExceedsTheGuarantee,
	"PityRoll.Sequence.NobodyEverGoesPastTheGuarantee", PityRollTests::TestFlags)

bool FPityRollNobodyEverExceedsTheGuarantee::RunTest(const FString&)
{
	using namespace PityRollTests;
	const FPityRules R = Rules();   // guaranteed on attempt 40

	// The worst possible luck: every roll comes back as the largest value there is.
	FPityState S;
	int32 LaengsteDurststrecke = 0;
	int32 Gewaehrt = 0;

	for (int32 i = 0; i < 500; ++i)
	{
		const FPityResult Ergebnis = UPityRollStatics::Roll(S, 0.05f, R, 0.9999f);
		S = Ergebnis.State;
		LaengsteDurststrecke = FMath::Max(LaengsteDurststrecke, S.Misses);
		Gewaehrt += Ergebnis.bGranted ? 1 : 0;
	}

	// That is the promise the whole plugin exists to make: however unlucky, never more than
	// thirty-nine attempts in a row without it.
	TestTrue(TEXT("the longest dry streak is under the guarantee"), LaengsteDurststrecke < 40);
	TestEqual(TEXT("five hundred attempts at a guarantee of forty is twelve grants"), Gewaehrt, 12);
	TestEqual(TEXT("all of them by the guarantee"), S.HardPityGrants, 12);

	// And with good luck the guarantee never has to step in at all.
	FPityState Glueck;
	for (int32 i = 0; i < 100; ++i)
	{
		Glueck = UPityRollStatics::Roll(Glueck, 0.05f, R, 0.0f).State;
	}
	TestEqual(TEXT("a hundred lucky rolls are a hundred grants"), Glueck.Grants, 100);
	TestEqual(TEXT("and the guarantee never fired"), Glueck.HardPityGrants, 0);
	TestNearlyEqual(TEXT("the observed rate reads back"),
		UPityRollStatics::ObservedRate(Glueck), 1.0f, 0.0001f);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
