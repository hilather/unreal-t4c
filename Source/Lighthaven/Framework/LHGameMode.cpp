#include "Framework/LHGameMode.h"
#include "Framework/LHCharacter.h"
#include "Framework/LHPlayerController.h"
#include "Framework/LHPlayerState.h"
#include "Framework/LHGameState.h"
ALHGameMode::ALHGameMode()
{
    DefaultPawnClass = ALHCharacter::StaticClass();
    PlayerControllerClass = ALHPlayerController::StaticClass();
    PlayerStateClass = ALHPlayerState::StaticClass();
    GameStateClass = ALHGameState::StaticClass();
}

#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
#include "Framework/LHSessionSubsystem.h"
#include "Engine/GameInstance.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
void ALHGameMode::InitGame(const FString& MapName,const FString& Options,FString& ErrorMessage)
{
    Super::InitGame(MapName,Options,ErrorMessage);
    if(!FParse::Param(FCommandLine::Get(),TEXT("LHCaptureSession"))) return;
    FString Body; FParse::Value(FCommandLine::Get(),TEXT("LHCaptureAppearance="),Body);
    FString Error;
    auto* Session=GetGameInstance()->GetSubsystem<ULHSessionSubsystem>();
    if(!Session->PrepareCaptureSession(GetWorld(),Body==TEXT("B"),Error))
    { UE_LOG(LogTemp,Error,TEXT("LH CAPTURE SESSION FAIL %s"),*Error); ErrorMessage=Error; }
}
#endif
