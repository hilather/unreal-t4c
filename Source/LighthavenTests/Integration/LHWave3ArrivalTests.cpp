#include "Misc/AutomationTest.h"
#include "Framework/LHArrivalReview.h"
#include "Framework/LHCharacter.h"
#include "World/LHWorldMarkers.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Components/CapsuleComponent.h"
#include "Misc/PackageName.h"
#include "UObject/StructOnScope.h"
#include "UObject/UnrealType.h"
#include "HAL/PlatformProcess.h"
#if WITH_EDITOR
#include "StaticMeshCompiler.h"
#endif
#if WITH_DEV_AUTOMATION_TESTS
namespace LHWave3ArrivalTestsPrivate
{
// Invoke the engine's reflected navigation API without adding an out-of-contract
// module dependency. Missing API/nav data fails closed, never substitutes a floor trace.
bool Project(UWorld* World,const FVector& Point)
{
    UClass* Class=FindObject<UClass>(nullptr,TEXT("/Script/NavigationSystem.NavigationSystemV1"));
    UFunction* Function=Class ? Class->FindFunctionByName(TEXT("K2_ProjectPointToNavigation")) : nullptr;
    if (!Function) return false;
    FStructOnScope Params(Function); uint8* Data=Params.GetStructMemory();
    auto* Context=FindFProperty<FObjectPropertyBase>(Function,TEXT("WorldContextObject"));
    auto* Input=FindFProperty<FStructProperty>(Function,TEXT("Point"));
    auto* Extent=FindFProperty<FStructProperty>(Function,TEXT("QueryExtent"));
    auto* Output=FindFProperty<FStructProperty>(Function,TEXT("ProjectedLocation"));
    auto* Result=FindFProperty<FBoolProperty>(Function,TEXT("ReturnValue"));
    if (!Context || !Input || !Extent || !Output || !Result) return false;
    Context->SetObjectPropertyValue_InContainer(Data,World);
    *Input->ContainerPtrToValuePtr<FVector>(Data)=Point;
    *Extent->ContainerPtrToValuePtr<FVector>(Data)=FVector(25,25,50);
    Class->GetDefaultObject()->ProcessEvent(Function,Data);
    return Result->GetPropertyValue_InContainer(Data) &&
        FVector::Dist2D(*Output->ContainerPtrToValuePtr<FVector>(Data),Point)<=25;
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWave3ArrivalSafety,"Lighthaven.Integration.Wave3.ArrivalSafety",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHWave3ArrivalSafety::RunTest(const FString&)
{
    FString Rows;
    const auto* Capsule=GetDefault<ALHCharacter>()->GetCapsuleComponent();
    const float Radius=Capsule->GetUnscaledCapsuleRadius(),Height=Capsule->GetUnscaledCapsuleHalfHeight();
    for (const auto& Area:LHWorld::Registry())
    {
        UPackage* Package=LoadPackage(nullptr,*Area.Map.GetLongPackageName(),LOAD_None);
        UWorld* World=Package ? UWorld::FindWorldInPackage(Package) : nullptr;
        if (World)
        {
#if WITH_EDITOR
            // UE suppresses physics on meshes still compiling; finish before scene registration.
            FStaticMeshCompilingManager::Get().FinishAllCompilation();
#endif
            auto& Context=GEngine->CreateNewWorldContext(EWorldType::Game); Context.SetCurrentWorld(World);
            World->WorldType=EWorldType::Game;
            if (!World->IsInitialized()) World->InitWorld(UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(true).CreateNavigation(true).CreateAISystem(true));
            World->InitializeActorsForPlay(FURL());
            GEngine->Exec(World,TEXT("RebuildNavigation"));
            // Headless build completion: bounded tick/poll, no assumption of baked nav.
            for (int32 Tick=0;Tick<1000;++Tick)
            { World->Tick(LEVELTICK_All,0.01f); FPlatformProcess::Sleep(0.01f); }
        }
        for (const auto& Entrance:Area.Entrances)
        {
            int32 Count=0; bool Transform=false,Overlap=false,Floor=false,Nav=false;
            if (World)
            {
                for (TActorIterator<ALHEntranceMarker> It(World);It;++It) if (LHWorld::SameEntrance(It->EntranceId,Entrance.Id))
                { ++Count; Transform=It->GetActorTransform().Equals(Entrance.SafeTransform) && It->SafeArrivalTransform.Equals(Entrance.SafeTransform); }
                const FVector Ground=Entrance.SafeTransform.GetLocation(),Center=Ground+FVector(0,0,Height+2);
                Overlap=!World->OverlapBlockingTestByChannel(Center,Entrance.SafeTransform.GetRotation(),ECC_Pawn,FCollisionShape::MakeCapsule(Radius,Height));
                FHitResult Hit;
                Floor=World->LineTraceSingleByChannel(Hit,Ground+FVector(0,0,10),Ground-FVector(0,0,15),ECC_Visibility) &&
                    FMath::Abs(Hit.ImpactPoint.Z-Ground.Z)<=5 && Hit.ImpactNormal.Z>=0.7;
                Nav=LHWave3ArrivalTestsPrivate::Project(World,Ground);
            }
            const bool Pass=Count==1 && Transform && Overlap && Floor && Nav;
            const FString Key=LHArrivalReview::Key(Entrance.Id),Hash=LHArrivalReview::Hash(Entrance.SafeTransform);
            AddInfo(FString::Printf(TEXT("ARRIVAL %s %s hash=%s count=%d transform=%d capsule=%d floor=%d nav=%d radius=%.1f halfheight=%.1f"),*Key,Pass?TEXT("PASS"):TEXT("FAIL"),*Hash,Count,Transform,Overlap,Floor,Nav,Radius,Height));
            TestTrue(*Key,Pass);
            Rows+=Key+TEXT("\t")+Hash+TEXT("\t")+(Pass?TEXT("PASS"):TEXT("FAIL"))+TEXT("\n");
        }
        if (World) { GEngine->DestroyWorldContext(World); World->DestroyWorld(false); }
    }
    TestTrue(TEXT("Write per-ID machine evidence"),FFileHelper::SaveStringToFile(Rows,*(FPaths::ProjectSavedDir()/TEXT("ArrivalSafety.tsv"))));
    return !HasAnyErrors();
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWave3PackagedContent,"Lighthaven.Integration.Wave3.PackagedContent",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHWave3PackagedContent::RunTest(const FString&)
{
    for (const auto& Area:LHWorld::Registry()) TestTrue(*Area.Map.ToString(),FPackageName::DoesPackageExist(Area.Map.GetLongPackageName()));
    TestTrue(TEXT("Frontend package exists"),FPackageName::DoesPackageExist(TEXT("/Game/Lighthaven/Maps/L_Frontend")));
    AddInfo(FPlatformProperties::RequiresCookedData()?TEXT("Cooked runtime package lookup"):TEXT("Editor package lookup; packaged invocation remains required"));
    return !HasAnyErrors();
}
#endif
