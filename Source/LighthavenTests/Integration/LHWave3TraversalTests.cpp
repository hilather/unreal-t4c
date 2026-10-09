#include "Misc/AutomationTest.h"
#include "World/LHTravelSaveAdapter.h"
#include "World/LHAreaStateSubsystem.h"
#include "Framework/LHArrivalReview.h"
#if WITH_DEV_AUTOMATION_TESTS
namespace LHWave3TraversalTestsPrivate
{
FLHInteger Integer(int64 Value) { FLHInteger I; I.Resolution=ELHValueResolution::Resolved; I.Value=Value; I.Provenance.Status=ELHProvenanceStatus::Prototype; I.Provenance.Notes=TEXT("Synthetic world persistence test sentinel"); return I; }
FLHNumber Number(double Value) { FLHNumber N; N.Resolution=ELHValueResolution::Resolved; N.Value=Value; N.Provenance.Status=ELHProvenanceStatus::Prototype; return N; }
FLHSaveSnapshot Fixture()
{
    FLHSaveSnapshot S; S.World.RunId=FGuid::NewGuid(); S.Session.RequestEpoch=FGuid::NewGuid();
    S.Header.CharacterId.Value=FGuid::NewGuid(); S.Header.TransactionSequence=4;
    S.Character.ActiveEntrance=LHWorld::Registry()[0].SafeFallback; auto& C=S.Character;
    S.Header.BuildId=TEXT("World.Automation"); S.Header.ChecksumAlgorithm=TEXT("SHA256"); S.Header.PayloadCodec=TEXT("LHCanonicalBinary1");
    S.Header.Ruleset.Id.Value=TEXT("Ruleset.WorldFixture"); S.Header.Ruleset.Revision=1; S.Header.Ruleset.HashAlgorithm=TEXT("SHA256"); S.Header.Ruleset.ContentHash=FString::ChrN(64,'a'); S.Header.ContentRevision=FString::ChrN(64,'b');
    C.DisplayName=TEXT("Synthetic world test"); C.BaseAttributes.Strength=C.BaseAttributes.Endurance=C.BaseAttributes.Agility=C.BaseAttributes.Intelligence=C.BaseAttributes.Wisdom=Integer(1);
    C.Creation.AcceptedAttributes=C.BaseAttributes; C.Creation.GenerationRevision=Integer(1); C.Creation.GenerationPolicy.Value=TEXT("CreationPolicy.WorldFixture");
    C.EarnedLevel=Integer(1); C.ExperienceBalance=Integer(29); C.ExperienceDebt=C.UnspentAttributePoints=C.UnspentSkillPoints=Integer(0); C.Gold=Integer(13);
    C.EarnedBaseHealth=C.EarnedBaseMana=C.CurrentHealth=C.CurrentMana=Number(1);
    S.Session.ManaRegenFractionalSeconds=Number(0); S.Session.EffectPolicy=ELHEffectSavePolicy::CompletedActionBoundaryOnly;
    const auto* E=LHWorld::FindEntrance(C.ActiveEntrance); S.Session.SafeRespawn.Entrance=E->Id; S.Session.SafeRespawn.SafeTransform=E->SafeTransform; S.Session.SafeRespawn.TransformResolution=ELHValueResolution::Resolved;
    for (const auto& A:LHWorld::Registry()) { FLHAreaRecord R; R.Area=A.Id; S.World.Areas.Add(R); }
    return S;
}
struct FMemoryStorage : ILHSaveStorage
{
    TArray<uint8> Slots[2]; FGuid Character; bool bFail=false; int32 Writes=0;
    virtual TArray<FGuid> Enumerate() override { return {Character}; }
    virtual bool Read(const FGuid& Id,int32 Slot,TArray<uint8>& Bytes,FLHSaveError& Error) override
    {
        if (Slots[Slot].IsEmpty()) { Error={ELHSaveReason::NotFound,TEXT("Synthetic empty slot")}; return false; }
        Bytes=Slots[Slot]; return true;
    }
    virtual void Write(const FGuid& Id,int32 Slot,TArray<uint8> Bytes,TFunction<void(bool)> Complete) override
    { ++Writes; Character=Id; if (!bFail) Slots[Slot]=MoveTemp(Bytes); Complete(!bFail); }
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWave3Traversal,"Lighthaven.Integration.Wave3.TraversalAndReload",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHWave3Traversal::RunTest(const FString&)
{
    using namespace LHWave3TraversalTestsPrivate;
    auto Disk=MakeShared<FMemoryStorage>(); auto Store=MakeShared<FLHSaveStore>(Disk); auto Current=Fixture();
    FLHSaveCompatibility Compatibility; Compatibility.Ruleset=Current.Header.Ruleset; Compatibility.ContentRevision=Current.Header.ContentRevision;
    Compatibility.ValidateReferences=[](const FLHSaveSnapshot& S,FLHSaveError& E) {
        FString Error; if (LHWorld::FindEntrance(S.Character.ActiveEntrance) && LHWorld::ValidateWorld(S.World,Error)) return true;
        E={ELHSaveReason::InvalidSnapshot,Error}; return false;
    };
    FLHTravelSaveAdapter* Adapter=nullptr; bool Frozen=false,InvalidSpawn=false;
    FLHTravelSaveAdapter::FHooks Hooks;
    Hooks.Freeze=[&](bool V) { Frozen=V; };
    Hooks.Capture=[&](FLHSaveSnapshot& S,FString&) { S=Current; return true; };
    Hooks.Durable=[&](const FLHSaveSnapshot& S) { Current=S; };
    Hooks.Load=[](const FLHAreaDefinition&,uint64) {};
    Hooks.Install=[&](const FLHSaveSnapshot&,const FLHEntranceDefinition&,FString& Error) { if (InvalidSpawn) Error=TEXT("Injected blocked capsule"); return !InvalidSpawn; };
    Hooks.Restore=[&](const FLHSaveSnapshot& S,const FLHEntranceDefinition&,uint64 Token) { Current=S; Adapter->Travel().OnSourceRestored(Token,true,TEXT("")); };
    FLHTravelSaveAdapter Travel(Store,Compatibility,MoveTemp(Hooks)); Adapter=&Travel;
    FLHSaveError SaveError; FString Error;
    TestTrue(TEXT("Save initial hub"),Store->RequestSave(Current,Compatibility,true,SaveError));
    const auto Initial=Current;
    auto Reload=[&]() {
        FLHSaveStore Independent(Disk); FLHSaveSnapshot Loaded;
        TestTrue(TEXT("Fresh store reload"),Independent.Load(Current.Header.CharacterId,Compatibility,Loaded,SaveError));
        TestTrue(TEXT("Reload exact entrance"),LHWorld::SameEntrance(Loaded.Character.ActiveEntrance,Current.Character.ActiveEntrance));
        TestNotNull(TEXT("Reload entrance registered"),LHWorld::FindEntrance(Loaded.Character.ActiveEntrance));
        TestEqual(TEXT("Gold unchanged"),Loaded.Character.Gold.Value,Initial.Character.Gold.Value);
        TestEqual(TEXT("XP unchanged"),Loaded.Character.ExperienceBalance.Value,Initial.Character.ExperienceBalance.Value);
    };
    Reload();
    // Four descent and four inverse edges, with an independent disk reload per floor.
    const int32 Route[]={1,2,3,4,3,2,1,0};
    for (int32 Destination:Route)
    {
        const auto* Area=LHWorld::FindArea(Current.Character.ActiveEntrance.Area);
        const auto* Edge=Area->Portals.FindByPredicate([&](const auto& P) { return LHWorld::SameArea(P.Destination.Area,LHWorld::Registry()[Destination].Id); });
        if (!TestNotNull(TEXT("Registered route edge"),Edge)) return false;
        FLHRequestTravelRequest Q; Q.Request.Epoch=Current.Session.RequestEpoch; Q.Request.Value=FGuid::NewGuid();
        Q.Portal=Edge->Portal; Q.Portal.RunId=Current.World.RunId; Q.Destination=Edge->Destination;
        TestTrue(TEXT("Begin checkpoint travel"),Travel.Travel().Begin(Q,Error));
        TestTrue(TEXT("Frozen until arrival"),Frozen);
        TestTrue(TEXT("Source remains reloadable"),LHWorld::SameEntrance(Current.Character.ActiveEntrance,Edge->Source));
        Reload();
        Travel.Travel().OnDestinationLoaded(Travel.Travel().GetToken(),true,Q.Destination,TEXT(""));
        TestFalse(TEXT("Arrival durable and unfrozen"),Frozen); Reload();
    }
    const auto& Edge=LHWorld::Registry()[0].Portals[0];
    FLHRequestTravelRequest Q; Q.Request.Epoch=Current.Session.RequestEpoch; Q.Request.Value=FGuid::NewGuid();
    Q.Portal=Edge.Portal; Q.Portal.RunId=Current.World.RunId; Q.Destination=Edge.Destination;
    auto Bad=Q; Bad.Destination.LocalId=TEXT("Missing"); const int32 Writes=Disk->Writes;
    TestFalse(TEXT("Missing destination rejected"),Travel.Travel().Begin(Bad,Error)); TestEqual(TEXT("No bad destination write"),Disk->Writes,Writes);
    InvalidSpawn=true; TestTrue(TEXT("Start invalid spawn case"),Travel.Travel().Begin(Q,Error));
    Travel.Travel().OnDestinationLoaded(Travel.Travel().GetToken(),true,Q.Destination,TEXT(""));
    TestFalse(TEXT("Restored source unfreezes"),Frozen); Reload();
    auto Corrupt=Current; Corrupt.World.Areas[0].Area.Content.Value=TEXT("Area.Corrupt"); Current=Corrupt;
    TestFalse(TEXT("Corrupt area rejected before travel"),Travel.Travel().Begin(Q,Error));
    TestFalse(TEXT("Corrupt source unfreezes"),Frozen);
    return !HasAnyErrors();
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWave3ReviewHash,"Lighthaven.Integration.Wave3.ReviewTransformHash",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHWave3ReviewHash::RunTest(const FString&)
{
    const auto& E=LHWorld::Registry()[0].Entrances[0]; auto Moved=E.SafeTransform; Moved.AddToTranslation(FVector(1,0,0));
    TestEqual(TEXT("Stable transform hash"),LHArrivalReview::Hash(E.SafeTransform),LHArrivalReview::Hash(E.SafeTransform));
    TestNotEqual(TEXT("Moved transform invalidates approval"),LHArrivalReview::Hash(E.SafeTransform),LHArrivalReview::Hash(Moved));
    return !HasAnyErrors();
}
#endif
