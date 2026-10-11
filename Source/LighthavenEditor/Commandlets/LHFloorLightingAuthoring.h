#pragma once

#include "Visual/LHFloorLighting.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Engine/SkyLight.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/World.h"
#include "EngineUtils.h"

// W5-19 Prototype presentation. Call AFTER Dress: the old point lights are
// fixture placement inputs. Never change geometry, arrival or dressing recipes.
namespace LHFloorLightingAuthoring
{
inline bool Apply(UWorld* World, ELHVisualStyle Style)
{
    const auto* Profile=LHFloorLighting::FindProfile(Style);
    if(!World || !Profile) return false; // B1 keeps its existing path verbatim.
    TArray<AActor*> Remove;
    auto ConfigureFill=[&](APointLight* Actor, bool Exterior)
    {
        auto* C=Actor->PointLightComponent.Get();
        C->SetMobility(EComponentMobility::Movable);
        C->SetIntensityUnits(ELightUnits::Lumens);
        C->SetUseTemperature(false);
        C->SetIntensity(Profile->FillLumens);
        C->SetAttenuationRadius(Profile->FillRadius);
        C->SetLightColor(Exterior?FLinearColor(.67f,.81f,1.f):Profile->FillColor);
        C->SetCastShadows(false);
        C->bUseInverseSquaredFalloff=true;
        C->SetSpecularScale(0.f); // Diffuse bounce; retain wet glints at fixtures.
        C->SetCanEverAffectNavigation(false);
        Actor->SetActorLocation(FVector(Actor->GetActorLocation().X,Actor->GetActorLocation().Y,Profile->FillHeight));
        Actor->Tags.AddUnique(TEXT("LH.Lighting.RoomFill"));
        if(Exterior) Actor->Tags.AddUnique(TEXT("LH.Lighting.ExteriorFill"));
    };
    for(TActorIterator<APointLight> It(World);It;++It)
    {
        const FString Name=It->GetName();
        const bool Coverage=Name.StartsWith(TEXT("Coverage_")) ||
            (Name.StartsWith(TEXT("A04_T1")) && Name.Len()==9);
        if(Coverage) ConfigureFill(*It,false);
        else Remove.Add(*It);
    }
    for(TActorIterator<ASpotLight> It(World);It;++It) Remove.Add(*It);
    for(AActor* A:Remove) World->DestroyActor(A);
    if(Style==ELHVisualStyle::Church)
    {
        // Room bounce reaches faces and full-height plaster. Exterior positions
        // use cooler sky colour. No fill is attached to an NPC or service marker.
        const FVector Rooms[]={
            {800,600,0},{800,1900,0},{-550,450,0},
            {-2450,1700,0},{-1350,-1900,0},{1050,-2850,0},
            {1100,-5800,0},{-1900,-6250,0},{-4200,7200,0},{-2300,7700,0}};
        const FVector Outside[]={{-300,-450,0},{-600,-3800,0},{-800,-5800,0},{-2700,5500,0}};
        int32 Index=0;
        auto Add=[&](FVector Position,bool Exterior)
        {
            FActorSpawnParameters Params;
            Params.Name=FName(*FString::Printf(TEXT("ChapelBounce_%02d"),Index++));
            Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            auto* A=World->SpawnActor<APointLight>(Position,FRotator::ZeroRotator,Params);
            if(A) ConfigureFill(A,Exterior);
            return A!=nullptr;
        };
        for(const FVector& P:Rooms) if(!Add(P,false)) return false;
        for(const FVector& P:Outside) if(!Add(P,true)) return false;
        for(TActorIterator<ADirectionalLight> It(World);It;++It)
        {
            auto* C=CastChecked<UDirectionalLightComponent>(It->GetLightComponent());
            C->SetIntensity(12.f); C->SetTemperature(6500.f);
            C->SetLightColor(FLinearColor(.74f,.84f,1.f));
        }
    }
    for(TActorIterator<ASkyLight> It(World);It;++It)
    {
        auto* C=It->GetLightComponent();
        C->SetIntensity(Profile->SkyIntensity); C->SetLightColor(Profile->SkyColor);
        C->SetCastShadows(false); C->bRealTimeCapture=false;
        C->bLowerHemisphereIsBlack=false; C->LowerHemisphereColor=FLinearColor::White;
    }
    for(TActorIterator<APostProcessVolume> It(World);It;++It)
        LHFloorLighting::ConfigurePostProcess(It->Settings,Style);
    FActorSpawnParameters Params; Params.Name=TEXT("FloorAtmosphere");
    Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    auto* Atmosphere=World->SpawnActor<ALHFloorAtmosphere>(FVector::ZeroVector,FRotator::ZeroRotator,Params);
    if(!Atmosphere) return false;
    Atmosphere->Configure(Style);
    int32 Fixtures=0, Fills=0;
    for(TActorIterator<ALHVisualPiece> It(World);It;++It)
        if(It->FindComponentByClass<ULHFloorFixtureLightComponent>()) ++Fixtures;
    for(TActorIterator<APointLight> It(World);It;++It)
        if(It->Tags.Contains(TEXT("LH.Lighting.RoomFill"))) ++Fills;
    UE_LOG(LogTemp,Display,TEXT("W5-19 lighting style=%s fixtures=%d room_fills=%d player_fills=1 EV100=%.2f"),
        *UEnum::GetValueAsString(Style),Fixtures,Fills,Profile->EV100);
    return true;
}
}
