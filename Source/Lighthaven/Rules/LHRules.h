// W1-02 — NOT YET COMPILED (UE 5.8.3 pending)
#pragma once
#include "Core/LHSaveSnapshot.h"

namespace LH::Rules
{
// Plain values: no reflection, asset/actor access, mutable globals or random stream.
enum class EReason : uint8 { None, Unresolved, InvalidData, Ineligible, Overflow };
struct FDiagnostic
{
    EReason Reason = EReason::InvalidData;
    FString Detail;
    bool IsAccepted() const { return Reason == EReason::None; }
};
template<typename T> struct TResult
{
    FDiagnostic Diagnostic;
    T Value{}; // Ignore unless Diagnostic.IsAccepted(). Rejection never exposes partial output.
};
struct FAttributes
{
    int64 Strength = 0, Endurance = 0, Agility = 0, Intelligence = 0, Wisdom = 0;
};
struct FAttributeParameters
{
    FLHInteger Strength, Endurance, Agility, Intelligence, Wisdom;
};
struct FLinearFormula
{
    FLHNumber Constant, Strength, Endurance, Agility, Intelligence, Wisdom;
};
struct FCreationOutcome
{
    TArray<FLHQuestionAnswer> Answers;
    FAttributeParameters Attributes;
    FLHInteger UnspentPoints;
};
struct FCreationParameters
{
    FLHInteger AnswerCount;
    FAttributeParameters Minimum, Maximum;
    FLHInteger TotalPoints;
    // Declared finite outcome table, NOT reconstructed Classic RNG. Index is an explicit caller roll.
    ELHValueResolution OutcomesResolution = ELHValueResolution::Unresolved;
    FLHFieldProvenance OutcomesProvenance;
    TArray<FCreationOutcome> Outcomes;
};
struct FCreationResult { FAttributes Attributes; int64 UnspentPoints = 0; int32 OutcomeIndex = INDEX_NONE; };
struct FStatsParameters
{
    FLinearFormula Accuracy, Avoidance, DamageBonus, Armor, Capacity;
};
struct FModifier
{
    FAttributeParameters Attributes;
    FLHNumber Health, Mana, Accuracy, Avoidance, DamageBonus, Armor, Capacity;
};
struct FStatsInput
{
    FAttributes Base;
    double EarnedHealth = 0, EarnedMana = 0;
    TArray<FModifier> Equipment;
};
struct FStatsResult
{
    FAttributes Effective;
    double MaxHealth = 0, MaxMana = 0, Accuracy = 0, Avoidance = 0, DamageBonus = 0, Armor = 0, Capacity = 0;
};
struct FProgressionParameters
{
    FLHInteger InitialLevel, AttributePointsPerLevel, SkillPointsPerLevel;
    // Entry zero is InitialLevel; finite table is also the level cap. Strictly increasing XP.
    TArray<FLHInteger> Thresholds;
    FLinearFormula HealthGrowth, ManaGrowth;
    FLHNumber HealthRollScale, ManaRollScale;
};
struct FGrowthRoll { double Health = 0, Mana = 0; FLHRewardId AwardId; };
struct FProgressionInput
{
    int64 EarnedLevel = 0, ExperienceBalance = 0, ExperienceDebt = 0, ExperienceGain = 0;
    int64 UnspentAttributes = 0, UnspentSkills = 0;
    double EarnedHealth = 0, EarnedMana = 0;
    FAttributes GrowthAttributes;
    FLHRulesetRef Ruleset;
    TArray<FGrowthRoll> Rolls; // Exactly one caller-provided normalized roll/ID per new level.
};
struct FProgressionResult
{
    int64 EarnedLevel = 0, ExperienceBalance = 0, ExperienceDebt = 0;
    int64 UnspentAttributes = 0, UnspentSkills = 0;
    double EarnedHealth = 0, EarnedMana = 0;
    TArray<FLHGrowthAward> Awards;
    TArray<FGrowthRoll> AcceptedRolls; // Index-aligned with Awards; integrator encodes canonical RollInputs.
};
enum class EAttributeBasis : uint8 { Unresolved, Base, Effective };
struct FRequirementPolicy
{
    EAttributeBasis Basis = EAttributeBasis::Unresolved;
    FLHFieldProvenance BasisProvenance;
    FLHInteger BowRequiresQuiver; // Resolved boolean integer (0 or 1).
};
struct FRequirementInput
{
    FAttributes Base, Effective;
    int64 Level = 0;
    TArray<FLHLearnedSkill> Skills;
    TArray<FLHContentId> Spells;
    bool bBow = false, bCompatibleQuiverEquipped = false;
};
struct FCombatParameters
{
    // Proposed linear physical chance and flat-armor / fractional-resistance model.
    FLHNumber HitBase, AccuracyScale, AvoidanceScale, MinimumChance, MaximumChance;
    FLHNumber ArmorScale, ResistanceScale, MinimumDamage, DamageQuantum;
};
struct FCombatInput
{
    double Accuracy = 0, Avoidance = 0, WeaponMinimum = 0, WeaponMaximum = 0;
    double DamageBonus = 0, QuiverBonus = 0, Armor = 0, Resistance = 0;
    double HitRoll = 0, DamageRoll = 0; // [0,1), supplied even for guaranteed-hit spells.
    bool bSpell = false;
};
struct FCombatResult { bool bHit = false; double Chance = 0, RawDamage = 0, MitigatedDamage = 0, Damage = 0; };
struct FManaParameters { FLHNumber Amount, IntervalSeconds; };
struct FManaInput
{
    double Current = 0, Maximum = 0, FractionalSeconds = 0, ActiveSeconds = 0;
    bool bAlive = true, bPaused = false; // ActiveSeconds excludes offline time; not frame delta.
};
struct FManaResult { double Current = 0, FractionalSeconds = 0, Ticks = 0; };
struct FRuleset
{
    FCreationParameters Creation;
    FStatsParameters Stats;
    FProgressionParameters Progression;
    FRequirementPolicy Requirements;
    FCombatParameters Combat;
    FManaParameters Mana;
};
LIGHTHAVEN_API TResult<FCreationResult> RollCreation(const FCreationParameters&, const TArray<FLHQuestionAnswer>&, int32 OutcomeIndex);
LIGHTHAVEN_API TResult<FCreationResult> ValidateCreation(const FCreationParameters&, const FAttributes&, int64 UnspentPoints);
LIGHTHAVEN_API TResult<FStatsResult> DeriveStats(const FStatsParameters&, const FStatsInput&);
LIGHTHAVEN_API TResult<FProgressionResult> Advance(const FProgressionParameters&, const FProgressionInput&);
LIGHTHAVEN_API TResult<bool> CheckRequirements(const FRequirementPolicy&, const FLHEligibility&, const FRequirementInput&);
LIGHTHAVEN_API TResult<FCombatResult> ResolveCombat(const FCombatParameters&, const FCombatInput&);
LIGHTHAVEN_API TResult<FManaResult> RegenerateMana(const FManaParameters&, const FManaInput&);
// Only ledger-supported values. Missing creation/combat/growth/XP parameters stay Unresolved.
LIGHTHAVEN_API FRuleset MakeLedgerPrototypeRuleset();
}
