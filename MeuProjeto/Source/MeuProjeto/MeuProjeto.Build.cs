// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MeuProjeto : ModuleRules
{
	public MeuProjeto(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"MeuProjeto",
			"MeuProjeto/Variant_Platforming",
			"MeuProjeto/Variant_Platforming/Animation",
			"MeuProjeto/Variant_Combat",
			"MeuProjeto/Variant_Combat/AI",
			"MeuProjeto/Variant_Combat/Animation",
			"MeuProjeto/Variant_Combat/Gameplay",
			"MeuProjeto/Variant_Combat/Interfaces",
			"MeuProjeto/Variant_Combat/UI",
			"MeuProjeto/Variant_SideScrolling",
			"MeuProjeto/Variant_SideScrolling/AI",
			"MeuProjeto/Variant_SideScrolling/Gameplay",
			"MeuProjeto/Variant_SideScrolling/Interfaces",
			"MeuProjeto/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
