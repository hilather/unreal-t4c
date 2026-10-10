#pragma once
#include "CoreMinimal.h"
namespace LHInteractionReach
{
// Prototype slice tuning: 250 cm horizontal reach and 150 cm capsule-centre
// to ground-marker vertical tolerance. Replace after interaction play review.
constexpr double HorizontalCm=250;
constexpr double VerticalCm=150;
inline bool Contains(const FVector& Pawn,const FVector& Marker,double Range=HorizontalCm)
{ return FVector::DistSquaredXY(Pawn,Marker)<=Range*Range && FMath::Abs(Pawn.Z-Marker.Z)<=VerticalCm; }
}
