#include "Misc/AutomationTest.h"
#include "Framework/LHWave2Session.h"
#include "Framework/LHWave2Profile.h"
#include "Framework/LHWave2Closure.h"
#include "Framework/LHPlayerState.h"
#include "Framework/LHPlayerController.h"
#include "Framework/LHCharacter.h"
#include "Abilities/LHCombatComponent.h"
#include "Abilities/LHAttributeSet.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#if WITH_DEV_AUTOMATION_TESTS
namespace LHWave2TestsPrivate
{
constexpr auto Flags=EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter;
class FStorage : public ILHSaveStorage
{
public:
    TMap<FGuid,TArray<uint8>> Slots[2];
    int32 Writes=0; bool bFail=false, bFailReadback=false, bReadbackArmed=false;
    TArray<FGuid> Enumerate() override
    { TArray<FGuid> R; Slots[0].GetKeys(R); for (const auto& K:Slots[1]) R.AddUnique(K.Key); return R; }
    bool Read(const FGuid& Id,int32 Slot,TArray<uint8>& Bytes,FLHSaveError& E) override
    { if (bReadbackArmed) { bReadbackArmed=false; E={ELHSaveReason::IoFailure,TEXT("Injected readback failure")}; return false; } if (auto* B=Slots[Slot].Find(Id)) { Bytes=*B; return true; } E={ELHSaveReason::NotFound,TEXT("Missing generation")}; return false; }
    void Write(const FGuid& Id,int32 Slot,TArray<uint8> Bytes,TFunction<void(bool)> Done) override
    { ++Writes; if (!bFail) Slots[Slot].Add(Id,MoveTemp(Bytes)); bReadbackArmed=bFailReadback; Done(!bFail); }
};
struct FRuntime
{
    UWorld* World=nullptr;
    ALHPlayerState* State=nullptr;
    ALHPlayerController* Controller=nullptr;
    TSharedPtr<FLHWave2Session> Session;
    TSharedPtr<FLHUIPresenter> UI;
    explicit FRuntime(TSharedRef<FStorage> Disk)
    {
        const FName Name=MakeUniqueObjectName(nullptr,UWorld::StaticClass(),TEXT("LHWave2World"),EUniqueObjectNameOptions::GloballyUnique);
        const auto Init=UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(true).RequiresHitProxies(false)
            .CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false).SetTransactional(false);
        auto& Context=GEngine->CreateNewWorldContext(EWorldType::Game);
        World=UWorld::CreateWorld(EWorldType::Game,false,Name,GetTransientPackage(),true,ERHIFeatureLevel::Num,&Init);
        Context.SetCurrentWorld(World); World->InitializeActorsForPlay(FURL());
        Controller=World->SpawnActor<ALHPlayerController>(); Controller->SetAsLocalPlayerController();
        State=World->SpawnActor<ALHPlayerState>(); Controller->SetPlayerState(State); Controller->InitInputSystem();
        Session=MakeShared<FLHWave2Session>(MakeShared<FLHSaveStore>(Disk)); Session->Bind(State);
        UI=MakeShared<FLHUIPresenter>(*Session,*Session,*Session);
    }
    ~FRuntime()
    {
        UI.Reset(); Session.Reset(); GEngine->DestroyWorldContext(World); World->DestroyWorld(false);
    }
    FLHCommandResult Create()
    {
        UI->Open(ELHUIScreen::Creation);
        TArray<FLHContentId> Appearance;
        for (const TCHAR* Key:{TEXT("Presentation.Player.Body.A"),TEXT("Presentation.Player.Hair.Cropped"),TEXT("Presentation.Player.Skin.LightWarm"),TEXT("Presentation.Player.Outfit.StarterLinen")})
        { FLHContentId I; I.Value=Key; Appearance.Add(I); }
        UI->EditCreation(TEXT("Same name"),Appearance,LHWave2::PrototypeAnswers()); UI->Roll(false);
        return UI->ConfirmCreation();
    }
    void Flush() { Session->Flush(); }
    bool Restore(FLHCharacterId Id,bool Ack=false)
    { UI->Open(ELHUIScreen::Characters); UI->SelectProfile(Id); return UI->Continue(Ack); }
};
bool Equal(const FLHSaveSnapshot& A,const FLHSaveSnapshot& B)
{
    TArray<uint8> X,Y; FLHSaveError E; return LHSave::Encode(A,X,E) && LHSave::Encode(B,Y,E) && X==Y;
}
FLHAttributeBlock Points()
{
    FLHAttributeBlock P; P.Strength=LHWave2::PrototypeInteger(1); P.Endurance=P.Agility=P.Intelligence=P.Wisdom=LHWave2::PrototypeInteger(0); return P;
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWave2Independence,"Lighthaven.Integration.Wave2.IndependentRebuild",LHWave2TestsPrivate::Flags)
bool FLHWave2Independence::RunTest(const FString&)
{
    using namespace LHWave2TestsPrivate;
    auto Disk=MakeShared<FStorage>(); FLHSaveSnapshot A,B;
    {
        FRuntime Runtime(Disk); TestTrue(TEXT("Create A through live presenter"),Runtime.Create().Disposition==ELHCommandDisposition::Accepted);
        TestEqual(TEXT("No write inside publication"),Disk->Writes,0); Runtime.Flush(); A=Runtime.Session->Snapshot();
        Runtime.UI->ConfirmCreation(); Runtime.Flush(); TestEqual(TEXT("Latched confirm does not write twice"),Disk->Writes,1);
    }
    {
        FRuntime Runtime(Disk); TestTrue(TEXT("Create B"),Runtime.Create().Disposition==ELHCommandDisposition::Accepted); Runtime.Flush(); B=Runtime.Session->Snapshot();
    }
    TestTrue(TEXT("Independent stable identities with equal names"),A.Header.CharacterId.Value!=B.Header.CharacterId.Value && A.World.RunId!=B.World.RunId);
    for (const auto& Expected:{A,B})
    {
        FRuntime Runtime(Disk); TestTrue(TEXT("Select restores"),Runtime.Restore(Expected.Header.CharacterId));
        TestTrue(TEXT("Complete canonical snapshot equal after runtime destruction"),Equal(Expected,Runtime.Session->Snapshot()));
        auto* C=Runtime.State->GetCombatComponent(); TestFalse(TEXT("No live ASC avatar before initialization"),C->IsAlive());
        TestEqual(TEXT("Restored health installed before avatar"),C->GetCombatAttributes()->GetHealth(),float(Expected.Character.CurrentHealth.Value));
        auto* Avatar=Runtime.World->SpawnActor<ALHCharacter>(); Runtime.State->InitializeAvatar(Avatar);
        TestEqual(TEXT("Initialization keeps restored resources"),C->GetCombatAttributes()->GetHealth(),float(Expected.Character.CurrentHealth.Value));
    }
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWave2Equipment,"Lighthaven.Integration.Wave2.EquipmentAllocationRebuild",LHWave2TestsPrivate::Flags)
bool FLHWave2Equipment::RunTest(const FString&)
{
    using namespace LHWave2TestsPrivate;
    auto Disk=MakeShared<FStorage>(); FLHSaveSnapshot Saved,Untouched;
    { FRuntime Other(Disk); Other.Create(); Other.Flush(); Untouched=Other.Session->Snapshot(); }
    {
        FRuntime Runtime(Disk); Runtime.Create(); Runtime.Flush(); Runtime.Session->Bind(Runtime.State); // emulate arrival, clears travel suppression
        auto Before=Runtime.Session->Snapshot();
        auto Review=Runtime.UI->ReviewAllocation(Points()); TestTrue(TEXT("Owner legal review"),Review.bLegal);
        TestTrue(TEXT("Review did not alter receipts or snapshot"),Equal(Before,Runtime.Session->Snapshot()));
        TestTrue(TEXT("Allocate"),Runtime.UI->Allocate(Points()).Disposition==ELHCommandDisposition::Accepted); Runtime.Flush();
        auto Items=Runtime.Session->Snapshot().Character.Inventory;
        const auto* Quiver=Items.FindByPredicate([](const auto& I){return I.Definition.Value==TEXT("Item.TestQuiver");});
        const auto* Bow=Items.FindByPredicate([](const auto& I){return I.Definition.Value==TEXT("Item.TestBow");});
        if (!Quiver || !Bow) { AddError(TEXT("Prototype starter inventory missing")); return false; }
        TestTrue(TEXT("Equip quiver"),Runtime.UI->Equip(Quiver->Id,ELHEquipmentSlot::Quiver,false).Disposition==ELHCommandDisposition::Accepted); Runtime.Flush();
        TestTrue(TEXT("Equip bow"),Runtime.UI->Equip(Bow->Id,ELHEquipmentSlot::MainHand,false).Disposition==ELHCommandDisposition::Accepted); Runtime.Flush();
        // Trusted growth settlement fixture, followed by a real completed allocation command/save.
        TestTrue(TEXT("Canonical growth IDs"),Runtime.State->GetCharacterAuthority()->Authority().GrantExperience(300)==ELHCommandReason::None);
        TestTrue(TEXT("Persist growth at completed command boundary"),Runtime.UI->Allocate(Points()).Disposition==ELHCommandDisposition::Accepted); Runtime.Flush();
        Saved=Runtime.Session->Snapshot(); TestEqual(TEXT("Growth retained"),Saved.Character.GrowthAwards.Num(),2);
    }
    FRuntime Restored(Disk); TestTrue(TEXT("Restore equipped character"),Restored.Restore(Saved.Header.CharacterId));
    TestTrue(TEXT("Inventory equipment attributes growth resources and session equal"),Equal(Saved,Restored.Session->Snapshot()));
    TestEqual(TEXT("Derived equipment rebuilt"),Restored.State->GetCombatComponent()->GetCombatAttributes()->GetAccuracy(),4.f);
    FRuntime Other(Disk); TestTrue(TEXT("Independent unequipped character loads"),Other.Restore(Untouched.Header.CharacterId));
    TestTrue(TEXT("Equipment/allocation did not change other character"),Equal(Untouched,Other.Session->Snapshot()));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWave2Recovery,"Lighthaven.Integration.Wave2.RecoveryAndUnreadable",LHWave2TestsPrivate::Flags)
