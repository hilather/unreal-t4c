// DRAFT schema rev 1 — NOT COMPILED (no Unreal Engine installed); integrator review pending
using UnrealBuildTool;
using System.Collections.Generic;

public class LighthavenEditorTarget : TargetRules
{
    public LighthavenEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.AddRange(new string[] { "Lighthaven", "LighthavenEditor", "LighthavenTests" });
    }
}
