#include "Misc/AutomationTest.h"
#include "Framework/LHWave2Session.h"
#include "Framework/LHWave2Profile.h"
#include "Framework/LHWave2Closure.h"
#include "World/LHTravelSaveAdapter.h"
#include "Framework/LHPlayerState.h"
#include "Framework/LHPlayerController.h"
#include "Framework/LHCharacter.h"
#include "Abilities/LHCombatComponent.h"
#include "Abilities/LHAttributeSet.h"
#include "AI/LHEncounterDirector.h"
#include "Framework/LHEnemyCharacter.h"
#include "World/LHWorldMarkers.h"
#include "Data/Enemies/LHEnemyCatalog.h"
#include "Data/Items/LHItemCatalog.h"
#include "Data/Encounters/LHEncounterCatalog.h"
#include "Rewards/LHEncounterLifecycle.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameInstance.h"
#include "EnhancedInputSubsystems.h"
#include "UI/LHUIWidgetHarness.h"
#include "UI/LHFrontendGameMode.h"
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
    TSharedPtr<FLHSaveStore> Store;
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
        Store=MakeShared<FLHSaveStore>(Disk); Session=MakeShared<FLHWave2Session>(Store.ToSharedRef()); Session->Bind(State);
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
    void EarnAllocationPoints()
    {
        check(State->GetCharacterAuthority()->Authority().GrantExperience(1000)==ELHCommandReason::None);
    }
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
        auto* C=Runtime.State->GetCombatComponent(); TestNull(TEXT("No combat pawn avatar before initialization"),Runtime.State->GetCombatAvatar());
        TestEqual(TEXT("Restored health installed before avatar"),C->GetCombatAttributes()->GetHealth(),float(Expected.Character.CurrentHealth.Value));
        auto* Avatar=Runtime.World->SpawnActor<ALHCharacter>(); Runtime.State->InitializeAvatar(Avatar);
        TestEqual(TEXT("Initialization binds the restored combat pawn"),Runtime.State->GetCombatAvatar(),static_cast<APawn*>(Avatar));
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
        Runtime.EarnAllocationPoints();
        auto Before=Runtime.Session->Snapshot();
        auto Review=Runtime.UI->ReviewAllocation(Points()); TestTrue(TEXT("Owner legal review"),Review.bLegal);
        TestTrue(TEXT("Review did not alter receipts or snapshot"),Equal(Before,Runtime.Session->Snapshot()));
        TestTrue(TEXT("Allocate"),Runtime.UI->Allocate(Points()).Disposition==ELHCommandDisposition::Accepted); Runtime.Flush();
        auto Items=Runtime.Session->Snapshot().Character.Inventory;
        const auto* Dirk=Items.FindByPredicate([](const auto& I){return I.Definition.Value==TEXT("Item.RustedDirk");});
        if (!Dirk) { AddError(TEXT("Stage 1 starter inventory missing")); return false; }
        TestTrue(TEXT("Unequip starter dirk"),Runtime.UI->Equip(Dirk->Id,ELHEquipmentSlot::MainHand,true).Disposition==ELHCommandDisposition::Accepted); Runtime.Flush();
        TestTrue(TEXT("Equip starter dirk"),Runtime.UI->Equip(Dirk->Id,ELHEquipmentSlot::MainHand,false).Disposition==ELHCommandDisposition::Accepted); Runtime.Flush();
        // Trusted growth settlement fixture, followed by a real completed allocation command/save.
        TestTrue(TEXT("Canonical growth IDs"),Runtime.State->GetCharacterAuthority()->Authority().GrantExperience(5700)==ELHCommandReason::None);
        TestTrue(TEXT("Persist growth at completed command boundary"),Runtime.UI->Allocate(Points()).Disposition==ELHCommandDisposition::Accepted); Runtime.Flush();
        Saved=Runtime.Session->Snapshot(); TestEqual(TEXT("Growth retained"),Saved.Character.GrowthAwards.Num(),2);
        Runtime.UI->Refresh(); const auto Sheet=ILHUIWidgetHarness::Create(*Runtime.UI); Sheet->Open(ELHUIScreen::CharacterSheet);
        const FString Text=Sheet->SummaryText();
        const auto Stats=Runtime.State->GetCharacterAuthority()->Authority().Stats();
        TestTrue(TEXT("Effective strength comes from authority"),Text.Contains(FString::Printf(TEXT("Strength %lld"),Stats.Value.Effective.Strength)));
        TestTrue(TEXT("Gear effects shown"),Text.Contains(TEXT("Gear attribute effects (Prototype)")));
        TestTrue(TEXT("Actual growth awards shown"),Text.Contains(TEXT("Growth awards: 2")) && Text.Contains(TEXT("HP +")));
        TestFalse(TEXT("No Unknown placeholders"),Text.Contains(TEXT("Unknown")));
    }
    FRuntime Restored(Disk); TestTrue(TEXT("Restore equipped character"),Restored.Restore(Saved.Header.CharacterId));
    TestTrue(TEXT("Inventory equipment attributes growth resources and session equal"),Equal(Saved,Restored.Session->Snapshot()));
    TestEqual(TEXT("Derived equipment rebuilt"),Restored.State->GetCombatComponent()->GetCombatAttributes()->GetAccuracy(),10.f);
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
        Runtime.EarnAllocationPoints(); Runtime.UI->Allocate(Points()); Runtime.Flush(); Newest=Runtime.Session->Snapshot();
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
    const auto Widget=ILHUIWidgetHarness::Create(*Runtime.UI);
    Widget->Open(ELHUIScreen::Creation);
    Widget->Activate("Quit"); Widget->Key(ELHUITestKey::Down,false); Widget->Key(ELHUITestKey::South,false);
    TestEqual(TEXT("Failed durability blocks quit"),Exits,0);
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWave2UnsavedSheet,"Lighthaven.Integration.Wave2.UnsavedSheetRetry",LHWave2TestsPrivate::Flags)
bool FLHWave2UnsavedSheet::RunTest(const FString&)
{
    using namespace LHWave2TestsPrivate;
    auto Disk=MakeShared<FStorage>(); FRuntime Runtime(Disk);
    Runtime.Create(); Runtime.Flush(); Runtime.Session->Bind(Runtime.State);
    Runtime.EarnAllocationPoints();
    Runtime.UI->Refresh(); const auto W=ILHUIWidgetHarness::Create(*Runtime.UI); W->Open(ELHUIScreen::CharacterSheet);
    const auto Previous=Runtime.Session->Snapshot();
    Disk->bFail=true; W->Activate("Strength"); W->Activate("Confirm"); W->Key(ELHUITestKey::Down); W->Key(ELHUITestKey::South);
    TestTrue(TEXT("Queued action marked unsaved"),W->SummaryText().Contains(TEXT("(unsaved)")));
    Runtime.Flush(); Runtime.UI->Refresh();
    TestTrue(TEXT("Failed action retains unsaved badge"),W->SummaryText().Contains(TEXT("(unsaved)")));
    TestEqual(TEXT("In-memory allocation retained"),Runtime.UI->Snapshot().Character.BaseAttributes.Strength.Value,Previous.Character.BaseAttributes.Strength.Value+1);
    FLHSaveStore Reader(Disk); FLHSaveSnapshot Loaded; FLHSaveError Error;
    TestTrue(TEXT("Previous disk generation readable"),Reader.Load(Previous.Header.CharacterId,FLHWave2Session::Compatibility(),Loaded,Error));
    TestEqual(TEXT("Disk still holds previous strength"),Loaded.Character.BaseAttributes.Strength.Value,Previous.Character.BaseAttributes.Strength.Value);
    TestTrue(TEXT("Failure replaces acceptance"),W->MessageText().Contains(TEXT("Save failed")));
    W->Activate("Reset"); TestTrue(TEXT("Reset preserves save failure"),W->MessageText().Contains(TEXT("Save failed")));
    Disk->bFail=false; W->Activate("RetrySave"); Runtime.UI->Refresh();
    TestFalse(TEXT("Durability removes unsaved badge"),W->SummaryText().Contains(TEXT("(unsaved)")));
    TestTrue(TEXT("Durability removes stale acceptance and failure"),W->MessageText().IsEmpty());
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
    Runtime.Session->Bind(Runtime.State); Runtime.EarnAllocationPoints(); Runtime.UI->Allocate(Points()); Runtime.Flush();
    TestEqual(TEXT("Next authority command advances from durable high-water"),Runtime.Session->Snapshot().Header.TransactionSequence,Saved.Header.TransactionSequence+2);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWave2WidgetQuit,"Lighthaven.Integration.Wave2.WidgetCreationFailureQuitRetry",LHWave2TestsPrivate::Flags)
