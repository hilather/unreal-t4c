#include "Visual/Monsters/LHMonsterVisual.h"
#if !UE_BUILD_SHIPPING
#include "HAL/IConsoleManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "EngineUtils.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "Visual/LHB1Lighting.h"
#include "Visual/LHVisualKit.h"
namespace LHCreatureLineupPrivate
{
// Presentation only; centimetres. Entry room is x=0..1800, y=-2800..-1000.
// Search inside its 3m inset, away from every dressed fixture/prop bounds.
bool ReviewSpot(UWorld* World,FVector& Spot)
{
    double Best=TNumericLimits<double>::Max();
    for(int X=350;X<=1450;X+=50) for(int Y=-2450;Y<=-1350;Y+=50)
    {
        FVector P(X,Y,0); bool Clear=true; double TorchDistance=Best;
        for(TActorIterator<ALHVisualPiece> It(World);It;++It)
        {
            const FString Id=It->GetRecipe().Id.ToString();
            if(Id.EndsWith(TEXT(".Torch")) || Id.EndsWith(TEXT(".Sconce")) ||
               Id.EndsWith(TEXT(".Barrel")) || Id.EndsWith(TEXT(".Crate")) ||
               Id.EndsWith(TEXT(".Table")) || Id.EndsWith(TEXT(".Bench")) || Id.EndsWith(TEXT(".Debris")))
            {
                const FBox B=It->GetComponentsBoundingBox(true);
                const double DX=FMath::Max(FMath::Max(B.Min.X-P.X,P.X-B.Max.X),0.);
                const double DY=FMath::Max(FMath::Max(B.Min.Y-P.Y,P.Y-B.Max.Y),0.);
                if(DX*DX+DY*DY<300.*300.) Clear=false;
            }
            if(auto* Light=It->FindComponentByClass<ULHB1TorchLightComponent>())
                TorchDistance=FMath::Min(TorchDistance,FVector::DistSquared2D(P,Light->GetComponentLocation()));
        }
        // Test the 3m horizontal clearance above the floor, including stair walls.
        FCollisionQueryParams Clearance(SCENE_QUERY_STAT(LHLineupClearance),false);
        Clear &= !World->OverlapBlockingTestByChannel(P+FVector(0,0,150),FQuat::Identity,
            ECC_Visibility,FCollisionShape::MakeBox(FVector(300,300,80)),Clearance);
        FHitResult Floor;
        if(Clear && TorchDistance<375.*375. && TorchDistance<Best &&
           World->LineTraceSingleByChannel(Floor,P+FVector(0,0,200),P-FVector(0,0,50),ECC_Visibility) &&
           Floor.ImpactNormal.Z>.9 && FMath::Abs(Floor.ImpactPoint.Z)<5)
        { Best=TorchDistance; Spot=Floor.ImpactPoint; }
    }
    return Best<TNumericLimits<double>::Max();
}
void Show(const TArray<FString>& Args,UWorld* World)
{
    if(!World || !World->IsGameWorld()) return;
    APlayerController* PC=World->GetFirstPlayerController();
    if(!PC || !PC->GetPawn()) return;
    const TCHAR* Ids[]={TEXT("BrownRat"),TEXT("Bat"),TEXT("GreenSlime"),TEXT("Goblin"),TEXT("GiantSpider"),TEXT("Balork"),TEXT("GoblinWarrior"),TEXT("Atrocity"),TEXT("DungeonBat"),TEXT("GiantBat"),TEXT("UndeadBat")};
    // Prototype review spacing only. These actors have no encounter, AI, collision or rewards.
    FVector Origin;
    if(!ReviewSpot(World,Origin)) { UE_LOG(LogTemp,Error,TEXT("LH_CREATURE_LINEUP no clear torch-pool spot")); return; }
    APawn* Pawn=PC->GetPawn();
    const bool WasHidden=Pawn->IsHidden();
    TWeakObjectPtr<APawn> WeakPawn=Pawn;
    TWeakObjectPtr<AActor> OldView=PC->GetViewTarget();
    TWeakObjectPtr<APlayerController> WeakPC=PC;
    // The player kit consists of attached actors; actor hiding does not recurse.
    auto HiddenAttachments=MakeShared<TArray<TPair<TWeakObjectPtr<AActor>,bool>>>();
    auto HideAttachments=[WeakPawn,HiddenAttachments](){
        if(!WeakPawn.IsValid()) return;
        TArray<AActor*> Attached; WeakPawn->GetAttachedActors(Attached,true,true);
        for(auto* Actor:Attached) {
            if(!HiddenAttachments->ContainsByPredicate([Actor](const auto& State){return State.Key.Get()==Actor;}))
                HiddenAttachments->Emplace(Actor,Actor->IsHidden());
            Actor->SetActorHiddenInGame(true);
        }
    };
    HideAttachments();
    // The character builds its kit on its first tick, after ExecCmds can run.
    FTimerHandle HideTimer;
    World->GetTimerManager().SetTimer(HideTimer,HideAttachments,.05f,true);
    Pawn->SetActorHiddenInGame(true);
    TArray<TWeakObjectPtr<AActor>> ReviewActors;
    AActor* Subject=nullptr;
    const int32 Focus=Args.Num()?FMath::Clamp(FCString::Atoi(*Args[0]),0,10):0;
    for(int32 I=0;I<11;++I)
    {
        if(Focus>=0 && I!=Focus) continue;
        AActor* Actor=World->SpawnActor<AActor>();
        ReviewActors.Add(Actor); Subject=Actor;
        auto* Root=NewObject<USceneComponent>(Actor);
        Actor->AddInstanceComponent(Root); Actor->SetRootComponent(Root); Root->RegisterComponent();
        auto* Visual=NewObject<ULHMonsterVisual>(Actor);
        Actor->AddInstanceComponent(Visual); Visual->SetupAttachment(Root);
        Visual->RegisterComponent();
        Actor->SetActorLocation(Origin+FVector(Focus>=0?0:(I%4)*350,Focus>=0?0:(I/4)*500,100));
        Visual->Build(FName(*(FString(TEXT("Enemy."))+Ids[I])),30,100);
        Actor->SetLifeSpan(60);
        TWeakObjectPtr<ULHMonsterVisual> Weak=Visual;
        FTimerHandle Timer;
        World->GetTimerManager().SetTimer(Timer,[Weak,World](){
            if(!Weak.IsValid()) return;
            Weak->Attack(.8);
            FTimerHandle End;
            World->GetTimerManager().SetTimer(End,[Weak](){ if(Weak.IsValid()) Weak->CancelAttack(); },1.25,false);
        },3,true,2);
    }
    if(!Subject) return;
    auto* Camera=World->SpawnActor<ACameraActor>();
    ReviewActors.Add(Camera);
    const bool Close=Args.Num()>1 && Args[1]==TEXT("close");
    // Include weapon and wing components, with margin for idle animation.
    const FBox Bounds=Subject->GetComponentsBoundingBox(true);
    const FVector Target=Bounds.GetCenter();
    const FRotator Rotation=Close?FRotator(-12,225,0):FRotator(-55,45,0);
    // A wider close lens keeps the largest bounds inside the open entry room;
    // the camera approaches from the room interior rather than the south wall.
    const float FOV=Close?75.f:45.f;
    double Distance=1200;
    if(Close)
    {
        const FVector Extent=Bounds.GetExtent();
        const FVector Forward=Rotation.Vector();
        const FVector Right=FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);
        const FVector Up=FRotationMatrix(Rotation).GetUnitAxis(EAxis::Z);
        const double Vertical=FVector::DotProduct(Extent,Up.GetAbs());
        const double Horizontal=FVector::DotProduct(Extent,Right.GetAbs());
        const double Depth=FVector::DotProduct(Extent,Forward.GetAbs());
        const double TanH=FMath::Tan(FMath::DegreesToRadians(FOV*.5));
        Distance=Depth+FMath::Max(Vertical/(TanH/(16./9.)*.70),Horizontal/(TanH*.85));
        const FVector Offsets[]={{250,250,300},{250,-250,180},{-300,-100,250}};
        const float Intensities[]={3500,2200,3000};
        for(int32 I=0;I<3;++I)
        {
            auto* Light=World->SpawnActor<APointLight>(); ReviewActors.Add(Light);
            auto* C=Light->PointLightComponent.Get(); C->SetMobility(EComponentMobility::Movable);
            Light->SetActorLocation(Target+Offsets[I]); Light->SetLifeSpan(60);
            C->SetIntensityUnits(ELightUnits::Lumens); C->SetIntensity(Intensities[I]);
            C->SetUseTemperature(true); C->SetTemperature(I==1?5500:4500);
            C->SetAttenuationRadius(2000); C->SetCastShadows(false); C->SetSourceRadius(80);
        }
    }
    Camera->SetActorLocation(Target-Rotation.Vector()*Distance);
    Camera->SetActorRotation(Rotation);
    Camera->GetCameraComponent()->SetFieldOfView(FOV);
    Camera->GetCameraComponent()->SetAspectRatio(16.f/9.f);
    Camera->GetCameraComponent()->SetConstraintAspectRatio(true);
    Camera->SetLifeSpan(60);
    // Use the pawn's actual gameplay camera/boom, preserving its collision probe.
    TWeakObjectPtr<USpringArmComponent> GameplayBoom;
    TWeakObjectPtr<UCameraComponent> GameplayCamera;
    FTransform OldBoomTransform; float OldArm=0,OldFOV=0;
    if(!Close) {
        auto* Boom=Pawn->FindComponentByClass<USpringArmComponent>();
        auto* View=Pawn->FindComponentByClass<UCameraComponent>();
        if(Boom && View) {
            GameplayBoom=Boom; GameplayCamera=View;
            OldBoomTransform=Boom->GetRelativeTransform(); OldArm=Boom->TargetArmLength; OldFOV=View->FieldOfView;
            Boom->SetWorldLocation(Target); Boom->SetWorldRotation(Rotation); Boom->TargetArmLength=1200;
            View->SetFieldOfView(45); PC->SetViewTarget(Pawn);
        } else PC->SetViewTarget(Camera); // Bare-pawn development fixtures.
    } else PC->SetViewTarget(Camera);
    FTimerHandle Cleanup;
    World->GetTimerManager().SetTimer(Cleanup,[WeakPawn,WasHidden,WeakPC,OldView,ReviewActors,HiddenAttachments,HideTimer,World,GameplayBoom,GameplayCamera,OldBoomTransform,OldArm,OldFOV]() mutable {
        if(WeakPawn.IsValid()) WeakPawn->SetActorHiddenInGame(WasHidden);
        World->GetTimerManager().ClearTimer(HideTimer);
        for(const auto& State:*HiddenAttachments) if(State.Key.IsValid()) State.Key->SetActorHiddenInGame(State.Value);
        if(GameplayBoom.IsValid()) { GameplayBoom->SetRelativeTransform(OldBoomTransform); GameplayBoom->TargetArmLength=OldArm; }
        if(GameplayCamera.IsValid()) GameplayCamera->SetFieldOfView(OldFOV);
        if(WeakPC.IsValid() && OldView.IsValid()) WeakPC->SetViewTarget(OldView.Get());
        for(const auto& Actor:ReviewActors) if(Actor.IsValid()) Actor->Destroy();
    },59,false);
    UE_LOG(LogTemp,Display,TEXT("LH_CREATURE_LINEUP focus=%d close=%d spot=%s pawnHidden=%d rigLights=%d bounds=%s"),
        Focus,Close,*Origin.ToString(),Pawn->IsHidden(),Close?3:0,*Bounds.ToString());
}
FAutoConsoleCommandWithWorldAndArgs Command(TEXT("lh.CreatureLineup"),
    TEXT("Presentation-only review actors at current B1 room. Optional roster index 0..10 and close."),
    FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Show));
}
#endif

