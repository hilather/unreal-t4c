#include "Misc/AutomationTest.h"
#include "UI/LHUIPresenter.h"
#include "UI/LHUIWidgetHarness.h"
#include "Input/LHInputConfig.h"
#include "Data/Items/LHItemCatalog.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "GameFramework/PlayerState.h"
#if WITH_DEV_AUTOMATION_TESTS
namespace LHGameplayUITestsPrivate
{
struct FGameplayOwners : ILHCommandHandler, ILHUIReadOwner, ILHUISessionOwner
{
    UWorld* World=nullptr; APlayerState* Pauser=nullptr;
    bool Paused=false; int32 Respawns=0;
    bool bHealing=false, bRefuseHealing=false;
    void SetGameplayPaused(bool V) override { Paused=V; if(World) World->GetWorldSettings()->SetPauserPlayerState(V?Pauser:nullptr); }
    FString RequestRespawn() override { ++Respawns; return {}; }
    FLHUIHud HudState() const override { FLHUIHud H; H.Objective=TEXT("Return to church"); H.SaveStatus=TEXT("Not saved"); H.bDead=true; FLHUIAbility A; A.Id.Value=TEXT("Attack.Melee.Basic"); A.Label=TEXT("Melee"); A.bAvailable=true; H.Abilities.Add(A); return H; }
    FString DialogueName(const FLHEntityId&) const override { return TEXT("Nevanis"); }
    TArray<FLHUIDialogueTopic> DialogueTopics(const FLHEntityId&) const override { FLHUIDialogueTopic T; T.Id.Value=bHealing?TEXT("Topic.Heal"):TEXT("Topic.Services"); T.Label=TEXT("Services"); T.Text=TEXT("Dialogue succeeded."); return {T}; }
    TArray<FLHUIServiceOffer> ServiceOffers(const FLHEntityId&) const override { FLHUIServiceOffer O; O.Kind=ELHUIOfferKind::Buy; O.Id.Value=TEXT("Offer.ManaPotion"); O.Quantity.Value=1; O.Quantity.Resolution=ELHValueResolution::Resolved; O.Price=TEXT("50 gold"); O.bEnabled=true; return {O}; }
    TArray<FLHUILootRow> CorpseContents(const FLHEntityId&) const override { FLHUILootRow L; L.Kind=ELHLootTransferKind::Item; L.Item.RunId=FGuid(1,2,3,4); L.Item.Area.Content.Value=TEXT("Area.TempleB1"); L.Item.InstanceId=FGuid(4,3,2,1); L.Quantity.Value=3; L.Quantity.Resolution=ELHValueResolution::Resolved; FLHUILootRow G=L; G.Kind=ELHLootTransferKind::Gold; G.Item={}; return {L,G}; }
    FLHSaveSnapshot State;
    FLHCreateCharacterRequest Last;
    int32 Creates = 0, Loads = 0, Allocates = 0, Equips = 0, Exits = 0;
    bool bReadable = true;
    bool bOwnsSaveStatus=false;
    FString SaveStatus;
    bool OwnsPersistenceStatus() const override { return bOwnsSaveStatus; }
    FString OwnerStatus() const override { return SaveStatus; }
    FString RetryPersistence() override { return SaveStatus; }
    int32 Resumes=0;
    bool ResumeGameplay() override { ++Resumes; return true; }
    FLHAttributeBlock LastAllocation;
    FLHEquipItemRequest LastEquipment;
    bool bAccept = false;
    bool bLegalAllocationReview=true;
    FGameplayOwners() { State.Session.RequestEpoch = FGuid::NewGuid(); }
    FLHCommandResult Execute(const FLHCreateCharacterRequest& R) override
    {
        Last = R; ++Creates; FLHCommandResult V; V.Request = R.Request;
        V.Disposition = bAccept ? ELHCommandDisposition::Accepted : ELHCommandDisposition::Rejected;
        V.Reason = bAccept ? ELHCommandReason::None : ELHCommandReason::InvalidRequest; return V;
    }
#define UNUSED_COMMAND(T) FLHCommandResult Execute(const T&) override { return {}; }
    FLHCommandResult Execute(const FLHAllocateAttributePointsRequest& R) override
    {
        ++Allocates; LastAllocation = R.Points; FLHCommandResult V;
        V.Disposition = bAccept ? ELHCommandDisposition::Accepted : ELHCommandDisposition::Rejected;
        V.Reason = bAccept ? ELHCommandReason::None : ELHCommandReason::InsufficientPoints; return V;
    }
    FLHUIIntentReview ReviewAllocation(const FLHAttributeBlock&) const override { FLHUIIntentReview R; R.bLegal = bLegalAllocationReview; R.Summary = bLegalAllocationReview ? TEXT("Fixture allocation review") : TEXT("Allocation rejected by character authority"); return R; }
    FLHUIIntentReview ReviewEquipment(const FLHEntityId&,ELHEquipmentSlot,bool) const override { FLHUIIntentReview R; R.bLegal = true; R.Summary = TEXT("Fixture equipment review"); return R; }
    UNUSED_COMMAND(FLHTrainSkillRequest)
    UNUSED_COMMAND(FLHLearnSpellRequest)
    FLHBuyItemRequest Buy;
    FLHCommandResult Execute(const FLHBuyItemRequest& R) override { Buy=R; return {}; }
    UNUSED_COMMAND(FLHSellItemRequest)
    FLHCommandResult Execute(const FLHEquipItemRequest& R) override
    {
        ++Equips; LastEquipment = R; FLHCommandResult V;
        V.Disposition = bAccept ? ELHCommandDisposition::Accepted : ELHCommandDisposition::Rejected;
        V.Reason = bAccept ? ELHCommandReason::None : ELHCommandReason::Ineligible; return V;
    }
    FLHUseAbilityRequest Attack;
    FLHCommandResult Execute(const FLHUseAbilityRequest& R) override { Attack=R; FLHCommandResult V; V.Request=R.Request; V.Disposition=ELHCommandDisposition::Accepted; V.Reason=ELHCommandReason::None; return V; }
    FLHUseItemRequest Item;
    FLHCommandResult Execute(const FLHUseItemRequest& R) override { Item=R; FLHCommandResult V; V.Request=R.Request; V.Reason=ELHCommandReason::NoEffect; return V; }
    FLHInteractRequest Topic;
    FLHCommandResult Execute(const FLHInteractRequest& R) override { Topic=R; FLHCommandResult V; V.Request=R.Request; V.Disposition=bRefuseHealing?ELHCommandDisposition::Rejected:ELHCommandDisposition::Accepted; V.Reason=bRefuseHealing?ELHCommandReason::NoEffect:ELHCommandReason::None; return V; }
    FLHTakeLootRequest Loot;
    FLHCommandResult Execute(const FLHTakeLootRequest& R) override { Loot=R; FLHCommandResult V; V.Request=R.Request; V.Reason=ELHCommandReason::InventoryFull; return V; }
    UNUSED_COMMAND(FLHRequestTravelRequest)
#undef UNUSED_COMMAND
    FLHSaveSnapshot Snapshot() const override { return State; }
    TArray<FLHUIProfile> Profiles() const override
    {
        FLHUIProfile P; P.Id = State.Header.CharacterId; P.Name = TEXT("Existing");
        P.bCanContinue = bReadable; P.bRequiresRecoveryAcknowledgment = true; return {P};
    }
    TArray<FLHContentId> AppearanceCatalog() const override { return {}; }
    FLHUICreationPreview Preview(const FString&, const TArray<FLHContentId>&,
        const TArray<FLHQuestionAnswer>& Answers, bool) override
    { FLHUICreationPreview P; P.Record.QuestionAnswers = Answers; P.Token = FGuid::NewGuid(); P.bLegal = true; return P; }
    FString Continue(FLHCharacterId, bool) override { ++Loads; return {}; }
    FString RequestExit() override { ++Exits; return TEXT("Storage unavailable"); }
};
FLHEntityId Entity() { FLHEntityId E; E.RunId=FGuid(1,2,3,4); E.Area.Content.Value=TEXT("Area.TempleB1"); E.InstanceId=FGuid(5,6,7,8); return E; }
}
#define LH_UI_TEST(C,N) IMPLEMENT_SIMPLE_AUTOMATION_TEST(C,"Lighthaven.UI." N,EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter) bool C::RunTest(const FString&)
LH_UI_TEST(FLHDialogueFeedbackPolishTest,"Polish.DialogueFeedback")
{
    using namespace LHGameplayUITestsPrivate; FGameplayOwners O; O.bHealing=true;
    FLHUIPresenter P(O,O,O); P.OpenTarget(ELHUIScreen::Dialogue,Entity());
    auto W=ILHUIWidgetHarness::Create(P);
    TestTrue(TEXT("dialogue heading shows owner NPC name"),W->SummaryText().StartsWith(TEXT("Nevanis\n")));
    W->Activate("Topic0");
    TestEqual(TEXT("success uses plain text"),W->MessageText(),FString(TEXT("Dialogue succeeded.")));
    TestFalse(TEXT("success uses information color"),P.HasError());
    O.bRefuseHealing=true; W->Activate("Topic0");
    TestEqual(TEXT("full HP refusal reason"),W->MessageText(),FString(TEXT("Error: You are already at full health.")));
    TestTrue(TEXT("refusal uses error color"),P.HasError());
    W->Open(ELHUIScreen::Hud); TestTrue(TEXT("closing dialogue clears feedback"),P.Error().IsEmpty());
    O.bRefuseHealing=false; P.OpenTarget(ELHUIScreen::Dialogue,Entity()); P.ActivateGameplay("Topic0");
    O.State.Character.ActiveEntrance.Area.Content.Value=TEXT("Area.TempleB2"); P.Refresh();
    TestTrue(TEXT("arrival refresh clears prior feedback"),P.Error().IsEmpty());
    O.SaveStatus=TEXT("Save failed: fixture"); P.Refresh();
    TestTrue(TEXT("owner failure is preserved"),P.Error().Contains(TEXT("Save failed")));
    return true;
}
LH_UI_TEST(FLHItemLabelsPolishTest,"Polish.ItemLabels")
{
    using namespace LHGameplayUITestsPrivate; FGameplayOwners O;
    for (const auto& Row:LHItemData::Catalog())
    {
        const auto Name=FLHUIPresenter::ItemName(Row.Id);
        TestFalse(TEXT("all catalog entries have display names"),Name==TEXT("Unknown item"));
        TestFalse(TEXT("display names do not expose content prefix"),Name.Contains(TEXT("Item.")));
    }
    FLHItemInstance Item; Item.Id=Entity(); Item.Definition.Value=TEXT("Item.RustedDirk"); O.State.Character.Inventory.Add(Item);
    FLHUIPresenter P(O,O,O);
    TestEqual(TEXT("equipment instance resolves via inventory and catalog"),P.EquippedItemName(Item.Id),FString(TEXT("Rusted Dirk")));
    TestEqual(TEXT("vendor and loot labels replace raw IDs"),P.DisplayItemNames(TEXT("Sell Item.PotionOfMana")),FString(TEXT("Sell Potion of Mana")));
    FLHContentId Unknown; Unknown.Value=TEXT("Item.Missing");
    TestEqual(TEXT("missing catalog entry has readable fallback"),P.ItemName(Unknown),FString(TEXT("Unknown item")));
    return true;
}
LH_UI_TEST(FLHHudSpellBarTest,"HudAndSpellBar")
{
    using namespace LHGameplayUITestsPrivate; FGameplayOwners O; FLHUIPresenter P(O,O,O); P.Open(ELHUIScreen::Hud);
    TestTrue(TEXT("objective and durability supplied by owner"),P.GameplaySummary().Contains(TEXT("Return to church")) && P.GameplaySummary().Contains(TEXT("Not saved")));
    TestTrue(TEXT("six focusable slots"),P.FocusOrder().Contains("Ability6")); P.SelectAbility(0); TestTrue(TEXT("selected catalog id"),P.SelectedAbility().Value==TEXT("Attack.Melee.Basic")); P.SelectAbility(5); TestTrue(TEXT("empty slot not fabricated"),P.SelectedAbility().Value.IsNone()); P.ActivateGameplay("Spell0"); P.ActivateGameplay("AssignAbility"); TestTrue(TEXT("available ability can fill any slot"),P.SelectedAbility().Value==TEXT("Attack.Melee.Basic")); return true;
}
LH_UI_TEST(FLHDialogueServicesPadTest,"DialogueAndServicesGamepad")
{
    using namespace LHGameplayUITestsPrivate; FGameplayOwners O; FLHUIPresenter P(O,O,O); P.OpenTarget(ELHUIScreen::Dialogue,Entity()); auto W=ILHUIWidgetHarness::Create(P); W->Open(ELHUIScreen::Dialogue); P.MoveFocus(-999); TestTrue(TEXT("topic first focus"),P.FocusedControl()=="Topic0"); W->Key(ELHUITestKey::South);
    TestTrue(TEXT("accepted service topic opens offers"),P.Screen()==ELHUIScreen::Services); TestTrue(TEXT("price from owner"),P.GameplayLabel("Offer0").Contains(TEXT("50 gold"))); W->Activate("Offer0"); TestTrue(TEXT("vendor identity preserved"),O.Buy.Vendor.InstanceId==Entity().InstanceId); TestTrue(TEXT("offer id preserved"),O.Buy.Offer.Value==TEXT("Offer.ManaPotion")); return true;
}
LH_UI_TEST(FLHDeathScreenTest,"DeathRespawnScreen")
{
    using namespace LHGameplayUITestsPrivate; FGameplayOwners O; FLHUIPresenter P(O,O,O); P.Open(ELHUIScreen::Death); TestTrue(TEXT("respawn reachable"),P.FocusOrder().Contains("Respawn")); P.ActivateGameplay("Respawn"); TestEqual(TEXT("owner requested once"),O.Respawns,1); return true;
}
LH_UI_TEST(FLHLootPanelTest,"LootPanelFullInventory")
{
    using namespace LHGameplayUITestsPrivate; FGameplayOwners O; FLHUIPresenter P(O,O,O); P.OpenTarget(ELHUIScreen::Loot,Entity()); P.ActivateGameplay("Loot0"); TestTrue(TEXT("item kind"),O.Loot.Kind==ELHLootTransferKind::Item); TestTrue(TEXT("container identity"),O.Loot.Container.InstanceId==Entity().InstanceId); TestTrue(TEXT("item has stable instance"),O.Loot.Item.InstanceId.IsValid()); TestEqual(TEXT("quantity from owner"),O.Loot.Quantity.Value,int64(3)); TestTrue(TEXT("full inventory explained"),P.Error().Contains(TEXT("Inventory full"))); auto ID=O.Loot.Request.Value; P.RetryCommand(); TestTrue(TEXT("retry retains request"),O.Loot.Request.Value==ID); P.ActivateGameplay("Loot1"); TestTrue(TEXT("explicit gold kind"),O.Loot.Kind==ELHLootTransferKind::Gold); TestTrue(TEXT("gold item unset"),!O.Loot.Item.InstanceId.IsValid()); return true;
}
LH_UI_TEST(FLHInventoryUseTest,"InventoryUseItem")
{
    using namespace LHGameplayUITestsPrivate; FGameplayOwners O; FLHUIPresenter P(O,O,O); P.UseItem(Entity()); const auto ID=O.Item.Request.Value; TestTrue(TEXT("epoch from owner"),O.Item.Request.Epoch==O.State.Session.RequestEpoch); TestTrue(TEXT("self target unset"),!O.Item.Target.InstanceId.IsValid()); P.RetryCommand(); TestTrue(TEXT("retry same ID"),O.Item.Request.Value==ID); P.AssignHotbarItem(Entity()); P.UseHotbarItem(); TestTrue(TEXT("fresh item intent fresh ID"),O.Item.Request.Value!=ID); TestTrue(TEXT("rejection shown"),P.Error().Contains(TEXT("Already full"))); return true;
}
LH_UI_TEST(FLHPauseOwnerTest,"PauseStopsSimulation")
{
    using namespace LHGameplayUITestsPrivate; FGameplayOwners O;
    auto Init=UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(false).CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false).SetTransactional(false);
    auto& Context=GEngine->CreateNewWorldContext(EWorldType::Game);
    O.World=UWorld::CreateWorld(EWorldType::Game,false,MakeUniqueObjectName(nullptr,UWorld::StaticClass(),TEXT("LHUIWorld")),GetTransientPackage(),true,ERHIFeatureLevel::Num,&Init);
    Context.SetCurrentWorld(O.World); O.World->InitializeActorsForPlay(FURL()); O.Pauser=O.World->SpawnActor<APlayerState>();
    FLHUIPresenter P(O,O,O); O.World->Tick(LEVELTICK_All,.1f); const auto Before=O.World->GetTimeSeconds();
    P.SetPaused(true); TestTrue(TEXT("actual world paused"),O.World->IsPaused()); O.World->Tick(LEVELTICK_All,.1f); TestEqual(TEXT("simulation clock stopped"),O.World->GetTimeSeconds(),Before);
    P.Open(ELHUIScreen::Pause); P.ActivateGameplay("Resume"); TestFalse(TEXT("resume clears world pause"),O.World->IsPaused()); O.World->Tick(LEVELTICK_All,.1f); TestTrue(TEXT("simulation advances again"),O.World->GetTimeSeconds()>Before); TestEqual(TEXT("input resume callback"),O.Resumes,1);
    O.World->DestroyWorld(false); GEngine->DestroyWorldContext(O.World); O.World=nullptr; return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHHotbarParityTest,"Lighthaven.Controls.HotbarParity",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHHotbarParityTest::RunTest(const FString&)
{
    auto* C=NewObject<ULHInputConfig>(); C->Initialize(); for(int32 I=1; I<=6; ++I) TestTrue(TEXT("keyboard and pad slot mapping"),C->HasDeviceParity(FName(*FString::Printf(TEXT("Hotbar%d"),I)))); TestTrue(TEXT("item parity"),C->HasDeviceParity("HotbarItem")); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLHLiveAttackTest,"Lighthaven.Controls.LiveAttackSubmitsUseAbility",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLHLiveAttackTest::RunTest(const FString&)
{
    using namespace LHGameplayUITestsPrivate; FGameplayOwners O; FLHUIPresenter P(O,O,O); const auto Target=Entity(); TestTrue(TEXT("command accepted"),P.UseAbility(Target).Disposition==ELHCommandDisposition::Accepted); TestTrue(TEXT("stable enemy id"),O.Attack.Target.InstanceId==Target.InstanceId); TestTrue(TEXT("selected ability"),O.Attack.Ability.Value==TEXT("Attack.Melee.Basic")); const auto ID=O.Attack.Request.Value; P.RetryCommand(); TestTrue(TEXT("attack retry ID"),O.Attack.Request.Value==ID); return true;
}
LH_UI_TEST(FLHSettingsNavigationTest,"Presentation.SettingsKeyboardAndPad")
{
    using namespace LHGameplayUITestsPrivate; FGameplayOwners O; FLHUIPresenter P(O,O,O);
    auto W=ILHUIWidgetHarness::Create(P); W->Open(ELHUIScreen::Settings);
    TestEqual(TEXT("settings initial focus"),P.FocusedControl(),FName("Master"));
    for(FName Id:{"Master","Effects","UI","TextScale","HighContrast","Apply","Revert","Back"})
        TestTrue(TEXT("setting reachable in logical focus order"),P.FocusOrder().Contains(Id));
    W->Key(ELHUITestKey::KeyboardDown); TestEqual(TEXT("keyboard advances"),P.FocusedControl(),FName("Effects"));
    W->Key(ELHUITestKey::Down); TestEqual(TEXT("pad advances"),P.FocusedControl(),FName("UI"));
    return true;
}
#endif
