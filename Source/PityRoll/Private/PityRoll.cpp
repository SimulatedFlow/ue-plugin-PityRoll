// Copyright 2026 Silvan Teufel. All Rights Reserved.

#include "PityRoll.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "HAL/IConsoleManager.h"
#include "PityRollComponent.h"
#include "PityRollLog.h"
#include "PityRollStatics.h"

DEFINE_LOG_CATEGORY(LogPityRoll);

#define LOCTEXT_NAMESPACE "FPityRollModule"

namespace
{
	UWorld* PityRollConsoleWorld()
	{
		if (!GEngine)
		{
			return nullptr;
		}
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World() && (Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game))
			{
				return Context.World();
			}
		}
		return nullptr;
	}

	void PityRollDumpCommand()
	{
		UWorld* World = PityRollConsoleWorld();
		if (!World)
		{
			UE_LOG(LogPityRoll, Warning, TEXT("PityRoll.Dump: no running world."));
			return;
		}

		int32 Gefunden = 0;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			const UPityRollComponent* Pity = It->FindComponentByClass<UPityRollComponent>();
			if (!Pity)
			{
				continue;
			}
			++Gefunden;
			UE_LOG(LogPityRoll, Display, TEXT("%s"), *It->GetName());
			for (const FName Key : Pity->GetKnownKeys())
			{
				const FPityState S = Pity->GetState(Key);
				const int32 Bis = Pity->GetAttemptsUntilGuarantee(Key);
				UE_LOG(LogPityRoll, Display,
					TEXT("    %-18s misses %4d  attempts %5d  grants %4d (%d by guarantee)  %s"),
					*Key.ToString(), S.Misses, S.Attempts, S.Grants, S.HardPityGrants,
					Bis < 0 ? TEXT("no guarantee")
					        : *FString::Printf(TEXT("%d until the guarantee"), Bis));
			}
		}

		if (Gefunden == 0)
		{
			UE_LOG(LogPityRoll, Display, TEXT("PityRoll.Dump: nothing in this level has a pity component."));
		}
	}

	FAutoConsoleCommand GPityRollDump(
		TEXT("PityRoll.Dump"),
		TEXT("Every pity counter in the level: misses, attempts, grants, and how far the guarantee is."),
		FConsoleCommandDelegate::CreateStatic(&PityRollDumpCommand));
}

void FPityRollModule::StartupModule()
{
}

void FPityRollModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPityRollModule, PityRoll)
