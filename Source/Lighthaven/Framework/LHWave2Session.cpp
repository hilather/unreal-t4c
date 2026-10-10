#include "Framework/LHWave2Session.h"
#include "World/LHWorldMarkers.h"
#include "EngineUtils.h"
#include "Framework/LHWave2Profile.h"
#include "Framework/LHPlayerState.h"
#include "Abilities/LHCombatComponent.h"
#include "Abilities/LHAttributeSet.h"
#include "World/LHAreaRegistry.h"
#include "World/LHAreaStateSubsystem.h"
#include "Abilities/LHResourceRecovery.h"
#include "Abilities/LHLightEffect.h"
#include "AI/LHEncounterDirector.h"
#include "Abilities/LHAbilityCatalog.h"
#include "Services/LHServiceCatalog.h"
#include "Data/Enemies/LHEnemyCatalog.h"
#include "Data/Items/LHItemCatalog.h"
namespace LHWave2SessionPrivate
{
bool ValidateReferences(const FLHSaveSnapshot& S, FLHSaveError& E)
{
    auto Bad=[&E]() { E={ELHSaveReason::InvalidSnapshot,TEXT("Unsupported Wave 3 Prototype reference or lifecycle")}; return false; };
    FString WorldError;
    const auto* Respawn=LHWorld::FindEntrance(S.Session.SafeRespawn.Entrance);
    if (!LHWorld::FindEntrance(S.Character.ActiveEntrance) || !Respawn ||
        !S.Session.SafeRespawn.SafeTransform.Equals(Respawn->SafeTransform) ||
        !LHWorld::ValidateWorld(S.World,WorldError)) return Bad();
    // Hash compatibility does not establish reference closure. Compare spelling,
    // too: FName equality alone accepts case variants of canonical catalog IDs.
    auto Canonical=[](const FLHContentId& A,const FLHContentId& B) {
        return A.Value.ToString().Equals(B.Value.ToString(),ESearchCase::CaseSensitive);
    };
    for (const auto& Skill:S.Character.LearnedSkills)
        if (!LHServices::Catalog().ContainsByPredicate([&](const auto& Offer) {
            return Offer.Kind==ELHServiceKind::TrainSkill && Canonical(Offer.Subject,Skill.Skill);
        })) return Bad();
    for (const auto& Spell:S.Character.LearnedSpells) {
        const auto* Row=LHAbilities::Find(Spell);
        if (!Row || !Canonical(Row->Id,Spell) || !Spell.Value.ToString().StartsWith(TEXT("Spell."))) return Bad();
    }
    auto Items=[&](const TArray<FLHItemInstance>& Values) {
        for (const auto& Item:Values) { const auto* Row=LHItemData::Find(Item.Definition);
            if (!Row || !Canonical(Row->Character.Id,Item.Definition)) return false; }
        return true;
    };
    if (!Items(S.Character.Inventory)) return Bad();
    for (const auto& Area:S.World.Areas) {
        for (const auto& Object:Area.Objects) if (!Items(Object.RemainingItems)) return Bad();
        for (const auto& Corpse:Area.Corpses) if (!Items(Corpse.RemainingItems)) return Bad();
    }
    for (const auto& Cooldown:S.Session.Cooldowns) {
        const auto* Row=LHAbilities::Find(Cooldown.Ability);
        if (!Row || !Canonical(Row->Id,Cooldown.Ability) || !LHWorld::FindArea(Cooldown.Owner.Area) ||
            Cooldown.Owner.RunId!=S.World.RunId) return Bad();
        if (Cooldown.Owner.InstanceId==S.Header.CharacterId.Value) {
            if (Cooldown.Owner.Area.Content.Value.ToString()!=TEXT("Area.LighthavenTempleDistrict")) return Bad();
        } else {
            const auto* Area=LHWorld::FindArea(Cooldown.Owner.Area);
            if (!Area->Spawns.ContainsByPredicate([&](const auto& Spawn){return Spawn.SpawnId==Cooldown.Owner.InstanceId;})) return Bad();
        }
    }
    TSet<FGuid> ExpectedClaims;
    bool BalorkDefeated=false;
    for (const auto& Boss:S.World.Bosses) {
        const auto* Row=LHEnemyData::Find(Boss.Boss);
        FLHRewardId Expected; FLHSaveError Error;
        if (!Row || !Row->Reward.bBoss || !Canonical(Row->Id,Boss.Boss) ||
            !LHSave::BossUniqueRewardId(S.World.RunId,Boss.Boss,Expected,Error)) return Bad();
        const bool Retained=S.World.ClaimedUniqueRewards.ContainsByPredicate([&](const auto& R){return R.Value==Expected.Value;});
        if (Boss.bDefeated) {
            if (Boss.UniqueClaim.Value!=Expected.Value || !Retained) return Bad();
            ExpectedClaims.Add(Expected.Value);
        } else if (Boss.UniqueClaim.Value.IsValid() || Retained) return Bad();
        if (Boss.Boss.Value.ToString()==TEXT("Enemy.Balork")) BalorkDefeated=Boss.bDefeated;
    }
    for (const auto& Quest:S.World.Quests) {
        const FString Id=Quest.Quest.Value.ToString();
        if (Id==TEXT("Quest.SamaritanRats")) {
            FLHRewardId Expected; FLHSaveError Error;
            if (!LHSave::QuestTurnInRewardId(S.World.RunId,Quest.Quest,TEXT("TurnIn"),Expected,Error)) return Bad();
            const bool Retained=S.World.ClaimedUniqueRewards.ContainsByPredicate([&](const auto& R){return R.Value==Expected.Value;});
            if (Quest.bRewarded) {
                if (!Quest.bAccepted || !Quest.bCompleted || Quest.Stage.ToString()!=TEXT("TurnIn") ||
                    Quest.EligibleKillCount.Value<15 || Quest.TurnInClaim.Value!=Expected.Value || !Retained) return Bad();
                ExpectedClaims.Add(Expected.Value);
            } else if (Quest.bCompleted || Quest.TurnInClaim.Value.IsValid() || Retained) return Bad();
        } else if (Id==TEXT("Quest.BalorkReturn")) {
            if (!BalorkDefeated || !Quest.bAccepted || Quest.bRewarded || Quest.TurnInClaim.Value.IsValid() ||
                Quest.Stage.ToString()!=(Quest.bCompleted?TEXT("Complete"):TEXT("ReturnToChurch"))) return Bad();
        } else return Bad();
    }
    if (BalorkDefeated && !S.World.Quests.ContainsByPredicate([](const auto& Q){return Q.Quest.Value.ToString()==TEXT("Quest.BalorkReturn");})) return Bad();
    for (const auto& Claim:S.World.ClaimedUniqueRewards) if (!ExpectedClaims.Contains(Claim.Value)) return Bad();
    FLHCharacterAuthority Check;
    if (!Check.Initialize(LHWave2::PrototypeProfile(),FGuid::NewGuid(),123) || Check.Import(S)!=ELHCommandReason::None) return Bad();
    return true;
}
FLHCommandResult Busy() { FLHCommandResult R; R.Reason=ELHCommandReason::Busy; return R; }
}
FLHSaveCompatibility FLHWave2Session::Compatibility()
{
    FLHSaveCompatibility C; C.Ruleset=LHWave2::PrototypeProfile().Reference; C.ContentRevision=LHWave2::CatalogHash();
    C.MaxGrowthAwards=LHWave2::PrototypeProfile().Rules.Progression.Thresholds.Num()-1; C.ValidateReferences=LHWave2SessionPrivate::ValidateReferences; return C;
}
FLHWave2Session::FLHWave2Session(TSharedRef<FLHSaveStore> Store) : Saves(Store)
{
    EventHandle=Saves->Events.AddRaw(this,&FLHWave2Session::OnSave);
}
FLHWave2Session::~FLHWave2Session()
{
    if (Director.IsValid()) { Director->SettleKill=nullptr; Director->AdvanceRespawns=nullptr; Director->SetSimulationFrozen(true); }
    Saves->Events.Remove(EventHandle);
}
FLHCharacterAuthority* FLHWave2Session::Authority() const
{ return Owner.IsValid() ? &Owner->GetCharacterAuthority()->Authority() : nullptr; }
bool FLHWave2Session::Bind(ALHPlayerState* State, bool Fresh)
{
    if (!State || (Fresh && IsTransactionBlocked())) return false;
    // Capture the outgoing component before replacing its owner at a completed boundary.
    if (!Fresh && Owner.IsValid() && Owner.Get()!=State && HasCharacter() && !SyncResources()) return false;
    Owner=State;
    auto* A=Authority(); *A=FLHCharacterAuthority();
    if (!A->Initialize(LHWave2::PrototypeProfile(),FGuid::NewGuid(),123)) return false;
    if (!bWorldTravelFrozen) bTravel=false;
    bDeadAwaitingRespawn=HasCharacter() && Complete.Character.CurrentHealth.Value<=0;
    if (Fresh) { Complete={}; Message.Empty(); bEnterAfterSave=false; bDeadAwaitingRespawn=false; RuntimeAbilities.Reset(); }
    if (HasCharacter())
    {
        if (A->Import(Complete)!=ELHCommandReason::None || !InstallDerived(true)) { Message=TEXT("Restore failed; avatar remains unavailable."); return false; }
    }
    else { A->Export(Complete); State->GetCombatComponent()->RestoreLightRemainingSeconds(0); }
    return true;
}
bool FLHWave2Session::InstallDerived(bool RestoreEffects)
{
    auto* A=Authority(); if (!A || !Owner.IsValid()) return false;
    const auto Stats=A->Stats(); if (!Stats.Diagnostic.IsAccepted()) return false;
    auto* C=Owner->GetCombatComponent();
    C->SetNumericAttributeBase(ULHAttributeSet::GetMaxHealthAttribute(),Stats.Value.MaxHealth);
    C->SetNumericAttributeBase(ULHAttributeSet::GetMaxManaAttribute(),Stats.Value.MaxMana);
    C->SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(),Complete.Character.CurrentHealth.Value);
    C->SetNumericAttributeBase(ULHAttributeSet::GetManaAttribute(),Complete.Character.CurrentMana.Value);
    C->SetNumericAttributeBase(ULHAttributeSet::GetAccuracyAttribute(),Stats.Value.Accuracy);
    C->SetNumericAttributeBase(ULHAttributeSet::GetAvoidanceAttribute(),Stats.Value.Avoidance);
    C->SetNumericAttributeBase(ULHAttributeSet::GetDamageBonusAttribute(),Stats.Value.DamageBonus);
    C->SetNumericAttributeBase(ULHAttributeSet::GetArmorAttribute(),Stats.Value.Armor);
    C->SetNumericAttributeBase(ULHAttributeSet::GetResistanceAttribute(),0);
    if (!RestoreEffects) return true;
    // Destination checkpoints may retain the source area's self identity. Validate
    // against that identity before recapturing under the installed player identity.
    auto SavedOwner=PlayerEntity();
    if (Complete.Session.DurableEffects.Num()==1) SavedOwner.Area=Complete.Session.DurableEffects[0].Owner.Area;
    if (!LHAbilities::RestoreLightEffect(*C,SavedOwner,Complete.Session)) return false;
    auto LightOwner=PlayerEntity(); LightOwner.Area=Complete.Character.ActiveEntrance.Area;
    if (!LHAbilities::CaptureLightEffect(*C,LightOwner,Complete.Session)) return false;
    return A->Import(Complete)==ELHCommandReason::None;
}
FLHSaveSnapshot FLHWave2Session::Snapshot() const { return Complete; }
TArray<FLHUIProfile> FLHWave2Session::Profiles() const
{
    if (!bProfilesDirty) return CachedProfiles;
    bProfilesDirty=false;
    TArray<FLHUIProfile> Out;
    for (const auto& P:Saves->Enumerate(Compatibility()))
    {
        FLHUIProfile V; V.Id=P.Character; V.Name=P.DisplayName.IsEmpty()?P.Character.Value.ToString():P.DisplayName;
        V.bCanContinue=P.bReadable; V.bRequiresRecoveryAcknowledgment=P.bRecovered;
        V.Status=P.bRecovered?TEXT("Recovered earlier generation; acknowledgment required"):P.bReadable?TEXT("Prototype — readable"):TEXT("Unreadable: ")+P.Error.Detail;
        if (P.Error.Reason==ELHSaveReason::Busy) V.Status=TEXT("Write in flight");
        V.Status=P.Character.Value.ToString()+TEXT(" | ")+V.Status;
        Out.Add(V);
        if (!P.bReadable && P.Error.Reason!=ELHSaveReason::Busy && !ReportedUnreadable.Contains(P.Character.Value))
        {
            ReportedUnreadable.Add(P.Character.Value);
            FLHSaveSnapshot Ignored; FLHSaveError Error;
            Saves->Load(P.Character,Compatibility(),Ignored,Error); // W2-01 publishes its concrete Unreadable event.
        }
    }
    CachedProfiles=Out; return Out;
}
TArray<FLHContentId> FLHWave2Session::AppearanceCatalog() const
{
    TArray<FLHContentId> R;
    for (const TCHAR* S:{TEXT("Presentation.Player.Body.A"),TEXT("Presentation.Player.Body.B"),TEXT("Presentation.Player.Hair.Cropped"),TEXT("Presentation.Player.Hair.Tied"),TEXT("Presentation.Player.Skin.LightWarm"),TEXT("Presentation.Player.Skin.MediumWarm"),TEXT("Presentation.Player.Skin.DeepWarm"),TEXT("Presentation.Player.Outfit.StarterLinen")})
    { FLHContentId I; I.Value=S; R.Add(I); } return R;
}
TArray<FLHUIQuestion> FLHWave2Session::QuestionCatalog() const
{
    TArray<FLHUIQuestion> R; for (auto A:LHWave2::PrototypeAnswers())
    { FLHUIQuestion Q; Q.Id=A.Question; Q.Prompt=A.Question.Value.ToString()+TEXT(" (Prototype)"); Q.Answers.Add(A.Answer); R.Add(Q); } return R;
}
FLHUICreationPreview FLHWave2Session::Preview(const FString& Name,const TArray<FLHContentId>& Appearance,const TArray<FLHQuestionAnswer>& Answers,bool)
{
    FLHUICreationPreview R;
    for (const TCHAR* Category:{TEXT("Body"),TEXT("Hair"),TEXT("Skin"),TEXT("Outfit")})
    {
        const FString Prefix=FString(TEXT("Presentation.Player."))+Category+TEXT(".");
        if (!Appearance.ContainsByPredicate([&](const FLHContentId& Id){return Id.Value.ToString().StartsWith(Prefix);}))
            R.FieldErrors.Add(FName(Category),TEXT("Select an appearance choice."));
    }
    if (!R.FieldErrors.IsEmpty()) return R;
    if (IsTransactionBlocked() || !Authority()) return R;
    FLHCharacterPreview V; if (Authority()->Preview(Answers,V)!=ELHCommandReason::None) return R;
    // Validate on a detached copy. Review cannot publish a command or save.
    auto Copy=*Authority(); FLHCreateCharacterRequest Q; Q.Request.Epoch=Complete.Session.RequestEpoch;
    Q.Request.Value=FGuid(1,2,3,4); Q.DisplayName=Name; Q.AppearanceIds=Appearance; Q.Creation=V.Creation; Q.PreviewToken=V.Token;
    R.bLegal=Copy.Execute(Q).Disposition==ELHCommandDisposition::Accepted; R.Token=V.Token; R.Record=V.Creation; return R;
}
FLHUIIntentReview FLHWave2Session::ReviewAllocation(const FLHAttributeBlock& Points) const
{
    FLHUIIntentReview R; if (IsTransactionBlocked() || !Authority() || !HasCharacter()) return R;
    auto Copy=*Authority(); FLHAllocateAttributePointsRequest Q; Q.Request.Epoch=Complete.Session.RequestEpoch;
    Q.Request.Value=FGuid(0xffffffff,0xffffffff,0xffffffff,1); Q.Points=Points;
    R.bLegal=Copy.Execute(Q).Disposition==ELHCommandDisposition::Accepted;
    R.Summary=R.bLegal?TEXT("Legal Prototype allocation"):TEXT("Allocation rejected by character authority"); return R;
}
FLHUIIntentReview FLHWave2Session::ReviewEquipment(const FLHEntityId& Item,ELHEquipmentSlot Slot,bool Undo) const
{
    FLHUIIntentReview R; if (IsTransactionBlocked() || !Authority() || !HasCharacter()) return R;
    auto Copy=*Authority(); FLHEquipItemRequest Q; Q.Request.Epoch=Complete.Session.RequestEpoch;
    Q.Request.Value=FGuid(0xffffffff,0xffffffff,0xffffffff,2); Q.Item=Item; Q.Slot=Slot; Q.bUnequip=Undo;
    R.bLegal=Copy.Execute(Q).Disposition==ELHCommandDisposition::Accepted;
    R.Summary=R.bLegal?TEXT("Legal Prototype equipment change"):TEXT("Equipment rejected by character authority"); return R;
}
void FLHWave2Session::Published(const FLHCommandResult& R,bool Creation)
{
    if (R.Disposition!=ELHCommandDisposition::Accepted || R.bReplay) return;
    Authority()->Export(Complete);
    if (Creation)
    {
        Complete.Header.BuildId=TEXT("Wave4.G4.Prototype.v1"); Complete.Header.ContentRevision=LHWave2::CatalogHash();
        Complete.Header.ChecksumAlgorithm=TEXT("SHA256"); Complete.Header.PayloadCodec=TEXT("LHCanonicalBinary1");
        FLHAreaRecord Area; Area.Area.Content.Value=TEXT("Area.LighthavenTempleDistrict"); Complete.World.Areas.Add(Area);
        Complete.Character.ActiveEntrance.Area=Area.Area; Complete.Character.ActiveEntrance.LocalId=TEXT("Temple.SafeSpawn");
        Complete.Session.SafeRespawn.Entrance=Complete.Character.ActiveEntrance;
        Complete.Session.SafeRespawn.TransformResolution=ELHValueResolution::Resolved;
        Complete.Session.SafeRespawn.SafeTransform=LHWorld::FindEntrance(Complete.Character.ActiveEntrance)->SafeTransform;
        Complete.Session.EffectPolicy=ELHEffectSavePolicy::CompletedActionBoundaryOnly;
        Complete.Session.ManaRegenFractionalSeconds=LHWave2::PrototypeNumber(0);
        for (const TCHAR* Id : {TEXT("Skill.Attack"),TEXT("Skill.Dodge")})
        { FLHLearnedSkill Skill; Skill.Skill.Value=Id; Skill.TrainedValue=LHWave2::PrototypeInteger(10); Complete.Character.LearnedSkills.Add(Skill); }
        for (const auto& Item : Complete.Character.Inventory)
        {
            const auto Profile=LHWave2::PrototypeProfile();
            const auto* Def=Profile.Items.FindByPredicate([&](const auto& D){ return D.Id.Value==Item.Definition.Value; });
            if (Def && Def->Slot!=ELHEquipmentSlot::Unspecified)
            { FLHEquipmentBinding Binding; Binding.Slot=Def->Slot; Binding.Item=Item.Id; Complete.Character.Equipment.Add(Binding); }
        }
        for (const TCHAR* Id : {TEXT("RNG.Combat"),TEXT("RNG.Loot")})
        {
            FLHRngState Rng; Rng.StreamId=Id; Rng.Algorithm=TEXT("UE.FRandomStream"); Rng.AlgorithmRevision=1;
            const uint32 Seed=GetTypeHash(FGuid::NewGuid());
            for (int32 B=0; B<4; ++B) Rng.State.Add(uint8(Seed>>(B*8)));
            Complete.Session.GameplayRng.Add(Rng);
        }
        // Import the newly completed world fields before any further command exports.
        Authority()->Import(Complete); bEnterAfterSave=true;
    }
    InstallDerived(); bSaveQueued=true; if (!bDurabilityError) Message=TEXT("Saving completed action…");
}
FLHCommandResult FLHWave2Session::Execute(const FLHCreateCharacterRequest& Q)
{ if (IsTransactionBlocked() || !Authority()) return LHWave2SessionPrivate::Busy(); auto R=Authority()->Execute(Q); Published(R,true); return R; }
FLHCommandResult FLHWave2Session::Execute(const FLHAllocateAttributePointsRequest& Q)
{ if (IsTransactionBlocked() || !Authority() || !SyncResources()) return LHWave2SessionPrivate::Busy(); auto R=Authority()->Execute(Q); Published(R,false); return R; }
FLHCommandResult FLHWave2Session::Execute(const FLHEquipItemRequest& Q)
{ if (IsTransactionBlocked() || !Authority() || !SyncResources()) return LHWave2SessionPrivate::Busy(); auto R=Authority()->Execute(Q); Published(R,false); return R; }
void FLHWave2Session::Flush()
{
    if (!bSaveQueued || bAwaitingSave || bWorldTravelFrozen) return;
    if (HasPendingCombat()) return;
    if (HasCharacter() && !SyncResources()) return;
    bSaveQueued=false; bAwaitingSave=true; AwaitedSequence=Complete.Header.TransactionSequence;
    FLHSaveError E;
    if (!Saves->RequestSave(Complete,Compatibility(),true,E)) { bAwaitingSave=false; bDurabilityError=true; Message=TEXT("Save failed: ")+E.Detail; }
}
void FLHWave2Session::OnSave(const FLHSaveEvent& E)
{
    bProfilesDirty=true;
    UE_LOG(LogTemp,Display,TEXT("LH Wave2 save event kind=%d character=%s sequence=%lld detail=%s"),
        int32(E.Kind),*E.Character.Value.ToString(),E.Sequence,*E.Error.Detail);
    if (E.Kind==ELHSaveEventKind::Unreadable || E.Kind==ELHSaveEventKind::Recovered)
    { if (!bDurabilityError) Message=(E.Kind==ELHSaveEventKind::Recovered?TEXT("Recovery: "):TEXT("Unreadable: "))+E.Error.Detail; return; }
    // W2-01 can advance the durable envelope sequence on retry after a failed readback
    // of an otherwise valid write. One owner/in-flight snapshot and command suppression
    // identify the operation; synchronise its returned high-water value before new commands.
    if (!bAwaitingSave || E.Character.Value!=Complete.Header.CharacterId.Value || E.Sequence<AwaitedSequence) return;
    bAwaitingSave=false;
    if (E.Kind==ELHSaveEventKind::Failed) { bDurabilityError=true; Message=TEXT("Save failed: ")+E.Error.Detail; return; }
    Complete.Header.TransactionSequence=FMath::Max(Complete.Header.TransactionSequence,E.Sequence);
    if (Authority() && Authority()->Import(Complete)!=ELHCommandReason::None)
    { bDurabilityError=true; Message=TEXT("Durable sequence synchronization failed; transition blocked."); return; }
    bDurabilityError=false; Message.Empty();
    if (bSaveQueued) return; // A newer completed boundary must become durable before a transition.
    if (bExit) { bExit=false; if (Exit) Exit(); }
    else if (bEnterAfterSave) { bEnterAfterSave=false; bTravel=true; if (Travel) Travel(); }
}
FString FLHWave2Session::Continue(FLHCharacterId Id,bool Ack)
{
    if (IsTransactionBlocked()) return TEXT("Save or travel is pending.");
    if (HasCharacter() && Saves->IsDirty(Complete.Header.CharacterId))
    {
        bAwaitingSave=true; AwaitedSequence=Complete.Header.TransactionSequence; FLHSaveError E;
        if (!Saves->Retry(Complete.Header.CharacterId,E)) { bAwaitingSave=false; bDurabilityError=true; Message=TEXT("Save retry failed: ")+E.Detail; }
        return Message;
    }
    const auto ProfilesNow=Profiles(); const auto* P=ProfilesNow.FindByPredicate([&](const auto& V){return V.Id.Value==Id.Value;});
    if (!P || !P->bCanContinue) return TEXT("Unreadable: selected save cannot continue.");
    if (P->bRequiresRecoveryAcknowledgment && !Ack) return TEXT("Recovery acknowledgment required.");
    FLHSaveSnapshot Loaded; FLHSaveError E;
    if (!Saves->Load(Id,Compatibility(),Loaded,E)) { Message=TEXT("Unreadable: ")+E.Detail; return Message; }
    if (!Authority() || Authority()->Import(Loaded)!=ELHCommandReason::None) return TEXT("Character restore rejected.");
    Complete=Loaded; if (!InstallDerived(true)) return TEXT("Derived restore rejected.");
    Message.Empty(); bTravel=true; if (Travel) Travel(); return {};
}
FString FLHWave2Session::RequestExit()
{
    if (bContentUnavailable) { if (Exit) Exit(); return {}; }
    if (bContentUnavailable || bTravel || bWorldTravelFrozen || bSaveQueued || bAwaitingSave || HasPendingCombat()) return TEXT("Wait for completed-action durability.");
    if (HasCharacter() && Saves->IsDirty(Complete.Header.CharacterId))
    { bExit=true; return Continue(Complete.Header.CharacterId,false); }
    if (HasCharacter() && bInGameplay)
    {
        if (!SyncResources()) return TEXT("Resource capture failed.");
        bExit=true; bSaveQueued=true; return {};
    }
    if (Exit) Exit(); return {};
}

