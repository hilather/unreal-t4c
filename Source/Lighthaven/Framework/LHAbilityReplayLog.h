#pragma once
#include "Core/LHCommands.h"
#include "Persistence/LHSaveCodec.h"
#include "Hash/Blake3.h"

// Runtime only, lifetime owned by FLHWave2Session. Never evict accepted IDs:
// a random GUID carries no trustworthy age or order.
class FLHAbilityReplayLog
{
public:
    FLHCommandResult Execute(const FLHUseAbilityRequest& Q,TFunctionRef<ELHCommandReason()> Activate)
    {
        FLHCommandResult R; R.Request=Q.Request;
        const FString Canonical=LHSave::RequestDigest(TEXT("UseAbility"),Q.StaticStruct(),&Q);
        if (Canonical.IsEmpty()) return R;
        const FTCHARToUTF8 Utf8(*Canonical);
        const auto Digest=FBlake3::HashBuffer(Utf8.Get(),Utf8.Length());
        if (const auto* Previous=Accepted.Find(Q.Request.Value))
        {
            if (*Previous!=Digest) { R.Reason=ELHCommandReason::ReusedRequestId; return R; }
            R.Reason=ELHCommandReason::None; R.Disposition=ELHCommandDisposition::Accepted; R.bReplay=true;
            return R;
        }
        R.Reason=Activate();
        if (R.Reason==ELHCommandReason::None)
        {
            // Acceptance only sets request, None, Accepted; sequence stays zero.
            R.Disposition=ELHCommandDisposition::Accepted;
            Accepted.Add(Q.Request.Value,Digest);
        }
        return R;
    }
    void Reset() { Accepted.Reset(); }
    SIZE_T GetAllocatedSize() const { return Accepted.GetAllocatedSize(); }
private:
    TMap<FGuid,FBlake3Hash> Accepted;
};
