#include "Misc/AutomationTest.h"
#include "World/LHAreaRegistry.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/PostProcessVolume.h"
#include "Components/LightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "UObject/Package.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHLightingAuditTest,"Lighthaven.World.LightingAudit",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHLightingAuditTest::RunTest(const FString& Parameters)
{
    int32 Index=0;
    const int32 ExpectedLocal[]={20,12,17,17,13}; // B3/B4 counts checked below against authored inventory.
    for(const auto& Area:LHWorld::Registry())
    {
        UPackage* Package=LoadPackage(nullptr,*Area.Map.GetLongPackageName(),LOAD_None);
        UWorld* World=Package?UWorld::FindWorldInPackage(Package):nullptr;
        if(!TestNotNull(*Area.Map.ToString(),World)) { ++Index; continue; }
        int32 Local=0,Sun=0,Sky=0,Exposure=0;
        for(TActorIterator<AActor> It(World);It;++It)
        {
            TArray<ULightComponent*> Lights; It->GetComponents(Lights);
            for(auto* C:Lights)
            {
                TestEqual(TEXT("No unbuilt static/stationary light"),C->Mobility,EComponentMobility::Movable);
                if(auto* P=Cast<UPointLightComponent>(C))
                {
                    ++Local; TestEqual(TEXT("Local units lumens"),P->IntensityUnits,ELightUnits::Lumens);
                    TestTrue(TEXT("Local intensity 275..3200 lm"),P->Intensity>=275 && P->Intensity<=3200);
                }
                else if(Cast<UDirectionalLightComponent>(C)) { ++Sun; TestEqual(TEXT("Sun 3000 lux"),C->Intensity,3000.f); }
            }
            TArray<USkyLightComponent*> Skies; It->GetComponents(Skies);
            for(auto* C:Skies)
            {
                ++Sky; TestEqual(TEXT("Sky movable"),C->Mobility,EComponentMobility::Movable);
                TestTrue(TEXT("Sky intensity .26.. .70"),C->Intensity>=.259f && C->Intensity<=.701f);
                TestEqual(TEXT("Specified sky cube"),C->SourceType,SLS_SpecifiedCubemap);
                TestNotNull(TEXT("Sky cube must cook"),C->Cubemap.Get());
            }
            if(auto* PP=Cast<APostProcessVolume>(*It))
            {
                ++Exposure; const auto& S=PP->Settings;
                TestTrue(TEXT("Unbound exposure"),PP->bUnbound);
                TestTrue(TEXT("Manual override"),S.bOverride_AutoExposureMethod && S.AutoExposureMethod==AEM_Manual);
                TestTrue(TEXT("Physical override"),S.bOverride_AutoExposureApplyPhysicalCameraExposure && S.AutoExposureApplyPhysicalCameraExposure);
                const float EV=Index==0?10.f:2.f;
                TestTrue(TEXT("Physical camera EV"),S.bOverride_CameraISO && S.bOverride_CameraShutterSpeed && S.bOverride_DepthOfFieldFstop && FMath::IsNearlyEqual(FMath::Log2(FMath::Square(S.DepthOfFieldFstop)*S.CameraShutterSpeed*100/S.CameraISO),EV));
                TestTrue(TEXT("Fixed bounds"),S.bOverride_AutoExposureMinBrightness && S.bOverride_AutoExposureMaxBrightness && S.AutoExposureMinBrightness==EV && S.AutoExposureMaxBrightness==EV);
                TestTrue(TEXT("Zero bias"),S.bOverride_AutoExposureBias && S.AutoExposureBias==0);
            }
        }
        TestEqual(TEXT("Local light inventory"),Local,ExpectedLocal[Index]);
        TestEqual(TEXT("Directional inventory"),Sun,Index==0?1:0);
        TestEqual(TEXT("One sky"),Sky,1); TestEqual(TEXT("One exposure"),Exposure,1);
        ++Index;
    }
    TestEqual(TEXT("Five maps audited"),Index,5);
    return true;
}
