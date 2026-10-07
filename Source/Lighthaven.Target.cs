// DRAFT schema rev 1 — NOT COMPILED (no Unreal Engine installed); integrator review pending
using UnrealBuildTool;
using System.Collections.Generic;

public class LighthavenTarget : TargetRules
{
    public LighthavenTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.AddRange(new string[] { "Lighthaven" });
    }
}
