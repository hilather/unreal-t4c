#include "LHGenerateDevMapsCommandlet.h"
#include "Editor.h"
#include "Framework/LHGameMode.h"
#include "Framework/LHEnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "FileHelpers.h"
#include "ActorFactories/ActorFactory.h"
#include "Builders/CubeBuilder.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Misc/Paths.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/TargetPoint.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/WorldSettings.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "Misc/PackageName.h"
#include "HAL/FileManager.h"

namespace
{
// All dimensions/tuning are W1-04 prototype choices; see dev-maps.md.
struct FRoom
{
    UWorld* World;
    UStaticMesh* Cube;
    UStaticMesh* Cylinder;
    bool bOK = true;

    template<class T> T* Actor(const TCHAR* Name, FVector Position, FRotator Rotation = FRotator::ZeroRotator)
    {
        FActorSpawnParameters Spawn;
        Spawn.Name = FName(Name);
        Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        T* Result = World->SpawnActor<T>(Position, Rotation, Spawn);
        if (!Result) { bOK = false; return nullptr; }
        Result->SetActorLabel(Name);
        Result->Tags.Add(TEXT("LH.Dev.Generated"));
        return Result;
    }

    void Shape(const TCHAR* Name, FVector Position, FVector Size, FRotator Rotation = FRotator::ZeroRotator,
               bool bCollision = true, bool bCylinder = false)
    {
        if (AStaticMeshActor* A = Actor<AStaticMeshActor>(Name, Position, Rotation))
        {
            auto* C = A->GetStaticMeshComponent();
            C->SetMobility(EComponentMobility::Static);
            C->SetStaticMesh(bCylinder ? Cylinder : Cube);
            C->SetCollisionProfileName(bCollision ? TEXT("BlockAll") : TEXT("NoCollision"));
            C->SetCanEverAffectNavigation(bCollision);
            A->SetActorScale3D(Size / 100.0);
        }
    }

    void Common()
    {
        Shape(TEXT("Floor"), FVector(0,0,-10), FVector(4000,3000,20));
        Shape(TEXT("WallWest"), FVector(-2010,0,200), FVector(20,3040,400));
        Shape(TEXT("WallEast"), FVector(2010,0,200), FVector(20,3040,400));
        Shape(TEXT("WallSouth"), FVector(0,-1510,200), FVector(4000,20,400));
        Shape(TEXT("WallNorth"), FVector(0,1510,200), FVector(4000,20,400));
        Actor<APlayerStart>(TEXT("PlayerStart"), FVector(-1600,0,100));
        if (auto* Sun = Actor<ADirectionalLight>(TEXT("Sun"), FVector(0,0,1000), FRotator(-55,-35,0)))
            CastChecked<UDirectionalLightComponent>(Sun->GetLightComponent())->SetAtmosphereSunLight(true);
        Actor<ASkyAtmosphere>(TEXT("SkyAtmosphere"), FVector::ZeroVector);
        Actor<ASkyLight>(TEXT("SkyLight"), FVector(0,0,800));
        if (auto* Nav = Actor<ANavMeshBoundsVolume>(TEXT("NavigationBounds"), FVector(0,0,500)))
        {
            auto* Builder = NewObject<UCubeBuilder>();
            Builder->X = 4000; Builder->Y = 3000; Builder->Z = 1200;
            UActorFactory::CreateBrushForVolumeActor(Nav, Builder);
        }
        World->GetWorldSettings()->DefaultGameMode = ALHGameMode::StaticClass();
    }

