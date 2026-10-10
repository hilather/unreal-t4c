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
    TArray<FVector> Protected;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        if (auto* E=Cast<ALHEntranceMarker>(*It)) Protected.Add(E->SafeArrivalTransform.GetLocation());
        else if (It->IsA<ALHSpawnMarker>() || It->IsA<ALHPortal>() || It->IsA<ALHInteractableMarker>()) Protected.Add(It->GetActorLocation());
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
        if (Floor && Size.X >= 600 && Size.Y >= 400 && Props < 48 && A->GetActorRotation().IsNearlyZero())
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
    for (auto* Light : Lights)
    {
        if (Light->GetActorLocation().Z > 350) continue; // ceiling/fill lights have no fixture.
        auto* Fixture = Piece(World,TEXT("Presentation.Environment.Shared.Sconce"),Style,
            Light->GetActorLocation(),FRotator::ZeroRotator);
        if (!Fixture) return false;
        Pieces.Add(Fixture);
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
