#include "LHEncounterLifecycle.h"
#include "Persistence/LHSaveCodec.h"

namespace LHEncounterLifecyclePrivate
{
bool Ready(const FLHInteger& V) { return V.Resolution==ELHValueResolution::Resolved && V.Value>=0; }
bool Ready(const FLHNumber& V) { return V.Resolution==ELHValueResolution::Resolved && FMath::IsFinite(V.Value) && V.Value>=0 && V.Value<=MAX_flt; }
FLHInteger I(int64 V)
{
    FLHInteger R; R.Resolution=ELHValueResolution::Resolved; R.Value=V;
    R.Provenance.Status=ELHProvenanceStatus::Prototype;
    R.Provenance.Notes=TEXT("W4-04 lifecycle bookkeeping, derived from authored reward spec; not historical evidence"); return R;
}
FLHNumber N(double V)
{ FLHNumber R; R.Resolution=ELHValueResolution::Resolved; R.Value=V; R.Provenance=I(0).Provenance; return R; }
bool Same(const FLHEntityId& A,const FLHEntityId& B)
{ return A.RunId==B.RunId && A.Area.Content.Value==B.Area.Content.Value && A.InstanceId==B.InstanceId; }
bool SameLife(const FLHSpawnLifeId& A,const FLHSpawnLifeId& B)
{ return A.Area.Content.Value==B.Area.Content.Value && A.SpawnSlot==B.SpawnSlot && A.LifeGeneration==B.LifeGeneration; }
bool InitAuthority(FLHCharacterAuthority& A,const FLHCharacterProfile& P,const FLHSaveSnapshot& S)
{ return A.Initialize(P,S.Session.RequestEpoch,0) && A.Import(S)==ELHCommandReason::None; }
FLHEntityId Identity(const FGuid& Run,const FLHSpawnLifeId& Life,const TCHAR* Purpose)
{
    FLHRewardId Reward; FLHSaveError Error;
    if (!LHSave::EnemyLifeRewardId(Run,Life,Reward,Error)) return {};
    FString Input=FString(Purpose)+Reward.Value.ToString(EGuidFormats::Digits);
    FTCHARToUTF8 Utf8(*Input); TArray<uint8> Bytes; Bytes.Append(reinterpret_cast<const uint8*>(Utf8.Get()),Utf8.Length());
    FLHRewardId Mapped; if (!LHSave::RewardIdFromDigest(LHSave::Sha256(Bytes),Mapped,Error)) return {};
    FLHEntityId Result; Result.RunId=Run; Result.Area=Life.Area; Result.InstanceId=Mapped.Value; return Result;
}
bool EntityExists(const FLHSaveSnapshot& S,const FLHEntityId& ID)
{
    for (const auto& I:S.Character.Inventory) if (Same(I.Id,ID)) return true;
    for (const auto& A:S.World.Areas)
    {
        for (const auto& C:A.Corpses) { if (Same(C.Container,ID)) return true; for (const auto& I:C.RemainingItems) if (Same(I.Id,ID)) return true; }
        for (const auto& O:A.Objects) { if (Same(O.Id,ID)) return true; for (const auto& I:O.RemainingItems) if (Same(I.Id,ID)) return true; }
    }
    return false;
}
bool RewardExists(const FLHSaveSnapshot& S,const FLHRewardId& ID)
{
    for (const auto& G:S.Character.GrowthAwards) if (G.AwardId.Value==ID.Value) return true;
    for (const auto& R:S.World.ClaimedUniqueRewards) if (R.Value==ID.Value) return true;
    for (const auto& A:S.World.Areas) {
        for (const auto& E:A.Encounters) if (E.KillReward.Value==ID.Value) return true;
        for (const auto& C:A.Corpses) if (C.Reward.Value==ID.Value) return true;
    }
    for (const auto& Q:S.World.Quests) if (Q.TurnInClaim.Value==ID.Value) return true;
    for (const auto& B:S.World.Bosses) if (B.UniqueClaim.Value==ID.Value) return true;
    return false;
}
}

