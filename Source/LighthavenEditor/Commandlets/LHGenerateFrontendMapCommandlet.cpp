#include "LHGenerateFrontendMapCommandlet.h"
#include "UI/LHFrontendGameMode.h"
#include "Editor.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "FileHelpers.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
ULHGenerateFrontendMapCommandlet::ULHGenerateFrontendMapCommandlet()
{ IsClient = false; IsServer = false; IsEditor = true; LogToConsole = true; }
int32 ULHGenerateFrontendMapCommandlet::Main(const FString& Params)
{
    const FString Package = TEXT("/Game/Lighthaven/Maps/L_Frontend");
    const FString Filename = FPackageName::LongPackageNameToFilename(Package,FPackageName::GetMapPackageExtension());
    if (!GEditor || (IFileManager::Get().FileExists(*Filename) && IFileManager::Get().IsReadOnly(*Filename))) return 1;
    UWorld* World = GEditor->NewMap(false); if (!World) return 1;
    World->GetWorldSettings()->DefaultGameMode = ALHFrontendGameMode::StaticClass();
    if (!IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename),true) || !FEditorFileUtils::SaveMap(World,Filename)) return 1;
    UE_LOG(LogTemp,Display,TEXT("Generated %s"),*Package); return 0;
}