bool FLHWave2WidgetQuit::RunTest(const FString&)
{
    using namespace LHWave2TestsPrivate;
    auto Disk=MakeShared<FStorage>(); Disk->bFail=true; FRuntime R(Disk); int32 Travels=0,Exits=0;
    R.Session->Travel=[&](){++Travels;}; R.Session->Exit=[&](){++Exits;};
    const auto W=ILHUIWidgetHarness::Create(*R.UI); W->Open(ELHUIScreen::Creation); W->EditName(TEXT("Charlie"));
    W->Activate("Roll");
    for (FName Category:{"Body","Hair","Skin","Outfit"})
    {
        TestTrue(TEXT("missing category visible in roll error"),R.UI->Error().Contains(Category.ToString()));
        TestTrue(TEXT("category has a separate control"),W->HasControl(Category));
    }
    for (FName Category:{"Body","Hair","Skin","Outfit"}) W->Activate(Category);
    for (FName Question:{"Question1","Question2","Question3","Question4"}) W->Activate(Question);
    W->Activate("Roll"); TestTrue(TEXT("category form gets authoritative legal roll"),R.UI->CreationPreview().bLegal);
    W->Activate("Confirm"); W->Key(ELHUITestKey::Down,false); W->Key(ELHUITestKey::South,false); R.Flush();
    TestTrue(TEXT("first-save failure avoids retained-generation claim"),R.UI->Error().Contains(TEXT("no saved generation")) && !R.UI->Error().Contains(TEXT("previous generation retained")));
    W->Activate("Quit"); W->Key(ELHUITestKey::Down,false); W->Key(ELHUITestKey::South,false);
    TestEqual(TEXT("failed durability still blocks quit"),Exits,0);
    Disk->bFail=false; W->Activate("RetrySave");
    TestEqual(TEXT("same widget accepted creation then confirmed quit exits on retry"),Exits,1);
    TestEqual(TEXT("confirmed quit suppresses creation travel"),Travels,0);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWave2InputHandoff,"Lighthaven.Integration.Wave2.GameplayInputHandoff",LHWave2TestsPrivate::Flags)
