#include "Misc/AutomationTest.h"
#include "Visual/LHB1Lighting.h"
#include "Visual/LHVisualKit.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "World/LHAreaRegistry.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/PostProcessVolume.h"
#include "Components/LightComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/PointLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "UObject/Package.h"
#include "UObject/LinkerInstancingContext.h"
#include "Engine/Engine.h"
#include "Engine/Level.h"
#include "Misc/App.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHLightingAuditTest,"Lighthaven.World.LightingAudit",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHLightingAuditTest::RunTest(const FString& Parameters)
{
    int32 Index=0;
    const int32 LandmarkLocal[]={20,12,17,17,13}; // Existing landmark inventory remains intact.
    for(const auto& Area:LHWorld::Registry())
    {
        UPackage* Package=LoadPackage(nullptr,*Area.Map.GetLongPackageName(),LOAD_None);
        UWorld* World=Package?UWorld::FindWorldInPackage(Package):nullptr;
        if(!TestNotNull(*Area.Map.ToString(),World)) { ++Index; continue; }
        int32 Local=0,Sun=0,Sky=0,Exposure=0,Coverage=0,Atmosphere=0,Fixtures=0;
        TArray<FVector> CoveragePositions;
        for(TActorIterator<AActor> It(World);It;++It)
        {
            if(auto* A=Cast<ALHB1Atmosphere>(*It))
            {
                ++Atmosphere;
                TestEqual(TEXT("B1 haze density"),A->Haze->FogDensity,.008f);
                TestFalse(TEXT("B1 no volumetric fog"),A->Haze->bEnableVolumetricFog);
                TestEqual(TEXT("B1 haze bounded opacity"),A->Haze->FogMaxOpacity,.12f);
                TestEqual(TEXT("B1 cool haze radiance"),A->Haze->FogInscatteringLuminance,FLinearColor(.10f,.14f,.20f));
            }
            if(auto* Piece=Cast<ALHVisualPiece>(*It))
            {
                const auto& R=Piece->GetRecipe(); const FString Id=R.Id.ToString();
                const bool Eligible=Index==1 && R.Style==ELHVisualStyle::B1Cellar &&
                    (Id.EndsWith(TEXT(".Torch")) || Id.EndsWith(TEXT(".Sconce")));
                TArray<ULHB1TorchLightComponent*> TorchLights; Piece->GetComponents(TorchLights);
                TestEqual(TEXT("Exactly one light per B1 fixture, zero elsewhere"),TorchLights.Num(),Eligible?1:0);
                if(Eligible) ++Fixtures;
            }
            TArray<ULightComponent*> Lights; It->GetComponents(Lights);
            for(auto* C:Lights)
            {
                TestEqual(TEXT("No unbuilt static/stationary light"),C->Mobility,EComponentMobility::Movable);
                if(auto* P=Cast<UPointLightComponent>(C))
                {
                    ++Local;
                    const bool bCoverage=It->GetName().StartsWith(TEXT("Coverage_")) || (It->GetName().StartsWith(TEXT("A04_T1")) && It->GetName().Len()==9);
                    if(bCoverage) { ++Coverage; CoveragePositions.Add(It->GetActorLocation()); }
                    TArray<UPrimitiveComponent*> Primitives; It->GetComponents(Primitives);
                    for(auto* Primitive:Primitives) TestEqual(TEXT("Light has no blocking collision"),Primitive->GetCollisionEnabled(),ECollisionEnabled::NoCollision);
                    TestEqual(TEXT("Local units lumens"),P->IntensityUnits,ELightUnits::Lumens);
                    if(Index==1)
                    {
                        TestFalse(TEXT("B1 no legacy grid light"),It->GetName().StartsWith(TEXT("A04_T")));
                        TestFalse(TEXT("B1 point lights cast no shadows"),P->CastShadows);
                        TestTrue(TEXT("B1 bounded pool radius"),P->AttenuationRadius<=750.f);
                        TestTrue(TEXT("B1 pool or readability intensity"),P->Intensity>=450.f && P->Intensity<=1908.f);
                    }
                    else TestTrue(TEXT("Local intensity Prototype 600..4000 lm"),P->Intensity>=600 && P->Intensity<=4000);
                    if(Index>1) TestTrue(TEXT("Interior local radius at least 1400cm"),P->AttenuationRadius>=1400);
                }
                else if(Cast<UDirectionalLightComponent>(C)) { ++Sun; TestEqual(TEXT("Sun 3000 lux"),C->Intensity,3000.f); }
            }
            TArray<USkyLightComponent*> Skies; It->GetComponents(Skies);
            for(auto* C:Skies)
            {
                ++Sky; TestEqual(TEXT("Sky movable"),C->Mobility,EComponentMobility::Movable);
                TestEqual(TEXT("Prototype neutral sky intensity"),C->Intensity,Index==0?.7f:(Index==1?.12f:.8f));
                TestFalse(TEXT("Fill casts no shadows"),C->CastShadows);
                if(Index>0) TestFalse(TEXT("Interior lower hemisphere fill"),C->bLowerHemisphereIsBlack);
                TestEqual(TEXT("Specified sky cube"),C->SourceType,SLS_SpecifiedCubemap);
                TestNotNull(TEXT("Sky cube must cook"),C->Cubemap.Get());
            }
            if(auto* PP=Cast<APostProcessVolume>(*It))
            {
                ++Exposure; const auto& S=PP->Settings;
                if(Index==1)
                {
                    TestTrue(TEXT("B1 AO"),S.bOverride_AmbientOcclusionIntensity && S.AmbientOcclusionIntensity==.65f);
                    TestTrue(TEXT("B1 AO world radius"),S.bOverride_AmbientOcclusionRadius && S.AmbientOcclusionRadius==120.f && S.bOverride_AmbientOcclusionRadiusInWS && S.AmbientOcclusionRadiusInWS);
                    TestTrue(TEXT("B1 grade"),S.bOverride_ColorSaturation && S.ColorSaturation==FVector4(.95f,.95f,.95f,1.f) && S.bOverride_ColorGainHighlights && S.ColorGainHighlights==FVector4(1.04f,1.01f,.96f,1.f) && S.bOverride_ColorGainShadows && S.ColorGainShadows==FVector4(.97f,1.f,1.04f,1.f));
                    TestTrue(TEXT("B1 flame bloom"),S.bOverride_BloomIntensity && S.BloomIntensity==.25f);
                    TestTrue(TEXT("B1 vignette"),S.bOverride_VignetteIntensity && S.VignetteIntensity==.2f);
                }
                TestTrue(TEXT("Unbound exposure"),PP->bUnbound);
                TestTrue(TEXT("Manual override"),S.bOverride_AutoExposureMethod && S.AutoExposureMethod==AEM_Manual);
                TestTrue(TEXT("Physical override"),S.bOverride_AutoExposureApplyPhysicalCameraExposure && S.AutoExposureApplyPhysicalCameraExposure);
                const float EV=Index==0?10.f:(Index>=3?4.f:2.5f);
                TestTrue(TEXT("Physical camera EV"),S.bOverride_CameraISO && S.bOverride_CameraShutterSpeed && S.bOverride_DepthOfFieldFstop && FMath::IsNearlyEqual(FMath::Log2(FMath::Square(S.DepthOfFieldFstop)*S.CameraShutterSpeed*100/S.CameraISO),EV));
                TestTrue(TEXT("Fixed bounds"),S.bOverride_AutoExposureMinBrightness && S.bOverride_AutoExposureMaxBrightness && S.AutoExposureMinBrightness==EV && S.AutoExposureMaxBrightness==EV);
                TestTrue(TEXT("Zero bias"),S.bOverride_AutoExposureBias && S.AutoExposureBias==0);
            }
        }
        if(Index==1)
        {
            TestEqual(TEXT("One B1 atmosphere"),Atmosphere,1);
            TestEqual(TEXT("No B1 grid lights"),Coverage,0);
            // 12 landmarks + 47 rectangle coverage samples retained as visual fixtures.
            TestEqual(TEXT("B1 authored fixture inventory"),Fixtures,59);
            TestEqual(TEXT("B1 fixtures plus one readability fill"),Local,Fixtures+1);
        }
        else
        {
            TestEqual(TEXT("No B1 atmosphere on other floors"),Atmosphere,0);
            TestEqual(TEXT("Landmark light inventory retained"),Local-Coverage,LandmarkLocal[Index]);
        }
        if(Index>1)
        {
            TestTrue(TEXT("Interior coverage lights present"),Coverage>=10);
            // Every canonical encounter and entrance needs nearby fill, independently of inventory.
            for(const auto& Spawn:Area.Spawns)
                TestTrue(*FString::Printf(TEXT("Fill reaches encounter %s"),*Spawn.Alias.ToString()),
                    CoveragePositions.ContainsByPredicate([&](const FVector& P){return FVector::DistSquared2D(P,Spawn.Anchor.GetLocation())<=FMath::Square(1400.); }));
            for(const auto& Entry:Area.Entrances)
                TestTrue(*FString::Printf(TEXT("Fill reaches entrance %s"),*Entry.Id.LocalId.ToString()),
                    CoveragePositions.ContainsByPredicate([&](const FVector& P){return FVector::DistSquared2D(P,Entry.SafeTransform.GetLocation())<=FMath::Square(1400.); }));
        }
        TestEqual(TEXT("Directional inventory"),Sun,Index==0?1:0);
        TestEqual(TEXT("One sky"),Sky,1); TestEqual(TEXT("One exposure"),Exposure,1);
        ++Index;
    }
    TestEqual(TEXT("Five maps audited"),Index,5);
    return true;
}

