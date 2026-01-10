// Copyright 2026 tbassignana. MIT License.

using UnrealBuildTool;
using System.Collections.Generic;

public class EonTarget : TargetRules
{
	public EonTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
		ExtraModuleNames.Add("Eon");
	}
}