ELHCommandReason LHRewards::PopulateArea(FLHAreaRecord& Area,const FLHAreaDefinition& Definition,TFunctionRef<FLHNumber(const FLHContentId&)> MaxHealth)
{
    using namespace LHEncounterLifecyclePrivate;
    if (!Area.Encounters.IsEmpty() || !Area.Corpses.IsEmpty() || !Area.Objects.IsEmpty()) return ELHCommandReason::None;
    if (!Area.Area.Content.Value.IsNone() && Area.Area.Content.Value!=Definition.Id.Content.Value) return ELHCommandReason::InvalidRequest;
    if (Definition.Spawns.Num()>4096) return ELHCommandReason::Busy;
    auto Next=Area; Next.Area=Definition.Id; TSet<FGuid> Slots;
    for (const auto& Spawn:Definition.Spawns)
    {
        if (!Spawn.SpawnId.IsValid() || Slots.Contains(Spawn.SpawnId) || Spawn.Enemy.Value.IsNone()) return ELHCommandReason::InvalidRequest;
        Slots.Add(Spawn.SpawnId);
        auto Health=MaxHealth(Spawn.Enemy); if (!Ready(Health) || Health.Value<=0) return ELHCommandReason::UnresolvedRules;
        FLHEncounterRecord E; E.Life.Area=Definition.Id; E.Life.SpawnSlot=Spawn.SpawnId; E.Definition=Spawn.Enemy;
        E.State=ELHEncounterLifeState::Alive; E.CurrentHealth=Health; E.RespawnRemainingSeconds=N(0); Next.Encounters.Add(E);
    }
    Area=MoveTemp(Next); return ELHCommandReason::None;
}
FLHEntityId LHRewards::CorpseContainerFor(const FGuid& Run,const FLHSpawnLifeId& Life)
{ return LHEncounterLifecyclePrivate::Identity(Run,Life,TEXT("LHCorpse1:")); }