bool FLHWave2Recovery::RunTest(const FString&)
{
    using namespace LHWave2TestsPrivate;
    auto Disk=MakeShared<FStorage>(); FLHSaveSnapshot Earlier,Newest,Unaffected;
    { FRuntime Other(Disk); Other.Create(); Other.Flush(); Unaffected=Other.Session->Snapshot(); }
    {
        FRuntime Runtime(Disk); Runtime.Create(); Runtime.Flush(); Earlier=Runtime.Session->Snapshot(); Runtime.Session->Bind(Runtime.State);
        Runtime.UI->Allocate(Points()); Runtime.Flush(); Newest=Runtime.Session->Snapshot();
    }
    // First save is slot 0; second completed action writes slot 1.
    Disk->Slots[1][Newest.Header.CharacterId.Value]={0,1,2};
    {
        FRuntime Runtime(Disk);
        TestFalse(TEXT("Recovery cannot continue without acknowledgment"),Runtime.Restore(Newest.Header.CharacterId,false));
        TestTrue(TEXT("Recovery warning visible"),Runtime.UI->Error().Contains(TEXT("Recovery")));
        TestTrue(TEXT("Acknowledgment restores prior generation"),Runtime.Restore(Newest.Header.CharacterId,true));
        TestTrue(TEXT("Prior canonical state"),Equal(Earlier,Runtime.Session->Snapshot()));
    }
    Disk->Slots[0][Newest.Header.CharacterId.Value]={3,4,5}; const int32 Writes=Disk->Writes;
    {
        FRuntime Runtime(Disk); TestFalse(TEXT("Both corrupt fail closed"),Runtime.Restore(Newest.Header.CharacterId,true));
        TestFalse(TEXT("No replacement identity"),Runtime.Session->HasCharacter());
        TestTrue(TEXT("Visible invalid profile"),Runtime.UI->Error().Contains(TEXT("Unreadable")));
    }
    { FRuntime Other(Disk); TestTrue(TEXT("Other character continues when selected pair is corrupt"),Other.Restore(Unaffected.Header.CharacterId));
      TestTrue(TEXT("Other record unchanged by corruption"),Equal(Unaffected,Other.Session->Snapshot())); }
    TestEqual(TEXT("No silent writes"),Disk->Writes,Writes);
    TestEqual(TEXT("Bad generations retained"),Disk->Slots[0][Newest.Header.CharacterId.Value].Num(),3);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWave2SaveFailure,"Lighthaven.Integration.Wave2.SaveFailureRetry",LHWave2TestsPrivate::Flags)
