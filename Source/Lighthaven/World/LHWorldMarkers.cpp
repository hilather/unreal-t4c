#include "LHWorldMarkers.h"
#include "LHAreaRegistry.h"
#include "Components/SceneComponent.h"
ALHPortal::ALHPortal() { PrimaryActorTick.bCanEverTick=false; SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root"))); }
ALHEntranceMarker::ALHEntranceMarker() { PrimaryActorTick.bCanEverTick=false; SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root"))); }
ALHSpawnMarker::ALHSpawnMarker() { PrimaryActorTick.bCanEverTick=false; SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root"))); }
ALHInteractableMarker::ALHInteractableMarker() { PrimaryActorTick.bCanEverTick=false; SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root"))); }
FLHEntityId ALHPortal::Materialize(const FGuid& Run) const { FLHEntityId E; E.RunId=Run; E.Area=Source.Area; E.InstanceId=PortalId; return E; }
FLHEntityId ALHInteractableMarker::Materialize(const FGuid& Run) const { FLHEntityId E; E.RunId=Run; E.Area=Area; E.InstanceId=InstanceId; return E; }
bool ALHSpawnMarker::ResolveLife(const FLHAreaRecord& Record,FLHSpawnLifeId& Out) const
{
    if (!SpawnId.IsValid() || EnemyDefinitionId.Value.IsNone() || !LHWorld::SameArea(Area,Record.Area)) return false;
    const auto* Definition=LHWorld::FindArea(Area);
    if (!Definition || !Definition->Spawns.ContainsByPredicate([&](const auto& Slot) { return Slot.SpawnId==SpawnId && Slot.Enemy.Value.ToString()==EnemyDefinitionId.Value.ToString(); })) return false;
    const FLHEncounterRecord* Found=nullptr;
    for (const auto& E:Record.Encounters) if (E.Life.SpawnSlot==SpawnId)
    {
        if (Found || E.Life.LifeGeneration<0 || E.Definition.Value.ToString()!=EnemyDefinitionId.Value.ToString() || !LHWorld::SameArea(E.Life.Area,Area)) return false;
        Found=&E;
    }
    FLHSpawnLifeId Life; Life.Area=Area; Life.SpawnSlot=SpawnId;
    if (Found) Life=Found->Life;
    Out=Life; return true;
}
