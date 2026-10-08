#include "Framework/LHGameMode.h"
#include "Framework/LHPlayerState.h"
#include "Framework/LHGameState.h"
ALHGameMode::ALHGameMode()
{
    PlayerStateClass = ALHPlayerState::StaticClass();
    GameStateClass = ALHGameState::StaticClass();
}