    void Movement()
    {
        // Door opening: Y +/-120; lintel underside Z=300.
        Shape(TEXT("DoorLeft"), FVector(-900,-810,200), FVector(20,1380,400));
        Shape(TEXT("DoorRight"), FVector(-900,810,200), FVector(20,1380,400));
        Shape(TEXT("DoorLintel"), FVector(-900,0,350), FVector(20,240,100));
        Shape(TEXT("CorridorSouth"), FVector(-400,-170,200), FVector(980,20,400));
        Shape(TEXT("CorridorNorth"), FVector(-400,170,200), FVector(980,20,400));
        // Slab centers compensate rotated top face: lower top edge sits at Z=0.
        const auto Ramp = [this](const TCHAR* Name, FVector Start, double Length, double Angle)
        {
            const double Radians = FMath::DegreesToRadians(Angle);
            const double Run = Length * FMath::Cos(Radians);
            const double Rise = Length * FMath::Sin(Radians);
            Shape(Name, Start + FVector(Run/2 + 10*FMath::Sin(Radians),0,Rise/2 - 10*FMath::Cos(Radians)),
                  FVector(Length,300,20), FRotator(Angle,0,0));
        };
        Ramp(TEXT("RampWalkable30"), FVector(200,-1050,0), 800, 30);
        Ramp(TEXT("RampSteep60"), FVector(200,-550,0), 600, 60);
        // Explicit geometric names describe step elevation rather than array-derived identity.
        Shape(TEXT("StepZ20"), FVector(300,600,10), FVector(100,300,20));
        Shape(TEXT("StepZ40"), FVector(400,600,20), FVector(100,300,40));
        Shape(TEXT("StepZ60"), FVector(500,600,30), FVector(100,300,60));
        Shape(TEXT("StepZ80"), FVector(600,600,40), FVector(100,300,80));
        Shape(TEXT("StepZ100"), FVector(700,600,50), FVector(100,300,100));
        Shape(TEXT("StairLanding"), FVector(910,600,50), FVector(320,320,100));
        Shape(TEXT("LowCeiling"), FVector(1300,0,230), FVector(600,600,20));
        Shape(TEXT("CeilingSupportSouth"), FVector(1300,-310,120), FVector(600,20,240));
        Shape(TEXT("CeilingSupportNorth"), FVector(1300,310,120), FVector(600,20,240));
        // Separate full-spread boss clearance ruler, 500 cm door x 360 cm high.
        Shape(TEXT("BossDoorSouth"), FVector(0,1200,200), FVector(20,100,400));
        Shape(TEXT("BossDoorNorth"), FVector(0,600,200), FVector(20,100,400));
        Shape(TEXT("BossDoorLintel"), FVector(0,900,380), FVector(20,500,40));
    }

    void Target(const TCHAR* Name, FVector Position, const TCHAR* DistanceTag, const TCHAR* SlotTag)
    {
        if (auto* Marker = Actor<ATargetPoint>(Name, Position))
        {
            Marker->Tags.Append({FName(TEXT("LH.Dev.DummySpawn")), FName(DistanceTag), FName(SlotTag)});
        }
        const FString DummyName = FString(Name) + TEXT("Dummy");
        const float HalfHeight = GetDefault<ALHEnemyCharacter>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
        if (auto* Dummy = Actor<ALHEnemyCharacter>(*DummyName, Position + FVector(0,0,HalfHeight)))
            Dummy->Tags.Add(TEXT("LH.Dev.CombatFixture"));
        // Visible capsule-sized ruler; no extra blocking/damage actor.
        Shape(*(DummyName + TEXT("Visual")), Position + FVector(0,0,HalfHeight),
            FVector(68,68,HalfHeight*2), FRotator::ZeroRotator, false, true);
        const FString VisualName = FString(Name) + TEXT("Ruler");
        Shape(*VisualName, FVector(Position.X,Position.Y,2), FVector(70,70,4), FRotator::ZeroRotator, false, true);
    }
    void Combat()
    {
        Shape(TEXT("LOSPillar"), FVector(-500,500,200), FVector(180,180,400), FRotator::ZeroRotator, true, true);
        // Distances from player floor origin (-1600,0,0), NOT authoritative attack ranges.
        Target(TEXT("TargetMelee"), FVector(-1450,0,0), TEXT("LH.Dev.DistanceCm.150"), TEXT("LH.Dev.Slot.Melee"));
        Target(TEXT("TargetMid"), FVector(-1000,0,0), TEXT("LH.Dev.DistanceCm.600"), TEXT("LH.Dev.Slot.Mid"));
        Target(TEXT("TargetLong"), FVector(0,0,0), TEXT("LH.Dev.DistanceCm.1600"), TEXT("LH.Dev.Slot.Long"));
        Target(TEXT("TargetLOS"), FVector(600,1000,0), TEXT("LH.Dev.LOS"), TEXT("LH.Dev.Slot.LOS"));
    }
};
}

ULHGenerateDevMapsCommandlet::ULHGenerateDevMapsCommandlet()
{
    IsClient = false;
    IsServer = false;
    IsEditor = true;
    LogToConsole = true;
}

int32 ULHGenerateDevMapsCommandlet::Main(const FString& Params)
{
    // Engine command-line flags may be present in Params; no output path is accepted.
    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (!GEditor || !Cube || !Cylinder) return 1;
    for (const bool bMovement : {true, false})
    {
        UWorld* World = GEditor->NewMap(false);
        if (!World) return 1;
        FRoom Room{World, Cube, Cylinder};
        Room.Common();
        if (bMovement) Room.Movement(); else Room.Combat();
        const FString Package = bMovement ? TEXT("/Game/Lighthaven/Maps/Dev_Movement") : TEXT("/Game/Lighthaven/Maps/Dev_Combat");
        const FString Filename = FPackageName::LongPackageNameToFilename(Package, FPackageName::GetMapPackageExtension());
        if (!Room.bOK || !IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename), true)
            || !FEditorFileUtils::SaveMap(World, Filename))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed generating %s"), *Package);
            return 1;
        }
        UE_LOG(LogTemp, Display, TEXT("Generated %s"), *Package);
    }
    return 0;
}
