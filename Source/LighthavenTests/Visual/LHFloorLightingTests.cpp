#include "Misc/AutomationTest.h"
#include "Visual/LHFloorLighting.h"
#include "Visual/LHVisualKit.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/Scene.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/SceneComponent.h"
#include "ProceduralMeshComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

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
        TestFalse(TEXT("Haze remains nonvolumetric"),Atmosphere->Haze->bEnableVolumetricFog);
        TestEqual(TEXT("Haze opacity is profile bounded"),Atmosphere->Haze->FogMaxOpacity,Profile->HazeOpacity);
        TestEqual(TEXT("Depth haze applies only to crypt and ritual void"),Atmosphere->Haze->StartDistance,
            Style==ELHVisualStyle::B3Crypt || Style==ELHVisualStyle::B4Ritual?4000.f:0.f);
    }
    Atmosphere->Configure(ELHVisualStyle::Church);
    TestEqual(TEXT("Leaving ritual resets distant haze"),Atmosphere->Haze->StartDistance,0.f);
    World->DestroyWorld(false); GEngine->DestroyWorldContext(World);
    return true;
}

// Guard the W5-22c intent in the settings actually delivered to the scene:
// ritual ambient delivery falls while local keys and the player remain readable;
// crypt exposure stays fixed while its diffuse footprint reaches floor gaps.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHFloorPolishContrastTest,"Lighthaven.Visual.FloorPolishContrast",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHFloorPolishContrastTest::RunTest(const FString& Parameters)
{
    const auto* Ritual=LHFloorLighting::FindProfile(ELHVisualStyle::B4Ritual);
    const auto* Crypt=LHFloorLighting::FindProfile(ELHVisualStyle::B3Crypt);
    if(!TestNotNull(TEXT("Ritual profile"),Ritual) || !TestNotNull(TEXT("Crypt profile"),Crypt)) return false;
    const float RitualAmbient=Ritual->FillLumens*Ritual->FillColor.GetLuminance()/FMath::Pow(2.f,Ritual->EV100);
    const float PriorRitualAmbient=14000.f*FLinearColor(.82f,.49f,.32f).GetLuminance()/FMath::Pow(2.f,1.5f);
    TestTrue(TEXT("Ritual ambient exposure falls by at least half"),RitualAmbient<PriorRitualAmbient*.5f);
    TestTrue(TEXT("Ritual sky remains restrained"),Ritual->SkyIntensity<=.12f);
    const float DarkPassAmbient=6500.f*FLinearColor(.62f,.44f,.38f).GetLuminance()/FMath::Pow(2.f,2.25f);
    TestTrue(TEXT("Ritual ambient rises modestly from the too-dark pass"),RitualAmbient>DarkPassAmbient && RitualAmbient<DarkPassAmbient*1.4f);
    TestTrue(TEXT("Ritual ambient is cool, separate from the warm fixture pools"),Ritual->FillColor.B>Ritual->FillColor.G && Ritual->FillColor.G>Ritual->FillColor.R);
    TestEqual(TEXT("Ritual exposure preserves the accepted entrance pools"),Ritual->EV100,2.25f);
    TestTrue(TEXT("Ritual fixture pools contract"),Ritual->FixtureRadius<450.f);
    TestTrue(TEXT("Ritual fixture keys retained"),Ritual->FixtureLumens>=1500.f);
    TestEqual(TEXT("Ritual player fill retained"),Ritual->PlayerLumens,650.f);
    TestEqual(TEXT("Crypt exposure retained"),Crypt->EV100,4.f);
    TestTrue(TEXT("Crypt diffuse radius reaches strip gaps"),Crypt->FillRadius>1450.f);
    TestTrue(TEXT("Crypt diffuse energy restrained"),Crypt->FillLumens<4500.f);
    TestEqual(TEXT("Crypt player fill retained"),Crypt->PlayerLumens,600.f);
    FPostProcessSettings RitualSettings,CryptSettings;
    LHFloorLighting::ConfigurePostProcess(RitualSettings,ELHVisualStyle::B4Ritual);
    LHFloorLighting::ConfigurePostProcess(CryptSettings,ELHVisualStyle::B3Crypt);
    TestTrue(TEXT("Ritual exposure is manual"),RitualSettings.bOverride_AutoExposureMethod && RitualSettings.AutoExposureMethod==EAutoExposureMethod::AEM_Manual);
    TestEqual(TEXT("Ritual shutter delivers requested exposure"),RitualSettings.CameraShutterSpeed,FMath::Pow(2.f,Ritual->EV100)/4.f);
    TestTrue(TEXT("Crypt AO shadow reduction is active"),CryptSettings.bOverride_AmbientOcclusionIntensity && CryptSettings.AmbientOcclusionIntensity<.65f);
    TestEqual(TEXT("Ritual AO contrast retained"),RitualSettings.AmbientOcclusionIntensity,.55f);
    TestTrue(TEXT("Ritual shadow grade favors cold basalt"),RitualSettings.ColorGainShadows.X<RitualSettings.ColorGainShadows.Z);
    TestEqual(TEXT("Polish uses no Lumen GI"),RitualSettings.DynamicGlobalIlluminationMethod,EDynamicGlobalIlluminationMethod::None);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHRitualGlowRebuildTest,"Lighthaven.Visual.RitualGlowRebuild",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHRitualGlowRebuildTest::RunTest(const FString& Parameters)
{
    auto* World=UWorld::CreateWorld(EWorldType::Game,false);
    if(!TestNotNull(TEXT("Ritual glow world"),World)) return false;
    auto* Piece=World->SpawnActor<ALHVisualPiece>(FVector(0,3000,0),FRotator::ZeroRotator);
    FLHVisualRecipe Recipe;
    LHVisual::MakeRecipe(TEXT("Presentation.Environment.Shared.Floor400"),ELHVisualStyle::B4Ritual,Recipe);
    Recipe.Collision.Reset();
    TestTrue(TEXT("Build ritual floor"),Piece->Build(Recipe));
    auto* Imported=Piece->GetImportedMesh();
    TestTrue(TEXT("Real imported sigil mesh required"),Imported && Imported->IsVisible() && Imported->GetStaticMesh());
    TArray<ULHRitualSigilLightComponent*> First; Piece->GetComponents(First);
    TestEqual(TEXT("Full 400cm tile carries one local sigil reflection"),First.Num(),1);
    Piece->Build(Recipe);
    TArray<ULHRitualSigilLightComponent*> Rebuilt; Piece->GetComponents(Rebuilt);
    TestEqual(TEXT("Rebuild never accumulates sigil lights"),Rebuilt.Num(),First.Num());
    if(First.Num()==1 && Rebuilt.Num()==1)
    {
        auto* Light=Rebuilt[0];
        TestEqual(TEXT("Rebuild reuses component"),Light,First[0]);
        TestEqual(TEXT("Deep hall glow is a low local source"),Light->Intensity,650.f);
        TestEqual(TEXT("Glow stays near its medallion"),Light->AttenuationRadius,360.f);
        TestEqual(TEXT("Glow anchored to existing tile inlay"),Light->GetRelativeLocation(),FVector(200,200,120));
        TestFalse(TEXT("Sigil glow casts no shadows"),Light->CastShadows);
        TestFalse(TEXT("Sigil glow cannot affect nav"),Light->CanEverAffectNavigation());
        TestEqual(TEXT("Sigil diffuse only"),Light->SpecularScale,0.f);
        Piece->SetActorLocation(FVector(0,0,0)); Piece->Build(Recipe);
        TestEqual(TEXT("Entrance retains gentler local reflection"),Light->Intensity,325.f);
    }
    int32 Inlays=0,Stone=0;
    if(Imported) for(int32 Slot=0;Slot<Imported->GetNumMaterials();++Slot)
    {
        auto* M=Cast<UMaterialInstanceDynamic>(Imported->GetMaterial(Slot));
        if(!M || !M->Parent) continue;
        float Flame=-1.f;
        TestTrue(TEXT("Cooked material exposes emission control"),M->GetScalarParameterValue(FMaterialParameterInfo(TEXT("Flame")),Flame));
        if(M->Parent->GetName()==TEXT("MI_B4Ritual_oxide"))
        {
            ++Inlays; TestEqual(TEXT("Existing carved inlay emits"),Flame,1.f);
            FLinearColor Color;
            TestTrue(TEXT("Glow color exists"),M->GetVectorParameterValue(FMaterialParameterInfo(TEXT("FlameColor")),Color));
            TestTrue(TEXT("Inlay glow is red amber"),Color.R>Color.G && Color.G>Color.B);
            FLinearColor Clip;
            TestTrue(TEXT("Glow retains authored clip bounds"),M->GetVectorParameterValue(FMaterialParameterInfo(TEXT("ClipExtent")),Clip));
            TestTrue(TEXT("Glow is clipped to actual floor"),Clip.R<1000 && Clip.G<1000);
        }
        else {++Stone; TestEqual(TEXT("Basalt stays nonemissive"),Flame,0.f);}
    }
    TestEqual(TEXT("One inlay slot is lit"),Inlays,1);
    TestTrue(TEXT("Basalt slot is retained"),Stone>0);
    // A clipped tile must not leave an invisible off-surface light behind.
    auto Cropped=Recipe;
    Cropped.Geometry.SetNum(1); Cropped.Geometry[0].Center=FVector(50,50,-10); Cropped.Geometry[0].Size=FVector(100,100,20);
    TestTrue(TEXT("Build cropped tile"),Piece->Build(Cropped));
    Rebuilt.Reset(); Piece->GetComponents(Rebuilt);
    TestEqual(TEXT("Clipped medallion removes its reflected light"),Rebuilt.Num(),0);
    LHVisual::MakeRecipe(TEXT("Presentation.Environment.Shared.Barrel"),ELHVisualStyle::B4Ritual,Recipe);
    Recipe.Collision.Reset(); Piece->Build(Recipe);
    Rebuilt.Reset(); Piece->GetComponents(Rebuilt);
    TestEqual(TEXT("Unrelated props carry no ritual glow"),Rebuilt.Num(),0);
    TestEqual(TEXT("Glow adds no collision"),Piece->GetBlockers().Num(),0);
    World->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHChurchExteriorRebuildTest,"Lighthaven.Visual.ChurchExteriorRebuild",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHChurchExteriorRebuildTest::RunTest(const FString& Parameters)
{
    auto* World=UWorld::CreateWorld(EWorldType::Game,false);
    if(!TestNotNull(TEXT("Exterior rebuild world"),World)) return false;
    auto* Piece=World->SpawnActor<ALHVisualPiece>();
    FLHVisualRecipe Recipe;
    LHVisual::MakeRecipe(TEXT("Presentation.Environment.Shared.Floor400"),ELHVisualStyle::Church,Recipe);
    Recipe.Collision.Reset();
    const auto Interior=Recipe;
    Recipe.Colors[0]=FLinearColor(.061f,.081f,.11f,1.f);
    Piece->Tags.Add(TEXT("LH.Church.Surroundings"));
    Piece->Build(Recipe); Piece->Build(Recipe);
    TestFalse(TEXT("Rebuilt apron excludes chapel lights"),Piece->GetImportedMesh()->LightingChannels.bChannel0);
    TestTrue(TEXT("Rebuilt apron receives moon channel"),Piece->GetImportedMesh()->LightingChannels.bChannel1);
    Piece->Tags.Remove(TEXT("LH.Church.Surroundings"));
    Piece->Build(Interior);
    TestTrue(TEXT("Recipe restored to interior receives chapel lights again"),Piece->GetImportedMesh()->LightingChannels.bChannel0);
    TestFalse(TEXT("Recipe restored to interior excludes exterior moon"),Piece->GetImportedMesh()->LightingChannels.bChannel1);
    for(int32 Slot=0;Slot<Piece->GetImportedMesh()->GetNumMaterials();++Slot)
    {
        auto* M=Cast<UMaterialInstanceDynamic>(Piece->GetImportedMesh()->GetMaterial(Slot));
        float Flame=-1;
        if(TestNotNull(TEXT("Imported church material rebuilt"),M))
        {
            TestTrue(TEXT("Material exposes Flame"),M->GetScalarParameterValue(FMaterialParameterInfo(TEXT("Flame")),Flame));
            TestEqual(TEXT("Interior surface does not retain exterior emission"),Flame,0.f);
        }
    }
    World->DestroyWorld(false);
    return true;
}
