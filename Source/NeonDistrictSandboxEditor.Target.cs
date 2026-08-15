// Copyright Neon District Sandbox. Public benchmark repo — original content only.

using UnrealBuildTool;
using System.Collections.Generic;

public class NeonDistrictSandboxEditorTarget : TargetRules
{
	public NeonDistrictSandboxEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		bOverrideBuildEnvironment = true;
		ExtraModuleNames.Add("NeonDistrict");
	}
}