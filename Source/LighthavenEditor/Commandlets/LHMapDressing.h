#pragma once
#include "CoreMinimal.h"
#include "Visual/LHVisualKit.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/PointLight.h"
#include "Engine/StaticMesh.h"
#include "World/LHWorldMarkers.h"
#include "EngineUtils.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"

// Presentation only: retain the original smooth colliders and their nav contribution.
// Recipe customization keeps unit actor scale and kit per-assembly budgets.
namespace LHMapDressing
{
inline ALHVisualPiece* Piece(UWorld* World, FName Id, ELHVisualStyle Style,
    FVector Position, FRotator Rotation, const FVector& Dimensions = FVector::ZeroVector)
{
    FLHVisualRecipe Recipe;
    if (!LHVisual::MakeRecipe(Id, Style, Recipe)) return nullptr;
    Recipe.Collision.Reset();
    if (!Dimensions.IsZero())
    {
        const bool Floor = Id.ToString().Contains(TEXT("Floor"));
        const FVector Nominal = Floor ? FVector(400,400,20) : FVector(400,20,400);
        const FVector Pivot = Floor ? FVector(200,200,-10) : FVector(200,-10,200);
        const FVector Scale = Dimensions / Nominal;
        for (auto& Box : Recipe.Geometry)
        {
            Box.Center = (Box.Center - Pivot) * Scale;
            Box.Size *= Scale;
        }
    }
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    auto* A = World->SpawnActor<ALHVisualPiece>(Position, Rotation, Params);
    if (!A || !A->Build(Recipe)) { if(A) A->Destroy(); return nullptr; }
    A->Tags.Append({TEXT("LH.Dressing"), TEXT("LH.Provenance.Prototype")});
    return A;
}
inline bool Dress(UWorld* World, ELHVisualStyle Style, const TArray<FVector>& Vantages,
    const TArray<FVector>& Targets)
{
    TArray<AStaticMeshActor*> Surfaces;
    TArray<APointLight*> Lights;
    for (TActorIterator<AStaticMeshActor> It(World); It; ++It) Surfaces.Add(*It);
    for (TActorIterator<APointLight> It(World); It; ++It) Lights.Add(*It);
    TArray<ALHVisualPiece*> Pieces;
    TArray<FBox> B1VisibleWalls, B1FlatFloors, B1Stairs;
    TArray<FVector> Protected;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        if (auto* E=Cast<ALHEntranceMarker>(*It)) Protected.Add(E->SafeArrivalTransform.GetLocation());
        else if (It->IsA<ALHSpawnMarker>() || It->IsA<ALHPortal>() || It->IsA<ALHInteractableMarker>()
            || (Style==ELHVisualStyle::B1Cellar && It->Tags.Contains(TEXT("LH.Landmark.Healer")))) Protected.Add(It->GetActorLocation());
    }
    int32 Props = 0;
    for (auto* A : Surfaces)
    {
        auto* C = A->GetStaticMeshComponent();
        if (A->IsHidden() || !C->IsVisible() || !C->GetStaticMesh()
            || C->GetStaticMesh()->GetName() != TEXT("Cube")) continue;
        const FString Label = A->GetActorLabel();
        if (A->GetAttachParentActor() || Label.StartsWith(TEXT("NPC.")) || Label.StartsWith(TEXT("Scenic."))
            || Label.Contains(TEXT("Arrow")) || Label.Contains(TEXT("Tread"))
            || Label.Contains(TEXT("RedAisle")) || Label.Contains(TEXT("Feature"))
            || Label.Contains(TEXT("Stain")) || Label.Contains(TEXT("Pale"))) continue;
        if (Label.StartsWith(TEXT("Temple.Pew.")) || Label == TEXT("Temple.AltarTable"))
        {
            auto* Prop = Piece(World,Label==TEXT("Temple.AltarTable")?TEXT("Presentation.Environment.Shared.Altar"):TEXT("Presentation.Environment.Shared.Bench"),
                Style,FVector(A->GetActorLocation().X,A->GetActorLocation().Y,0),A->GetActorRotation());
            if (!Prop) return false;
            Pieces.Add(Prop); C->SetVisibility(false); C->SetCastShadow(false);
            continue;
        }
        const FVector Size = A->GetActorScale3D().GetAbs() * 100;
        const bool Floor = Size.Z <= 20 && Size.X >= 5 && Size.Y >= 5;
        const bool Wall = Size.Z >= 80 && FMath::Min(Size.X,Size.Y) <= 40 && FMath::Max(Size.X,Size.Y) >= 5;
        if (Label.Contains(TEXT("Lintel")) || (FMath::IsNearlyEqual(A->GetActorLocation().Z,350.,.1)
            && FMath::IsNearlyEqual(Size.Z,100.,.1) && FMath::IsNearlyEqual(FMath::Min(Size.X,Size.Y),20.,.1)
            && FMath::Max(Size.X,Size.Y)>=240 && FMath::Max(Size.X,Size.Y)<=320))
        {
            const double Width = FMath::Max(Size.X,Size.Y);
            const FName Id = Width >= 500 ? TEXT("Presentation.Environment.Basement.ArchBoss500") : TEXT("Presentation.Environment.Shared.Arch240");
            auto* Arch = Piece(World,Id,Style,FVector(A->GetActorLocation().X,A->GetActorLocation().Y,0),FRotator(0,Size.Y>Size.X?90:0,0));
            if (!Arch) return false;
            Pieces.Add(Arch);
            C->SetVisibility(false); C->SetCastShadow(false);
        }
        if (!Floor && !Wall) continue; // NPC assemblies, signs and route proxies retain their owner.
        if(Style==ELHVisualStyle::B1Cellar && Wall && A->GetActorRotation().IsNearlyZero()
            && A->GetActorLocation().Z-Size.Z/2<=.1 && A->GetActorLocation().Z+Size.Z/2<=120.1)
            B1VisibleWalls.Add(FBox(A->GetActorLocation()-Size/2,A->GetActorLocation()+Size/2));
        if(Style==ELHVisualStyle::B1Cellar && Floor)
        {
            if(A->GetActorRotation().IsNearlyZero())
                B1FlatFloors.Add(FBox(A->GetActorLocation()-Size/2,A->GetActorLocation()+Size/2));
            else B1Stairs.Add(C->Bounds.GetBox().ExpandBy(FVector(200,200,0)));
        }
        if (Floor && !A->GetActorRotation().IsNearlyZero() && FMath::IsNearlyEqual(FMath::Min(Size.X,Size.Y),300.,1.))
        {
            const FVector Axis = Size.X > Size.Y ? FVector::ForwardVector : FVector::RightVector;
            const FVector Delta = A->GetActorQuat().RotateVector(Axis) * FMath::Max(Size.X,Size.Y);
            const FVector Normal = A->GetActorQuat().RotateVector(FVector::UpVector);
            FLHVisualRecipe Stair;
            if (!LHVisual::MakeRecipe(TEXT("Presentation.Environment.Shared.Stair600x120"),Style,Stair,Delta.Z<0)) return false;
            Stair.Collision.Reset();
            const double Rise = FMath::Abs(Delta.Z);
            const FVector Scale(Delta.Size2D()/600.,1,Rise/120.);
            const FVector Pivot(300,0,Delta.Z<0?-60:60);
            for (auto& Box : Stair.Geometry) { Box.Center=(Box.Center-Pivot)*Scale; Box.Size*=Scale; }
            // Refit the rotated slab explicitly; anisotropic box scaling alone
            // cannot change its slope. Tread centers follow the signed rise above.
            const FRotator Slope(FMath::RadiansToDegrees(FMath::Atan2(Delta.Z,Delta.Size2D())),0,0);
            Stair.Geometry[0].Center=-Slope.RotateVector(FVector::UpVector)*10;
            Stair.Geometry[0].Size=FVector(Delta.Size(),300,20);
            Stair.Geometry[0].Rotation=Slope;
            auto* Step = World->SpawnActor<ALHVisualPiece>(A->GetActorLocation()+Normal*10,
                FRotator(0,Delta.Rotation().Yaw,0));
            if (!Step || !Step->Build(Stair)) return false;
            Step->Tags.Add(TEXT("LH.Dressing")); Pieces.Add(Step);
            C->SetVisibility(false); C->SetCastShadow(false);
            A->Tags.Add(TEXT("LH.Dressing.RetainedCollider"));
            continue;
        }
        FRotator Rotation = A->GetActorRotation();
        FVector Dimensions = Size;
        if (Wall && Size.Y > Size.X) { Swap(Dimensions.X,Dimensions.Y); Rotation.Yaw += 90; }
        auto* Replacement = Piece(World, Floor ? TEXT("Presentation.Environment.Shared.Floor400")
            : TEXT("Presentation.Environment.Shared.Wall400"), Style, A->GetActorLocation(), Rotation, Dimensions);
        if (!Replacement) return false;
        Replacement->SetActorLabel(TEXT("Dressing.") + A->GetActorLabel());
        Pieces.Add(Replacement);
        C->SetVisibility(false); C->SetCastShadow(false);
        A->Tags.Add(TEXT("LH.Dressing.RetainedCollider"));
        // Corners of broad room floor surfaces, never new obstacles or interactables.
        if (Style!=ELHVisualStyle::B1Cellar && Floor && Size.X >= 600 && Size.Y >= 400 && Props < 48 && A->GetActorRotation().IsNearlyZero())
        {
            const TCHAR* Catalog[] = {TEXT("Barrel"), TEXT("Crate"), TEXT("Table"), TEXT("Bench"), TEXT("Debris")};
            FVector Position = A->GetActorLocation() + FVector(-Size.X/2+130,-Size.Y/2+130,Size.Z/2);
            if (Protected.ContainsByPredicate([&](const FVector& P){return FVector::DistSquared2D(P,Position)<FMath::Square(250.);})) continue;
            const FString Id = TEXT("Presentation.Environment.Shared.") + FString(Catalog[Props%5]);
            auto* Prop = Piece(World,FName(*Id),Style,Position,FRotator::ZeroRotator);
            if (!Prop) return false;
            Pieces.Add(Prop); ++Props;
        }
    }
    TMap<ALHVisualPiece*,FBox> B1ClutterFootprints;
    if(Style==ELHVisualStyle::B1Cellar)
    {
        // Prototype perimeter clusters. Seed only the interior of continuous wall
        // segments: the 320cm end margin protects every open mouth/door approach.
        // Full padded footprints must be supported by the union of flat slabs;
        // slopes, markers and earlier props veto a seed. No floor-center seeding.
        TArray<FBox> Occupied;
        for(const FBox& Wall:B1VisibleWalls)
        {
            const bool AlongX=Wall.GetSize().X>=Wall.GetSize().Y;
            const double Low=AlongX?Wall.Min.X:Wall.Min.Y;
            const double High=AlongX?Wall.Max.X:Wall.Max.Y;
            const FVector Tangent=AlongX?FVector(1,0,0):FVector(0,1,0);
            for(double Station=Low+320;Station<=High-320;Station+=500)
            for(double Sign:{-1.,1.})
            {
                const FVector Inward=AlongX?FVector(0,Sign,0):FVector(Sign,0,0);
                FVector Seed=Wall.GetCenter();
                if(AlongX) { Seed.X=Station; Seed.Y+=Sign*(Wall.GetSize().Y/2+75); }
                else { Seed.Y=Station; Seed.X+=Sign*(Wall.GetSize().X/2+75); }
                const TCHAR* Catalog[]={TEXT("Barrel"),TEXT("Crate"),TEXT("Debris"),TEXT("Table"),TEXT("Bench")};
                for(int32 Member=0;Member<3 && Props<90;++Member)
                {
                    const TCHAR* Kind=Catalog[(Props+Member)%5];
                    const bool Long=FCString::Strcmp(Kind,TEXT("Table"))==0 || FCString::Strcmp(Kind,TEXT("Bench"))==0;
                    const FVector Half=AlongX?FVector(Long?88:55,55,0):FVector(55,Long?88:55,0);
                    // Broken wall-side silhouettes rather than a ruler-straight row.
                    const double Inset[]={65.,100.,85.};
                    const double Stagger[]={-12.,18.,-6.};
                    FVector Position=Seed+Tangent*((Member-1)*150+Stagger[Member])+Inward*(Inset[Member]-75.);
                    Position.Z=0;
                    FBox Foot(Position-Half,Position+Half);
                    auto Intersects2D=[](const FBox& A,const FBox& B)
                    { return A.Min.X<B.Max.X && A.Max.X>B.Min.X && A.Min.Y<B.Max.Y && A.Max.Y>B.Min.Y; };
                    if(B1Stairs.ContainsByPredicate([&](const FBox& B){return Intersects2D(Foot,B);})
                        || B1VisibleWalls.ContainsByPredicate([&](const FBox& B){return Intersects2D(Foot,B);})
                        || Occupied.ContainsByPredicate([&](const FBox& B){return Intersects2D(Foot,B);})
                        || Protected.ContainsByPredicate([&](const FVector& P)
                        {
                            const double X=FMath::Clamp(P.X,Foot.Min.X,Foot.Max.X),Y=FMath::Clamp(P.Y,Foot.Min.Y,Foot.Max.Y);
                            return FVector::DistSquared2D(P,FVector(X,Y,0))<FMath::Square(350.);
                        })) continue;
                    bool Supported=true;
                    for(double X:{Foot.Min.X,Position.X,Foot.Max.X})
                    for(double Y:{Foot.Min.Y,Position.Y,Foot.Max.Y})
                        Supported &= B1FlatFloors.ContainsByPredicate([&](const FBox& F)
                        {return X>=F.Min.X && X<=F.Max.X && Y>=F.Min.Y && Y<=F.Max.Y && FMath::Abs(F.Max.Z)<=.1;});
                    // Dress broad rooms only. Sweep a 600cm inward strip from
                    // this footprint; floor support alone could pass through an
                    // interior partition into the next room, so walls also veto it.
                    const FVector DepthEnd=Position+Inward*600;
                    const FBox DepthStrip(Foot.Min.ComponentMin(DepthEnd-Half),Foot.Max.ComponentMax(DepthEnd+Half));
                    Supported &= !B1VisibleWalls.ContainsByPredicate([&](const FBox& B){return Intersects2D(DepthStrip,B);});
                    for(double Depth=100;Depth<=600;Depth+=100)
                    for(double Side:{-1.,0.,1.})
                    {
                        const FVector Sample=Position+Inward*Depth+Tangent*(Side*(AlongX?Half.X:Half.Y));
                        Supported &= B1FlatFloors.ContainsByPredicate([&](const FBox& F)
                        {return Sample.X>=F.Min.X && Sample.X<=F.Max.X && Sample.Y>=F.Min.Y && Sample.Y<=F.Max.Y && FMath::Abs(F.Max.Z)<=.1;});
                    }
                    if(!Supported) continue;
                    const FString Id=TEXT("Presentation.Environment.Shared.")+FString(Kind);
                    auto* Prop=Piece(World,FName(*Id),Style,Position,FRotator(0,AlongX?0:90,0));
                    if(!Prop) return false;
                    Prop->Tags.Add(TEXT("LH.B1.PerimeterClutter"));
                    Prop->SetActorLabel(FString::Printf(TEXT("Dressing.B1.Cluster.%02d.%s"),Props,Kind));
                    Pieces.Add(Prop); Occupied.Add(Foot); B1ClutterFootprints.Add(Prop,Foot); ++Props;
                }
            }
        }
    }
    TArray<FVector> B1FixtureFlames, B1FloorFixtureFlames;
    for (auto* Light : Lights)
    {
        if (Light->GetActorLocation().Z > 350) continue; // ceiling/fill lights have no fixture.
        FVector Position=Light->GetActorLocation(); FRotator Rotation=FRotator::ZeroRotator;
        FName FixtureId=TEXT("Presentation.Environment.Shared.Sconce");
        if(Style==ELHVisualStyle::B1Cellar)
        {
            FixtureId=TEXT("Presentation.Environment.Shared.Torch");
            // Prefer existing wall faces within a torch-pool radius. This removes
            // near-wall furniture silhouettes without adding gameplay geometry.
            double Nearest=FMath::Square(375.); bool Mounted=false; FVector B1MountPosition;
            for(const FBox& Wall:B1VisibleWalls)
            {
                const FVector Size=Wall.GetSize(), Center=Wall.GetCenter();
                const bool AlongX=Size.X>=Size.Y;
                const double HalfLength=(AlongX?Size.X:Size.Y)/2;
                if(HalfLength<45) continue; // plate clear of segment cuts and door jambs
                FVector Candidate=Center, Inward;
                if(AlongX)
                {
                    Candidate.X=FMath::Clamp(Position.X,Wall.Min.X+45,Wall.Max.X-45);
                    const double Sign=Position.Y>=Center.Y?1.:-1.;
                    Candidate.Y=Center.Y+Sign*(Size.Y/2+.5); Inward={0,Sign,0};
                }
                else
                {
                    Candidate.Y=FMath::Clamp(Position.Y,Wall.Min.Y+45,Wall.Max.Y-45);
                    const double Sign=Position.X>=Center.X?1.:-1.;
                    Candidate.X=Center.X+Sign*(Size.X/2+.5); Inward={Sign,0,0};
                }
                const double Distance=FVector::DistSquared2D(Position,Candidate);
                if(Distance>=Nearest) continue;
                Nearest=Distance;
                Candidate.Z=Wall.Max.Z-30;
                // Imported plate projects in local +Y, below the 120cm cutaway.
                Rotation=FRotator(0,Inward.Rotation().Yaw-90,0);
                // Keep search source unchanged until every candidate has been compared.
                Mounted=true;
                FixtureId=TEXT("Presentation.Environment.Shared.Sconce");
                // Candidate is retained separately below to avoid order-dependent distance.
                B1MountPosition=Candidate;
            }
            if(Mounted) Position=B1MountPosition;
            else
            {
                FHitResult Floor;
                FCollisionQueryParams Query(SCENE_QUERY_STAT(LHB1BrazierPlacement),false);
                bool FoundFloor=false;
                for(int32 Retry=0;Retry<8;++Retry)
                {
                    if(!World->LineTraceSingleByObjectType(Floor,Position,Position-FVector(0,0,2000),
                        FCollisionObjectQueryParams(ECC_WorldStatic),Query)) break;
                    if(!Floor.bStartPenetrating && Floor.ImpactNormal.Z>=.5f) { FoundFloor=true; break; }
                    if(!Floor.GetActor()) break;
                    Query.AddIgnoredActor(Floor.GetActor());
                }
                if(!FoundFloor) continue; // never float a floor fixture over a stair void
                Position.Z=Floor.ImpactPoint.Z+90;
            }
            const FVector Flame=Position+Rotation.RotateVector(FVector(0,25,14));
            if(Mounted)
            {
                // Cutaway height is fixed while a neighboring ramp may rise up
                // into a wall fixture. Drop that visual seed rather than bury it.
                FHitResult Floor;
                FCollisionQueryParams Query(SCENE_QUERY_STAT(LHB1WallFixtureGround),false);
                bool FoundFloor=false;
                for(int32 Retry=0;Retry<8;++Retry)
                {
                    if(!World->LineTraceSingleByObjectType(Floor,Flame+FVector(0,0,200),Flame-FVector(0,0,2000),
                        FCollisionObjectQueryParams(ECC_WorldStatic),Query)) break;
                    if(!Floor.bStartPenetrating && Floor.ImpactNormal.Z>=.5f) { FoundFloor=true; break; }
                    if(!Floor.GetActor()) break;
                    Query.AddIgnoredActor(Floor.GetActor());
                }
                if(!FoundFloor || Flame.Z-Floor.ImpactPoint.Z<70) continue;
            }
            if(B1FixtureFlames.ContainsByPredicate([&](const FVector& P)
                {return FVector::DistSquared2D(P,Flame)<FMath::Square(300.);})) continue;
            B1FixtureFlames.Add(Flame);
            if(!Mounted) B1FloorFixtureFlames.Add(Flame);
        }
        auto* Fixture = Piece(World,FixtureId,Style,Position,Rotation);
        if (!Fixture) return false;
        if(Style==ELHVisualStyle::B1Cellar)
            Fixture->Tags.Add(FixtureId.ToString().EndsWith(TEXT("Sconce"))?TEXT("LH.B1.WallFixture"):TEXT("LH.B1.FloorFixture"));
        Pieces.Add(Fixture);
    }
    // Preserve fixture/light transforms. Remove only conflicting presentation
    // clutter once actual wall mounting/floor placement has established flames.
    for(const auto& Pair:B1ClutterFootprints)
    {
        const FBox& Foot=Pair.Value;
        if(!B1FixtureFlames.ContainsByPredicate([&](const FVector& Flame)
        {
            const FVector Nearest(FMath::Clamp(Flame.X,Foot.Min.X,Foot.Max.X),FMath::Clamp(Flame.Y,Foot.Min.Y,Foot.Max.Y),0);
            // Low rubble/benches can dress below wall sconces. Keep the full
            // clearance for tall silhouettes and every freestanding floor bowl.
            const FString Id=Pair.Key->GetRecipe().Id.ToString();
            const bool Low=Id.EndsWith(TEXT("Debris")) || Id.EndsWith(TEXT("Bench"));
            const bool FloorFixture=B1FloorFixtureFlames.Contains(Flame);
            return FVector::DistSquared2D(Flame,Nearest)<FMath::Square(Low && !FloorFixture?40.:120.);
        })) continue;
        Pieces.Remove(Pair.Key);
        World->DestroyActor(Pair.Key);
    }
    if (Style == ELHVisualStyle::B1Cellar || Style == ELHVisualStyle::B2Damp)
    {
        // The A generator's open 320cm baffle mouths have no named lintel actors.
        const FVector Mouth=Style==ELHVisualStyle::B1Cellar?FVector(900,-1000,0):FVector(1000,-4600,0);
        auto* Arch=Piece(World,TEXT("Presentation.Environment.Shared.Arch320"),Style,Mouth,FRotator::ZeroRotator);
        if (!Arch) return false;
        Pieces.Add(Arch);
    }
    if (Style == ELHVisualStyle::B4Ritual)
    {
        auto* Altar = Piece(World,TEXT("Presentation.Environment.Shared.Altar"),Style,FVector(1600,7800,0),FRotator::ZeroRotator);
        if (!Altar) return false;
        Pieces.Add(Altar);
    }
    if (Vantages.Num() != Targets.Num() || Vantages.Num() != 3) return false;
    for (int32 I=0; I<Vantages.Num(); ++I)
    {
        FActorSpawnParameters Params; Params.Name = FName(*FString::Printf(TEXT("LH_Capture_%d"),I+1));
        auto* Camera = World->SpawnActor<ACameraActor>(Vantages[I],(Targets[I]-Vantages[I]).Rotation(),Params);
        if (!Camera) return false;
        Camera->GetCameraComponent()->SetFieldOfView(65);
        Camera->Tags.Add(TEXT("LH.Capture"));
    }
    Pieces.Reset();
    for (TActorIterator<ALHVisualPiece> It(World); It; ++It) Pieces.Add(*It);
    const auto Totals = LHVisual::ValidatePlacedSet(Pieces);
    UE_LOG(LogTemp,Display,TEXT("Dressing: %d pieces, %d triangles, %d base sections, %d blockers"),
        Totals.Pieces,Totals.Triangles,Totals.DrawCalls,Totals.CollisionBoxes);
    for (const auto& Error : Totals.Errors) UE_LOG(LogTemp,Error,TEXT("Dressing: %s"),*Error);
    return Totals.Errors.IsEmpty() && Totals.CollisionBoxes == 0 && Totals.Pieces <= 6000
        && Totals.Triangles <= 2000000 && Totals.DrawCalls <= 12000;
}
}
