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
#include "AI/NavigationSystemBase.h"
#include "Components/PrimitiveComponent.h"
#if WITH_EDITOR
#include "StaticMeshCompiler.h"
#include "AssetCompilingManager.h"
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
        // Select Editor before PostLoad/OnAssetLoaded: inactive-world initialization
        // omits physics and Editor-only world subsystems (navigation repository).
        // Changing WorldType after that initialization cannot create those subsystems.
        const FName PackageName(*Area.Map.GetLongPackageName());
        UWorld::WorldTypePreLoadMap.Add(PackageName,EWorldType::Editor);
        UPackage* Package=LoadPackage(nullptr,*Area.Map.GetLongPackageName(),LOAD_None);
        UWorld::WorldTypePreLoadMap.Remove(PackageName);
        UWorld* World=Package ? UWorld::FindWorldInPackage(Package) : nullptr;
        bool PhysicsControl=false,FloorControl=false,NavDataControl=false,TilesControl=false;
        if (World)
        {
            auto& Context=GEngine->CreateNewWorldContext(EWorldType::Editor); Context.SetCurrentWorld(World);
            World->WorldType=EWorldType::Editor;
            if (!World->IsInitialized()) World->InitWorld(UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(true).CreateNavigation(true).CreateAISystem(true));
            // Package loading can leave an initialized editor world without a
            // physics scene. InitWorld is single-use, so repair the missing scene
            // before registration rather than reinitializing the loaded world.
            const bool HadPhysicsScene=World->GetPhysicsScene()!=nullptr;
            if (!HadPhysicsScene) World->CreatePhysicsScene();
            World->UpdateWorldComponents(true,false);
            AddInfo(FString::Printf(TEXT("ARRIVAL_SETUP %s initialized=%d scene-before=%d scene-after=%d levels=%d"),
                *Area.Map.ToString(),World->IsInitialized(),HadPhysicsScene,World->GetPhysicsScene()!=nullptr,World->GetNumLevels()));
#if WITH_EDITOR
            // Registration can start additional compilation. Finish after registration,
            // then recreate bodies that registration skipped while the mesh was compiling.
            FStaticMeshCompilingManager::Get().FinishAllCompilation();
#endif
            for (TActorIterator<AActor> It(World);It;++It)
            {
                TInlineComponentArray<UPrimitiveComponent*> Components(*It);
                for (auto* Component:Components)
                {
                    Component->UpdateComponentToWorld();
                    Component->RecreatePhysicsState();
                    if (Component->IsRegistered() && Component->IsQueryCollisionEnabled() &&
                        Component->GetCollisionResponseToChannel(ECC_Pawn)==ECR_Block && Component->IsPhysicsStateCreated())
                        PhysicsControl=true;
                }
            }
            PhysicsControl=PhysicsControl && World->GetPhysicsScene()!=nullptr;
            // Independent authored flat-floor probes (cm), not registry arrival positions:
            // hub nave, B1/B2 route controls, B3 entry room, B4 entry room. Floor tops Z=0.
            const FVector Probes[]={FVector(800,1200,0),FVector(900,900,0),FVector(1100,1000,0),FVector(3500,500,0),FVector(600,600,0)};
            const int32 AreaIndex=&Area-LHWorld::Registry().GetData();
            FHitResult ControlHit;
            FloorControl=AreaIndex>=0 && AreaIndex<UE_ARRAY_COUNT(Probes) && World->LineTraceSingleByChannel(ControlHit,Probes[AreaIndex]+FVector(0,0,10),Probes[AreaIndex]-FVector(0,0,15),ECC_Pawn) &&
                FMath::Abs(ControlHit.ImpactPoint.Z)<=5 && ControlHit.ImpactNormal.Z>=0.7;
            // InitWorld creates the system without initializing it for this loaded world.
            // Initialize bounds/octree after component repair, then use the native Build
            // delegate. UE 5.8.3 Build calls EnsureBuildCompletion on every nav data set.
#if WITH_EDITOR
            // This synchronous automation command cannot service the editor's
            // 16-frame/two-second delayed async-load unlock. Finish all assets,
            // then disable that automatic-build delay on this test world only.
            // Explicit Build still performs the real Recast build to completion.
            FAssetCompilingManager::Get().FinishAllCompilation();
            auto* LoadedNavSystem=World->GetNavigationSystem();
            auto* WaitProperty=LoadedNavSystem ? FindFProperty<FBoolProperty>(LoadedNavSystem->GetClass(),TEXT("bWaitForAsyncLoadingBeforeBuildingNavigationAutomatically")) : nullptr;
            if (WaitProperty) WaitProperty->SetPropertyValue_InContainer(LoadedNavSystem,false);
            else AddError(TEXT("Missing navigation async-load wait property"));
#endif
            FNavigationSystem::AddNavigationSystemToWorld(*World,FNavigationSystemRunMode::EditorMode);
            FNavigationSystem::Build(*World);
            auto* NavSystem=World->GetNavigationSystem();
            NavDataControl=NavSystem && NavSystem->GetMainNavData()!=nullptr;
            // Recast GetBounds accumulates only Detour tiles with populated headers;
            // valid computed bounds prove populated tiles, unlike tile pool capacity.
            TilesControl=NavDataControl && NavSystem->ComputeNavDataBounds().IsValid;
            AddInfo(FString::Printf(TEXT("ARRIVAL_CONTROL %s physics=%d floor=%d navdata=%d tiles=%d"),*Area.Map.ToString(),PhysicsControl,FloorControl,NavDataControl,TilesControl));
        }
        const bool Controls=PhysicsControl && FloorControl && NavDataControl && TilesControl;
        if (!Controls) AddError(FString::Printf(TEXT("Arrival world controls failed: %s world=%d blocking-physics=%d known-floor=%d navdata=%d populated-tiles=%d"),*Area.Map.ToString(),World!=nullptr,PhysicsControl,FloorControl,NavDataControl,TilesControl));
        for (const auto& Entrance:Area.Entrances)
        {
            int32 Count=0; bool Transform=false,Overlap=false,Floor=false,Nav=false,FloorHit=false;
            double FloorDelta=0,FloorNormal=0;
            if (World)
            {
                for (TActorIterator<ALHEntranceMarker> It(World);It;++It) if (LHWorld::SameEntrance(It->EntranceId,Entrance.Id))
                { ++Count; Transform=It->GetActorTransform().Equals(Entrance.SafeTransform) && It->SafeArrivalTransform.Equals(Entrance.SafeTransform); }
                const FVector Ground=Entrance.SafeTransform.GetLocation(),Center=Ground+FVector(0,0,Height+2);
                Overlap=PhysicsControl && FloorControl && !World->OverlapBlockingTestByChannel(Center,Entrance.SafeTransform.GetRotation(),ECC_Pawn,FCollisionShape::MakeCapsule(Radius,Height));
                FHitResult Hit;
                FloorHit=World->LineTraceSingleByChannel(Hit,Ground+FVector(0,0,10),Ground-FVector(0,0,15),ECC_Pawn);
                FloorDelta=Hit.ImpactPoint.Z-Ground.Z; FloorNormal=Hit.ImpactNormal.Z;
                Floor=FloorHit && FMath::Abs(FloorDelta)<=5 && FloorNormal>=0.7;
                Nav=TilesControl && LHWave3ArrivalTestsPrivate::Project(World,Ground);
            }
            const bool Pass=Controls && Count==1 && Transform && Overlap && Floor && Nav;
            const FString Key=LHArrivalReview::Key(Entrance.Id),Hash=LHArrivalReview::Hash(Entrance.SafeTransform);
            FString Reason;
            if (!World) Reason+=TEXT("world-load;");
            if (!PhysicsControl) Reason+=TEXT("no-blocking-physics;");
            if (!FloorControl) Reason+=TEXT("known-floor-miss;");
            if (!NavDataControl) Reason+=TEXT("no-navdata;");
            if (!TilesControl) Reason+=TEXT("no-populated-tiles;");
            if (Count!=1) Reason+=TEXT("marker-count;");
            if (!Transform) Reason+=TEXT("transform-mismatch;");
            if (!Overlap) Reason+=(PhysicsControl && FloorControl)?TEXT("capsule-blocked;"):TEXT("capsule-control-failed;");
            if (!FloorHit) Reason+=TEXT("floor-trace-miss;");
            else
            {
                if (FMath::Abs(FloorDelta)>5) Reason+=TEXT("floor-height-error;");
                if (FloorNormal<0.7) Reason+=TEXT("floor-not-walkable;");
            }
            if (!Nav) Reason+=TEXT("nav-projection-failed;");
            if (Pass) Reason=TEXT("ok");
            AddInfo(FString::Printf(TEXT("ARRIVAL %s %s hash=%s count=%d transform=%d capsule=%d floor=%d nav=%d radius=%.1f halfheight=%.1f controls=%d physics=%d knownfloor=%d navdata=%d tiles=%d floorhit=%d floordelta=%.2f floornormal=%.2f reason=%s"),*Key,Pass?TEXT("PASS"):TEXT("FAIL"),*Hash,Count,Transform,Overlap,Floor,Nav,Radius,Height,Controls,PhysicsControl,FloorControl,NavDataControl,TilesControl,FloorHit,FloorDelta,FloorNormal,*Reason));
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
