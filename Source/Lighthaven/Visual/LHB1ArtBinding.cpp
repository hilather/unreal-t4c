#include "Visual/LHB1ArtBinding.h"

const TArray<FString>& LHB1Art::AssetNames()
{
    static const TArray<FString> Names={TEXT("Wall400"),TEXT("Floor400"),TEXT("Stair600x120"),
        TEXT("Stair600x120Descending"),TEXT("Arch240"),TEXT("Arch320"),TEXT("Sconce"),
        TEXT("Barrel"),TEXT("Crate"),TEXT("Debris"),TEXT("Table"),TEXT("Bench")};
    return Names;
}

bool LHB1Art::Resolve(const FLHVisualRecipe& R, FFit& Out)
{
    if(R.Style!=ELHVisualStyle::B1Cellar || R.Geometry.IsEmpty()) return false;
    FString Name=R.Id.ToString();
    if(!Name.RemoveFromStart(TEXT("Presentation.Environment.Shared.")) || !AssetNames().Contains(Name)) return false;
    FFit Fit; Fit.AssetName=Name;
    const auto& First=R.Geometry[0];
    if(Name==TEXT("Wall400") || Name==TEXT("Floor400"))
    {
        const bool Floor=Name==TEXT("Floor400");
        const FVector Nominal=Floor?FVector(400,400,18):FVector(400,18,400);
        const FVector Center=Floor?FVector(200,200,-11):FVector(200,-10,200);
        const FVector Scale=First.Size/Nominal;
        const FVector Origin=First.Center-Center*Scale;
        // Thickness is never tiled. Keep it within the requested 15% limit.
        const double Thickness=Floor?Scale.Z:Scale.Y;
        if(Scale.ContainsNaN() || Scale.GetMin()<=0 || Thickness<.85 || Thickness>1.15) return false;
        const double Length=400*Scale.X, Other=400*(Floor?Scale.Y:Scale.Z);
        const int32 NX=FMath::CeilToInt(Length/400), NY=FMath::CeilToInt(Other/400);
        if(NX*int64(NY)>256) return false; // bound presentation instance cost
        const FVector MeshScale=Floor?FVector(1,1,Thickness):FVector(1,Thickness,1);
        Fit.ClipBounds=Floor?FBox(Origin+FVector(0,0,-20*Thickness),Origin+FVector(Length,Other,0)):
            FBox(Origin+FVector(0,-20*Thickness,0),Origin+FVector(Length,0,Other));
        for(int32 X=0;X<NX;++X) for(int32 Y=0;Y<NY;++Y)
            Fit.Instances.Add(FTransform(FQuat::Identity,Origin+(Floor?FVector(X*400,Y*400,0):FVector(X*400,0,Y*400)),MeshScale));
    }
    else if(Name==TEXT("Stair600x120"))
    {
        const FVector Path=First.Rotation.RotateVector(FVector(First.Size.X,0,0));
        const double Run=Path.X, Rise=Path.Z;
        if(Run<=0 || Run>600 || FMath::Abs(Rise)<1 || !FMath::IsNearlyEqual(First.Size.Y,300.,.1)) return false;
        const bool Descending=Rise<0;
        if(Descending) Fit.AssetName+=TEXT("Descending");
        const double CanonicalAngle=FMath::RadiansToDegrees(FMath::Atan2(Descending?-120.:120.,600.));
        const FRotator Rotation(First.Rotation.Pitch-CanonicalAngle,0,0);
        // First slab top midpoint is the authoritative saved path midpoint.
        const FVector Midpoint=First.Center+First.Rotation.RotateVector(FVector::UpVector)*10;
        const FVector Start=Midpoint-Path/2;
        Fit.Instances.Add(FTransform(Rotation,Start));
        // Crop at the exact path endpoints; preserve slab normal thickness.
        Fit.ClipBounds=FBox(FVector(Start.X,-150,FMath::Min(Start.Z,Start.Z+Rise)-30),
            FVector(Start.X+Run,150,FMath::Max(Start.Z,Start.Z+Rise)+5));
    }
    else Fit.Instances.Add(FTransform::Identity);
    Out=MoveTemp(Fit); return true;
}

TArray<FLHVisualBox> LHB1Art::SolidBacking(const FLHVisualRecipe& R, const FFit& Fit)
{
    TArray<FLHVisualBox> Out;
    if(R.Style!=ELHVisualStyle::B1Cellar) return Out;
    if(Fit.AssetName==TEXT("Wall400"))
    {
        FLHVisualBox B; B.Center=Fit.ClipBounds.GetCenter(); B.Size=Fit.ClipBounds.GetSize();
        // Slightly inset broad faces retain the imported bevels and authored UV density.
        B.Size.Y-=.2; Out.Add(B);
    }
    else if(Fit.AssetName==TEXT("Arch240") || Fit.AssetName==TEXT("Arch320"))
    {
        for(int32 I=0;I<3 && R.Geometry.IsValidIndex(I);++I)
        {
            auto B=R.Geometry[I]; B.Size.Y-=.2;
            // Preserve the imported segmental opening below 357cm, not a rectangular lintel.
            if(I==2) { B.Center.Z=378.5; B.Size.Z=43; }
            Out.Add(B);
        }
    }
    return Out;
}
