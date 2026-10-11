#include "Framework/LHSessionSubsystem.h"
#include "Persistence/LHSaveSubsystem.h"
#include "World/LHWorldTravelSubsystem.h"
#include "World/LHWorldMarkers.h"
#include "Framework/LHPlayerState.h"
#include "Framework/LHCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Misc/PackageName.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
namespace LHSessionSubsystemPrivate
{
bool Place(UWorld* World,const FLHEntranceDefinition& Entrance,FString& Error)
{
    auto* Pawn=Cast<ALHCharacter>(UGameplayStatics::GetPlayerPawn(World,0));
    if (!Pawn) { Error=TEXT("Arrival avatar unavailable"); return false; }
    int32 Count=0;
    for (TActorIterator<ALHEntranceMarker> It(World);It;++It)
        if (LHWorld::SameEntrance(It->EntranceId,Entrance.Id) && It->bSafetyReviewed && It->SafeArrivalTransform.Equals(Entrance.SafeTransform)) ++Count;
    if (Count!=1) { Error=TEXT("Arrival requires one reviewed registry marker"); return false; }
    const auto* Capsule=Pawn->GetCapsuleComponent();
    const FVector Center=Entrance.SafeTransform.GetLocation()+FVector(0,0,Capsule->GetScaledCapsuleHalfHeight()+2);
    FCollisionQueryParams Params; Params.AddIgnoredActor(Pawn);
    if (World->OverlapBlockingTestByChannel(Center,Entrance.SafeTransform.GetRotation(),ECC_Pawn,
        FCollisionShape::MakeCapsule(Capsule->GetScaledCapsuleRadius(),Capsule->GetScaledCapsuleHalfHeight()),Params))
    { Error=TEXT("Arrival capsule blocked"); return false; }
    Pawn->GetCharacterMovement()->StopMovementImmediately();
    return Pawn->SetActorLocationAndRotation(Center,Entrance.SafeTransform.GetRotation(),false,nullptr,ETeleportType::TeleportPhysics);
}
}
void ULHSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection); Collection.InitializeDependency<ULHSaveSubsystem>();
    Collection.InitializeDependency<ULHWorldTravelSubsystem>();
    auto* Saves=GetGameInstance()->GetSubsystem<ULHSaveSubsystem>();
    Saves->Configure(FLHWave2Session::Compatibility()); Live=MakeShared<FLHWave2Session>(Saves->GetStore().ToSharedRef());
    if (FPlatformProperties::RequiresCookedData() || FParse::Param(FCommandLine::Get(),TEXT("LHCheckPackagedContent")))
    {
        bool Available=FPackageName::DoesPackageExist(TEXT("/Game/Lighthaven/Maps/L_Frontend"));
        for (const auto& A:LHWorld::Registry())
        {
            const bool Exists=FPackageName::DoesPackageExist(A.Map.GetLongPackageName()); Available &= Exists;
            UE_LOG(LogTemp,Display,TEXT("LH PACKAGED CONTENT %s %s"),*A.Map.GetLongPackageName(),Exists?TEXT("PASS"):TEXT("FAIL"));
        }
        Live->bContentUnavailable=!Available;
        if (FParse::Param(FCommandLine::Get(),TEXT("LHCheckPackagedContent")))
        { FPlatformMisc::RequestExitWithStatus(false,Available?0:1); return; }
    }
    auto* WorldTravel=GetGameInstance()->GetSubsystem<ULHWorldTravelSubsystem>();
    FLHWorldTravelBindings Bindings;
    Bindings.PrepareArrival=FLHWave2Session::PopulateEncounterCheckpoint;
    Bindings.FreezeInteractionAndAutosaves=[this](bool Frozen) { Live->FreezeWorldTravel(Frozen); };
    Bindings.SettleAndCapture=[this](FLHSaveSnapshot& Out,FString& Error) { return Live->CaptureTravel(Out,Error); };
    Bindings.CheckpointDurable=[this](const FLHSaveSnapshot& Snapshot) { Live->InstallTravel(Snapshot); };
    Bindings.InstallCheckpoint=[this](const FLHSaveSnapshot& Snapshot,UWorld* World,const FLHEntranceDefinition& Entrance,FString& Error)
    { const bool Success=LHSessionSubsystemPrivate::Place(World,Entrance,Error) && Live->InstallTravel(Snapshot) && Live->StartEncounters(); if (Success) Live->CompleteGameplayArrival(); return Success; };
    FString Error;
    if (!WorldTravel->Configure(Saves->GetStore().ToSharedRef(),FLHWave2Session::Compatibility(),MoveTemp(Bindings),Error))
        UE_LOG(LogTemp,Error,TEXT("Travel configuration: %s"),*Error);
    Live->RequestWorldTravel=[WorldTravel](const FLHRequestTravelRequest& Request,FString& Detail) { return WorldTravel->RequestTravel(Request,Detail); };
    Live->WorldTravelStatus=[WorldTravel]() { return WorldTravel->Travel() ? WorldTravel->Travel()->GetError() : FString(TEXT("Travel unavailable")); };
    Live->RetryWorldTravel=[WorldTravel](FString& Detail) { return WorldTravel->Travel() && WorldTravel->Travel()->Retry(Detail); };
    Live->Travel=[this]() {
        const auto* Area=LHWorld::FindArea(Live->Snapshot().Character.ActiveEntrance.Area);
        if (Area) UGameplayStatics::OpenLevel(GetGameInstance(),FName(*Area->Map.GetLongPackageName()));
    };
    Live->Exit=[]() { FPlatformMisc::RequestExit(false); };
}
void ULHSessionSubsystem::Deinitialize()
{
    if (Live) { Live->Travel=nullptr; Live->Exit=nullptr; Live->RequestWorldTravel=nullptr; Live->RetryWorldTravel=nullptr; Live->WorldTravelStatus=nullptr; } Live.Reset(); Super::Deinitialize();
}
bool ULHSessionSubsystem::PlaceSessionArrival(UWorld* World,FString& Error)
{
    if (!Live || !Live->HasCharacter()) return false;
    const auto* E=LHWorld::FindEntrance(Live->Snapshot().Character.ActiveEntrance);
    const bool Success=E && LHSessionSubsystemPrivate::Place(World,*E,Error) && Live->StartEncounters();
    if (Success) Live->CompleteGameplayArrival();
    return Success;
}

