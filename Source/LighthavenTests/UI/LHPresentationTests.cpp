#include "Misc/AutomationTest.h"
#include "UI/LHPresentation.h"
#include "UI/LHUIStyle.h"
#include "Input/LHInputGlyphs.h"
#include "Rewards/LHEncounterLifecycle.h"
#include "Persistence/LHSaveCodec.h"
#include "Framework/LHWave2Profile.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#if WITH_DEV_AUTOMATION_TESTS
namespace LHPresentationTestsPrivate
{
FLHInteger I(int64 V) { return LHWave2::PrototypeInteger(V); }
FLHNumber N(double V) { return LHWave2::PrototypeNumber(V); }
struct FFixture
{
    FLHCharacterProfile Profile=LHWave2::PrototypeProfile();
    FLHSaveSnapshot S;
    bool Init()
    {
        FLHCharacterAuthority A; if (!A.Initialize(Profile,FGuid(1,2,3,4),42)) return false;
        FLHCharacterPreview V; if (A.Preview(LHWave2::PrototypeAnswers(),V)!=ELHCommandReason::None) return false;
        FLHCreateCharacterRequest R; R.Request.Epoch=FGuid(1,2,3,4); R.Request.Value=FGuid(5,6,7,8); R.DisplayName=TEXT("Reward fixture"); R.Creation=V.Creation; R.PreviewToken=V.Token;
        for (const TCHAR* Name:{TEXT("Presentation.Player.Body.A"),TEXT("Presentation.Player.Hair.Cropped"),TEXT("Presentation.Player.Skin.LightWarm"),TEXT("Presentation.Player.Outfit.StarterLinen")}) { FLHContentId ID; ID.Value=Name; R.AppearanceIds.Add(ID); }
        if (A.Execute(R).Disposition!=ELHCommandDisposition::Accepted) return false;
        A.Export(S); S.Header.BuildId=TEXT("W4RewardFixture"); S.Header.ContentRevision=LHWave2::CatalogHash(); S.Header.ChecksumAlgorithm=TEXT("SHA256"); S.Header.PayloadCodec=TEXT("LHCanonicalBinary1");
        S.Character.ActiveEntrance=LHWorld::Registry()[1].SafeFallback;
        S.Session.SafeRespawn.Entrance=LHWorld::Registry()[0].SafeFallback; S.Session.SafeRespawn.TransformResolution=ELHValueResolution::Resolved; S.Session.SafeRespawn.SafeTransform=LHWorld::FindEntrance(S.Session.SafeRespawn.Entrance)->SafeTransform;
        S.Session.ManaRegenFractionalSeconds=N(0); S.Session.EffectPolicy=ELHEffectSavePolicy::CompletedActionBoundaryOnly;
        FLHAreaRecord Hub; Hub.Area=LHWorld::Registry()[0].Id; S.World.Areas.Add(Hub);
        FLHAreaRecord B1; LHRewards::PopulateArea(B1,LHWorld::Registry()[1],[](const auto&) { return N(25); }); S.World.Areas.Add(B1);
        FLHRngState Rng; Rng.StreamId=TEXT("RNG.Loot"); Rng.Algorithm=TEXT("UE.FRandomStream"); Rng.AlgorithmRevision=1; Rng.State={42,0,0,0}; S.Session.GameplayRng.Add(Rng);
        FLHSaveError E; if (!LHSave::Validate(S,E)) return false;
        FLHCharacterAuthority Imported; if (!Imported.Initialize(Profile,S.Session.RequestEpoch,0)) return false;
        return Imported.Import(S)==ELHCommandReason::None;
    }
    FLHAreaRecord& B1() { return *S.World.Areas.FindByPredicate([](const auto& A) { return A.Area.Content.Value==TEXT("Area.TempleB1"); }); }
    const FLHAreaRecord& B1() const { return *S.World.Areas.FindByPredicate([](const auto& A) { return A.Area.Content.Value==TEXT("Area.TempleB1"); }); }
    FLHEncounterRecord& Encounter(const FGuid& Slot) { return *B1().Encounters.FindByPredicate([&](const auto& E) { return E.Life.SpawnSlot==Slot; }); }
    FLHKillFacts Facts(int32 Index=0) const
    { const auto& E=B1().Encounters[Index]; FLHKillFacts F; F.Area=E.Life.Area; F.Life=E.Life; F.Enemy=E.Definition; F.bKillerIsPlayer=true; return F; }
    FLHKillRewardSpec Spec(bool Item=true) const
    {
        FLHKillRewardSpec R; R.Experience=I(10); R.GoldMin=I(2); R.GoldMax=I(2); R.ItemDropChance=N(Item?1:0); R.RespawnSeconds=N(120); R.SafetyDistanceCm=N(1000);
        if (Item) { FLHLootEntry L; L.Item=Profile.Items[0].Id; L.MinimumQuantity=L.MaximumQuantity=I(1); L.Weight=N(1); R.Loot.Add(L); }
        return R;
    }
    ELHCommandReason Kill(int32 Index=0) { return LHRewards::SettleKill(S,Facts(Index),Spec(),Profile,{}); }
    bool Reload()
    { TArray<uint8> B; FLHSaveError E; FLHSaveSnapshot Loaded; if (!LHSave::Encode(S,B,E) || !LHSave::Decode(B,S.Header.CharacterId,{S.Header.Ruleset,S.Header.ContentRevision},Loaded,E)) return false; S=MoveTemp(Loaded); return true; }
};
}
#define LH_PRESENT_TEST(C,N) IMPLEMENT_SIMPLE_AUTOMATION_TEST(C,"Lighthaven.UI.Presentation." N,EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter) bool C::RunTest(const FString&)
LH_PRESENT_TEST(FLHPresentationSettingsTest,"SettingsPersistAndApply")
{
    const FString File=FPaths::ProjectSavedDir()/TEXT("PresentationTest.ini");
    FLHPresentationSettings A; A.Master=.5; A.Effects=.4; A.UI=.2; A.TextScale=1.4; A.HighContrast=true; TestTrue(TEXT("settings file written"),A.Save(File));
    FLHPresentationSettings B; B.Load(File);
    TestEqual(TEXT("effects gain"),B.Gain(false),.2f); TestEqual(TEXT("UI gain"),B.Gain(true),.1f);
    TestEqual(TEXT("text persisted"),B.TextScale,1.4f); TestTrue(TEXT("contrast persisted"),B.HighContrast);
    auto Old=FLHPresentationSettings::Get(); FLHPresentationSettings::Get()=B;
    float Heard=0; FName Cue;
    FLHPresentationAudio::Sink=[&](FName Id,float Gain){Cue=Id; Heard=Gain;};
    FLHPresentationAudio::Cue("UI.Click",true); TestEqual(TEXT("applied gain"),Heard,.1f); TestEqual(TEXT("semantic seam"),Cue,FName("UI.Click"));
    FLHPresentationSettings::Get().Master=0; Heard=0; FLHPresentationAudio::Cue("Combat.Impact",false); TestEqual(TEXT("mute"),Heard,0.f);
    FLHPresentationAudio::Sink=nullptr; FLHPresentationSettings::Get()=Old;
    IFileManager::Get().Delete(*File); return true;
}
LH_PRESENT_TEST(FLHPresentationGlyphTest,"GlyphSwitching")
{
    FLHInputGlyphs::Observe(EKeys::Gamepad_FaceButton_Bottom); TestTrue(TEXT("pad glyph"),FLHInputGlyphs::MenuHints().Contains(TEXT("[South]")));
    FLHInputGlyphs::Observe(EKeys::E); TestFalse(TEXT("keyboard device"),FLHInputGlyphs::Gamepad()); TestTrue(TEXT("keyboard interact"),FLHInputGlyphs::GameplayHints().Contains(TEXT("[E]"))); return true;
}
LH_PRESENT_TEST(FLHPresentationCueTest,"HitAndResourceChange")
{
    using namespace LHPresentationTestsPrivate;
    FLHPresentationCues C; FLHUIHud H; H.Health=N(100); H.MaxHealth=N(100); H.Mana=N(10); H.MaxMana=N(10);
    C.Observe(H,I(1),0); TestEqual(TEXT("hydration silent"),C.ResourceEvents,0);
    H.Health=N(50); C.Observe(H,I(1),.1); TestEqual(TEXT("resource event"),C.ResourceEvents,1); TestTrue(TEXT("bar moves gradually"),C.Health>.5f && C.Health<1.f);
    C.Hit(true,12); TestEqual(TEXT("hit fires"),C.HitEvents,1); TestTrue(TEXT("damage explicit"),C.Notice.Contains(TEXT("12 damage")));
    C.Observe(H,I(2),.1); TestTrue(TEXT("level notice"),C.Notice.Contains(TEXT("Level up")));
    H.bDead=true; C.Observe(H,I(2),.1); TestTrue(TEXT("death notice"),C.Notice.Contains(TEXT("Defeated"))); return true;
}
LH_PRESENT_TEST(FLHPresentationSettlementTest,"CombatAwardGuard")
{
    using namespace LHPresentationTestsPrivate;
    FFixture F; if(!TestTrue(TEXT("fixture"),F.Init())) return false;
    auto On=F.S, Off=F.S;
    const auto P=LH::Rules::MakeStage1PrototypeCombat(); LH::Rules::FCombatInput Input; Input.Accuracy=20; Input.Avoidance=10; Input.WeaponMinimum=25; Input.WeaponMaximum=25;
    FLHPresentationCues C; const auto HitOn=LH::Rules::ResolveCombat(P,Input); C.Hit(HitOn.Value.bHit,HitOn.Value.Damage);
    auto A=LHRewards::SettleKill(On,F.Facts(),F.Spec(),F.Profile,{});
    C.Enabled=false; const auto HitOff=LH::Rules::ResolveCombat(P,Input); C.Hit(HitOff.Value.bHit,HitOff.Value.Damage);
    TestTrue(TEXT("combat accepted"),HitOn.Diagnostic.IsAccepted() && HitOff.Diagnostic.IsAccepted());
    TestEqual(TEXT("same damage"),HitOn.Value.Damage,HitOff.Value.Damage); TestEqual(TEXT("same hit"),HitOn.Value.bHit,HitOff.Value.bHit);
    auto B=LHRewards::SettleKill(Off,F.Facts(),F.Spec(),F.Profile,{});
    TestTrue(TEXT("both settled"),A==ELHCommandReason::None && B==A);
    TArray<uint8> BytesOn,BytesOff; FLHSaveError Error;
    TestTrue(TEXT("on encodes"),LHSave::Encode(On,BytesOn,Error)); TestTrue(TEXT("off encodes"),LHSave::Encode(Off,BytesOff,Error));
    TestTrue(TEXT("identical combat/XP/loot/RNG/award settlement"),BytesOn==BytesOff); return true;
}
#endif
#if WITH_DEV_AUTOMATION_TESTS
LH_PRESENT_TEST(FLHPresentationContrastTest,"ContrastAndMinimumText")
{
    const FLHUIStyle S;
    auto L=[](FLinearColor C){return .2126*C.R+.7152*C.G+.0722*C.B;};
    TestTrue(TEXT("primary on HUD panel >=4.5"),(L(S.TextPrimary)+.05)/(L(S.Panel)+.05)>=4.5);
    TestTrue(TEXT("secondary on HUD panel >=4.5"),(L(S.TextSecondary)+.05)/(L(S.Panel)+.05)>=4.5);
    TestTrue(TEXT("metadata >=16 at 720p"),S.Metadata*FLHUIStyle::Scale(FVector2D(1280,720))>=16);
    TestTrue(TEXT("body >=20 at 720p"),S.Body*FLHUIStyle::Scale(FVector2D(1280,720))>=20);
    return true;
}
#endif
