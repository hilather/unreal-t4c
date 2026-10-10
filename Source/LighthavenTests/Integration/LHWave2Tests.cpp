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
#include "Abilities/LHAbilityCatalog.h"
#include "AI/LHEncounterDirector.h"
#include "Framework/LHEnemyCharacter.h"
#include "World/LHWorldMarkers.h"
#include "Data/Enemies/LHEnemyCatalog.h"
#include "Data/Items/LHItemCatalog.h"
#include "Data/Encounters/LHEncounterCatalog.h"
#include "Rewards/LHEncounterLifecycle.h"
#include "TimerManager.h"
#include "Async/TaskGraphInterfaces.h"
#include "HAL/PlatformProcess.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/TargetPoint.h"
#include "EngineUtils.h"
#include "Components/PrimitiveComponent.h"
#include "Components/CapsuleComponent.h"
#include "StaticMeshCompiler.h"
#include "AssetCompilingManager.h"
#include "AI/NavigationSystemBase.h"
#include "UObject/UnrealType.h"
#include "UObject/Package.h"
#include "UObject/LinkerInstancingContext.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameInstance.h"
#include "Framework/LHSessionSubsystem.h"
#include "Persistence/LHSaveSubsystem.h"
#include "EnhancedInputSubsystems.h"
#include "UI/LHUIWidgetHarness.h"
#include "UI/LHFrontendGameMode.h"
#if WITH_DEV_AUTOMATION_TESTS
namespace LHWave2TestsPrivate
{
constexpr auto LHWave2TestsFlags=EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter;
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
    UGameInstance* Instance=nullptr;
    ULocalPlayer* Local=nullptr;
    UWorld* World=nullptr;
    ALHPlayerState* State=nullptr;
    ALHPlayerController* Controller=nullptr;
    TSharedPtr<FLHSaveStore> Store;
    TSharedPtr<FLHWave2Session> Session;
    TSharedPtr<FLHUIPresenter> UI;
    explicit FRuntime(TSharedRef<FStorage> Disk,bool OwnedSession=false)
    {
        const FName Name=MakeUniqueObjectName(nullptr,UWorld::StaticClass(),TEXT("LHWave2World"),EUniqueObjectNameOptions::GloballyUnique);
        const auto Init=UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(true).RequiresHitProxies(false)
            .CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false).SetTransactional(false);
        auto& Context=GEngine->CreateNewWorldContext(EWorldType::Game);
        World=UWorld::CreateWorld(EWorldType::Game,false,Name,GetTransientPackage(),true,ERHIFeatureLevel::Num,&Init);
        Context.SetCurrentWorld(World); World->InitializeActorsForPlay(FURL());
        Controller=World->SpawnActor<ALHPlayerController>(); Controller->SetAsLocalPlayerController();
        State=World->SpawnActor<ALHPlayerState>(); Controller->SetPlayerState(State); Controller->InitInputSystem();
        if (OwnedSession)
        {
            Instance=NewObject<UGameInstance>(GEngine); Instance->AddToRoot();
            Instance->InitializeStandalone(MakeUniqueObjectName(nullptr,UWorld::StaticClass(),TEXT("LHSessionDummy")));
            auto* Dummy=Instance->GetWorld(); auto* OwnedContext=GEngine->GetWorldContextFromWorld(Dummy);
            Dummy->DestroyWorld(false); GEngine->DestroyWorldContext(World);
            OwnedContext->SetCurrentWorld(World); World->SetGameInstance(Instance);
            Local=NewObject<ULocalPlayer>(GEngine); Instance->AddLocalPlayer(Local,FPlatformUserId::CreateFromInternalId(0));
            Local->PlayerController=Controller; Controller->Player=Local;
            Store=Instance->GetSubsystem<ULHSaveSubsystem>()->GetStore();
            Session=Instance->GetSubsystem<ULHSessionSubsystem>()->Session();
            // Test adapters load generated worlds synchronously. Prevent OpenLevel
            // while retaining the real initialized subsystem and death bridge.
            Session->Travel=[](){}; Session->Exit=[](){};
        }
        else { Store=MakeShared<FLHSaveStore>(Disk); Session=MakeShared<FLHWave2Session>(Store.ToSharedRef()); }
        Session->Bind(State);
        UI=MakeShared<FLHUIPresenter>(*Session,*Session,*Session);
    }
    ~FRuntime()
    {
        UI.Reset(); Session.Reset(); if(Instance) { Controller->Player=nullptr; Local->PlayerController=nullptr; Instance->RemoveLocalPlayer(Local); Instance->Shutdown(); Instance->RemoveFromRoot(); } if(World) { World->DestroyWorld(false); GEngine->DestroyWorldContext(World); }
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
    void Flush()
    {
        Session->Flush();
        if(Instance && Session->HasCharacter())
        {
            const double Deadline=FPlatformTime::Seconds()+10;
            while(Store->IsWriting(Session->Snapshot().Header.CharacterId) && FPlatformTime::Seconds()<Deadline)
            { FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread); FPlatformProcess::Sleep(0.001f); }
            FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread);
        }
    }
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
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWave2Independence,"Lighthaven.Integration.Wave2.IndependentRebuild",LHWave2TestsPrivate::LHWave2TestsFlags)
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
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWave2Equipment,"Lighthaven.Integration.Wave2.EquipmentAllocationRebuild",LHWave2TestsPrivate::LHWave2TestsFlags)
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
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWave2Recovery,"Lighthaven.Integration.Wave2.RecoveryAndUnreadable",LHWave2TestsPrivate::LHWave2TestsFlags)
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
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWave2SaveFailure,"Lighthaven.Integration.Wave2.SaveFailureRetry",LHWave2TestsPrivate::LHWave2TestsFlags)
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWave2ClosureTest,"Lighthaven.Integration.Wave2.CanonicalClosure",LHWave2TestsPrivate::LHWave2TestsFlags)
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWave2UnsavedSheet,"Lighthaven.Integration.Wave2.UnsavedSheetRetry",LHWave2TestsPrivate::LHWave2TestsFlags)
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWave2ReadbackFailure,"Lighthaven.Integration.Wave2.ReadbackRetryHighWater",LHWave2TestsPrivate::LHWave2TestsFlags)
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
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWave2WidgetQuit,"Lighthaven.Integration.Wave2.WidgetCreationFailureQuitRetry",LHWave2TestsPrivate::LHWave2TestsFlags)
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
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWave2InputHandoff,"Lighthaven.Integration.Wave2.GameplayInputHandoff",LHWave2TestsPrivate::LHWave2TestsFlags)
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
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHWave3LiveSession,"Lighthaven.Integration.Wave3.SessionCheckpoints",LHWave2TestsPrivate::LHWave2TestsFlags)
bool FLHWave3LiveSession::RunTest(const FString&)
{
    using namespace LHWave2TestsPrivate;
    auto Disk=MakeShared<FStorage>(); FRuntime R(Disk);
    TestTrue(TEXT("Create production session"),R.Create().Disposition==ELHCommandDisposition::Accepted); R.Flush();
    TestTrue(TEXT("Bind initial hub avatar authority"),R.Session->Bind(R.State));
    auto* TravelAvatar=R.World->SpawnActor<ALHCharacter>(); R.State->InitializeAvatar(TravelAvatar);
    TravelAvatar->SetActorLocation(FVector(0,0,92));
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
        auto* Marker=R.World->SpawnActor<ALHPortal>(); Marker->PortalId=Edge->Portal.InstanceId;
        Marker->Source=Edge->Source; Marker->Destination=Edge->Destination;
        Marker->SetActorLocation(FVector(100,0,0));
        TestTrue(TEXT("Production session dispatches travel"),R.Session->Execute(Q).Disposition==ELHCommandDisposition::Accepted);
        TestTrue(TEXT("Session blocked during load"),R.Session->IsBlocked());
        Travel.Travel().OnDestinationLoaded(Travel.Travel().GetToken(),true,Q.Destination,TEXT(""));
        TestFalse(TEXT("Session resumes after durable arrival"),R.Session->IsBlocked());
        FLHSaveStore Reload(Disk); FLHSaveSnapshot Loaded;
        TestTrue(TEXT("Reload using production catalog compatibility"),Reload.Load(S.Header.CharacterId,FLHWave2Session::Compatibility(),Loaded,SaveError));
        TestTrue(TEXT("Production reload destination entrance"),LHWorld::SameEntrance(Loaded.Character.ActiveEntrance,Q.Destination));
        TestTrue(TEXT("Authority capture after reload install"),R.Session->InstallTravel(Loaded));
        TestEqual(TEXT("No travel rewards"),Loaded.Character.Gold.Value,Initial.Character.Gold.Value);
        Marker->Destroy();
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
    for(int32 Swing=0;Swing<1000 && Enemy->IsAlive();++Swing)
    {
        TMap<FName,double> Empty; Combat->RestoreCooldownMap(Empty);
        FLHUseAbilityRequest Q; Q.Request.Value=FGuid::NewGuid(); Q.Request.Epoch=Before.Session.RequestEpoch;
        Q.Ability.Value=TEXT("Attack.Melee.Basic"); Q.Target=Enemy->GetEntityId(Before.World.RunId);
        if(R.Session->Execute(Q).Disposition!=ELHCommandDisposition::Accepted) return false;
    }
    return Enemy->IsCorpse();
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHStage1Errand,"Lighthaven.Integration.Wave4.Stage1A.ErrandLoop",LHWave2TestsPrivate::LHWave2TestsFlags)
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
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHStage1Melee,"Lighthaven.Integration.Wave4.LiveMeleeCommand",LHWave2TestsPrivate::LHWave2TestsFlags)
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
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHStage1Resources,"Lighthaven.Integration.Wave4.ResourceSyncAtBoundary",LHWave2TestsPrivate::LHWave2TestsFlags)
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
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHStage1Catalog,"Lighthaven.Integration.Wave4.CatalogCrossRefs",LHWave2TestsPrivate::LHWave2TestsFlags)
bool FLHStage1Catalog::RunTest(const FString&)
{
    TestEqual(TEXT("eleven enemies"),LHEnemyData::Catalog().Num(),11); TestEqual(TEXT("77 slots"),LHEncounterData::Catalog().Num(),77);
    for(const auto& E:LHEncounterData::Catalog()) TestNotNull(TEXT("slot resolves"),LHEnemyData::Find(E.Enemy));
    for(const auto& E:LHEnemyData::Catalog()) for(const auto& L:E.Reward.Loot) TestNotNull(TEXT("loot item resolves"),LHItemData::Find(L.Item));
    const auto P=LHWave2::PrototypeProfile(); TestEqual(TEXT("Bible XP table"),P.Rules.Progression.Thresholds.Num(),200);
    TestEqual(TEXT("starting HP Prototype"),P.InitialHealth.Value,30.0); TestEqual(TEXT("starting gold Prototype"),P.InitialGold.Value,int64(100));
    TestEqual(TEXT("six individual kit instances"),P.StarterItems.Num(),6); return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHStage1Death,"Lighthaven.Integration.Wave4.DeathRespawnSafe",LHWave2TestsPrivate::LHWave2TestsFlags)
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
namespace LHG4TestsPrivate
{
using namespace LHStage1IntegrationTestsPrivate;
FLHRequestId Request(FRuntime& R)
{ FLHRequestId Id; Id.Epoch=R.Session->Snapshot().Session.RequestEpoch; Id.Value=FGuid::NewGuid(); return Id; }
bool Floor(FRuntime& R,ALHCharacter* Pawn,int32 Index)
{
    auto S=R.Session->Snapshot(); S.Character.ActiveEntrance=LHWorld::Registry()[Index].Entrances[0].Id;
    if (!R.Session->InstallTravel(S)) return false;
    for (const auto& Slot:LHWorld::Registry()[Index].Spawns)
    {
        auto* M=R.World->SpawnActor<ALHSpawnMarker>(Slot.Anchor.GetLocation(),Slot.Anchor.Rotator());
        M->Area=LHWorld::Registry()[Index].Id; M->SpawnId=Slot.SpawnId; M->EnemyDefinitionId=Slot.Enemy;
    }
    if (Index==4)
    {
        const auto* Slot=LHWorld::Registry()[4].Spawns.FindByPredicate([](const auto& X){return X.Enemy.Value==TEXT("Enemy.Balork");});
        if(!Slot) return false;
        auto* Arena=R.World->SpawnActor<ATargetPoint>(Slot->Anchor.GetLocation(),FRotator::ZeroRotator); Arena->Tags.Add(TEXT("B4.BalorkArena"));
    }
    return R.Session->StartEncounters();
}
// Synthetic safety callback, real respawn law and settlement/loot/session commands.
// Does not establish physical nav safety or time-to-afford at normal attack cadence.
bool Earn(FRuntime& R,ALHCharacter* Pawn,int32 Count)
{
    if(!B1(R,Pawn)) return false; R.Flush();
    auto* Director=R.World->GetSubsystem<ULHEncounterDirector>();
    for(int32 I=0;I<Count;++I)
    {
        auto S=R.Session->Snapshot(); auto* A=S.World.Areas.FindByPredicate([](const auto& X){return X.Area.Content.Value==TEXT("Area.TempleB1");});
        if(!A) return false;
        auto* E=A->Encounters.FindByPredicate([](const auto& X){return X.Definition.Value==TEXT("Enemy.BrownRat") && X.State==ELHEncounterLifeState::Alive;});
        if(!E)
        {
            TArray<FLHSpawnLifeId> Lives;
            if(LHRewards::AdvanceRespawns(*A,S.World.RunId,120,[](const FGuid&){return true;},[](const FLHContentId& Id){return LHEnemyData::Find(Id)->Health;},Lives)!=ELHCommandReason::None) return false;
            if(!R.Session->InstallTravel(S)) return false; Director->Populate(*A);
            E=A->Encounters.FindByPredicate([](const auto& X){return X.Definition.Value==TEXT("Enemy.BrownRat") && X.State==ELHEncounterLifeState::Alive;});
        }
        if(!E) return false; const auto Life=E->Life;
        if(!Kill(R,Pawn,Director->FindByLife(Life))) return false; R.Flush();
        FLHTakeLootRequest Q; Q.Request=Request(R); Q.Container=LHRewards::CorpseContainerFor(S.World.RunId,Life); Q.Kind=ELHLootTransferKind::Gold; Q.Quantity=LHWave2::PrototypeInteger(3);
        if(R.Session->Execute(Q).Disposition!=ELHCommandDisposition::Accepted) return false; R.Flush();
    }
    auto S=R.Session->Snapshot(); S.Character.ActiveEntrance=LHWorld::Registry()[0].SafeFallback;
    return R.Session->InstallTravel(S);
}
FLHBuyItemRequest Buy(FRuntime& R,ALHInteractableMarker* N,const TCHAR* Offer)
{ FLHBuyItemRequest Q; Q.Request=Request(R); Q.Vendor=N->Materialize(R.Session->Snapshot().World.RunId); Q.Offer.Value=Offer; Q.Quantity=LHWave2::PrototypeInteger(1); return Q; }
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHG4Services,"Lighthaven.Integration.Wave4.ServicesThroughSession",LHWave2TestsPrivate::LHWave2TestsFlags)
bool FLHG4Services::RunTest(const FString&)
{
    using namespace LHG4TestsPrivate; auto Disk=MakeShared<FStorage>(); FRuntime R(Disk);
    if(!TestTrue(TEXT("create"),R.Create().Disposition==ELHCommandDisposition::Accepted)) return false;
    R.Flush(); R.Session->Bind(R.State); auto* Pawn=Avatar(R);
    if(!TestTrue(TEXT("normal kills earn costs/points"),Earn(R,Pawn,200))) return false;
    AddInfo(TEXT("Budget: 200 Brown Rats, 9000 XP and 600 looted gold + 100 starting; 129 bow/quiver + 15 Archery + 532 FireDart = 676 gold."));
    auto* Sig=Npc(R,TEXT("NPC.Sigfried"),Pawn); auto Bow=Buy(R,Sig,TEXT("Offer.Sigfried.Item.AshwoodFlatbow"));
    const auto BowResult=R.Session->Execute(Bow); AddInfo(FString::Printf(TEXT("Production buy reason=%d"),int32(BowResult.Reason)));
    if(!TestTrue(TEXT("buy bow"),BowResult.Disposition==ELHCommandDisposition::Accepted)) return false; R.Flush();
    TestTrue(TEXT("buy replays"),R.Session->Execute(Bow).bReplay);
    auto Arrows=Buy(R,Sig,TEXT("Offer.Sigfried.Item.WoodenArrows")); TestTrue(TEXT("buy unlimited quiver"),R.Session->Execute(Arrows).Disposition==ELHCommandDisposition::Accepted); R.Flush();
    auto* Trainer=Npc(R,TEXT("NPC.Ortanalas"),Pawn); FLHTrainSkillRequest Train; Train.Request=Request(R); Train.Trainer=Trainer->Materialize(R.Session->Snapshot().World.RunId); Train.Skill.Value=TEXT("Skill.Archery"); Train.Points=LHWave2::PrototypeInteger(1);
    TestTrue(TEXT("train"),R.Session->Execute(Train).Disposition==ELHCommandDisposition::Accepted); R.Flush(); TestTrue(TEXT("train replays"),R.Session->Execute(Train).bReplay);
    FLHAllocateAttributePointsRequest Allocate; Allocate.Request=Request(R); Allocate.Points.Strength=Allocate.Points.Endurance=Allocate.Points.Agility=Allocate.Points.Intelligence=Allocate.Points.Wisdom=LHWave2::PrototypeInteger(0); Allocate.Points.Intelligence.Value=5;
    TestTrue(TEXT("earned INT21"),R.Session->Execute(Allocate).Disposition==ELHCommandDisposition::Accepted); R.Flush();
    auto* Mage=Npc(R,TEXT("NPC.Iraltok"),Pawn); FLHLearnSpellRequest Learn; Learn.Request=Request(R); Learn.Trainer=Mage->Materialize(R.Session->Snapshot().World.RunId); Learn.Spell.Value=TEXT("Spell.FireDart");
    TestTrue(TEXT("learn first damage spell"),R.Session->Execute(Learn).Disposition==ELHCommandDisposition::Accepted); R.Flush(); TestTrue(TEXT("learn replays"),R.Session->Execute(Learn).bReplay);
    const auto S=R.Session->Snapshot(); FLHEquipItemRequest Unequip; Unequip.Request=Request(R); Unequip.bUnequip=true; Unequip.Item=S.Character.Inventory.FindByPredicate([](const auto& I){return I.Definition.Value==TEXT("Item.RustedDirk");})->Id; Unequip.Slot=ELHEquipmentSlot::MainHand;
    TestTrue(TEXT("unequip starter for sale"),R.Session->Execute(Unequip).Disposition==ELHCommandDisposition::Accepted); R.Flush();
    FLHSellItemRequest Sell; Sell.Request=Request(R); Sell.Vendor=Sig->Materialize(S.World.RunId); Sell.Item=S.Character.Inventory.FindByPredicate([](const auto& I){return I.Definition.Value==TEXT("Item.RustedDirk");})->Id; Sell.Quantity=LHWave2::PrototypeInteger(1);
    TestTrue(TEXT("sell"),R.Session->Execute(Sell).Disposition==ELHCommandDisposition::Accepted); R.Flush(); TestTrue(TEXT("sell replays"),R.Session->Execute(Sell).bReplay);
    if(!TestTrue(TEXT("populate B3"),Floor(R,Pawn,3))) return false; R.Flush();
    const auto Deep=R.Session->Snapshot(); const auto* Area=Deep.World.Areas.FindByPredicate([](const auto& X){return X.Area.Content.Value==TEXT("Area.TempleB3");});
    if(!Area || Area->Encounters.IsEmpty()) return false;
    auto* Enemy=R.World->GetSubsystem<ULHEncounterDirector>()->FindByLife(Area->Encounters[0].Life); if(!Enemy) return false;
    Pawn->SetActorLocation(Enemy->GetActorLocation()+FVector(100,0,0));
    const float Mana=R.State->GetCombatComponent()->GetCombatAttributes()->GetMana();
    FLHUseAbilityRequest Spell; Spell.Request=Request(R); Spell.Ability=Learn.Spell; Spell.Target=Enemy->GetEntityId(Deep.World.RunId);
    TestTrue(TEXT("earned FireDart routes to live combat"),R.Session->Execute(Spell).Disposition==ELHCommandDisposition::Accepted);
    TestEqual(TEXT("real mana cost"),R.State->GetCombatComponent()->GetCombatAttributes()->GetMana(),Mana-1.f);
    R.Session->bInGameplay=true;
    TestTrue(TEXT("exit captures cast boundary"),R.Session->RequestExit().IsEmpty()); R.Flush();
    FRuntime Reload(Disk); TestTrue(TEXT("reload earned purchases and spell"),Reload.Restore(S.Header.CharacterId));
    TestTrue(TEXT("same durable state"),Equal(R.Session->Snapshot(),Reload.Session->Snapshot())); return !HasAnyErrors();
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHG4Potion,"Lighthaven.Integration.Wave4.UseItemThroughSession",LHWave2TestsPrivate::LHWave2TestsFlags)
bool FLHG4Potion::RunTest(const FString&)
{
    using namespace LHG4TestsPrivate; auto Disk=MakeShared<FStorage>(); FRuntime R(Disk); R.Create(); R.Flush(); R.Session->Bind(R.State); auto* Pawn=Avatar(R);
    auto* Fali=Npc(R,TEXT("NPC.Fali"),Pawn); auto Purchase=Buy(R,Fali,TEXT("Offer.Fali.Item.PotionOfMana"));
    if(!TestTrue(TEXT("real potion purchase"),R.Session->Execute(Purchase).Disposition==ELHCommandDisposition::Accepted)) return false; R.Flush();
    R.State->GetCombatComponent()->SetNumericAttributeBase(ULHAttributeSet::GetManaAttribute(),0);
    auto S=R.Session->Snapshot(); FLHUseItemRequest Q; Q.Request=Request(R); Q.Item=S.Character.Inventory.FindByPredicate([](const auto& I){return I.Definition.Value==TEXT("Item.PotionOfMana");})->Id;
    TestTrue(TEXT("UseItem accepted"),R.Session->Execute(Q).Disposition==ELHCommandDisposition::Accepted); R.Flush();
    TestEqual(TEXT("live mana clamped"),R.State->GetCombatComponent()->GetCombatAttributes()->GetMana(),10.f);
    TestTrue(TEXT("potion replay"),R.Session->Execute(Q).bReplay); TestFalse(TEXT("one consumed"),R.Session->Snapshot().Character.Inventory.ContainsByPredicate([&](const auto& I){return I.Id.InstanceId==Q.Item.InstanceId;})); return !HasAnyErrors();
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHG4Balork,"Lighthaven.Integration.Wave4.BalorkSingleClaim",LHWave2TestsPrivate::LHWave2TestsFlags)
bool FLHG4Balork::RunTest(const FString&)
{
    using namespace LHG4TestsPrivate; auto Disk=MakeShared<FStorage>(); FLHSaveSnapshot Finished;
    {
        FRuntime R(Disk); R.Create(); R.Flush(); R.Session->Bind(R.State); auto* Pawn=Avatar(R);
        if(!TestTrue(TEXT("hydrate B4 encounters"),Floor(R,Pawn,4))) return false; R.Flush();
        auto* Director=R.World->GetSubsystem<ULHEncounterDirector>();
        for(int32 I=0;I<2;++I)
        {
            auto S=R.Session->Snapshot(); auto* A=S.World.Areas.FindByPredicate([](const auto& X){return X.Area.Content.Value==TEXT("Area.TempleB4");});
            if(!A) return false;
            auto* E=A->Encounters.FindByPredicate([](const auto& X){return X.Definition.Value==TEXT("Enemy.Balork");}); if(!E) return false;
            if(I)
            {
                TArray<FLHSpawnLifeId> Lives; auto Hp=[](const FLHContentId& Id){return LHEnemyData::Find(Id)->Health;};
                LHRewards::AdvanceRespawns(*A,S.World.RunId,899,[](const FGuid&){return true;},Hp,Lives);
                E=A->Encounters.FindByPredicate([](const auto& X){return X.Definition.Value==TEXT("Enemy.Balork");});
                TestTrue(TEXT("Balork still dead before 15:00"),E && E->State!=ELHEncounterLifeState::Alive);
                LHRewards::AdvanceRespawns(*A,S.World.RunId,1,[](const FGuid&){return true;},Hp,Lives);
                E=A->Encounters.FindByPredicate([](const auto& X){return X.Definition.Value==TEXT("Enemy.Balork");});
                if(!TestTrue(TEXT("Balork alive at 15:00"),E && E->State==ELHEncounterLifeState::Alive)) return false;
                Pawn->SetActorLocation(LHWorld::Registry()[4].Spawns.FindByPredicate([](const auto& X){return X.Enemy.Value==TEXT("Enemy.Balork");})->Anchor.GetLocation()+FVector(1500,0,90));
                R.Session->InstallTravel(S); Director->Populate(*A);
            }
            if(!TestTrue(TEXT("live Balork kill"),Kill(R,Pawn,Director->FindByLife(E->Life)))) return false; R.Flush();
            TestEqual(TEXT("single unique boss claim"),R.Session->Snapshot().World.ClaimedUniqueRewards.Num(),1);
        }
        Finished=R.Session->Snapshot();
    }
    FRuntime R(Disk); if(!TestTrue(TEXT("reload B4"),R.Restore(Finished.Header.CharacterId))) return false; R.Session->Bind(R.State); auto* Pawn=Avatar(R);
    auto S=R.Session->Snapshot(); S.Character.ActiveEntrance=LHWorld::Registry()[0].SafeFallback; R.Session->InstallTravel(S);
    TestFalse(TEXT("arrival does not complete"),S.World.Quests[0].bCompleted);
    auto* Kiran=Npc(R,TEXT("NPC.BrotherKiran"),Pawn); auto Q=Interact(R,Kiran,TEXT("Topic.BalorkReturn"));
    TestTrue(TEXT("return topic completes"),R.Session->Execute(Q).Disposition==ELHCommandDisposition::Accepted); R.Flush();
    TestTrue(TEXT("repeat replays"),R.Session->Execute(Q).bReplay); TestEqual(TEXT("still one claim"),R.Session->Snapshot().World.ClaimedUniqueRewards.Num(),1);
    TestTrue(TEXT("new return intent cannot complete twice"),R.Session->Execute(Interact(R,Kiran,TEXT("Topic.BalorkReturn"))).Disposition==ELHCommandDisposition::Rejected); return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHG4Economy,"Lighthaven.Integration.Wave4.ProgressionRouteAffordable",LHWave2TestsPrivate::LHWave2TestsFlags)
