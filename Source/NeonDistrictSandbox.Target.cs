// Copyright Neon District Sandbox. Public benchmark repo — original content only

using UnrealBuildTool;
using System.Collections.Generic;

public class NeonDistrictSandboxTarget : TargetRules
{
	public NeonDistrictSandboxTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange(new string[] { "NeonDistrict" });
	}
}