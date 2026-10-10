#include "Visual/Monsters/LHMonsterVisual.h"
#if !UE_BUILD_SHIPPING
#include "HAL/IConsoleManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
namespace LHCreatureLineupPrivate
{
void Show(const TArray<FString>& Args,UWorld* World)
{
    if(!World || !World->IsGameWorld()) return;
    APlayerController* PC=World->GetFirstPlayerController();
    if(!PC || !PC->GetPawn()) return;
    const TCHAR* Ids[]={TEXT("BrownRat"),TEXT("Bat"),TEXT("GreenSlime"),TEXT("Goblin"),TEXT("GiantSpider"),TEXT("Balork"),TEXT("GoblinWarrior"),TEXT("Atrocity"),TEXT("DungeonBat"),TEXT("GiantBat"),TEXT("UndeadBat")};
    // Prototype review spacing only. These actors have no encounter, AI, collision or rewards.
    const FVector Origin=PC->GetPawn()->GetActorLocation()-FVector(0,0,90);
    const int32 Focus=Args.Num()?FMath::Clamp(FCString::Atoi(*Args[0]),0,10):-1;
    for(int32 I=0;I<11;++I)
    {
        if(Focus>=0 && I!=Focus) continue;
        AActor* Actor=World->SpawnActor<AActor>();
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
    auto* Camera=World->SpawnActor<ACameraActor>();
    const bool Close=Args.Num()>1 && Args[1]==TEXT("close");
    Camera->SetActorLocation(Origin+FVector(Close?350:850,Close?-450:-850,Close?300:1100));
    const FVector Target=Origin+FVector(0,0,Focus==5?160:90);
    Camera->SetActorRotation((Target-Camera->GetActorLocation()).Rotation());
    Camera->GetCameraComponent()->SetFieldOfView(Close?40:55);
    Camera->SetLifeSpan(60);
    PC->SetViewTarget(Camera);
    UE_LOG(LogTemp,Display,TEXT("LH_CREATURE_LINEUP focus=%d close=%d"),Focus,Close);
}
FAutoConsoleCommandWithWorldAndArgs Command(TEXT("lh.CreatureLineup"),
    TEXT("Presentation-only review actors at current B1 room. Optional roster index 0..10 and close."),
    FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Show));
}
#endif
