#include "Misc/AutomationTest.h"
#include "Visual/Npc/LHNpcVisual.h"
#include "World/LHWorldMarkers.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Visual/Npc/LHNpcRoster.inl"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHNpcAssets,"Lighthaven.Visual.Npc.ImportedRoster",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHNpcAssets::RunTest(const FString&)
{
    UWorld* World=UWorld::CreateWorld(EWorldType::Game,false);
    int32 Count=0;
    for(const TCHAR* Name:LHNpcVisualPrivate::Names)
    {
        const FString Path=FString(TEXT("/Game/Lighthaven/Art/Npc/DA_"))+Name;
        auto* Row=LoadObject<ULHNpcAppearance>(nullptr,*Path);
        if(!TestNotNull(*Path,Row)) continue;
        auto* Marker=World->SpawnActor<ALHInteractableMarker>();
        Marker->DefinitionId.Value=FName(FString(TEXT("NPC."))+Name);
        Marker->InstanceId=FGuid(1,2,3,++Count);
        Marker->SetActorTransform(FTransform(FRotator(0,37,0),FVector(50*Count,0,0)));
        auto* Fallback=World->SpawnActor<AActor>(); Marker->RegisterNpcFallback(Fallback);
        auto* Other=World->SpawnActor<AActor>();
        const FTransform Before=Marker->GetActorTransform();
        const auto Entity=Marker->Materialize(FGuid(5,6,7,8));
        Marker->NpcVisual->Refresh();
        TestTrue(TEXT("Imported complete character"),Marker->NpcVisual->UsesImportedCharacter());
        TestTrue(TEXT("Exactly own fallback hidden"),Fallback->IsHidden());
        TestFalse(TEXT("Unowned nearby actor visible"),Other->IsHidden());
        TestTrue(TEXT("Interaction pivot preserved"),Before.Equals(Marker->GetActorTransform()));
        TestEqual(TEXT("Interaction GUID preserved"),Entity.InstanceId,Marker->Materialize(FGuid(5,6,7,8)).InstanceId);
        TestTrue(TEXT("Visual up matches marker up"),Marker->NpcVisual->GetUpVector().Equals(FVector::UpVector,.001));
        TestTrue(TEXT("Visual faces marker yaw"),Marker->NpcVisual->GetForwardVector().Equals(Marker->GetActorForwardVector(),.001));
        TestTrue(TEXT("No capsule offset"),Marker->NpcVisual->GetComponentLocation().Equals(Marker->GetActorLocation(),.001));
        TestTrue(TEXT("Stature applied once"),Marker->NpcVisual->GetRelativeScale3D().Equals(FVector(Row->Stature),.001));
        FVector Min(FLT_MAX),Max(-FLT_MAX);
        for(USkeletalMesh* Mesh:Row->Meshes) if(Mesh)
        {
            const FBoxSphereBounds Bounds=Mesh->GetBounds();
            Min=Min.ComponentMin(Bounds.Origin-Bounds.BoxExtent); Max=Max.ComponentMax(Bounds.Origin+Bounds.BoxExtent);
            TestTrue(TEXT("Idle skeleton shared"),Row->Idle && Mesh->GetSkeleton()==Row->Idle->GetSkeleton());
            TestTrue(TEXT("Gesture skeleton shared"),Row->Gesture && Mesh->GetSkeleton()==Row->Gesture->GetSkeleton());
        }
        TestTrue(TEXT("Assembly minimum matches real source within 0.5 cm"),(Min*Row->Stature).Equals(Row->SourceMin,.5));
        TestTrue(TEXT("Assembly maximum matches real source within 0.5 cm"),(Max*Row->Stature).Equals(Row->SourceMax,.5));
        TestTrue(TEXT("Upright tall axis"),Max.Z-Min.Z>Max.X-Min.X && Max.Z-Min.Z>Max.Y-Min.Y);
        TArray<USkeletalMeshComponent*> Parts; Marker->GetComponents(Parts);
        TestEqual(TEXT("Exactly selected modules"),Parts.Num(),Row->Meshes.Num());
        for(auto* Part:Parts) TestEqual(TEXT("Visual cannot block interaction"),Part->GetCollisionEnabled(),ECollisionEnabled::NoCollision);
    }
    TestEqual(TEXT("All fifteen identities resolved"),Count,15);
    World->DestroyWorld(false); return true;
}
#endif
#if WITH_DEV_AUTOMATION_TESTS
#include "EngineUtils.h"
#include "Framework/LHInteractionReach.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHNpcPlaced,"Lighthaven.Visual.Npc.PlacedMarkers",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHNpcPlaced::RunTest(const FString&)
{
    TSet<FName> Seen;
    for(const TCHAR* Map:{TEXT("/Game/Lighthaven/Maps/L_LighthavenTempleDistrict"),TEXT("/Game/Lighthaven/Maps/L_TempleB1")})
    {
        const FName PackageName(Map);
        UWorld::WorldTypePreLoadMap.Add(PackageName,EWorldType::Editor);
        UPackage* Package=LoadPackage(nullptr,Map,LOAD_None);
        UWorld::WorldTypePreLoadMap.Remove(PackageName);
        UWorld* World=Package?UWorld::FindWorldInPackage(Package):nullptr;
        if(!TestNotNull(Map,World)) continue;
        const bool CreatedContext=GEngine->GetWorldContextFromWorld(World)==nullptr;
        if(CreatedContext) { auto& Context=GEngine->CreateNewWorldContext(EWorldType::Editor); Context.SetCurrentWorld(World); }
        World->WorldType=EWorldType::Editor;
        if(!World->IsInitialized()) World->InitWorld(UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(true).CreateNavigation(false).CreateAISystem(false));
        World->UpdateWorldComponents(true,false);
        int32 Count=0;
        for(TActorIterator<ALHInteractableMarker> It(World);It;++It)
        {
            auto* Marker=*It;
            if(!Marker->DefinitionId.Value.ToString().StartsWith(TEXT("NPC."))) continue;
            ++Count; TestFalse(TEXT("NPC placed only once"),Seen.Contains(Marker->DefinitionId.Value)); Seen.Add(Marker->DefinitionId.Value);
            const FTransform Before=Marker->GetActorTransform();
            const FGuid Run(1,2,3,4); const auto Entity=Marker->Materialize(Run);
            TArray<bool> ReachBefore;
            const FVector Origin=Marker->GetActorLocation();
            const FVector Samples[]={Origin+FVector(249,0,90),Origin+FVector(251,0,90),Origin+FVector(0,0,151)};
            for(const auto& Sample:Samples) ReachBefore.Add(LHInteractionReach::Contains(Sample,Origin));
            Marker->NpcVisual->Refresh();
            TestTrue(TEXT("Placed NPC imported"),Marker->NpcVisual->UsesImportedCharacter());
            TestTrue(TEXT("Transform unchanged"),Before.Equals(Marker->GetActorTransform()));
            TestEqual(TEXT("Target entity unchanged"),Entity.InstanceId,Marker->Materialize(Run).InstanceId);
            for(int32 I=0;I<UE_ARRAY_COUNT(Samples);++I) TestEqual(TEXT("Interaction boundary identical"),ReachBefore[I],LHInteractionReach::Contains(Samples[I],Marker->GetActorLocation()));
            TestTrue(TEXT("Placed visual upright"),Marker->NpcVisual->GetUpVector().Equals(FVector::UpVector,.001));
            TestTrue(TEXT("Placed visual facing"),Marker->NpcVisual->GetForwardVector().Equals(Marker->GetActorForwardVector(),.001));
            TestTrue(TEXT("Explicit fallback registered"),!Marker->NpcFallbackFigures.IsEmpty());
            for(AActor* Figure:Marker->NpcFallbackFigures)
            {
                TestTrue(TEXT("Fallback owned by this marker"),Figure && Figure->GetOwner()==Marker);
                if(Figure) TestTrue(TEXT("Owned fallback hidden"),Figure->IsHidden());
            }
        }
        TestEqual(TEXT("Map NPC count"),Count,FString(Map).Contains(TEXT("TempleB1"))?2:13);
        if(CreatedContext) GEngine->DestroyWorldContext(World);
    }
    TestEqual(TEXT("All placed identities"),Seen.Num(),15); return true;
}
#endif
