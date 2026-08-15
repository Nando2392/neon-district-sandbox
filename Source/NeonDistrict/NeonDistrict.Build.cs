// Copyright Neon District Sandbox. Public benchmark repo — original content only.

using UnrealBuildTool;
using System.Collections.Generic;

public class NeonDistrict : ModuleRules
{
	public NeonDistrict(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

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
			"GameplayTasks"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"PhysicsCore",
			"Chaos"
		});
	}
}
