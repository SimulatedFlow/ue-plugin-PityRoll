// Copyright 2026 Silvan Teufel. All Rights Reserved.

#include "PityRollDemoDirector.h"

#include "Components/TextRenderComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "PityRollStatics.h"

namespace PityRollDemoLocal
{
	/**
	 * Horizontal is X - the camera looks along +Y, so Y is depth.
	 *
	 * The camera looks along +Y with +Z up, which puts world -X on the RIGHT of the picture. So the
	 * unprotected column, the one the headline calls "the left column", has to sit at POSITIVE X.
	 * Getting this backwards is not a cosmetic slip: the board would name one column and point at
	 * the other.
	 *
	 * The columns stand outside the two text blocks (x = +-480, about 900 wide), or the bars draw
	 * straight through the numbers.
	 */
	constexpr float SpalteX[2] = {1050.0f, -1050.0f};
	constexpr float FloorY = 300.0f;

	const FColor Ohne(228, 104, 96);
	const FColor Mit(110, 200, 235);
	const FColor Rahmen(58, 60, 70);
	const FColor Grenze(240, 200, 110);
	const FColor Text(212, 218, 232);
}

APityRollDemoDirector::APityRollDemoDirector()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	BoardText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("BoardText"));
	BoardText->SetupAttachment(Root);
	BoardText->SetHorizontalAlignment(EHTA_Center);
	BoardText->SetVerticalAlignment(EVRTA_TextBottom);
	BoardText->SetWorldSize(54.0f);
	BoardText->SetTextRenderColor(FColor::White);
	// Yaw 270, not 90: at 90 a TextRender renders mirrored.
	BoardText->SetRelativeRotation(FRotator(0.0f, 270.0f, 0.0f));
	BoardText->SetRelativeLocation(FVector(0.0f, 0.0f, 790.0f));

	StateText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StateText"));
	StateText->SetupAttachment(Root);
	StateText->SetHorizontalAlignment(EHTA_Center);
	StateText->SetVerticalAlignment(EVRTA_TextTop);
	StateText->SetWorldSize(33.0f);
	StateText->SetTextRenderColor(PityRollDemoLocal::Text);
	StateText->SetRelativeRotation(FRotator(0.0f, 270.0f, 0.0f));
	StateText->SetRelativeLocation(FVector(-480.0f, 0.0f, 680.0f));

	RuleText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("RuleText"));
	RuleText->SetupAttachment(Root);
	RuleText->SetHorizontalAlignment(EHTA_Center);
	RuleText->SetVerticalAlignment(EVRTA_TextTop);
	RuleText->SetWorldSize(33.0f);
	RuleText->SetTextRenderColor(FColor(250, 205, 120));
	RuleText->SetRelativeRotation(FRotator(0.0f, 270.0f, 0.0f));
	RuleText->SetRelativeLocation(FVector(480.0f, 0.0f, 680.0f));
}

FVector APityRollDemoDirector::Basis() const
{
	return GetActorLocation() - FVector(0.0f, PityRollDemoLocal::FloorY, GetActorLocation().Z);
}

void APityRollDemoDirector::BeginPlay()
{
	Super::BeginPlay();
	StartCycle();
}

void APityRollDemoDirector::StartCycle()
{
	CycleTime = 0.0f;
	SeitLetzterRunde = 0.0f;
	Runden = 0;

	// A fixed seed, so the demo plays out the same way on every take and the sentence printed under
	// it stays true.
	Zustand = 0x1E3A7C5Du;

	Ohne = FSpalte();
	Ohne.Name = TEXT("NO PROTECTION");
	Ohne.Regeln = FPityRules();          // nothing: pure five percent

	Mit = FSpalte();
	Mit.Name = TEXT("PITYROLL");
	Mit.Regeln.SoftPityAfter = 10;
	Mit.Regeln.RampPerMiss = 0.01f;
	Mit.Regeln.HardPityAt = 40;
	Mit.Regeln.MaxChance = 1.0f;

	bBereit = true;
}

float APityRollDemoDirector::NaechsteZufallszahl()
{
	// A tiny xorshift. Deliberately NOT FMath::FRand: both columns have to be fed the SAME
	// sequence, or the picture proves nothing except that two runs differ.
	Zustand ^= Zustand << 13;
	Zustand ^= Zustand >> 17;
	Zustand ^= Zustand << 5;
	return static_cast<float>(Zustand % 100000u) / 100000.0f;
}

void APityRollDemoDirector::EineRunde()
{
	const float Zufall = NaechsteZufallszahl();
	++Runden;

	auto Wuerfeln = [&](FSpalte& S)
	{
		const int32 Vorher = S.Stand.Misses;
		const FPityResult E = UPityRollStatics::Roll(S.Stand, BasisChance, S.Regeln, Zufall);
		S.Stand = E.State;
		if (E.bGranted)
		{
			S.LaengsteDurststrecke = FMath::Max(S.LaengsteDurststrecke, Vorher);
			S.Strecken.Add(Vorher);
			while (S.Strecken.Num() > 26)
			{
				S.Strecken.RemoveAt(0);
			}
		}
		else
		{
			S.LaengsteDurststrecke = FMath::Max(S.LaengsteDurststrecke, S.Stand.Misses);
		}
	};

	Wuerfeln(Ohne);
	Wuerfeln(Mit);
}

void APityRollDemoDirector::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bAutoRun)
	{
		StepDemo(FMath::Clamp(DeltaSeconds, 0.0f, 0.1f));
	}
}

