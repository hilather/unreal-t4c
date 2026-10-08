#include "UI/LHFrontendGameMode.h"
#include "UI/LHFrontendWidget.h"
#include "Input/LHInputConfig.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"

// Explicit unavailable bootstrap, not an authority or playable fixture. Owned adapters replace it.
class FLHUnavailableUIOwner : public ILHCommandHandler, public ILHUIReadOwner, public ILHUISessionOwner
{
public:
#define REJECT(T) FLHCommandResult Execute(const T&) override { FLHCommandResult R; R.Reason = ELHCommandReason::UnresolvedRules; return R; }
    REJECT(FLHCreateCharacterRequest) REJECT(FLHAllocateAttributePointsRequest) REJECT(FLHTrainSkillRequest)
    REJECT(FLHLearnSpellRequest) REJECT(FLHBuyItemRequest) REJECT(FLHSellItemRequest) REJECT(FLHEquipItemRequest)
    REJECT(FLHUseAbilityRequest) REJECT(FLHInteractRequest) REJECT(FLHTakeLootRequest) REJECT(FLHRequestTravelRequest)
#undef REJECT
    FLHSaveSnapshot Snapshot() const override { return {}; }
    TArray<FLHUIProfile> Profiles() const override { return {}; }
    TArray<FLHContentId> AppearanceCatalog() const override { return {}; }
    FLHUICreationPreview Preview(const FString&,const TArray<FLHContentId>&,const TArray<FLHQuestionAnswer>&,bool) override { return {}; }
    FString Continue(FLHCharacterId,bool) override { return TEXT("Unavailable: local save owner has not been connected."); }
    FString RequestExit() override { FPlatformMisc::RequestExit(false); return {}; }
};
ALHFrontendGameMode::ALHFrontendGameMode()
{
    PlayerControllerClass = ALHFrontendController::StaticClass(); DefaultPawnClass = nullptr;
}
void ALHFrontendController::BeginPlay()
{
    Super::BeginPlay();
    if (!IsLocalController()) return;
    InputConfig = NewObject<ULHInputConfig>(this); InputConfig->Initialize();
    if (!Presenter)
    {
        UnavailableOwner = MakeShared<FLHUnavailableUIOwner>();
        Presenter = MakeShared<FLHUIPresenter>(*UnavailableOwner,*UnavailableOwner,*UnavailableOwner);
    }
    ShowScreen();
}
void ALHFrontendController::InstallPresenter(TSharedPtr<FLHUIPresenter> InPresenter)
{
    if (!InPresenter || (Presenter && Presenter->IsPending())) return;
    if (Screen && GetWorld() && GetWorld()->GetGameViewport()) GetWorld()->GetGameViewport()->RemoveViewportWidgetContent(Screen.ToSharedRef());
    Screen.Reset(); Presenter = MoveTemp(InPresenter); UnavailableOwner.Reset();
    if (HasActorBegunPlay()) ShowScreen();
}
void ALHFrontendController::ShowScreen()
{
    if (!GetWorld() || !GetWorld()->GetGameViewport() || !Presenter) return;
    SAssignNew(Screen,SLHFrontendWidget).Presenter(Presenter.Get());
    GetWorld()->GetGameViewport()->AddViewportWidgetContent(Screen.ToSharedRef(),100);
    bShowMouseCursor = true;
    FInputModeUIOnly Mode; Mode.SetWidgetToFocus(Screen); Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); SetInputMode(Mode);
    Screen->Open(ELHUIScreen::Frontend);
    ContextScreen = ELHUIScreen::Settings; Tick(0);
}
void ALHFrontendController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!Presenter || !InputConfig || !GetLocalPlayer() || ContextScreen == Presenter->Screen()) return;
    if (auto* S = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        S->RemoveMappingContext(InputConfig->Context(ELHInputContext::UI));
        S->RemoveMappingContext(InputConfig->Context(ELHInputContext::Creation));
        S->AddMappingContext(InputConfig->Context(Presenter->Screen() == ELHUIScreen::Creation ? ELHInputContext::Creation : ELHInputContext::UI),100);
    }
    ContextScreen = Presenter->Screen();
}
void ALHFrontendController::EndPlay(const EEndPlayReason::Type Reason)
{
    if (Screen && GetWorld() && GetWorld()->GetGameViewport()) GetWorld()->GetGameViewport()->RemoveViewportWidgetContent(Screen.ToSharedRef());
    if (InputConfig && GetLocalPlayer()) if (auto* S = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        S->RemoveMappingContext(InputConfig->Context(ELHInputContext::UI)); S->RemoveMappingContext(InputConfig->Context(ELHInputContext::Creation));
    }
    Screen.Reset(); Presenter.Reset(); UnavailableOwner.Reset(); Super::EndPlay(Reason);
}
