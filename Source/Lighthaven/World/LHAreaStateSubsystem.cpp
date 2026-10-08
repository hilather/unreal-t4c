#include "LHAreaStateSubsystem.h"
#include "LHAreaRegistry.h"
#include "Persistence/LHSaveCodec.h"
namespace LHAreaStatePrivate
{
bool Number(const FLHNumber& N) { return N.Resolution==ELHValueResolution::Resolved && FMath::IsFinite(N.Value) && N.Value>=0 && N.Value<=MAX_flt; }
}
bool LHWorld::ValidateWorld(const FLHWorldRecord& W,FString& Error)
{
    Error.Reset();
    auto Bad=[&](const TCHAR* Message) { Error=Message; return false; };
    if (!W.RunId.IsValid() || W.Areas.Num()>5) return Bad(TEXT("Invalid run or area count"));
    TSet<FName> Areas; TSet<FGuid> Slots; TSet<FGuid> Instances;
    for (const auto& A:W.Areas)
    {
        const auto* Def=FindArea(A.Area);
        if (!Def || Areas.Contains(A.Area.Content.Value) || A.Encounters.Num()>4096 || A.Objects.Num()>4096 || A.Corpses.Num()>1024) return Bad(TEXT("Unknown/duplicate/oversize area"));
        Areas.Add(A.Area.Content.Value);
        for (const auto& E:A.Encounters)
        {
            const auto* Slot=Def->Spawns.FindByPredicate([&](const auto& S) { return S.SpawnId==E.Life.SpawnSlot; });
            if (!Slot || Slots.Contains(E.Life.SpawnSlot) || !SameArea(E.Life.Area,A.Area) || E.Life.LifeGeneration<0 || Slot->Enemy.Value.ToString()!=E.Definition.Value.ToString() ||
                E.State==ELHEncounterLifeState::Unresolved || static_cast<uint8>(E.State)>static_cast<uint8>(ELHEncounterLifeState::PermanentlyDefeated) || !LHAreaStatePrivate::Number(E.CurrentHealth) || !LHAreaStatePrivate::Number(E.RespawnRemainingSeconds)) return Bad(TEXT("Invalid spawn/lifecycle data"));
            if ((E.State==ELHEncounterLifeState::Alive && (E.CurrentHealth.Value<=0 || E.bRewardCommitted)) || (E.State!=ELHEncounterLifeState::Alive && E.CurrentHealth.Value!=0)) return Bad(TEXT("Inconsistent encounter health/state"));
            if (E.bRewardCommitted || E.KillReward.Value.IsValid())
            {
                FLHRewardId Expected; FLHSaveError RewardError;
                if (!LHSave::EnemyLifeRewardId(W.RunId,E.Life,Expected,RewardError) || Expected.Value!=E.KillReward.Value) return Bad(TEXT("Invalid life reward identity"));
            }
            Slots.Add(E.Life.SpawnSlot);
        }
        auto Entity=[&](const FLHEntityId& E) { return E.RunId==W.RunId && SameArea(E.Area,A.Area) && E.InstanceId.IsValid() && !Instances.Contains(E.InstanceId); };
        auto Items=[&](const TArray<FLHItemInstance>& Values)
        {
            for (const auto& Item:Values)
            {
                if (Item.Id.RunId!=W.RunId || !FindArea(Item.Id.Area) || !Item.Id.InstanceId.IsValid() || Instances.Contains(Item.Id.InstanceId) || Item.Definition.Value.IsNone() || Item.Quantity.Resolution!=ELHValueResolution::Resolved || Item.Quantity.Value<=0 || Item.PermanentRolledValues.Num()>64) return false;
                for (const auto& Field:Item.PermanentRolledValues) if (Field.Key.IsNone() || Field.Value.Resolution!=ELHValueResolution::Resolved || !FMath::IsFinite(Field.Value.Value)) return false;
                Instances.Add(Item.Id.InstanceId);
            }
            return true;
        };
        for (const auto& O:A.Objects)
        {
            if (!Entity(O.Id) || O.Definition.Value.IsNone() || O.RemainingItems.Num()>512) return Bad(TEXT("Invalid interactable identity/data"));
            Instances.Add(O.Id.InstanceId);
            if (!Items(O.RemainingItems)) return Bad(TEXT("Invalid interactable item ownership"));
        }
        for (const auto& C:A.Corpses)
        {
            const auto* E=A.Encounters.FindByPredicate([&](const auto& R) { return R.Life.SpawnSlot==C.SourceLife.SpawnSlot; });
            if (!Entity(C.Container) || !E || !SameArea(C.SourceLife.Area,A.Area) || C.SourceLife.LifeGeneration<0 || C.SourceLife.LifeGeneration>E->Life.LifeGeneration || !C.bFinalized || !C.Reward.Value.IsValid() || C.RemainingItems.Num()>512 || (C.CleanupRemainingSeconds.Resolution==ELHValueResolution::Resolved && !LHAreaStatePrivate::Number(C.CleanupRemainingSeconds))) return Bad(TEXT("Invalid corpse data"));
            FLHRewardId Expected; FLHSaveError RewardError;
            if (!LHSave::EnemyLifeRewardId(W.RunId,C.SourceLife,Expected,RewardError) || Expected.Value!=C.Reward.Value || (C.SourceLife.LifeGeneration==E->Life.LifeGeneration && (!E->bRewardCommitted || E->KillReward.Value!=C.Reward.Value))) return Bad(TEXT("Invalid corpse reward"));
            Instances.Add(C.Container.InstanceId);
            if (C.RemainingGold.Resolution!=ELHValueResolution::Resolved || C.RemainingGold.Value<0 || (C.bClaimed && (!C.RemainingItems.IsEmpty() || C.RemainingGold.Value!=0)) || !Items(C.RemainingItems)) return Bad(TEXT("Invalid corpse contents"));
        }
    }
    return true;
}
bool ULHAreaStateSubsystem::Hydrate(const FLHWorldRecord& W,FString& Error)
{
    if (!LHWorld::ValidateWorld(W,Error)) return false;
    State=W; bHydrated=true; return true;
}
const FLHAreaRecord* ULHAreaStateSubsystem::Find(const FLHAreaId& Area) const
{ return bHydrated ? State.Areas.FindByPredicate([&](const auto& A) { return LHWorld::SameArea(A.Area,Area); }) : nullptr; }
bool ULHAreaStateSubsystem::StoreArea(const FLHAreaRecord& Area,FString& Error)
{
    if (!bHydrated) { Error=TEXT("Hydration required"); return false; }
    FLHWorldRecord Candidate=State;
    auto* Existing=Candidate.Areas.FindByPredicate([&](const auto& A) { return LHWorld::SameArea(A.Area,Area.Area); });
    if (Existing)
    {
        for (const auto& Old:Existing->Encounters)
        {
            const auto* New=Area.Encounters.FindByPredicate([&](const auto& E) { return E.Life.SpawnSlot==Old.Life.SpawnSlot; });
            if (!New || New->Life.LifeGeneration<Old.Life.LifeGeneration || (New->Life.LifeGeneration==Old.Life.LifeGeneration && Old.bRewardCommitted && !New->bRewardCommitted) || (New->Life.LifeGeneration==Old.Life.LifeGeneration && Old.State!=ELHEncounterLifeState::Alive && New->State==ELHEncounterLifeState::Alive) || (Old.State==ELHEncounterLifeState::PermanentlyDefeated && (New->State!=Old.State || New->Life.LifeGeneration!=Old.Life.LifeGeneration)))
            { Error=TEXT("Encounter high-water rollback rejected"); return false; }
        }
        *Existing=Area;
    }
    else Candidate.Areas.Add(Area);
    if (!LHWorld::ValidateWorld(Candidate,Error)) return false;
    State=MoveTemp(Candidate); return true;
}
bool ULHAreaStateSubsystem::PersistInto(FLHWorldRecord& W,FString& Error) const
{
    if (!bHydrated || W.RunId!=State.RunId) { Error=TEXT("Missing hydration or different run"); return false; }
    FLHWorldRecord Candidate=W; Candidate.Areas=State.Areas;
    if (!LHWorld::ValidateWorld(Candidate,Error)) return false;
    W=MoveTemp(Candidate); return true;
}
void ULHAreaStateSubsystem::Deinitialize() { State={}; bHydrated=false; Super::Deinitialize(); }