bool FLHG4Economy::RunTest(const FString&)
{
    using namespace LHG4TestsPrivate; auto Disk=MakeShared<FStorage>(); FRuntime R(Disk); R.Create(); R.Flush(); R.Session->Bind(R.State); auto* Pawn=Avatar(R);
    if(!TestTrue(TEXT("normal kill route"),Earn(R,Pawn,200))) return false;
    AddInfo(TEXT("Observed normal kill budget: 200 Brown Rat settlements/loot transfers. Shared route costs 676 gold, initial100 + loot600 = 700; 9000 XP awards growth/points."));
    TestEqual(TEXT("normal gold budget"),R.Session->Snapshot().Character.Gold.Value,int64(700));
    TestEqual(TEXT("normal XP budget"),R.Session->Snapshot().Character.ExperienceBalance.Value,int64(9000));
    auto* Sig=Npc(R,TEXT("NPC.Sigfried"),Pawn);
    for(const TCHAR* Offer:{TEXT("Offer.Sigfried.Item.AshwoodFlatbow"),TEXT("Offer.Sigfried.Item.WoodenArrows")})
    { const auto Result=R.Session->Execute(Buy(R,Sig,Offer)); AddInfo(FString::Printf(TEXT("Production buy reason=%d"),int32(Result.Reason))); if(!TestTrue(TEXT("earned purchase"),Result.Disposition==ELHCommandDisposition::Accepted)) return false; R.Flush(); }
    for(const auto Slot:{ELHEquipmentSlot::Quiver,ELHEquipmentSlot::MainHand})
    {
        const auto S=R.Session->Snapshot(); const FName Name=Slot==ELHEquipmentSlot::MainHand?TEXT("Item.AshwoodFlatbow"):TEXT("Item.WoodenArrows");
        FLHEquipItemRequest Q; Q.Request=Request(R); Q.Slot=Slot; Q.Item=S.Character.Inventory.FindByPredicate([&](const auto& I){return I.Definition.Value==Name;})->Id;
        TestTrue(TEXT("equip earned ranged gear"),R.Session->Execute(Q).Disposition==ELHCommandDisposition::Accepted); R.Flush();
    }
    if(!TestTrue(TEXT("populate B2"),Floor(R,Pawn,2))) return false; R.Flush();
    auto S=R.Session->Snapshot(); auto* Director=R.World->GetSubsystem<ULHEncounterDirector>(); const auto* A=S.World.Areas.FindByPredicate([](const auto& X){return X.Area.Content.Value==TEXT("Area.TempleB2");});
    if(!A || A->Encounters.IsEmpty()) return false;
    auto* E=Director->FindByLife(A->Encounters[0].Life); if(!E) return false; Pawn->SetActorLocation(E->GetActorLocation()+FVector(100,0,0));
    FLHUseAbilityRequest Attack; Attack.Request=Request(R); Attack.Ability.Value=TEXT("Attack.Ranged.Bow"); Attack.Target=E->GetEntityId(S.World.RunId);
    TestTrue(TEXT("earned bow routes to live combat"),R.Session->Execute(Attack).Disposition==ELHCommandDisposition::Accepted);
    TestTrue(TEXT("activation replay"),R.Session->Execute(Attack).bReplay); return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHG4EnemyContinuation,"Lighthaven.Integration.Wave4.EnemyContinuation",LHWave2TestsPrivate::LHWave2TestsFlags)
