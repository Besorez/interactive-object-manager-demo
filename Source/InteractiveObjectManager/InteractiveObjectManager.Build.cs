using UnrealBuildTool;
using System.Collections.Generic;

public class InteractiveObjectManager : ModuleRules
{
    public InteractiveObjectManager(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        OptimizeCode = CodeOptimization.Never;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "UMG",
                "CommonUI",
                "Slate",
                "SlateCore",
                "CommonUI"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Projects",
                "DeveloperSettings"
            }
        );
    }
}