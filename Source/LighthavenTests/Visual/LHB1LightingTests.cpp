#include "Misc/AutomationTest.h"
#include "Visual/LHB1Lighting.h"
#include "Visual/LHVisualKit.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/SceneComponent.h"
#include "Components/ExponentialHeightFogComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHB1TorchLightingTest,"Lighthaven.Visual.B1TorchLighting",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHB1TorchLightingTest::RunTest(const FString& Parameters)
{
    UWorld* World=UWorld::CreateWorld(EWorldType::Game,false);
    if(!TestNotNull(TEXT("Transient world"),World)) return false;
    ALHVisualPiece* Piece=World->SpawnActor<ALHVisualPiece>();
    FLHVisualRecipe Recipe;
    // Production constructor defaults own the prototype fixture tuning.
    const auto* Defaults=GetDefault<ULHB1TorchLightComponent>();
    for(const FName Id:{FName(TEXT("Presentation.Environment.Shared.Sconce")),FName(TEXT("Presentation.Environment.Shared.Torch"))})
    {
        LHVisual::MakeRecipe(Id,ELHVisualStyle::B1Cellar,Recipe);
        TestTrue(TEXT("Build fixture"),Piece->Build(Recipe));
        auto* Light=Piece->FindComponentByClass<ULHB1TorchLightComponent>();
        TestNotNull(TEXT("Build binds B1 flame light without helper call"),Light);
        Piece->Build(Recipe);
        TArray<ULHB1TorchLightComponent*> Lights; Piece->GetComponents(Lights);
        TestEqual(TEXT("Rebuild never duplicates light"),Lights.Num(),1);
        if(Light)
        {
            TestEqual(TEXT("Rebuild reuses light"),Piece->FindComponentByClass<ULHB1TorchLightComponent>(),Light);
            TestFalse(TEXT("No shadow cost"),Light->CastShadows);
            TestEqual(TEXT("Warm flame kelvin"),Light->Temperature,Defaults->Temperature);
            TestEqual(TEXT("Flame intensity"),Light->Intensity,Defaults->Intensity);
            TestEqual(TEXT("Flame intensity units"),Light->IntensityUnits,Defaults->IntensityUnits);
            TestEqual(TEXT("Flame pool radius"),Light->AttenuationRadius,Defaults->AttenuationRadius);
            TestEqual(TEXT("Temperature enabled"),Light->bUseTemperature,Defaults->bUseTemperature);
            TestEqual(TEXT("Flame location"),Light->GetRelativeLocation(),FVector(0,25,14));
        }
    }
    LHVisual::MakeRecipe(TEXT("Presentation.Environment.Shared.Barrel"),ELHVisualStyle::B1Cellar,Recipe);
    Piece->Build(Recipe);
    TestNull(TEXT("Non-flame rebuild removes torch light"),Piece->FindComponentByClass<ULHB1TorchLightComponent>());
    for(double T=0;T<60;T+=.13)
    {
        const float A=ULHB1TorchLightComponent::Flicker(T,234);
        TestEqual(TEXT("Capture time deterministic"),A,ULHB1TorchLightComponent::Flicker(T,234));
        TestTrue(TEXT("Flicker within six percent"),A>=.94f && A<=1.06f);
    }
    for(auto Style:{ELHVisualStyle::Church,ELHVisualStyle::B2Damp,ELHVisualStyle::B3Crypt,ELHVisualStyle::B4Ritual})
    {
        LHVisual::MakeRecipe(TEXT("Presentation.Environment.Shared.Sconce"),Style,Recipe);
        Piece->Build(Recipe);
        TestNull(TEXT("Other floors receive no B1 light"),Piece->FindComponentByClass<ULHB1TorchLightComponent>());
    }
    World->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHB1ReadabilityLifecycleTest,"Lighthaven.Visual.B1ReadabilityLifecycle",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHB1ReadabilityLifecycleTest::RunTest(const FString& Parameters)
{
    const auto Init=UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(true)
        .CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false);
    UWorld* World=UWorld::CreateWorld(EWorldType::Game,false,
        MakeUniqueObjectName(GetTransientPackage(),UWorld::StaticClass(),TEXT("B1ReadabilityTest")),
        GetTransientPackage(),true,ERHIFeatureLevel::Num,&Init);
    if(!TestNotNull(TEXT("Readability world"),World)) return false;
    GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
    World->InitializeActorsForPlay(FURL());
    auto* Controller=World->SpawnActor<APlayerController>();
    auto* Atmosphere=World->SpawnActor<ALHB1Atmosphere>();
    auto MakePawn=[&](FVector Location)
    {
        auto* Pawn=World->SpawnActor<APawn>();
        auto* Root=NewObject<USceneComponent>(Pawn);
        Pawn->AddInstanceComponent(Root); Pawn->SetRootComponent(Root);
        Root->SetMobility(EComponentMobility::Movable); Root->RegisterComponent();
        Pawn->SetActorLocation(Location); return Pawn;
    };
    auto* First=MakePawn(FVector(100,200,80));
    auto* Respawn=MakePawn(FVector(-400,500,90));
    const FVector FogOrigin=Atmosphere->Haze->GetComponentLocation();
    TArray<FVector> BouncePositions;
    for(const auto& Fill:Atmosphere->RoomFills) BouncePositions.Add(Fill->GetComponentLocation());
    TestEqual(TEXT("Four room bounce fills"),BouncePositions.Num(),4);
    Atmosphere->Tick(0);
    TestFalse(TEXT("No pawn hides fill"),Atmosphere->ReadabilityFill->IsVisible());
    Controller->Possess(First); Atmosphere->Tick(0);
    TestTrue(TEXT("Possession reveals fill"),Atmosphere->ReadabilityFill->IsVisible());
    TestEqual(TEXT("Fill follows first pawn"),Atmosphere->ReadabilityFill->GetComponentLocation(),First->GetActorLocation()+FVector(0,0,220));
    TestEqual(TEXT("Light never moves pawn"),First->GetActorLocation(),FVector(100,200,80));
    First->SetActorLocation(FVector(800,-100,85)); Atmosphere->Tick(0);
    TestEqual(TEXT("Fill follows movement"),Atmosphere->ReadabilityFill->GetComponentLocation(),FVector(800,-100,305));
    Controller->Possess(Respawn); Atmosphere->Tick(0);
    TestEqual(TEXT("Respawn rebinds fill"),Atmosphere->ReadabilityFill->GetComponentLocation(),FVector(-400,500,310));
    TestEqual(TEXT("Light never moves respawn pawn"),Respawn->GetActorLocation(),FVector(-400,500,90));
    Controller->UnPossess(); Atmosphere->Tick(0);
    TestFalse(TEXT("Unpossessed fill hidden"),Atmosphere->ReadabilityFill->IsVisible());
    Controller->Possess(First); Atmosphere->Tick(0);
    TestTrue(TEXT("Repossessed fill visible"),Atmosphere->ReadabilityFill->IsVisible());
    TestEqual(TEXT("Repossessed fill rebinds"),Atmosphere->ReadabilityFill->GetComponentLocation(),FVector(800,-100,305));
    TestEqual(TEXT("Fog never follows player"),Atmosphere->Haze->GetComponentLocation(),FogOrigin);
    for(int32 I=0;I<Atmosphere->RoomFills.Num();++I)
    {
        const auto* Fill=Atmosphere->RoomFills[I].Get();
        TestEqual(TEXT("Room bounce stays fixed through possession and respawn"),Fill->GetComponentLocation(),BouncePositions[I]);
        TestTrue(TEXT("Room bounce remains visible without player"),Fill->IsVisible());
        TestFalse(TEXT("Room bounce has no shadows"),Fill->CastShadows);
        TestFalse(TEXT("Room bounce has no nav influence"),Fill->CanEverAffectNavigation());
    }
    World->DestroyWorld(false); GEngine->DestroyWorldContext(World);
    return true;
}
