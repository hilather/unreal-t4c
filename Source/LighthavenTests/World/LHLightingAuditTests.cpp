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