bool FLHWave2Session::BeginCreation()
{
    if (bInGameplay || IsTransactionBlocked() || (HasCharacter() && Saves->IsDirty(Complete.Header.CharacterId))) return false;
    return Bind(Owner.Get(),true);
}
FString FLHWave2Session::RetryPersistence()
{
    if (bWorldTravelFrozen && RetryWorldTravel)
    { FString Error; if (!RetryWorldTravel(Error)) { if (!bDurabilityError) Message=Error; }
      else if (!bDurabilityError && !HasUnsavedChanges()) Message.Empty(); return Message; }
    if (bTravel || bSaveQueued || bAwaitingSave || bContentUnavailable) return TEXT("Save is pending.");
    if (!HasCharacter() || !Saves->IsDirty(Complete.Header.CharacterId)) return Message;
    bAwaitingSave=true; AwaitedSequence=Complete.Header.TransactionSequence; FLHSaveError E;
    if (!Saves->Retry(Complete.Header.CharacterId,E)) { bAwaitingSave=false; bDurabilityError=true; Message=TEXT("Save retry failed: ")+E.Detail; }
    return Message;
}

void FLHWave2Session::ClearSelectionError()
{
    // Save failures remain visible until durability succeeds. Profile errors live on each row.
    if (Message.StartsWith(TEXT("Unreadable:")) || Message.StartsWith(TEXT("Recovery:"))) Message.Empty();
}

