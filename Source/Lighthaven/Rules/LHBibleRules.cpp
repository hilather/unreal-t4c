#include "Rules/LHBibleRules.h"
#include <cmath>
namespace LH::Rules
{
namespace LHBibleRulesPrivate
{
template<class T> TResult<T> Fail(EReason R,const TCHAR* D) { TResult<T> O; O.Diagnostic={R,D}; return O; }
template<class T> TResult<T> Accept(const T& V) { TResult<T> O; O.Value=V; O.Diagnostic={EReason::None,TEXT("Bible documentary lookup; caller owns policy and mutation")}; return O; }
bool Ready(const FLHInteger& V) { return V.Resolution==ELHValueResolution::Resolved && V.Provenance.Status!=ELHProvenanceStatus::Missing; }
bool Ready(const FLHNumber& V) { return V.Resolution==ELHValueResolution::Resolved && V.Provenance.Status!=ELHProvenanceStatus::Missing; }
}
TResult<int64> BibleCost(const FLHInteger& V)
{
    if (!LHBibleRulesPrivate::Ready(V)) return LHBibleRulesPrivate::Fail<int64>(EReason::Unresolved,TEXT("Missing Bible value"));
    if (V.Value<0) return LHBibleRulesPrivate::Fail<int64>(EReason::InvalidData,TEXT("Negative Bible value"));
    return LHBibleRulesPrivate::Accept(V.Value);
}
TResult<FBibleBand> LookupBibleBand(const TArray<FBibleBand>& Bands,int64 Attribute)
{
    if (Attribute<0) return LHBibleRulesPrivate::Fail<FBibleBand>(EReason::InvalidData,TEXT("Negative attribute"));
    const FBibleBand* Found=nullptr; int64 Previous=-1;
    for (const auto& B:Bands)
    {
        if (!LHBibleRulesPrivate::Ready(B.Lower)||!LHBibleRulesPrivate::Ready(B.Upper)||!LHBibleRulesPrivate::Ready(B.Minimum)||!LHBibleRulesPrivate::Ready(B.Maximum)) return LHBibleRulesPrivate::Fail<FBibleBand>(EReason::Unresolved,TEXT("Missing growth band"));
        if (B.Lower.Value<=Previous||B.Upper.Value<B.Lower.Value||B.Minimum.Value<0||B.Maximum.Value<B.Minimum.Value) return LHBibleRulesPrivate::Fail<FBibleBand>(EReason::InvalidData,TEXT("Invalid or overlapping growth bands"));
        Previous=B.Upper.Value;
        if (Attribute>=B.Lower.Value && Attribute<=B.Upper.Value) Found=&B;
    }
    return Found ? LHBibleRulesPrivate::Accept(*Found) : LHBibleRulesPrivate::Fail<FBibleBand>(EReason::Unresolved,TEXT("Outside documented growth coverage"));
}
TResult<FBibleXP> LookupBibleXP(const FBibleRuleset& R,int64 Level)
{
    if (Level<1) return LHBibleRulesPrivate::Fail<FBibleXP>(EReason::InvalidData,TEXT("Invalid level"));
    int64 Previous=-1,PreviousLevel=0; const FBibleXP* Found=nullptr;
    for (const auto& X:R.Experience)
    {
        if (!LHBibleRulesPrivate::Ready(X.Level)||!LHBibleRulesPrivate::Ready(X.Threshold)) return LHBibleRulesPrivate::Fail<FBibleXP>(EReason::Unresolved,TEXT("Missing XP row"));
        if (X.Level.Value!=PreviousLevel+1||X.Threshold.Value<=Previous||(LHBibleRulesPrivate::Ready(X.Next) && X.Next.Value<0)) return LHBibleRulesPrivate::Fail<FBibleXP>(EReason::InvalidData,TEXT("Invalid XP table"));
        Previous=X.Threshold.Value; PreviousLevel=X.Level.Value;
        if (X.Level.Value==Level) Found=&X;
    }
    return Found ? LHBibleRulesPrivate::Accept(*Found) : LHBibleRulesPrivate::Fail<FBibleXP>(EReason::Unresolved,TEXT("Outside documented XP coverage; no inferred cap"));
}
TResult<FBibleAnswer> LookupBibleAnswer(const FBibleRuleset& R,FName Theme,FName Answer)
{
    for (const auto& A:R.Answers) if (A.Theme==Theme && A.Answer==Answer)
    {
        if (A.Provenance.Status==ELHProvenanceStatus::Missing) return LHBibleRulesPrivate::Fail<FBibleAnswer>(EReason::Unresolved,TEXT("Missing affinity"));
        return LHBibleRulesPrivate::Accept(A);
    }
    return LHBibleRulesPrivate::Fail<FBibleAnswer>(EReason::InvalidData,TEXT("Unknown creation answer"));
}
TResult<bool> ValidateBibleStatCap(const FBibleRuleset& R,const FAttributes& A)
{
    if (!LHBibleRulesPrivate::Ready(R.StatCap)) return LHBibleRulesPrivate::Fail<bool>(EReason::Unresolved,TEXT("Missing stat cap"));
    if (R.StatCap.Value<0) return LHBibleRulesPrivate::Fail<bool>(EReason::InvalidData,TEXT("Invalid cap"));
    for (int64 V:{A.Strength,A.Endurance,A.Agility,A.Intelligence,A.Wisdom})
        if (V<0||V>R.StatCap.Value) return LHBibleRulesPrivate::Fail<bool>(EReason::Ineligible,TEXT("Attribute outside chart cap"));
    return LHBibleRulesPrivate::Accept(true); // Does not claim minima, total budget or RNG validity.
}
TResult<double> BibleEncumbrance(const FBibleRuleset& R,int64 Strength)
{
    if (!LHBibleRulesPrivate::Ready(R.CapacityScale)||!LHBibleRulesPrivate::Ready(R.CapacityOffset)) return LHBibleRulesPrivate::Fail<double>(EReason::Unresolved,TEXT("Missing capacity formula"));
    if (Strength<0||!std::isfinite(R.CapacityScale.Value)||!std::isfinite(R.CapacityOffset.Value)||R.CapacityScale.Value<0||R.CapacityOffset.Value<=0) return LHBibleRulesPrivate::Fail<double>(EReason::InvalidData,TEXT("Invalid capacity input"));
    const double V=double(Strength)/(double(Strength)+R.CapacityOffset.Value)*R.CapacityScale.Value;
    return std::isfinite(V) ? LHBibleRulesPrivate::Accept(V) : LHBibleRulesPrivate::Fail<double>(EReason::Overflow,TEXT("Capacity overflow"));
}
TResult<bool> CheckBibleRequirements(const FBibleAcquisition& R,EAttributeBasis Basis,const FRequirementInput& Input)
{
    if (Basis==EAttributeBasis::Unresolved) return LHBibleRulesPrivate::Fail<bool>(EReason::Unresolved,TEXT("Requirement stat basis missing"));
    if (Basis!=EAttributeBasis::Base && Basis!=EAttributeBasis::Effective) return LHBibleRulesPrivate::Fail<bool>(EReason::InvalidData,TEXT("Unknown stat basis"));
    const auto& A=Basis==EAttributeBasis::Base ? Input.Base:Input.Effective;
    const FLHInteger Min[]={R.Minimum.Strength,R.Minimum.Endurance,R.Minimum.Agility,R.Minimum.Intelligence,R.Minimum.Wisdom,R.Level};
    const int64 Values[]={A.Strength,A.Endurance,A.Agility,A.Intelligence,A.Wisdom,Input.Level};
    for (int32 Index=0;Index<6;++Index)
    {
        if (!LHBibleRulesPrivate::Ready(Min[Index])) return LHBibleRulesPrivate::Fail<bool>(EReason::Unresolved,TEXT("Missing requirement"));
        if (Min[Index].Value<0||Values[Index]<0) return LHBibleRulesPrivate::Fail<bool>(EReason::InvalidData,TEXT("Negative requirement input"));
    }
    if (!R.RequiredSkill.IsNone() && !LHBibleRulesPrivate::Ready(R.SkillRank)) return LHBibleRulesPrivate::Fail<bool>(EReason::Unresolved,TEXT("Missing skill rank"));
    for (int32 Index=0;Index<6;++Index) if (Values[Index]<Min[Index].Value) return LHBibleRulesPrivate::Fail<bool>(EReason::Ineligible,TEXT("Below requirement"));
    for (FName Name:R.Prerequisites)
    {
        bool Found=false; for (const auto& S:Input.Spells) Found|=S.Value==Name;
        if (!Found) return LHBibleRulesPrivate::Fail<bool>(EReason::Ineligible,TEXT("Missing prerequisite spell"));
    }
    if (!R.RequiredSkill.IsNone())
    {
        bool Found=false; for (const auto& S:Input.Skills) if (S.Skill.Value==R.RequiredSkill && LHBibleRulesPrivate::Ready(S.TrainedValue) && S.TrainedValue.Value>=R.SkillRank.Value) Found=true;
        if (!Found) return LHBibleRulesPrivate::Fail<bool>(EReason::Ineligible,TEXT("Missing trained skill"));
    }
    return LHBibleRulesPrivate::Accept(true);
}
TResult<int64> BibleSkillCost(const FBibleAcquisition& R,bool bTraining)
{
    if (!LHBibleRulesPrivate::Ready(R.CostUnit)) return LHBibleRulesPrivate::Fail<int64>(EReason::Unresolved,TEXT("Printed skill cost unit missing"));
    return BibleCost(bTraining ? R.TrainCost:R.LearnCost);
}
}

