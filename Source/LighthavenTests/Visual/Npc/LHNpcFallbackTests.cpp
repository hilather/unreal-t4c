#include "Misc/AutomationTest.h"
#include "World/LHWorldMarkers.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHNpcFallbackOwnership, "Lighthaven.Visual.Npc.FallbackOwnership", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FLHNpcFallbackOwnership::RunTest(const FString&)
{
    UWorld* World=UWorld::CreateWorld(EWorldType::Game,false);
    auto* First=World->SpawnActor<ALHInteractableMarker>();
    auto* Second=World->SpawnActor<ALHInteractableMarker>();
    First->DefinitionId.Value=TEXT("NPC.Nevanis"); Second->DefinitionId.Value=TEXT("NPC.Shovanis");
    First->SetActorLocation(FVector(100,200,0));
    auto* Figure=World->SpawnActor<AStaticMeshActor>();
    auto* Other=World->SpawnActor<AStaticMeshActor>();
    Figure->SetActorLocation(FVector(100,200,90)); Figure->SetActorScale3D(FVector(.7,.7,1.8));
    const FTransform Before=Figure->GetActorTransform();
    const auto Collision=Figure->GetStaticMeshComponent()->GetCollisionEnabled();
    const FGuid Instance=First->InstanceId;
    TestTrue(TEXT("Explicit ownership registered"),First->RegisterNpcFallback(Figure));
    TestTrue(TEXT("Other NPC registered"),Second->RegisterNpcFallback(Other));
    TestFalse(TEXT("Cannot steal another NPC's figure"),Second->RegisterNpcFallback(Figure));
    TestTrue(TEXT("Registration preserves world transform"),Before.Equals(Figure->GetActorTransform()));
    TestTrue(TEXT("Persistent NPC identity tag"),Figure->ActorHasTag(First->DefinitionId.Value));
    First->SetNpcFallbackVisible(false);
    TestTrue(TEXT("Only own figure hidden"),Figure->IsHidden());
    TestFalse(TEXT("Other figure remains visible"),Other->IsHidden());
    TestEqual(TEXT("Visibility preserves collision"),Figure->GetStaticMeshComponent()->GetCollisionEnabled(),Collision);
    TestTrue(TEXT("Visibility preserves transform"),Before.Equals(Figure->GetActorTransform()));
    TestEqual(TEXT("Visibility preserves gameplay GUID"),First->InstanceId,Instance);
    First->SetNpcFallbackVisible(true);
    TestFalse(TEXT("Missing asset fallback can be restored"),Figure->IsHidden());
    // A stale/malformed serialized reference must never hide somebody else's actor.
    First->NpcFallbackFigures.Add(Other); First->SetNpcFallbackVisible(false);
    TestFalse(TEXT("Wrong-owner serialized reference ignored"),Other->IsHidden());
    World->DestroyWorld(false);
    return true;
}
#endif
