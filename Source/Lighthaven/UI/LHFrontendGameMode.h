#pragma once
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "UI/LHUIPresenter.h"
#include "LHFrontendGameMode.generated.h"
class ULHInputConfig;
class SLHFrontendWidget;
UCLASS()
class LIGHTHAVEN_API ALHFrontendController : public APlayerController
{
    GENERATED_BODY()
public:
    // Integrator installs owner-bound adapters. No mock data or save mutation in this controller.
    void InstallPresenter(TSharedPtr<FLHUIPresenter> Presenter);
    void EstablishFrontendInput();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    virtual void Tick(float DeltaSeconds) override;
private:
    void ShowScreen();
    UPROPERTY() TObjectPtr<ULHInputConfig> InputConfig;
    TSharedPtr<FLHUIPresenter> Presenter;
    TSharedPtr<SLHFrontendWidget> Screen;
    TSharedPtr<class FLHWave2Session> LiveOwner;
    ELHUIScreen ContextScreen = ELHUIScreen::Settings;
};
UCLASS()
class LIGHTHAVEN_API ALHFrontendGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ALHFrontendGameMode();
};
