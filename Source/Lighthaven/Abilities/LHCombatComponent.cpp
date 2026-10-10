#include "Abilities/LHCombatComponent.h"
#include "Abilities/LHAttributeSet.h"
#include "Abilities/LHBasicAttackAbility.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Framework/LHEnemyCharacter.h"
#include "EngineUtils.h"
#include <limits>
namespace LHCombatComponentPrivate
{
bool Ready(const FLHNumber& N)
{
    return N.Resolution == ELHValueResolution::Resolved && N.Provenance.Status != ELHProvenanceStatus::Missing &&
        N.Provenance.Status != ELHProvenanceStatus::Disputed && FMath::IsFinite(N.Value) && N.Value >= 0 &&
        N.Value <= std::numeric_limits<float>::max();
}
bool LiveActor(const AActor* Actor) { return IsValid(Actor) && !Actor->IsActorBeingDestroyed(); }
}
ULHCombatComponent::ULHCombatComponent()
{
    SetIsReplicatedByDefault(false);
    PrimaryComponentTick.bCanEverTick=true;
    Attributes = CreateDefaultSubobject<ULHAttributeSet>(TEXT("CombatAttributes"));
    AddAttributeSetSubobject(Attributes.Get());
}
void ULHCombatComponent::InitializeCombatActorInfo(AActor* Owner, AActor* Avatar)
{
    TGuardValue<bool> Transition(bLifecycleTransition,true);
    CancelAttack(ELHAttackCancelReason::AvatarCleared);
    ++LifeRevision;
    InitAbilityActorInfo(Owner, Avatar);
    bDeathPublished = !IsAlive();
    AppliedHits.Reset();
    if (!AttackHandle.IsValid()) AttackHandle = GiveAbility(FGameplayAbilitySpec(ULHBasicAttackAbility::StaticClass(), 1));
}
void ULHCombatComponent::ClearCombatAvatar()
{
    TGuardValue<bool> Transition(bLifecycleTransition,true);
    CancelAttack(ELHAttackCancelReason::AvatarCleared);
    // Light survives avatar detachment during travel; capture/restore is session-owned.
    ClearActorInfo();
}
void ULHCombatComponent::ConfigureAttack(const FLHBasicAttackConfig& Config, const LH::Rules::FRequirementInput& Requirements)
{
    if (bPending || bPublishingCommit) return;
    Attack = Config;
    RequirementInput = Requirements;
}
bool ULHCombatComponent::IsAlive() const
{
    return LHCombatComponentPrivate::LiveActor(GetAvatarActor()) && FMath::IsFinite(Attributes->GetHealth()) && Attributes->GetHealth() > 0;
}
bool ULHCombatComponent::InRangeAndSight(const ULHCombatComponent* Target) const
{
    if (!Target || !LHCombatComponentPrivate::LiveActor(GetAvatarActor()) || !LHCombatComponentPrivate::LiveActor(Target->GetAvatarActor()) || (GetAvatarActor() == Target->GetAvatarActor() && !Attack.bHeal && !Attack.bLight)) return false;
    UWorld* World = GetAvatarActor()->GetWorld();
    if (!World || World != Target->GetAvatarActor()->GetWorld()) return false;
    const FVector Start = GetAvatarActor()->GetActorLocation(), End = Target->GetAvatarActor()->GetActorLocation();
    const double Distance = FVector::Dist(Start, End);
    if (!FMath::IsFinite(Distance) || Distance > Attack.RangeCm.Value) return false;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(LHAttackSight), false, GetAvatarActor());
    Params.AddIgnoredActor(Target->GetAvatarActor());
    // Corpse Visibility remains available to cursor/loot selection. Attack sight
    // excludes retained dead lives, including corpses co-located with a respawn.
    for (TActorIterator<ALHEnemyCharacter> It(World); It; ++It)
        if (It->IsCorpse()) Params.AddIgnoredActor(*It);
    return !World->LineTraceTestByChannel(Start, End, ECC_Visibility, Params);
}
ELHCommandReason ULHCombatComponent::ValidateAttack(ULHCombatComponent* Target) const
{
    if (!IsValid(Target) || !LHCombatComponentPrivate::LiveActor(Target->GetAvatarActor())) return ELHCommandReason::NotFound;
    if (!IsAlive() || !Target->IsAlive()) return ELHCommandReason::InvalidLifeState;
    if (bLifecycleTransition) return ELHCommandReason::Busy;
    if (bPending || bPublishingCommit) return ELHCommandReason::ActiveAction;
    if (GetRemainingCooldown() > 0) return ELHCommandReason::Cooldown;
    if (!bRandomReady || !LHCombatComponentPrivate::Ready(Attack.ManaCost) || !LHCombatComponentPrivate::Ready(Attack.CooldownSeconds) || !LHCombatComponentPrivate::Ready(Attack.ImpactSeconds) ||
        !LHCombatComponentPrivate::Ready(Attack.RangeCm) || !LHCombatComponentPrivate::Ready(Attack.WeaponMinimum) || !LHCombatComponentPrivate::Ready(Attack.WeaponMaximum)) return ELHCommandReason::UnresolvedRules;
    if (Attack.bLight && (!FMath::IsFinite(Attack.LightDuration) || Attack.LightDuration<=0 || Attack.LightDuration>600)) return ELHCommandReason::UnresolvedRules;
    const auto Eligibility = LH::Rules::CheckRequirements(Attack.RequirementPolicy, Attack.Eligibility, RequirementInput);
    if (!Eligibility.Diagnostic.IsAccepted()) return Eligibility.Diagnostic.Reason == LH::Rules::EReason::Ineligible ? ELHCommandReason::Ineligible : ELHCommandReason::UnresolvedRules;
    // Validate formula parameters without consuming the combat random stream or mutating state.
    LH::Rules::FCombatInput Input;
    Input.WeaponMinimum = Attack.WeaponMinimum.Value; Input.WeaponMaximum = Attack.WeaponMaximum.Value;
    Input.bSpell=Attack.bSpell; Input.QuiverBonus=Attack.QuiverBonus;
    Input.Accuracy = Attributes->GetAccuracy(); Input.DamageBonus = Attributes->GetDamageBonus();
    Input.Avoidance = Target->Attributes->GetAvoidance(); Input.Armor = Target->Attributes->GetArmor(); Input.Resistance = Target->Attributes->GetResistance();
    if (!LH::Rules::ResolveCombat(Attack.Combat, Input).Diagnostic.IsAccepted()) return ELHCommandReason::UnresolvedRules;
    if (!FMath::IsFinite(Attributes->GetMana()) || Attributes->GetMana() < Attack.ManaCost.Value) return ELHCommandReason::InsufficientMana;
    if (!InRangeAndSight(Target)) return ELHCommandReason::OutOfRange;
    return ELHCommandReason::None;
}
ELHCommandReason ULHCombatComponent::RequestBasicAttack(ULHCombatComponent* Target)
{
    if (!GetWorld() || GetWorld()->GetNetMode() != NM_Standalone) return ELHCommandReason::InvalidRequest;
    const auto Reason = ValidateAttack(Target);
    if (Reason != ELHCommandReason::None) return Reason;
    if (!AttackHandle.IsValid()) return ELHCommandReason::InvalidRequest;
    PendingTarget = Target;
    PendingTargetLifeRevision = Target->LifeRevision;
    PendingIdentity = {FGuid::NewGuid(), 0};
    // GAS activates synchronously on the local game thread; CommitAttack establishes pending state.
    if (!TryActivateAbility(AttackHandle, false)) { FinishAttack(); return ELHCommandReason::Busy; }
    return ELHCommandReason::None;
}
ELHCommandReason ULHCombatComponent::CommitAttack()
{
    const auto Reason = ValidateAttack(PendingTarget.Get());
    if (Reason != ELHCommandReason::None || !PendingIdentity.ActivationId.IsValid()) return Reason == ELHCommandReason::None ? ELHCommandReason::InvalidRequest : Reason;
    bPending = true; bCommitted = true; bImpactConsumed = false;
    CooldownEnds.Add(Attack.Ability.Value, GetWorld()->GetTimeSeconds() + Attack.CooldownSeconds.Value);
    PendingOutcome=ELHAttackOutcome::ImpactInvalidated;
    PendingEvent={PendingIdentity,StableEntity,PendingTarget->StableEntity,Attack.Ability,LifeRevision,PendingTargetLifeRevision,GetWorld()->GetTimeSeconds(),Attack.ImpactSeconds.Value};
    const auto Event=PendingEvent;
    TGuardValue<int32> Publication(PublicationDepth,PublicationDepth+1);
    bPublishingCommit=true;
    SetNumericAttributeBase(ULHAttributeSet::GetManaAttribute(), Attributes->GetMana() - static_cast<float>(Attack.ManaCost.Value));
    const bool DeferredFinish=bDeferredFinish; bDeferredFinish=false;
    // Cost listeners cannot replace until the matching commit has been published.
    // A committed-event listener may cancel and replace; never finish that replacement.
    bPublishingCommit=DeferredFinish;
    OnAttackCommitted.Broadcast(Event);
    bPublishingCommit=false;
    bDeferredFinish=false;
    if (DeferredFinish && PendingIdentity==Event.Identity) FinishAttack();
    return ELHCommandReason::None;
}
bool ULHCombatComponent::ResolveImpact(const FLHHitIdentity& Id)
{
    if (!bPending || !bCommitted || bImpactConsumed || !(Id == PendingIdentity)) return false;
    TGuardValue<int32> Publication(PublicationDepth,PublicationDepth+1);
    bImpactConsumed = true; // Consume even failed/missed callbacks, before any event can reenter.
    auto* Target = PendingTarget.Get();
    if (!IsAlive() || !IsValid(Target) || !Target->IsAlive() || Target->LifeRevision != PendingTargetLifeRevision || !InRangeAndSight(Target)) return false;
    LH::Rules::FCombatInput Input;
    Input.bSpell=Attack.bSpell;
    Input.Accuracy = Attributes->GetAccuracy(); Input.Avoidance = Target->Attributes->GetAvoidance();
    if (!Attack.bSpell && Attack.bUseLearnedSkills && Attack.Combat.Model==LH::Rules::ECombatModel::Stage1Ratio) {
        Input.Accuracy=0;
        const FName Skill=Attack.bBow?TEXT("Skill.Archery"):TEXT("Skill.Attack");
        for (const auto& S:RequirementInput.Skills) if (S.Skill.Value==Skill) Input.Accuracy=S.TrainedValue.Value;
        Input.DamageBonus=LH::Rules::PhysicalAttributeBonus(RequirementInput.Base.Strength,RequirementInput.Base.Agility,Attack.bBow);
    }
    if ((!Attack.bUseLearnedSkills || Attack.Combat.Model!=LH::Rules::ECombatModel::Stage1Ratio) && !Attack.bSpell) Input.DamageBonus = Attributes->GetDamageBonus();
    Input.QuiverBonus=Attack.QuiverBonus; Input.Armor = Target->Attributes->GetArmor(); Input.Resistance = Target->Attributes->GetResistance();
    Input.WeaponMinimum = Attack.WeaponMinimum.Value; Input.WeaponMaximum = Attack.WeaponMaximum.Value;
    Input.HitRoll = CombatRandom.GetFraction(); Input.DamageRoll = CombatRandom.GetFraction();
    auto Parameters=Attack.Combat;
    if (Attack.bLight) { Parameters.MinimumDamage.Value=0; Input.DamageBonus=0; }
    const auto Result = LH::Rules::ResolveCombat(Parameters, Input);
    if (!Result.Diagnostic.IsAccepted()) return false;
    const FLHHitIdentity CapturedId=Id;
    PendingOutcome=Result.Value.bHit?ELHAttackOutcome::ResolvedHit:ELHAttackOutcome::ResolvedMiss;
    if (Attack.bHeal) Target->SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(),FMath::Min(Target->Attributes->GetMaxHealth(),Target->Attributes->GetHealth()+static_cast<float>(Result.Value.Damage)));
    else if (Attack.bLight) { if (!FMath::IsFinite(Attack.LightDuration) || Attack.LightDuration<=0 || Attack.LightDuration>600) return false; LightRemaining=Attack.LightDuration; }
    else if (Result.Value.bHit && !Target->ApplyResolvedDamage(CapturedId, Result.Value.Damage)) return false;
    OnImpact.Broadcast(CapturedId, Result.Value);
    return true;
}
bool ULHCombatComponent::ApplyResolvedDamage(const FLHHitIdentity& Id, double Damage)
{
    if (!IsAlive() || AppliedHits.Contains(Id) || !FMath::IsFinite(Damage) || Damage < 0) return false;
    AppliedHits.Add(Id);
    const float Health = static_cast<float>(FMath::Max(0.0, static_cast<double>(Attributes->GetHealth()) - Damage));
    const bool bDied = Health <= 0 && !bDeathPublished;
    if (bDied) bDeathPublished = true; // Set before attribute delegates or lifecycle listeners run.
    SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(), Health);
    if (bDied)
    {
        TGuardValue<int32> Publication(PublicationDepth,PublicationDepth+1);
        CancelAttack(ELHAttackCancelReason::SourceDeath);
        OnDeath.Broadcast(Id);
    }
    return true;
}
void ULHCombatComponent::FinishAttack()
{
    if (bPublishingCommit) { bDeferredFinish=true; return; }
    const bool Publish=bCommitted;
    const auto Event=PendingEvent;
    const auto Outcome=bImpactConsumed?PendingOutcome:ELHAttackOutcome::Cancelled;
    const auto Reason=CancelReason;
    bPending = false; bCommitted = false; bImpactConsumed = false;
    PendingTarget.Reset(); PendingIdentity = {};
    CancelReason=ELHAttackCancelReason::Explicit;
    if (Publish) {
        TGuardValue<int32> Publication(PublicationDepth,PublicationDepth+1);
        if (Outcome==ELHAttackOutcome::Cancelled) OnAttackCancelled.Broadcast(Event,Reason);
        OnAttackFinished.Broadcast(Event,Outcome);
    }
}
double ULHCombatComponent::GetRemainingCooldown() const { return GetRemainingCooldown(Attack.Ability); }
double ULHCombatComponent::GetRemainingCooldown(const FLHContentId& Id) const
{
    const auto* End=CooldownEnds.Find(Id.Value); return End && GetWorld()?FMath::Max(0.0,*End-GetWorld()->GetTimeSeconds()):0;
}
bool ULHCombatComponent::RestoreRemainingCooldown(double Seconds)
{
    if (bPending || !GetWorld() || !FMath::IsFinite(Seconds) || Seconds<0) return false;
    CooldownEnds.Add(Attack.Ability.Value,GetWorld()->GetTimeSeconds()+Seconds); return true;
}
TMap<FName,double> ULHCombatComponent::GetCooldownMap() const
{
    TMap<FName,double> R; for (const auto& E:CooldownEnds) { FLHContentId Id; Id.Value=E.Key; const double V=GetRemainingCooldown(Id); if(V>0) R.Add(E.Key,V); } return R;
}
bool ULHCombatComponent::RestoreCooldownMap(const TMap<FName,double>& R)
{
    if (bPending || !GetWorld()) return false;
    for (const auto& E:R) if (E.Key.IsNone() || !FMath::IsFinite(E.Value) || E.Value<0) return false;
    CooldownEnds.Reset(); for(const auto& E:R) CooldownEnds.Add(E.Key,GetWorld()->GetTimeSeconds()+E.Value); return true;
}
void ULHCombatComponent::CancelAttack(ELHAttackCancelReason Reason) { CancelReason=Reason; CancelAllAbilities(); FinishAttack(); if (!bPublishingCommit) CancelReason=ELHAttackCancelReason::Explicit; }
bool ULHCombatComponent::AdvanceManaRegen(double Seconds,bool bPaused,const LH::Rules::FManaParameters& P)
{
    LH::Rules::FManaInput I; I.Current=Attributes->GetMana(); I.Maximum=Attributes->GetMaxMana(); I.FractionalSeconds=ManaRegenFractionalSeconds; I.ActiveSeconds=Seconds; I.bAlive=IsAlive(); I.bPaused=bPaused;
    const auto R=LH::Rules::RegenerateMana(P,I); if(!R.Diagnostic.IsAccepted()) return false;
    ManaRegenFractionalSeconds=R.Value.FractionalSeconds; SetNumericAttributeBase(ULHAttributeSet::GetManaAttribute(),R.Value.Current); return true;
}
double ULHCombatComponent::GetLightRemainingSeconds() const { return LightRemaining; }
bool ULHCombatComponent::RestoreLightRemainingSeconds(double Seconds)
{
    if (bPending || IsPublishingActionEvents() || !FMath::IsFinite(Seconds) || Seconds<0 || Seconds>600) return false;
    LightRemaining=Seconds; return true;
}
bool ULHCombatComponent::AdvanceLightEffect(double Seconds,bool GamePaused,bool AIPaused,bool Travel)
{
    if (!FMath::IsFinite(Seconds) || Seconds<0) return false;
    if (!GamePaused && !AIPaused && !Travel) LightRemaining=FMath::Max(0.0,LightRemaining-Seconds);
    return true;
}

void ULHCombatComponent::SetNumericAttributeBase(const FGameplayAttribute& Attribute, float NewBaseValue)
{
    Super::SetNumericAttributeBase(Attribute, NewBaseValue);
}
void ULHCombatComponent::CancelAllAbilities(UGameplayAbility* Ignore)
{
    Super::CancelAllAbilities(Ignore);
}

void ULHCombatComponent::TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime,TickType,ThisTickFunction);
    if (bManaTickerEnabled && GetWorld()) AdvanceManaRegen(DeltaTime,bRecoveryMenuPaused || GetWorld()->IsPaused(),ManaParameters);
}

FOnGameplayAttributeValueChange& ULHCombatComponent::GetGameplayAttributeValueChangeDelegate(FGameplayAttribute Attribute)
{
    return Super::GetGameplayAttributeValueChangeDelegate(Attribute);
}
