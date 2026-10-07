// W1-02 — NOT YET COMPILED (UE 5.8.3 pending)
#include "Rules/LHRules.h"
#include <cmath>
#include <limits>

namespace LH::Rules
{
namespace
{
bool Finite(double V) { return std::isfinite(V); }
bool Roll(double V) { return Finite(V) && V >= 0 && V < 1; }
bool Add(int64 A, int64 B, int64& Out)
{
    if ((B > 0 && A > std::numeric_limits<int64>::max() - B) ||
        (B < 0 && A < std::numeric_limits<int64>::min() - B)) return false;
    Out = A + B;
    return true;
}
FDiagnostic Accepted() { return {EReason::None, TEXT("Accepted; caller must commit once and persist canonical inputs.")}; }
template<typename T> TResult<T> Reject(EReason R, const FString& D) { TResult<T> Out; Out.Diagnostic = {R, D}; return Out; }
bool Ready(const FLHFieldProvenance& P)
{
    return P.Status != ELHProvenanceStatus::Missing && P.Status != ELHProvenanceStatus::Disputed;
}
bool Read(const FLHInteger& P, int64& V)
{
    if (P.Resolution != ELHValueResolution::Resolved || !Ready(P.Provenance)) return false;
    V = P.Value; return true;
}
bool Read(const FLHNumber& P, double& V)
{
    if (P.Resolution != ELHValueResolution::Resolved || !Ready(P.Provenance)) return false;
    V = P.Value; return true;
}
bool ReadAttributes(const FAttributeParameters& P, FAttributes& V)
{
    return Read(P.Strength, V.Strength) && Read(P.Endurance, V.Endurance) && Read(P.Agility, V.Agility) &&
        Read(P.Intelligence, V.Intelligence) && Read(P.Wisdom, V.Wisdom);
}
bool ReadAttributes(const FLHAttributeBlock& P, FAttributes& V)
{
    return Read(P.Strength, V.Strength) && Read(P.Endurance, V.Endurance) && Read(P.Agility, V.Agility) &&
        Read(P.Intelligence, V.Intelligence) && Read(P.Wisdom, V.Wisdom);
}
bool Nonnegative(const FAttributes& A)
{
    return A.Strength >= 0 && A.Endurance >= 0 && A.Agility >= 0 && A.Intelligence >= 0 && A.Wisdom >= 0;
}
bool AtLeast(const FAttributes& A, const FAttributes& B)
{
    return A.Strength >= B.Strength && A.Endurance >= B.Endurance && A.Agility >= B.Agility &&
        A.Intelligence >= B.Intelligence && A.Wisdom >= B.Wisdom;
}
bool Sum(const FAttributes& A, int64& V)
{
    V = 0;
    return Add(V,A.Strength,V) && Add(V,A.Endurance,V) && Add(V,A.Agility,V) && Add(V,A.Intelligence,V) && Add(V,A.Wisdom,V);
}
bool AddAttributes(FAttributes& A, const FAttributes& B)
{
    return Add(A.Strength,B.Strength,A.Strength) && Add(A.Endurance,B.Endurance,A.Endurance) &&
        Add(A.Agility,B.Agility,A.Agility) && Add(A.Intelligence,B.Intelligence,A.Intelligence) && Add(A.Wisdom,B.Wisdom,A.Wisdom);
}
FDiagnostic Linear(const FLinearFormula& P, const FAttributes& A, double& V)
{
    double C,S,E,G,I,W;
    if (!Read(P.Constant,C) || !Read(P.Strength,S) || !Read(P.Endurance,E) || !Read(P.Agility,G) ||
        !Read(P.Intelligence,I) || !Read(P.Wisdom,W)) return {EReason::Unresolved, TEXT("Unresolved linear coefficient (Derived stats / HP growth dependence / MP growth dependence)")};
    if (!Finite(C) || !Finite(S) || !Finite(E) || !Finite(G) || !Finite(I) || !Finite(W))
        return {EReason::InvalidData, TEXT("Nonfinite linear coefficient")};
    V = C + S*A.Strength + E*A.Endurance + G*A.Agility + I*A.Intelligence + W*A.Wisdom;
    if (!Finite(V)) return {EReason::Overflow, TEXT("Nonfinite linear result")};
    return Accepted();
}
FLHFieldProvenance Provenance(ELHProvenanceStatus Status, const TCHAR* Row, const TCHAR* Source = TEXT(""))
{
    FLHFieldProvenance P;
    P.Status = Status; P.SourceUrl = Source; P.SourceBaseline = TEXT("LH_Prototype_v1 composite; see rules-ledger.md");
    P.RetrievedDate = TEXT("2026-10-07"); P.Notes = Row;
    return P;
}
FLHInteger Integer(int64 V, const FLHFieldProvenance& P)
{
    FLHInteger Out; Out.Resolution = ELHValueResolution::Resolved; Out.Value = V; Out.Provenance = P; return Out;
}
FLHNumber Number(double V, const FLHFieldProvenance& P)
{
    FLHNumber Out; Out.Resolution = ELHValueResolution::Resolved; Out.Value = V; Out.Provenance = P; return Out;
}
FLHAttributeBlock SavedAttributes(const FAttributes& A)
{
    const auto P = Provenance(ELHProvenanceStatus::Modernized, TEXT("Atomic level-up / debt recovery: caller-provided growth inputs"));
    FLHAttributeBlock Out;
    Out.Strength=Integer(A.Strength,P); Out.Endurance=Integer(A.Endurance,P); Out.Agility=Integer(A.Agility,P);
    Out.Intelligence=Integer(A.Intelligence,P); Out.Wisdom=Integer(A.Wisdom,P); return Out;
}
bool Contains(const TArray<FLHContentId>& Ids, FName Id)
{
    return Ids.ContainsByPredicate([Id](const FLHContentId& Other) { return Other.Value == Id; });
}
}

TResult<FCreationResult> ValidateCreation(const FCreationParameters& P, const FAttributes& A, int64 Unspent)
{
    FAttributes Min,Max; int64 Total;
    if (!ReadAttributes(P.Minimum,Min) || !ReadAttributes(P.Maximum,Max) || !Read(P.TotalPoints,Total))
        return Reject<FCreationResult>(EReason::Unresolved,TEXT("Creation RNG / legal ranges / total points unresolved"));
    if (!Nonnegative(Min) || !AtLeast(Max,Min) || Total < 0)
        return Reject<FCreationResult>(EReason::InvalidData,TEXT("Invalid creation bounds or budget"));
    if (!AtLeast(A,Min) || !AtLeast(Max,A) || Unspent < 0)
        return Reject<FCreationResult>(EReason::Ineligible,TEXT("Outside legal attribute ranges or negative point pool"));
    int64 Spent;
    if (!Sum(A,Spent) || !Add(Spent,Unspent,Spent)) return Reject<FCreationResult>(EReason::Overflow,TEXT("Point sum overflow"));
    if (Spent != Total) return Reject<FCreationResult>(EReason::Ineligible,TEXT("Creation point conservation failed"));
    TResult<FCreationResult> Out; Out.Diagnostic=Accepted(); Out.Value.Attributes=A; Out.Value.UnspentPoints=Unspent; return Out;
}

TResult<FCreationResult> RollCreation(const FCreationParameters& P, const TArray<FLHQuestionAnswer>& Answers, int32 Index)
{
    int64 Count;
    if (!Read(P.AnswerCount,Count) || P.OutcomesResolution != ELHValueResolution::Resolved || !Ready(P.OutcomesProvenance))
        return Reject<FCreationResult>(EReason::Unresolved,TEXT("Creation RNG outcome table unresolved"));
    if (Count <= 0 || Answers.Num() != Count || !P.Outcomes.IsValidIndex(Index))
        return Reject<FCreationResult>(EReason::InvalidData,TEXT("Expected complete question answers and an explicit legal outcome index"));
    TSet<FName> Questions;
    for (const auto& A : Answers)
    {
        if (A.Question.Value.IsNone() || A.Answer.Value.IsNone() || Questions.Contains(A.Question.Value))
            return Reject<FCreationResult>(EReason::InvalidData,TEXT("Empty or repeated question ID"));
        Questions.Add(A.Question.Value);
    }
    const auto& Outcome=P.Outcomes[Index];
    if (Outcome.Answers.Num() != Answers.Num()) return Reject<FCreationResult>(EReason::InvalidData,TEXT("Outcome answer count mismatch"));
    for (int32 N=0; N<Answers.Num(); ++N)
        if (Answers[N].Question.Value != Outcome.Answers[N].Question.Value || Answers[N].Answer.Value != Outcome.Answers[N].Answer.Value)
            return Reject<FCreationResult>(EReason::Ineligible,TEXT("Outcome belongs to different ordered answers"));
    FAttributes A; int64 Unspent;
    if (!ReadAttributes(Outcome.Attributes,A) || !Read(Outcome.UnspentPoints,Unspent))
        return Reject<FCreationResult>(EReason::Unresolved,TEXT("Creation outcome values unresolved"));
    auto Out=ValidateCreation(P,A,Unspent);
    if (Out.Diagnostic.IsAccepted()) Out.Value.OutcomeIndex=Index;
    return Out;
}

TResult<FStatsResult> DeriveStats(const FStatsParameters& P, const FStatsInput& I)
{
    if (!Nonnegative(I.Base) || !Finite(I.EarnedHealth) || !Finite(I.EarnedMana) || I.EarnedHealth < 0 || I.EarnedMana < 0)
        return Reject<FStatsResult>(EReason::InvalidData,TEXT("Invalid canonical stats"));
    FStatsResult V; V.Effective=I.Base; V.MaxHealth=I.EarnedHealth; V.MaxMana=I.EarnedMana;
    double Acc=0,Avoid=0,Damage=0,Armor=0,Capacity=0;
    for (const auto& M : I.Equipment)
    {
        FAttributes A; double H,N,C,D,B,R,K;
        if (!ReadAttributes(M.Attributes,A) || !Read(M.Health,H) || !Read(M.Mana,N) || !Read(M.Accuracy,C) ||
            !Read(M.Avoidance,D) || !Read(M.DamageBonus,B) || !Read(M.Armor,R) || !Read(M.Capacity,K))
            return Reject<FStatsResult>(EReason::Unresolved,TEXT("Unresolved equipment modifier; explicit resolved zeros required"));
        if (!Finite(H)||!Finite(N)||!Finite(C)||!Finite(D)||!Finite(B)||!Finite(R)||!Finite(K))
            return Reject<FStatsResult>(EReason::InvalidData,TEXT("Nonfinite equipment modifier"));
        if (!AddAttributes(V.Effective,A)) return Reject<FStatsResult>(EReason::Overflow,TEXT("Equipment attribute overflow"));
        V.MaxHealth+=H; V.MaxMana+=N; Acc+=C; Avoid+=D; Damage+=B; Armor+=R; Capacity+=K;
    }
    if (!Nonnegative(V.Effective)) return Reject<FStatsResult>(EReason::Ineligible,TEXT("Negative effective attributes"));
    const FLinearFormula* Formulas[]={&P.Accuracy,&P.Avoidance,&P.DamageBonus,&P.Armor,&P.Capacity};
    double* Values[]={&V.Accuracy,&V.Avoidance,&V.DamageBonus,&V.Armor,&V.Capacity};
    for (int32 N=0; N<UE_ARRAY_COUNT(Formulas); ++N)
    {
        const auto D=Linear(*Formulas[N],V.Effective,*Values[N]);
        if (!D.IsAccepted()) return Reject<FStatsResult>(D.Reason,D.Detail);
    }
    V.Accuracy+=Acc; V.Avoidance+=Avoid; V.DamageBonus+=Damage; V.Armor+=Armor; V.Capacity+=Capacity;
    const double Results[]={V.MaxHealth,V.MaxMana,V.Accuracy,V.Avoidance,V.DamageBonus,V.Armor,V.Capacity};
    for (double Result : Results) if (!Finite(Result) || Result < 0) return Reject<FStatsResult>(EReason::InvalidData,TEXT("Invalid derived stat"));
    TResult<FStatsResult> Out; Out.Diagnostic=Accepted(); Out.Value=V; return Out;
}

TResult<FProgressionResult> Advance(const FProgressionParameters& P, const FProgressionInput& I)
{
    int64 Initial,AP,SP;
    if (!Read(P.InitialLevel,Initial) || !Read(P.AttributePointsPerLevel,AP) || !Read(P.SkillPointsPerLevel,SP) || P.Thresholds.IsEmpty())
        return Reject<FProgressionResult>(EReason::Unresolved,TEXT("Start level / XP curve / level entitlement unresolved"));
    if (Initial < 0 || AP < 0 || SP < 0 || I.EarnedLevel < Initial || I.ExperienceBalance < 0 || I.ExperienceDebt < 0 || I.ExperienceGain < 0 ||
        I.UnspentAttributes < 0 || I.UnspentSkills < 0 || !Nonnegative(I.GrowthAttributes) || !Finite(I.EarnedHealth) || !Finite(I.EarnedMana) || I.EarnedHealth < 0 || I.EarnedMana < 0)
        return Reject<FProgressionResult>(EReason::InvalidData,TEXT("Invalid progression input"));
    TArray<int64> Thresholds;
    for (const auto& T : P.Thresholds)
    {
        int64 V;
        if (!Read(T,V)) return Reject<FProgressionResult>(EReason::Unresolved,TEXT("Unresolved XP threshold"));
        if (V < 0 || (!Thresholds.IsEmpty() && V <= Thresholds.Last())) return Reject<FProgressionResult>(EReason::InvalidData,TEXT("XP thresholds must increase strictly"));
        Thresholds.Add(V);
    }
    const int64 OldIndex=I.EarnedLevel-Initial;
    if (OldIndex > std::numeric_limits<int32>::max() || !Thresholds.IsValidIndex(static_cast<int32>(OldIndex)))
        return Reject<FProgressionResult>(EReason::InvalidData,TEXT("Earned level outside XP table"));
    // Draft contracts-v1 convention: balance is retained progress; debt is a separate deficit.
    // Keeping balance at its pre-loss value avoids representing the same lost XP twice.
    if (I.ExperienceBalance < Thresholds[static_cast<int32>(OldIndex)])
        return Reject<FProgressionResult>(EReason::InvalidData,TEXT("Balance below earned threshold: normalize debt separately per contracts-v1"));
    FProgressionResult V;

    V.EarnedLevel=I.EarnedLevel; V.ExperienceBalance=I.ExperienceBalance; V.ExperienceDebt=I.ExperienceDebt;
    V.UnspentAttributes=I.UnspentAttributes; V.UnspentSkills=I.UnspentSkills; V.EarnedHealth=I.EarnedHealth; V.EarnedMana=I.EarnedMana;
    const int64 Repaid=FMath::Min(V.ExperienceDebt,I.ExperienceGain);
    V.ExperienceDebt-=Repaid;
    const int64 ProgressGain=I.ExperienceGain-Repaid;
    if (!Add(V.ExperienceBalance,ProgressGain,V.ExperienceBalance)) return Reject<FProgressionResult>(EReason::Overflow,TEXT("XP balance overflow"));
    int32 Target=static_cast<int32>(OldIndex);
    if (V.ExperienceDebt == 0)
        while (Thresholds.IsValidIndex(Target+1) && V.ExperienceBalance >= Thresholds[Target+1]) ++Target;
    const int32 NewLevels=Target-static_cast<int32>(OldIndex);
    if (I.Rolls.Num() != NewLevels) return Reject<FProgressionResult>(EReason::InvalidData,TEXT("Supply exactly one growth roll and award ID per newly earned level"));
    if (NewLevels > 0)
    {
        if (I.Ruleset.Id.Value.IsNone() || I.Ruleset.Revision <= 0 || I.Ruleset.ContentHash.IsEmpty())
            return Reject<FProgressionResult>(EReason::InvalidData,TEXT("Growth requires frozen ruleset identity/revision/hash"));
        double H,M,HS,MS;
        const auto HD=Linear(P.HealthGrowth,I.GrowthAttributes,H), MD=Linear(P.ManaGrowth,I.GrowthAttributes,M);
        if (!HD.IsAccepted()) return Reject<FProgressionResult>(HD.Reason,HD.Detail);
        if (!MD.IsAccepted()) return Reject<FProgressionResult>(MD.Reason,MD.Detail);
        if (!Read(P.HealthRollScale,HS) || !Read(P.ManaRollScale,MS)) return Reject<FProgressionResult>(EReason::Unresolved,TEXT("Unresolved growth random scale"));
        if (!Finite(HS) || !Finite(MS) || HS < 0 || MS < 0 || H < 0 || M < 0)
            return Reject<FProgressionResult>(EReason::InvalidData,TEXT("Invalid growth parameters"));
        TSet<FGuid> Ids;
        for (const auto& R : I.Rolls)
        {
            if (!Roll(R.Health) || !Roll(R.Mana) || !R.AwardId.Value.IsValid() || Ids.Contains(R.AwardId.Value))
                return Reject<FProgressionResult>(EReason::InvalidData,TEXT("Invalid/duplicate growth roll or award ID"));
            Ids.Add(R.AwardId.Value);
            const double HG=std::floor(H+HS*R.Health), MG=std::floor(M+MS*R.Mana);
            if (!Finite(HG) || !Finite(MG) || !Finite(V.EarnedHealth+HG) || !Finite(V.EarnedMana+MG) ||
                !Add(V.UnspentAttributes,AP,V.UnspentAttributes) || !Add(V.UnspentSkills,SP,V.UnspentSkills))
                return Reject<FProgressionResult>(EReason::Overflow,TEXT("Growth award overflow"));
            FLHGrowthAward A; A.AwardId=R.AwardId; A.Ruleset=I.Ruleset; A.GrowthInputs=SavedAttributes(I.GrowthAttributes);
            const auto Policy=Provenance(ELHProvenanceStatus::Modernized,TEXT("Atomic level-up / debt recovery"));
            A.FromLevel=Integer(V.EarnedLevel,Policy);
            if (!Add(V.EarnedLevel,1,V.EarnedLevel)) return Reject<FProgressionResult>(EReason::Overflow,TEXT("Level overflow"));
            A.ToLevel=Integer(V.EarnedLevel,Policy);
            // Formula family is provisional even when a coefficient/sample is source-backed.
            const auto Growth=Provenance(ELHProvenanceStatus::Prototype,TEXT("HP growth dependence / MP growth dependence: proposed linear floor model; not Classic parity"));
            A.HealthIncrement=Number(HG,Growth); A.ManaIncrement=Number(MG,Growth);
            A.AttributePoints=Integer(AP,P.AttributePointsPerLevel.Provenance); A.SkillPoints=Integer(SP,P.SkillPointsPerLevel.Provenance);
            V.EarnedHealth+=HG; V.EarnedMana+=MG; V.Awards.Add(A); V.AcceptedRolls.Add(R);
        }
    }
    TResult<FProgressionResult> Out; Out.Diagnostic=Accepted(); Out.Value=MoveTemp(V); return Out;
}

TResult<bool> CheckRequirements(const FRequirementPolicy& P, const FLHEligibility& E, const FRequirementInput& I)
{
    FAttributes Minimum; int64 Level,Quiver;
    if (P.Basis == EAttributeBasis::Unresolved || !Ready(P.BasisProvenance) || !ReadAttributes(E.MinimumAttributes,Minimum) || !Read(E.MinimumLevel,Level))
        return Reject<bool>(EReason::Unresolved,TEXT("Requirement evaluation stat basis / minima unresolved"));
    // No arbitrary policy strings are silently ignored.
    if (!E.Policies.IsEmpty()) return Reject<bool>(EReason::Unresolved,TEXT("Eligibility policy extension not supported; integrator must define semantics"));
    if (Level < 0 || I.Level < 0 || !Nonnegative(Minimum) || !Nonnegative(I.Base) || !Nonnegative(I.Effective))
        return Reject<bool>(EReason::InvalidData,TEXT("Invalid requirement attributes/level"));
    if (I.bBow)
    {
        if (!Read(P.BowRequiresQuiver,Quiver)) return Reject<bool>(EReason::Unresolved,TEXT("Bow quiver policy unresolved"));
        if (Quiver != 0 && Quiver != 1) return Reject<bool>(EReason::InvalidData,TEXT("Quiver policy must be a boolean integer"));
        if (Quiver != 0 && !I.bCompatibleQuiverEquipped) return Reject<bool>(EReason::Ineligible,TEXT("Bow requires compatible equipped quiver; unlimited quiver not consumed"));
    }
    // Resolve every skill minimum before deciding eligibility.
    for (const auto& M : E.SkillMinimums)
    {
        double Value;
        if (!Read(M.Value,Value)) return Reject<bool>(EReason::Unresolved,TEXT("Unresolved skill minimum"));
        if (M.Key.IsNone() || !Finite(Value) || Value < 0) return Reject<bool>(EReason::InvalidData,TEXT("Invalid skill minimum"));
    }
    for (const auto& S : I.Skills)
    {
        int64 Value;
        if (!Read(S.TrainedValue,Value)) return Reject<bool>(EReason::Unresolved,TEXT("Unresolved trained skill value"));
        if (S.Skill.Value.IsNone() || Value < 0) return Reject<bool>(EReason::InvalidData,TEXT("Invalid learned skill"));
    }
    const FAttributes& A=P.Basis == EAttributeBasis::Base ? I.Base : I.Effective;
    if (!AtLeast(A,Minimum) || I.Level < Level) return Reject<bool>(EReason::Ineligible,TEXT("Attribute or level prerequisite failed"));
    for (const auto& S : E.RequiredSkills)
        if (S.Value.IsNone() || !I.Skills.ContainsByPredicate([&S](const FLHLearnedSkill& K) { return K.Skill.Value == S.Value; }))
            return Reject<bool>(EReason::Ineligible,TEXT("Required learned skill missing"));
    for (const auto& S : E.RequiredSpells)
        if (S.Value.IsNone() || !Contains(I.Spells,S.Value)) return Reject<bool>(EReason::Ineligible,TEXT("Required learned spell missing"));
    for (const auto& M : E.SkillMinimums)
    {
        const auto* Skill=I.Skills.FindByPredicate([&M](const FLHLearnedSkill& K) { return K.Skill.Value == M.Key; });
        if (!Skill || Skill->TrainedValue.Value < M.Value.Value) return Reject<bool>(EReason::Ineligible,TEXT("Trained skill minimum failed"));
    }
    TResult<bool> Out; Out.Diagnostic=Accepted(); Out.Value=true; return Out;
}

TResult<FCombatResult> ResolveCombat(const FCombatParameters& P, const FCombatInput& I)
{
    double Base,Acc,Avoid,Min,Max,Armor,Resistance,Floor,Quantum;
    if (!Read(P.HitBase,Base)||!Read(P.AccuracyScale,Acc)||!Read(P.AvoidanceScale,Avoid)||!Read(P.MinimumChance,Min)||!Read(P.MaximumChance,Max)||
        !Read(P.ArmorScale,Armor)||!Read(P.ResistanceScale,Resistance)||!Read(P.MinimumDamage,Floor)||!Read(P.DamageQuantum,Quantum))
        return Reject<FCombatResult>(EReason::Unresolved,TEXT("Physical hit/damage parameters unresolved"));
    const double Values[]={Base,Acc,Avoid,Min,Max,Armor,Resistance,Floor,Quantum,I.Accuracy,I.Avoidance,I.WeaponMinimum,I.WeaponMaximum,I.DamageBonus,I.QuiverBonus,I.Armor,I.Resistance};
    for (double V : Values) if (!Finite(V)) return Reject<FCombatResult>(EReason::InvalidData,TEXT("Nonfinite combat input/parameter"));
    if (Min < 0 || Max > 1 || Min > Max || Armor < 0 || Resistance < 0 || Floor < 0 || Quantum <= 0 ||
        I.WeaponMinimum < 0 || I.WeaponMaximum < I.WeaponMinimum || I.Armor < 0 || I.Resistance < 0 || !Roll(I.HitRoll) || !Roll(I.DamageRoll))
        return Reject<FCombatResult>(EReason::InvalidData,TEXT("Invalid combat bounds, quantum or explicit normalized rolls"));
    FCombatResult V;
    const double Chance=Base+Acc*I.Accuracy-Avoid*I.Avoidance;
    if (!Finite(Chance)) return Reject<FCombatResult>(EReason::Overflow,TEXT("Hit chance overflow"));
    V.Chance=I.bSpell ? 1 : FMath::Clamp(Chance,Min,Max);
    V.bHit=I.bSpell || I.HitRoll < V.Chance;
    if (V.bHit)
    {
        V.RawDamage=I.WeaponMinimum+(I.WeaponMaximum-I.WeaponMinimum)*I.DamageRoll+I.DamageBonus+I.QuiverBonus;
        const double ArmorReduction=I.bSpell ? 0 : Armor*I.Armor;
        const double ResistanceProduct=Resistance*I.Resistance;
        if (!Finite(V.RawDamage)||!Finite(ArmorReduction)||!Finite(ResistanceProduct)) return Reject<FCombatResult>(EReason::Overflow,TEXT("Damage composition overflow"));
        V.MitigatedDamage=FMath::Max(Floor,V.RawDamage-ArmorReduction)*(1-FMath::Clamp(ResistanceProduct,0.0,1.0));
        // Floor to an explicit data-defined quantum; no implicit minimum-one-damage rule.
        V.Damage=std::floor(V.MitigatedDamage/Quantum)*Quantum;
        if (!Finite(V.MitigatedDamage)||!Finite(V.Damage)) return Reject<FCombatResult>(EReason::Overflow,TEXT("Damage rounding overflow"));
    }
    TResult<FCombatResult> Out; Out.Diagnostic=Accepted(); Out.Value=V; return Out;
}

TResult<FManaResult> RegenerateMana(const FManaParameters& P, const FManaInput& I)
{
    double Amount,Interval;
    if (!Read(P.Amount,Amount)||!Read(P.IntervalSeconds,Interval)) return Reject<FManaResult>(EReason::Unresolved,TEXT("Mana regeneration rate unresolved"));
    const double Values[]={Amount,Interval,I.Current,I.Maximum,I.FractionalSeconds,I.ActiveSeconds};
    for (double V : Values) if (!Finite(V)) return Reject<FManaResult>(EReason::InvalidData,TEXT("Nonfinite mana parameter/input"));
    if (Amount < 0 || Interval <= 0 || I.Current < 0 || I.Maximum < I.Current || I.FractionalSeconds < 0 || I.FractionalSeconds >= Interval || I.ActiveSeconds < 0)
        return Reject<FManaResult>(EReason::InvalidData,TEXT("Invalid mana resource, carry, elapsed time or interval"));
    FManaResult V; V.Current=I.Current; V.FractionalSeconds=I.FractionalSeconds;
    if (I.bAlive && !I.bPaused)
    {
        const double Total=I.FractionalSeconds+I.ActiveSeconds;
        if (!Finite(Total)) return Reject<FManaResult>(EReason::Overflow,TEXT("Mana timer overflow"));
        V.Ticks=std::floor(Total/Interval); V.FractionalSeconds=std::fmod(Total,Interval);
        const double Restored=V.Ticks*Amount;
        if (!Finite(Restored)||!Finite(I.Current+Restored)) return Reject<FManaResult>(EReason::Overflow,TEXT("Mana recovery overflow"));
        V.Current=FMath::Min(I.Maximum,I.Current+Restored);
    }
    TResult<FManaResult> Out; Out.Diagnostic=Accepted(); Out.Value=V; return Out;
}
}
