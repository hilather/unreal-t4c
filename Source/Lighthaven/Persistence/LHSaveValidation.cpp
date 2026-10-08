#include "LHSaveCodec.h"

namespace LHSaveValidationPrivate
{
bool Digest(const FString& S)
{
    if (S.Len()!=64) return false;
    for (TCHAR C:S) if (!((C>='0' && C<='9') || (C>='a' && C<='f'))) return false;
    return true;
}
bool Id(FName N, bool Dotted=true)
{
    const FString S=N.ToString();
    if (N.IsNone() || S.Len()>128) return false;
    bool Start=true, Dot=false;
    for (TCHAR C:S)
    {
        const bool Letter=(C>='A' && C<='Z') || (C>='a' && C<='z');
        if (C=='.') { if (Start) return false; Start=true; Dot=true; }
        else { if (!(Letter || (!Start && ((C>='0' && C<='9') || C=='_')))) return false; Start=false; }
    }
    return !Start && (!Dotted || Dot);
}
bool Area(const FLHAreaId& A)
{
    const FString S=A.Content.Value.ToString();
    return S==TEXT("Area.LighthavenTempleDistrict") || S==TEXT("Area.TempleB1") || S==TEXT("Area.TempleB2") || S==TEXT("Area.TempleB3") || S==TEXT("Area.TempleB4");
}
bool Integer(const FLHInteger& V, int64 Min=0)
{ return V.Resolution==ELHValueResolution::Resolved && V.Value>=Min; }
bool Number(const FLHNumber& V, double Min=0)
{ return V.Resolution==ELHValueResolution::Resolved && FMath::IsFinite(V.Value) && V.Value>=Min && V.Value<=MAX_flt; }
bool Attributes(const FLHAttributeBlock& A)
{ return Integer(A.Strength) && Integer(A.Endurance) && Integer(A.Agility) && Integer(A.Intelligence) && Integer(A.Wisdom); }
bool Rules(const FLHRulesetRef& R)
{ return Id(R.Id.Value) && R.Revision>0 && R.HashAlgorithm==TEXT("SHA256") && Digest(R.ContentHash); }
bool SameRules(const FLHRulesetRef& A, const FLHRulesetRef& B)
{ return A.Id.Value.ToString()==B.Id.Value.ToString() && A.Revision==B.Revision && A.ContentHash==B.ContentHash && A.HashAlgorithm==B.HashAlgorithm; }
bool Rng(const FLHRngState& R, bool Gameplay)
{
    if (!Id(R.StreamId) || R.AlgorithmRevision!=1 || R.State.Num()>64) return false;
    if (R.Algorithm==TEXT("UE.FRandomStream")) return R.State.Num()==4;
    if (Gameplay || R.Algorithm!=TEXT("LH.NormalizedRollPair") || R.State.Num()!=16) return false;
    for (int32 Offset : {0,8})
    {
        uint64 U=0; for (int32 I=0; I<8; ++I) U|=uint64(R.State[Offset+I])<<(I*8);
        double V; FMemory::Memcpy(&V,&U,8); if (!FMath::IsFinite(V) || V<0 || V>1) return false;
    }
    return true;
}
}

