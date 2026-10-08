#include "LHSaveStore.h"

void FLHSaveStore::Emit(ELHSaveEventKind Kind, const FLHCharacterId& Character, int64 Sequence,
    const FLHSaveError& Error, const FPair* Pair)
{
    FLHSaveEvent Event; Event.Kind=Kind; Event.Character=Character; Event.Sequence=Sequence; Event.Error=Error;
    if (Pair) { Event.Slots[0]=Pair->Errors[0]; Event.Slots[1]=Pair->Errors[1]; }
    Events.Broadcast(Event);
}
FLHSaveStore::FPair FLHSaveStore::ReadPair(const FLHCharacterId& Character, const FLHSaveCompatibility& Compatibility)
{
    FPair Pair;
    for (int32 Slot=0; Slot<2; ++Slot)
    {
        TArray<uint8> Bytes;
        Pair.Valid[Slot]=Storage->Read(Character.Value,Slot,Bytes,Pair.Errors[Slot]) &&
            LHSave::Decode(Bytes,Character,Compatibility,Pair.Saves[Slot],Pair.Errors[Slot]);
        if (Pair.Valid[Slot] && (Pair.Newest==INDEX_NONE || Pair.Saves[Slot].Header.TransactionSequence>Pair.Saves[Pair.Newest].Header.TransactionSequence)) Pair.Newest=Slot;
    }
    if (Pair.Valid[0] && Pair.Valid[1] && Pair.Saves[0].Header.TransactionSequence==Pair.Saves[1].Header.TransactionSequence)
    {
        TArray<uint8> A,B; FLHSaveError Error;
        LHSave::Encode(Pair.Saves[0],A,Error); LHSave::Encode(Pair.Saves[1],B,Error);
        if (A!=B)
        {
            Pair.Newest=INDEX_NONE; Pair.Valid[0]=Pair.Valid[1]=false;
            Pair.Errors[0]=Pair.Errors[1]={ELHSaveReason::InvalidSnapshot,TEXT("Ambiguous equal-sequence generations")};
        }
    }
    return Pair;
}
TArray<FLHSaveProfile> FLHSaveStore::Enumerate(const FLHSaveCompatibility& Compatibility)
{
    check(IsInGameThread()); TArray<FLHSaveProfile> Profiles;
    for (FGuid Id:Storage->Enumerate())
    {
        FLHSaveProfile P; P.Character.Value=Id;
        if (IsWriting(P.Character)) { P.Error={ELHSaveReason::Busy,TEXT("Character write in flight")}; Profiles.Add(P); continue; }
        auto Pair=ReadPair(P.Character,Compatibility);
        P.bReadable=Pair.Newest!=INDEX_NONE;
        if (P.bReadable)
        {
            const auto& S=Pair.Saves[Pair.Newest]; P.Sequence=S.Header.TransactionSequence; P.DisplayName=S.Character.DisplayName;
            const int32 Other=1-Pair.Newest; P.bRecovered=!Pair.Valid[Other] && Pair.Errors[Other].Reason!=ELHSaveReason::NotFound;
            if (P.bRecovered) P.Error=Pair.Errors[Other];
        }
        else P.Error=Pair.Errors[0].Reason==ELHSaveReason::NotFound ? Pair.Errors[1] : Pair.Errors[0];
        Profiles.Add(MoveTemp(P));
    }
    return Profiles;
}
bool FLHSaveStore::Load(const FLHCharacterId& Character, const FLHSaveCompatibility& Compatibility,
    FLHSaveSnapshot& Out, FLHSaveError& Error)
{
    check(IsInGameThread()); Error={};
    if (IsWriting(Character)) { Error={ELHSaveReason::Busy,TEXT("Character write in flight")}; return false; }
    auto Pair=ReadPair(Character,Compatibility);
    if (Pair.Newest==INDEX_NONE)
    {
        Error=Pair.Errors[0].Reason==ELHSaveReason::NotFound ? Pair.Errors[1] : Pair.Errors[0];
        Emit(ELHSaveEventKind::Unreadable,Character,0,Error,&Pair); return false;
    }
    Out=MoveTemp(Pair.Saves[Pair.Newest]);
    const int32 Other=1-Pair.Newest;
    if (!Pair.Valid[Other] && Pair.Errors[Other].Reason!=ELHSaveReason::NotFound)
        Emit(ELHSaveEventKind::Recovered,Character,Out.Header.TransactionSequence,Pair.Errors[Other],&Pair);
    return true;
}
bool FLHSaveStore::IsDirty(const FLHCharacterId& Character) const
{ const FState* S=States.Find(Character.Value); return S && (S->bWriting || S->Pending.IsValid()); }
bool FLHSaveStore::IsWriting(const FLHCharacterId& Character) const
{ const FState* S=States.Find(Character.Value); return S && S->bWriting; }
bool FLHSaveStore::RequestSave(const FLHSaveSnapshot& Snapshot, const FLHSaveCompatibility& Compatibility,
    bool bCompletedActionBoundary, FLHSaveError& Error)
{
    check(IsInGameThread()); Error={};
    if (!bCompletedActionBoundary)
    {
        Error={ELHSaveReason::BoundaryRequired,TEXT("Settle/cancel all actions and unwind publication before snapshot")};
        Emit(ELHSaveEventKind::Failed,Snapshot.Header.CharacterId,0,Error); return false;
    }
    if (!LHSave::Validate(Snapshot,Error)) { Emit(ELHSaveEventKind::Failed,Snapshot.Header.CharacterId,0,Error); return false; }
    FState& S=States.FindOrAdd(Snapshot.Header.CharacterId.Value);
    S.Pending=MakeShared<const FLHSaveSnapshot>(Snapshot); S.Compatibility=Compatibility;
    if (S.bWriting) return true; // Replace only the pending value, never the active snapshot.
    return Start(Snapshot.Header.CharacterId,Error);
}
bool FLHSaveStore::Retry(const FLHCharacterId& Character, FLHSaveError& Error)
{
    check(IsInGameThread()); Error={};
    const FState* S=States.Find(Character.Value);
    if (!S || !S->Pending) { Error={ELHSaveReason::NotFound,TEXT("No dirty snapshot")}; return false; }
    if (S->bWriting) { Error={ELHSaveReason::Busy,TEXT("Character write in flight")}; return false; }
    return Start(Character,Error);
}
bool FLHSaveStore::Start(const FLHCharacterId& Character, FLHSaveError& Error)
{
    FState& State=States.FindChecked(Character.Value);
    const auto Pending=State.Pending;
    const auto Compatibility=State.Compatibility;
    // Reserve the operation before reading, encoding or publishing to prevent re-entrant loads/writes.
    State.bWriting=true;
    auto Pair=ReadPair(Character,Compatibility);
    auto Failure=[&](const FLHSaveError& E)
    { States.FindChecked(Character.Value).bWriting=false; Emit(ELHSaveEventKind::Failed,Character,0,E,&Pair); return false; };
    if (Pair.Newest==INDEX_NONE && (Pair.Errors[0].Reason!=ELHSaveReason::NotFound || Pair.Errors[1].Reason!=ELHSaveReason::NotFound))
    {
        Error=Pair.Errors[0].Reason!=ELHSaveReason::NotFound ? Pair.Errors[0] : Pair.Errors[1]; return Failure(Error);
    }
    const int64 Previous=Pair.Newest==INDEX_NONE ? 0 : Pair.Saves[Pair.Newest].Header.TransactionSequence;
    if (Previous==MAX_int64) { Error={ELHSaveReason::SequenceExhausted,TEXT("Save sequence exhausted")}; return Failure(Error); }
    FLHSaveSnapshot Active=*Pending;
    Active.Header.TransactionSequence=FMath::Max(Previous+1,Active.Header.TransactionSequence);
    TArray<uint8> Bytes;
    if (!LHSave::Encode(Active,Bytes,Error)) return Failure(Error);
    // Check caller compatibility before any disk mutation.
    FLHSaveSnapshot Check;
    if (!LHSave::Decode(Bytes,Character,Compatibility,Check,Error)) return Failure(Error);
    const int32 Slot=Pair.Newest==INDEX_NONE ? 0 : 1-Pair.Newest;
    const int64 Sequence=Active.Header.TransactionSequence;
    State.Pending.Reset(); const uint64 Serial=++State.Serial;
    const FString ExpectedDigest=LHSave::Sha256(Bytes);
    TWeakPtr<FLHSaveStore> Weak=AsShared();
    Storage->Write(Character.Value,Slot,MoveTemp(Bytes),[Weak,Character,Serial,Slot,Sequence,Pending,Compatibility,ExpectedDigest](bool Success)
    {
        auto Self=Weak.Pin(); if (!Self) return;
        check(IsInGameThread());
        auto* Current=Self->States.Find(Character.Value);
        if (!Current || Current->Serial!=Serial) return;
        FLHSaveError CompletionError;
        // A successful OS write is not enough: read back the exact generation and validate it before success.
        if (Success)
        {
            TArray<uint8> Readback; FLHSaveSnapshot Saved;
            Success=Self->Storage->Read(Character.Value,Slot,Readback,CompletionError) &&
                LHSave::Decode(Readback,Character,Compatibility,Saved,CompletionError) && Saved.Header.TransactionSequence==Sequence && LHSave::Sha256(Readback)==ExpectedDigest;
        }
        Current->bWriting=false;
        if (!Success)
        {
            if (!Current->Pending) { Current->Pending=Pending; Current->Compatibility=Compatibility; }
            if (CompletionError.Reason==ELHSaveReason::None) CompletionError={ELHSaveReason::IoFailure,TEXT("Progress not safely saved; previous generation retained")};
            Self->Emit(ELHSaveEventKind::Failed,Character,Sequence,CompletionError);
            return; // No automatic infinite retry on a failed device.
        }
        Self->Emit(ELHSaveEventKind::Succeeded,Character,Sequence,{});
        // Event subscribers may add requests or characters: reacquire map state after publication.
        Current=Self->States.Find(Character.Value);
        if (Current && Current->Pending && !Current->bWriting) { FLHSaveError NextError; Self->Start(Character,NextError); }
    });
    return true;
}
