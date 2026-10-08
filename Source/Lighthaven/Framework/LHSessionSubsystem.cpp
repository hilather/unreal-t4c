#include "Framework/LHSessionSubsystem.h"
#include "Persistence/LHSaveSubsystem.h"
#include "Kismet/GameplayStatics.h"
void ULHSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection); Collection.InitializeDependency<ULHSaveSubsystem>();
    auto* Saves=GetGameInstance()->GetSubsystem<ULHSaveSubsystem>();
    Saves->Configure(FLHWave2Session::Compatibility()); Live=MakeShared<FLHWave2Session>(Saves->GetStore().ToSharedRef());
    Live->Travel=[this]() { UGameplayStatics::OpenLevel(GetGameInstance(),TEXT("/Game/Lighthaven/Maps/Dev_Movement")); };
    Live->Exit=[]() { FPlatformMisc::RequestExit(false); };
}
void ULHSessionSubsystem::Deinitialize()
{
    if (Live) { Live->Travel=nullptr; Live->Exit=nullptr; } Live.Reset(); Super::Deinitialize();
}