#if !UE_BUILD_SHIPPING && WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "UObject/Package.h"
#include "Engine/StaticMesh.h"
#include "Framework/LHCharacter.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
// Advance the synthetic world's timers on distinct automation frames: timers
// created before their first tick are pending until that tick activates them.
class FLineupCleanupCheck : public IAutomationLatentCommand
{
    FAutomationTestBase* Test; UWorld* World; APawn* Pawn; int32 Stage=0;
public:
    FLineupCleanupCheck(FAutomationTestBase* InTest,UWorld* InWorld,APawn* InPawn)
        :Test(InTest),World(InWorld),Pawn(InPawn) {}
    bool Update() override
    {
        ++Stage;
        if(Stage==1) {
            World->GetTimerManager().Tick(0);
            auto* Late=World->SpawnActor<AActor>();
            auto* Root=NewObject<USceneComponent>(Late); Late->AddInstanceComponent(Root); Late->SetRootComponent(Root); Root->RegisterComponent();
            Late->AttachToActor(Pawn,FAttachmentTransformRules::KeepRelativeTransform);
            return false;
        }
        if(Stage==2) {
            World->GetTimerManager().Tick(.1f);
            TArray<AActor*> Attached; Pawn->GetAttachedActors(Attached,true,true);
            for(auto* Actor:Attached) Test->TestTrue(TEXT("First-tick player attachments hidden"),Actor->IsHidden());
            return false;
        }
        World->GetTimerManager().Tick(59.5f);
        Test->TestFalse(TEXT("Pawn restored"),Pawn->IsHidden());
        if(auto* Boom=Pawn->FindComponentByClass<USpringArmComponent>())
            Test->TestTrue(TEXT("Gameplay boom pivot restored"),Boom->GetRelativeLocation().IsNearlyZero());
        TArray<AActor*> Attached; Pawn->GetAttachedActors(Attached,true,true);
        for(auto* Actor:Attached) Test->TestFalse(TEXT("Player attachment restored"),Actor->IsHidden());
        if(Stage==3)
        {
            LHCreatureLineupPrivate::Show({TEXT("5"),TEXT("close")},World);
            int32 Subjects=0,Lights=0;
            FVector Spot; LHCreatureLineupPrivate::ReviewSpot(World,Spot);
            for(TActorIterator<AActor> It(World);It;++It)
            {
                if(It->FindComponentByClass<ULHMonsterVisual>()) {
                    ++Subjects; Test->TestTrue(TEXT("Close subject on chosen spot"),It->GetActorLocation().Equals(Spot+FVector(0,0,100),.01));
                }
                if(auto* Light=Cast<APointLight>(*It)) {
                    ++Lights;
                    const auto* C=Light->PointLightComponent.Get();
                    Test->TestTrue(TEXT("Rig component movable and registered"),C->GetMobility()==EComponentMobility::Movable && C->IsRegistered());
                    Test->TestTrue(TEXT("Rig lights placed near subject"),FVector::DistSquared2D(C->GetComponentLocation(),Spot)<600.*600.);
                }
            }
            Test->TestTrue(TEXT("Close pawn hidden"),Pawn->IsHidden());
            for(auto* Actor:Attached) Test->TestTrue(TEXT("Close player attachment hidden"),Actor->IsHidden());
            Test->TestEqual(TEXT("Close-only rig lights"),Lights,3);
            Test->TestEqual(TEXT("Previous subject removed"),Subjects,1);
            return false;
        }
        int32 Lights=0;
        for(TActorIterator<APointLight> It(World);It;++It) ++Lights;
        Test->TestEqual(TEXT("Rig removed on cleanup"),Lights,0);
        World->DestroyWorld(false); GEngine->DestroyWorldContext(World); return true;
    }
};
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHCreatureLineupTest,"Lighthaven.Visual.CreatureLineup.NullRHI",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHCreatureLineupTest::RunTest(const FString&)
{
    auto Init=UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(true)
        .CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false);
    auto* W=UWorld::CreateWorld(EWorldType::Game,false,MakeUniqueObjectName(GetTransientPackage(),UWorld::StaticClass(),TEXT("LineupTest")),
        GetTransientPackage(),true,ERHIFeatureLevel::Num,&Init);
    GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(W);
    W->InitializeActorsForPlay(FURL());
    auto* Floor=W->SpawnActor<AActor>();
    auto* Mesh=NewObject<UStaticMeshComponent>(Floor); Floor->AddInstanceComponent(Mesh); Floor->SetRootComponent(Mesh);
    Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));
    Mesh->SetCollisionProfileName(TEXT("BlockAll")); Mesh->RegisterComponent();
    Floor->SetActorLocation(FVector(900,-1900,-10)); Floor->SetActorScale3D(FVector(18,18,.2));
    auto* Fixture=W->SpawnActor<ALHVisualPiece>();
    FLHVisualRecipe Recipe; Recipe.Id=TEXT("Presentation.Environment.Shared.Torch"); Recipe.Style=ELHVisualStyle::B1Cellar;
    FLHVisualBox Box; Box.Size=FVector(20,20,100); Recipe.Geometry.Add(Box);
    Fixture->Build(Recipe); Fixture->SetActorLocation(FVector(900,-1900,50));
    ULHB1TorchLightComponent::Configure(Fixture);
    auto* PC=W->SpawnActor<APlayerController>(); auto* Pawn=W->SpawnActor<ALHCharacter>(); PC->Possess(Pawn);
    auto* Child=W->SpawnActor<AActor>();
    auto* ChildRoot=NewObject<USceneComponent>(Child); Child->AddInstanceComponent(ChildRoot); Child->SetRootComponent(ChildRoot); ChildRoot->RegisterComponent();
    Child->AttachToActor(Pawn,FAttachmentTransformRules::KeepRelativeTransform);
    FVector Spot; TestTrue(TEXT("Clear floor spot inside torch pool"),LHCreatureLineupPrivate::ReviewSpot(W,Spot));
    for(int32 Index:{5}) for(bool Close:{false})
    {
        const TArray<FString> Args={FString::FromInt(Index),Close?TEXT("close"):TEXT("gameplay")};
        LHCreatureLineupPrivate::Show(Args,W);
        TestTrue(TEXT("Pawn hidden"),Pawn->IsHidden());
        TestTrue(TEXT("Player attachment hidden"),Child->IsHidden());
        int32 Subjects=0,Lights=0;
        for(TActorIterator<AActor> It(W);It;++It)
        {
            if(It->FindComponentByClass<ULHMonsterVisual>())
            { ++Subjects; TestTrue(TEXT("Subject on chosen floor"),It->GetActorLocation().Equals(Spot+FVector(0,0,100),.01)); }
            if(It->IsA<APointLight>()) ++Lights;
        }
        TestEqual(TEXT("One subject"),Subjects,1); TestEqual(TEXT("Close-only rig"),Lights,Close?3:0);
        TestTrue(TEXT("Actual gameplay camera view target"),PC->GetViewTarget()==Pawn);
        auto* Boom=Pawn->FindComponentByClass<USpringArmComponent>();
        auto* Camera=Pawn->FindComponentByClass<UCameraComponent>();
        TestNotNull(TEXT("Gameplay boom"),Boom); TestNotNull(TEXT("Gameplay camera"),Camera);
        if(Boom && Camera) {
            TestTrue(TEXT("Gameplay pitch/yaw"),Boom->GetComponentRotation().Equals(FRotator(-55,45,0),.01));
            TestEqual(TEXT("Gameplay arm"),Boom->TargetArmLength,1200.f);
            TestEqual(TEXT("Gameplay FOV"),Camera->FieldOfView,45.f);
        }

    }
    AddCommand(new FLineupCleanupCheck(this,W,Pawn)); return true;
}
#endif