bool FLHWave2InputHandoff::RunTest(const FString&)
{
    using namespace LHWave2TestsPrivate;
    auto Disk=MakeShared<FStorage>(); FRuntime R(Disk);
    auto* Viewport=NewObject<UGameViewportClient>(GEngine);
    auto* Context=GEngine->GetWorldContextFromWorld(R.World); Context->GameViewport=Viewport;
    // Headless automation has no SViewport: exercise both controllers' runtime
    // input application seams against the same viewport and local subsystem.
    auto* Instance=NewObject<UGameInstance>(GEngine);
    auto* Local=NewObject<ULocalPlayer>(GEngine);
    Instance->AddLocalPlayer(Local,FPlatformUserId::CreateFromInternalId(0));
    // The standalone test instance has no world context; attach its headless
    // viewport after registration has initialized the local player subsystems.
    Local->ViewportClient=Viewport;
    R.Controller->Player=Local; Local->PlayerController=R.Controller;
    auto* Frontend=R.World->SpawnActor<ALHFrontendController>();
    Frontend->EstablishFrontendInput();
    TestTrue(TEXT("frontend UI-only ignores gameplay input"),Viewport->IgnoreInput());
    R.Controller->EstablishGameplayInput();
    TestFalse(TEXT("gameplay handoff restores viewport input"),Viewport->IgnoreInput());
    auto* Input=Local->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
    TestNotNull(TEXT("local enhanced input subsystem"),Input);
    if (Input) TestTrue(TEXT("Gameplay mapping context installed"),Input->HasMappingContext(R.Controller->InputConfig->Context(ELHInputContext::Gameplay)));
    Frontend->EstablishFrontendInput(); R.Controller->Possess(R.World->SpawnActor<ALHCharacter>());
    TestFalse(TEXT("possession also restores viewport input"),Viewport->IgnoreInput());
    TestTrue(TEXT("pointer remains visible for selection"),R.Controller->bShowMouseCursor);
    Frontend->EstablishFrontendInput(); Frontend->EndPlay(EEndPlayReason::LevelTransition);
    TestFalse(TEXT("frontend handoff restores defaults"),Viewport->IgnoreInput());
    R.Controller->Player=nullptr; Local->PlayerController=nullptr; Instance->RemoveLocalPlayer(Local); Context->GameViewport=nullptr;
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWave3LiveSession,"Lighthaven.Integration.Wave3.SessionCheckpoints",LHWave2TestsPrivate::Flags)
bool FLHWave3LiveSession::RunTest(const FString&)
{
    using namespace LHWave2TestsPrivate;
    auto Disk=MakeShared<FStorage>(); FRuntime R(Disk);
    TestTrue(TEXT("Create production session"),R.Create().Disposition==ELHCommandDisposition::Accepted); R.Flush();
    TestTrue(TEXT("Bind initial hub avatar authority"),R.Session->Bind(R.State));
    const auto Initial=R.Session->Snapshot();
    TestTrue(TEXT("Creation uses hub safe spawn"),LHWorld::SameEntrance(Initial.Character.ActiveEntrance,LHWorld::Registry()[0].SafeFallback));
    FLHSaveError SaveError; auto Old=Initial; Old.Character.ActiveEntrance.LocalId=TEXT("Entry");
    TestFalse(TEXT("Temporary Wave2 entrance not silently reinterpreted"),FLHWave2Session::Compatibility().ValidateReferences(Old,SaveError));
    FLHTravelSaveAdapter* Adapter=nullptr;
    FLHTravelSaveAdapter::FHooks Hooks;
    Hooks.Freeze=[&](bool Frozen) { R.Session->FreezeWorldTravel(Frozen); };
    Hooks.Capture=[&](FLHSaveSnapshot& Out,FString& Error) { return R.Session->CaptureTravel(Out,Error); };
    Hooks.Durable=[&](const FLHSaveSnapshot& Snapshot) { TestTrue(TEXT("Publish durable canonical session"),R.Session->InstallTravel(Snapshot)); };
    Hooks.Load=[](const FLHAreaDefinition&,uint64) {};
    Hooks.Install=[&](const FLHSaveSnapshot& Snapshot,const FLHEntranceDefinition&,FString&) { return R.Session->InstallTravel(Snapshot); };
    Hooks.Restore=[&](const FLHSaveSnapshot& Snapshot,const FLHEntranceDefinition&,uint64 Token) { R.Session->InstallTravel(Snapshot); Adapter->Travel().OnSourceRestored(Token,true,TEXT("")); };
    FLHTravelSaveAdapter Travel(R.Store.ToSharedRef(),FLHWave2Session::Compatibility(),MoveTemp(Hooks)); Adapter=&Travel;
    R.Session->RequestWorldTravel=[&](const FLHRequestTravelRequest& Q,FString& Error) { return Travel.Travel().Begin(Q,Error); };
    const int32 Route[]={1,2,3,4,3,2,1,0};
    for (int32 Destination:Route)
    {
        const auto S=R.Session->Snapshot(); const auto* Area=LHWorld::FindArea(S.Character.ActiveEntrance.Area);
        const auto* Edge=Area->Portals.FindByPredicate([&](const auto& P) { return LHWorld::SameArea(P.Destination.Area,LHWorld::Registry()[Destination].Id); });
        if (!TestNotNull(TEXT("Session registered edge"),Edge)) return false;
        FLHRequestTravelRequest Q; Q.Request.Value=FGuid::NewGuid(); Q.Request.Epoch=S.Session.RequestEpoch;
        Q.Portal=Edge->Portal; Q.Portal.RunId=S.World.RunId; Q.Destination=Edge->Destination;
        TestTrue(TEXT("Production session dispatches travel"),R.Session->Execute(Q).Disposition==ELHCommandDisposition::Accepted);
        TestTrue(TEXT("Session blocked during load"),R.Session->IsBlocked());
        Travel.Travel().OnDestinationLoaded(Travel.Travel().GetToken(),true,Q.Destination,TEXT(""));
        TestFalse(TEXT("Session resumes after durable arrival"),R.Session->IsBlocked());
        FLHSaveStore Reload(Disk); FLHSaveSnapshot Loaded;
        TestTrue(TEXT("Reload using production catalog compatibility"),Reload.Load(S.Header.CharacterId,FLHWave2Session::Compatibility(),Loaded,SaveError));
        TestTrue(TEXT("Production reload destination entrance"),LHWorld::SameEntrance(Loaded.Character.ActiveEntrance,Q.Destination));
        TestTrue(TEXT("Authority capture after reload install"),R.Session->InstallTravel(Loaded));
        TestEqual(TEXT("No travel rewards"),Loaded.Character.Gold.Value,Initial.Character.Gold.Value);
    }
    R.Session->RequestWorldTravel=nullptr;
    R.Session->AbortGameplayArrival(TEXT("Injected invalid spawn"));
    TestFalse(TEXT("Refused startup permits frontend recovery"),R.Session->IsBlocked());
    TestTrue(TEXT("Refusal is visible"),R.Session->OwnerStatus().Contains(TEXT("save retained")));
    return !HasAnyErrors();
}