#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
#include "Framework/LHWave2Profile.h"
namespace LHCaptureSessionPrivate
{
class FStorage final : public ILHSaveStorage
{
    TMap<FGuid,TArray<uint8>> Slots[2];
public:
    TArray<FGuid> Enumerate() override { TArray<FGuid> Ids; Slots[0].GetKeys(Ids); return Ids; }
    bool Read(const FGuid& Id,int32 Slot,TArray<uint8>& Bytes,FLHSaveError& Error) override
    { if(auto* Found=Slots[Slot].Find(Id)) { Bytes=*Found; return true; } Error={ELHSaveReason::NotFound,TEXT("Capture generation absent")}; return false; }
    void Write(const FGuid& Id,int32 Slot,TArray<uint8> Bytes,TFunction<void(bool)> Done) override
    { Slots[Slot].Add(Id,MoveTemp(Bytes)); Done(true); }
};
}
bool ULHSessionSubsystem::PrepareCaptureSession(UWorld* World,bool BodyB,FString& Error)
{
    if(!World || (Live && Live->HasCharacter())) { Error=TEXT("Capture requires an empty session"); return false; }
    const auto* Area=LHWorld::Registry().FindByPredicate([&](const auto& A){return A.Map.GetAssetName()==World->GetMapName();});
    if(!Area) { Error=TEXT("Capture map is not registered"); return false; }
    // Production creation/save boundary, isolated from the user's profiles and disk.
    Live=MakeShared<FLHWave2Session>(MakeShared<FLHSaveStore>(MakeShared<LHCaptureSessionPrivate::FStorage>()));
    auto* State=World->SpawnActor<ALHPlayerState>();
    if(!State) { Error=TEXT("Capture creation owner unavailable"); return false; }
    bool Success=Live->Bind(State,true);
    TArray<FLHContentId> Looks;
    for(const TCHAR* Id:{BodyB?TEXT("Presentation.Player.Body.B"):TEXT("Presentation.Player.Body.A"),TEXT("Presentation.Player.Hair.Cropped"),TEXT("Presentation.Player.Skin.LightWarm"),TEXT("Presentation.Player.Outfit.StarterLinen")})
    { FLHContentId V; V.Value=Id; Looks.Add(V); }
    const auto Preview=Live->Preview(TEXT("Capture Starter"),Looks,LHWave2::PrototypeAnswers(),false);
    FLHCreateCharacterRequest Q; Q.Request.Epoch=Live->Snapshot().Session.RequestEpoch; Q.Request.Value=FGuid::NewGuid();
    Q.DisplayName=TEXT("Capture Starter"); Q.AppearanceIds=Looks; Q.Creation=Preview.Record; Q.PreviewToken=Preview.Token;
    Success=Success && Preview.bLegal && Live->Execute(Q).Disposition==ELHCommandDisposition::Accepted;
    if(Success)
    {
        Live->Flush(); // synchronous in-memory readback, same codec and compatibility validation
        Success=!Live->HasUnsavedChanges() && Live->Bind(State);
        auto Snapshot=Live->Snapshot(); Snapshot.Character.ActiveEntrance=Area->SafeFallback;
        if(!Snapshot.World.Areas.ContainsByPredicate([&](const auto& A){return LHWorld::SameArea(A.Area,Area->Id);}))
        { FLHAreaRecord A; A.Area=Area->Id; Snapshot.World.Areas.Add(A); }
        Success=Success && FLHWave2Session::PopulateEncounterCheckpoint(Snapshot,Error) && Live->InstallTravel(Snapshot);
    }
    State->Destroy(); // next possession binds the actual player state and restores full resources
    if(!Success && Error.IsEmpty()) Error=TEXT("Production capture creation failed");
    return Success;
}
#endif
