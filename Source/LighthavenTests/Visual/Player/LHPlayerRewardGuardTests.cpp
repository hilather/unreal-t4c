#include "Misc/AutomationTest.h"
#include "Rewards/LHEncounterLifecycle.h"
#include "Persistence/LHSaveCodec.h"
#include "Framework/LHWave2Profile.h"
#include "Visual/Player/LHPlayerVisual.h"
#include "Framework/LHCharacter.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#if WITH_DEV_AUTOMATION_TESTS
namespace LHPlayerRewardTestsPrivate
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
TArray<uint8> Bytes(const FLHSaveSnapshot& S) { TArray<uint8> B; FLHSaveError E; LHSave::Encode(S,B,E); return B; }
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHPlayerRewardGuard,"Lighthaven.Visual.Player.RewardSettlementGuard",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHPlayerRewardGuard::RunTest(const FString&)
{
    using namespace LHPlayerRewardTestsPrivate;
    FFixture F; if(!TestTrue(TEXT("Valid reward fixture"),F.Init())) return false;
    const auto Initial=F.S; TArray<uint8> Settled[2];
    const auto Init=UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(true).CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false);
    auto* W=UWorld::CreateWorld(EWorldType::Game,false,MakeUniqueObjectName(GetTransientPackage(),UWorld::StaticClass(),TEXT("PlayerRewardGuard")),GetTransientPackage(),true,ERHIFeatureLevel::Num,&Init);
    GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(W);
    FActorSpawnParameters Spawn; Spawn.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    auto* Player=W->SpawnActor<ALHCharacter>(FVector::ZeroVector,FRotator::ZeroRotator,Spawn);
    for(int32 Enabled=0;Enabled<2;++Enabled)
    {
        F.S=Initial; Player->PlayerVisual->SetPresentationEnabled(Enabled!=0);
        Player->PlayerVisual->Present(&F.S.Character,nullptr,450,.3f);
        TestTrue(TEXT("Native reward accepted"),F.Kill()==ELHCommandReason::None);
        Player->PlayerVisual->Present(&F.S.Character,nullptr,0,.1f);
        Settled[Enabled]=Bytes(F.S);
        TestTrue(TEXT("Canonical bytes present"),!Settled[Enabled].IsEmpty());
    }
    TestTrue(TEXT("Identical canonical XP loot RNG life and award checkpoint"),Settled[0]==Settled[1]);
    Player->Destroy(); W->DestroyWorld(false); GEngine->DestroyWorldContext(W); return true;
}
#endif
