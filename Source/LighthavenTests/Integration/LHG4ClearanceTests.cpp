#include "Misc/AutomationTest.h"
#include "World/LHAreaRegistry.h"
#include "Data/Enemies/LHEnemyCatalog.h"
#include "Framework/LHCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Engine/TargetPoint.h"
#include "Data/Encounters/LHEncounterCatalog.h"
#include "StaticMeshCompiler.h"
#include "GameFramework/CharacterMovementComponent.h"
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHG4DoorClearance,"Lighthaven.Integration.G4.DoorClearance",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHG4DoorClearance::RunTest(const FString&)
{
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
        // Match the director's home and leash, including its arena-centered boss.
        auto Envelope=[&](const FVector& From,const FVector& To,double& R,double& H,FString& Routes)
        {
            R=0; H=0; Routes.Empty();
            for(const auto& Slot:Area.Spawns)
            {
                const auto* Encounter=LHEncounterData::ForSpawn(Slot.SpawnId);
                const auto* Row=Encounter?LHEnemyData::Find(Encounter->Enemy):nullptr;
                if(!TestNotNull(TEXT("registry encounter resolves"),Row)) continue;
                FVector Home=Slot.Anchor.GetLocation();
                if(Row->Runtime.bBoss)
                {
                    ATargetPoint* Arena=nullptr;
                    for(TActorIterator<ATargetPoint> It(World);It;++It)
                        if(It->ActorHasTag(TEXT("B4.BalorkArena")) || It->GetFName()==FName(TEXT("B4.BalorkArena"))) Arena=*It;
                    if(!TestNotNull(TEXT("boss leash arena"),Arena)) continue;
                    Home=Arena->GetActorLocation();
                }
                FVector A=From,B=To; A.Z=0; B.Z=0; Home.Z=0;
                // Any path point outside this disk is refused in production. Disk
                // intersection is conservative: walls/NoCombat may reduce reach.
                if(FMath::PointDistToSegment(Home,A,B)>Row->Runtime.LeashRadiusCm.Value) continue;
                const double ER=Row->Runtime.CapsuleRadiusCm.Value, EH=Row->Runtime.CapsuleHalfHeightCm.Value;
                R=FMath::Max(R,ER); H=FMath::Max(H,EH);
                Routes+=FString::Printf(TEXT(" %s home=%s leash=%g capsule=%g/%g"),*Slot.Alias.ToString(),*Home.ToString(),Row->Runtime.LeashRadiusCm.Value,ER,EH);
            }
        };
        int32 Doors=0,Special=0;
        for(TActorIterator<AActor> It(World);It;++It)
        {
            const FString Label=It->GetActorLabel(); if(!Label.Contains(TEXT("Lintel"))) continue;
            ++Doors;
            if(Index==4 && (Label.Contains(TEXT("C04")) || Label.Contains(TEXT("Partition")))) ++Special;
            const FBox Bounds=It->GetComponentsBoundingBox(true);
            const FVector Center=Bounds.GetCenter(),Extent=Bounds.GetExtent();
            const FVector Axis=Extent.X<Extent.Y?FVector(1,0,0):FVector(0,1,0);
            double Radius,Height; FString Routes;
            Envelope(Center-Axis*200,Center+Axis*200,Radius,Height,Routes);
            AddInfo(FString::Printf(TEXT("REGION %s underside=%g routes:%s"),*Label,Bounds.Min.Z,*Routes));
            for(int32 Shape=0;Shape<2;++Shape)
            {
                if(Shape && Radius==0) continue;
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
                bool First=true; FVector Previous;
                for(FVector Point:Points)
                {
                    Point*=100;
                    double Radius,Height; FString Routes;
                    Envelope(First?Point:Previous,Point,Radius,Height,Routes);
                    const double R=Shape?Radius:Player->GetUnscaledCapsuleRadius();
                    const double H=Shape?Height:Player->GetUnscaledCapsuleHalfHeight();
                    Point.Z=H+2; Previous.Z=H+2;
                    if(Shape && Radius==0) { Previous=Point; First=false; continue; }
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
        // Drive the actual player CharacterMovement component on the generated
        // smooth stair collision. The terminal portal wall is deliberately not
        // crossed: stop 100cm before it, as in the generator's clearance controls.
        auto Walk=[&](FVector From,FVector To,const TCHAR* Name)
        {
            FActorSpawnParameters Spawn; Spawn.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            auto* Pawn=World->SpawnActor<ALHCharacter>(From+FVector(0,0,Player->GetUnscaledCapsuleHalfHeight()+2),FRotator::ZeroRotator,Spawn);
            if(!TestNotNull(Name,Pawn)) return;
            auto* Movement=Pawn->GetCharacterMovement();
            Movement->SetUpdatedComponent(Pawn->GetCapsuleComponent());
            if(!Movement->HasBeenInitialized()) Movement->InitializeComponent();
            Movement->bRunPhysicsWithNoController=true;
            Movement->SetMovementMode(MOVE_Walking);
            for(int32 Step=0;Step<600 && FVector::Dist2D(Pawn->GetActorLocation(),To)>15;++Step)
            {
                FVector Direction=To-Pawn->GetActorLocation(); Direction.Z=0;
                Movement->AddInputVector(Direction.GetSafeNormal(),true);
                Movement->TickComponent(1.f/60,LEVELTICK_All,nullptr);
            }
            const FVector Feet=Pawn->GetActorLocation()-FVector(0,0,Player->GetUnscaledCapsuleHalfHeight());
            AddInfo(FString::Printf(TEXT("WALK B%d/%s from=%s target=%s feet=%s mode=%d"),Index,Name,*From.ToString(),*To.ToString(),*Feet.ToString(),int32(Movement->MovementMode)));
            TestTrue(Name,FVector::Dist2D(Feet,To)<35 && FMath::Abs(Feet.Z-To.Z)<15 && Movement->IsMovingOnGround());
            Pawn->Destroy();
        };
        if(Index==1) { Walk({450,-1950,0},{450,-2500,100},TEXT("B1.S01 ascend")); Walk({4500,1750,0},{4500,2100,-75},TEXT("B1.S02 descend")); }
        if(Index==2) { Walk({500,-5750,0},{500,-6300,100},TEXT("B2.S01 ascend")); Walk({5300,6050,0},{5300,6600,-100},TEXT("B2.S02 descend")); }
        if(Index==3) { Walk({3200,900,0},{3200,1150,64},TEXT("B3 return ascend")); Walk({4700,6550,0},{4700,6850,-66.67},TEXT("B3 descent")); }
        if(Index==4) Walk({300,600,0},{0,600,66.67},TEXT("B4 return ascend"));
        const FVector Probes[]={FVector(900,900,0),FVector(1100,1000,0),FVector(3500,500,0),FVector(600,600,0)};
        FHitResult Floor;
        TestTrue(TEXT("Blocking floor positive control"),World->LineTraceSingleByChannel(Floor,Probes[Index-1]+FVector(0,0,10),Probes[Index-1]-FVector(0,0,15),ECC_Pawn));
        if(Index==4) TestEqual(TEXT("C04 pair and D05-D07 partitions probed"),Special,5);
        GEngine->DestroyWorldContext(World); World->DestroyWorld(false);
    }
    return !HasAnyErrors();
}
#endif
