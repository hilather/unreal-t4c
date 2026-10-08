#include "LHSaveSubsystem.h"
#include "Misc/Paths.h"

void ULHSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    Store=MakeShared<FLHSaveStore>(LHCreateLocalSaveStorage(FPaths::ProjectSavedDir()/TEXT("SaveGames/Lighthaven")));
    Store->Events.AddUObject(this,&ULHSaveSubsystem::ForwardEvent);
}
void ULHSaveSubsystem::Deinitialize()
{
    Store.Reset(); bConfigured=false; Events.Clear(); Super::Deinitialize();
}
void ULHSaveSubsystem::Configure(const FLHSaveCompatibility& InCompatibility)
{
    check(IsInGameThread());
    // Compatibility is immutable for the lifetime of this instance. Changing rules needs a new session/migration.
    if (bConfigured)
    {
        FLHSaveEvent E; E.Kind=ELHSaveEventKind::Unreadable; E.Error={ELHSaveReason::IncompatibleRuleset,TEXT("Changing compatibility requires a new session/migration")};
        Events.Broadcast(E); return;
    }
    Compatibility=InCompatibility; bConfigured=true;
}
bool ULHSaveSubsystem::Ready(FLHSaveError& Error) const
{
    Error={};
    if (!Store || !bConfigured || !Compatibility.ValidateReferences) { Error={ELHSaveReason::IncompatibleRuleset,TEXT("Save subsystem needs authoritative compatibility configuration and registry validator")}; return false; }
    return true;
}
TArray<FLHSaveProfile> ULHSaveSubsystem::EnumerateCharacters()
{
    FLHSaveError Error;
    if (Ready(Error)) return Store->Enumerate(Compatibility);
    FLHSaveEvent E; E.Kind=ELHSaveEventKind::Unreadable; E.Error=Error; Events.Broadcast(E); return {};
}
bool ULHSaveSubsystem::LoadCharacter(const FLHCharacterId& Character, FLHSaveSnapshot& Snapshot, FLHSaveError& Error)
{
    if (Ready(Error)) return Store->Load(Character,Compatibility,Snapshot,Error);
    FLHSaveEvent E; E.Kind=ELHSaveEventKind::Unreadable; E.Character=Character; E.Error=Error; Events.Broadcast(E); return false;
}
bool ULHSaveSubsystem::SaveSnapshot(const FLHSaveSnapshot& Snapshot, bool bCompletedActionBoundary, FLHSaveError& Error)
{
    if (Ready(Error)) return Store->RequestSave(Snapshot,Compatibility,bCompletedActionBoundary,Error);
    FLHSaveEvent E; E.Kind=ELHSaveEventKind::Failed; E.Character=Snapshot.Header.CharacterId; E.Error=Error; Events.Broadcast(E); return false;
}
bool ULHSaveSubsystem::RetrySave(const FLHCharacterId& Character, FLHSaveError& Error)
{
    if (Ready(Error)) return Store->Retry(Character,Error);
    FLHSaveEvent E; E.Kind=ELHSaveEventKind::Failed; E.Character=Character; E.Error=Error; Events.Broadcast(E); return false;
}
bool ULHSaveSubsystem::IsDirty(const FLHCharacterId& Character) const
{ return Store && Store->IsDirty(Character); }
bool ULHSaveSubsystem::IsWriting(const FLHCharacterId& Character) const
{ return Store && Store->IsWriting(Character); }

void ULHSaveSubsystem::ForwardEvent(const FLHSaveEvent& Event) { Events.Broadcast(Event); }