void FLHWave2Session::FreezeWorldTravel(bool Frozen)
{
    bWorldTravelFrozen=Frozen; bTravel=Frozen;
    if (Director.IsValid()) Director->SetSimulationFrozen(Frozen || bGameplayPaused || bDeadAwaitingRespawn);
    if (Owner.IsValid()) Owner->GetCombatComponent()->SetRecoveryMenuPaused(Frozen || bGameplayPaused || bDeadAwaitingRespawn);
}
bool FLHWave2Session::CaptureTravel(FLHSaveSnapshot& Out,FString& Error)
{
    if (!HasCharacter() || bSaveQueued || bAwaitingSave || !Authority() ||
        HasPendingCombat())
    { Error=TEXT("Travel requires a completed action and durable session"); return false; }
    if (!SyncResources()) { Error=TEXT("Resource capture failed"); return false; }
    Out=Complete; return true;
}
bool FLHWave2Session::InstallTravel(const FLHSaveSnapshot& Snapshot)
{
    if (!Authority()) return false;
    auto Next=Snapshot;
    // Light retains its area-scoped effect identity. Only remap the allowlisted
    // self effect belonging to this character/run; validation still rejects others.
    for (auto& Effect:Next.Session.DurableEffects)
        if (Effect.Effect.Value==TEXT("Effect.SpellLight") &&
            Effect.Owner.RunId==Next.World.RunId && Effect.Owner.InstanceId==Next.Header.CharacterId.Value &&
            Effect.Source.RunId==Effect.Owner.RunId && Effect.Source.InstanceId==Effect.Owner.InstanceId &&
            LHWorld::SameArea(Effect.Source.Area,Effect.Owner.Area))
        { Effect.Owner.Area=Next.Character.ActiveEntrance.Area; Effect.Source.Area=Effect.Owner.Area; }
    auto Check=*Authority(); if (Check.Import(Next)!=ELHCommandReason::None) return false;
    *Authority()=MoveTemp(Check); Complete=MoveTemp(Next);
    return InstallDerived(true);
}
FLHCommandResult FLHWave2Session::Execute(const FLHRequestTravelRequest& Q)
{
    FLHCommandResult R; R.Request=Q.Request;
    if (IsTransactionBlocked() || !RequestWorldTravel) { R.Reason=ELHCommandReason::Busy; return R; }
    FString Error;
    ALHPortal* Portal=nullptr;
    if (Owner.IsValid()) for (TActorIterator<ALHPortal> It(Owner->GetWorld());It;++It)
        if (It->Materialize(Complete.World.RunId).InstanceId==Q.Portal.InstanceId &&
            Q.Portal.RunId==Complete.World.RunId && LHWorld::SameArea(Q.Portal.Area,Complete.Character.ActiveEntrance.Area))
        { if (Portal) { R.Reason=ELHCommandReason::InvalidRequest; return R; } Portal=*It; }
    if (!Portal) { R.Reason=ELHCommandReason::NotFound; return R; }
    if (!Spatial(Portal,250)) { R.Reason=ELHCommandReason::OutOfRange; return R; }
    if (RequestWorldTravel(Q,Error)) { R.Reason=ELHCommandReason::None; R.Disposition=ELHCommandDisposition::Accepted; if (!bDurabilityError && !HasUnsavedChanges()) Message.Empty(); }
    else { R.Reason=ELHCommandReason::InvalidRequest; if (!bDurabilityError) Message=Error; }
    return R;
}

