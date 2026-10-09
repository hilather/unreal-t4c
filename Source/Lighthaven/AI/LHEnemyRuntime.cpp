#include "AI/LHEnemyRuntime.h"
#include <limits>
#include "AI/LHEnemyPresentation.h"
namespace LHEnemyRuntimePrivate
{
using FMember = FLHNumber FLHEnemyRuntimeSpec::*;
const FMember Members[] = { &FLHEnemyRuntimeSpec::AggroRadiusCm, &FLHEnemyRuntimeSpec::LeashRadiusCm,
    &FLHEnemyRuntimeSpec::MoveSpeedCmPerSec, &FLHEnemyRuntimeSpec::PathRetryLimit,
    &FLHEnemyRuntimeSpec::MaxActivePursuers, &FLHEnemyRuntimeSpec::DecisionIntervalSeconds,
    &FLHEnemyRuntimeSpec::HomeToleranceCm, &FLHEnemyRuntimeSpec::SpawnSafetyDistanceCm };
bool Ready(const FLHNumber& N, bool Positive = false)
{
    return N.Resolution == ELHValueResolution::Resolved && FMath::IsFinite(N.Value) &&
        N.Value <= std::numeric_limits<float>::max() && (Positive ? N.Value > 0 : N.Value >= 0) &&
        N.Provenance.Status != ELHProvenanceStatus::Missing && N.Provenance.Status != ELHProvenanceStatus::Disputed;
}
}
const TArray<LHAI::FParameterKey>& LHAI::AllowedParameterKeys()
{
    static const TArray<FParameterKey> Keys = {
        {TEXT("AI.AggroRadiusCm"), TEXT("cm"), TEXT("Maximum acquisition distance with LOS")},
        {TEXT("AI.LeashRadiusCm"), TEXT("cm"), TEXT("Maximum distance from authored home, including boss arena anchor")},
        {TEXT("AI.MoveSpeedCmPerSec"), TEXT("cm/s"), TEXT("Navigation movement speed")},
        {TEXT("AI.PathRetryLimit"), TEXT("integer attempts"), TEXT("Maximum consecutive failed path requests before returning home")},
        {TEXT("AI.MaxActivePursuers"), TEXT("integer enemies"), TEXT("Floor concurrent chase/attack cap; minimum participating cap wins")},
        {TEXT("AI.DecisionIntervalSeconds"), TEXT("seconds"), TEXT("Minimum interval between navigation/acquisition decisions")},
        {TEXT("AI.HomeToleranceCm"), TEXT("cm"), TEXT("Home arrival tolerance")},
        {TEXT("AI.SpawnSafetyDistanceCm"), TEXT("cm"), TEXT("Minimum player distance for respawn, also requires no player LOS")}
    };
    return Keys;
}
bool LHAI::ApplyParameters(const TArray<FLHMechanicalField>& Fields, FLHEnemyRuntimeSpec& InOut, FString& Error)
{
    auto Candidate = InOut;
    TSet<FName> Seen;
    const auto& Keys = AllowedParameterKeys();
    for (const auto& Field : Fields)
    {
        const int32 Index = Keys.IndexOfByPredicate([&](const FParameterKey& K) { return K.Name == Field.Key; });
        if (Index == INDEX_NONE || Seen.Contains(Field.Key) || !LHEnemyRuntimePrivate::Ready(Field.Value, true))
        { Error = FString::Printf(TEXT("Unknown, duplicate or unresolved AI parameter: %s"), *Field.Key.ToString()); return false; }
        Seen.Add(Field.Key); Candidate.*LHEnemyRuntimePrivate::Members[Index] = Field.Value;
    }
    if (Seen.Num() != Keys.Num()) { Error = TEXT("All registered AI parameters are required"); return false; }
    for (auto Member : { &FLHEnemyRuntimeSpec::PathRetryLimit, &FLHEnemyRuntimeSpec::MaxActivePursuers })
    {
        const double V = (Candidate.*Member).Value;
        if (V != FMath::FloorToDouble(V) || V > MAX_int32) { Error = TEXT("AI counts must be positive int32 integers"); return false; }
    }
    if (Candidate.LeashRadiusCm.Value < Candidate.AggroRadiusCm.Value || Candidate.HomeToleranceCm.Value >= Candidate.LeashRadiusCm.Value)
    { Error = TEXT("Leash must cover aggro and exceed home tolerance"); return false; }
    InOut = MoveTemp(Candidate); Error.Reset(); return true;
}
bool LHAI::ValidateSpec(const FLHEnemyRuntimeSpec& S, FString& Error)
{
    TArray<FLHMechanicalField> Fields;
    for (int32 I = 0; I < AllowedParameterKeys().Num(); ++I)
    { FLHMechanicalField F; F.Key = AllowedParameterKeys()[I].Name; F.Value = S.*LHEnemyRuntimePrivate::Members[I]; Fields.Add(F); }
    auto Copy = S;
    if (!ApplyParameters(Fields, Copy, Error)) return false;
    if (S.ContentId.Value.IsNone() || !LHEnemyPresentation::Known(S.PresentationId.Value))
    { Error = TEXT("Enemy and presentation IDs required"); return false; }
    for (const auto* N : { &S.CapsuleRadiusCm, &S.CapsuleHalfHeightCm, &S.MaxHealth, &S.Attack.RangeCm })
        if (!LHEnemyRuntimePrivate::Ready(*N, true)) { Error = TEXT("Unresolved positive enemy dimension/resource/range"); return false; }
    if (S.CapsuleHalfHeightCm.Value < S.CapsuleRadiusCm.Value) { Error = TEXT("Capsule half height smaller than radius"); return false; }
    for (const auto* N : { &S.Accuracy, &S.Avoidance, &S.Armor, &S.Resistance, &S.DamageBonus, &S.MaxMana,
        &S.Attack.ManaCost, &S.Attack.CooldownSeconds, &S.Attack.ImpactSeconds, &S.Attack.WeaponMinimum, &S.Attack.WeaponMaximum })
        if (!LHEnemyRuntimePrivate::Ready(*N)) { Error = TEXT("Unresolved enemy combat value"); return false; }
    LH::Rules::FCombatInput Input; Input.WeaponMinimum = S.Attack.WeaponMinimum.Value; Input.WeaponMaximum = S.Attack.WeaponMaximum.Value;
    if (!LH::Rules::ResolveCombat(S.Attack.Combat, Input).Diagnostic.IsAccepted() ||
        !LH::Rules::CheckRequirements(S.Attack.RequirementPolicy, S.Attack.Eligibility, S.Requirements).Diagnostic.IsAccepted())
    { Error = TEXT("Invalid attack formula or eligibility"); return false; }
    Error.Reset(); return true;
}
bool LHAI::SameLife(const FLHSpawnLifeId& A, const FLHSpawnLifeId& B)
{ return A.Area.Content.Value == B.Area.Content.Value && A.SpawnSlot == B.SpawnSlot && A.LifeGeneration == B.LifeGeneration; }
