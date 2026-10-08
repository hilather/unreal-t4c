#include "Misc/AutomationTest.h"
#include "World/LHTravelCoordinator.h"
#include "World/LHTravelSaveAdapter.h"
#include "World/LHAreaStateSubsystem.h"
#include "World/LHWorldValidation.h"
#include "World/LHWorldMarkers.h"
#include "Persistence/LHSaveCodec.h"
#include "UObject/StrongObjectPtr.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#if WITH_DEV_AUTOMATION_TESTS
namespace LHWorldTestsPrivate
{
struct FHost : ILHTravelHost
{
    FLHSaveSnapshot Captured, Saved, Restored, Installed;
    int32 SaveCount=0, LoadCount=0, RestoreCount=0;
    bool bFrozen=false, bInstall=true, bSettle=true;
    FHost()
    {
        Captured.World.RunId=FGuid::NewGuid(); Captured.Session.RequestEpoch=FGuid::NewGuid();
        Captured.Header.CharacterId.Value=FGuid::NewGuid(); Captured.Header.TransactionSequence=4;
        Captured.Character.ActiveEntrance=LHWorld::Registry()[0].SafeFallback;
        Captured.Character.Gold.Value=13; // synthetic retention sentinel, not gameplay tuning
        Captured.Character.CurrentHealth.Resolution=ELHValueResolution::Resolved; Captured.Character.CurrentHealth.Value=1;
        Captured.Character.ExperienceBalance.Value=29;
    }
    virtual void FreezeInteraction(bool bFreeze) override { bFrozen=bFreeze; }
    virtual bool SettleAndCapture(FLHSaveSnapshot& Out,FString& Error) override { Out=Captured; if (!bSettle) Error=TEXT("Active action cannot settle"); return bSettle; }
    virtual void SaveCheckpoint(const FLHSaveSnapshot& S,uint64 Token) override { Saved=S; ++SaveCount; }
    virtual void LoadDestination(const FLHAreaDefinition& Area,uint64 Token) override { ++LoadCount; }
    virtual bool ValidateAndInstallArrival(const FLHSaveSnapshot& S,const FLHEntranceDefinition& E,FString& Error) override { Installed=S; if (!bInstall) Error=TEXT("Synthetic unsafe arrival/corrupt area"); return bInstall; }
    virtual void RestoreSource(const FLHSaveSnapshot& S,const FLHEntranceDefinition& E,uint64 Token) override { Restored=S; ++RestoreCount; }
    FLHRequestTravelRequest Request() const
    {
        FLHRequestTravelRequest R; R.Request.Epoch=Captured.Session.RequestEpoch; R.Request.Value=FGuid::NewGuid();
        const auto& P=LHWorld::Registry()[0].Portals[0]; R.Portal=P.Portal; R.Portal.RunId=Captured.World.RunId; R.Destination=P.Destination; return R;
    }
};
void Placements(TArray<FLHAreaId>& Maps,TArray<FLHPlacedIdentity>& Ids,TArray<FLHPlacedEntrance>& Entrances)
{
    for (const auto& A:LHWorld::Registry())
    {
        Maps.Add(A.Id);
        for (const auto& S:A.Spawns) { FLHPlacedIdentity I; I.Area=A.Id; I.Id=S.SpawnId; I.Definition=S.Enemy; Ids.Add(I); }
        for (const auto& P:A.Portals) { FLHPlacedIdentity I; I.Area=A.Id; I.Id=P.Portal.InstanceId; I.Kind=ELHPlacedIdKind::Portal; I.Source=P.Source; I.Destination=P.Destination; I.bReturn=P.Source.LocalId==TEXT("Entry"); Ids.Add(I); }
        for (const auto& E:A.Entrances) { FLHPlacedEntrance I; I.Id=E.Id; I.Transform=E.SafeTransform; I.bSafetyReviewed=true; Entrances.Add(I); }
    }
}
FLHEncounterRecord Encounter(const FLHAreaDefinition& A)
{
    FLHEncounterRecord E; E.Life.Area=A.Id; E.Life.SpawnSlot=A.Spawns[0].SpawnId; E.Definition=A.Spawns[0].Enemy; E.State=ELHEncounterLifeState::Alive;
    E.CurrentHealth.Resolution=ELHValueResolution::Resolved; E.CurrentHealth.Value=1;
    E.RespawnRemainingSeconds.Resolution=ELHValueResolution::Resolved; return E;
}
}
using namespace LHWorldTestsPrivate;
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWorldRegistryTest,"Lighthaven.World.RegistryIntegrity",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHWorldRegistryTest::RunTest(const FString& Parameters)
{
    TArray<FString> Errors; TestTrue(TEXT("Registry validates"),LHWorld::ValidateRegistry(Errors));
    int32 Spawns=0,Portals=0,Entrances=0;
    for (const auto& A:LHWorld::Registry()) { Spawns+=A.Spawns.Num(); Portals+=A.Portals.Num(); Entrances+=A.Entrances.Num(); TestNotNull(TEXT("Fallback resolves"),LHWorld::FindEntrance(A.SafeFallback)); }
    TestEqual(TEXT("77 explicit spawn IDs"),Spawns,77); TestEqual(TEXT("Eight directed portals"),Portals,8); TestEqual(TEXT("Nine entrances"),Entrances,9);
    FLHAreaId Wrong=LHWorld::Registry()[0].Id; Wrong.Content.Value=TEXT("Area.lighthaventempledistrict"); TestNull(TEXT("Wrong casing fails"),LHWorld::FindArea(Wrong));
    auto Entrance=LHWorld::Registry()[0].SafeFallback;
    TestNotNull(TEXT("Canonical entrance resolves"),LHWorld::FindEntrance(Entrance));
    Entrance.LocalId=TEXT("temple.SafeSpawn");
    TestNull(TEXT("Wrong entrance casing fails"),LHWorld::FindEntrance(Entrance));
    auto Portal=LHWorld::Registry()[0].Portals[0].Portal;
    TestNotNull(TEXT("Canonical portal resolves"),LHWorld::FindPortal(Portal));
    Portal.Area=Wrong;
    TestNull(TEXT("Wrong portal area casing fails"),LHWorld::FindPortal(Portal));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWorldPairTest,"Lighthaven.World.PortalPairing",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHWorldPairTest::RunTest(const FString& Parameters)
{
    TArray<FLHAreaId> Maps; TArray<FLHPlacedIdentity> Ids; TArray<FLHPlacedEntrance> Entrances; TArray<FString> Errors;
    Placements(Maps,Ids,Entrances); TestTrue(TEXT("Complete placement graph"),LHWorld::ValidatePlacements(Maps,Ids,Entrances,Errors));
    const int32 Index=Ids.IndexOfByPredicate([](const auto& I) { return I.Kind==ELHPlacedIdKind::Portal; });
    Ids[Index].Destination=LHWorld::Registry()[4].SafeFallback;
    TestFalse(TEXT("Non-reciprocal destination rejected"),LHWorld::ValidatePlacements(Maps,Ids,Entrances,Errors)); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWorldDuplicateTest,"Lighthaven.World.DuplicateIdsAndSafeTransforms",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHWorldDuplicateTest::RunTest(const FString& Parameters)
{
    TArray<FLHAreaId> Maps; TArray<FLHPlacedIdentity> Ids; TArray<FLHPlacedEntrance> Entrances; TArray<FString> Errors;
    Placements(Maps,Ids,Entrances); const auto Original=Ids[0]; auto Duplicate=Original; Duplicate.Kind=ELHPlacedIdKind::Interactable; Duplicate.Definition.Value=TEXT("Item.Synthetic"); Ids.Add(Duplicate);
    TestFalse(TEXT("Cross-kind ID duplication rejected"),LHWorld::ValidatePlacements(Maps,Ids,Entrances,Errors)); Ids.Pop();
    const auto Portal=Ids[0]; Ids.Add(Portal); TestFalse(TEXT("Duplicate portal ID rejected"),LHWorld::ValidatePlacements(Maps,Ids,Entrances,Errors)); Ids.Pop();
    const auto Spawn=Ids[Ids.IndexOfByPredicate([](const auto& I) { return I.Kind==ELHPlacedIdKind::Spawn; })]; Ids.Add(Spawn);
    TestFalse(TEXT("Duplicate SpawnId rejected"),LHWorld::ValidatePlacements(Maps,Ids,Entrances,Errors)); Ids.Pop();
    auto Copy=Entrances[0]; Entrances.Add(Copy); TestFalse(TEXT("Duplicate entrance rejected"),LHWorld::ValidatePlacements(Maps,Ids,Entrances,Errors)); Entrances.Pop();
    Entrances[0].bSafetyReviewed=false; TestFalse(TEXT("Unreviewed safety rejected"),LHWorld::ValidatePlacements(Maps,Ids,Entrances,Errors));
    Entrances[0].bSafetyReviewed=true; Entrances[0].Transform.SetScale3D(FVector::ZeroVector); TestFalse(TEXT("Invalid transform rejected"),LHWorld::ValidatePlacements(Maps,Ids,Entrances,Errors)); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWorldTravelTest,"Lighthaven.World.TravelSuccessAndCheckpointResume",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHWorldTravelTest::RunTest(const FString& Parameters)
{
    FHost Host; FLHTravelCoordinator Travel(Host); FString Error; auto Request=Host.Request();
    TestTrue(TEXT("Begin"),Travel.Begin(Request,Error)); const uint64 Token=Travel.GetToken();
    TestTrue(TEXT("Frozen before source save"),Host.bFrozen); TestEqual(TEXT("No load before durability"),Host.LoadCount,0);
    TestFalse(TEXT("Second travel rejected"),Travel.Begin(Request,Error));
    Travel.OnSaveCompleted(Token+1,true,5,TEXT("")); TestEqual(TEXT("Stale callback ignored"),Host.LoadCount,0);
    Travel.OnSaveCompleted(Travel.GetToken(),true,7,TEXT("")); TestEqual(TEXT("Load after source durability"),Host.LoadCount,1);
    TestTrue(TEXT("Crash checkpoint is source"),LHWorld::SameEntrance(Host.Saved.Character.ActiveEntrance,LHWorld::Registry()[0].Portals[0].Source));
    Travel.OnDestinationLoaded(Travel.GetToken(),true,Request.Destination,TEXT("")); TestEqual(TEXT("Two saves only"),Host.SaveCount,2);
    Travel.OnSaveCompleted(Token,true,7,TEXT("Delayed source completion"));
    TestEqual(TEXT("Old source completion cannot change arrival phase"),Travel.GetPhase(),ELHTravelPhase::SavingArrival);
    TestTrue(TEXT("Frozen until arrival commit"),Host.bFrozen); TestEqual(TEXT("Arrival uses durable high water"),Host.Saved.Header.TransactionSequence,int64(8));
    TestEqual(TEXT("No gold reward"),Host.Saved.Character.Gold.Value,Host.Captured.Character.Gold.Value); TestEqual(TEXT("No XP reward"),Host.Saved.Character.ExperienceBalance.Value,Host.Captured.Character.ExperienceBalance.Value);
    Travel.OnSaveCompleted(Travel.GetToken(),true,8,TEXT("")); TestFalse(TEXT("Arrival unfreezes"),Host.bFrozen);
    TestTrue(TEXT("Continue checkpoint points to destination"),LHWorld::SameEntrance(Host.Saved.Character.ActiveEntrance,Request.Destination));
    Travel.OnDestinationLoaded(Travel.GetToken(),true,Request.Destination,TEXT("")); TestEqual(TEXT("Repeated completion no extra save"),Host.SaveCount,2); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWorldFailureTest,"Lighthaven.World.MissingDestinationAndInvalidEntrance",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHWorldFailureTest::RunTest(const FString& Parameters)
{
    FHost Host; FLHTravelCoordinator Travel(Host); FString Error; auto Request=Host.Request(); auto Bad=Request; Bad.Destination.Area.Content.Value=TEXT("Area.Missing");
    TestFalse(TEXT("Missing destination fails before save"),Travel.Begin(Bad,Error)); TestEqual(TEXT("No save"),Host.SaveCount,0);
    Host.Captured.Character.CurrentHealth.Value=0; TestFalse(TEXT("Dead character cannot travel"),Travel.Begin(Request,Error)); Host.Captured.Character.CurrentHealth.Value=1;
    Host.bSettle=false; TestFalse(TEXT("Unsettled action cannot travel"),Travel.Begin(Request,Error)); Host.bSettle=true;
    auto WrongRun=Request; WrongRun.Portal.RunId=FGuid::NewGuid(); TestFalse(TEXT("Cross-run portal rejected"),Travel.Begin(WrongRun,Error));
    auto WrongEpoch=Request; WrongEpoch.Request.Epoch=FGuid::NewGuid(); TestFalse(TEXT("Stale epoch rejected"),Travel.Begin(WrongEpoch,Error));
    TestTrue(TEXT("Begin valid"),Travel.Begin(Request,Error)); auto Token=Travel.GetToken(); Travel.OnSaveCompleted(Travel.GetToken(),true,5,TEXT(""));
    Travel.OnDestinationLoaded(Travel.GetToken(),false,{},TEXT("Missing map")); TestEqual(TEXT("Restore requested"),Host.RestoreCount,1); TestTrue(TEXT("Recovery frozen"),Host.bFrozen);
    Travel.OnSourceRestored(Travel.GetToken(),false,TEXT("Source unavailable")); TestTrue(TEXT("Recovery failure stays frozen"),Host.bFrozen); TestTrue(TEXT("Retry source restoration"),Travel.Retry(Error));
    Travel.OnSourceRestored(Travel.GetToken(),true,TEXT("")); TestFalse(TEXT("Source restored"),Host.bFrozen);
    TestTrue(TEXT("Begin again"),Travel.Begin(Request,Error)); Token=Travel.GetToken(); Travel.OnSaveCompleted(Travel.GetToken(),true,5,TEXT(""));
    Travel.OnDestinationLoaded(Travel.GetToken(),true,LHWorld::Registry()[0].SafeFallback,TEXT("")); TestEqual(TEXT("Invalid entrance restores source"),Host.RestoreCount,3); TestEqual(TEXT("No arrival save"),Host.SaveCount,2); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWorldSaveFailureTest,"Lighthaven.World.SaveFailureRetryCancelAndCorruptArrival",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHWorldSaveFailureTest::RunTest(const FString& Parameters)
{
    FHost Host; FLHTravelCoordinator Travel(Host); FString Error; auto Request=Host.Request();
    Travel.Begin(Request,Error); auto Token=Travel.GetToken(); Travel.OnSaveCompleted(Travel.GetToken(),false,0,TEXT("Disk error"));
    TestEqual(TEXT("Failed source never loads"),Host.LoadCount,0); TestTrue(TEXT("Cancel source failure"),Travel.Cancel(Error)); TestFalse(TEXT("Cancelled unfreezes"),Host.bFrozen);
    Travel.Begin(Request,Error); Token=Travel.GetToken(); Travel.OnSaveCompleted(Travel.GetToken(),false,0,TEXT("Disk error")); TestTrue(TEXT("Retry source"),Travel.Retry(Error)); Travel.OnSaveCompleted(Travel.GetToken(),true,5,TEXT(""));
    Host.bInstall=false; Travel.OnDestinationLoaded(Travel.GetToken(),true,Request.Destination,TEXT("")); TestEqual(TEXT("Corrupt area restored"),Host.RestoreCount,1); Travel.OnSourceRestored(Travel.GetToken(),true,TEXT(""));
    Host.bInstall=true; Travel.Begin(Request,Error); Token=Travel.GetToken(); Travel.OnSaveCompleted(Travel.GetToken(),true,5,TEXT("")); Travel.OnDestinationLoaded(Travel.GetToken(),true,Request.Destination,TEXT(""));
    Travel.OnSaveCompleted(Travel.GetToken(),false,0,TEXT("Arrival write failed")); TestTrue(TEXT("Arrival remains frozen"),Host.bFrozen); TestFalse(TEXT("Cannot unsafe cancel arrival"),Travel.Cancel(Error));
    TestTrue(TEXT("Retry arrival"),Travel.Retry(Error)); Travel.OnSaveCompleted(Travel.GetToken(),true,6,TEXT("")); TestFalse(TEXT("Successful arrival retry unfreezes"),Host.bFrozen); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWorldStateTest,"Lighthaven.World.AreaHydrationAndHighWater",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHWorldStateTest::RunTest(const FString& Parameters)
{
    TStrongObjectPtr<UGameInstance> GameInstance(NewObject<UGameInstance>(GEngine));
    TStrongObjectPtr<ULHAreaStateSubsystem> State(NewObject<ULHAreaStateSubsystem>(GameInstance.Get()));
    FSubsystemCollection<UGameInstanceSubsystem> Collection;
    State->Initialize(Collection); FString Error;
    FLHWorldRecord World; World.RunId=FGuid::NewGuid(); FLHAreaRecord Area; Area.Area=LHWorld::Registry()[1].Id; Area.Encounters.Add(Encounter(LHWorld::Registry()[1])); World.Areas.Add(Area);
    TestTrue(TEXT("Hydrate"),State->Hydrate(World,Error));
    auto WrongArea=Area.Area; WrongArea.Content.Value=TEXT("Area.templeB1");
    TestNull(TEXT("Wrong state lookup casing fails"),State->Find(WrongArea));
    auto WrongEnemy=World; WrongEnemy.Areas[0].Encounters[0].Definition.Value=TEXT("Enemy.brownRat");
    TestFalse(TEXT("Wrong spawn enemy casing rejected"),State->Hydrate(WrongEnemy,Error));
    auto Corrupt=World; Corrupt.Areas[0].Encounters[0].Life.SpawnSlot=FGuid::NewGuid();
    TestFalse(TEXT("Unknown spawn rejected"),State->Hydrate(Corrupt,Error)); TestNotNull(TEXT("Existing state retained"),State->Find(Area.Area));
    Corrupt=World; Corrupt.Areas[0].Encounters[0].Life.LifeGeneration=-1; TestFalse(TEXT("Negative generation rejected"),State->Hydrate(Corrupt,Error));
    Area.Encounters[0].Life.LifeGeneration=3; TestTrue(TEXT("Trusted committed generation update"),State->StoreArea(Area,Error));
    auto Old=World.Areas[0]; TestFalse(TEXT("High-water rollback rejected"),State->StoreArea(Old,Error));
    Old=Area; Old.Encounters.Reset(); TestFalse(TEXT("Cannot forget latest life"),State->StoreArea(Old,Error));
    TestTrue(TEXT("Persist world"),State->PersistInto(World,Error)); TestEqual(TEXT("Generation persisted"),World.Areas[0].Encounters[0].Life.LifeGeneration,int64(3));
    auto WrongRun=World; WrongRun.RunId=FGuid::NewGuid(); TestFalse(TEXT("Different run cannot persist"),State->PersistInto(WrongRun,Error));
    State->Deinitialize();
    return true;
}
namespace LHWorldSaveTestsPrivate
{
FLHInteger Integer(int64 Value) { FLHInteger I; I.Resolution=ELHValueResolution::Resolved; I.Value=Value; I.Provenance.Status=ELHProvenanceStatus::Prototype; I.Provenance.Notes=TEXT("Synthetic world persistence test sentinel"); return I; }
FLHNumber Number(double Value) { FLHNumber N; N.Resolution=ELHValueResolution::Resolved; N.Value=Value; N.Provenance.Status=ELHProvenanceStatus::Prototype; return N; }
FLHSaveSnapshot Fixture()
{
    FHost Host; auto S=Host.Captured; auto& C=S.Character;
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
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWorldStorageTest,"Lighthaven.World.StoreDurabilityAndSourceResume",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHWorldStorageTest::RunTest(const FString& Parameters)
{
    using namespace LHWorldSaveTestsPrivate;
    auto Storage=MakeShared<FMemoryStorage>(); auto Store=MakeShared<FLHSaveStore>(Storage); auto Snapshot=Fixture();
    FLHSaveCompatibility Compatibility; Compatibility.Ruleset=Snapshot.Header.Ruleset; Compatibility.ContentRevision=Snapshot.Header.ContentRevision;
    Compatibility.ValidateReferences=[](const FLHSaveSnapshot& S,FLHSaveError& E) { FString Error; if (LHWorld::ValidateWorld(S.World,Error) && LHWorld::FindEntrance(S.Character.ActiveEntrance)) return true; E={ELHSaveReason::InvalidSnapshot,Error}; return false; };
    bool Frozen=false; int32 Loads=0, DurableCount=0; int64 DurableSequence=0; FLHTravelSaveAdapter::FHooks Hooks;
    Hooks.Freeze=[&](bool Value) { Frozen=Value; }; Hooks.Capture=[&](FLHSaveSnapshot& S,FString& E) { S=Snapshot; return true; };
    Hooks.Durable=[&](const FLHSaveSnapshot& S) { ++DurableCount; DurableSequence=S.Header.TransactionSequence; };
    Hooks.Load=[&](const FLHAreaDefinition& A,uint64 Token) { ++Loads; }; Hooks.Install=[](const FLHSaveSnapshot& S,const FLHEntranceDefinition& E,FString& Error) { return true; };
    Hooks.Restore=[](const FLHSaveSnapshot& S,const FLHEntranceDefinition& E,uint64 Token) {};
    FLHTravelSaveAdapter Adapter(Store,Compatibility,MoveTemp(Hooks)); FString Error;
    FHost RequestHost; RequestHost.Captured=Snapshot; auto Request=RequestHost.Request();
    FLHSaveError FixtureError; TestTrue(TEXT("Actual codec accepts fixture"),LHSave::Validate(Snapshot,FixtureError));
    TestTrue(TEXT("Begin storage-backed travel"),Adapter.Travel().Begin(Request,Error));
    TestTrue(TEXT("Frozen during destination load"),Frozen); TestEqual(TEXT("Actual source write"),Storage->Writes,1); TestEqual(TEXT("Load follows successful readback"),Loads,1);
    auto IndependentStore=MakeShared<FLHSaveStore>(Storage); FLHSaveSnapshot Resumed; FLHSaveError SaveError;
    TestTrue(TEXT("Independent store loads source while arrival incomplete"),IndependentStore->Load(Snapshot.Header.CharacterId,Compatibility,Resumed,SaveError));
    TestTrue(TEXT("Source endpoint persisted"),LHWorld::SameEntrance(Resumed.Character.ActiveEntrance,LHWorld::Registry()[0].Portals[0].Source));
    Storage->bFail=true; Adapter.Travel().OnDestinationLoaded(Adapter.Travel().GetToken(),true,Request.Destination,TEXT(""));
    TestTrue(TEXT("Actual write failure remains frozen"),Frozen); TestEqual(TEXT("Arrival failure phase"),Adapter.Travel().GetPhase(),ELHTravelPhase::ArrivalSaveFailed);
    TestTrue(TEXT("Failed arrival preserves durable source"),IndependentStore->Load(Snapshot.Header.CharacterId,Compatibility,Resumed,SaveError));
    TestTrue(TEXT("Still source"),LHWorld::SameEntrance(Resumed.Character.ActiveEntrance,LHWorld::Registry()[0].Portals[0].Source));
    Storage->bFail=false; TestTrue(TEXT("Retry through real store"),Adapter.Travel().Retry(Error)); TestFalse(TEXT("Successful readback unfreezes"),Frozen);
    TestTrue(TEXT("Independent load sees arrival"),IndependentStore->Load(Snapshot.Header.CharacterId,Compatibility,Resumed,SaveError)); TestTrue(TEXT("Arrival endpoint persisted"),LHWorld::SameEntrance(Resumed.Character.ActiveEntrance,Request.Destination));
    TestEqual(TEXT("Both checkpoint commits published"),DurableCount,2); TestEqual(TEXT("Session receives actual durability sequence"),DurableSequence,Resumed.Header.TransactionSequence);
    TestEqual(TEXT("Gold retained by codec"),Resumed.Character.Gold.Value,Snapshot.Character.Gold.Value); TestEqual(TEXT("XP retained by codec"),Resumed.Character.ExperienceBalance.Value,Snapshot.Character.ExperienceBalance.Value);
    TestTrue(TEXT("Safe respawn retained"),LHWorld::SameEntrance(Resumed.Session.SafeRespawn.Entrance,Snapshot.Session.SafeRespawn.Entrance)); return true;
}

#endif