void APityRollDemoDirector::StepDemo(const float Seconds)
{
	// BeginPlay does not run in an editor viewport.
	if (!bBereit)
	{
		StartCycle();
	}

	CycleTime += Seconds;
	if (CycleTime > CycleSeconds)
	{
		StartCycle();
		return;
	}

	SeitLetzterRunde += Seconds;
	while (SeitLetzterRunde >= SekundenJeRunde)
	{
		SeitLetzterRunde -= SekundenJeRunde;
		EineRunde();
	}

	if (BoardText)
	{
		BoardText->SetText(FText::FromString(HeadlineFor()));
	}
	if (StateText)
	{
		StateText->SetText(FText::FromString(BuildStateText()));
	}
	if (RuleText)
	{
		RuleText->SetText(FText::FromString(RuleFor()));
	}

	if (bDrawDemo)
	{
		DrawBars();
	}
}

FString APityRollDemoDirector::HeadlineFor() const
{
	// Every headline is true for the WHOLE phase it covers.
	if (CycleTime < 9.0f)
	{
		return TEXT("the same rolls, the same five percent - one of them has a floor");
	}
	if (CycleTime < 18.0f)
	{
		return TEXT("the left column is having a bad run; the right one cannot");
	}
	return TEXT("however unlucky, never more than thirty-nine attempts in a row");
}

FString APityRollDemoDirector::RuleFor() const
{
	if (CycleTime < 9.0f)
	{
		return TEXT("THE SETUP\n\nboth columns get the SAME\nrandom sequence\n\nleft:  5%, nothing else\nright: 5%, +1 point per\n       miss from ten,\n       guaranteed at forty");
	}
	if (CycleTime < 18.0f)
	{
		return TEXT("THE RULE\n\nthe guarantee lands on\nexactly the attempt you\nnamed\n\nnot one early, not one\nlate - players count");
	}
	return TEXT("AND IT IS CONSUMED ONCE\n\na natural hit inside the\nwindow resets the counter\ntoo\n\nnobody gets the drop AND\nkeeps the protection");
}

FString APityRollDemoDirector::BuildStateText() const
{
	FString S = FString::Printf(TEXT("%d rolls at %.0f%%\n\n"), Runden, BasisChance * 100.0f);

	auto Zeile = [](const FSpalte& Sp) -> FString
	{
		return FString::Printf(TEXT("%s\n  dry now      %4d\n  longest dry  %4d\n  granted      %4d\n  by guarantee %4d\n  chance now  %5.1f%%\n\n"),
			*Sp.Name, Sp.Stand.Misses, Sp.LaengsteDurststrecke, Sp.Stand.Grants,
			Sp.Stand.HardPityGrants,
			UPityRollStatics::EffectiveChance(BasisChance, Sp.Stand.Misses, Sp.Regeln) * 100.0f);
	};

	S += Zeile(Ohne);
	S += Zeile(Mit);
	return S;
}

void APityRollDemoDirector::DrawBars() const
{
#if ENABLE_DRAW_DEBUG
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	UWorld* Mutable = const_cast<UWorld*>(World);

	// Persistent lines plus a flush at the start of every step - NOT lines with a lifetime. A
	// lifetime is counted down by the world tick, and an editor viewport that is not set to
	// realtime never ticks at all: the shapes would either pile up forever or never appear.
	FlushPersistentDebugLines(Mutable);

	using namespace PityRollDemoLocal;

	constexpr float Hoehe = 340.0f;
	constexpr float Fuss = 70.0f;
	constexpr float ProMiss = Hoehe / 60.0f;   // sixty misses fills the bar

	const FVector B = Basis();
	const FSpalte* Spalten[2] = {&Ohne, &Mit};
	const FColor Farben[2] = {PityRollDemoLocal::Ohne, PityRollDemoLocal::Mit};

	for (int32 i = 0; i < 2; ++i)
	{
		const float X = SpalteX[i];

		// The frame, and the line at forty where the guarantee sits.
		for (int32 Ply = 0; Ply < 5; ++Ply)
		{
			const FVector Unten = B + FVector(X + (Ply - 2) * 7.0f, 0.0f, Fuss);
			DrawDebugLine(Mutable, Unten, Unten + FVector(0.0f, 0.0f, Hoehe),
				Rahmen, true, -1.0f, 0, 4.0f);

			const float Jetzt = FMath::Min(Spalten[i]->Stand.Misses * ProMiss, Hoehe);
			if (Jetzt > 0.0f)
			{
				DrawDebugLine(Mutable, Unten, Unten + FVector(0.0f, 0.0f, Jetzt),
					Farben[i], true, -1.0f, 0, 13.0f);
			}
		}

		// The guarantee, drawn across both columns so the left one can visibly break it.
		const FVector Marke = B + FVector(X - 90.0f, 0.0f, Fuss + 40.0f * ProMiss);
		DrawDebugLine(Mutable, Marke, Marke + FVector(180.0f, 0.0f, 0.0f),
			Grenze, true, -1.0f, 0, 4.0f);

		// The dry streaks so far, as a row of small bars: the shape of the run, not just its worst
		// moment.
		const TArray<int32>& Strecken = Spalten[i]->Strecken;
		for (int32 k = 0; k < Strecken.Num(); ++k)
		{
			const float Sx = X - 250.0f + k * 20.0f;
			const float Hoch = FMath::Min(Strecken[k] * ProMiss, Hoehe);
			const FVector Unten = B + FVector(Sx, -260.0f, Fuss);
			DrawDebugLine(Mutable, Unten, Unten + FVector(0.0f, 0.0f, FMath::Max(4.0f, Hoch)),
				Farben[i], true, -1.0f, 0, 7.0f);
		}
	}
#endif
}