namespace LH::Rules
{
TResult<FBibleRoll> LookupBibleRoll(const FBibleRuleset& R,const TArray<int32>& Counts)
{
    if (!LHBibleRulesPrivate::Ready(R.AnswerCount)) return LHBibleRulesPrivate::Fail<FBibleRoll>(EReason::Unresolved,TEXT("Missing answer count"));
    if (Counts.Num()!=5) return LHBibleRulesPrivate::Fail<FBibleRoll>(EReason::InvalidData,TEXT("Five category counts required"));
    int64 Total=0; for (int32 C:Counts) { if (C<0) return LHBibleRulesPrivate::Fail<FBibleRoll>(EReason::InvalidData,TEXT("Negative category count")); Total+=C; }
    if (Total!=R.AnswerCount.Value) return LHBibleRulesPrivate::Fail<FBibleRoll>(EReason::InvalidData,TEXT("Wrong answer count"));
    const FString Names[]={TEXT("Str"),TEXT("Agi"),TEXT("Wis"),TEXT("Int"),TEXT("N/A")};
    for (const auto& Row:R.Rolls)
    {
        TArray<int32> RowCounts; RowCounts.Init(0,5); TArray<FString> Parts; Row.Categories.ParseIntoArray(Parts,TEXT(","),true);
        bool Valid=true;
        for (FString Part:Parts)
        {
            Part.TrimStartAndEndInline(); int32 Count=1; FString Category=Part;
            if (!Part.IsEmpty() && FChar::IsDigit(Part[0])) { Count=Part[0]-TCHAR('0'); Category=Part.Mid(2); }
            int32 Index=INDEX_NONE; for (int32 I=0;I<5;++I) if (Category==Names[I]) Index=I;
            if (Index==INDEX_NONE) { Valid=false; break; } RowCounts[Index]+=Count;
        }
        if (!Valid) return LHBibleRulesPrivate::Fail<FBibleRoll>(EReason::InvalidData,TEXT("Invalid chart category"));
        if (RowCounts==Counts)
        {
            const auto& A=Row.Attributes;
            if (!LHBibleRulesPrivate::Ready(Row.PrintedTotal)||!LHBibleRulesPrivate::Ready(A.Strength)||!LHBibleRulesPrivate::Ready(A.Endurance)||!LHBibleRulesPrivate::Ready(A.Agility)||!LHBibleRulesPrivate::Ready(A.Intelligence)||!LHBibleRulesPrivate::Ready(A.Wisdom)) return LHBibleRulesPrivate::Fail<FBibleRoll>(EReason::Unresolved,TEXT("Missing chart value"));
            auto Cap=ValidateBibleStatCap(R,{A.Strength.Value,A.Endurance.Value,A.Agility.Value,A.Intelligence.Value,A.Wisdom.Value});
            if (!Cap.Diagnostic.IsAccepted()) return LHBibleRulesPrivate::Fail<FBibleRoll>(Cap.Diagnostic.Reason,TEXT("Chart exceeds cap"));
            return LHBibleRulesPrivate::Accept(Row);
        }
    }
    return LHBibleRulesPrivate::Fail<FBibleRoll>(EReason::Unresolved,TEXT("Combination absent from representative chart"));
}
TResult<int64> BibleCombinedMana(const FBibleRuleset& R,int64 Intelligence,int64 Wisdom)
{
    // A resolved integer cannot specify a combination algorithm; no supported law yet.
    return LHBibleRulesPrivate::Fail<int64>(EReason::Unresolved,TEXT("INT/WIS combination and RNG law missing; query components separately"));
}
}