void FLHWave2Session::AbortGameplayArrival(const FString& Error)
{
    FreezeWorldTravel(false); bInGameplay=false;
    if (!bDurabilityError) Message=TEXT("Arrival refused; save retained: ")+Error;
}

FString FLHWave2Session::DerivedSummary() const
{
    if (!Authority() || !HasCharacter()) return ILHUIReadOwner::DerivedSummary();
    const auto Stats=Authority()->Stats();
    if (!Stats.Diagnostic.IsAccepted()) return ILHUIReadOwner::DerivedSummary();
    const auto& V=Stats.Value;
    const auto& B=Complete.Character.BaseAttributes;
    FString Summary=FString::Printf(TEXT("Effective attributes (Prototype): Strength %lld | Endurance %lld | Agility %lld | Intelligence %lld | Wisdom %lld\nGear attribute effects (Prototype): Strength %+lld | Endurance %+lld | Agility %+lld | Intelligence %+lld | Wisdom %+lld\nEffective stats (Prototype): Max HP %g | Max MP %g | Accuracy %g | Avoidance %g | Damage bonus %g | Armor %g | Capacity unlimited (deferred by owner decision)\nTemporary effects: Light %g active seconds"),
        V.Effective.Strength,V.Effective.Endurance,V.Effective.Agility,V.Effective.Intelligence,V.Effective.Wisdom,
        V.Effective.Strength-B.Strength.Value,V.Effective.Endurance-B.Endurance.Value,V.Effective.Agility-B.Agility.Value,V.Effective.Intelligence-B.Intelligence.Value,V.Effective.Wisdom-B.Wisdom.Value,
        V.MaxHealth,V.MaxMana,V.Accuracy,V.Avoidance,V.DamageBonus,V.Armor,Owner->GetCombatComponent()->GetLightRemainingSeconds());
    const auto Profile=LHWave2::PrototypeProfile();
    for (const auto& Binding : Complete.Character.Equipment)
    {
        const auto* Item=Complete.Character.Inventory.FindByPredicate([&](const auto& I) { return I.Id.InstanceId==Binding.Item.InstanceId; });
        if (!Item) continue;
        const auto* Definition=Profile.Items.FindByPredicate([&](const auto& D) { return D.Id.Value==Item->Definition.Value; });
        if (!Definition) continue;
        const auto& M=Definition->Modifier;
        Summary += TEXT("\n")+Definition->Id.Value.ToString()+TEXT(" gear modifiers: HP ")+FLHUIPresenter::Format(M.Health)
            +TEXT(" | MP ")+FLHUIPresenter::Format(M.Mana)+TEXT(" | Accuracy ")+FLHUIPresenter::Format(M.Accuracy)
            +TEXT(" | Avoidance ")+FLHUIPresenter::Format(M.Avoidance)+TEXT(" | Damage ")+FLHUIPresenter::Format(M.DamageBonus)
            +TEXT(" | Armor ")+FLHUIPresenter::Format(M.Armor)+TEXT(" | Capacity unlimited (deferred by owner decision)");
    }
    return Summary;
}

void FLHWave2Session::CompleteGameplayArrival()
{
    // A successful arrival resolves travel feedback, never an outstanding save failure.
    if (!bDurabilityError && !HasUnsavedChanges()) Message.Empty();
}
