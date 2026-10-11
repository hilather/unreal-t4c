#include "Visual/LHEnvArtBinding.h"

FString LHEnvArt::Folder(ELHVisualStyle Style)
{
    switch(Style)
    {
    case ELHVisualStyle::Church: return TEXT("Church");
    case ELHVisualStyle::B1Cellar: return TEXT("B1");
    case ELHVisualStyle::B2Damp: return TEXT("B2Damp");
    case ELHVisualStyle::B3Crypt: return TEXT("B3Crypt");
    case ELHVisualStyle::B4Ritual: return TEXT("B4Ritual");
    default: return FString();
    }
}
FString LHEnvArt::MeshFolder(ELHVisualStyle Style, const FString& Name)
{
    const bool Common=Name==TEXT("Sconce") || Name==TEXT("Barrel") || Name==TEXT("Crate") || Name==TEXT("Table") || Name==TEXT("Bench");
    const bool B1Geometry=(Style==ELHVisualStyle::Church || Style==ELHVisualStyle::B2Damp) &&
        (Name==TEXT("Debris") || Name==TEXT("Arch240") || Name==TEXT("Arch320") || Name==TEXT("Stair600x120") || Name==TEXT("Stair600x120Descending"));
    return Common || B1Geometry?TEXT("B1"):Folder(Style);
}
const TArray<FString>& LHEnvArt::AssetNames(ELHVisualStyle Style)
{
    static const TArray<FString> Church=[] { auto N=LHB1Art::AssetNames(); N.Append({TEXT("Altar"),TEXT("Pillar"),TEXT("DoorLeafPreview"),TEXT("Carpet")}); return N; }();
    static const TArray<FString> Ritual=[] { auto N=LHB1Art::AssetNames(); N.Append({TEXT("Altar"),TEXT("ArchBoss500")}); return N; }();
    if(Style==ELHVisualStyle::Church) return Church;
    if(Style==ELHVisualStyle::B4Ritual) return Ritual;
    return LHB1Art::AssetNames();
}
bool LHEnvArt::Resolve(const FLHVisualRecipe& R, FFit& Out)
{
    if(Folder(R.Style).IsEmpty() || R.Geometry.IsEmpty()) return false;
    if(R.Style==ELHVisualStyle::B1Cellar) return LHB1Art::Resolve(R,Out);
    auto Canonical=R; Canonical.Style=ELHVisualStyle::B1Cellar;
    if(LHB1Art::Resolve(Canonical,Out)) return true;
    FString Name=R.Id.ToString();
    bool Allowed=Name.RemoveFromStart(TEXT("Presentation.Environment.Shared."));
    if(!Allowed && R.Style==ELHVisualStyle::Church) Allowed=Name.RemoveFromStart(TEXT("Presentation.Environment.Church."));
    if(!Allowed && R.Style==ELHVisualStyle::B4Ritual) Allowed=Name.RemoveFromStart(TEXT("Presentation.Environment.Basement."));
    if(!Allowed || !AssetNames(R.Style).Contains(Name) || LHB1Art::AssetNames().Contains(Name)) return false;
    FFit Fit; Fit.AssetName=Name; Fit.Instances.Add(FTransform::Identity); Out=MoveTemp(Fit); return true;
}

TArray<FLHVisualBox> LHEnvArt::SolidBacking(const FLHVisualRecipe& R, const FFit& Fit)
{
    if(R.Style==ELHVisualStyle::B1Cellar) return LHB1Art::SolidBacking(R,Fit);
    auto Canonical=R; Canonical.Style=ELHVisualStyle::B1Cellar;
    auto Boxes=LHB1Art::SolidBacking(Canonical,Fit);
    if(R.Style==ELHVisualStyle::B3Crypt || R.Style==ELHVisualStyle::B4Ritual)
        for(auto& B:Boxes)
        {
            // Close cropped masonry at its rear plane; keep the complete
            // front recess depth free of the B1 ten-centimeter core.
            const double Rear=B.Center.Y-B.Size.Y/2;
            B.Size.Y=1.; B.Center.Y=Rear+.5;
        }
    return Boxes;
}
TArray<FLHVisualBox> LHEnvArt::MasonryCaps(const FLHVisualRecipe& R, const FFit& Fit)
{
    auto Canonical=R; Canonical.Style=ELHVisualStyle::B1Cellar;
    return LHB1Art::MasonryCaps(Canonical,Fit);
}
