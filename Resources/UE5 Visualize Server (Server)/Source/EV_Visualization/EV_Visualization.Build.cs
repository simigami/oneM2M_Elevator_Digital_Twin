// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class EV_Visualization : ModuleRules
{
	public EV_Visualization(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "Sockets", "Networking", "Json", "JsonUtilities" });
	}
}
