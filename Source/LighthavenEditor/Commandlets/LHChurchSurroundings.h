#pragma once
#include "CoreMinimal.h"
#include "Visual/LHVisualKit.h"

// Imported catalog pieces only. The apron fills presentation void, never the
// footprint of a playable slab (including the descending wing ramp).
namespace LHChurchSurroundings
{
inline bool OverlapXY(const FBox& A,const FBox& B)
{
    return A.Min.X<B.Max.X && A.Max.X>B.Min.X && A.Min.Y<B.Max.Y && A.Max.Y>B.Min.Y;
}
inline void Subtract(const FBox& Source,const FBox& Hole,TArray<FBox>& Out)
{
    if(!OverlapXY(Source,Hole)) {Out.Add(Source); return;}
    const double X0=FMath::Max(Source.Min.X,Hole.Min.X),X1=FMath::Min(Source.Max.X,Hole.Max.X);
    const double Y0=FMath::Max(Source.Min.Y,Hole.Min.Y),Y1=FMath::Min(Source.Max.Y,Hole.Max.Y);
    auto Add=[&](double LoX,double HiX,double LoY,double HiY)
    {
        if(HiX-LoX>=1. && HiY-LoY>=1.) Out.Add(FBox(FVector(LoX,LoY,Source.Min.Z),FVector(HiX,HiY,Source.Max.Z)));
    };
    // Four disjoint rectangles, rather than overlapping strips at the corners.
    Add(Source.Min.X,X0,Source.Min.Y,Source.Max.Y);
    Add(X1,Source.Max.X,Source.Min.Y,Source.Max.Y);
    Add(X0,X1,Source.Min.Y,Y0);
    Add(X0,X1,Y1,Source.Max.Y);
}
inline bool Apply(UWorld* World,const TArray<FBox>& WalkableFloors,
    TFunctionRef<ALHVisualPiece*(UWorld*,FName,ELHVisualStyle,FVector,FRotator,const FVector&)> MakePiece,
    TFunctionRef<bool(const FBox&,const TArray<FBox>&,bool)> CanRaise)
{
    if(!World || WalkableFloors.IsEmpty()) return false;
    const FBox Regions[]={
        FBox(FVector(-1800,-1600,-44),FVector(2400,3000,-24)),
        FBox(FVector(-5200,5600,-44),FVector(-1200,8800,-24))};
    TArray<ALHVisualPiece*> Added;
    int32 Triangles=0,Aprons=0,Walls=0;
    auto Admit=[&](ALHVisualPiece* Piece,bool Apron)
    {
        if(!Piece) return false;
        TArray<ALHVisualPiece*> Single{Piece};
        const auto Budget=LHVisual::ValidatePlacedSet(Single);
        if(!Budget.Errors.IsEmpty() || Triangles+Budget.Triangles>400000)
        {
            for(const auto& Error:Budget.Errors) UE_LOG(LogTemp,Error,TEXT("Church surroundings: %s"),*Error);
            UE_LOG(LogTemp,Error,TEXT("Church surroundings cannot fit imported budget: added=%d candidate=%d cap=400000"),Triangles,Budget.Triangles);
            Piece->Destroy(); return false;
        }
        Triangles+=Budget.Triangles; Added.Add(Piece);
        Piece->Tags.Append({TEXT("LH.Church.Surroundings"),Apron?FName(TEXT("LH.Church.Apron")):FName(TEXT("LH.Church.Outline"))});
        Piece->SetActorLabel(FString::Printf(TEXT("Dressing.Church.Surroundings.%s.%03d"),Apron?TEXT("Apron"):TEXT("Outline"),Apron?Aprons++:Walls++));
        return true;
    };
    auto Rollback=[&]() {for(auto* Piece:Added) Piece->Destroy();};
    for(const FBox& Region:Regions)
    {
        TArray<FBox> Remaining{Region};
        for(const FBox& Floor:WalkableFloors)
        {
            TArray<FBox> Next;
            FBox Hole=Floor.ExpandBy(FVector(8,8,0));
            // The apron lies above the descending ramp. Extend its cutout
            // southwest far enough for every camera-to-foot/head ray, so the
            // apron lip cannot hide the low end of the ramp from the camera.
            const FVector Boom=-FRotator(-55,45,0).Vector()*1200;
            const double Reach=FMath::Max(0.,Region.Max.Z-Floor.Min.Z)*(-Boom.X/(Boom.Z-90.));
            Hole.Min.X-=Reach; Hole.Min.Y-=Reach;
            for(const FBox& Rect:Remaining) Subtract(Rect,Hole,Next);
            Remaining=MoveTemp(Next);
        }
        for(const FBox& Rect:Remaining)
        for(double X=Rect.Min.X;X<Rect.Max.X-.01;X+=1200.)
        for(double Y=Rect.Min.Y;Y<Rect.Max.Y-.01;Y+=1200.)
        {
            const double Width=FMath::Min(1200.,Rect.Max.X-X),Depth=FMath::Min(1200.,Rect.Max.Y-Y);
            // Piece's centered floor recipe has top at actor Z+10.
            if(!Admit(MakePiece(World,TEXT("Presentation.Environment.Shared.Floor400"),ELHVisualStyle::Church,
                FVector(X+Width/2,Y+Depth/2,-34),FRotator::ZeroRotator,FVector(Width,Depth,20)),true))
            {Rollback(); return false;}
        }
        // Existing stone wall asset, cropped to 80cm, beyond the playable
        // footprint. The conservative camera shadow veto applies even here.
        for(int32 Side=0;Side<4;++Side)
        {
            const bool AlongX=Side<2;
            const double Low=AlongX?Region.Min.X:Region.Min.Y,High=AlongX?Region.Max.X:Region.Max.Y;
            const double Fixed=Side==0?Region.Min.Y:Side==1?Region.Max.Y:Side==2?Region.Min.X:Region.Max.X;
            for(double Station=Low;Station<High-.01;Station+=1200.)
            {
                const double Length=FMath::Min(1200.,High-Station);
                const FVector Position=AlongX?FVector(Station+Length/2,Fixed,16):FVector(Fixed,Station+Length/2,16);
                const FVector Half=AlongX?FVector(Length/2,10,40):FVector(10,Length/2,40);
                const FBox Envelope(Position-Half,Position+Half);
                if(WalkableFloors.ContainsByPredicate([&](const FBox& Floor){return OverlapXY(Envelope,Floor.ExpandBy(FVector(8,8,0)));})
                    || !CanRaise(Envelope,WalkableFloors,false)) continue;
                if(!Admit(MakePiece(World,TEXT("Presentation.Environment.Shared.Wall400"),ELHVisualStyle::Church,
                    Position,FRotator(0,AlongX?0:90,0),FVector(Length,20,80)),false))
                {Rollback(); return false;}
            }
        }
    }
    UE_LOG(LogTemp,Display,TEXT("Church surroundings: imported apron pieces=%d outline pieces=%d added triangles=%d /400000; playable footprints excluded with 8cm margin"),Aprons,Walls,Triangles);
    return Aprons>0 && Walls>0;
}
}