ELHCommandReason LHRewards::SettleKill(FLHSaveSnapshot& S,const FLHKillFacts& Facts,const FLHKillRewardSpec& Spec,const FLHCharacterProfile& Profile,TArrayView<ILHKillObserver* const> Observers)
{
    using namespace LHEncounterLifecyclePrivate;
    if (!Facts.bKillerIsPlayer || Facts.Area.Content.Value!=Facts.Life.Area.Content.Value) return ELHCommandReason::InvalidRequest;
    auto Next=S;
    auto* Area=Next.World.Areas.FindByPredicate([&](const auto& A) { return A.Area.Content.Value==Facts.Area.Content.Value; });
    if (!Area) return ELHCommandReason::NotFound;
    auto* E=Area->Encounters.FindByPredicate([&](const auto& X) { return X.Life.SpawnSlot==Facts.Life.SpawnSlot; });
    if (!E || E->Definition.Value!=Facts.Enemy.Value) return ELHCommandReason::NotFound;
    if (!SameLife(E->Life,Facts.Life) || E->bRewardCommitted || E->State==ELHEncounterLifeState::PermanentlyDefeated) return ELHCommandReason::InvalidLifeState;
    if (E->State!=ELHEncounterLifeState::Alive) return ELHCommandReason::InvalidLifeState;
    if (!Ready(Spec.Experience) || !Ready(Spec.GoldMin) || !Ready(Spec.GoldMax) || Spec.GoldMax.Value<Spec.GoldMin.Value ||
        !Ready(Spec.ItemDropChance) || Spec.ItemDropChance.Value>1 || !Ready(Spec.RespawnSeconds) || !Ready(Spec.SafetyDistanceCm)) return ELHCommandReason::UnresolvedRules;
    // Integer loot RNG uses bounded int32 intervals, never overflowed int64 range arithmetic.
    if (Spec.GoldMax.Value>MAX_int32 || Spec.GoldMax.Value-Spec.GoldMin.Value>=MAX_int32 || Spec.Loot.Num()>4096 || Area->Corpses.Num()>=1024) return ELHCommandReason::Busy;
    double TotalWeight=0;
    for (const auto& L:Spec.Loot)
    {
        if (L.Item.Value.IsNone() || !Ready(L.MinimumQuantity) || L.MinimumQuantity.Value<=0 || !Ready(L.MaximumQuantity) || L.MaximumQuantity.Value<L.MinimumQuantity.Value || L.MaximumQuantity.Value>MAX_int32 || !Ready(L.Weight)) return ELHCommandReason::UnresolvedRules;
        TotalWeight+=L.Weight.Value;
    }
    if (!FMath::IsFinite(TotalWeight) || (!Spec.Loot.IsEmpty() && TotalWeight<=0)) return ELHCommandReason::UnresolvedRules;
    FLHSaveError Error; FLHRewardId Reward;
    if (!LHSave::EnemyLifeRewardId(S.World.RunId,Facts.Life,Reward,Error)) return ELHCommandReason::InvalidRequest;
    if (E->KillReward.Value.IsValid() && E->KillReward.Value!=Reward.Value) return ELHCommandReason::InvalidRequest;
    auto CollisionCheck=S;
    for (auto& A:CollisionCheck.World.Areas) for (auto& X:A.Encounters) if (SameLife(X.Life,Facts.Life)) X.KillReward={};
    if (RewardExists(CollisionCheck,Reward)) return ELHCommandReason::InvalidRequest;
    FLHCharacterAuthority Authority;
    if (!InitAuthority(Authority,Profile,Next)) return ELHCommandReason::InvalidRequest;
    auto Reason=Authority.GrantExperience(Spec.Experience.Value); if (Reason!=ELHCommandReason::None) return Reason;
    Authority.Export(Next);
    Next.Header.TransactionSequence=S.Header.TransactionSequence;
    for (const auto& G:Next.Character.GrowthAwards)
        if (!S.Character.GrowthAwards.ContainsByPredicate([&](const auto& Old) { return Old.AwardId.Value==G.AwardId.Value; }) &&
            (G.AwardId.Value==Reward.Value || RewardExists(S,G.AwardId))) return ELHCommandReason::InvalidRequest;
    auto* Rng=Next.Session.GameplayRng.FindByPredicate([](const auto& R) { return R.StreamId==TEXT("RNG.Loot"); });
    if (!Rng || Rng->Algorithm!=TEXT("UE.FRandomStream") || Rng->AlgorithmRevision!=1 || Rng->State.Num()!=4) return ELHCommandReason::UnresolvedRules;
    uint32 Seed=0; for (int32 B=0; B<4; ++B) Seed|=uint32(Rng->State[B])<<(B*8);
    FRandomStream Random(static_cast<int32>(Seed));
    FLHCorpseLootRecord C; C.Container=CorpseContainerFor(S.World.RunId,Facts.Life);
    if (!C.Container.InstanceId.IsValid() || EntityExists(Next,C.Container)) return ELHCommandReason::InvalidRequest;
    C.SourceLife=Facts.Life; C.Reward=Reward; C.bFinalized=true;
    C.RemainingGold=I(Random.RandRange(static_cast<int32>(Spec.GoldMin.Value),static_cast<int32>(Spec.GoldMax.Value)));
    C.CleanupRemainingSeconds=N(300); C.CleanupRemainingSeconds.Provenance.Notes=TEXT("Prototype 300s loaded/unpaused ordinary corpse cleanup; R-03 missing, world-ledger 2026-10-09; replace at economy review");
    if (!Spec.Loot.IsEmpty() && Random.FRand()<Spec.ItemDropChance.Value)
    {
        double Pick=Random.FRand()*TotalWeight; const FLHLootEntry* Chosen=nullptr;
        for (const auto& L:Spec.Loot) if (L.Weight.Value>0) { Chosen=&L; Pick-=L.Weight.Value; if (Pick<0) break; }
        if (!Chosen) return ELHCommandReason::UnresolvedRules;
        FLHItemInstance Item; Item.Id=Identity(S.World.RunId,Facts.Life,TEXT("LHCorpseItem1:")); Item.Definition=Chosen->Item;
        if (!Item.Id.InstanceId.IsValid() || Same(Item.Id,C.Container) || EntityExists(Next,Item.Id)) return ELHCommandReason::InvalidRequest;
        const auto* Definition=Profile.Items.FindByPredicate([&](const auto& D) { return D.Id.Value==Item.Definition.Value; });
        Item.Quantity=I(Random.RandRange(static_cast<int32>(Chosen->MinimumQuantity.Value),static_cast<int32>(Chosen->MaximumQuantity.Value)));
        if (!Definition || !Ready(Definition->StackLimit) || Item.Quantity.Value>Definition->StackLimit.Value) return ELHCommandReason::UnresolvedRules;
        C.RemainingItems.Add(Item);
    }
    Seed=static_cast<uint32>(Random.GetCurrentSeed()); for (int32 B=0; B<4; ++B) Rng->State[B]=static_cast<uint8>(Seed>>(B*8));
    C.bClaimed=C.RemainingItems.IsEmpty() && C.RemainingGold.Value==0;
    // Export changes character and RNG only; pointers into Next.World are still valid here.
    E->CurrentHealth=N(0); E->State=Spec.RespawnPolicy==ELHRespawnPolicy::PermanentDefeat?ELHEncounterLifeState::PermanentlyDefeated:ELHEncounterLifeState::Dead;
    E->RespawnRemainingSeconds=Spec.RespawnSeconds; E->KillReward=Reward; E->bRewardCommitted=true;
    Area->Corpses.Add(C);
    if (Spec.bBoss)
    {
        FLHRewardId Unique; if (!LHSave::BossUniqueRewardId(S.World.RunId,Facts.Enemy,Unique,Error)) return ELHCommandReason::InvalidRequest;
        auto* Boss=Next.World.Bosses.FindByPredicate([&](const auto& B) { return B.Boss.Value==Facts.Enemy.Value; });
        if (!Boss) { FLHBossRecord B; B.Boss=Facts.Enemy; Next.World.Bosses.Add(B); Boss=&Next.World.Bosses.Last(); }
        if (!Boss->bDefeated)
        {
            if (RewardExists(Next,Unique) || Next.World.ClaimedUniqueRewards.Num()>=4096) return ELHCommandReason::Busy;
            Boss->bDefeated=true; Boss->UniqueClaim=Unique; Next.World.ClaimedUniqueRewards.Add(Unique);
        }
        else if (Boss->UniqueClaim.Value!=Unique.Value || !Next.World.ClaimedUniqueRewards.ContainsByPredicate([&](const auto& R) { return R.Value==Unique.Value; })) return ELHCommandReason::InvalidRequest;
    }
    for (auto* Observer:Observers)
    {
        if (!Observer) return ELHCommandReason::InvalidRequest;
        Reason=Observer->OnSettledKill(Facts,Next); if (Reason!=ELHCommandReason::None) return Reason;
    }
    // Observer edits are subject to both snapshot and character validation before publication.
    if (!LHSave::Validate(Next,Error) || Authority.Import(Next)!=ELHCommandReason::None) return ELHCommandReason::InvalidRequest;
    S=MoveTemp(Next); return ELHCommandReason::None;
}