bool FLHG4EnemyContinuation::RunTest(const FString&)
{
    using namespace LHG4TestsPrivate; auto Disk=MakeShared<FStorage>(); FLHSaveSnapshot Captured; FLHSpawnLifeId Life; int32 ExpectedSeed=0;
    {
        FRuntime R(Disk); R.Create(); R.Flush(); R.Session->Bind(R.State); auto* Pawn=Avatar(R);
        if(!TestTrue(TEXT("hydrate B2"),Floor(R,Pawn,2))) return false; R.Flush();
        const auto S=R.Session->Snapshot(); const auto* Area=S.World.Areas.FindByPredicate([](const auto& A){return A.Area.Content.Value==TEXT("Area.TempleB2");});
        if(!Area || Area->Encounters.IsEmpty()) return false; Life=Area->Encounters[0].Life;
        auto* Enemy=R.World->GetSubsystem<ULHEncounterDirector>()->FindByLife(Life); if(!Enemy) return false;
        auto* Combat=Enemy->GetCombatComponent(); auto Random=Combat->GetCombatRandomState(); Random.FRand(); ExpectedSeed=Random.GetCurrentSeed(); Combat->SetCombatRandomState(Random);
        TMap<FName,double> Cooldowns; Cooldowns.Add(TEXT("Attack.Melee.Basic"),1.25); Combat->RestoreCooldownMap(Cooldowns);
        FString Detail; TestTrue(TEXT("capture all combat continuation"),R.Session->CaptureTravel(Captured,Detail));
        R.Session->bInGameplay=true; TestTrue(TEXT("save exit boundary"),R.Session->RequestExit().IsEmpty()); R.Flush();
    }
    FRuntime R(Disk); if(!TestTrue(TEXT("reload saved enemy continuation"),R.Restore(Captured.Header.CharacterId))) return false;
    auto* Pawn=Avatar(R); if(!TestTrue(TEXT("rehydrate same B2 lives"),Floor(R,Pawn,2))) return false;
    auto* Enemy=R.World->GetSubsystem<ULHEncounterDirector>()->FindByLife(Life); if(!Enemy) return false;
    TestEqual(TEXT("enemy RNG resumes exact state"),Enemy->GetCombatComponent()->GetCombatRandomState().GetCurrentSeed(),ExpectedSeed);
    FLHContentId Ability; Ability.Value=TEXT("Attack.Melee.Basic");
    TestEqual(TEXT("enemy cooldown resumes remainder"),Enemy->GetCombatComponent()->GetRemainingCooldown(Ability),1.25); return !HasAnyErrors();
}


