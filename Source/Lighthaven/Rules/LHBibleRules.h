#pragma once
#include "Rules/LHRules.h"

namespace LH::Rules
{
struct FBibleBand { FLHInteger Lower, Upper, Minimum, Maximum; };
struct FBibleXP { FLHInteger Level, Threshold, Next; };
// Affinity is qualitative, never a numeric delta or generator.
struct FBibleAnswer { FName Theme, Answer, Category; FAttributes Affinity; FLHFieldProvenance Provenance; };
struct FBibleRoll { FString Categories; FLHInteger PrintedTotal; FAttributeParameters Attributes; };
struct FBibleAcquisition
{
    FName Name;
    FAttributeParameters Minimum;
    FLHInteger Level, Mana, Points, Gold, LearnCost, TrainCost;
    TArray<FName> Prerequisites;
    FName RequiredSkill;
    FLHInteger SkillRank;
    FLHInteger CostUnit; // Unresolved for printed skill costs.
    FLHFieldProvenance Provenance;
};
struct FBibleRuleset
{
    TArray<FBibleBand> Health, IntelligenceMana, WisdomMana;
    TArray<FBibleXP> Experience;
    TArray<FBibleAnswer> Answers;
    TArray<FBibleRoll> Rolls;
    TArray<FBibleAcquisition> Skills, Spells;
    FLHInteger StatCap, AnswerCount, AttributeGrant, SkillGrant;
    FLHNumber CapacityScale, CapacityOffset;
    FLHInteger CreationRNG, ManaCombination, GrowthTiming, InitialResources;
    EAttributeBasis RequirementBasis = EAttributeBasis::Unresolved;
};
// Classic selections accept Disputed documentary rows only in this separate API.
// No runtime composite ruleset migration is implied.
LIGHTHAVEN_API FBibleRuleset MakeBibleRuleset();
LIGHTHAVEN_API TResult<FBibleBand> LookupBibleBand(const TArray<FBibleBand>&, int64 Attribute);
LIGHTHAVEN_API TResult<FBibleXP> LookupBibleXP(const FBibleRuleset&, int64 Level);
LIGHTHAVEN_API TResult<FBibleAnswer> LookupBibleAnswer(const FBibleRuleset&, FName Theme, FName Answer);
LIGHTHAVEN_API TResult<bool> ValidateBibleStatCap(const FBibleRuleset&, const FAttributes&);
LIGHTHAVEN_API TResult<double> BibleEncumbrance(const FBibleRuleset&, int64 Strength);
// Caller explicitly supplies the unresolved base/effective policy; no default assumed.
LIGHTHAVEN_API TResult<bool> CheckBibleRequirements(const FBibleAcquisition&, EAttributeBasis, const FRequirementInput&);
LIGHTHAVEN_API TResult<int64> BibleCost(const FLHInteger&);
LIGHTHAVEN_API TResult<int64> BibleSkillCost(const FBibleAcquisition&, bool bTraining);
}

namespace LH::Rules
{
// Counts ordered Str, Agi, Wis, Int, N/A; retrieves representative chart evidence.
LIGHTHAVEN_API TResult<FBibleRoll> LookupBibleRoll(const FBibleRuleset&, const TArray<int32>& CategoryCounts);
// Combined actual growth remains unresolved until an explicit combination policy is sourced.
LIGHTHAVEN_API TResult<int64> BibleCombinedMana(const FBibleRuleset&, int64 Intelligence, int64 Wisdom);
}
