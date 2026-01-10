// Copyright 2026 tbassignana. MIT License.

using UnrealBuildTool;

public class Eon : ModuleRules
{
	public Eon(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"UMG",
			"Slate",
			"SlateCore",
			"HTTP",
			"Json",
			"JsonUtilities",
			"WebSockets"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks"
		});

		// iOS specific settings
		if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			PublicDefinitions.Add("PLATFORM_IOS=1");
		}

		// Mac specific settings
		if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			PublicDefinitions.Add("PLATFORM_MAC=1");
		}
	}
}
