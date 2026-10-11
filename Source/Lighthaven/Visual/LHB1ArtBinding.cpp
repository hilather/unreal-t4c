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
        // Imported 3.8cm bevels and 5cm-deep mortar need actual recesses.
        // A 0.1cm inset filled both, leaving a continuous plaster-like face.
        B.Size.Y=FMath::Max(1.,B.Size.Y-10.);
        // Coping stones retain the exact silhouette; mortar bed recedes at
        // their joints on all four cropped edges rather than forming a flush cut.
        B.Size.X=FMath::Max(.1,B.Size.X-3.);
        B.Size.Z=FMath::Max(.1,B.Size.Z-3.); Out.Add(B);
    }
    else if(Fit.AssetName==TEXT("Arch240") || Fit.AssetName==TEXT("Arch320"))
    {
        for(int32 I=0;I<3 && R.Geometry.IsValidIndex(I);++I)
        {
            auto B=R.Geometry[I]; B.Size.Y=FMath::Max(1.,B.Size.Y-10.);
            // Preserve the imported segmental opening below 357cm, not a rectangular lintel.
            if(I==2) { B.Center.Z=378.5; B.Size.Z=43; }
            if(I==0) { B.Center.X+=.75; B.Size.X=FMath::Max(.1,B.Size.X-1.5); }
            if(I==1) { B.Center.X-=.75; B.Size.X=FMath::Max(.1,B.Size.X-1.5); }
            // Every upright and crown meets the coping bed at z=398.5.
            // Its lower edge and each inner jamb remain unchanged.
            B.Center.Z-=.75; B.Size.Z=FMath::Max(.1,B.Size.Z-1.5);
            Out.Add(B);
        }
    }
    return Out;
}

TArray<FLHVisualBox> LHB1Art::MasonryCaps(const FLHVisualRecipe& R, const FFit& Fit)
{
    TArray<FLHVisualBox> Out;
    if(R.Style!=ELHVisualStyle::B1Cellar) return Out;
    const bool Wall=Fit.AssetName==TEXT("Wall400");
    const bool Arch=Fit.AssetName==TEXT("Arch240") || Fit.AssetName==TEXT("Arch320");
    if(!Wall && !Arch) return Out;
    FBox Bounds=Fit.ClipBounds;
    if(Arch)
    {
        Bounds=FBox(ForceInit);
        for(int32 I=0;I<3 && R.Geometry.IsValidIndex(I);++I)
            Bounds+=FBox(R.Geometry[I].Center-R.Geometry[I].Size/2,R.Geometry[I].Center+R.Geometry[I].Size/2);
    }
    if(!Bounds.IsValid || Bounds.GetSize().GetMin()<=0) return Out;
    const FVector Lo=Bounds.Min, Hi=Bounds.Max, Mid=Bounds.GetCenter();
    // Prototype 6cm coping depth and 1.6cm joints. The recessed solid core
    // beneath it closes the mesh while these full-width stones catch warm fill.
    // Vary course lengths deterministically; never alter the aperture or silhouette.
    const auto Course=[&](bool AlongX,double Fixed,bool Upper,int32 Seed)
    {
        const double Depth=FMath::Min(6.,(AlongX?Hi.Z-Lo.Z:Hi.X-Lo.X)/2);
        // Horizontal coping owns the corners. End stones stop at its lower
        // face, avoiding overlapping boxes and coplanar faces with different UVs.
        const double HorizontalDepth=FMath::Min(6.,(Hi.Z-Lo.Z)/2);
        const double Start=AlongX?Lo.X:Lo.Z+(Wall?HorizontalDepth:0.);
        const double End=AlongX?Hi.X:Hi.Z-HorizontalDepth;
        double Cursor=Start; int32 Index=Seed;
        while(Cursor<End-KINDA_SMALL_NUMBER)
        {
            double Next=FMath::Min(End,Cursor+(Index%3==0?72.:Index%3==1?106.:88.));
            if(End-Next<16.) Next=End; // avoid slivers at arbitrary saved crop lengths
            const double InsetStart=Cursor>Start?.8:0., InsetEnd=Next<End?.8:0.;
            FLHVisualBox B;
            const double Center=(Cursor+InsetStart+Next-InsetEnd)/2;
            const double Length=Next-Cursor-InsetStart-InsetEnd;
            if(AlongX)
            {
                B.Center={Center,Mid.Y,Fixed+(Upper?-Depth/2:Depth/2)};
                B.Size={Length,Hi.Y-Lo.Y,Depth};
            }
            else
            {
                B.Center={Fixed+(Upper?-Depth/2:Depth/2),Mid.Y,Center};
                B.Size={Depth,Hi.Y-Lo.Y,Length};
            }
            if(Length>0) Out.Add(B);
            Cursor=Next; ++Index;
        }
    };
    Course(true,Hi.Z,true,0);
    if(Wall) Course(true,Lo.Z,false,1);
    // Arch outer jamb faces close the ends only; no cap crosses its opening.
    Course(false,Lo.X,false,1);
    Course(false,Hi.X,true,2);
    return Out;
}
