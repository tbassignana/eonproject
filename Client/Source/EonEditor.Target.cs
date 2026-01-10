// Copyright 2026 tbassignana. MIT License.

using UnrealBuildTool;
using System.Collections.Generic;

public class EonEditorTarget : TargetRules
{
	public EonEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
		ExtraModuleNames.Add("Eon");
	}
}
