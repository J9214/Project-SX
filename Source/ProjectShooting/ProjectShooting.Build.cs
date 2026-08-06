// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class ProjectShooting : ModuleRules
{
	public ProjectShooting(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"DeveloperSettings",
			"UMG",
			"Slate",
			"Niagara"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"ProjectShooting",
			"ProjectShooting/Variant_Horror",
			"ProjectShooting/Variant_Horror/UI",
			"ProjectShooting/Variant_Shooter",
			"ProjectShooting/Variant_Shooter/AI",
			"ProjectShooting/Variant_Shooter/UI",
			"ProjectShooting/Variant_Shooter/Weapons"
		});

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			string StoveSDKDirectory = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../ThirdParty/StovePCSDK"));
			string StoveIncludeDirectory = Path.Combine(StoveSDKDirectory, "Include");
			string StoveLibraryPath = Path.Combine(StoveSDKDirectory, "Lib/Win64/BaseSDK.lib");
			string StoveDLLPath = Path.Combine(StoveSDKDirectory, "Dll/Win64/BaseSDK.dll");

			PublicSystemIncludePaths.Add(StoveIncludeDirectory);
			PublicAdditionalLibraries.Add(StoveLibraryPath);
			PublicDelayLoadDLLs.Add("BaseSDK.dll");
			RuntimeDependencies.Add("$(TargetOutputDir)/BaseSDK.dll", StoveDLLPath, StagedFileType.NonUFS);
		}

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
