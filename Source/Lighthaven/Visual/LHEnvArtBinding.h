#pragma once
#include "Visual/LHB1ArtBinding.h"

namespace LHEnvArt
{
    using FFit = LHB1Art::FFit;
    LIGHTHAVEN_API FString Folder(ELHVisualStyle Style);
    LIGHTHAVEN_API FString MeshFolder(ELHVisualStyle Style, const FString& Name);
    LIGHTHAVEN_API const TArray<FString>& AssetNames(ELHVisualStyle Style);
    LIGHTHAVEN_API TArray<FLHVisualBox> SolidBacking(const FLHVisualRecipe& Recipe, const FFit& Fit);
    LIGHTHAVEN_API TArray<FLHVisualBox> MasonryCaps(const FLHVisualRecipe& Recipe, const FFit& Fit);
    LIGHTHAVEN_API bool Resolve(const FLHVisualRecipe& Recipe, FFit& Out);
}
