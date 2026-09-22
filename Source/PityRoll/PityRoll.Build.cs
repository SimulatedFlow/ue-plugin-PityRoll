// Copyright 2026 Silvan Teufel. All Rights Reserved.

using UnrealBuildTool;

public class PityRoll : ModuleRules
{
	public PityRoll(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",

			// AActor, UActorComponent and DrawDebugHelpers for the demo level.
			"Engine",

			// UPityRollSettings is a UDeveloperSettings, so the thresholds sit under
			// Project Settings > Plugins > PityRoll without an editor module.
			"DeveloperSettings",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});

		// Deliberately NOT here:
		//   anything that owns a random stream. The random value is handed IN, so a test, a replay
		//   and a dedicated server all get the same answer from the same inputs. A plugin that
		//   draws its own numbers cannot be replayed and cannot be tested at a boundary.
	}
}