ELHCommandReason LHRewards::AdvanceRespawns(FLHAreaRecord& Area,double Seconds,TFunctionRef<bool(const FGuid&)> Safe,TArray<FLHSpawnLifeId>& NewLives)
{
    using namespace LHEncounterLifecyclePrivate;
    if (!FMath::IsFinite(Seconds) || Seconds<0) return ELHCommandReason::InvalidRequest;
    auto Next=Area; TArray<FLHSpawnLifeId> Lives;
    for (auto& E:Next.Encounters)
    {
        if (E.State!=ELHEncounterLifeState::Dead && E.State!=ELHEncounterLifeState::RespawnPending) continue;
        if (!E.bRewardCommitted || !Ready(E.RespawnRemainingSeconds)) return ELHCommandReason::InvalidRequest;
        E.RespawnRemainingSeconds.Value=FMath::Max(0.0,E.RespawnRemainingSeconds.Value-Seconds);
        if (E.RespawnRemainingSeconds.Value>0) continue;
        E.State=ELHEncounterLifeState::RespawnPending;
        if (Seconds==0 || !Safe(E.Life.SpawnSlot)) continue;
        if (E.Life.LifeGeneration==MAX_int64) return ELHCommandReason::Busy;
        ++E.Life.LifeGeneration; E.bRewardCommitted=false; E.KillReward={}; E.State=ELHEncounterLifeState::Alive;
        // Core stores current HP only. Integrator fills catalog max HP for returned lives in this same candidate.
        Lives.Add(E.Life);
    }
    Area=MoveTemp(Next); NewLives.Append(Lives); return ELHCommandReason::None;
}

