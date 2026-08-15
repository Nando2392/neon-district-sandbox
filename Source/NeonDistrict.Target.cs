// Copyright Neon District Sandbox. Public benchmark repo — original content only.

using UnrealBuildTool;
using System.Collections.Generic;

public class NeonDistrictTarget : TargetRules
{
	public NeonDistrictTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		ExtraModuleNames.Add("NeonDistrict");
	}
}