bool FLHWave2SaveFailure::RunTest(const FString&)
{
    using namespace LHWave2TestsPrivate;
    auto Disk=MakeShared<FStorage>(); Disk->bFail=true; FRuntime Runtime(Disk); int32 Travels=0,Exits=0;
    Runtime.Session->Travel=[&](){++Travels;}; Runtime.Session->Exit=[&](){++Exits;};
    Runtime.Create(); const auto Committed=Runtime.Session->Snapshot(); Runtime.Flush();
    TestEqual(TEXT("Failed save cannot travel"),Travels,0); TestTrue(TEXT("Failure visible in presenter"),Runtime.UI->Error().Contains(TEXT("Save failed")));
    Runtime.UI->ConfirmCreation(); Runtime.Flush(); TestEqual(TEXT("Repeated confirm no duplicate write"),Disk->Writes,1);
    TestEqual(TEXT("Identity retained"),Runtime.Session->Snapshot().Header.CharacterId.Value,Committed.Header.CharacterId.Value);
    Runtime.UI->Quit(); TestEqual(TEXT("Failed durability blocks quit"),Exits,0);
    Disk->bFail=false; Runtime.UI->RetryPersistence(); TestEqual(TEXT("Confirmed quit completes only after retry durability"),Exits,1);
    TestEqual(TEXT("Quit takes precedence over initial entry"),Travels,0);
    TestEqual(TEXT("Retry same identity"),Runtime.Session->Snapshot().Header.CharacterId.Value,Committed.Header.CharacterId.Value);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWave2ClosureTest,"Lighthaven.Integration.Wave2.CanonicalClosure",LHWave2TestsPrivate::Flags)
