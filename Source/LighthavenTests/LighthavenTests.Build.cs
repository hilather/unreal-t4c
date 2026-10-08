// DRAFT schema rev 1 — NOT COMPILED (no Unreal Engine installed); integrator review pending
using UnrealBuildTool;

public class LighthavenTests : ModuleRules
{
    public LighthavenTests(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        // Direct dependencies for the native PlayerInput/mapping regression fixture.
        PrivateDependencyModuleNames.AddRange(new string[] { "InputCore", "EnhancedInput" });
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "Lighthaven" });
    }
}
