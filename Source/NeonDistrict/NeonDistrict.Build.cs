// Copyright Neon District Sandbox. Public benchmark repo — original content only.

using UnrealBuildTool;
using System.Collections.Generic;

public class NeonDistrict : ModuleRules
{
	public NeonDistrict(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Our module uses subfolder-relative includes ("Player/X.h"), so the
		// module root must be on the include path (UBT only adds Source/).
		PrivateIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"ChaosVehicles",
			"Niagara",
			"UMG",
			"Slate",
			"SlateCore",
			"AIModule",
			"NavigationSystem",
			"GameplayTasks",
			"AudioMixer"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"PhysicsCore",
			"Chaos"
		});

		if (Target.bBuildEditor)
		{
			// Editor-only: MaterialEditor for runtime material creation
			// Note: AssetTools/ObjectTools removed - using AssetRegistry instead
			PrivateDependencyModuleNames.AddRange(new string[] {
				"UnrealEd",
				"MaterialEditor",
				"AssetRegistry"
			});

			// Add any needed editor-only classes
			PublicIncludePaths.Add(ModuleDirectory + "/Editor");
		}
	}
}