bool FLHWave2ClosureTest::RunTest(const FString&)
{
    auto P=LHWave2::PrototypeProfile(); const auto Bytes=LHWave2::MechanicalClosure(P);
    const uint8 Prefix[]={21,0,0,0,10,0,0,0,'A','l','g','o','r','i','t','h','m','s'};
    TestTrue(TEXT("Frozen canonical closure struct prefix"),Bytes.Num()>int32(sizeof(Prefix)) && FMemory::Memcmp(Bytes.GetData(),Prefix,sizeof(Prefix))==0);
    TestEqual(TEXT("Mechanical hash covers actual resolved profile"),LHSave::Sha256(Bytes),P.Reference.ContentHash);
    P.Reference.ContentHash=TEXT("excluded"); P.Reference.MigrationPolicy=ELHMigrationPolicy::RequireExplicitMigration;
    TestTrue(TEXT("Hash and migration policy excluded"),LHWave2::MechanicalClosure(P)==Bytes);
    P.Items.Swap(0,1); P.StarterItems.Swap(0,1);
    TestTrue(TEXT("Mechanical definition sets canonicalize"),LHWave2::MechanicalClosure(P)==Bytes);
    P.InitialHealth.Value++; TestTrue(TEXT("Mechanical mutation changes hash"),LHSave::Sha256(LHWave2::MechanicalClosure(P))!=LHSave::Sha256(Bytes));
    P=LHWave2::PrototypeProfile(); P.InitialHealth.Provenance.Notes+=TEXT(" edited");
    TestTrue(TEXT("Provenance participates"),LHSave::Sha256(LHWave2::MechanicalClosure(P))!=LHSave::Sha256(Bytes));
    TestEqual(TEXT("Gameplay catalog hash"),LHSave::Sha256(LHWave2::GameplayCatalogClosure()),LHWave2::CatalogHash());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWave2ReadbackFailure,"Lighthaven.Integration.Wave2.ReadbackRetryHighWater",LHWave2TestsPrivate::Flags)
bool FLHWave2ReadbackFailure::RunTest(const FString&)
{
    using namespace LHWave2TestsPrivate;
    auto Disk=MakeShared<FStorage>(); Disk->bFailReadback=true; FRuntime Runtime(Disk); int32 Travels=0;
    Runtime.Session->Travel=[&](){++Travels;}; Runtime.Create(); auto Initial=Runtime.Session->Snapshot(); Runtime.Flush();
    TestEqual(TEXT("Readback failure blocks initial travel"),Travels,0);
    TestTrue(TEXT("Concrete readback error visible"),Runtime.UI->Error().Contains(TEXT("readback")));
    Disk->bFailReadback=false; Runtime.UI->RetryPersistence();
    TestEqual(TEXT("Retry higher durable sequence matches owner operation"),Travels,1);
    auto Saved=Runtime.Session->Snapshot(); TestEqual(TEXT("Durable high-water retained"),Saved.Header.TransactionSequence,Initial.Header.TransactionSequence+1);
    TestEqual(TEXT("Committed identity retained"),Saved.Header.CharacterId.Value,Initial.Header.CharacterId.Value);
    Runtime.Session->Bind(Runtime.State); Runtime.UI->Allocate(Points()); Runtime.Flush();
    TestEqual(TEXT("Next authority command advances from durable high-water"),Runtime.Session->Snapshot().Header.TransactionSequence,Saved.Header.TransactionSequence+1);
    return true;
}
#endif