bool LHSave::Validate(const FLHSaveSnapshot& S, FLHSaveError& E)
{
    using namespace LHSaveValidationPrivate;
    E={};
    auto Bad=[&E](const TCHAR* D) { E={ELHSaveReason::InvalidSnapshot,D}; return false; };
    if (S.Header.SchemaVersion>1) { E={ELHSaveReason::FutureSchema,TEXT("Future schema")}; return false; }
    if (!SupportsVersion(S.Header.SchemaVersion)) { E={ELHSaveReason::UnsupportedSchema,TEXT("No migration dispatch")}; return false; }
    if (S.Header.ChecksumAlgorithm!=TEXT("SHA256") || S.Header.Ruleset.HashAlgorithm!=TEXT("SHA256"))
    { E={ELHSaveReason::UnknownAlgorithm,TEXT("Explicit SHA256 required")}; return false; }
    if (S.Header.PayloadCodec!=TEXT("LHCanonicalBinary1")) { E={ELHSaveReason::UnknownCodec,TEXT("Explicit LHCanonicalBinary1 required")}; return false; }
    if (S.Header.Magic!=TEXT("LHSave") || S.Header.TransactionSequence<=0 || S.Header.BuildId.IsEmpty() ||
        !S.Header.CharacterId.Value.IsValid() || !Rules(S.Header.Ruleset) || !Digest(S.Header.ContentRevision) || !S.World.RunId.IsValid())
        return Bad(TEXT("Invalid envelope/run identity"));
    if (!S.Session.RequestEpoch.IsValid()) { E={ELHSaveReason::EpochMismatch,TEXT("Missing active request epoch")}; return false; }
    if (S.Session.EffectPolicy!=ELHEffectSavePolicy::CompletedActionBoundaryOnly || !S.Session.DurableEffects.IsEmpty())
        return Bad(TEXT("Only completed boundaries and empty v1 durable-effect allowlist supported"));
    const auto& C=S.Character;
    if (C.DisplayName.IsEmpty() || C.RebirthCount!=0 || !Attributes(C.BaseAttributes) || !Attributes(C.Creation.AcceptedAttributes) ||
        !Integer(C.Creation.GenerationRevision,1) || !Id(C.Creation.GenerationPolicy.Value) ||
        !Integer(C.EarnedLevel,1) || !Integer(C.ExperienceBalance) || !Integer(C.ExperienceDebt) ||
        !Integer(C.UnspentAttributePoints) || !Integer(C.UnspentSkillPoints) || !Integer(C.Gold) ||
        !Number(C.EarnedBaseHealth) || !Number(C.EarnedBaseMana) || !Number(C.CurrentHealth) || !Number(C.CurrentMana) ||
        !Number(S.Session.ManaRegenFractionalSeconds)) return Bad(TEXT("Required character values unresolved/invalid"));
    // Registry/rules authority checks maxima after equipment/effect restoration; current pools are not clamped to base here.
    for (auto A:C.AppearanceIds) if (!Id(A.Value)) return Bad(TEXT("Invalid appearance ID"));
    for (const auto& Q:C.Creation.QuestionAnswers) if (!Id(Q.Question.Value) || !Id(Q.Answer.Value)) return Bad(TEXT("Invalid creation answer"));
    for (const auto& R:C.Creation.AcceptedRollInputs) if (!Rng(R,false)) return Bad(TEXT("Unsupported creation RNG input"));
    for (const auto& R:S.Session.GameplayRng) if (!Rng(R,true)) return Bad(TEXT("Unsupported gameplay RNG state"));
    auto HasArea=[&S](const FLHAreaId& A)
    { return Area(A) && S.World.Areas.ContainsByPredicate([&A](const FLHAreaRecord& V) { return V.Area.Content.Value.ToString()==A.Content.Value.ToString(); }); };
    auto Entity=[&](const FLHEntityId& I)
    { return I.RunId==S.World.RunId && I.InstanceId.IsValid() && HasArea(I.Area); };
    auto Entrance=[&](const FLHEntranceId& I) { return HasArea(I.Area) && Id(I.LocalId,false); };
    if (!Entrance(C.ActiveEntrance) || !Entrance(S.Session.SafeRespawn.Entrance) || S.Session.SafeRespawn.TransformResolution!=ELHValueResolution::Resolved)
        return Bad(TEXT("Required active/safe entrance unresolved"));
    auto Fields=[](const TArray<FLHMechanicalField>& Values)
    { for (const auto& F:Values) if (!Id(F.Key,false) || F.Value.Resolution!=ELHValueResolution::Resolved || !FMath::IsFinite(F.Value.Value)) return false; return true; };
    TSet<FString> ItemIds;
    auto Item=[&](const FLHItemInstance& I)
    {
        const FString Key=I.Id.RunId.ToString()+I.Id.Area.Content.Value.ToString()+I.Id.InstanceId.ToString();
        if (ItemIds.Contains(Key)) return false;
        ItemIds.Add(Key);
        return Entity(I.Id) && Id(I.Definition.Value) && Integer(I.Quantity,1) && Fields(I.PermanentRolledValues);
    };
    for (const auto& I:C.Inventory) if (!Item(I)) return Bad(TEXT("Invalid inventory instance"));
    TSet<FGuid> Equipped;
    for (const auto& B:C.Equipment)
    {
        if (B.Slot==ELHEquipmentSlot::Unspecified || !Entity(B.Item) || Equipped.Contains(B.Item.InstanceId) ||
            !C.Inventory.ContainsByPredicate([&B](const FLHItemInstance& I) { return I.Id.InstanceId==B.Item.InstanceId && I.Id.RunId==B.Item.RunId && I.Id.Area.Content.Value==B.Item.Area.Content.Value; }))
            return Bad(TEXT("Equipment must reference a unique owned inventory instance"));
        Equipped.Add(B.Item.InstanceId);
    }
    for (const auto& K:C.LearnedSkills) if (!Id(K.Skill.Value) || !Integer(K.TrainedValue)) return Bad(TEXT("Invalid learned skill"));
    for (auto K:C.LearnedSpells) if (!Id(K.Value)) return Bad(TEXT("Invalid learned spell"));
    int64 Previous=0;
    for (const auto& G:C.GrowthAwards)
    {
        if (!G.AwardId.Value.IsValid() || !Integer(G.FromLevel,1) || !Integer(G.ToLevel,2) || G.FromLevel.Value==MAX_int64 ||
            G.ToLevel.Value!=G.FromLevel.Value+1 || G.FromLevel.Value<=Previous || G.ToLevel.Value>C.EarnedLevel.Value ||
            !Attributes(G.GrowthInputs) || !Number(G.HealthIncrement) || !Number(G.ManaIncrement) ||
            !Integer(G.AttributePoints) || !Integer(G.SkillPoints) || !SameRules(G.Ruleset,S.Header.Ruleset)) return Bad(TEXT("Invalid historical growth award"));
        Previous=G.FromLevel.Value;
        for (const auto& R:G.RollInputs) if (!Rng(R,false)) return Bad(TEXT("Invalid historical roll input"));
    }
    for (const auto& R:S.Session.RecentRequests)
    {
        if (R.Request.Epoch!=S.Session.RequestEpoch) { E={ELHSaveReason::EpochMismatch,TEXT("Receipt belongs to a different request epoch")}; return false; }
        if (!R.Request.Value.IsValid() || !Digest(R.PayloadDigest) || R.TransactionSequence<=0 || R.TransactionSequence>S.Header.TransactionSequence)
            return Bad(TEXT("Invalid completed request receipt"));
    }
    for (const auto& A:S.World.Areas)
    {
        if (!Area(A.Area)) return Bad(TEXT("Unsupported playable area"));
        auto Life=[&A](const FLHSpawnLifeId& L) { return L.Area.Content.Value.ToString()==A.Area.Content.Value.ToString() && L.SpawnSlot.IsValid() && L.LifeGeneration>=0; };
        TSet<FGuid> SpawnSlots;
        for (const auto& R:A.Encounters)
        {
            if (SpawnSlots.Contains(R.Life.SpawnSlot)) return Bad(TEXT("Encounter must retain exactly one latest high-water record per slot"));
            SpawnSlots.Add(R.Life.SpawnSlot);
            if (!Life(R.Life) || !Id(R.Definition.Value) || R.State==ELHEncounterLifeState::Unresolved || !Number(R.CurrentHealth) ||
                !Number(R.RespawnRemainingSeconds) || (R.bRewardCommitted && !R.KillReward.Value.IsValid())) return Bad(TEXT("Invalid encounter lifecycle"));
        }
        for (const auto& R:A.Corpses)
        {
            if (!Entity(R.Container) || R.Container.Area.Content.Value!=A.Area.Content.Value || !Life(R.SourceLife) || !R.Reward.Value.IsValid() ||
                !R.bFinalized || !Integer(R.RemainingGold) || (R.bClaimed && (!R.RemainingItems.IsEmpty() || R.RemainingGold.Value!=0)) ||
                (R.CleanupRemainingSeconds.Resolution==ELHValueResolution::Resolved && !Number(R.CleanupRemainingSeconds))) return Bad(TEXT("Invalid finalized corpse"));
            const auto* Encounter=A.Encounters.FindByPredicate([&R](const FLHEncounterRecord& X) { return X.Life.SpawnSlot==R.SourceLife.SpawnSlot; });
            if (!Encounter || R.SourceLife.LifeGeneration>Encounter->Life.LifeGeneration ||
                (R.SourceLife.LifeGeneration==Encounter->Life.LifeGeneration && (!Encounter->bRewardCommitted || Encounter->KillReward.Value!=R.Reward.Value)))
                return Bad(TEXT("Corpse lacks retained encounter high-water/reward"));
            for (const auto& I:R.RemainingItems) if (!Item(I)) return Bad(TEXT("Invalid corpse item"));
        }
        for (const auto& R:A.Objects)
        {
            if (!Entity(R.Id) || R.Id.Area.Content.Value!=A.Area.Content.Value || !Id(R.Definition.Value)) return Bad(TEXT("Invalid world object"));
            for (const auto& I:R.RemainingItems) if (!Item(I)) return Bad(TEXT("Invalid object item"));
        }
    }
    for (const auto& Q:S.World.Quests)
    {
        if (!Id(Q.Quest.Value) || !Id(Q.Stage,false) || !Integer(Q.EligibleKillCount) || (Q.bCompleted && !Q.bAccepted) ||
            (Q.bRewarded && (!Q.bCompleted || !Q.TurnInClaim.Value.IsValid()))) return Bad(TEXT("Invalid quest transaction"));
        for (auto F:Q.Flags) if (!Id(F.Value)) return Bad(TEXT("Invalid quest flag"));
    }
    for (const auto& B:S.World.Bosses)
    {
        if (!Id(B.Boss.Value)) return Bad(TEXT("Invalid boss"));
        for (auto F:B.Marks) if (!Id(F.Value)) return Bad(TEXT("Invalid boss mark"));
        for (auto F:B.DialogueFlags) if (!Id(F.Value)) return Bad(TEXT("Invalid boss dialogue flag"));
    }
    for (auto R:S.World.ClaimedUniqueRewards) if (!R.Value.IsValid()) return Bad(TEXT("Invalid retained unique claim"));
    for (const auto& P:S.World.UnlockedPortals) if (!Entity(P)) return Bad(TEXT("Invalid unlocked portal"));
    for (const auto& T:S.Session.Cooldowns)
        if (!Entity(T.Owner) || !Id(T.Ability.Value) || !Number(T.RemainingSeconds)) return Bad(TEXT("Invalid owner-scoped cooldown"));
    // Diagnostics get a stricter field-specific byte bound, including UTF-8 expansion.
    int32 DiagnosticBytes=0;
    for (const auto& D:S.Session.Diagnostics)
    {
        FTCHARToUTF8 U(*D); if (U.Length()>512 || U.Length()>32768-DiagnosticBytes) { E={ELHSaveReason::Oversize,TEXT("Diagnostic byte limit")}; return false; }
        DiagnosticBytes+=U.Length();
    }
    return true;
}