namespace LHB1SpawnTestsPrivate
{
using namespace LHStage1IntegrationTestsPrivate;
void AdvanceWorldClock(FRuntime& R,double Seconds)
{
    // UWorld clamps long delta times (observed 2.1s -> 0.4s). Substep its
    // supported time-only seam and GAS timers together, without restoring any
    // cooldown state, forcing impacts or editing persisted simulation time.
    while(Seconds>0)
    {
        const float Step=float(FMath::Min(Seconds,0.1));
        ++GFrameCounter;
        R.World->Tick(LEVELTICK_TimeOnly,Step);
        R.World->GetTimerManager().Tick(Step);
        Seconds-=Step;
    }
}
bool LoadB1(FRuntime& R,int32 Index=1,bool Navigation=false)
{
    const FString Map=LHWorld::Registry()[Index].Map.GetLongPackageName();
    const FString InstanceName=TEXT("/Temp/B1SpawnTest_")+FGuid::NewGuid().ToString(EGuidFormats::Digits);
    FLinkerInstancingContext Instancing;
    Instancing.AddPackageMapping(FName(*Map),FName(*InstanceName));
    // Load a fresh editor-world instance from disk for navigation. Duplicating
    // an initialized world can copy its initialized flag without its subsystems.
    const FName LoadName(*(Navigation?InstanceName:Map));
    UWorld::WorldTypePreLoadMap.Add(LoadName,EWorldType::Editor);
    auto* Package=Navigation?LoadPackage(CreatePackage(*InstanceName),*Map,LOAD_None,nullptr,&Instancing):LoadPackage(nullptr,*Map,LOAD_None);
    UWorld::WorldTypePreLoadMap.Remove(LoadName);
    auto* Source=Package?UWorld::FindWorldInPackage(Package):nullptr;
    if (!Source) return false;
    R.State->ClearAvatar();
    auto* OwnedContext=R.Instance?GEngine->GetWorldContextFromWorld(R.World):nullptr;
    R.World->DestroyWorld(false); if(!OwnedContext) GEngine->DestroyWorldContext(R.World);
    if(Navigation) R.World=Source;
    else
    {
        auto* Destination=CreatePackage(*InstanceName);
        R.World=Cast<UWorld>(StaticDuplicateObject(Source,Destination,Source->GetFName()));
    }
    R.World->WorldType=EWorldType::Editor;
    auto& Context=OwnedContext?*OwnedContext:GEngine->CreateNewWorldContext(EWorldType::Editor); Context.WorldType=EWorldType::Editor; Context.SetCurrentWorld(R.World);
    if(R.Instance) { R.World->SetGameInstance(R.Instance); Context.OwningGameInstance=R.Instance; }
    if (!R.World->IsInitialized()) R.World->InitWorld(UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(true).CreateNavigation(Navigation).CreateAISystem(true));
    if (!R.World->GetPhysicsScene()) R.World->CreatePhysicsScene();
    R.World->UpdateWorldComponents(true,false);
    FStaticMeshCompilingManager::Get().FinishAllCompilation();
    for(TActorIterator<AActor> It(R.World);It;++It)
    {
        TInlineComponentArray<UPrimitiveComponent*> Components(*It);
        for(auto* C:Components) { C->UpdateComponentToWorld(); C->RecreatePhysicsState(); }
    }
    if(Navigation)
    {
        FAssetCompilingManager::Get().FinishAllCompilation();
        FNavigationSystem::AddNavigationSystemToWorld(*R.World,FNavigationSystemRunMode::EditorMode,nullptr,false);
        auto* Nav=R.World->GetNavigationSystem();
        auto* Wait=Nav?FindFProperty<FBoolProperty>(Nav->GetClass(),TEXT("bWaitForAsyncLoadingBeforeBuildingNavigationAutomatically")):nullptr;
        if(Wait) Wait->SetPropertyValue_InContainer(Nav,false);
        FNavigationSystem::AddNavigationSystemToWorld(*R.World,FNavigationSystemRunMode::EditorMode);
        FNavigationSystem::Build(*R.World);
        UE_LOG(LogTemp,Display,TEXT("G4 NAV world=%s system=%s data=%d tiles=%d"),*R.World->GetName(),*GetNameSafe(Nav),Nav && Nav->GetMainNavData()!=nullptr,Nav && Nav->ComputeNavDataBounds().IsValid);
    }
    R.Controller=R.World->SpawnActor<ALHPlayerController>(); R.Controller->SetAsLocalPlayerController(); R.Controller->InitInputSystem();
    R.State=R.World->SpawnActor<ALHPlayerState>(); R.Controller->SetPlayerState(R.State);
    if (!R.Session->Bind(R.State)) return false;
    auto* Pawn=Avatar(R); Pawn->SetActorLocation(FVector(-2500,600,90));
    const bool Frozen=R.Session->bWorldTravelFrozen;
    if(R.Instance) R.Session->FreezeWorldTravel(true);
    R.Controller->Possess(Pawn);
    if(R.Instance) { R.Local->PlayerController=R.Controller; R.Controller->Player=R.Local; R.Session->FreezeWorldTravel(Frozen); }
    R.World->GetTimerManager().Tick(0.f); return true;
}
void BaselineProbe(FAutomationTestBase& Test,FRuntime& R)
{
    const auto& Slot=LHWorld::Registry()[1].Spawns[0];
    const auto* Row=LHEnemyData::Find(Slot.Enemy);
    const FTransform At(Slot.Anchor.GetRotation(),Slot.Anchor.GetLocation()+FVector(0,0,Row->Runtime.CapsuleHalfHeightCm.Value));
    auto* Actor=R.World->SpawnActorDeferred<ALHEnemyCharacter>(ALHEnemyCharacter::StaticClass(),At,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding);
    Test.TestNull(TEXT("baseline deferred spawn rejected before runtime spec"),Actor);
    Test.AddInfo(FString::Printf(TEXT("BASELINE SpawnActorDeferred DontSpawnIfColliding marker=%s runtimeCenter=%s result=%s"),*Slot.SpawnId.ToString(),*At.GetLocation().ToString(),Actor?TEXT("actor"):TEXT("null")));
    if(Actor) Actor->Destroy();
}
bool Check(FAutomationTestBase& Test,FRuntime& R)
{
    const auto S=R.Session->Snapshot();
    const auto* Area=S.World.Areas.FindByPredicate([](const auto& A){return A.Area.Content.Value==TEXT("Area.TempleB1");});
    if(!Test.TestNotNull(TEXT("B1 checkpoint"),Area)) return false;
    Test.TestEqual(TEXT("17 encounters exactly once"),Area->Encounters.Num(),17);
    auto* D=R.World->GetSubsystem<ULHEncounterDirector>();
    FLHBasicAttackConfig Attack; FString AttackError; FLHContentId Ability; Ability.Value=TEXT("Attack.Melee.Basic");
    Test.TestTrue(TEXT("player attack config"),LHAbilities::BuildAttackConfig(S,Ability,[](const FLHContentId& Id)->const FLHCombatItemData* { const auto* Row=LHItemData::Find(Id); return Row?&Row->Combat:nullptr; },LHWave2::PrototypeProfile().Rules.Combat,Attack,AttackError));
    LH::Rules::FRequirementInput Requirements; const auto& B=S.Character.BaseAttributes;
    Requirements.Base={B.Strength.Value,B.Endurance.Value,B.Agility.Value,B.Intelligence.Value,B.Wisdom.Value}; Requirements.Effective=Requirements.Base; Requirements.Level=S.Character.EarnedLevel.Value; Requirements.Skills=S.Character.LearnedSkills; Requirements.Spells=S.Character.LearnedSpells;
    R.State->GetCombatComponent()->ConfigureAttack(Attack,Requirements);
    Test.TestNotNull(TEXT("loaded world director"),D); if(!D) return false;
    int32 Count=0; for(TActorIterator<ALHEnemyCharacter> It(R.World);It;++It) if(It->IsAlive()) ++Count;
    Test.TestEqual(TEXT("17 live actors"),Count,17);
    for(const auto& E:Area->Encounters)
    {
        auto* Actor=D->FindByLife(E.Life);
        if(!Test.TestNotNull(TEXT("registered life"),Actor)) continue;
        const auto* Slot=LHWorld::Registry()[1].Spawns.FindByPredicate([&](const auto& X){return X.SpawnId==E.Life.SpawnSlot;});
        Test.TestTrue(TEXT("XY anchor preserved"),Slot && FVector::Dist2D(Actor->GetActorLocation(),Slot->Anchor.GetLocation())<0.1);
        Test.TestTrue(TEXT("targetable alive capsule"),Actor->IsAlive() && Actor->GetCapsuleComponent()->GetCollisionResponseToChannel(ECC_Visibility)==ECR_Block);
        Test.TestEqual(TEXT("entity registered"),D->FindByEntity(Actor->GetEntityId(S.World.RunId)),Actor);
        R.State->GetCombatAvatar()->SetActorLocation(Actor->GetActorLocation()+FVector(100,0,0));
        Test.TestTrue(TEXT("controller can select enemy"),R.Controller->SelectTarget(Actor));
    }
    // Real floor control: the default 88cm half-height intersects at rat center.
    const auto& Slot=LHWorld::Registry()[1].Spawns[0];
    const auto* Spec=LHEnemyData::Find(Slot.Enemy);
    const FVector Center=Slot.Anchor.GetLocation()+FVector(0,0,Spec->Runtime.CapsuleHalfHeightCm.Value+2);
    FCollisionQueryParams Params; for(TActorIterator<ALHEnemyCharacter> It(R.World);It;++It) Params.AddIgnoredActor(*It);
    const auto* Default=GetDefault<ALHEnemyCharacter>()->GetCapsuleComponent();
    Test.TestTrue(TEXT("baseline default capsule intersects loaded floor"),R.World->OverlapBlockingTestByChannel(Center,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(Default->GetUnscaledCapsuleRadius(),Default->GetUnscaledCapsuleHalfHeight()),Params));
    Test.AddInfo(FString::Printf(TEXT("B1 floor defaultHH=%g runtimeHH=%g center=%s encounters=%d actors=%d"),Default->GetUnscaledCapsuleHalfHeight(),Spec->Runtime.CapsuleHalfHeightCm.Value,*Center.ToString(),Area->Encounters.Num(),Count));
    return true;
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHB1TravelSpawn,"Lighthaven.Integration.Wave4.B1SpawnOnTravelArrival",LHWave2TestsPrivate::LHWave2TestsFlags)
bool FLHB1TravelSpawn::RunTest(const FString&)
{
    using namespace LHB1SpawnTestsPrivate;
    auto Disk=MakeShared<FStorage>(); FRuntime R(Disk); R.Create(); R.Flush(); R.Session->Bind(R.State); Avatar(R);
    FLHTravelSaveAdapter::FHooks Hooks;
    Hooks.PrepareArrival=FLHWave2Session::PopulateEncounterCheckpoint;
    Hooks.Freeze=[&](bool F){R.Session->FreezeWorldTravel(F);};
    Hooks.Capture=[&](FLHSaveSnapshot& S,FString& E){return R.Session->CaptureTravel(S,E);};
    Hooks.Durable=[&](const FLHSaveSnapshot& S){TestTrue(TEXT("durable session install"),R.Session->InstallTravel(S));};
    Hooks.Load=[&](const FLHAreaDefinition&,uint64){TestTrue(TEXT("load real generated B1"),LoadB1(R));};
    Hooks.Install=[&](const FLHSaveSnapshot& S,const FLHEntranceDefinition&,FString&){ BaselineProbe(*this,R); return R.Session->InstallTravel(S) && R.Session->StartEncounters();};
    Hooks.Restore=[](const FLHSaveSnapshot&,const FLHEntranceDefinition&,uint64){};
    FLHTravelSaveAdapter Travel(R.Store.ToSharedRef(),FLHWave2Session::Compatibility(),MoveTemp(Hooks));
    const auto S=R.Session->Snapshot(); const auto& Edge=LHWorld::Registry()[0].Portals[0];
    FLHRequestTravelRequest Q; Q.Request.Epoch=S.Session.RequestEpoch; Q.Request.Value=FGuid::NewGuid(); Q.Portal=Edge.Portal; Q.Portal.RunId=S.World.RunId; Q.Destination=Edge.Destination;
    FString Error; TestTrue(TEXT("real travel coordinator begin"),Travel.Travel().Begin(Q,Error));
    const uint64 Token=Travel.Travel().GetToken(); Travel.Travel().OnDestinationLoaded(Token,true,Q.Destination,TEXT(""));
    Check(*this,R);
    FLHSaveSnapshot Saved; FLHSaveError SaveError; TestTrue(TEXT("load durable arrival"),R.Store->Load(S.Header.CharacterId,FLHWave2Session::Compatibility(),Saved,SaveError));
    const auto* A=Saved.World.Areas.FindByPredicate([](const auto& X){return X.Area.Content.Value==TEXT("Area.TempleB1");});
    TestTrue(TEXT("durable arrival contains 17 lives"),A && A->Encounters.Num()==17);
    Travel.Travel().OnDestinationLoaded(Token,true,Q.Destination,TEXT(""));
    TestTrue(TEXT("repeated population succeeds"),R.Session->StartEncounters()); Check(*this,R);
    return !HasAnyErrors();
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHB1ContinueSpawn,"Lighthaven.Integration.Wave4.B1SpawnOnContinue",LHWave2TestsPrivate::LHWave2TestsFlags)
bool FLHB1ContinueSpawn::RunTest(const FString&)
{
    using namespace LHB1SpawnTestsPrivate;
    auto Disk=MakeShared<FStorage>(); FRuntime R(Disk); R.Create(); R.Flush(); R.Session->Bind(R.State);
    auto S=R.Session->Snapshot(); S.Character.ActiveEntrance=LHWorld::Registry()[1].Entrances[0].Id; ++S.Header.TransactionSequence;
    FLHAreaRecord Empty; Empty.Area=LHWorld::Registry()[1].Id; S.World.Areas.Add(Empty);
    FLHSaveError Error; TestTrue(TEXT("save legacy empty B1"),R.Store->RequestSave(S,FLHWave2Session::Compatibility(),true,Error));
    R.Session->Travel=[&](){if(!TestTrue(TEXT("Continue loads real B1"),LoadB1(R))) return; BaselineProbe(*this,R); TestTrue(TEXT("Continue arrival starts encounters"),R.Session->StartEncounters());};
    const FString ContinueError=R.Session->Continue(S.Header.CharacterId,false);
    if (!TestTrue(*FString(TEXT("real Continue: ")+ContinueError),ContinueError.IsEmpty())) { R.Session->Travel=nullptr; return false; }
    Check(*this,R); R.Flush();
    FLHSaveSnapshot Durable; FLHSaveError DurableError;
    TestTrue(TEXT("load durable Continue population"),R.Store->Load(S.Header.CharacterId,FLHWave2Session::Compatibility(),Durable,DurableError));
    const auto* DurableArea=Durable.World.Areas.FindByPredicate([](const auto& A){return A.Area.Content.Value==TEXT("Area.TempleB1");});
    TestTrue(TEXT("Continue persists 17 lives"),DurableArea && DurableArea->Encounters.Num()==17);
    const auto Before=R.Session->Snapshot(); TestTrue(TEXT("repeat hydration"),R.Session->StartEncounters());
    TestEqual(TEXT("no duplicate population transaction"),R.Session->Snapshot().Header.TransactionSequence,Before.Header.TransactionSequence);
    Check(*this,R);
    auto* D=R.World->GetSubsystem<ULHEncounterDirector>(); auto Now=R.Session->Snapshot();
    auto* Area=Now.World.Areas.FindByPredicate([](const auto& A){return A.Area.Content.Value==TEXT("Area.TempleB1");});
    if (!Area || Area->Encounters.IsEmpty() || !D) { R.Session->Travel=nullptr; return false; }
    auto Record=Area->Encounters[0]; D->Despawn(Record.Life);
    for(TActorIterator<ALHSpawnMarker> It(R.World);It;++It) if(It->SpawnId==Record.Life.SpawnSlot) It->SetActorLocation(It->GetActorLocation()-FVector(0,0,10));
    AddExpectedError(TEXT("runtime capsule blocked"),EAutomationExpectedErrorFlags::Contains,1);
    D->Populate(*Area);
    TestNull(TEXT("blocked marker refused"),D->FindByLife(Record.Life));
    R.Session->Travel=nullptr;
    return !HasAnyErrors();
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHG4LightPersistence,"Lighthaven.Integration.Wave4.LightPersistsAcrossReloadAndTravel",LHWave2TestsPrivate::LHWave2TestsFlags)
bool FLHG4LightPersistence::RunTest(const FString&)
{
    using namespace LHG4TestsPrivate;
    auto Disk=MakeShared<FStorage>(); FLHCharacterId Character;
    {
        FRuntime R(Disk); R.Create(); R.Flush(); R.Session->Bind(R.State); Avatar(R);
        // Fixture isolates Light's session lifecycle; native service/economy tests
        // cover learning. No free spell is granted by production code.
        R.EarnAllocationPoints();
        FLHAllocateAttributePointsRequest Allocate; Allocate.Request=Request(R); Allocate.Points=LHWave2TestsPrivate::Points();
        Allocate.Points.Strength=LHWave2::PrototypeInteger(0); Allocate.Points.Intelligence=LHWave2::PrototypeInteger(2);
        if (!TestTrue(TEXT("allocate Light requirements"),R.Session->Execute(Allocate).Disposition==ELHCommandDisposition::Accepted)) return false;
        R.Flush(); R.Session->bInGameplay=true;
        auto S=R.Session->Snapshot();
        FLHContentId Spell; Spell.Value=TEXT("Spell.Light"); S.Character.LearnedSpells.Add(Spell);
        if (!TestTrue(TEXT("install learned fixture"),R.Session->InstallTravel(S))) return false;
        if (!TestTrue(TEXT("initialize gameplay RNG"),R.Session->StartEncounters())) return false;
        FLHUseAbilityRequest Q; Q.Request=Request(R); Q.Ability=Spell; Q.Target=R.Session->HudState().Player;
        if (!TestTrue(TEXT("cast through session"),R.Session->Execute(Q).Disposition==ELHCommandDisposition::Accepted)) return false;
        R.World->GetTimerManager().Tick(0.1f);
        auto* C=R.State->GetCombatComponent();
        TestEqual(TEXT("catalog duration applies at impact"),C->GetLightRemainingSeconds(),600.0);
        R.Session->TickGameplay(10); TestEqual(TEXT("one active tick"),C->GetLightRemainingSeconds(),590.0);
        R.Session->SetGameplayPaused(true); R.Session->TickGameplay(20);
        TestEqual(TEXT("menu pause preserves duration"),C->GetLightRemainingSeconds(),590.0);
        R.Session->SetGameplayPaused(false); R.Session->FreezeWorldTravel(true); R.Session->TickGameplay(20);
        TestEqual(TEXT("travel freeze preserves duration"),C->GetLightRemainingSeconds(),590.0);
        FLHSaveSnapshot Travel; FString Error;
        if (!TestTrue(TEXT("capture frozen travel"),R.Session->CaptureTravel(Travel,Error))) return false;
        TestEqual(TEXT("one durable Light"),Travel.Session.DurableEffects.Num(),1);
        Travel.Character.ActiveEntrance=LHWorld::Registry()[1].Entrances[0].Id;
        if (!TestTrue(TEXT("prepare destination checkpoint"),FLHWave2Session::PopulateEncounterCheckpoint(Travel,Error))) return false;
        TestTrue(TEXT("install destination"),R.Session->InstallTravel(Travel));
        TestTrue(TEXT("repeat restore"),R.Session->InstallTravel(R.Session->Snapshot()));
        TestEqual(TEXT("restore does not tick"),C->GetLightRemainingSeconds(),590.0);
        TestEqual(TEXT("destination identity remapped"),R.Session->Snapshot().Session.DurableEffects[0].Owner.Area.Content.Value,Travel.Character.ActiveEntrance.Area.Content.Value);
        R.Session->FreezeWorldTravel(false); R.Session->TickGameplay(5);
        TestEqual(TEXT("resume ticks once"),C->GetLightRemainingSeconds(),585.0);
        Character=Travel.Header.CharacterId;
        TestTrue(TEXT("queue exit save"),R.Session->RequestExit().IsEmpty()); R.Flush();
    }
    FRuntime Reload(Disk);
    if (!TestTrue(TEXT("Continue durable Light"),Reload.Restore(Character))) return false;
    Reload.Session->Bind(Reload.State); Avatar(Reload); Reload.Session->bInGameplay=true;
    auto* C=Reload.State->GetCombatComponent();
    TestEqual(TEXT("reload retains active remainder"),C->GetLightRemainingSeconds(),585.0);
    Reload.Session->TickGameplay(584); TestEqual(TEXT("no double elapsed time"),C->GetLightRemainingSeconds(),1.0);
    Reload.Session->TickGameplay(1); TestEqual(TEXT("expiry clears runtime"),C->GetLightRemainingSeconds(),0.0);
    TestTrue(TEXT("queue expired save"),Reload.Session->RequestExit().IsEmpty()); Reload.Flush();
    TestTrue(TEXT("expired effect removed from snapshot"),Reload.Session->Snapshot().Session.DurableEffects.IsEmpty());
    FLHSaveStore Read(Disk); FLHSaveSnapshot Expired; FLHSaveError Error;
    TestTrue(TEXT("expired save loads"),Read.Load(Character,FLHWave2Session::Compatibility(),Expired,Error));
    TestTrue(TEXT("expired effect removed from save"),Expired.Session.DurableEffects.IsEmpty());
    TestTrue(TEXT("capacity policy explicit"),LHWave2::CarryCapacityPolicy().bUnlimited);
    TestTrue(TEXT("capacity display explicit"),Reload.Session->DerivedSummary().Contains(TEXT("Capacity unlimited (deferred by owner decision)")));
    return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHReachHorizontal,"Lighthaven.Integration.Wave4.InteractReachHorizontal",LHWave2TestsPrivate::LHWave2TestsFlags)
bool FLHReachHorizontal::RunTest(const FString&)
{
    using namespace LHWave2TestsPrivate;
    FRuntime R(MakeShared<FStorage>()); R.Create(); R.Flush(); R.Session->Bind(R.State);
    auto* Avatar=R.World->SpawnActor<ALHCharacter>(); R.State->InitializeAvatar(Avatar);
    Avatar->SetActorLocation(FVector(0,0,92));
    auto* Npc=R.World->SpawnActor<ALHInteractableMarker>();
    Npc->Area=R.Session->Snapshot().Character.ActiveEntrance.Area; Npc->InstanceId=FGuid::NewGuid(); Npc->DefinitionId.Value=TEXT("NPC.BrotherKiran");
    auto* Portal=R.World->SpawnActor<ALHPortal>(); Portal->PortalId=FGuid::NewGuid(); Portal->Source=R.Session->Snapshot().Character.ActiveEntrance;
    int Calls=0; R.Session->RequestWorldTravel=[&](const auto&,FString&){++Calls; return true;};
    R.Controller->Possess(Avatar); R.Controller->LiveSession=R.Session; R.Controller->JournalPresenter=R.UI;
    for (double X:{241.,249.,250.,251.,260.})
    {
        Npc->SetActorLocation(FVector(X,0,0)); Portal->SetActorLocation(FVector(X,20,0));
        const auto Topics=R.Session->DialogueTopics(Npc->Materialize(R.Session->Snapshot().World.RunId));
        TestTrue(TEXT("NPC topic reach"),!Topics.IsEmpty() && Topics[0].bEnabled==(X<=250));
        auto Interact=LHStage1IntegrationTestsPrivate::Interact(R,Npc,TEXT("Topic.Church"));
        const auto NpcResult=R.Session->Execute(Interact);
        TestEqual(TEXT("NPC authoritative reach"),NpcResult.Reason,X<=250?ELHCommandReason::None:ELHCommandReason::OutOfRange);
        R.Flush();
        Portal->SetActorLocation(FVector(X,0,0));
        FLHRequestTravelRequest Q; Q.Portal=Portal->Materialize(R.Session->Snapshot().World.RunId);
        const int Before=Calls; auto Result=R.Session->Execute(Q);
        TestEqual(TEXT("portal delegated only in reach"),Calls-Before,X<=250?1:0);
        if (X>250) TestEqual(TEXT("portal out of range"),Result.Reason,ELHCommandReason::OutOfRange);
        R.Controller->ActiveContext=ELHInputContext::Gameplay;
        const int BeforeSelection=Calls; R.Controller->Interact();
        TestEqual(TEXT("controller portal selection agrees"),Calls-BeforeSelection,X<=250?1:0);
        Portal->SetActorLocation(FVector(1000,0,0));
        R.UI->Open(ELHUIScreen::Hud); R.Controller->ActiveContext=ELHInputContext::Gameplay;
        R.Controller->Interact();
        TestEqual(TEXT("controller NPC selection agrees"),R.UI->Screen(),X<=250?ELHUIScreen::Dialogue:ELHUIScreen::Hud);
    }
    Npc->SetActorLocation(FVector(100,0,243)); Portal->SetActorLocation(FVector(100,0,243));
    TestFalse(TEXT("NPC other floor refused"),R.Session->DialogueTopics(Npc->Materialize(R.Session->Snapshot().World.RunId))[0].bEnabled);
    FLHRequestTravelRequest Q; Q.Portal=Portal->Materialize(R.Session->Snapshot().World.RunId);
    TestEqual(TEXT("portal other floor refused"),R.Session->Execute(Q).Reason,ELHCommandReason::OutOfRange);
    R.Controller->LiveSession.Reset(); R.Controller->JournalPresenter.Reset();
    return !HasAnyErrors();
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHDialogueNpcName,"Lighthaven.Integration.Wave4.DialogueShowsNpcName",LHWave2TestsPrivate::LHWave2TestsFlags)
bool FLHDialogueNpcName::RunTest(const FString&)
{
    using namespace LHWave2TestsPrivate;
    FRuntime R(MakeShared<FStorage>()); R.Create(); R.Flush();
    auto* Npc=R.World->SpawnActor<ALHInteractableMarker>(); Npc->Area=R.Session->Snapshot().Character.ActiveEntrance.Area; Npc->InstanceId=FGuid::NewGuid();
    const auto Id=Npc->Materialize(R.Session->Snapshot().World.RunId);
    Npc->DefinitionId.Value=TEXT("NPC.Nevanis"); TestEqual(TEXT("live Nevanis"),R.Session->DialogueName(Id),FString(TEXT("Nevanis")));
    Npc->DefinitionId.Value=TEXT("NPC.BrotherKiran"); TestEqual(TEXT("spaced name"),R.Session->DialogueName(Id),FString(TEXT("Brother Kiran")));
    Npc->DefinitionId.Value=TEXT("NPC.Unknown"); TestEqual(TEXT("unknown fallback"),R.Session->DialogueName(Id),FString(TEXT("Unknown NPC")));
    return !HasAnyErrors();
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHStaleStatus,"Lighthaven.Integration.Wave4.StaleStatusClears",LHWave2TestsPrivate::LHWave2TestsFlags)
bool FLHStaleStatus::RunTest(const FString&)
{
    using namespace LHWave2TestsPrivate;
    auto Disk=MakeShared<FStorage>(); FRuntime R(Disk); R.Create();
    Disk->bFail=true; R.Flush(); const auto Failure=R.Session->Status();
    TestTrue(TEXT("save failure visible"),Failure.StartsWith(TEXT("Save failed:")));
    R.Session->CompleteGameplayArrival(); TestEqual(TEXT("arrival preserves durability failure"),R.Session->Status(),Failure);
    R.Session->FreezeWorldTravel(true);
    R.Session->RetryWorldTravel=[](FString& Error){Error=TEXT("travel retry refused"); return false;};
    R.Session->RetryPersistence(); TestEqual(TEXT("travel retry preserves save error"),R.Session->OwnerStatus(),Failure);
    R.Session->FreezeWorldTravel(false);
    Disk->bFail=false; R.Session->RetryPersistence(); TestTrue(TEXT("retry durability clears failure"),R.Session->Status().IsEmpty());
    R.Session->AbortGameplayArrival(TEXT("fixture refused")); TestFalse(TEXT("arrival failure visible"),R.Session->Status().IsEmpty());
    R.Session->CompleteGameplayArrival(); TestTrue(TEXT("successful arrival clears status"),R.Session->Status().IsEmpty());
    R.Session->AbortGameplayArrival(TEXT("retry fixture")); R.Session->FreezeWorldTravel(true);
    R.Session->RetryWorldTravel=[](FString&){return true;}; R.Session->RetryPersistence();
    R.Session->FreezeWorldTravel(false); TestTrue(TEXT("successful travel retry clears old feedback"),R.Session->Status().IsEmpty());
    return !HasAnyErrors();
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHG4LegalReplay,"Lighthaven.Integration.G4.LegalPurchaseReplayAndRetrySave",LHWave2TestsPrivate::LHWave2TestsFlags)
bool FLHG4LegalReplay::RunTest(const FString&)
{
    using namespace LHG4TestsPrivate;
    auto Disk=MakeShared<FStorage>(); FRuntime R(Disk);
    if(!TestTrue(TEXT("legal Bible creation"),R.Create().Disposition==ELHCommandDisposition::Accepted)) return false;
    R.Flush(); R.Session->Bind(R.State); auto* Pawn=Avatar(R);
    const auto Initial=R.Session->Snapshot();
    TestEqual(TEXT("Bible starting gold"),Initial.Character.Gold.Value,int64(100));
    TestEqual(TEXT("Bible starting HP"),Initial.Character.CurrentHealth.Value,30.0);
    TestEqual(TEXT("Bible starting MP"),Initial.Character.CurrentMana.Value,10.0);
    auto* Fali=Npc(R,TEXT("NPC.Fali"),Pawn);
    auto Q=Buy(R,Fali,TEXT("Offer.Fali.Item.PotionOfMana"));
    Disk->bFail=true;
    if(!TestTrue(TEXT("normal purchase accepted"),R.Session->Execute(Q).Disposition==ELHCommandDisposition::Accepted)) return false;
    R.Flush(); const auto Purchased=R.Session->Snapshot();
    TestTrue(TEXT("save failure visible"),R.Session->HasUnsavedChanges());
    const int32 Writes=Disk->Writes;
    TestTrue(TEXT("same request replays while dirty"),R.Session->Execute(Q).bReplay);
    TestTrue(TEXT("replay preserves full snapshot"),Equal(Purchased,R.Session->Snapshot()));
    TestEqual(TEXT("replay does not write"),Disk->Writes,Writes);
    Disk->bFail=false;
    auto Widget=ILHUIWidgetHarness::Create(*R.UI); Widget->Activate("RetrySave");
    TestFalse(TEXT("RetrySave clears dirty state"),R.Session->HasUnsavedChanges());
    FRuntime Reload(Disk);
    TestTrue(TEXT("independent reload"),Reload.Restore(Initial.Header.CharacterId));
    TestTrue(TEXT("purchase persisted once"),Equal(Purchased,Reload.Session->Snapshot()));
    return !HasAnyErrors();
}
// Travel-only prefix of the requested build routes. Does not claim earned combat completion.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHG4SessionRoute,"Lighthaven.Integration.G4.SessionFloorRouteAndReload",LHWave2TestsPrivate::LHWave2TestsFlags)
bool FLHG4SessionRoute::RunTest(const FString&)
{
    using namespace LHB1SpawnTestsPrivate;
    auto Disk=MakeShared<FStorage>(); FRuntime R(Disk);
    if(!TestTrue(TEXT("fresh legal creation"),R.Create().Disposition==ELHCommandDisposition::Accepted)) return false;
    R.Flush(); R.Session->Bind(R.State); Avatar(R);
    if(!TestTrue(TEXT("load generated hub"),LoadB1(R,0))) return false;
    FLHTravelSaveAdapter* Adapter=nullptr; bool OccupyArrival=false;
    FLHTravelSaveAdapter::FHooks Hooks;
    Hooks.PrepareArrival=FLHWave2Session::PopulateEncounterCheckpoint;
    Hooks.Freeze=[&](bool F){R.Session->FreezeWorldTravel(F);};
    Hooks.Capture=[&](FLHSaveSnapshot& S,FString& E){return R.Session->CaptureTravel(S,E);};
    Hooks.Durable=[&](const FLHSaveSnapshot& S){TestTrue(TEXT("durable install"),R.Session->InstallTravel(S));};
    Hooks.Load=[&](const FLHAreaDefinition& A,uint64){
        const int32 Index=LHWorld::Registry().IndexOfByPredicate([&](const auto& X){return LHWorld::SameArea(X.Id,A.Id);});
        TestTrue(TEXT("load generated destination"),Index>=0 && LoadB1(R,Index));
    };
    Hooks.Install=[&](const FLHSaveSnapshot& S,const FLHEntranceDefinition& E,FString& Error){
        auto* Pawn=Cast<ALHCharacter>(R.State->GetCombatAvatar());
        if(!Pawn) { Error=TEXT("No avatar"); return false; }
        const auto* C=Pawn->GetCapsuleComponent(); FCollisionQueryParams Params; Params.AddIgnoredActor(Pawn);
        const FVector At=E.SafeTransform.GetLocation()+FVector(0,0,C->GetScaledCapsuleHalfHeight()+2);
        if(OccupyArrival)
        {
            FActorSpawnParameters Spawn; Spawn.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            auto* Occupant=R.World->SpawnActor<ALHCharacter>(At,FRotator::ZeroRotator,Spawn);
            if(!Occupant) { Error=TEXT("Test occupant failed to spawn"); return false; }
            Occupant->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn,ECR_Block);
        }
        if(R.World->OverlapBlockingTestByChannel(At,E.SafeTransform.GetRotation(),ECC_Pawn,
            FCollisionShape::MakeCapsule(C->GetScaledCapsuleRadius(),C->GetScaledCapsuleHalfHeight()),Params))
        { Error=TEXT("Arrival capsule blocked"); AddInfo(Error); return false; }
        Pawn->SetActorLocation(At);
        return R.Session->InstallTravel(S) && R.Session->StartEncounters();
    };
    Hooks.Restore=[&](const FLHSaveSnapshot&,const FLHEntranceDefinition&,uint64 Token){Adapter->Travel().OnSourceRestored(Token,true,TEXT(""));};
    FLHTravelSaveAdapter Travel(R.Store.ToSharedRef(),FLHWave2Session::Compatibility(),MoveTemp(Hooks)); Adapter=&Travel;
    R.Session->RequestWorldTravel=[&](const FLHRequestTravelRequest& Q,FString& E){return Travel.Travel().Begin(Q,E);};
    const int32 Route[]={1,2,3,4,3,2,1,0};
    for(int32 Index:Route)
    {
        const auto Before=R.Session->Snapshot();
        const auto* Area=LHWorld::FindArea(Before.Character.ActiveEntrance.Area);
        const auto* Edge=Area->Portals.FindByPredicate([&](const auto& P){return LHWorld::SameArea(P.Destination.Area,LHWorld::Registry()[Index].Id);});
        if(!TestNotNull(TEXT("registered edge"),Edge)) return false;
        ALHPortal* Portal=nullptr;
        for(TActorIterator<ALHPortal> It(R.World);It;++It) if(It->Materialize(Before.World.RunId).InstanceId==Edge->Portal.InstanceId) Portal=*It;
        if(!TestNotNull(TEXT("authored portal"),Portal)) return false;
        R.State->GetCombatAvatar()->SetActorLocation(Portal->GetActorLocation()+FVector(0,0,90));
        FLHRequestTravelRequest Q; Q.Request.Epoch=Before.Session.RequestEpoch; Q.Request.Value=FGuid::NewGuid();
        Q.Portal=Portal->Materialize(Before.World.RunId); Q.Destination=Edge->Destination;
        // Enemy windups are real GAS actions with the catalog's one-second
        // impact timer. Player attacks currently have zero impact delay, so an
        // enemy action is the reachable pending-action travel boundary.
        if(Area->Spawns.Num()>0)
        {
            ALHEnemyCharacter* Enemy=nullptr;
            for(TActorIterator<ALHEnemyCharacter> It(R.World);It;++It) if(It->IsAlive()) { Enemy=*It; break; }
            if(!TestNotNull(TEXT("pending probe live enemy"),Enemy)) return false;
            auto* Pawn=R.State->GetCombatAvatar();
            Pawn->SetActorLocation(Enemy->GetActorLocation()+FVector(100,0,0));
            auto* Combat=Enemy->GetCombatComponent();
            AdvanceWorldClock(R,2.1);
            const auto WindupReason=Combat->RequestBasicAttack(R.State->GetCombatComponent());
            AddInfo(FString::Printf(TEXT("WINDUP area=%s enemy=%s reason=%d player=%s enemyAt=%s"),*Area->Id.Content.Value.ToString(),*Enemy->GetName(),int32(WindupReason),*Pawn->GetActorLocation().ToString(),*Enemy->GetActorLocation().ToString()));
            if(!TestTrue(TEXT("real enemy action accepted"),WindupReason==ELHCommandReason::None)) return false;
            TestTrue(TEXT("catalog windup is pending"),Combat->IsActionPending());
            Pawn->SetActorLocation(Portal->GetActorLocation()+FVector(0,0,90));
            const int32 PendingWrites=Disk->Writes;
            const uint64 PendingToken=Travel.Travel().GetToken();
            TestTrue(TEXT("floor change refused during enemy windup"),R.Session->Execute(Q).Disposition!=ELHCommandDisposition::Accepted);
            TestEqual(TEXT("pending refusal does not begin adapter"),Travel.Travel().GetToken(),PendingToken);
            TestEqual(TEXT("pending refusal does not save"),Disk->Writes,PendingWrites);
            // Supported world/timer clock: no cooldown-map restore or forced impact.
            AdvanceWorldClock(R,1.1);
            TestFalse(TEXT("timer completes enemy action"),Combat->IsActionPending());
            R.Flush();
            Q.Request.Value=FGuid::NewGuid();
        }
        if(!TestTrue(TEXT("session travel accepted"),R.Session->Execute(Q).Disposition==ELHCommandDisposition::Accepted)) return false;
        const auto Token=Travel.Travel().GetToken();
        Travel.Travel().OnDestinationLoaded(Token,true,Q.Destination,TEXT(""));
        TestFalse(TEXT("arrival unfrozen"),Travel.Travel().IsFrozen());
        TestTrue(TEXT("destination installed"),LHWorld::SameEntrance(R.Session->Snapshot().Character.ActiveEntrance,Q.Destination));
        const int32 Writes=Disk->Writes;
        Travel.Travel().OnDestinationLoaded(Token,true,Q.Destination,TEXT(""));
        TestEqual(TEXT("duplicate callback has no save"),Disk->Writes,Writes);
        FLHSaveStore Independent(Disk); FLHSaveSnapshot Loaded; FLHSaveError E;
        TestTrue(TEXT("independent durable reload on every floor"),Independent.Load(Before.Header.CharacterId,FLHWave2Session::Compatibility(),Loaded,E));
        TestTrue(TEXT("exact durable arrival"),Equal(Loaded,R.Session->Snapshot()));
        TestEqual(TEXT("travel grants no gold"),Loaded.Character.Gold.Value,Before.Character.Gold.Value);
        TestEqual(TEXT("travel grants no XP"),Loaded.Character.ExperienceBalance.Value,Before.Character.ExperienceBalance.Value);
        if(Index==4)
        {
            auto* D=R.World->GetSubsystem<ULHEncounterDirector>();
            const auto* Slot=LHWorld::Registry()[4].Spawns.FindByPredicate([](const auto& X){return X.Enemy.Value==TEXT("Enemy.Balork");});
            ALHEnemyCharacter* Boss=nullptr;
            for(TActorIterator<ALHEnemyCharacter> It(R.World);It;++It) if(Slot && It->GetLife().SpawnSlot==Slot->SpawnId) Boss=*It;
            if(!TestNotNull(TEXT("real B4 Balork"),Boss)) return false;
            auto* Pawn=R.State->GetCombatAvatar(); Pawn->SetActorLocation(Boss->GetActorLocation()+FVector(150,0,0));
            for(int32 Swing=0;Swing<2000 && Boss->IsAlive();++Swing)
            {
                AdvanceWorldClock(R,1.6);
                FLHUseAbilityRequest Attack; Attack.Request=LHG4TestsPrivate::Request(R);
                Attack.Ability.Value=TEXT("Attack.Melee.Basic"); Attack.Target=Boss->GetEntityId(R.Session->Snapshot().World.RunId);
                if(!TestTrue(TEXT("legal timed melee command"),R.Session->Execute(Attack).Disposition==ELHCommandDisposition::Accepted)) return false;
                R.Flush();
            }
            if(!TestTrue(TEXT("Balork settled via combat"),Boss->IsCorpse())) return false;
            R.Flush(); TestEqual(TEXT("one Balork claim"),R.Session->Snapshot().World.ClaimedUniqueRewards.Num(),1);
            auto BossRecord=[&]() { const auto Snapshot=R.Session->Snapshot();
                const auto* A=Snapshot.World.Areas.FindByPredicate([](const auto& X){return X.Area.Content.Value==TEXT("Area.TempleB4");});
                return *A->Encounters.FindByPredicate([](const auto& X){return X.Definition.Value==TEXT("Enemy.Balork");}); };
            TestEqual(TEXT("Bible 15-minute initial clock"),BossRecord().RespawnRemainingSeconds.Value,900.0);
            Pawn->SetActorLocation(LHWorld::Registry()[4].Entrances[0].SafeTransform.GetLocation()+FVector(0,0,92));
            // Same public session-clock seam the controller/pawn drives. Runtime
            // fixture has no GameInstance subsystem, so bind its gameplay flag.
            R.Session->bInGameplay=true;
            R.Session->SetGameplayPaused(true); R.Session->TickGameplay(900);
            TestEqual(TEXT("pause cannot advance boss clock"),BossRecord().RespawnRemainingSeconds.Value,900.0);
            R.Session->SetGameplayPaused(false); R.Session->FreezeWorldTravel(true); R.Session->TickGameplay(900);
            TestEqual(TEXT("travel freeze cannot advance boss clock"),BossRecord().RespawnRemainingSeconds.Value,900.0);
            R.Session->FreezeWorldTravel(false); R.Session->TickGameplay(899); R.Flush();
            TestEqual(TEXT("14:59 has one second remaining"),BossRecord().RespawnRemainingSeconds.Value,1.0);
            TestTrue(TEXT("14:59 still dead"),BossRecord().State==ELHEncounterLifeState::Dead);
            R.Session->TickGameplay(1); R.Flush();
            TestEqual(TEXT("15:00 reaches respawn boundary"),BossRecord().RespawnRemainingSeconds.Value,0.0);
            // LoadB1 intentionally has no navigation system. Never substitute a
            // synthetic true safety callback to fabricate a safe new generation.
            FString Safety; TestFalse(TEXT("real boss safety refuses absent navigation"),D->IsSpawnSafe(Slot->SpawnId,&Safety));
            AddInfo(TEXT("BALORK RESPAWN SAFETY ")+Safety);
            TestTrue(TEXT("unsafe deadline defers respawn"),BossRecord().State==ELHEncounterLifeState::RespawnPending);
            TestEqual(TEXT("deadline cannot duplicate claim"),R.Session->Snapshot().World.ClaimedUniqueRewards.Num(),1);
            FLHSaveStore AfterKill(Disk); FLHSaveSnapshot Reloaded; FLHSaveError ReloadError;
            TestTrue(TEXT("independent claimed boss reload"),AfterKill.Load(Before.Header.CharacterId,FLHWave2Session::Compatibility(),Reloaded,ReloadError));
            TestEqual(TEXT("claimed boss remains single after independent reload"),Reloaded.World.ClaimedUniqueRewards.Num(),1);
            // Ordinary timer ticks do not schedule a durable action themselves;
            // the next real floor travel captures the pending clock state. Its
            // exact independent arrival reload is asserted by the route loop.
        }
    }
    TestEqual(TEXT("return travel retains one boss claim"),R.Session->Snapshot().World.ClaimedUniqueRewards.Num(),1);
    ALHInteractableMarker* Kiran=nullptr;
    for(TActorIterator<ALHInteractableMarker> It(R.World);It;++It) if(It->DefinitionId.Value==TEXT("NPC.BrotherKiran")) Kiran=*It;
    if(!TestNotNull(TEXT("authored return NPC"),Kiran)) return false;
    R.State->GetCombatAvatar()->SetActorLocation(Kiran->GetActorLocation()+FVector(0,0,92));
    auto Return=LHG4TestsPrivate::Interact(R,Kiran,TEXT("Topic.BalorkReturn"));
    TestTrue(TEXT("real session return completes"),R.Session->Execute(Return).Disposition==ELHCommandDisposition::Accepted); R.Flush();
    TestTrue(TEXT("return request exact replay"),R.Session->Execute(Return).bReplay);
    TestTrue(TEXT("new return intent refuses second completion"),R.Session->Execute(LHG4TestsPrivate::Interact(R,Kiran,TEXT("Topic.BalorkReturn"))).Disposition==ELHCommandDisposition::Rejected);
    TestEqual(TEXT("completion remains single claim"),R.Session->Snapshot().World.ClaimedUniqueRewards.Num(),1);
    // Occupied destination: source save is allowed, arrival save is forbidden.
    const auto Source=R.Session->Snapshot(); const auto& Edge=LHWorld::Registry()[0].Portals[0];
    ALHPortal* Portal=nullptr;
    for(TActorIterator<ALHPortal> It(R.World);It;++It) if(It->Materialize(Source.World.RunId).InstanceId==Edge.Portal.InstanceId) Portal=*It;
    if(!TestNotNull(TEXT("blocked-case source portal"),Portal)) return false;
    R.State->GetCombatAvatar()->SetActorLocation(Portal->GetActorLocation()+FVector(0,0,90));
    FLHRequestTravelRequest Q; Q.Request.Epoch=Source.Session.RequestEpoch; Q.Request.Value=FGuid::NewGuid();
    Q.Portal=Portal->Materialize(Source.World.RunId); Q.Destination=Edge.Destination;
    OccupyArrival=true;
    if(!TestTrue(TEXT("occupied case begins through session"),R.Session->Execute(Q).Disposition==ELHCommandDisposition::Accepted)) return false;
    const int32 SourceWrites=Disk->Writes;
    Travel.Travel().OnDestinationLoaded(Travel.Travel().GetToken(),true,Q.Destination,TEXT(""));
    TestEqual(TEXT("occupied arrival has no save"),Disk->Writes,SourceWrites);
    TestEqual(TEXT("blocked reason retained"),Travel.Travel().GetError(),FString(TEXT("Arrival capsule blocked")));
    FLHSaveStore Independent(Disk); FLHSaveSnapshot Loaded; FLHSaveError SaveError;
    TestTrue(TEXT("blocked case source reload"),Independent.Load(Source.Header.CharacterId,FLHWave2Session::Compatibility(),Loaded,SaveError));
    TestTrue(TEXT("blocked case durable floor remains hub"),LHWorld::SameArea(Loaded.Character.ActiveEntrance.Area,Source.Character.ActiveEntrance.Area));
    R.Session->RequestWorldTravel=nullptr;
    return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHG4DeathChurch,"Lighthaven.Integration.G4.DeathAndChurchRespawn",LHWave2TestsPrivate::LHWave2TestsFlags)
