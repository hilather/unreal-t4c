#include "LHTravelCoordinator.h"
#include "LHAreaStateSubsystem.h"
#include <limits>

bool FLHTravelCoordinator::Begin(const FLHRequestTravelRequest& Request,FString& Error)
{
    check(IsInGameThread()); Error.Reset();
    if (IsFrozen() || bDispatching) { Error=TEXT("Travel already pending"); return false; }
    if (Operation>std::numeric_limits<uint64>::max()-8) { Error=TEXT("Travel token exhausted"); return false; }
    const auto* Portal=LHWorld::FindPortal(Request.Portal);
    if (!Portal || !LHWorld::SameEntrance(Portal->Destination,Request.Destination) || !LHWorld::FindEntrance(Portal->Source) || !LHWorld::FindEntrance(Portal->Destination))
    { Error=TEXT("Missing destination or unauthorized portal edge"); return false; }
    // Freeze before capture prevents a reentrant command from changing the checkpoint.
    Phase=ELHTravelPhase::SavingSource; ++Operation;
    bDispatching=true; Host.FreezeInteraction(true);
    FLHSaveSnapshot Captured;
    const bool bCaptured=Host.SettleAndCapture(Captured,Error); bDispatching=false;
    if (!bCaptured) { LastError=Error; Finish(); return false; }
    if (!Request.Request.Epoch.IsValid() || Captured.Character.CurrentHealth.Resolution!=ELHValueResolution::Resolved || !FMath::IsFinite(Captured.Character.CurrentHealth.Value) || Captured.Character.CurrentHealth.Value<=0 || !Request.Request.Value.IsValid() || Request.Request.Epoch!=Captured.Session.RequestEpoch || Request.Portal.RunId!=Captured.World.RunId || !LHWorld::SameArea(Captured.Character.ActiveEntrance.Area,Portal->Source.Area) ||
        !LHWorld::ValidateWorld(Captured.World,Error) || Captured.Header.TransactionSequence<0 || Captured.Header.TransactionSequence>=std::numeric_limits<int64>::max()-1)
    { if (Error.IsEmpty()) Error=TEXT("Invalid source/run/request/sequence"); LastError=Error; Finish(); return false; }
    Source=MoveTemp(Captured); Source.Character.ActiveEntrance=Portal->Source; ++Source.Header.TransactionSequence;
    if (!Source.World.Areas.ContainsByPredicate([&](const auto& A) { return LHWorld::SameArea(A.Area,Portal->Destination.Area); }))
    { FLHAreaRecord EmptyArea; EmptyArea.Area=Portal->Destination.Area; Source.World.Areas.Add(EmptyArea); }
    Edge=*Portal; Arrival={}; LastError.Reset();
    Host.SaveCheckpoint(Source,Operation); return true;
}
void FLHTravelCoordinator::OnSaveCompleted(uint64 Token,bool bSuccess,int64 Sequence,const FString& Error)
{
    check(IsInGameThread());
    if (bDispatching || Token!=Operation || (Phase!=ELHTravelPhase::SavingSource && Phase!=ELHTravelPhase::SavingArrival)) return;
    const bool bSource=Phase==ELHTravelPhase::SavingSource;
    if (!bSuccess)
    { Phase=bSource ? ELHTravelPhase::SourceSaveFailed : ELHTravelPhase::ArrivalSaveFailed; LastError=Error; return; }
    const int64 Minimum=bSource ? Source.Header.TransactionSequence : Arrival.Header.TransactionSequence;
    if (Sequence<Minimum || (bSource && Sequence>=std::numeric_limits<int64>::max()))
    { Phase=bSource ? ELHTravelPhase::SourceSaveFailed : ELHTravelPhase::ArrivalSaveFailed; LastError=TEXT("Invalid durability sequence"); return; }
    if (bSource)
    {
        Source.Header.TransactionSequence=Sequence;
        const auto* Destination=LHWorld::FindArea(Edge.Destination.Area);
        if (!Destination) { Recover(TEXT("Destination registry missing")); return; }
        Phase=ELHTravelPhase::LoadingDestination; ++Operation;
        bDispatching=true; Host.CheckpointDurable(Source); bDispatching=false;
        Host.LoadDestination(*Destination,Operation);
    }
    else { Arrival.Header.TransactionSequence=Sequence; bDispatching=true; Host.CheckpointDurable(Arrival); bDispatching=false; Finish(); }
}
void FLHTravelCoordinator::OnDestinationLoaded(uint64 Token,bool bSuccess,const FLHEntranceId& LoadedEntrance,const FString& Error)
{
    check(IsInGameThread());
    if (Token!=Operation || Phase!=ELHTravelPhase::LoadingDestination) return;
    const auto* Entrance=LHWorld::FindEntrance(Edge.Destination);
    if (!bSuccess || !Entrance || !LHWorld::SameEntrance(LoadedEntrance,Edge.Destination) || Entrance->TransformResolution!=ELHValueResolution::Resolved || !LHWorld::SafeTransform(Entrance->SafeTransform))
    { Recover(Error.IsEmpty() ? TEXT("Invalid destination load/entrance") : Error); return; }
    Arrival=Source; Arrival.Character.ActiveEntrance=Edge.Destination; ++Arrival.Header.TransactionSequence;
    FString InstallError;
    // Guard duplicate/reentrant load notifications before calling the host.
    Phase=ELHTravelPhase::SavingArrival; ++Operation; bDispatching=true;
    const bool bInstalled=Host.PrepareArrival(Arrival,InstallError) && Host.ValidateAndInstallArrival(Arrival,*Entrance,InstallError); bDispatching=false;
    if (!bInstalled) { Recover(InstallError); return; }
    Host.SaveCheckpoint(Arrival,Operation);
}
void FLHTravelCoordinator::Recover(const FString& Error)
{
    LastError=Error; Phase=ELHTravelPhase::RestoringSource; ++Operation;
    const auto* Entrance=LHWorld::FindEntrance(Source.Character.ActiveEntrance);
    if (!Entrance) { Phase=ELHTravelPhase::RecoveryFailed; LastError=TEXT("Source entrance missing; interaction remains frozen"); return; }
    Host.RestoreSource(Source,*Entrance,Operation);
}
void FLHTravelCoordinator::OnSourceRestored(uint64 Token,bool bSuccess,const FString& Error)
{
    check(IsInGameThread());
    if (Token!=Operation || Phase!=ELHTravelPhase::RestoringSource) return;
    if (!bSuccess) { Phase=ELHTravelPhase::RecoveryFailed; LastError=Error; return; }
    Finish();
}
void FLHTravelCoordinator::Finish()
{ Phase=ELHTravelPhase::Idle; bDispatching=true; Host.FreezeInteraction(false); bDispatching=false; }
bool FLHTravelCoordinator::Retry(FString& Error)
{
    check(IsInGameThread()); Error.Reset();
    if (bDispatching || Operation>std::numeric_limits<uint64>::max()-8) { Error=TEXT("Host callback in progress or travel tickets exhausted"); return false; }
    if (Phase==ELHTravelPhase::SourceSaveFailed) { Phase=ELHTravelPhase::SavingSource; ++Operation; Host.SaveCheckpoint(Source,Operation); return true; }
    if (Phase==ELHTravelPhase::ArrivalSaveFailed) { Phase=ELHTravelPhase::SavingArrival; ++Operation; Host.SaveCheckpoint(Arrival,Operation); return true; }
    if (Phase==ELHTravelPhase::RecoveryFailed) { Recover(LastError); return true; }
    Error=TEXT("No failed travel operation to retry"); return false;
}
bool FLHTravelCoordinator::Cancel(FString& Error)
{
    check(IsInGameThread()); Error.Reset();
    if (bDispatching || Operation>std::numeric_limits<uint64>::max()-8) { Error=TEXT("Host callback in progress or travel tickets exhausted"); return false; }
    if (Phase==ELHTravelPhase::SourceSaveFailed) { Finish(); return true; }
    // A failed arrival write can contain readable bytes: cancellation cannot claim source is durable.
    // Retry arrival or restart with the save store's visible recovery policy instead.
    Error=TEXT("Cancel is only safe before destination load; retry arrival/recovery"); return false;
}
