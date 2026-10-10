#include "LHTravelSaveAdapter.h"
#include "LHAreaStateSubsystem.h"
FLHTravelSaveAdapter::FLHTravelSaveAdapter(TSharedRef<FLHSaveStore> InStore,const FLHSaveCompatibility& InCompatibility,FHooks InHooks)
    : Store(InStore),Compatibility(InCompatibility),Hooks(MoveTemp(InHooks)),Coordinator(*this)
{ SaveHandle=Store->Events.AddRaw(this,&FLHTravelSaveAdapter::OnSave); }
FLHTravelSaveAdapter::~FLHTravelSaveAdapter() { Store->Events.Remove(SaveHandle); }
void FLHTravelSaveAdapter::FreezeInteraction(bool bFreeze) { if (Hooks.Freeze) Hooks.Freeze(bFreeze); }
bool FLHTravelSaveAdapter::SettleAndCapture(FLHSaveSnapshot& Out,FString& Error)
{
    if (!Hooks.Freeze || !Hooks.Capture || !Hooks.Load || !Hooks.Install || !Hooks.Restore || !Compatibility.ValidateReferences)
    { Error=TEXT("Travel requires complete session/map/compatibility hooks"); return false; }
    if (!Hooks.Capture(Out,Error)) return false;
    if (Store->IsDirty(Out.Header.CharacterId) || Store->IsWriting(Out.Header.CharacterId)) { Error=TEXT("Pending save must finish before travel"); return false; }
    FLHSaveError SaveError;
    if (!LHSave::Validate(Out,SaveError) || !Compatibility.ValidateReferences(Out,SaveError) || !LHWorld::ValidateWorld(Out.World,Error))
    { if (Error.IsEmpty()) Error=SaveError.Detail; return false; }
    return true;
}
void FLHTravelSaveAdapter::SaveCheckpoint(const FLHSaveSnapshot& Snapshot,uint64 Token)
{
    Character=Snapshot.Header.CharacterId.Value; SaveToken=Token; bAwaitingSave=true;
    FLHSaveError Error;
    // Dirty state belongs to this immutable failed checkpoint: retry it without coalescing.
    const bool bAccepted=Store->IsDirty(Snapshot.Header.CharacterId)
        ? Store->Retry(Snapshot.Header.CharacterId,Error)
        : Store->RequestSave(Snapshot,Compatibility,true,Error);
    // Store failures may publish inline. Notify only if its callback has not already consumed this ticket.
    if (!bAccepted && bAwaitingSave && SaveToken==Token)
    { bAwaitingSave=false; Coordinator.OnSaveCompleted(Token,false,0,Error.Detail); }
}
void FLHTravelSaveAdapter::OnSave(const FLHSaveEvent& Event)
{
    if (!bAwaitingSave || Event.Character.Value!=Character || (Event.Kind!=ELHSaveEventKind::Succeeded && Event.Kind!=ELHSaveEventKind::Failed)) return;
    const uint64 Token=SaveToken; bAwaitingSave=false;
    Coordinator.OnSaveCompleted(Token,Event.Kind==ELHSaveEventKind::Succeeded,Event.Sequence,Event.Error.Detail);
}
void FLHTravelSaveAdapter::CheckpointDurable(const FLHSaveSnapshot& Snapshot) { if (Hooks.Durable) Hooks.Durable(Snapshot); }
void FLHTravelSaveAdapter::LoadDestination(const FLHAreaDefinition& Area,uint64 Token) { Hooks.Load(Area,Token); }
bool FLHTravelSaveAdapter::ValidateAndInstallArrival(const FLHSaveSnapshot& Snapshot,const FLHEntranceDefinition& Entrance,FString& Error)
{ return Hooks.Install(Snapshot,Entrance,Error); }
void FLHTravelSaveAdapter::RestoreSource(const FLHSaveSnapshot& Snapshot,const FLHEntranceDefinition& Entrance,uint64 Token)
{ Hooks.Restore(Snapshot,Entrance,Token); }

bool FLHTravelSaveAdapter::PrepareArrival(FLHSaveSnapshot& Snapshot,FString& Error)
{ return !Hooks.PrepareArrival || Hooks.PrepareArrival(Snapshot,Error); }
