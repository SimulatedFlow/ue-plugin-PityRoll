// Copyright 2026 Silvan Teufel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PityRollTypes.h"
#include "PityRollDemoDirector.generated.h"

class UTextRenderComponent;

/**
 * Runs the shipped demo: the same rolls, the same base chance, one of them with a floor under it.
 *
 * Both columns are fed the SAME pseudo-random sequence from a fixed seed, so the difference on
 * screen is the protection and nothing else. A demo where the two sides roll different numbers
 * proves nothing at all.
 *
 * Every decision comes from UPityRollStatics::Roll, which is the plugin; the component is a map of
 * counters around exactly that call, and the tests call it too.
 */
UCLASS(meta = (DisplayName = "PityRoll Demo Director"))
class PITYROLL_API APityRollDemoDirector : public AActor
{
	GENERATED_BODY()

public:
	APityRollDemoDirector();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PityRoll Demo",
		meta = (ClampMin = "16.0", ClampMax = "180.0"))
	float CycleSeconds = 26.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PityRoll Demo")
	bool bDrawDemo = true;

	/** Let Tick drive the demo. Off when something else steps it - see StepDemo. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PityRoll Demo")
	bool bAutoRun = true;

	/**
	 * Advance the demo by exactly this many seconds and redraw.
	 *
	 * An actor only ticks in an editor viewport while that viewport is set to realtime, which is
	 * the user's setting and not the plugin's. The screenshot run steps the demo by hand instead.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "PityRoll Demo")
	void StepDemo(float Seconds);

	/** The headline. TextRender, because HighResShot does not capture DrawDebugString. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PityRoll Demo")
	TObjectPtr<UTextRenderComponent> BoardText;

	/** The two columns, in words. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PityRoll Demo")
	TObjectPtr<UTextRenderComponent> StateText;

	/** The rule the current phase is showing. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PityRoll Demo")
	TObjectPtr<UTextRenderComponent> RuleText;

private:
	/** One column: the plugin's state, plus what the run has looked like. */
	struct FSpalte
	{
		FPityState Stand;
		FPityRules Regeln;
		FString Name;
		int32 LaengsteDurststrecke = 0;
		TArray<int32> Strecken;      // the dry streaks, in order, for the bars
	};

	void StartCycle();
	void EineRunde();
	float NaechsteZufallszahl();
	FString BuildStateText() const;
	FString HeadlineFor() const;
	FString RuleFor() const;
	void DrawBars() const;

	FVector Basis() const;

	FSpalte Ohne;
	FSpalte Mit;

	/** A tiny fixed-seed generator, so the demo plays out the same way on every take. */
	uint32 Zustand = 0;

	float CycleTime = 0.0f;
	float SeitLetzterRunde = 0.0f;
	int32 Runden = 0;
	bool bBereit = false;

	static constexpr float BasisChance = 0.05f;
	static constexpr float SekundenJeRunde = 0.12f;
};
