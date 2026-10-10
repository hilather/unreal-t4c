#include "LHWorldTravelSubsystem.h"
#include "LHAreaStateSubsystem.h"
#include "LHWorldMarkers.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"
#include "UObject/UObjectGlobals.h"
void ULHWorldTravelSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection); Collection.InitializeDependency<ULHAreaStateSubsystem>();
    MapLoadedHandle=FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this,&ULHWorldTravelSubsystem::OnMapLoaded);
    if (GEngine) TravelFailureHandle=GEngine->OnTravelFailure().AddUObject(this,&ULHWorldTravelSubsystem::OnTravelFailure);
}
void ULHWorldTravelSubsystem::Deinitialize()
{
    FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(MapLoadedHandle);
    if (GEngine) GEngine->OnTravelFailure().Remove(TravelFailureHandle);
    Adapter.Reset(); SessionBindings={}; MapToken=0; Super::Deinitialize();
}
bool ULHWorldTravelSubsystem::Configure(TSharedRef<FLHSaveStore> Store,const FLHSaveCompatibility& Compatibility,FLHWorldTravelBindings Bindings,FString& Error)
{
    Error.Reset();
    if (Adapter || !Bindings.FreezeInteractionAndAutosaves || !Bindings.SettleAndCapture || !Bindings.CheckpointDurable || !Bindings.InstallCheckpoint || !Compatibility.ValidateReferences)
    { Error=TEXT("Configure once with complete session and compatibility adapters"); return false; }
    SessionBindings=MoveTemp(Bindings);
    FLHTravelSaveAdapter::FHooks Hooks;
    Hooks.PrepareArrival=SessionBindings.PrepareArrival;
    Hooks.Freeze=[this](bool Frozen) { SessionBindings.FreezeInteractionAndAutosaves(Frozen); };
    Hooks.Capture=[this](FLHSaveSnapshot& Snapshot,FString& CaptureError) { return SessionBindings.SettleAndCapture(Snapshot,CaptureError); };
    Hooks.Durable=[this](const FLHSaveSnapshot& Snapshot) { SessionBindings.CheckpointDurable(Snapshot); };
    Hooks.Load=[this](const FLHAreaDefinition& Area,uint64 Token)
    {
        // The coordinator has already frozen the authoritative permitted edge.
        const auto& Source=Adapter->Travel().SourceCheckpoint();
        const auto* SourceArea=LHWorld::FindArea(Source.Character.ActiveEntrance.Area);
        const auto* Edge=SourceArea ? SourceArea->Portals.FindByPredicate([&](const auto& P) { return LHWorld::SameEntrance(P.Source,Source.Character.ActiveEntrance) && LHWorld::SameArea(P.Destination.Area,Area.Id); }) : nullptr;
        if (!Edge) { Adapter->Travel().OnDestinationLoaded(Token,false,{},TEXT("Permitted edge missing")); return; }
        OpenMap(Area,Edge->Destination,Token,false);
    };
    Hooks.Install=[this](const FLHSaveSnapshot& Snapshot,const FLHEntranceDefinition& Entrance,FString& InstallError)
    { return Install(Snapshot,GetGameInstance()->GetWorld(),Entrance,InstallError); };
    Hooks.Restore=[this](const FLHSaveSnapshot& Snapshot,const FLHEntranceDefinition& Entrance,uint64 Token)
    {
        Recovery=Snapshot; const auto* Area=LHWorld::FindArea(Entrance.Id.Area);
        if (!Area) { Adapter->Travel().OnSourceRestored(Token,false,TEXT("Source map missing")); return; }
        OpenMap(*Area,Entrance.Id,Token,true);
    };
    Adapter=MakeUnique<FLHTravelSaveAdapter>(Store,Compatibility,MoveTemp(Hooks)); return true;
}
FLHTravelCoordinator* ULHWorldTravelSubsystem::Travel() const { return Adapter ? &Adapter->Travel() : nullptr; }
bool ULHWorldTravelSubsystem::RequestTravel(const FLHRequestTravelRequest& Request,FString& Error)
{
    if (!Adapter) { Error=TEXT("Travel session is not configured"); return false; }
    const auto* Area=LHWorld::FindArea(Request.Portal.Area); UWorld* World=GetGameInstance()->GetWorld();
    if (!Area || !World || UWorld::RemovePIEPrefix(World->GetOutermost()->GetName())!=Area->Map.GetLongPackageName())
    { Error=TEXT("Portal is not in the current playable map"); return false; }
    return Adapter->Travel().Begin(Request,Error);
}
void ULHWorldTravelSubsystem::OpenMap(const FLHAreaDefinition& Area,const FLHEntranceId& Entrance,uint64 Token,bool bRecovery)
{
    MapToken=Token; ExpectedEntrance=Entrance; bRestoring=bRecovery;
    const FString Package=Area.Map.GetLongPackageName();
    if (!FPackageName::DoesPackageExist(Package)) { FailLoad(TEXT("Destination package missing; generate/cook required map")); return; }
    UGameplayStatics::OpenLevel(GetGameInstance(),FName(*Package));
}
bool ULHWorldTravelSubsystem::Install(const FLHSaveSnapshot& Snapshot,UWorld* World,const FLHEntranceDefinition& Entrance,FString& Error)
{
    const auto* Area=LHWorld::FindArea(Entrance.Id.Area);
    if (!World || !Area || UWorld::RemovePIEPrefix(World->GetOutermost()->GetName())!=Area->Map.GetLongPackageName()) { Error=TEXT("Loaded map does not match checkpoint"); return false; }
    const ALHEntranceMarker* Found=nullptr;
    for (TActorIterator<ALHEntranceMarker> It(World); It; ++It) if (LHWorld::SameEntrance(It->EntranceId,Entrance.Id))
    {
        if (Found) { Error=TEXT("Duplicate arrival marker"); return false; } Found=*It;
    }
    if (!Found || !Found->bSafetyReviewed || !LHWorld::SafeTransform(Found->SafeArrivalTransform) || !Found->SafeArrivalTransform.Equals(Entrance.SafeTransform))
    { Error=TEXT("Missing/invalid/unreviewed arrival marker"); return false; }
    if (!LHWorld::ValidateWorld(Snapshot.World,Error)) return false;
    if (!SessionBindings.InstallCheckpoint(Snapshot,World,Entrance,Error)) return false;
    return GetGameInstance()->GetSubsystem<ULHAreaStateSubsystem>()->Hydrate(Snapshot.World,Error);
}
void ULHWorldTravelSubsystem::OnMapLoaded(UWorld* World)
{
    if (!Adapter || !MapToken || !World || World->GetGameInstance()!=GetGameInstance()) return;
    const uint64 Token=MapToken; MapToken=0;
    if (bRestoring)
    {
        FString Error; const auto* Entrance=LHWorld::FindEntrance(ExpectedEntrance);
        const bool Success=Entrance && Install(Recovery,World,*Entrance,Error);
        Adapter->Travel().OnSourceRestored(Token,Success,Error); return;
    }
    const auto* Area=LHWorld::FindArea(ExpectedEntrance.Area);
    const bool Success=Area && UWorld::RemovePIEPrefix(World->GetOutermost()->GetName())==Area->Map.GetLongPackageName();
    Adapter->Travel().OnDestinationLoaded(Token,Success,ExpectedEntrance,Success ? FString() : TEXT("Unexpected loaded map"));
}
void ULHWorldTravelSubsystem::FailLoad(const FString& Error)
{
    if (!Adapter || !MapToken) return;
    const uint64 Token=MapToken; MapToken=0;
    if (bRestoring) Adapter->Travel().OnSourceRestored(Token,false,Error);
    else Adapter->Travel().OnDestinationLoaded(Token,false,{},Error);
}
void ULHWorldTravelSubsystem::OnTravelFailure(UWorld* World,ETravelFailure::Type Type,const FString& Error)
{ if (!World || World->GetGameInstance()==GetGameInstance()) FailLoad(Error); }
