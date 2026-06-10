using UnrealBuildTool;
using System.IO;

public class CustomSwatches : ModuleRules
{
    public CustomSwatches(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "Json",
            "JsonUtilities",
            "InputCore",
        });
        
        PrivateDependencyModuleNames.AddRange(new string[] {
            "FactoryGame",
            "SML",
        });
        
        // Public include paths for the mod
        PublicIncludePaths.AddRange(new string[] {
            Path.Combine(ModuleDirectory, "Public"),
        });
        
        // Optimizations
        if (Target.Configuration == UnrealBuildTool.UnrealTargetConfiguration.Shipping)
        {
            OptimizeCode = CodeOptimization.Aggressive;
        }
        
        // Link against SML
        PublicDefinitions.Add("WITH_SML=1");
    }
}