namespace LHStage1IntegrationTestsPrivate
{
using namespace LHWave2TestsPrivate;
ALHCharacter* Avatar(FRuntime& R)
{
    FActorSpawnParameters P; P.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    auto* A=R.World->SpawnActor<ALHCharacter>(FVector(0,0,90),FRotator::ZeroRotator,P);
    R.State->InitializeAvatar(A); return A;
}
ALHInteractableMarker* Npc(FRuntime& R,const TCHAR* Name,ALHCharacter* Pawn)
{
    auto* N=R.World->SpawnActor<ALHInteractableMarker>(Pawn->GetActorLocation()-FVector(0,0,90),FRotator::ZeroRotator);
    N->Area=R.Session->Snapshot().Character.ActiveEntrance.Area; N->InstanceId=FGuid::NewGuid(); N->DefinitionId.Value=Name; return N;
}
FLHInteractRequest Interact(FRuntime& R,ALHInteractableMarker* N,const TCHAR* Topic)
{
    const auto S=R.Session->Snapshot(); FLHInteractRequest Q; Q.Request.Epoch=S.Session.RequestEpoch; Q.Request.Value=FGuid::NewGuid();
    Q.Target=N->Materialize(S.World.RunId); Q.Topic.Value=Topic; return Q;
}
bool B1(FRuntime& R,ALHCharacter* Pawn)
{
    auto S=R.Session->Snapshot(); S.Character.ActiveEntrance=LHWorld::Registry()[1].Entrances[0].Id;
    if (!R.Session->InstallTravel(S)) return false;
    Pawn->SetActorLocation(FVector(-2500,600,90));
    for (const auto& Slot:LHWorld::Registry()[1].Spawns)
    {
        auto* M=R.World->SpawnActor<ALHSpawnMarker>(Slot.Anchor.GetLocation(),Slot.Anchor.Rotator());
        M->Area=LHWorld::Registry()[1].Id; M->SpawnId=Slot.SpawnId; M->EnemyDefinitionId=Slot.Enemy;
    }
    return R.Session->StartEncounters();
}
bool Kill(FRuntime& R,ALHCharacter* Pawn,ALHEnemyCharacter* Enemy)
{
    if (!Enemy) return false;
    Pawn->SetActorLocation(Enemy->GetActorLocation()+FVector(100,0,0));
    auto* Combat=R.State->GetCombatComponent();
    const auto Before=R.Session->Snapshot();
    for(int32 Swing=0;Swing<100 && Enemy->IsAlive();++Swing)
    {
        TMap<FName,double> Empty; Combat->RestoreCooldownMap(Empty);
        FLHUseAbilityRequest Q; Q.Request.Value=FGuid::NewGuid(); Q.Request.Epoch=Before.Session.RequestEpoch;
        Q.Ability.Value=TEXT("Attack.Melee.Basic"); Q.Target=Enemy->GetEntityId(Before.World.RunId);
        if(R.Session->Execute(Q).Disposition!=ELHCommandDisposition::Accepted) return false;
    }
    return Enemy->IsCorpse();
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHStage1Errand,"Lighthaven.Integration.Wave4.Stage1A.ErrandLoop",LHWave2TestsPrivate::Flags)
bool FLHStage1Errand::RunTest(const FString&)
{
    using namespace LHStage1IntegrationTestsPrivate;
    auto Disk=MakeShared<FStorage>(); FLHSaveSnapshot Finished;
    {
        FRuntime R(Disk); if(!TestTrue(TEXT("create"),R.Create().Disposition==ELHCommandDisposition::Accepted)) return false;
        R.Flush(); R.Session->Bind(R.State); auto* Pawn=Avatar(R);
        auto* Samaritan=Npc(R,TEXT("NPC.Samaritan"),Pawn); auto Accept=Interact(R,Samaritan,TEXT("Topic.AcceptRats"));
        TestTrue(TEXT("accept through session"),R.Session->Execute(Accept).Disposition==ELHCommandDisposition::Accepted); R.Flush();
        TestTrue(TEXT("same accept replays"),R.Session->Execute(Accept).bReplay);
        if(!TestTrue(TEXT("hydrate B1"),B1(R,Pawn))) return false;
        R.Flush(); auto* Director=R.World->GetSubsystem<ULHEncounterDirector>();
        for(int32 K=0;K<15;++K)
        {
            auto S=R.Session->Snapshot(); auto* A=S.World.Areas.FindByPredicate([](const auto& X){return X.Area.Content.Value==TEXT("Area.TempleB1");});
            if (!A) return false;
            auto* E=A->Encounters.FindByPredicate([](const auto& X){return X.Definition.Value==TEXT("Enemy.BrownRat") && X.State==ELHEncounterLifeState::Alive;});
            if(!E)
            {
                // Synthetic safe-spawn fixture exercises the real persisted timer/generation function; physical nav safety is a host check.
                TArray<FLHSpawnLifeId> Lives;
                auto Hp=[](const FLHContentId& Id){return LHEnemyData::Find(Id)->Health;};
                TestTrue(TEXT("advance loaded timers"),LHRewards::AdvanceRespawns(*A,S.World.RunId,120,[](const FGuid&){return true;},Hp,Lives)==ELHCommandReason::None);
                TestTrue(TEXT("install respawn candidate"),R.Session->InstallTravel(S)); Director->Populate(*A);
                E=A->Encounters.FindByPredicate([](const auto& X){return X.Definition.Value==TEXT("Enemy.BrownRat") && X.State==ELHEncounterLifeState::Alive;});
            }
            if(!E || !TestTrue(TEXT("legal live melee kill"),Kill(R,Pawn,Director->FindByLife(E->Life)))) return false;
            R.Flush();
            const auto After=R.Session->Snapshot();
            TestEqual(TEXT("one eligible kill"),After.World.Quests[0].EligibleKillCount.Value,int64(K+1));
        }
        auto S=R.Session->Snapshot(); S.Character.ActiveEntrance=LHWorld::Registry()[0].SafeFallback; R.Session->InstallTravel(S);
        Pawn->SetActorLocation(Samaritan->GetActorLocation()+FVector(0,0,90));
        auto TurnIn=Interact(R,Samaritan,TEXT("Topic.TurnInRats"));
        TestTrue(TEXT("turn in through session"),R.Session->Execute(TurnIn).Disposition==ELHCommandDisposition::Accepted); R.Flush();
        Finished=R.Session->Snapshot(); TestEqual(TEXT("15 rats plus single 2500 XP reward"),Finished.Character.ExperienceBalance.Value,int64(3175));
        TestTrue(TEXT("turn-in receipt replays"),R.Session->Execute(TurnIn).bReplay);
        auto Again=Interact(R,Samaritan,TEXT("Topic.TurnInRats")); TestTrue(TEXT("new turn-in cannot reward twice"),R.Session->Execute(Again).Disposition==ELHCommandDisposition::Rejected);
    }
    FRuntime Reload(Disk); TestTrue(TEXT("reload completed errand"),Reload.Restore(Finished.Header.CharacterId));
    TestTrue(TEXT("canonical errand state and claims persist"),Equal(Finished,Reload.Session->Snapshot())); return !HasAnyErrors();
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHStage1Melee,"Lighthaven.Integration.Wave4.LiveMeleeCommand",LHWave2TestsPrivate::Flags)
bool FLHStage1Melee::RunTest(const FString&)
{
    using namespace LHStage1IntegrationTestsPrivate;
    auto Disk=MakeShared<FStorage>(); FRuntime R(Disk); R.Create(); R.Flush(); R.Session->Bind(R.State); auto* Pawn=Avatar(R);
    if(!TestTrue(TEXT("B1 encounter startup"),B1(R,Pawn))) return false; R.Flush();
    auto S=R.Session->Snapshot(); auto* Director=R.World->GetSubsystem<ULHEncounterDirector>(); const auto Life=S.World.Areas.Last().Encounters[0].Life;
    auto* Enemy=Director->FindByLife(Life); if(!TestNotNull(TEXT("live actor"),Enemy)) return false;
    TestTrue(TEXT("command produces settled kill"),Kill(R,Pawn,Enemy));
    auto After=R.Session->Snapshot(); TestEqual(TEXT("one Brown Rat XP grant"),After.Character.ExperienceBalance.Value,int64(45));
    TestFalse(TEXT("save queued does not freeze combat/movement"),R.Session->IsBlocked());
    TestFalse(TEXT("duplicate life rejects"),R.Session->SettleEnemyKill(Life,{},Pawn)); R.Flush();
    FLHTakeLootRequest Q; Q.Request.Value=FGuid::NewGuid(); Q.Request.Epoch=After.Session.RequestEpoch; Q.Container=LHRewards::CorpseContainerFor(After.World.RunId,Life);
    Q.Kind=ELHLootTransferKind::Gold; Q.Quantity=LHWave2::PrototypeInteger(3);
    TestTrue(TEXT("near corpse gold through session"),R.Session->Execute(Q).Disposition==ELHCommandDisposition::Accepted); R.Flush();
    TestTrue(TEXT("gold pickup replays"),R.Session->Execute(Q).bReplay); TestEqual(TEXT("gold once"),R.Session->Snapshot().Character.Gold.Value,int64(103));
    FLHSaveStore Read(Disk); FLHSaveSnapshot Loaded; FLHSaveError Error;
    TestTrue(TEXT("kill and gold reload"),Read.Load(After.Header.CharacterId,FLHWave2Session::Compatibility(),Loaded,Error));
    TestEqual(TEXT("durable XP"),Loaded.Character.ExperienceBalance.Value,int64(45)); TestEqual(TEXT("durable gold"),Loaded.Character.Gold.Value,int64(103)); return !HasAnyErrors();
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHStage1Resources,"Lighthaven.Integration.Wave4.ResourceSyncAtBoundary",LHWave2TestsPrivate::Flags)
bool FLHStage1Resources::RunTest(const FString&)
{
    using namespace LHStage1IntegrationTestsPrivate;
    auto Disk=MakeShared<FStorage>(); FRuntime R(Disk); R.Create(); R.Flush(); R.Session->Bind(R.State); Avatar(R);
    auto* C=R.State->GetCombatComponent(); C->SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(),13); C->SetNumericAttributeBase(ULHAttributeSet::GetManaAttribute(),2); C->ManaRegenFractionalSeconds=4.75;
    FLHSaveSnapshot Captured; FString Detail; TestTrue(TEXT("travel boundary captures live pools"),R.Session->CaptureTravel(Captured,Detail));
    TestEqual(TEXT("health capture"),Captured.Character.CurrentHealth.Value,13.0); TestEqual(TEXT("mana capture"),Captured.Character.CurrentMana.Value,2.0);
    TestEqual(TEXT("fraction capture without ticking"),Captured.Session.ManaRegenFractionalSeconds.Value,4.75);
    R.Session->bInGameplay=true;
    TestTrue(TEXT("exit queues live boundary"),R.Session->RequestExit().IsEmpty()); R.Flush();
    FLHSaveStore Read(Disk); FLHSaveSnapshot Loaded; FLHSaveError Error;
    TestTrue(TEXT("load live boundary"),Read.Load(Captured.Header.CharacterId,FLHWave2Session::Compatibility(),Loaded,Error));
    TestEqual(TEXT("damage survives reload"),Loaded.Character.CurrentHealth.Value,13.0); TestEqual(TEXT("spent mana survives reload"),Loaded.Character.CurrentMana.Value,2.0);
    TestEqual(TEXT("no extra regen tick"),Loaded.Session.ManaRegenFractionalSeconds.Value,4.75); return !HasAnyErrors();
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHStage1Catalog,"Lighthaven.Integration.Wave4.CatalogCrossRefs",LHWave2TestsPrivate::Flags)
bool FLHStage1Catalog::RunTest(const FString&)
{
    TestEqual(TEXT("eleven enemies"),LHEnemyData::Catalog().Num(),11); TestEqual(TEXT("77 slots"),LHEncounterData::Catalog().Num(),77);
    for(const auto& E:LHEncounterData::Catalog()) TestNotNull(TEXT("slot resolves"),LHEnemyData::Find(E.Enemy));
    for(const auto& E:LHEnemyData::Catalog()) for(const auto& L:E.Reward.Loot) TestNotNull(TEXT("loot item resolves"),LHItemData::Find(L.Item));
    const auto P=LHWave2::PrototypeProfile(); TestEqual(TEXT("Bible XP table"),P.Rules.Progression.Thresholds.Num(),200);
    TestEqual(TEXT("starting HP Prototype"),P.InitialHealth.Value,30.0); TestEqual(TEXT("starting gold Prototype"),P.InitialGold.Value,int64(100));
    TestEqual(TEXT("six individual kit instances"),P.StarterItems.Num(),6); return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHStage1Death,"Lighthaven.Integration.Wave4.DeathRespawnSafe",LHWave2TestsPrivate::Flags)
