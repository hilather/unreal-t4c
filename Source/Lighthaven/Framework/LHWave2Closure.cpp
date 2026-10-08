#include "Framework/LHWave2Closure.h"
#include "Persistence/LHSaveCodec.h"
#include "Framework/LHWave2Profile.h"
namespace LHWave2ClosurePrivate
{
using FBytes=TArray<uint8>;
using FFields=TMap<FString,FBytes>;
void Count(FBytes& Out,int32 N) { for (int32 I=0;I<4;++I) Out.Add(uint8(uint32(N)>>(I*8))); }
template<class T> FBytes Value(T V) { FLHSaveError E; auto B=LHSave::CanonicalValue(V,E); check(E.Reason==ELHSaveReason::None); return B; }
FBytes Text(const TCHAR* S) { return Value(FString(S)); }
FBytes Struct(FFields Fields)
{
    FBytes R; Count(R,Fields.Num()); TArray<FString> Keys; Fields.GetKeys(Keys); Keys.Sort();
    for (const auto& K:Keys) { R.Append(Value(K)); R.Append(Fields[K]); } return R;
}
FBytes Ordered(const TArray<FBytes>& Values)
{ FBytes R; Count(R,Values.Num()); for (const auto& V:Values) R.Append(V); return R; }
bool Less(const FBytes& A,const FBytes& B)
{ for (int32 I=0;I<FMath::Min(A.Num(),B.Num());++I) if (A[I]!=B[I]) return A[I]<B[I]; return A.Num()<B.Num(); }
FBytes IdSet(const TArray<FLHContentId>& Values)
{
    auto Sorted=Values; Sorted.Sort([](const auto& A,const auto& B){return Less(Value(A.Value),Value(B.Value));});
    TArray<FBytes> R; for (const auto& V:Sorted) R.Add(Value(V)); return Ordered(R);
}
FBytes Attributes(const LH::Rules::FAttributeParameters& A)
{ FLHAttributeBlock B; B.Strength=A.Strength; B.Endurance=A.Endurance; B.Agility=A.Agility; B.Intelligence=A.Intelligence; B.Wisdom=A.Wisdom; return Value(B); }
FBytes Linear(const LH::Rules::FLinearFormula& F)
{ return Struct({{TEXT("Agility"),Value(F.Agility)},{TEXT("Constant"),Value(F.Constant)},{TEXT("Endurance"),Value(F.Endurance)},{TEXT("Intelligence"),Value(F.Intelligence)},{TEXT("Strength"),Value(F.Strength)},{TEXT("Wisdom"),Value(F.Wisdom)}}); }
FBytes Modifier(const LH::Rules::FModifier& M)
{ return Struct({{TEXT("Accuracy"),Value(M.Accuracy)},{TEXT("Armor"),Value(M.Armor)},{TEXT("Attributes"),Attributes(M.Attributes)},{TEXT("Avoidance"),Value(M.Avoidance)},{TEXT("Capacity"),Value(M.Capacity)},{TEXT("DamageBonus"),Value(M.DamageBonus)},{TEXT("Health"),Value(M.Health)},{TEXT("Mana"),Value(M.Mana)}}); }
FBytes Eligibility(const FLHEligibility& E)
{
    // Enabled synthetic catalog has explicitly empty ancillary eligibility arrays. Changes require an adapter revision.
    check(E.Policies.IsEmpty() && E.SkillMinimums.IsEmpty());
    return Struct({{TEXT("MinimumAttributes"),Value(E.MinimumAttributes)},{TEXT("MinimumLevel"),Value(E.MinimumLevel)},
        {TEXT("Policies"),Ordered({})},{TEXT("RequiredSkills"),IdSet(E.RequiredSkills)},{TEXT("RequiredSpells"),IdSet(E.RequiredSpells)},{TEXT("SkillMinimums"),Ordered({})}});
}
FBytes Basis(LH::Rules::EAttributeBasis V)
{ return Text(V==LH::Rules::EAttributeBasis::Base?TEXT("Base"):V==LH::Rules::EAttributeBasis::Effective?TEXT("Effective"):TEXT("Unresolved")); }
FBytes Items(const TArray<FLHCharacterItemDefinition>& Items)
{
    auto Sorted=Items; Sorted.Sort([](const auto& A,const auto& B){return Less(Value(A.Id.Value),Value(B.Id.Value));});
    TArray<FBytes> R;
    for (const auto& I:Sorted) R.Add(Struct({{TEXT("Bow"),Value(I.bBow)},{TEXT("CompatibleQuivers"),IdSet(I.CompatibleQuivers)},
        {TEXT("Eligibility"),Eligibility(I.Eligibility)},{TEXT("Id"),Value(I.Id)},{TEXT("Modifier"),Modifier(I.Modifier)},
        {TEXT("Slot"),Value(I.Slot)},{TEXT("StackLimit"),Value(I.StackLimit)},{TEXT("Weight"),Value(I.Weight)}}));
    return Ordered(R);
}
}
TArray<uint8> LHWave2::MechanicalClosure(const FLHCharacterProfile& P)
{
    using namespace LHWave2ClosurePrivate;
    TArray<FBytes> Outcomes,Thresholds;
    for (const auto& O:P.Rules.Creation.Outcomes)
    { TArray<FBytes> Answers; for (const auto& A:O.Answers) Answers.Add(Value(A)); Outcomes.Add(Struct({{TEXT("Answers"),Ordered(Answers)},{TEXT("Attributes"),Attributes(O.Attributes)},{TEXT("UnspentPoints"),Value(O.UnspentPoints)}})); }
    for (const auto& T:P.Rules.Progression.Thresholds) Thresholds.Add(Value(T));
    const auto& C=P.Rules.Creation; const auto& G=P.Rules.Progression; const auto& S=P.Rules.Stats;
    const auto Creation=Struct({{TEXT("AnswerCount"),Value(C.AnswerCount)},{TEXT("Maximum"),Attributes(C.Maximum)},{TEXT("Minimum"),Attributes(C.Minimum)},
        {TEXT("Outcomes"),Ordered(Outcomes)},{TEXT("OutcomesProvenance"),Value(C.OutcomesProvenance)},
        {TEXT("OutcomesResolution"),Text(C.OutcomesResolution==ELHValueResolution::Resolved?TEXT("Resolved"):TEXT("Unresolved"))},{TEXT("TotalPoints"),Value(C.TotalPoints)}});
    const auto Progression=Struct({{TEXT("AttributePointsPerLevel"),Value(G.AttributePointsPerLevel)},{TEXT("HealthGrowth"),Linear(G.HealthGrowth)},
        {TEXT("HealthRollScale"),Value(G.HealthRollScale)},{TEXT("InitialLevel"),Value(G.InitialLevel)},{TEXT("ManaGrowth"),Linear(G.ManaGrowth)},
        {TEXT("ManaRollScale"),Value(G.ManaRollScale)},{TEXT("SkillPointsPerLevel"),Value(G.SkillPointsPerLevel)},{TEXT("Thresholds"),Ordered(Thresholds)}});
    const auto Stats=Struct({{TEXT("Accuracy"),Linear(S.Accuracy)},{TEXT("Armor"),Linear(S.Armor)},{TEXT("Avoidance"),Linear(S.Avoidance)},
        {TEXT("Capacity"),Linear(S.Capacity)},{TEXT("DamageBonus"),Linear(S.DamageBonus)}});
    return Struct({{TEXT("Algorithms"),Text(TEXT("LHMechanicalClosure1/LHCanonicalBinary1/LHRequest1/LHReward1/UE.FRandomStream.1/finiteTable.1"))},
        {TEXT("CombatPolicy"),Text(TEXT("Disabled"))},{TEXT("Creation"),Creation},{TEXT("CreationPolicy"),Value(P.CreationPolicy)},
        {TEXT("CreationRevision"),Value(P.CreationRevision)},{TEXT("GrowthBasis"),Basis(P.GrowthBasis)},{TEXT("InitialGold"),Value(P.InitialGold)},
        {TEXT("InitialHealth"),Value(P.InitialHealth)},{TEXT("InitialMana"),Value(P.InitialMana)},{TEXT("InitialSkillPoints"),Value(P.InitialSkillPoints)},
        {TEXT("InventorySlots"),Value(P.InventorySlots)},{TEXT("Items"),Items(P.Items)},{TEXT("ManaPolicy"),Text(TEXT("Disabled"))},
        {TEXT("Progression"),Progression},{TEXT("RequirementBasis"),Basis(P.Rules.Requirements.Basis)},
        {TEXT("RequirementBasisProvenance"),Value(P.Rules.Requirements.BasisProvenance)},{TEXT("RequiresQuiver"),Value(P.Rules.Requirements.BowRequiresQuiver)},
        {TEXT("RulesetId"),Value(P.Reference.Id)},{TEXT("RulesetRevision"),Value(P.Reference.Revision)},
        {TEXT("StarterItems"),IdSet(P.StarterItems)},{TEXT("Stats"),Stats}});
}
TArray<uint8> LHWave2::GameplayCatalogClosure()
{
    using namespace LHWave2ClosurePrivate;
    // Complete enabled gameplay catalog: its mechanical profile plus one temporary, authored entrance.
    // Cosmetic presentation choices intentionally do not participate.
    FLHContentId Area; Area.Value=TEXT("Area.LighthavenTempleDistrict");
    return Struct({{TEXT("Area"),Value(Area)},{TEXT("Checkpoint"),Value(FTransform::Identity)},
        {TEXT("CheckpointProvenance"),Value(LHWave2::PrototypeInteger(0).Provenance)},
        {TEXT("Codec"),Text(TEXT("LHGameplayCatalog1"))},{TEXT("Entrance"),Text(TEXT("Entry"))},
        {TEXT("Map"),Text(TEXT("/Game/Lighthaven/Maps/Dev_Movement"))},{TEXT("MechanicalClosure"),LHWave2::MechanicalClosure(LHWave2::PrototypeProfile())},
        {TEXT("Topology"),Ordered({})},{TEXT("WorldPolicy"),Text(TEXT("Empty encounters/objects/corpses/quests/bosses/claims/portals/timers/effects"))}});
}
