#include "Misc/AutomationTest.h"
#if !UE_BUILD_SHIPPING
#include "Visual/Monsters/LHCreatureLineup.h"
#include "Visual/Monsters/LHMonsterVisual.h"
#include "Engine/World.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHCreatureSequenceTest,"Lighthaven.Visual.CreatureLineup.SequenceNullRHI",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHCreatureSequenceTest::RunTest(const FString&)
{
    auto* World=UWorld::CreateWorld(EWorldType::Game,false);
    const TCHAR* Ids[]={TEXT("BrownRat"),TEXT("Bat"),TEXT("GreenSlime"),TEXT("Goblin"),TEXT("GiantSpider"),TEXT("Balork"),TEXT("GoblinWarrior"),TEXT("Atrocity"),TEXT("DungeonBat"),TEXT("GiantBat"),TEXT("UndeadBat")};
    for(const TCHAR* Id:Ids) {
        auto* Actor=World->SpawnActor<AActor>();
        auto* Visual=NewObject<ULHMonsterVisual>(Actor);
        Actor->AddInstanceComponent(Visual); Actor->SetRootComponent(Visual); Visual->RegisterComponent();
        const FName Definition(*(FString(TEXT("Enemy."))+Id)); Visual->Build(Definition,30,100);
        TArray<USkeletalMeshComponent*> Meshes; Actor->GetComponents(Meshes);
        if(!TestTrue(TEXT("Imported lineup figure"),!Meshes.IsEmpty())) { Actor->Destroy(); continue; }
        for(const FString& Action:{FString(TEXT("attack")),FString(TEXT("idle"))}) {
            const int32 Count=Action==TEXT("attack")?8:4;
            for(int32 Frame=0;Frame<Count;++Frame) {
                const double T=double(Frame)/(Count-1);
                TestTrue(TEXT("Sample accepted"),LHCreatureLineupPrivate::Sample(Visual,Definition,Action,T));
                auto* Node=Meshes[0]->GetSingleNodeInstance();
                if(TestNotNull(TEXT("Single-node animation"),Node)) {
                    auto* Clip=Cast<UAnimSequence>(Node->GetCurrentAsset());
                    if(TestNotNull(TEXT("Requested clip"),Clip)) {
                        TestEqual(TEXT("Action selected"),Clip->GetName(),FString(TEXT("A_"))+Action);
                        TestTrue(TEXT("Absolute clip time"),FMath::IsNearlyEqual(double(Node->GetCurrentTime()),T*Clip->GetPlayLength(),.0001));
                    }
                    TestEqual(TEXT("Frozen play rate"),Node->GetPlayRate(),0.f);
                }
                TestFalse(TEXT("Visual tick disabled"),Visual->IsComponentTickEnabled());
                TestFalse(TEXT("Mesh tick disabled"),Meshes[0]->IsComponentTickEnabled());
            }
        }
        TestFalse(TEXT("Invalid time rejected"),LHCreatureLineupPrivate::Sample(Visual,Definition,TEXT("attack"),-1));
        Actor->Destroy();
    }
    World->DestroyWorld(false); return true;
}
#endif
