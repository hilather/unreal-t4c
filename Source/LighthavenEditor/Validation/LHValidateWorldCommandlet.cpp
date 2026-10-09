#include "LHValidateWorldCommandlet.h"
#include "World/LHWorldMarkers.h"
#include "World/LHWorldValidation.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "UObject/StrongObjectPtr.h"
ULHValidateWorldCommandlet::ULHValidateWorldCommandlet()
{ IsClient=false; IsServer=false; IsEditor=true; LogToConsole=true; }
int32 ULHValidateWorldCommandlet::Main(const FString& Params)
{
    TArray<FLHAreaId> Maps; TArray<FLHPlacedIdentity> Identities; TArray<FLHPlacedEntrance> Entrances;
    // Keep all packages alive; no BeginPlay, simulation or binary writes.
    TArray<TStrongObjectPtr<UWorld>> Worlds;
    bool bLoadFailed=false;
    for (const auto& Area:LHWorld::Registry())
    {
        UWorld* World=LoadObject<UWorld>(nullptr,*Area.Map.ToString());
        if (!World || !World->PersistentLevel) { UE_LOG(LogTemp,Error,TEXT("Cannot load %s"),*Area.Map.ToString()); bLoadFailed=true; continue; }
        Worlds.Emplace(World); Maps.Add(Area.Id);
        TArray<const ULevel*> Levels; Levels.Add(World->PersistentLevel);
        for (const ULevel* Level:World->GetLevels()) if (Level && Level!=World->PersistentLevel) Levels.AddUnique(Level);
        for (const ULevel* Level:Levels) for (const AActor* Actor:Level->Actors)
        {
            if (!Actor) continue;
            const FString Context=Area.Id.Content.Value.ToString()+TEXT("/")+Actor->GetName();
            if (const auto* E=Cast<ALHEntranceMarker>(Actor))
            {
                FLHPlacedEntrance Entry; Entry.Id=E->EntranceId; Entry.Transform=E->SafeArrivalTransform; Entry.bSafetyReviewed=E->bSafetyReviewed; Entry.Context=Context;
                if (!LHWorld::SameArea(Entry.Id.Area,Area.Id)) bLoadFailed=true;
                Entrances.Add(Entry);
            }
            else if (const auto* P=Cast<ALHPortal>(Actor))
            {
                FLHPlacedIdentity I; I.Area=Area.Id; I.Id=P->PortalId; I.Kind=ELHPlacedIdKind::Portal; I.Source=P->Source; I.Destination=P->Destination; I.bReturn=P->Direction==ELHPortalDirection::Return; I.Context=Context; Identities.Add(I);
            }
            else if (const auto* S=Cast<ALHSpawnMarker>(Actor))
            {
                FLHPlacedIdentity I; I.Area=S->Area; I.Id=S->SpawnId; I.Definition=S->EnemyDefinitionId; I.Kind=ELHPlacedIdKind::Spawn; I.Context=Context;
                if (!LHWorld::SameArea(S->Area,Area.Id)) bLoadFailed=true;
                Identities.Add(I);
            }
            else if (const auto* O=Cast<ALHInteractableMarker>(Actor))
            {
                FLHPlacedIdentity I; I.Area=O->Area; I.Id=O->InstanceId; I.Definition=O->DefinitionId; I.Kind=ELHPlacedIdKind::Interactable; I.Context=Context;
                if (!LHWorld::SameArea(O->Area,Area.Id)) bLoadFailed=true;
                Identities.Add(I);
            }
        }
    }
    TArray<FString> Errors; const bool bValid=LHWorld::ValidatePlacements(Maps,Identities,Entrances,Errors);
    for (const auto& Error:Errors) UE_LOG(LogTemp,Error,TEXT("%s"),*Error);
    if (bLoadFailed || !bValid) return 1;
    UE_LOG(LogTemp,Display,TEXT("Lighthaven world identity/portal/entrance validation passed for five maps")); return 0;
}
