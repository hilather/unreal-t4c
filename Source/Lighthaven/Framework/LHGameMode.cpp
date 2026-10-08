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
