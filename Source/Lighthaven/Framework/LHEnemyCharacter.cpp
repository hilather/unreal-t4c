#include "Framework/LHEnemyCharacter.h"
#include "Abilities/LHCombatComponent.h"
#include "Framework/LHDevCombatFixture.h"
#include "Components/CapsuleComponent.h"
#include "Abilities/LHAttributeSet.h"
#include "Visual/Monsters/LHMonsterVisual.h"
#include "AI/LHEnemyAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
ALHEnemyCharacter::ALHEnemyCharacter()
{
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    Combat = CreateDefaultSubobject<ULHCombatComponent>(TEXT("Combat"));
    MonsterVisual = CreateDefaultSubobject<ULHMonsterVisual>(TEXT("MonsterVisual"));
    MonsterVisual->SetupAttachment(GetRootComponent());
}
UAbilitySystemComponent* ALHEnemyCharacter::GetAbilitySystemComponent() const { return Combat; }
void ALHEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (!bHasSpec) LHDevCombat::InitializeForMap(Combat, this);
    InitializeAfterRestore();
    BindPresentationEvents();
}
void ALHEnemyCharacter::InitializeAfterRestore() { Combat->InitializeCombatActorInfo(this, this); }
void ALHEnemyCharacter::EndPlay(const EEndPlayReason::Type Reason) { Combat->ClearCombatAvatar(); Super::EndPlay(Reason); }

bool ALHEnemyCharacter::ApplyRuntimeSpec(const FLHEnemyRuntimeSpec& InSpec, const FLHSpawnLifeId& InLife, const FLHNumber& Health, FString& Error)
{
    if (bHasSpec || !LHAI::ValidateSpec(InSpec, Error)) return false;
    if (!InLife.SpawnSlot.IsValid() || InLife.Area.Content.Value.IsNone() || InLife.LifeGeneration < 0 ||
        Health.Resolution != ELHValueResolution::Resolved || !FMath::IsFinite(Health.Value) || Health.Value <= 0 || Health.Value > InSpec.MaxHealth.Value)
    { Error = TEXT("Invalid alive life or persisted health"); return false; }
    Spec = InSpec; Life = InLife; bHasSpec = true;
    GetCapsuleComponent()->SetCapsuleSize(Spec.CapsuleRadiusCm.Value, Spec.CapsuleHalfHeightCm.Value);
    GetCharacterMovement()->MaxWalkSpeed = Spec.MoveSpeedCmPerSec.Value;
    GetCharacterMovement()->UpdateNavAgent(*GetCapsuleComponent());
    Combat->SetNumericAttributeBase(ULHAttributeSet::GetMaxHealthAttribute(), Spec.MaxHealth.Value);
    Combat->SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(), Health.Value);
    Combat->SetNumericAttributeBase(ULHAttributeSet::GetMaxManaAttribute(), Spec.MaxMana.Value);
    Combat->SetNumericAttributeBase(ULHAttributeSet::GetManaAttribute(), Spec.MaxMana.Value);
    Combat->SetNumericAttributeBase(ULHAttributeSet::GetAccuracyAttribute(), Spec.Accuracy.Value);
    Combat->SetNumericAttributeBase(ULHAttributeSet::GetAvoidanceAttribute(), Spec.Avoidance.Value);
    Combat->SetNumericAttributeBase(ULHAttributeSet::GetArmorAttribute(), Spec.Armor.Value);
    Combat->SetNumericAttributeBase(ULHAttributeSet::GetResistanceAttribute(), Spec.Resistance.Value);
    Combat->SetNumericAttributeBase(ULHAttributeSet::GetDamageBonusAttribute(), Spec.DamageBonus.Value);
    Combat->ConfigureAttack(Spec.Attack, Spec.Requirements);
    Combat->SetCombatRandomState(FRandomStream(GetTypeHash(Life.SpawnSlot) ^ GetTypeHash(Life.LifeGeneration)));
    InitializeAfterRestore();
    MonsterVisual->Build(Spec.ContentId.Value, Spec.CapsuleRadiusCm.Value, Spec.CapsuleHalfHeightCm.Value);
    BindPresentationEvents();
    return true;
}
bool ALHEnemyCharacter::IsAlive() const { return !bCorpse && Combat->IsAlive(); }
FLHEntityId ALHEnemyCharacter::GetEntityId(const FGuid& RunId) const
{ FLHEntityId Id; Id.RunId = RunId; Id.Area = Life.Area; Id.InstanceId = Life.SpawnSlot; return Id; }
void ALHEnemyCharacter::MarkCorpse()
{
    if (bCorpse) return;
    bCorpse = true;
    Combat->CancelAllAbilities(); GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->DisableMovement();
    // Corpses remain targetable by visibility traces, but cannot block exits or navigation.
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    GetCapsuleComponent()->SetCanEverAffectNavigation(false);
    MonsterVisual->Die();
    if (auto* AI = Cast<ALHEnemyAIController>(GetController())) AI->EnterDead();
}

void ALHEnemyCharacter::BindPresentationEvents()
{
    if (bPresentationBound) return;
    bPresentationBound = true;
    Combat->OnDeath.AddWeakLambda(this, [this](const FLHHitIdentity&) { MarkCorpse(); });
    Combat->OnAttackCommitted.AddWeakLambda(this, [this](const FLHAttackEvent& E) { MonsterVisual->Attack(E.ImpactSeconds, E.CommitSimulationTime); });
    Combat->OnAttackCancelled.AddWeakLambda(this, [this](const FLHAttackEvent&, ELHAttackCancelReason) { MonsterVisual->CancelAttack(); });
    Combat->OnAttackFinished.AddWeakLambda(this, [this](const FLHAttackEvent&, ELHAttackOutcome Outcome)
    {
        if (Outcome == ELHAttackOutcome::ResolvedHit || Outcome == ELHAttackOutcome::ResolvedMiss) MonsterVisual->Strike();
        else MonsterVisual->CancelAttack();
    });
    Combat->GetGameplayAttributeValueChangeDelegate(ULHAttributeSet::GetHealthAttribute()).AddWeakLambda(this, [this](const FOnAttributeChangeData& D)
    { if (D.NewValue < D.OldValue && D.NewValue > 0) MonsterVisual->Hit(); });
}