bool FLHG4DeathChurch::RunTest(const FString&)
{
    using namespace LHB1SpawnTestsPrivate;
    auto Disk=MakeShared<FStorage>(); FRuntime R(Disk,true);
    if(!TestTrue(TEXT("fresh creation"),R.Create().Disposition==ELHCommandDisposition::Accepted)) return false;
    R.Flush(); R.Session->Bind(R.State); Avatar(R);
    if(!TestTrue(TEXT("generated hub"),LoadB1(R,0))) return false;
    FLHTravelSaveAdapter::FHooks Hooks;
    Hooks.Restore=[&](const FLHSaveSnapshot&,const FLHEntranceDefinition&,uint64){AddError(TEXT("unexpected source recovery"));};
    Hooks.PrepareArrival=FLHWave2Session::PopulateEncounterCheckpoint;
    Hooks.Freeze=[&](bool F){R.Session->FreezeWorldTravel(F);};
    Hooks.Capture=[&](FLHSaveSnapshot& S,FString& E){return R.Session->CaptureTravel(S,E);};
    Hooks.Durable=[&](const FLHSaveSnapshot& S){TestTrue(TEXT("durable travel"),R.Session->InstallTravel(S));};
    Hooks.Load=[&](const FLHAreaDefinition&,uint64){TestTrue(TEXT("generated B1"),LoadB1(R,1));};
    Hooks.Install=[&](const FLHSaveSnapshot& S,const FLHEntranceDefinition& E,FString& Error){
        auto* Pawn=R.State->GetCombatAvatar(); auto* Capsule=CastChecked<ALHCharacter>(Pawn)->GetCapsuleComponent();
        const FVector At=E.SafeTransform.GetLocation()+FVector(0,0,Capsule->GetScaledCapsuleHalfHeight()+2);
        FCollisionQueryParams Params; Params.AddIgnoredActor(Pawn);
        if(R.World->OverlapBlockingTestByChannel(At,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(Capsule->GetScaledCapsuleRadius(),Capsule->GetScaledCapsuleHalfHeight()),Params))
        { Error=TEXT("blocked arrival"); return false; }
        Pawn->SetActorLocation(At); return R.Session->InstallTravel(S) && R.Session->StartEncounters();
    };
    FLHTravelSaveAdapter Travel(R.Store.ToSharedRef(),FLHWave2Session::Compatibility(),MoveTemp(Hooks));
    R.Session->RequestWorldTravel=[&](const FLHRequestTravelRequest& Q,FString& E){return Travel.Travel().Begin(Q,E);};
    const auto S=R.Session->Snapshot(); const auto& Edge=LHWorld::Registry()[0].Portals[0];
    ALHPortal* Portal=nullptr;
    for(TActorIterator<ALHPortal> It(R.World);It;++It) if(It->Materialize(S.World.RunId).InstanceId==Edge.Portal.InstanceId) Portal=*It;
    if(!TestNotNull(TEXT("hub portal"),Portal)) return false;
    R.State->GetCombatAvatar()->SetActorLocation(Portal->GetActorLocation()+FVector(0,0,90));
    FLHRequestTravelRequest Q; Q.Request=LHG4TestsPrivate::Request(R); Q.Portal=Portal->Materialize(S.World.RunId); Q.Destination=Edge.Destination;
    if(!TestTrue(TEXT("session descent"),R.Session->Execute(Q).Disposition==ELHCommandDisposition::Accepted)) return false;
    R.Flush();
    Travel.Travel().OnDestinationLoaded(Travel.Travel().GetToken(),true,Q.Destination,TEXT(""));
    R.Flush();
    ALHEnemyCharacter* Enemy=nullptr; ALHEnemyCharacter* Second=nullptr;
    for(TActorIterator<ALHEnemyCharacter> It(R.World);It;++It) if(It->IsAlive()) { if(!Enemy) Enemy=*It; else { Second=*It; break; } }
    if(!TestNotNull(TEXT("real populated enemy"),Enemy)) return false;
    if(!TestNotNull(TEXT("second real encounter"),Second)) return false;
    const int64 BeforeDeathSequence=R.Session->Snapshot().Header.TransactionSequence;
    auto* Player=R.State->GetCombatComponent();
    int32 DeathPublications=0;
    const auto DeathHandle=Player->OnDeath.AddLambda([&](const FLHHitIdentity&){++DeathPublications;});
    // Positioning isolates two same-frame GAS windups; it grants no resources,
    // changes no combat spec and does not claim AI pathfinding evidence.
    Second->SetActorLocation(Enemy->GetActorLocation()+FVector(100,100,0));
    R.State->GetCombatAvatar()->SetActorLocation(Enemy->GetActorLocation()+FVector(100,0,0));
    for(int32 Attack=0;Attack<200 && Player->IsAlive();++Attack)
    {
        const auto Reason=Enemy->GetCombatComponent()->RequestBasicAttack(Player);
        AddInfo(FString::Printf(TEXT("DEATH attack=%d reason=%d hp=%g time=%g pending=%d"),Attack,int32(Reason),Player->GetCombatAttributes()->GetHealth(),R.World->GetTimeSeconds(),Enemy->GetCombatComponent()->IsActionPending()));
        if(!TestTrue(TEXT("catalog enemy attack"),Reason==ELHCommandReason::None)) { Player->OnDeath.Remove(DeathHandle); return false; }
        if(!TestTrue(TEXT("second same-frame attack"),Second->GetCombatComponent()->RequestBasicAttack(Player)==ELHCommandReason::None)) { Player->OnDeath.Remove(DeathHandle); return false; }
        AdvanceWorldClock(R,2.1);
        R.Flush();
    }
    Player->OnDeath.Remove(DeathHandle);
    if(!TestFalse(TEXT("earned lethal damage"),Player->IsAlive())) return false;
    TestEqual(TEXT("simultaneous attackers publish death once"),DeathPublications,1);
    R.Flush(); const auto Dead=R.Session->Snapshot();
    TestEqual(TEXT("durable dead HP"),Dead.Character.CurrentHealth.Value,0.0);
    TestEqual(TEXT("exactly one durable death boundary"),Dead.Header.TransactionSequence,BeforeDeathSequence+1);
    TestTrue(TEXT("HUD death state"),R.UI->Hud().bDead);
    FLHSaveSnapshot Persisted; FLHSaveError SaveError;
    TestTrue(TEXT("independent dead reload"),R.Store->Load(Dead.Header.CharacterId,FLHWave2Session::Compatibility(),Persisted,SaveError));
    TestTrue(TEXT("exact durable death"),Equal(Dead,Persisted));
    const int64 Sequence=Dead.Header.TransactionSequence;
    AdvanceWorldClock(R,2.1); R.Flush();
    TestEqual(TEXT("late lethal impacts cannot settle twice"),R.Session->Snapshot().Header.TransactionSequence,Sequence);
    const auto Respawn=R.UI->Respawn(); AddInfo(TEXT("RESPAWN ")+Respawn);
    if(!TestTrue(TEXT("church recovery command"),Respawn.IsEmpty())) return false;
    R.Flush();
    const auto Recovered=R.Session->Snapshot();
    TestTrue(TEXT("recovered HP"),Recovered.Character.CurrentHealth.Value>0);
    TestEqual(TEXT("inventory count intact"),Recovered.Character.Inventory.Num(),Dead.Character.Inventory.Num());
    for(int32 I=0;I<Dead.Character.Inventory.Num();++I)
    {
        const auto& Before=Dead.Character.Inventory[I]; const auto& After=Recovered.Character.Inventory[I];
        TestTrue(TEXT("inventory identity intact"),Before.Id.InstanceId==After.Id.InstanceId);
        TestEqual(TEXT("inventory quantity intact"),Before.Quantity.Value,After.Quantity.Value);
    }
    TestTrue(TEXT("church generated world"),LoadB1(R,0));
    FString PlacementError;
    TestTrue(TEXT("production reviewed church placement"),R.Instance->GetSubsystem<ULHSessionSubsystem>()->PlaceSessionArrival(R.World,PlacementError));
    AddInfo(TEXT("CHURCH ")+PlacementError);
    TestFalse(TEXT("HUD alive after church bind"),R.UI->Hud().bDead);
    const auto* Entrance=LHWorld::FindEntrance(Recovered.Character.ActiveEntrance);
    TestTrue(TEXT("church arrival exact"),Entrance && R.State->GetCombatAvatar()->GetActorLocation().Equals(
        Entrance->SafeTransform.GetLocation()+FVector(0,0,CastChecked<ALHCharacter>(R.State->GetCombatAvatar())->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()+2),0.1));
    TestTrue(TEXT("independent recovery reload"),R.Store->Load(Recovered.Header.CharacterId,FLHWave2Session::Compatibility(),Persisted,SaveError));
    TestTrue(TEXT("exact durable recovery"),Equal(Recovered,Persisted));
    R.Session->RequestWorldTravel=nullptr;
    return !HasAnyErrors();
}
namespace LHG4EarnedPrivate
{
using namespace LHB1SpawnTestsPrivate;
struct FRoute
{
    FAutomationTestBase& Test;
    TSharedRef<FStorage> Disk=MakeShared<FStorage>();
    FRuntime R{Disk};
    TUniquePtr<FLHTravelSaveAdapter> Adapter;
    int32 EnemyWindups=0;
    explicit FRoute(FAutomationTestBase& In):Test(In)
    {
        FLHTravelSaveAdapter::FHooks H;
        H.PrepareArrival=FLHWave2Session::PopulateEncounterCheckpoint;
        H.Freeze=[this](bool F){R.Session->FreezeWorldTravel(F);};
        H.Capture=[this](FLHSaveSnapshot& S,FString& E){return R.Session->CaptureTravel(S,E);};
        H.Durable=[this](const FLHSaveSnapshot& S){Test.TestTrue(TEXT("durable travel install"),R.Session->InstallTravel(S));};
        H.Load=[this](const FLHAreaDefinition& A,uint64){
            const int32 I=LHWorld::Registry().IndexOfByPredicate([&](const auto& X){return LHWorld::SameArea(X.Id,A.Id);});
            Test.TestTrue(TEXT("generated navigation world"),I>=0 && LoadB1(R,I,true));
        };
        H.Install=[this](const FLHSaveSnapshot& S,const FLHEntranceDefinition& E,FString& Error){
            auto* P=Cast<ALHCharacter>(R.State->GetCombatAvatar()); if(!P) return false;
            const auto* C=P->GetCapsuleComponent(); const FVector At=E.SafeTransform.GetLocation()+FVector(0,0,C->GetScaledCapsuleHalfHeight()+2);
            FCollisionQueryParams Params; Params.AddIgnoredActor(P);
            if(R.World->OverlapBlockingTestByChannel(At,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(C->GetScaledCapsuleRadius(),C->GetScaledCapsuleHalfHeight()),Params))
            { Error=TEXT("blocked generated arrival"); return false; }
            P->SetActorLocation(At);
            const bool Ok=R.Session->InstallTravel(S) && R.Session->StartEncounters();
            if(Ok) { R.Session->bInGameplay=true; R.Session->CompleteGameplayArrival(); } return Ok;
        };
        H.Restore=[this](const FLHSaveSnapshot&,const FLHEntranceDefinition&,uint64){Test.AddError(TEXT("unexpected travel recovery"));};
        Adapter=MakeUnique<FLHTravelSaveAdapter>(R.Store.ToSharedRef(),FLHWave2Session::Compatibility(),MoveTemp(H));
        R.Session->RequestWorldTravel=[this](const FLHRequestTravelRequest& Q,FString& E){return Adapter->Travel().Begin(Q,E);};
    }
    ~FRoute(){R.Session->RequestWorldTravel=nullptr;}
    bool Start()
    {
        if(!Test.TestTrue(TEXT("fresh legal build"),R.Create().Disposition==ELHCommandDisposition::Accepted)) return false;
        R.Flush(); R.Session->Bind(R.State); Avatar(R);
        if(!Test.TestTrue(TEXT("generated hub"),LoadB1(R,0,true))) return false;
        R.Session->bInGameplay=true; R.Session->CompleteGameplayArrival(); return Go(1);
    }
    bool Go(int32 Index)
    {
        const auto S=R.Session->Snapshot(); const auto* A=LHWorld::FindArea(S.Character.ActiveEntrance.Area);
        const auto* Edge=A?A->Portals.FindByPredicate([&](const auto& P){return LHWorld::SameArea(P.Destination.Area,LHWorld::Registry()[Index].Id);}):nullptr;
        if(!Test.TestNotNull(TEXT("legal route edge"),Edge)) return false;
        ALHPortal* Portal=nullptr;
        for(TActorIterator<ALHPortal> It(R.World);It;++It) if(It->Materialize(S.World.RunId).InstanceId==Edge->Portal.InstanceId) Portal=*It;
        if(!Test.TestNotNull(TEXT("authored portal"),Portal)) return false;
        R.State->GetCombatAvatar()->SetActorLocation(Portal->GetActorLocation()+FVector(0,0,92));
        Clock(2.1); R.Flush();
        FLHRequestTravelRequest Q; Q.Request=LHG4TestsPrivate::Request(R); Q.Portal=Portal->Materialize(S.World.RunId); Q.Destination=Edge->Destination;
        if(!Accepted(TEXT("session portal travel"),R.Session->Execute(Q))) return false;
        Adapter->Travel().OnDestinationLoaded(Adapter->Travel().GetToken(),true,Q.Destination,TEXT(""));
        if(!Test.TestTrue(TEXT("exact arrival"),LHWorld::SameEntrance(R.Session->Snapshot().Character.ActiveEntrance,Q.Destination))) return false;
        FLHSaveStore Independent(Disk); FLHSaveSnapshot Loaded; FLHSaveError E;
        return Test.TestTrue(TEXT("independent durable floor"),Independent.Load(S.Header.CharacterId,FLHWave2Session::Compatibility(),Loaded,E) && Equal(Loaded,R.Session->Snapshot()));
    }
    void Clock(double Seconds)
    {
        while(Seconds>0)
        {
            const float Step=float(FMath::Min(Seconds,0.1)); AdvanceWorldClock(R,Step);
            R.State->GetCombatComponent()->TickComponent(Step,LEVELTICK_All,nullptr);
            R.Session->TickGameplay(Step); Seconds-=Step;
        }
    }
    bool Accepted(const TCHAR* Label,const FLHCommandResult& Result)
    {
        if(Result.Disposition!=ELHCommandDisposition::Accepted)
            Test.AddInfo(FString::Printf(TEXT("%s reason=%d"),Label,int32(Result.Reason)));
        return Test.TestTrue(Label,Result.Disposition==ELHCommandDisposition::Accepted);
    }
    ALHInteractableMarker* Service(const TCHAR* Id)
    {
        for(TActorIterator<ALHInteractableMarker> It(R.World);It;++It) if(It->DefinitionId.Value==Id)
        { R.State->GetCombatAvatar()->SetActorLocation(It->GetActorLocation()+FVector(0,0,92)); return *It; }
        Test.AddError(FString(TEXT("missing authored service "))+Id); return nullptr;
    }
    bool Buy(const TCHAR* Offer)
    {
        auto* N=Service(TEXT("NPC.Sigfried")); if(!N) return false;
        const bool Ok=Accepted(TEXT("earned vendor purchase"),R.Session->Execute(LHG4TestsPrivate::Buy(R,N,Offer))); R.Flush(); return Ok;
    }
    bool Train(const TCHAR* Skill,int32 Points)
    {
        auto* N=Service(TEXT("NPC.Ortanalas")); if(!N) return false;
        FLHTrainSkillRequest Q; Q.Request=LHG4TestsPrivate::Request(R); Q.Trainer=N->Materialize(R.Session->Snapshot().World.RunId);
        Q.Skill.Value=Skill; Q.Points=LHWave2::PrototypeInteger(Points);
        const bool Ok=Accepted(TEXT("earned skill training"),R.Session->Execute(Q)); R.Flush(); return Ok;
    }
    bool Equip(const TCHAR* Item,ELHEquipmentSlot Slot)
    {
        const auto S=R.Session->Snapshot(); const auto* I=S.Character.Inventory.FindByPredicate([&](const auto& X){return X.Definition.Value==Item;});
        if(!Test.TestNotNull(TEXT("legally owned equipment"),I)) return false;
        FLHEquipItemRequest Q; Q.Request=LHG4TestsPrivate::Request(R); Q.Item=I->Id; Q.Slot=Slot;
        const bool Ok=Accepted(TEXT("earned equipment"),R.Session->Execute(Q)); R.Flush(); return Ok;
    }
    bool Fight(ALHEnemyCharacter* Enemy,const TCHAR* Ability)
    {
        if(!Test.TestNotNull(TEXT("real live encounter"),Enemy)) return false;
        auto* Pawn=R.State->GetCombatAvatar();
        for(int32 Swing=0;Swing<3000 && Enemy->IsAlive();++Swing)
        {
            // Spatial command fixture: retreat during the enemy's real windup,
            // then approach for the next command. No locomotion/path-feel claim.
            Pawn->SetActorLocation(LHWorld::FindEntrance(R.Session->Snapshot().Character.ActiveEntrance)->SafeTransform.GetLocation()+FVector(0,0,92)); Clock(1.6); R.Flush();
            if(!Test.TestTrue(TEXT("player survived opposed clock"),R.State->GetCombatComponent()->IsAlive())) return false;
            if(Ability==FString(TEXT("Spell.FireDart")) && R.State->GetCombatComponent()->GetCombatAttributes()->GetMana()<1)
            { Clock(120); R.Flush(); }
            Pawn->SetActorLocation(Enemy->GetActorLocation()+FVector(150,0,0));
            Clock(0.5); // Allow both acquire and attack decisions before the player command.
            if(Enemy->GetCombatComponent()->IsActionPending()) ++EnemyWindups;
            FLHUseAbilityRequest Q; Q.Request=LHG4TestsPrivate::Request(R); Q.Ability.Value=Ability; Q.Target=Enemy->GetEntityId(R.Session->Snapshot().World.RunId);
            const auto Result=R.Session->Execute(Q);
            if(Result.Disposition!=ELHCommandDisposition::Accepted)
            {
                Test.AddInfo(TEXT("F6 session status: ")+R.Session->Status());
                auto* Resolved=R.World->GetSubsystem<ULHEncounterDirector>()->FindByEntity(Q.Target);
                Test.AddInfo(FString::Printf(TEXT("TARGET requested=%s generation=%lld alive=%d resolved=%s generation=%lld alive=%d playerHP=%g"),
                    *Enemy->GetName(),Enemy->GetLife().LifeGeneration,Enemy->IsAlive(),*GetNameSafe(Resolved),Resolved?Resolved->GetLife().LifeGeneration:-1,
                    Resolved && Resolved->IsAlive(),R.State->GetCombatComponent()->GetCombatAttributes()->GetHealth()));
            }
            if(!Accepted(TEXT("legal earned combat command"),Result)) return false;
            R.Flush();
        }
        // Finish surviving windups before loot/save; retreat makes their real
        // range validation fail rather than cancelling or manually settling them.
        Pawn->SetActorLocation(LHWorld::FindEntrance(R.Session->Snapshot().Character.ActiveEntrance)->SafeTransform.GetLocation()+FVector(0,0,92)); Clock(2.1); R.Flush();
        return Test.TestTrue(TEXT("real combat corpse"),Enemy->IsCorpse());
    }
    bool Farm(int32 Kills)
    {
        auto* D=R.World->GetSubsystem<ULHEncounterDirector>();
        for(int32 I=0;I<Kills;++I)
        {
            ALHEnemyCharacter* Rat=nullptr;
            for(TActorIterator<ALHEnemyCharacter> It(R.World);It;++It)
                if(It->IsAlive() && It->GetRuntimeSpec()->ContentId.Value==TEXT("Enemy.BrownRat")) { Rat=*It; break; }
            if(!Rat)
            {
                R.State->GetCombatAvatar()->SetActorLocation(LHWorld::Registry()[1].Entrances[0].SafeTransform.GetLocation()+FVector(0,0,92));
                Clock(121); R.Flush();
                for(TActorIterator<ALHEnemyCharacter> It(R.World);It;++It)
                    if(It->IsAlive() && It->GetRuntimeSpec()->ContentId.Value==TEXT("Enemy.BrownRat")) { Rat=*It; break; }
            }
            if(!Rat)
                for(const auto& Slot:LHWorld::Registry()[1].Spawns) if(Slot.Enemy.Value==TEXT("Enemy.BrownRat"))
                { FString Why; D->IsSpawnSafe(Slot.SpawnId,&Why); Test.AddInfo(TEXT("RAT SAFETY ")+Why); }
            if(!Test.TestNotNull(TEXT("navigation-safe real respawn rat"),Rat)) return false;
            if(!Fight(Rat,TEXT("Attack.Melee.Basic"))) { Test.AddInfo(FString::Printf(TEXT("F6 farm completed=%d requested=%d"),I,Kills)); return false; }
            const auto S=R.Session->Snapshot();
            const auto* A=S.World.Areas.FindByPredicate([](const auto& X){return X.Area.Content.Value==TEXT("Area.TempleB1");});
            const auto* Corpse=A?A->Corpses.FindByPredicate([&](const auto& X){return LHAI::SameLife(X.SourceLife,Rat->GetLife());}):nullptr;
            if(!Test.TestNotNull(TEXT("earned corpse record"),Corpse)) return false;
            R.State->GetCombatAvatar()->SetActorLocation(Rat->GetActorLocation()+FVector(100,0,0));
            FLHTakeLootRequest Q; Q.Request=LHG4TestsPrivate::Request(R); Q.Container=Corpse->Container; Q.Kind=ELHLootTransferKind::Gold; Q.Quantity=Corpse->RemainingGold;
            if(Q.Quantity.Value>0 && !Accepted(TEXT("earned gold transfer"),R.Session->Execute(Q))) return false;
            R.Flush();
        }
        Test.AddInfo(FString::Printf(TEXT("EARNED kills=%d XP=%lld gold=%lld enemy-windups=%d"),Kills,R.Session->Snapshot().Character.ExperienceBalance.Value,R.Session->Snapshot().Character.Gold.Value,EnemyWindups));
        return Go(0);
    }
};
bool EarnedRoute(FAutomationTestBase& Test,int32 Build)
{
    FRoute F(Test); if(!F.Start() || !F.Farm(Build==2?200:60)) return false;
    const TCHAR* Ability=TEXT("Attack.Melee.Basic");
    if(Build==0) { if(!F.Train(TEXT("Skill.Attack"),10)) return false; }
    if(Build==1)
    {
        if(!F.Buy(TEXT("Offer.Sigfried.Item.AshwoodFlatbow")) || !F.Buy(TEXT("Offer.Sigfried.Item.WoodenArrows")) || !F.Train(TEXT("Skill.Archery"),1)
            || !F.Equip(TEXT("Item.WoodenArrows"),ELHEquipmentSlot::Quiver) || !F.Equip(TEXT("Item.AshwoodFlatbow"),ELHEquipmentSlot::MainHand)) return false;
        Ability=TEXT("Attack.Ranged.Bow");
    }
    if(Build==2)
    {
        FLHAllocateAttributePointsRequest Q; Q.Request=LHG4TestsPrivate::Request(F.R);
        Q.Points.Strength=Q.Points.Endurance=Q.Points.Agility=Q.Points.Intelligence=Q.Points.Wisdom=LHWave2::PrototypeInteger(0); Q.Points.Intelligence.Value=5;
        if(!F.Accepted(TEXT("earned INT allocation"),F.R.Session->Execute(Q))) return false; F.R.Flush();
        auto* N=F.Service(TEXT("NPC.Iraltok")); if(!N) return false;
        FLHLearnSpellRequest Learn; Learn.Request=LHG4TestsPrivate::Request(F.R); Learn.Trainer=N->Materialize(F.R.Session->Snapshot().World.RunId); Learn.Spell.Value=TEXT("Spell.FireDart");
        if(!F.Accepted(TEXT("earned first damage spell"),F.R.Session->Execute(Learn))) return false; F.R.Flush(); Ability=TEXT("Spell.FireDart");
    }
    for(int32 I:{1,2,3,4})
    {
        if(!F.Go(I)) return false;
        ALHEnemyCharacter* Enemy=nullptr;
        for(TActorIterator<ALHEnemyCharacter> It(F.R.World);It;++It)
            if(It->IsAlive() && (I!=4 || It->GetRuntimeSpec()->bBoss)) { Enemy=*It; break; }
        if(!F.Fight(Enemy,Ability)) return false;
        if(I==4 && Build==0)
        {
            const int64 Generation=Enemy->GetLife().LifeGeneration;
            F.R.State->GetCombatAvatar()->SetActorLocation(LHWorld::Registry()[4].Entrances[0].SafeTransform.GetLocation()+FVector(0,0,92));
            F.Clock(900); F.R.Flush();
            ALHEnemyCharacter* Respawned=nullptr;
            for(TActorIterator<ALHEnemyCharacter> It(F.R.World);It;++It)
                if(It->IsAlive() && It->GetRuntimeSpec()->bBoss) Respawned=*It;
            if(!Respawned)
            {
                const auto* Slot=LHWorld::Registry()[4].Spawns.FindByPredicate([](const auto& X){return X.Enemy.Value==TEXT("Enemy.Balork");});
                FString Why; if(Slot) F.R.World->GetSubsystem<ULHEncounterDirector>()->IsSpawnSafe(Slot->SpawnId,&Why);
                Test.AddInfo(TEXT("SECOND BALORK SAFETY ")+Why);
            }
            if(!Test.TestNotNull(TEXT("real navigation-safe Balork respawn"),Respawned)) return false;
            Test.TestEqual(TEXT("new boss generation"),Respawned->GetLife().LifeGeneration,Generation+1);
            if(!F.Fight(Respawned,Ability)) return false;
            Test.TestEqual(TEXT("second boss death cannot duplicate unique claim"),F.R.Session->Snapshot().World.ClaimedUniqueRewards.Num(),1);
        }
    }
    Test.TestEqual(TEXT("earned Balork claim"),F.R.Session->Snapshot().World.ClaimedUniqueRewards.Num(),1);
    for(int32 I:{3,2,1,0}) if(!F.Go(I)) return false;
    auto* N=F.Service(TEXT("NPC.BrotherKiran")); if(!N) return false;
    if(!F.Accepted(TEXT("earned return completion"),F.R.Session->Execute(LHG4TestsPrivate::Interact(F.R,N,TEXT("Topic.BalorkReturn"))))) return false;
    F.R.Flush(); Test.TestTrue(TEXT("opposed enemy windups observed"),F.EnemyWindups>0); return !Test.HasAnyErrors();
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHG4EarnedMelee,"Lighthaven.Integration.G4.EarnedMeleeRoute",LHWave2TestsPrivate::LHWave2TestsFlags)
bool FLHG4EarnedMelee::RunTest(const FString&){return LHG4EarnedPrivate::EarnedRoute(*this,0);}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHG4EarnedRanged,"Lighthaven.Integration.G4.EarnedRangedRoute",LHWave2TestsPrivate::LHWave2TestsFlags)
bool FLHG4EarnedRanged::RunTest(const FString&){return LHG4EarnedPrivate::EarnedRoute(*this,1);}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHG4EarnedMagic,"Lighthaven.Integration.G4.EarnedMagicRoute",LHWave2TestsPrivate::LHWave2TestsFlags)
bool FLHG4EarnedMagic::RunTest(const FString&){return LHG4EarnedPrivate::EarnedRoute(*this,2);}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHG4InventoryRollback,"Lighthaven.Integration.G4.SessionInventoryRollback",LHWave2TestsPrivate::LHWave2TestsFlags)
bool FLHG4InventoryRollback::RunTest(const FString&)
{
    using namespace LHG4EarnedPrivate;
    FRoute F(*this); if(!F.Start() || !F.Farm(400)) return false;
    const int32 Limit=int32(LHWave2::PrototypeProfile().InventorySlots.Value);
    while(F.R.Session->Snapshot().Character.Inventory.Num()<Limit)
        if(!F.Buy(TEXT("Offer.Sigfried.Item.AshwoodFlatbow"))) return false;
    TestEqual(TEXT("legal purchases fill inventory"),F.R.Session->Snapshot().Character.Inventory.Num(),Limit);
    if(!F.Go(1)) return false;
    for(int32 Attempt=0;Attempt<100;++Attempt)
    {
        ALHEnemyCharacter* Rat=nullptr;
        for(TActorIterator<ALHEnemyCharacter> It(F.R.World);It;++It)
            if(It->IsAlive() && It->GetRuntimeSpec()->ContentId.Value==TEXT("Enemy.BrownRat")) { Rat=*It; break; }
        if(!Rat)
        {
            F.R.State->GetCombatAvatar()->SetActorLocation(LHWorld::Registry()[1].Entrances[0].SafeTransform.GetLocation()+FVector(0,0,92)); F.Clock(121); F.R.Flush();
            continue;
        }
        if(!F.Fight(Rat,TEXT("Attack.Melee.Basic"))) return false;
        const auto S=F.R.Session->Snapshot();
        const auto* A=S.World.Areas.FindByPredicate([](const auto& X){return X.Area.Content.Value==TEXT("Area.TempleB1");});
        const auto* C=A?A->Corpses.FindByPredicate([&](const auto& X){return LHAI::SameLife(X.SourceLife,Rat->GetLife());}):nullptr;
        if(!C || C->RemainingItems.IsEmpty()) continue;
        F.R.State->GetCombatAvatar()->SetActorLocation(Rat->GetActorLocation()+FVector(100,0,0));
        FLHTakeLootRequest Q; Q.Request=LHG4TestsPrivate::Request(F.R); Q.Container=C->Container; Q.Kind=ELHLootTransferKind::Item;
        Q.Item=C->RemainingItems[0].Id; Q.Quantity=C->RemainingItems[0].Quantity;
        const int32 Writes=F.Disk->Writes;
        const auto Result=F.R.Session->Execute(Q);
        TestTrue(TEXT("session rejects full inventory"),Result.Disposition==ELHCommandDisposition::Rejected && Result.Reason==ELHCommandReason::InventoryFull);
        TestTrue(TEXT("exact live state rollback incl corpse and request journal"),Equal(S,F.R.Session->Snapshot()));
        F.R.Flush(); TestEqual(TEXT("rejection has no storage write"),F.Disk->Writes,Writes);
        FLHSaveStore Independent(F.Disk); FLHSaveSnapshot Loaded; FLHSaveError E;
        TestTrue(TEXT("independent durable rollback"),Independent.Load(S.Header.CharacterId,FLHWave2Session::Compatibility(),Loaded,E) && Equal(S,Loaded));
        return !HasAnyErrors();
    }
    AddError(TEXT("No catalog item drop in 100 actual encounter opportunities; no synthetic loot substitute.")); return false;
}
#endif
