#include "Framework/LHWave2Closure.h"
#include "Persistence/LHSaveCodec.h"
#include "Framework/LHWave2Profile.h"
#include "World/LHAreaRegistry.h"
#include "Framework/LHArrivalReview.h"
#include "Data/Enemies/LHEnemyCatalog.h"
#include "Data/Items/LHItemCatalog.h"
#include "Data/Encounters/LHEncounterCatalog.h"
#include "Abilities/LHAbilityCatalog.h"
#include "Services/LHServiceCatalog.h"
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
FBytes Policy(const FLHPolicyField& P)
{ return Struct({{TEXT("Key"),Value(P.Key)},{TEXT("Value"),Value(P.Value)},{TEXT("Resolution"),Value(int32(P.Resolution))},{TEXT("Provenance"),Value(P.Provenance)}}); }
FBytes LootEntry(const FLHLootEntry& L)
{ return Struct({{TEXT("Item"),Value(L.Item)},{TEXT("Minimum"),Value(L.MinimumQuantity)},{TEXT("Maximum"),Value(L.MaximumQuantity)},{TEXT("Weight"),Value(L.Weight)}}); }
FBytes Combat(const LH::Rules::FCombatParameters& C)
{
    return Struct({{TEXT("Model"),Value(int32(C.Model))},
        {TEXT("HitBase"),Value(C.HitBase)},
        {TEXT("AccuracyScale"),Value(C.AccuracyScale)},
        {TEXT("AvoidanceScale"),Value(C.AvoidanceScale)},
        {TEXT("MinimumChance"),Value(C.MinimumChance)},
        {TEXT("MaximumChance"),Value(C.MaximumChance)},
        {TEXT("ArmorScale"),Value(C.ArmorScale)},
        {TEXT("ResistanceScale"),Value(C.ResistanceScale)},
        {TEXT("MinimumDamage"),Value(C.MinimumDamage)},
        {TEXT("DamageQuantum"),Value(C.DamageQuantum)}});
}
FBytes Stage1Catalogs()
{
    TArray<FBytes> Enemies,Items,Abilities,Encounters,Offers;
    for (const auto& E:LHEnemyData::Catalog())
    {
        const auto& R=E.Runtime; const auto& K=E.Reward;
        TArray<FBytes> AI,Loot;
        for (const auto& A:E.AIParameters) AI.Add(Value(A));
        for (const auto& L:K.Loot) Loot.Add(LootEntry(L));
        Enemies.Add(Struct({{TEXT("Id"),Value(E.Id)},{TEXT("Presentation"),Value(E.PresentationId)},
            {TEXT("Level"),Value(E.Level)},{TEXT("XP0"),Value(E.ExperienceColumns[0])},{TEXT("XP1"),Value(E.ExperienceColumns[1])},{TEXT("XP2"),Value(E.ExperienceColumns[2])},
            {TEXT("GoldMinimum"),Value(E.GoldMinimum)},{TEXT("GoldMaximum"),Value(E.GoldMaximum)},
            {TEXT("Health"),Value(E.Health)},{TEXT("MinDamage"),Value(E.MinimumDamage)},{TEXT("MaxDamage"),Value(E.MaximumDamage)},
            {TEXT("LootMembership"),Value(E.LootMembership)},{TEXT("AI"),Ordered(AI)},
            {TEXT("Runtime"),Struct({{TEXT("Id"),Value(R.ContentId)},{TEXT("Presentation"),Value(R.PresentationId)},{TEXT("Boss"),Value(R.bBoss)},
                {TEXT("CapsuleRadiusCm"),Value(R.CapsuleRadiusCm)},
                {TEXT("CapsuleHalfHeightCm"),Value(R.CapsuleHalfHeightCm)},
                {TEXT("MaxHealth"),Value(R.MaxHealth)},
                {TEXT("Accuracy"),Value(R.Accuracy)},
                {TEXT("Avoidance"),Value(R.Avoidance)},
                {TEXT("Armor"),Value(R.Armor)},
                {TEXT("Resistance"),Value(R.Resistance)},
                {TEXT("DamageBonus"),Value(R.DamageBonus)},
                {TEXT("MaxMana"),Value(R.MaxMana)},
                {TEXT("AggroRadiusCm"),Value(R.AggroRadiusCm)},
                {TEXT("LeashRadiusCm"),Value(R.LeashRadiusCm)},
                {TEXT("MoveSpeedCmPerSec"),Value(R.MoveSpeedCmPerSec)},
                {TEXT("PathRetryLimit"),Value(R.PathRetryLimit)},
                {TEXT("MaxActivePursuers"),Value(R.MaxActivePursuers)},
                {TEXT("DecisionIntervalSeconds"),Value(R.DecisionIntervalSeconds)},
                {TEXT("HomeToleranceCm"),Value(R.HomeToleranceCm)},
                {TEXT("SpawnSafetyDistanceCm"),Value(R.SpawnSafetyDistanceCm)},
                {TEXT("AttackCombat"),Combat(R.Attack.Combat)},{TEXT("AttackId"),Value(R.Attack.Ability)},
                {TEXT("AttackMana"),Value(R.Attack.ManaCost)},{TEXT("AttackCooldown"),Value(R.Attack.CooldownSeconds)},
                {TEXT("AttackImpact"),Value(R.Attack.ImpactSeconds)},{TEXT("AttackRange"),Value(R.Attack.RangeCm)},
                {TEXT("AttackMin"),Value(R.Attack.WeaponMinimum)},{TEXT("AttackMax"),Value(R.Attack.WeaponMaximum)},
                {TEXT("AttackEligibility"),Eligibility(R.Attack.Eligibility)}})},
            {TEXT("Reward"),Struct({{TEXT("XP"),Value(K.Experience)},{TEXT("GoldMin"),Value(K.GoldMin)},{TEXT("GoldMax"),Value(K.GoldMax)},
                {TEXT("DropChance"),Value(K.ItemDropChance)},{TEXT("Loot"),Ordered(Loot)},{TEXT("Respawn"),Value(K.RespawnSeconds)},
                {TEXT("Safety"),Value(K.SafetyDistanceCm)},{TEXT("Policy"),Value(int32(K.RespawnPolicy))},{TEXT("Boss"),Value(K.bBoss)}})}}));
    }
    for (const auto& I:LHItemData::Catalog())
    {
        const auto& C=I.Combat; const auto& W=C.Weapon;
        Items.Add(Struct({{TEXT("Id"),Value(I.Id)},{TEXT("Buy"),Value(I.BuyGold)},{TEXT("Sell"),Value(I.SellGold)},
            {TEXT("Encumbrance"),Value(I.Encumbrance)},{TEXT("Carry"),Policy(I.CarryPolicy)},{TEXT("Membership"),Value(I.Membership)},
            {TEXT("Equipable"),Value(I.bEquipable)},{TEXT("Character"),LHWave2ClosurePrivate::Items({I.Character})},
            {TEXT("Weapon"),Value(C.bWeapon)},{TEXT("Quiver"),Value(C.bQuiver)},{TEXT("Consumable"),Value(C.bConsumable)},
            {TEXT("Min"),Value(W.MinimumDamage)},{TEXT("Max"),Value(W.MaximumDamage)},{TEXT("Range"),Value(W.RangeCm)},
            {TEXT("Bow"),Value(W.bBow)},{TEXT("Quivers"),IdSet(W.CompatibleQuivers)},{TEXT("QuiverBonus"),Value(W.QuiverDamageBonus)},
            {TEXT("Consumption"),Value(int32(W.QuiverConsumption))},{TEXT("ManaRestore"),Value(C.Consumable.ManaRestore)}}));
    }
    for (const auto& A:LHAbilities::Catalog())
        Abilities.Add(Struct({{TEXT("Id"),Value(A.Id)},{TEXT("Eligibility"),Eligibility(A.Eligibility)},
            {TEXT("LearningSkillPoints"),Value(A.LearningSkillPoints)},
            {TEXT("LearningGold"),Value(A.LearningGold)},
            {TEXT("ManaCost"),Value(A.ManaCost)},
            {TEXT("RangeCm"),Value(A.RangeCm)},
            {TEXT("CooldownSeconds"),Value(A.CooldownSeconds)},
            {TEXT("ImpactSeconds"),Value(A.ImpactSeconds)},
            {TEXT("MinimumMagnitude"),Value(A.MinimumMagnitude)},
            {TEXT("MaximumMagnitude"),Value(A.MaximumMagnitude)},
            {TEXT("DurationSeconds"),Value(A.DurationSeconds)},
            {TEXT("Effects"),Value(int32(A.Effects))},{TEXT("Target"),Value(int32(A.TargetPolicy))},
            {TEXT("Classification"),Value(A.ClassificationTag)},{TEXT("Refund"),Value(A.RefundPolicy)}}));
    for (const auto& E:LHEncounterData::Catalog())
    {
        TArray<FBytes> Policies; for (const auto& P:E.Policies) Policies.Add(Policy(P));
        Encounters.Add(Struct({{TEXT("Spawn"),Value(E.SpawnId.ToString(EGuidFormats::Digits))},{TEXT("Area"),Value(E.Area.Content)},{TEXT("Enemy"),Value(E.Enemy)},
            {TEXT("Respawn"),Value(E.RespawnSeconds)},{TEXT("Safety"),Value(E.SafetyDistanceCm)},
            {TEXT("Policy"),Value(int32(E.RespawnPolicy))},{TEXT("Boss"),Value(E.bBoss)},
            {TEXT("PlacementResolution"),Value(int32(E.PlacementResolution))},{TEXT("PlacementProvenance"),Value(E.PlacementProvenance)},
            {TEXT("Policies"),Ordered(Policies)}}));
    }
    for (const auto& O:LHServices::Catalog()) Offers.Add(Struct({{TEXT("Id"),Value(O.Id)},{TEXT("Npc"),Value(O.Npc)},
        {TEXT("Subject"),Value(O.Subject)},{TEXT("Area"),Value(O.Area)},{TEXT("Kind"),Value(int32(O.Kind))},
        {TEXT("Gold"),Value(O.Gold)},{TEXT("Points"),Value(O.SkillPoints)},{TEXT("Cap"),Value(O.RankCap)}}));
    return Struct({{TEXT("Enemies"),Ordered(Enemies)},{TEXT("Items"),Ordered(Items)},{TEXT("Abilities"),Ordered(Abilities)},
        {TEXT("Encounters"),Ordered(Encounters)},{TEXT("Offers"),Ordered(Offers)},
        {TEXT("Quests"),Text(TEXT("LHQuests.v1/SamaritanRats.15.2500/Nevanis.FullHP.0Gold/BalorkReturn.Kiran.SingleClaim"))},
        {TEXT("StarterSkills"),Text(TEXT("Attack.10/Dodge.10"))},
        {TEXT("RuntimePolicies"),Text(TEXT("G4.CatalogAbilities.Services.AllFloors/EnemyLifeRng.Cooldowns.v1/Npc250.LOS/Loot200.LOS/Death.FullPools.NoPenalty.LightClear/Light.ActiveOnly.CaptureRestore.AreaRemap.v1/Corpse300/NativeCatalogs.v1"))}});
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
        {TEXT("CarryCapacityPolicy"),Text(LHWave2::CarryCapacityPolicy().bUnlimited?TEXT("Unlimited; deferred by owner decision"):TEXT("Limited"))},{TEXT("Capacity"),Linear(S.Capacity)},{TEXT("DamageBonus"),Linear(S.DamageBonus)}});
    return Struct({{TEXT("Algorithms"),Text(TEXT("LHMechanicalClosure1/LHCanonicalBinary1/LHRequest1/LHReward1/UE.FRandomStream.1/finiteTable.1"))},
        {TEXT("CombatPolicy"),Combat(P.Rules.Combat)},{TEXT("Creation"),Creation},{TEXT("CreationPolicy"),Value(P.CreationPolicy)},
        {TEXT("CreationRevision"),Value(P.CreationRevision)},{TEXT("GrowthBasis"),Basis(P.GrowthBasis)},{TEXT("InitialGold"),Value(P.InitialGold)},
        {TEXT("InitialHealth"),Value(P.InitialHealth)},{TEXT("InitialMana"),Value(P.InitialMana)},{TEXT("InitialSkillPoints"),Value(P.InitialSkillPoints)},
        {TEXT("InventorySlots"),Value(P.InventorySlots)},{TEXT("Items"),Items(P.Items)},{TEXT("ManaPolicy"),Struct({{TEXT("Amount"),Value(P.Rules.Mana.Amount)},{TEXT("Interval"),Value(P.Rules.Mana.IntervalSeconds)}})},
        {TEXT("Progression"),Progression},{TEXT("RequirementBasis"),Basis(P.Rules.Requirements.Basis)},
        {TEXT("RequirementBasisProvenance"),Value(P.Rules.Requirements.BasisProvenance)},{TEXT("RequiresQuiver"),Value(P.Rules.Requirements.BowRequiresQuiver)},
        {TEXT("RulesetId"),Value(P.Reference.Id)},{TEXT("RulesetRevision"),Value(P.Reference.Revision)},
        {TEXT("StarterItems"),IdSet(P.StarterItems)},{TEXT("Stats"),Stats}});
}
TArray<uint8> LHWave2::GameplayCatalogClosure()
{
    using namespace LHWave2ClosurePrivate;
    TArray<FBytes> Areas;
    for (const auto& A:LHWorld::Registry())
    {
        TArray<FBytes> Entrances,Portals,Spawns;
        for (const auto& E:A.Entrances) Entrances.Add(Struct({{TEXT("Id"),Value(LHArrivalReview::Key(E.Id))},{TEXT("Transform"),Value(E.SafeTransform)}}));
        for (const auto& P:A.Portals) Portals.Add(Struct({{TEXT("Id"),Value(P.Portal.InstanceId.ToString())},{TEXT("Source"),Value(LHArrivalReview::Key(P.Source))},{TEXT("Destination"),Value(LHArrivalReview::Key(P.Destination))}}));
        for (const auto& P:A.Spawns) Spawns.Add(Struct({{TEXT("Id"),Value(P.SpawnId.ToString())},{TEXT("Enemy"),Value(P.Enemy)},{TEXT("Anchor"),Value(P.Anchor)}}));
        Areas.Add(Struct({{TEXT("Area"),Value(A.Id.Content)},{TEXT("Map"),Value(A.Map.ToString())},
            {TEXT("Entrances"),Ordered(Entrances)},{TEXT("Portals"),Ordered(Portals)},{TEXT("Spawns"),Ordered(Spawns)}}));
    }
    return Struct({{TEXT("Codec"),Text(TEXT("LHGameplayCatalog5"))},{TEXT("Stage1Catalogs"),Stage1Catalogs()},{TEXT("Areas"),Ordered(Areas)},
        {TEXT("MechanicalClosure"),LHWave2::MechanicalClosure(LHWave2::PrototypeProfile())}});
}
