#include "Data/Encounters/LHEncounterCatalog.h"
#include "Data/Enemies/LHEnemyCatalog.h"
#include "Data/LHDataProvenance.h"
const TArray<FLHEncounterCatalogRow>& LHEncounterData::Catalog()
{
    static const TArray<FLHEncounterCatalogRow> Rows=[]
    {
        TArray<FLHEncounterCatalogRow> Result;
        for (const auto& Area:LHWorld::Registry()) for (const auto& Slot:Area.Spawns)
        {
            const auto* E=LHEnemyData::Find(Slot.Enemy); check(E);
            FLHEncounterCatalogRow R; R.SpawnId=Slot.SpawnId; R.Area=Area.Id; R.Enemy=Slot.Enemy;
            R.RespawnSeconds=E->Reward.RespawnSeconds; R.SafetyDistanceCm=E->Reward.SafetyDistanceCm; R.bBoss=E->Runtime.bBoss;
            R.PlacementResolution=ELHValueResolution::Resolved;
            R.PlacementProvenance=LHData::Prototype(TEXT("Placement"),TEXT("Registry-owned anchor/count; world-ledger source spawn positions/counts missing. Replace only via W4-06 registry/generator review; CatalogHash changes."));
            if (R.Enemy.Value==TEXT("Enemy.DungeonBat"))
                R.PlacementProvenance.Notes+=TEXT(" R-03 Dungeon Bat floors missing; B2 provisional, no parity claim.");
            if (R.bBoss) R.PlacementProvenance.Notes+=TEXT(" B4 floor confirmed, arena anchor pending M4 check.");
            R.PlacementProvenance.ValueAsRecorded=Slot.Alias.ToString();
            R.Policies={
                LHData::Policy(TEXT("RespawnClock"),TEXT("LoadedFloorActiveSimulation"),LHData::Prototype(TEXT("RespawnClock"),TEXT("R-03 timer origin/clock missing; loaded unpaused only; replace after research."))),
                LHData::Policy(TEXT("LiveReload"),TEXT("AnchorKeepHealth"),LHData::Prototype(TEXT("LiveReload"),TEXT("world-ledger live reload details missing; retain health at registry anchor; replace after review."))),
                LHData::Policy(TEXT("RespawnPolicy"),TEXT("OrdinaryRepeat"),LHData::Provenance(TEXT("RespawnPolicy"),TEXT(""),TEXT("2026-10-09"),ELHProvenanceStatus::Modernized,TEXT("Matt recurring Balork decision; completion single-claim is separate.")))};
            Result.Add(MoveTemp(R));
        }
        return Result;
    }(); return Rows;
}
const FLHEncounterCatalogRow* LHEncounterData::ForSpawn(const FGuid& Id)
{ return Catalog().FindByPredicate([&](const auto& R){return R.SpawnId==Id;}); }
