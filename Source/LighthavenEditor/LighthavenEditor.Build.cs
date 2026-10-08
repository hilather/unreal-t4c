// Editor-only validation and reproducible dev-map generation dependencies.
using UnrealBuildTool;

public class LighthavenEditor : ModuleRules
{
    public LighthavenEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "Lighthaven" });
        PrivateDependencyModuleNames.AddRange(new string[] { "UnrealEd", "DataValidation", "NavigationSystem" });
    }
}
