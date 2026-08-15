// Copyright Neon District Sandbox. Public benchmark repo — original content only.

using UnrealBuildTool;
using System.Collections.Generic;

public class NeonDistrictEditorTarget : TargetRules
{
	public NeonDistrictEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		bOverrideBuildEnvironment = true;
		ExtraModuleNames.Add("NeonDistrict");
	}
}