ELHCommandReason LHRewards::ExecuteTakeLoot(FLHSaveSnapshot& S,const FLHCharacterProfile& Profile,const FLHTakeLootRequest& R)
{
    using namespace LHEncounterLifecyclePrivate;
    auto Reason=LHValidateLootTransferPayload(R); if (Reason!=ELHCommandReason::None) return Reason;
    if (R.Container.RunId!=S.World.RunId || R.Container.Area.Content.Value!=S.Character.ActiveEntrance.Area.Content.Value) return ELHCommandReason::NotFound;
    if (!Ready(S.Character.CurrentHealth) || S.Character.CurrentHealth.Value<=0) return ELHCommandReason::InvalidLifeState;
    auto Next=S; auto* Area=Next.World.Areas.FindByPredicate([&](const auto& A) { return A.Area.Content.Value==R.Container.Area.Content.Value; });
    if (!Area) return ELHCommandReason::NotFound;
    auto* C=Area->Corpses.FindByPredicate([&](const auto& X) { return Same(X.Container,R.Container); });
    if (!C || !C->bFinalized || C->bClaimed) return ELHCommandReason::NotFound;
    if (R.Kind==ELHLootTransferKind::Gold)
    {
        if (!Ready(C->RemainingGold) || !Ready(Next.Character.Gold) || R.Quantity.Value>C->RemainingGold.Value || R.Quantity.Value>MAX_int64-Next.Character.Gold.Value) return ELHCommandReason::InvalidRequest;
        C->RemainingGold.Value-=R.Quantity.Value; Next.Character.Gold.Value+=R.Quantity.Value;
    }
    else
    {
        auto* Item=C->RemainingItems.FindByPredicate([&](const auto& I) { return Same(I.Id,R.Item); });
        if (!Item || !Ready(Item->Quantity) || R.Quantity.Value>Item->Quantity.Value) return ELHCommandReason::NotFound;
        // Partial stacks get a deterministic request-derived identity, preserving global single ownership.
        auto Transfer=*Item; Transfer.Quantity.Value=R.Quantity.Value;
        if (Transfer.Quantity.Value<Item->Quantity.Value)
        {
            if (!R.Request.Value.IsValid() || R.Request.Epoch!=S.Session.RequestEpoch) return ELHCommandReason::InvalidRequest;
            FString Key=TEXT("LHPartialLoot1:")+R.Request.Epoch.ToString(EGuidFormats::Digits)+R.Request.Value.ToString(EGuidFormats::Digits);
            FTCHARToUTF8 Utf8(*Key); TArray<uint8> B; B.Append(reinterpret_cast<const uint8*>(Utf8.Get()),Utf8.Length()); FLHRewardId ID; FLHSaveError Error;
            if (!LHSave::RewardIdFromDigest(LHSave::Sha256(B),ID,Error)) return ELHCommandReason::InvalidRequest;
            Transfer.Id.InstanceId=ID.Value; if (EntityExists(Next,Transfer.Id)) return ELHCommandReason::InvalidRequest;
        }
        FLHCharacterAuthority Authority; if (!InitAuthority(Authority,Profile,Next)) return ELHCommandReason::InvalidRequest;
        Reason=Authority.AddItem(Transfer); if (Reason!=ELHCommandReason::None) return Reason;
        Authority.Export(Next);
        Next.Header.TransactionSequence=S.Header.TransactionSequence;
        Item->Quantity.Value-=R.Quantity.Value;
        C->RemainingItems.RemoveAll([](const auto& I) { return I.Quantity.Value==0; });
    }
    C->bClaimed=C->RemainingItems.IsEmpty() && C->RemainingGold.Value==0;
    FLHSaveError Error; FLHCharacterAuthority Authority;
    if (!LHSave::Validate(Next,Error) || !InitAuthority(Authority,Profile,Next)) return ELHCommandReason::InvalidRequest;
    S=MoveTemp(Next); return ELHCommandReason::None;
}