bool FLHStage1Death::RunTest(const FString&)
{
    using namespace LHStage1IntegrationTestsPrivate;
    auto Disk=MakeShared<FStorage>(); FRuntime R(Disk); R.Create(); R.Flush(); R.Session->Bind(R.State); Avatar(R);
    const auto Before=R.Session->Snapshot();
    R.State->GetCombatComponent()->SetNumericAttributeBase(ULHAttributeSet::GetHealthAttribute(),0);
    R.Session->HandlePlayerDeath(); R.State->ClearAvatar();
    TestTrue(TEXT("dead player is movement blocked"),R.Session->IsBlocked());
    TestTrue(TEXT("death screen owner state"),R.Session->HudState().bDead);
    R.Flush(); TestTrue(TEXT("respawn accepted after durable death"),R.Session->RequestRespawn().IsEmpty());
    TestTrue(TEXT("no duplicate recovery while save queued"),!R.Session->RequestRespawn().IsEmpty()); R.Flush();
    auto After=R.Session->Snapshot();
    TestTrue(TEXT("church destination"),LHWorld::SameEntrance(After.Character.ActiveEntrance,After.Session.SafeRespawn.Entrance));
    TestEqual(TEXT("inventory intact"),After.Character.Inventory.Num(),Before.Character.Inventory.Num());
    TestEqual(TEXT("full Prototype HP"),After.Character.CurrentHealth.Value,30.0);
    TestEqual(TEXT("no duplicate rewards"),After.World.ClaimedUniqueRewards.Num(),Before.World.ClaimedUniqueRewards.Num());
    FLHSaveStore Read(Disk); FLHSaveSnapshot Loaded; FLHSaveError Error;
    TestTrue(TEXT("recovered checkpoint reloads"),Read.Load(After.Header.CharacterId,FLHWave2Session::Compatibility(),Loaded,Error));
    TestEqual(TEXT("recovered HP persists"),Loaded.Character.CurrentHealth.Value,30.0);
    // This is a session/review-hash fixture; physical capsule/floor/nav review remains the host ArrivalSafety test.
    return !HasAnyErrors();
}
#endif
