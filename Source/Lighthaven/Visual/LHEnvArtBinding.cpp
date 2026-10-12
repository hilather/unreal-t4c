#include "Visual/LHEnvArtBinding.h"
#include "Visual/LHFloorLighting.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

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
static bool ResolveUnclipped(const FLHVisualRecipe& R, LHEnvArt::FFit& Out)
{
    if(LHEnvArt::Folder(R.Style).IsEmpty() || R.Geometry.IsEmpty()) return false;
    if(R.Style==ELHVisualStyle::B1Cellar) return LHB1Art::Resolve(R,Out);
    auto Canonical=R; Canonical.Style=ELHVisualStyle::B1Cellar;
    if(LHB1Art::Resolve(Canonical,Out)) return true;
    FString Name=R.Id.ToString();
    bool Allowed=Name.RemoveFromStart(TEXT("Presentation.Environment.Shared."));
    if(!Allowed && R.Style==ELHVisualStyle::Church) Allowed=Name.RemoveFromStart(TEXT("Presentation.Environment.Church."));
    if(!Allowed && R.Style==ELHVisualStyle::B4Ritual) Allowed=Name.RemoveFromStart(TEXT("Presentation.Environment.Basement."));
    if(!Allowed || !LHEnvArt::AssetNames(R.Style).Contains(Name) || LHB1Art::AssetNames().Contains(Name)) return false;
    LHEnvArt::FFit Fit; Fit.AssetName=Name;
    if(Name==TEXT("DoorLeafPreview"))
    {
        // Use the first saved solid as the fit authority. Crop the imported
        // leaf instead of squashing its plank/hinge detail into a near cutaway.
        const auto& B=R.Geometry[0];
        const double Height=B.Size.Z;
        if(Height<=0 || Height>300 || !FMath::IsNearlyEqual(B.Size.X,160.,.1)) return false;
        Fit.ClipBounds=FBox(FVector(0,-5,0),FVector(160,5,Height));
    }
    Fit.Instances.Add(FTransform::Identity); Out=MoveTemp(Fit); return true;
}

bool LHEnvArt::Resolve(const FLHVisualRecipe& R, FFit& Out)
{
    if(!ResolveUnclipped(R,Out)) return false;
    // Saved marker belongs only to the exterior apron, never the chapel kit.
    if(R.Style==ELHVisualStyle::Church && R.Id==TEXT("Presentation.Environment.Shared.Floor400")
        && R.Colors[0].Equals(FLinearColor(.061f,.081f,.11f,1.f),.00001f))
    {
        const FBox Bounds=Out.ClipBounds;
        const FVector Size=Bounds.GetSize();
        const double Thickness=Size.Z/20.;
        const FVector Origin(Bounds.Min.X,Bounds.Min.Y,Bounds.Max.Z);
        Out.Instances.Reset();
        for(int32 X=0;X<FMath::CeilToInt(Size.X/800.);++X)
        for(int32 Y=0;Y<FMath::CeilToInt(Size.Y/800.);++Y)
            Out.Instances.Add(FTransform(FQuat::Identity,Origin+FVector(X*800.,Y*800.,0),FVector(2,2,Thickness)));
    }
    if(R.Id.ToString().Contains(TEXT(".Arch")))
    {
        double Top=-DBL_MAX;
        for(const auto& B:R.Geometry) Top=FMath::Max(Top,B.Center.Z+B.Size.Z/2);
        // Saved cropped jambs are the fit authority; preserve stone scale.
        Out.ClipBounds.Max.Z=FMath::Min(Out.ClipBounds.Max.Z,Top);
    }
    return true;
}

TArray<FLHVisualBox> LHEnvArt::SolidBacking(const FLHVisualRecipe& R, const FFit& Fit)
{
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
    for(int32 I=Boxes.Num()-1;I>=0;--I)
    {
        auto& B=Boxes[I];
        const double Bottom=B.Center.Z-B.Size.Z/2;
        const double Top=FMath::Min(Fit.ClipBounds.Max.Z,B.Center.Z+B.Size.Z/2);
        if(Top<=Bottom) { Boxes.RemoveAt(I); continue; }
        B.Center.Z=(Bottom+Top)/2; B.Size.Z=Top-Bottom;
    }
    return Boxes;
}
TArray<FLHVisualBox> LHEnvArt::MasonryCaps(const FLHVisualRecipe& R, const FFit& Fit)
{
    auto Canonical=R; Canonical.Style=ELHVisualStyle::B1Cellar;
    if(R.Id.ToString().Contains(TEXT(".Arch")) && Fit.ClipBounds.Max.Z<399.)
    {
        TArray<FLHVisualBox> Caps;
        // Cap each surviving jamb separately, never bridge the clear aperture.
        for(const auto& B:R.Geometry)
        {
            FFit JambFit; JambFit.AssetName=TEXT("Wall400");
            JambFit.ClipBounds=FBox(B.Center-B.Size/2,B.Center+B.Size/2);
            Caps.Append(LHB1Art::MasonryCaps(Canonical,JambFit));
        }
        return Caps;
    }
    return LHB1Art::MasonryCaps(Canonical,Fit);
}

void LHFloorLighting::ConfigureChurchExterior(ALHVisualPiece* Piece)
{
    if(!Piece) return;
    const bool Exterior=Piece->GetRecipe().Style==ELHVisualStyle::Church
        && Piece->Tags.Contains(TEXT("LH.Church.Surroundings"));
    TArray<UPrimitiveComponent*> Surfaces;
    Piece->GetComponents(Surfaces);
    for(auto* Surface:Surfaces)
    {
        if(Exterior)
        {
            Surface->SetLightingChannels(false,true,false);
            Surface->ComponentTags.AddUnique(TEXT("LH.Church.ExteriorSurface"));
        }
        else if(Surface->ComponentTags.Remove(TEXT("LH.Church.ExteriorSurface"))>0)
            Surface->SetLightingChannels(true,false,false);
    }
    if(!Exterior) return;
    auto* Imported=Piece->GetImportedMesh();
    if(!Imported) return;
    const FVector Position=Piece->GetActorLocation();
    const bool Mage=Position.Y>4000.;
    const FVector2D Low=Mage?FVector2D(-5200,5600):FVector2D(-1800,-1600);
    const FVector2D High=Mage?FVector2D(-1200,8800):FVector2D(2400,3000);
    const double EdgeDistance=FMath::Min(FMath::Min(Position.X-Low.X,High.X-Position.X),
        FMath::Min(Position.Y-Low.Y,High.Y-Position.Y));
    // The existing imported master blends texture albedo and adds the same
    // low-energy color as emission. Retain textured relief in the inner apron,
    // then soften its outer band into blue haze without touching global fog.
    const float Edge=1.f-FMath::Clamp(float(EdgeDistance/1000.),0.f,1.f);
    const float Smooth=Edge*Edge*(3.f-2.f*Edge);
    const FLinearColor Color=FMath::Lerp(FLinearColor(.035f,.055f,.095f,1.f),
        FLinearColor(.11f,.17f,.28f,1.f),Smooth);
    for(int32 Slot=0;Slot<Imported->GetNumMaterials();++Slot)
        if(auto* Material=Cast<UMaterialInstanceDynamic>(Imported->GetMaterial(Slot)))
        {
            Material->SetScalarParameterValue(TEXT("Flame"),FMath::Lerp(.82f,.96f,Smooth));
            Material->SetVectorParameterValue(TEXT("FlameColor"),Color);
        }
}
