#include "Framework/LHWave2Session.h"
#include "Framework/LHWave2Profile.h"
#include "Framework/LHPlayerState.h"
#include "Abilities/LHCombatComponent.h"
#include "Abilities/LHAttributeSet.h"
#include "World/LHAreaRegistry.h"
#include "World/LHAreaStateSubsystem.h"
#include "Abilities/LHResourceRecovery.h"
#include "AI/LHEncounterDirector.h"
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
    Owner=State;
    auto* A=Authority(); *A=FLHCharacterAuthority();
    if (!A->Initialize(LHWave2::PrototypeProfile(),FGuid::NewGuid(),123)) return false;
    if (!bWorldTravelFrozen) bTravel=false;
    bDeadAwaitingRespawn=HasCharacter() && Complete.Character.CurrentHealth.Value<=0;
    if (Fresh) { Complete={}; Message.Empty(); bEnterAfterSave=false; bDeadAwaitingRespawn=false; RuntimeAbilities.Reset(); }
    if (HasCharacter())
    {
        if (A->Import(Complete)!=ELHCommandReason::None || !InstallDerived()) { Message=TEXT("Restore failed; avatar remains unavailable."); return false; }
    }
    else A->Export(Complete);
    return true;
}
bool FLHWave2Session::InstallDerived()
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
    return true;
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
    InstallDerived(); bSaveQueued=true; Message=TEXT("Saving completed action…");
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
    if (!Saves->RequestSave(Complete,Compatibility(),true,E)) { bAwaitingSave=false; Message=TEXT("Save failed: ")+E.Detail; }
}
void FLHWave2Session::OnSave(const FLHSaveEvent& E)
{
    bProfilesDirty=true;
    UE_LOG(LogTemp,Display,TEXT("LH Wave2 save event kind=%d character=%s sequence=%lld detail=%s"),
        int32(E.Kind),*E.Character.Value.ToString(),E.Sequence,*E.Error.Detail);
    if (E.Kind==ELHSaveEventKind::Unreadable || E.Kind==ELHSaveEventKind::Recovered)
    { Message=(E.Kind==ELHSaveEventKind::Recovered?TEXT("Recovery: "):TEXT("Unreadable: "))+E.Error.Detail; return; }
    // W2-01 can advance the durable envelope sequence on retry after a failed readback
    // of an otherwise valid write. One owner/in-flight snapshot and command suppression
    // identify the operation; synchronise its returned high-water value before new commands.
    if (!bAwaitingSave || E.Character.Value!=Complete.Header.CharacterId.Value || E.Sequence<AwaitedSequence) return;
    bAwaitingSave=false;
    if (E.Kind==ELHSaveEventKind::Failed) { Message=TEXT("Save failed: ")+E.Error.Detail; return; }
    Complete.Header.TransactionSequence=FMath::Max(Complete.Header.TransactionSequence,E.Sequence);
    if (Authority() && Authority()->Import(Complete)!=ELHCommandReason::None)
    { Message=TEXT("Durable sequence synchronization failed; transition blocked."); return; }
    Message.Empty();
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
        if (!Saves->Retry(Complete.Header.CharacterId,E)) { bAwaitingSave=false; Message=TEXT("Save retry failed: ")+E.Detail; }
        return Message;
    }
    const auto ProfilesNow=Profiles(); const auto* P=ProfilesNow.FindByPredicate([&](const auto& V){return V.Id.Value==Id.Value;});
    if (!P || !P->bCanContinue) return TEXT("Unreadable: selected save cannot continue.");
    if (P->bRequiresRecoveryAcknowledgment && !Ack) return TEXT("Recovery acknowledgment required.");
    FLHSaveSnapshot Loaded; FLHSaveError E;
    if (!Saves->Load(Id,Compatibility(),Loaded,E)) { Message=TEXT("Unreadable: ")+E.Detail; return Message; }
    if (!Authority() || Authority()->Import(Loaded)!=ELHCommandReason::None) return TEXT("Character restore rejected.");
    Complete=Loaded; if (!InstallDerived()) return TEXT("Derived restore rejected.");
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
    { FString Error; RetryWorldTravel(Error); Message=Error; return Message; }
    if (bTravel || bSaveQueued || bAwaitingSave || bContentUnavailable) return TEXT("Save is pending.");
    if (!HasCharacter() || !Saves->IsDirty(Complete.Header.CharacterId)) return Message;
    bAwaitingSave=true; AwaitedSequence=Complete.Header.TransactionSequence; FLHSaveError E;
    if (!Saves->Retry(Complete.Header.CharacterId,E)) { bAwaitingSave=false; Message=TEXT("Save retry failed: ")+E.Detail; }
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
    auto Check=*Authority(); if (Check.Import(Snapshot)!=ELHCommandReason::None) return false;
    *Authority()=MoveTemp(Check); Complete=Snapshot;
    return InstallDerived();
}
FLHCommandResult FLHWave2Session::Execute(const FLHRequestTravelRequest& Q)
{
    if (IsTransactionBlocked() || !RequestWorldTravel) return LHWave2SessionPrivate::Busy();
    FString Error; FLHCommandResult R;
    if (RequestWorldTravel(Q,Error)) R.Disposition=ELHCommandDisposition::Accepted;
    else { R.Reason=ELHCommandReason::InvalidRequest; Message=Error; }
    return R;
}

void FLHWave2Session::AbortGameplayArrival(const FString& Error)
{
    FreezeWorldTravel(false); bInGameplay=false;
    Message=TEXT("Arrival refused; save retained: ")+Error;
}

FString FLHWave2Session::DerivedSummary() const
{
    if (!Authority() || !HasCharacter()) return ILHUIReadOwner::DerivedSummary();
    const auto Stats=Authority()->Stats();
    if (!Stats.Diagnostic.IsAccepted()) return ILHUIReadOwner::DerivedSummary();
    const auto& V=Stats.Value;
    const auto& B=Complete.Character.BaseAttributes;
    FString Summary=FString::Printf(TEXT("Effective attributes (Prototype): Strength %lld | Endurance %lld | Agility %lld | Intelligence %lld | Wisdom %lld\nGear attribute effects (Prototype): Strength %+lld | Endurance %+lld | Agility %+lld | Intelligence %+lld | Wisdom %+lld\nEffective stats (Prototype): Max HP %g | Max MP %g | Accuracy %g | Avoidance %g | Damage bonus %g | Armor %g | Capacity %g\nTemporary effects: — (not available in this prototype)"),
        V.Effective.Strength,V.Effective.Endurance,V.Effective.Agility,V.Effective.Intelligence,V.Effective.Wisdom,
        V.Effective.Strength-B.Strength.Value,V.Effective.Endurance-B.Endurance.Value,V.Effective.Agility-B.Agility.Value,V.Effective.Intelligence-B.Intelligence.Value,V.Effective.Wisdom-B.Wisdom.Value,
        V.MaxHealth,V.MaxMana,V.Accuracy,V.Avoidance,V.DamageBonus,V.Armor,V.Capacity);
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
            +TEXT(" | Armor ")+FLHUIPresenter::Format(M.Armor)+TEXT(" | Capacity ")+FLHUIPresenter::Format(M.Capacity);
    }
    return Summary;
}
