#include "Misc/AutomationTest.h"
#include "Persistence/LHSaveCodec.h"
#include "Abilities/LHItemUse.h"
#include "Abilities/LHAbilityCatalog.h"
#include "Abilities/LHResourceRecovery.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHManaPotionEdges,"Lighthaven.Abilities.ManaPotionEdges",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHManaPotionEdges::RunTest(const FString&)
{
    LHAbilities::FLHUseItemContext C; C.bAlive=true; C.MaximumMana=30;
    C.Snapshot.Character.CurrentMana=LHAbilities::PrototypeNumber(30,TEXT("test"));
    FLHItemInstance I; I.Id.RunId=FGuid::NewGuid(); I.Id.InstanceId=FGuid::NewGuid(); I.Id.Area.Content.Value=TEXT("Area.Test"); I.Definition.Value=TEXT("Item.PotionOfMana"); I.Quantity.Resolution=ELHValueResolution::Resolved; I.Quantity.Value=2; C.Snapshot.Character.Inventory.Add(I);
    FLHCombatItemData D; D.bConsumable=true; D.Consumable.ManaRestore=LHAbilities::PrototypeNumber(25,TEXT("fixture: Bible Potions +25"));
    C.ItemLookup=[&](const FLHContentId&){return &D;}; FLHUseItemRequest R; R.Item=I.Id;
    TestTrue(TEXT("Full mana rejects"),LHAbilities::ExecuteUseItem(C,R)==ELHCommandReason::NoEffect);
    TestEqual(TEXT("Unconsumed"),C.Snapshot.Character.Inventory[0].Quantity.Value,int64(2));
    C.Snapshot.Character.CurrentMana.Value=20;
    TestTrue(TEXT("Partial accepts"),LHAbilities::ExecuteUseItem(C,R)==ELHCommandReason::None);
    TestEqual(TEXT("Clamped"),C.Snapshot.Character.CurrentMana.Value,30.0);
    TestEqual(TEXT("Exactly one"),C.Snapshot.Character.Inventory[0].Quantity.Value,int64(1));
    D.bConsumable=false; TestTrue(TEXT("Not usable"),LHAbilities::ExecuteUseItem(C,R)==ELHCommandReason::NotUsable);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHRecoveryCarry,"Lighthaven.Abilities.RecoveryCarry",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHRecoveryCarry::RunTest(const FString&)
{
    FLHSaveSnapshot S; S.Character.CurrentHealth=LHAbilities::PrototypeNumber(1,TEXT("test")); S.Character.EarnedBaseMana=LHAbilities::PrototypeNumber(10,TEXT("test")); S.Character.CurrentMana=LHAbilities::PrototypeNumber(0,TEXT("test")); S.Session.ManaRegenFractionalSeconds=LHAbilities::PrototypeNumber(0,TEXT("test"));
    const auto P=LH::Rules::MakeLedgerPrototypeRuleset().Mana;
    TestTrue(TEXT("Partial tick"),LHAbilities::AdvanceManaRegen(S,4,P)); TestEqual(TEXT("Carry"),S.Session.ManaRegenFractionalSeconds.Value,4.0);
    TestTrue(TEXT("No load catchup"),LHAbilities::AdvanceManaRegen(S,0,P)); TestEqual(TEXT("No mana"),S.Character.CurrentMana.Value,0.0);
    TestTrue(TEXT("One tick"),LHAbilities::AdvanceManaRegen(S,1,P)); TestEqual(TEXT("One MP"),S.Character.CurrentMana.Value,1.0);
    S.Character.CurrentHealth.Value=0; TestTrue(TEXT("Dead no tick"),LHAbilities::AdvanceManaRegen(S,100,P)); TestEqual(TEXT("Dead unchanged"),S.Character.CurrentMana.Value,1.0);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHRegenReload,"Lighthaven.Abilities.RegenTimerReload",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHRegenReload::RunTest(const FString&)
{
    FLHSaveSnapshot S; S.Header.TransactionSequence=1; S.Header.BuildId=TEXT("Automation.W4Recovery"); S.Header.CharacterId.Value=FGuid::NewGuid(); S.Header.ChecksumAlgorithm=TEXT("SHA256"); S.Header.PayloadCodec=TEXT("LHCanonicalBinary1"); S.Header.Ruleset.Id.Value=TEXT("Ruleset.Test"); S.Header.Ruleset.Revision=1; S.Header.Ruleset.HashAlgorithm=TEXT("SHA256"); S.Header.Ruleset.ContentHash=FString::ChrN(64,'a'); S.Header.ContentRevision=FString::ChrN(64,'b'); S.World.RunId=FGuid::NewGuid(); S.Session.RequestEpoch=FGuid::NewGuid(); S.Session.EffectPolicy=ELHEffectSavePolicy::CompletedActionBoundaryOnly;
    auto N=[](double V){return LHAbilities::PrototypeNumber(V,TEXT("Synthetic codec recovery fixture"));}; auto I=[&](int64 V){FLHInteger R; R.Value=V; R.Resolution=ELHValueResolution::Resolved; R.Provenance=N(V).Provenance; return R;};
    auto& C=S.Character; C.DisplayName=TEXT("Recovery"); C.BaseAttributes.Strength=I(20); C.BaseAttributes.Endurance=I(20); C.BaseAttributes.Agility=I(20); C.BaseAttributes.Intelligence=I(20); C.BaseAttributes.Wisdom=I(20); C.Creation.AcceptedAttributes=C.BaseAttributes; C.Creation.GenerationRevision=I(1); C.Creation.GenerationPolicy.Value=TEXT("CreationPolicy.Test"); C.EarnedLevel=I(1); C.ExperienceBalance=I(0); C.ExperienceDebt=I(0); C.UnspentAttributePoints=I(0); C.UnspentSkillPoints=I(0); C.Gold=I(0); C.CurrentHealth=N(30); C.EarnedBaseHealth=N(30); C.CurrentMana=N(0); C.EarnedBaseMana=N(10); C.ActiveEntrance.Area.Content.Value=TEXT("Area.LighthavenTempleDistrict"); C.ActiveEntrance.LocalId=TEXT("SafeSpawn");
    FLHAreaRecord A; A.Area=C.ActiveEntrance.Area; S.World.Areas.Add(A); S.Session.SafeRespawn.Entrance=C.ActiveEntrance; S.Session.SafeRespawn.TransformResolution=ELHValueResolution::Resolved; S.Session.SafeRespawn.SafeTransform=FTransform::Identity; S.Session.ManaRegenFractionalSeconds=N(4);
    FLHCooldownRecord CD; CD.Owner.RunId=S.World.RunId; CD.Owner.Area=A.Area; CD.Owner.InstanceId=S.Header.CharacterId.Value; CD.Ability.Value=TEXT("Spell.FireDart"); CD.RemainingSeconds=N(1.5); S.Session.Cooldowns.Add(CD);
    TArray<uint8> Bytes; FLHSaveError Error;
    if(!TestTrue(TEXT("Encode"),LHSave::Encode(S,Bytes,Error))) {AddError(Error.Detail);return false;}
    FLHSaveSnapshot Loaded; FLHSaveCompatibility Compatibility; Compatibility.Ruleset=S.Header.Ruleset; Compatibility.ContentRevision=S.Header.ContentRevision;
    if(!TestTrue(TEXT("Decode"),LHSave::Decode(Bytes,S.Header.CharacterId,Compatibility,Loaded,Error))) {AddError(Error.Detail);return false;}
    TestEqual(TEXT("Cooldown serialized"),Loaded.Session.Cooldowns[0].RemainingSeconds.Value,1.5);
    const auto P=LH::Rules::MakeLedgerPrototypeRuleset().Mana;
    TestTrue(TEXT("Load adds no tick"),LHAbilities::AdvanceManaRegen(Loaded,0,P)); TestEqual(TEXT("Carry preserved"),Loaded.Session.ManaRegenFractionalSeconds.Value,4.0);
    TestTrue(TEXT("Resume completes tick"),LHAbilities::AdvanceManaRegen(Loaded,1,P)); TestEqual(TEXT("One MP"),Loaded.Character.CurrentMana.Value,1.0);return true;
}
#endif
