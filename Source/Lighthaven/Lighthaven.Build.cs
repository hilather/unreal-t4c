// DRAFT schema rev 1 — NOT COMPILED (no Unreal Engine installed); integrator review pending
using UnrealBuildTool;

public class Lighthaven : ModuleRules
{
    public Lighthaven(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicIncludePaths.Add(ModuleDirectory);
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "GameplayTags", "GameplayAbilities", "GameplayTasks", "EnhancedInput", "InputCore", "UMG", "Slate", "SlateCore" });
    }
}