namespace LHB1LoadedLightingAuditPrivate
{
// UE5.8 PointLightComponent.cpp: lumens -> cm^2 candela = lm*10000/(4*pi).
// DeferredLightingCommon.ush:260 radius window (1-(d^2/r^2)^2)^2.
// GetCapsule:309 sets distance bias 1; CapsuleLightIntegrate.ush:56 gives
// 1/(d^2+1) for zero-length point lights (sphere horizon wrap excluded).
// Unoccluded point-source photometric estimate, before temperature/tint, finite-source
// integration, material, skylight, fog, exposure and tonemapping. This is not pixel luma.
double DirectLux(FVector Light,FVector Surface,FVector Normal,double Lumens,double Radius)
{
    const FVector Delta=Light-Surface;
    const double D2=Delta.SizeSquared();
    const double Window=FMath::Square(FMath::Max(0.,1.-FMath::Square(D2/FMath::Square(Radius))));
    return Lumens*10000./(4.*PI)*Window/(D2+1.)*
        FMath::Max(0.,FVector::DotProduct(Delta.GetSafeNormal(),Normal));
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHB1LoadedGameLightingTest,"Lighthaven.World.B1LoadedGameLightingAudit",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHB1LoadedGameLightingTest::RunTest(const FString& Parameters)
{
    using namespace LHB1LoadedLightingAuditPrivate;
    const FString Map=LHWorld::Registry()[1].Map.GetLongPackageName();
    const FString Instance=TEXT("/Temp/B1LoadedLighting_")+FGuid::NewGuid().ToString(EGuidFormats::Digits);
    FLinkerInstancingContext Instancing;
    Instancing.AddPackageMapping(FName(*Map),FName(*Instance));
    // Read serialized recipes afresh: duplicating a previously rebuilt editor map
    // can conceal a PostLoad registration defect with copied transient components.
    UWorld::WorldTypePreLoadMap.Add(FName(*Instance),EWorldType::Game);
    UPackage* Package=LoadPackage(CreatePackage(*Instance),*Map,LOAD_None,nullptr,&Instancing);
    UWorld::WorldTypePreLoadMap.Remove(FName(*Instance));
    UWorld* World=Package?UWorld::FindWorldInPackage(Package):nullptr;
    if(!TestNotNull(TEXT("Fresh serialized B1 world"),World)) return false;
    TestEqual(TEXT("Loaded as game world, not editor world"),World->WorldType,EWorldType::Game);
    TestFalse(TEXT("Serialized game world starts uninitialized"),World->IsInitialized());
    TestTrue(TEXT("Serialized game world has no scene before InitWorld"),World->Scene==nullptr);
    int32 PreInitFixtureLights=0;
    // TActorIterator filters actors in uninitialized levels; inspect serialized
    // PersistentLevel actors directly so pre-scene registration is observable.
    if(TestNotNull(TEXT("Serialized persistent level"),World->PersistentLevel.Get()))
    {
        for(AActor* Actor:World->PersistentLevel->Actors)
        {
            auto* Piece=Cast<ALHVisualPiece>(Actor);
            if(!Piece) continue;
            TArray<ULHB1TorchLightComponent*> Lights; Piece->GetComponents(Lights);
            for(const auto* Light:Lights)
            {
                ++PreInitFixtureLights;
                TestFalse(*FString::Printf(TEXT("%s never registered before game scene exists"),*Piece->GetName()),Light->IsRegistered());
            }
        }
    }
    TestEqual(TEXT("Pre-init assertions examine every serialized torch light"),PreInitFixtureLights,59);
    GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
    if(!World->IsInitialized()) World->InitWorld(UWorld::InitializationValues().AllowAudioPlayback(false)
        .CreatePhysicsScene(true).CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false));
    World->UpdateWorldComponents(true,false);
    World->InitializeActorsForPlay(FURL());
    // Exercise real actor BeginPlay without starting gameplay/session or spawning a pawn.
    for(TActorIterator<AActor> It(World);It;++It) if(!It->HasActorBegunPlay()) It->DispatchBeginPlay();
    TArray<ULHB1TorchLightComponent*> Fixtures;
    for(TActorIterator<ALHVisualPiece> It(World);It;++It)
    {
        const auto& Recipe=It->GetRecipe();
        const FString Id=Recipe.Id.ToString();
        if(Recipe.Style!=ELHVisualStyle::B1Cellar || !(Id.EndsWith(TEXT(".Torch")) || Id.EndsWith(TEXT(".Sconce")))) continue;
        TArray<ULHB1TorchLightComponent*> Lights; It->GetComponents(Lights);
        const FString Label=It->GetName();
        TestEqual(*FString::Printf(TEXT("%s exactly one loaded flame light"),*Label),Lights.Num(),1);
        for(auto* Light:Lights)
        {
            Fixtures.Add(Light);
            TestTrue(*FString::Printf(TEXT("%s registered in initialized game scene"),*Label),Light->IsRegistered() && Light->GetWorld()==World && World->Scene!=nullptr);
            if(FApp::CanEverRender())
                TestTrue(*FString::Printf(TEXT("%s render state created"),*Label),Light->IsRenderStateCreated());
            TestEqual(*FString::Printf(TEXT("%s lumens"),*Label),Light->IntensityUnits,ELightUnits::Lumens);
            TestEqual(*FString::Printf(TEXT("%s base intensity"),*Label),Light->Intensity,1800.f);
            TestEqual(*FString::Printf(TEXT("%s pool radius"),*Label),Light->AttenuationRadius,700.f);
            TestEqual(*FString::Printf(TEXT("%s source radius"),*Label),Light->SourceRadius,12.f);
            TestEqual(*FString::Printf(TEXT("%s zero source length"),*Label),Light->SourceLength,0.f);
            TestEqual(*FString::Printf(TEXT("%s white input tint before temperature"),*Label),Light->LightColor,FColor::White);
            TestEqual(*FString::Printf(TEXT("%s no distance culling"),*Label),Light->MaxDrawDistance,0.f);
            TestEqual(*FString::Printf(TEXT("%s no distance fade"),*Label),Light->MaxDistanceFadeRange,0.f);
            TestEqual(*FString::Printf(TEXT("%s movable"),*Label),Light->Mobility,EComponentMobility::Movable);
            TestTrue(*FString::Printf(TEXT("%s visible and affects world"),*Label),Light->IsVisible() && !Light->bHiddenInGame && Light->bAffectsWorld && !It->IsHidden());
            TestTrue(*FString::Printf(TEXT("%s physical falloff and temperature"),*Label),Light->bUseInverseSquaredFalloff && Light->bUseTemperature && Light->Temperature==2000.f);
            TestFalse(*FString::Printf(TEXT("%s shadow cost"),*Label),Light->CastShadows);
            TestTrue(*FString::Printf(TEXT("%s flame world position"),*Label),Light->GetComponentLocation().Equals(It->GetActorTransform().TransformPosition(FVector(0,25,14)),.01));
        }
    }
    TestEqual(TEXT("All serialized fixtures survive game load"),Fixtures.Num(),59);
    auto RuntimeLux=[&](FVector Surface,FVector Normal)
    {
        double Lux=0;
        for(const auto* Light:Fixtures)
            if(Light->IsRegistered() && Light->IsVisible() && !Light->bHiddenInGame && Light->bAffectsWorld &&
                Light->GetOwner() && !Light->GetOwner()->IsHidden())
                Lux+=DirectLux(Light->GetComponentLocation(),Surface,Normal,Light->Intensity,Light->AttenuationRadius);
        return Lux;
    };
    // Receiver planes near EVERY torch: floor below its flame and a vertical
    // wall-facing receiver 100 cm to its local -X, 64 cm below the flame.
    // These are light-delivery probes, not an assertion that a wall exists there.
    for(const auto* Light:Fixtures)
    {
        const FVector Flame=Light->GetComponentLocation();
        const FVector Floor(Flame.X,Flame.Y,0);
        const FVector Wall=Flame+FVector(-100,0,-64);
        const double FloorLux=RuntimeLux(Floor,FVector::UpVector);
        const double WallLux=RuntimeLux(Wall,FVector::ForwardVector);
        const double OwnFloor=DirectLux(Flame,Floor,FVector::UpVector,1800.,700.);
        const double OwnWall=DirectLux(Flame,Wall,FVector::ForwardVector,1800.,700.);
        AddInfo(FString::Printf(TEXT("B1 loaded fixture %s flame=%s registered=%d visible=%d hiddenInGame=%d affectsWorld=%d ownerHidden=%d lumens=%.1f radius=%.1f kelvin=%.1f inverseSquare=%d shadows=%d sourceRadius=%.1f sourceLength=%.1f inputTint=%s maxDrawDistance=%.1f fadeRange=%.1f floorReceiver=%s totalLux=%.4f ownLux=%.4f verticalReceiver=%s totalLux=%.4f ownLux=%.4f"),
            *Light->GetOwner()->GetName(),*Flame.ToString(),Light->IsRegistered(),Light->IsVisible(),Light->bHiddenInGame,Light->bAffectsWorld,Light->GetOwner()->IsHidden(),Light->Intensity,Light->AttenuationRadius,Light->Temperature,Light->bUseInverseSquaredFalloff,Light->CastShadows,Light->SourceRadius,Light->SourceLength,*Light->LightColor.ToString(),Light->MaxDrawDistance,Light->MaxDistanceFadeRange,*Floor.ToString(),FloorLux,OwnFloor,*Wall.ToString(),WallLux,OwnWall));
        TestTrue(*FString::Printf(TEXT("%s near floor lit pool >= own %.3f lux (actual %.3f)"),*Light->GetOwner()->GetName(),OwnFloor,FloorLux),FloorLux>=OwnFloor-.001 && FloorLux>=19.);
        TestTrue(*FString::Printf(TEXT("%s near vertical receiver lit pool >= own %.3f lux (actual %.3f)"),*Light->GetOwner()->GetName(),OwnWall,WallLux),WallLux>=OwnWall-.001 && WallLux>=80.);
    }
    struct FLegacyLight { FVector Position; double Lumens; };
    // Reconstruct the native pre-W5-09 grid independently from generator B1()
    // and CoverageLighting(); no transient runtime light supplies this baseline.
    TArray<FLegacyLight> Legacy={
        {{200,-2000,250},1800},{{770,-500,250},900},{{900,40,250},900},
        {{40,1200,250},900},{{1760,1200,250},900},{{-1200,1200,250},1500},
        {{-3160,1000,250},1800},{{2400,1030,250},900},{{4000,1000,250},1500},
        {{5360,700,250},900},{{3300,2560,250},900},{{4850,1800,250},1800}
    };
    struct FRect { double X0,X1,Y0,Y1; };
    const FRect Rects[]={{0,18,-28,-10},{0,18,0,18},{-3,0,3,7},{18,21,13,17},
        {-32,-10,0,20},{30,54,0,26},{7.4,10.6,-10,0},{-10,0,7.4,10.6},{18,30,7.4,10.6}};
    for(const auto& Rect:Rects)
    {
        const int32 NX=FMath::CeilToInt((Rect.X1-Rect.X0)/8.);
        const int32 NY=FMath::CeilToInt((Rect.Y1-Rect.Y0)/8.);
        for(int32 X=0;X<NX;++X) for(int32 Y=0;Y<NY;++Y)
            Legacy.Add({FVector((Rect.X0+(X+.5)*(Rect.X1-Rect.X0)/NX)*100,
                (Rect.Y0+(Y+.5)*(Rect.Y1-Rect.Y0)/NY)*100,250),1800.});
    }
    TestEqual(TEXT("Independent original landmark and coverage count"),Legacy.Num(),59);
    struct FSample { const TCHAR* Name; FVector Position,Normal; double ExpectedNew,ExpectedOld; };
    // Independent reference values from the original B1 authoring rectangles:
    // 47 ceil(width/8m)*ceil(depth/8m) coverage centres at 1800 lm plus 12
    // landmarks at their N/W/T 1800/1500/900 lm profiles, all z250/r1400.
    // New references use the same fixture positions plus flame (0,25,14),
    // 1800 lm/r700; floor z0 and inward-facing vertical surface are fixed cm.
    const FSample Samples[]={
        {TEXT("near fixture floor"),{200,-1975,0},{0,0,1},33.4760,42.5590},
        {TEXT("near fixture vertical surface"),{100,-1975,200},{1,0,0},108.3937,127.5781},
        {TEXT("entry centre"),{900,-1900,0},{0,0,1},19.7737,29.8458},
        {TEXT("hub centre"),{900,900,0},{0,0,1},19.7737,29.7579},
        {TEXT("healer centre"),{-2100,1000,0},{0,0,1},19.4530,26.8730},
        {TEXT("east centre"),{4200,1300,0},{0,0,1},11.4277,15.9162},
        {TEXT("entry dark corner"),{100,-2700,0},{0,0,1},4.6971,8.1876},
        {TEXT("hub dark corner"),{100,100,0},{0,0,1},5.4557,10.8801},
        {TEXT("healer dark corner"),{-3100,100,0},{0,0,1},2.6932,4.9602},
        {TEXT("east dark corner"),{5300,2500,0},{0,0,1},2.9468,4.5416}
    };
    for(const auto& Sample:Samples)
    {
        const double Lux=RuntimeLux(Sample.Position,Sample.Normal);
        double OldLux=0;
        for(const auto& Light:Legacy)
            OldLux+=DirectLux(Light.Position,Sample.Position,Sample.Normal,Light.Lumens,1400.);
        TestTrue(*FString::Printf(TEXT("%s independent original grid lux"),Sample.Name),FMath::Abs(OldLux-Sample.ExpectedOld)<.02);
        if(FString(Sample.Name).Contains(TEXT("dark corner")))
            TestTrue(*FString::Printf(TEXT("%s stays below 6 direct lux"),Sample.Name),Lux>0. && Lux<6.);
        else if(FString(Sample.Name).Contains(TEXT("centre")))
            TestTrue(*FString::Printf(TEXT("%s remains readable >=10 direct lux"),Sample.Name),Lux>=10.);
        TestTrue(*FString::Printf(TEXT("%s expected unoccluded direct lux %.4f, actual %.4f"),Sample.Name,Sample.ExpectedNew,Lux),FMath::Abs(Lux-Sample.ExpectedNew)<.02);
        TestTrue(*FString::Printf(TEXT("%s reduced vs legacy grid"),Sample.Name),Lux<Sample.ExpectedOld);
        AddInfo(FString::Printf(TEXT("B1 photometric prediction %s: new %.4f lux / legacy %.4f lux; excludes sky/fill/colour/render response"),Sample.Name,Lux,Sample.ExpectedOld));
    }
    AddInfo(TEXT("Legacy host rendered mean approximately .3; proposed .15-.25 new rendered mean remains an unobserved tuning prediction. Direct lux is not exported sRGB luma acceptance."));
    // DestroyWorld unregisters components/releases the scene and clears world
    // standalone/root flags. Remove this isolated context as well; the unique
    // /Temp package can then be reclaimed without disturbing shared editor maps.
    World->DestroyWorld(false); GEngine->DestroyWorldContext(World);
    Package->ClearFlags(RF_Standalone);
    TestTrue(TEXT("Isolated game-world context removed"),GEngine->GetWorldContextFromWorld(World)==nullptr);
    return true;
}
