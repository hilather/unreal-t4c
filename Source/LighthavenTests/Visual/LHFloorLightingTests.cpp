#include "Misc/AutomationTest.h"
#include "Visual/LHFloorLighting.h"
#include "Visual/LHVisualKit.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/SceneComponent.h"
#include "ProceduralMeshComponent.h"
#include "Components/ExponentialHeightFogComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHFloorFixtureRebuildTest,"Lighthaven.Visual.FloorFixtureRebuild",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHFloorFixtureRebuildTest::RunTest(const FString& Parameters)
{
    UWorld* World=UWorld::CreateWorld(EWorldType::Game,false);
    if(!TestNotNull(TEXT("Fixture world"),World)) return false;
    auto* Piece=World->SpawnActor<ALHVisualPiece>();
    FLHVisualRecipe Recipe;
    for(const auto Style:{ELHVisualStyle::Church,ELHVisualStyle::B2Damp,ELHVisualStyle::B3Crypt,ELHVisualStyle::B4Ritual})
    {
        const auto* Profile=LHFloorLighting::FindProfile(Style);
        if(!TestNotNull(TEXT("Floor profile"),Profile)) continue;
        for(const FName Id:{FName(TEXT("Presentation.Environment.Shared.Torch")),FName(TEXT("Presentation.Environment.Shared.Sconce"))})
        {
            TestTrue(TEXT("Fixture recipe"),LHVisual::MakeRecipe(Id,Style,Recipe));
            TestTrue(TEXT("Fixture build"),Piece->Build(Recipe));
            auto* Light=Piece->FindComponentByClass<ULHFloorFixtureLightComponent>();
            Piece->Build(Recipe);
            TArray<ULHFloorFixtureLightComponent*> Lights; Piece->GetComponents(Lights);
            TestEqual(TEXT("Repeated build owns exactly one fixture light"),Lights.Num(),1);
            TestEqual(TEXT("Repeated build reuses light"),Piece->FindComponentByClass<ULHFloorFixtureLightComponent>(),Light);
            if(!Light) continue;
            TestEqual(TEXT("Fixture follows flame"),Light->GetRelativeLocation(),FVector(0,25,14));
            TestEqual(TEXT("Floor fixture lumen tuning"),Light->Intensity,Profile->FixtureLumens);
            TestEqual(TEXT("Floor fixture radius tuning"),Light->AttenuationRadius,Profile->FixtureRadius);
            TestEqual(TEXT("Floor fixture kelvin"),Light->Temperature,Profile->FixtureKelvin);
            TestEqual(TEXT("Lumens"),Light->IntensityUnits,ELightUnits::Lumens);
            TestFalse(TEXT("Fixture casts no shadows"),Light->CastShadows);
            TestFalse(TEXT("Fixture does not affect navigation"),Light->CanEverAffectNavigation());
        }
        LHVisual::MakeRecipe(TEXT("Presentation.Environment.Shared.Altar"),Style,Recipe);
        Piece->Build(Recipe);
        const int32 AltarColliders=Piece->GetRecipe().Collision.Num();
        const auto AltarCollisionMode=Piece->GetMesh()->GetCollisionEnabled();
        ULHFloorFixtureLightComponent::Configure(Piece);
        TestEqual(TEXT("Adding altar key preserves collider inventory"),Piece->GetRecipe().Collision.Num(),AltarColliders);
        TestEqual(TEXT("Adding altar key preserves collision mode"),Piece->GetMesh()->GetCollisionEnabled(),AltarCollisionMode);
        auto* AltarLight=Piece->FindComponentByClass<ULHFloorFixtureLightComponent>();
        if(Style==ELHVisualStyle::Church || Style==ELHVisualStyle::B4Ritual)
        {
            if(TestNotNull(TEXT("Church and ritual altars have keys"),AltarLight))
            {
                TestEqual(TEXT("Altar key overhead"),AltarLight->GetRelativeLocation(),FVector(0,0,180));
                TestFalse(TEXT("Altar has no shadows"),AltarLight->CastShadows);
                TestTrue(TEXT("Altar key bounded"),AltarLight->Intensity>=3000.f && AltarLight->AttenuationRadius<=700.f);
                if(Style==ELHVisualStyle::B4Ritual)
                {
                    const auto Color=AltarLight->GetLightColor();
                    TestTrue(TEXT("Ritual altar glows red amber"),Color.R>Color.G && Color.G>Color.B);
                }
            }
        }
        else TestNull(TEXT("Damp and crypt altar props do not emit"),AltarLight);
        LHVisual::MakeRecipe(TEXT("Presentation.Environment.Shared.Barrel"),Style,Recipe);
        Piece->Build(Recipe);
        TestNull(TEXT("Nonfixture rebuild removes floor light"),Piece->FindComponentByClass<ULHFloorFixtureLightComponent>());
    }
    LHVisual::MakeRecipe(TEXT("Presentation.Environment.Shared.Sconce"),ELHVisualStyle::B1Cellar,Recipe);
    Piece->Build(Recipe);
    TestNull(TEXT("B1 keeps its own lighting"),Piece->FindComponentByClass<ULHFloorFixtureLightComponent>());
    TestNull(TEXT("B1 has no floor profile"),LHFloorLighting::FindProfile(ELHVisualStyle::B1Cellar));
    World->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHFloorReadabilityLifecycleTest,"Lighthaven.Visual.FloorReadabilityLifecycle",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHFloorReadabilityLifecycleTest::RunTest(const FString& Parameters)
{
    const auto Init=UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(true)
        .CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false);
    auto* World=UWorld::CreateWorld(EWorldType::Game,false,MakeUniqueObjectName(GetTransientPackage(),UWorld::StaticClass(),TEXT("FloorReadability")),GetTransientPackage(),true,ERHIFeatureLevel::Num,&Init);
    if(!TestNotNull(TEXT("Lifecycle world"),World)) return false;
    GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
    World->InitializeActorsForPlay(FURL());
    auto* Controller=World->SpawnActor<APlayerController>();
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
    auto* Atmosphere=World->SpawnActor<ALHFloorAtmosphere>();
    for(const auto Style:{ELHVisualStyle::Church,ELHVisualStyle::B2Damp,ELHVisualStyle::B3Crypt,ELHVisualStyle::B4Ritual})
    {
        Atmosphere->Configure(Style);
        const auto* Profile=LHFloorLighting::FindProfile(Style);
        const FVector FogOrigin=Atmosphere->Haze->GetComponentLocation();
        Controller->UnPossess(); Atmosphere->Tick(0);
        TestFalse(TEXT("No pawn hides fill"),Atmosphere->ReadabilityFill->IsVisible());
        Controller->Possess(First); Atmosphere->Tick(0);
        TestTrue(TEXT("Possession reveals fill"),Atmosphere->ReadabilityFill->IsVisible());
        const FVector Offset=Atmosphere->ReadabilityFill->GetComponentLocation()-First->GetActorLocation();
        TestTrue(TEXT("Fill is overhead"),Offset.Z>100 && Offset.X==0 && Offset.Y==0);
        First->SetActorLocation(FVector(800,-100,85)); Atmosphere->Tick(0);
        TestEqual(TEXT("Fill follows movement"),Atmosphere->ReadabilityFill->GetComponentLocation(),First->GetActorLocation()+Offset);
        Controller->Possess(Respawn); Atmosphere->Tick(0);
        TestEqual(TEXT("Fill follows respawn"),Atmosphere->ReadabilityFill->GetComponentLocation(),Respawn->GetActorLocation()+Offset);
        TestEqual(TEXT("Lighting never moves pawn"),Respawn->GetActorLocation(),FVector(-400,500,90));
        Controller->UnPossess(); Atmosphere->Tick(0);
        TestFalse(TEXT("Unpossession hides fill"),Atmosphere->ReadabilityFill->IsVisible());
        TestEqual(TEXT("Haze never follows pawn"),Atmosphere->Haze->GetComponentLocation(),FogOrigin);
        TestEqual(TEXT("Style resets player fill"),Atmosphere->ReadabilityFill->Intensity,Profile->PlayerLumens);
        TestFalse(TEXT("Player fill has no shadows"),Atmosphere->ReadabilityFill->CastShadows);
    }
    World->DestroyWorld(false); GEngine->DestroyWorldContext(World);
    return true;
}
