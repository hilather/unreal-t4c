#include "Misc/AutomationTest.h"
#include "World/LHAreaRegistry.h"
#include "Data/Enemies/LHEnemyCatalog.h"
#include "Framework/LHCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "StaticMeshCompiler.h"
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHG4DoorClearance,"Lighthaven.Integration.G4.DoorClearance",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHG4DoorClearance::RunTest(const FString&)
{
    double Radius=0,Height=0;
    // Component-wise maximum encloses every catalog enemy, even if maxima differ.
    for(const auto& Row:LHEnemyData::Catalog())
    { Radius=FMath::Max(Radius,Row.Runtime.CapsuleRadiusCm.Value); Height=FMath::Max(Height,Row.Runtime.CapsuleHalfHeightCm.Value); }
    if(!TestTrue(TEXT("Resolved enemy capsule envelope"),Radius>0 && Height>=Radius)) return false;
    const auto* Player=GetDefault<ALHCharacter>()->GetCapsuleComponent();
    for(int32 Index=1;Index<5;++Index)
    {
        const auto& Area=LHWorld::Registry()[Index]; const FString Map=Area.Map.GetLongPackageName();
        UWorld::WorldTypePreLoadMap.Add(FName(*Map),EWorldType::Editor);
        auto* Package=LoadPackage(nullptr,*Map,LOAD_None);
        UWorld::WorldTypePreLoadMap.Remove(FName(*Map));
        auto* World=Package?UWorld::FindWorldInPackage(Package):nullptr;
        if(!TestNotNull(*Map,World)) continue;
        auto& Context=GEngine->CreateNewWorldContext(EWorldType::Editor); Context.SetCurrentWorld(World);
        if(!World->IsInitialized()) World->InitWorld(UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(true).CreateNavigation(false));
        if(!World->GetPhysicsScene()) World->CreatePhysicsScene();
        World->UpdateWorldComponents(true,false); FStaticMeshCompilingManager::Get().FinishAllCompilation();
        for(TActorIterator<AActor> It(World);It;++It)
        {
            TInlineComponentArray<UPrimitiveComponent*> Components(*It);
            for(auto* C:Components) { C->UpdateComponentToWorld(); C->RecreatePhysicsState(); }
        }
        int32 Doors=0,Special=0;
        for(TActorIterator<AActor> It(World);It;++It)
        {
            const FString Label=It->GetActorLabel(); if(!Label.Contains(TEXT("Lintel"))) continue;
            ++Doors;
            if(Index==4 && (Label.Contains(TEXT("C04")) || Label.Contains(TEXT("Partition")))) ++Special;
            const FBox Bounds=It->GetComponentsBoundingBox(true);
            const FVector Center=Bounds.GetCenter(),Extent=Bounds.GetExtent();
            const FVector Axis=Extent.X<Extent.Y?FVector(1,0,0):FVector(0,1,0);
            for(int32 Shape=0;Shape<2;++Shape)
            {
                const double R=Shape?Radius:Player->GetUnscaledCapsuleRadius();
                const double H=Shape?Height:Player->GetUnscaledCapsuleHalfHeight();
                const FVector At(Center.X,Center.Y,H+2);
                FHitResult Hit; const FVector Offset=Axis*(R+FMath::Min(Extent.X,Extent.Y)+25);
                const bool Blocked=World->SweepSingleByChannel(Hit,At-Offset,At+Offset,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(R,H));
                const FString Key=FString::Printf(TEXT("%s/%s/%s"),*Map,*Label,Shape?TEXT("enemy-envelope"):TEXT("player"));
                AddInfo(FString::Printf(TEXT("CLEARANCE %s radius=%g halfheight=%g blocker=%s"),*Key,R,H,*GetNameSafe(Hit.GetActor())));
                TestFalse(*Key,Blocked);
            }
        }
        if(Index>=3) TestTrue(TEXT("Authored lintel probes present (coverage control)"),Doors>0);
        // B1/B2 are generated from polygon strips, without separate lintel actors.
        // Flat route control points mirror the authored generator paths (cm).
        auto Route=[&](std::initializer_list<FVector> Points)
        {
            for(int32 Shape=0;Shape<2;++Shape)
            {
                const double R=Shape?Radius:Player->GetUnscaledCapsuleRadius();
                const double H=Shape?Height:Player->GetUnscaledCapsuleHalfHeight();
                bool First=true; FVector Previous;
                for(FVector Point:Points)
                {
                    Point*=100; Point.Z=H+2;
                    if(!First) { FHitResult Hit;
                        const bool Blocked=World->SweepSingleByChannel(Hit,Previous,Point,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(R,H));
                        const FString Key=FString::Printf(TEXT("B%d %s %s -> %s blocker=%s"),Index,Shape?TEXT("enemy"):TEXT("player"),*Previous.ToString(),*Point.ToString(),*GetNameSafe(Hit.GetActor()));
                        AddInfo(Key); TestFalse(*Key,Blocked);
                    }
                    Previous=Point; First=false;
                }
            }
        };
        if(Index==1)
        {
            Route({{4.5,-16.5,0},{9,-16.5,0},{9,9,0},{-21,9,0}});
            Route({{9,9,0},{35,9,0},{35,5,0},{45,5,0},{45,14,0}});
            Route({{35,5,0},{52,5,0},{52,24,0},{35,24,0},{35,5,0}});
        }
        if(Index==2)
        {
            Route({{5,-54.5,0},{10,-54.5,0},{10,-30,0},{3.5,-30,0},{3.5,-16,0},{11,-16,0},{11,10,0}});
            Route({{10,-30,0},{18,-30,0},{18,-16,0},{11,-16,0}});
            Route({{11,10,0},{15,10,0},{15,38,0},{-1,38,0},{-1,56,0},{53,56,0},{53,57.5,0}});
            Route({{11,10,0},{-19,10,0},{-19,-23,0}});
            Route({{-19,10,0},{-14,10,0},{-14,42,0},{-30,42,0}});
        }
        const FVector Probes[]={FVector(900,900,0),FVector(1100,1000,0),FVector(3500,500,0),FVector(600,600,0)};
        FHitResult Floor;
        TestTrue(TEXT("Blocking floor positive control"),World->LineTraceSingleByChannel(Floor,Probes[Index-1]+FVector(0,0,10),Probes[Index-1]-FVector(0,0,15),ECC_Pawn));
        if(Index==4) TestEqual(TEXT("C04 pair and D05-D07 partitions probed"),Special,5);
        GEngine->DestroyWorldContext(World); World->DestroyWorld(false);
    }
    return !HasAnyErrors();
}
#endif
