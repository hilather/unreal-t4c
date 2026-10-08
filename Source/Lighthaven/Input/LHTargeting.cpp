#include "Input/LHTargeting.h"
TArray<FString> LHControls::OrderedTargets(const TArray<FLHTargetCandidate>& Candidates, double MaxDistanceSquared)
{
    TArray<FLHTargetCandidate> Valid;
    for (const auto& C : Candidates) if (!C.Key.IsEmpty() && C.bAlive && C.bReachable && C.bVisible && FMath::IsFinite(C.DistanceSquared) && C.DistanceSquared>=0 && C.DistanceSquared<=MaxDistanceSquared) Valid.Add(C);
    Valid.Sort([](const auto& A,const auto& B) { return A.DistanceSquared==B.DistanceSquared ? A.Key<B.Key : A.DistanceSquared<B.DistanceSquared; });
    TArray<FString> Result;
    for (const auto& C : Valid) Result.AddUnique(C.Key);
    return Result;
}
FString LHControls::CycleTarget(const TArray<FString>& Ordered, const FString& Current, int32 Direction)
{
    if (Ordered.IsEmpty()) return {};
    const int32 Index=Ordered.IndexOfByKey(Current);
    if (Index==INDEX_NONE) return Direction<0 ? Ordered.Last() : Ordered[0];
    return Ordered[(Index+(Direction<0 ? -1 : 1)+Ordered.Num())%Ordered.Num()];
}
