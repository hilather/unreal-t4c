#pragma once
#include "World/LHAreaRegistry.h"
#include "Persistence/LHSaveCodec.h"
#include "Misc/SecureHash.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

// Shared by automation and deterministic generators. No manual approval path.
namespace LHArrivalReview
{
inline FString Key(const FLHEntranceId& Id)
{ return Id.Area.Content.Value.ToString()+TEXT("/")+Id.LocalId.ToString(); }
inline FString Hash(const FTransform& Transform)
{
    FLHSaveError Error; const auto Bytes=LHSave::CanonicalValue(Transform,Error);
    if (Error.Reason!=ELHSaveReason::None) return {};
    uint8 Digest[20]; FSHA1::HashBuffer(Bytes.GetData(),Bytes.Num(),Digest);
    return BytesToHex(Digest,20);
}
inline bool IsReviewed(const FLHEntranceDefinition& Entrance)
{
    TArray<FString> Rows;
    if (!FFileHelper::LoadFileToStringArray(Rows,*(FPaths::ProjectConfigDir()/TEXT("Lighthaven/ReviewedArrivals.tsv")))) return false;
    const FString Expected=Key(Entrance.Id)+TEXT("\t")+Hash(Entrance.SafeTransform);
    return Rows.Contains(Expected);
}
}
