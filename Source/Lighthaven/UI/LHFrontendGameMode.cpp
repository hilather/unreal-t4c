#include "UI/LHFrontendGameMode.h"
#include "UI/LHFrontendWidget.h"
#include "Framework/LHSessionSubsystem.h"
#include "Framework/LHPlayerState.h"
#include "Input/LHInputConfig.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"

ALHFrontendGameMode::ALHFrontendGameMode()
{
    PlayerStateClass = ALHPlayerState::StaticClass();
    PlayerControllerClass = ALHFrontendController::StaticClass(); DefaultPawnClass = nullptr;
}
void ALHFrontendController::BeginPlay()
{
    Super::BeginPlay();
    if (!IsLocalController()) return;
    InputConfig = NewObject<ULHInputConfig>(this); InputConfig->Initialize();
    if (!Presenter)
    {
        LiveOwner=GetGameInstance()->GetSubsystem<ULHSessionSubsystem>()->Session();
        if (!LiveOwner || !LiveOwner->Bind(GetPlayerState<ALHPlayerState>())) return;
        Presenter = MakeShared<FLHUIPresenter>(*LiveOwner,*LiveOwner,*LiveOwner);
    }
    ShowScreen();
}
void ALHFrontendController::InstallPresenter(TSharedPtr<FLHUIPresenter> InPresenter)
{
    if (!InPresenter || (Presenter && Presenter->IsPending())) return;
    if (Screen && GetWorld() && GetWorld()->GetGameViewport()) GetWorld()->GetGameViewport()->RemoveViewportWidgetContent(Screen.ToSharedRef());
    Screen.Reset(); Presenter = MoveTemp(InPresenter); LiveOwner.Reset();
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
    if (LiveOwner) LiveOwner->Flush();
    if (Presenter) Presenter->Refresh();
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
    Screen.Reset(); Presenter.Reset(); LiveOwner.Reset(); Super::EndPlay(Reason);
}