ELHCommandReason LHRewards::SettlePlayerDeath(FLHSaveSnapshot& S,const FLHCharacterProfile& Profile,const FLHCheckpoint& Recovery,bool bRecoverySafetyReviewed)
{
    using namespace LHEncounterLifecyclePrivate;
    if (!Ready(S.Character.CurrentHealth) || S.Character.CurrentHealth.Value!=0) return ELHCommandReason::InvalidLifeState;
    if (Recovery.TransformResolution!=ELHValueResolution::Resolved) return ELHCommandReason::InvalidDestination;
    const auto* Entrance=LHWorld::FindEntrance(Recovery.Entrance);
    if (!Entrance || !bRecoverySafetyReviewed || !Recovery.SafeTransform.Equals(Entrance->SafeTransform)) return ELHCommandReason::InvalidDestination;
    FLHCharacterAuthority Authority; if (!InitAuthority(Authority,Profile,S)) return ELHCommandReason::InvalidRequest;
    const auto Stats=Authority.Stats(); if (!Stats.Diagnostic.IsAccepted()) return ELHCommandReason::UnresolvedRules;
    auto Next=S; Next.Session.SafeRespawn=Recovery; Next.Character.ActiveEntrance=Recovery.Entrance;
    Next.Character.CurrentHealth=N(Stats.Value.MaxHealth); Next.Character.CurrentMana=N(Stats.Value.MaxMana);
    Next.Character.CurrentHealth.Provenance.Notes=TEXT("Prototype full HP/MP church recovery; R-03 missing, world-ledger 2026-10-09; no-development-penalty owner policy; replace at death-policy review");
    Next.Character.CurrentMana.Provenance=Next.Character.CurrentHealth.Provenance;
    if (Next.Character.CurrentHealth.Value<=0) return ELHCommandReason::UnresolvedRules;
    FLHSaveError Error; if (!LHSave::Validate(Next,Error) || Authority.Import(Next)!=ELHCommandReason::None) return ELHCommandReason::InvalidRequest;
    S=MoveTemp(Next); return ELHCommandReason::None;
}
ELHCommandReason LHRewards::AdvanceLootCleanup(FLHAreaRecord& Area,double Seconds,TFunctionRef<bool(const FLHContentId&)> Protected,TFunctionRef<bool(const FLHEntityId&)> Referenced)
{
    using namespace LHEncounterLifecyclePrivate;
    if (!FMath::IsFinite(Seconds) || Seconds<0) return ELHCommandReason::InvalidRequest;
    auto Next=Area;
    for (auto& C:Next.Corpses)
    {
        if (Referenced(C.Container)) continue;
        if (C.RemainingItems.ContainsByPredicate([&](const auto& I) { return Protected(I.Definition); })) continue;
        if (!Ready(C.CleanupRemainingSeconds)) return ELHCommandReason::UnresolvedRules;
        C.CleanupRemainingSeconds.Value=FMath::Max(0.0,C.CleanupRemainingSeconds.Value-Seconds);
        if (C.CleanupRemainingSeconds.Value==0) { C.RemainingItems.Reset(); C.RemainingGold=I(0); C.bClaimed=true; }
    }
    Next.Corpses.RemoveAll([&](const auto& C) { return C.bClaimed && !Referenced(C.Container); });
    Area=MoveTemp(Next); return ELHCommandReason::None;
}

ELHCommandReason LHRewards::AdvanceRespawns(FLHAreaRecord& Area,const FGuid& Run,double Seconds,TFunctionRef<bool(const FGuid&)> Safe,TFunctionRef<FLHNumber(const FLHContentId&)> MaxHealth,TArray<FLHSpawnLifeId>& NewLives)
{
    using namespace LHEncounterLifecyclePrivate;
    if (!Run.IsValid()) return ELHCommandReason::InvalidRequest;
    auto Next=Area; TArray<FLHSpawnLifeId> Lives;
    auto Reason=AdvanceRespawns(Next,Seconds,Safe,Lives); if (Reason!=ELHCommandReason::None) return Reason;
    for (const auto& Life:Lives)
    {
        auto* E=Next.Encounters.FindByPredicate([&](const auto& X) { return SameLife(X.Life,Life); });
        auto Health=MaxHealth(E->Definition); if (!Ready(Health) || Health.Value<=0) return ELHCommandReason::UnresolvedRules;
        FLHSaveError Error; if (!LHSave::EnemyLifeRewardId(Run,Life,E->KillReward,Error)) return ELHCommandReason::InvalidRequest;
        E->CurrentHealth=Health;
    }
    Area=MoveTemp(Next); NewLives.Append(Lives); return ELHCommandReason::None;
}
