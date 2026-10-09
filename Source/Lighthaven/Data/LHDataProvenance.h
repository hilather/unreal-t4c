#pragma once
#include "Core/LHDefinitions.h"
namespace LHData
{
inline FLHFieldProvenance Provenance(const TCHAR* Field, const TCHAR* Url, const TCHAR* Date,
    ELHProvenanceStatus Status, const TCHAR* Notes)
{
    FLHFieldProvenance P; P.FieldPath=Field; P.SourceUrl=Url; P.RetrievedDate=Date; P.Status=Status;
    P.SourceBaseline=Status==ELHProvenanceStatus::Prototype ? TEXT("LH_Prototype_v1 / W4-02") : TEXT("T4C Bible; selected classic, live fills gaps; server version unstated");
    P.Notes=Notes; return P;
}
inline FLHNumber Number(double V, FLHFieldProvenance P)
{ FLHNumber N; N.Resolution=ELHValueResolution::Resolved; N.Value=V; P.ValueAsRecorded=FString::SanitizeFloat(V); N.Provenance=MoveTemp(P); return N; }
inline FLHInteger Integer(int64 V, FLHFieldProvenance P)
{ FLHInteger I; I.Resolution=ELHValueResolution::Resolved; I.Value=V; P.ValueAsRecorded=FString::Printf(TEXT("%lld"),static_cast<long long>(V)); I.Provenance=MoveTemp(P); return I; }
inline FLHFieldProvenance Prototype(const TCHAR* Field, const TCHAR* Notes)
{ return Provenance(Field,TEXT(""),TEXT("2026-10-09"),ELHProvenanceStatus::Prototype,Notes); }
inline FLHNumber PNumber(double V, const TCHAR* Field, const TCHAR* Notes)
{ return Number(V,Prototype(Field,Notes)); }
inline FLHInteger PInteger(int64 V, const TCHAR* Field, const TCHAR* Notes)
{ return Integer(V,Prototype(Field,Notes)); }
inline FLHFieldProvenance Classic(const TCHAR* Field, const TCHAR* Page, const TCHAR* Notes=TEXT("R-03 retained classic capture; not verified in play"))
{ return Provenance(Field,Page,TEXT("2026-10-07"),ELHProvenanceStatus::Confirmed,Notes); }
inline FLHFieldProvenance Live(const TCHAR* Field, const TCHAR* Page)
{ return Provenance(Field,Page,TEXT("2026-10-09"),ELHProvenanceStatus::Confirmed,TEXT("R-03 live documentary value; not verified in play")); }
inline FLHPolicyField Policy(const TCHAR* Key, const TCHAR* Value, FLHFieldProvenance P)
{ FLHPolicyField F; F.Key=Key; F.Value=Value; F.Resolution=ELHValueResolution::Resolved; P.ValueAsRecorded=Value; F.Provenance=MoveTemp(P); return F; }
}
