#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Components/SceneComponent.h"
#include "LHNpcVisual.generated.h"
class USkeletalMesh;
class USkeletalMeshComponent;
class UAnimSequence;
class UMaterialInterface;
UCLASS()
class LIGHTHAVEN_API ULHNpcAppearance : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere) FName Identity;
    UPROPERTY(EditAnywhere) float Stature=1;
    // Real GLB assembly bounds, already scaled to roster stature (centimetres).
    UPROPERTY(EditAnywhere) FVector SourceMin=FVector::ZeroVector;
    UPROPERTY(EditAnywhere) FVector SourceMax=FVector::ZeroVector;
    UPROPERTY(EditAnywhere) TArray<TObjectPtr<USkeletalMesh>> Meshes;
    // Flattened in mesh slot order.
    UPROPERTY(EditAnywhere) TArray<TObjectPtr<UMaterialInterface>> Materials;
    UPROPERTY(EditAnywhere) TObjectPtr<UAnimSequence> Idle;
    UPROPERTY(EditAnywhere) TObjectPtr<UAnimSequence> Gesture;
};
UCLASS()
class LIGHTHAVEN_API ULHNpcVisualComponent : public USceneComponent
{
    GENERATED_BODY()
public:
    ULHNpcVisualComponent();
    void Refresh();
    bool UsesImportedCharacter() const { return !Parts.IsEmpty(); }
    UFUNCTION(BlueprintCallable, Category="Lighthaven|Art")
    static bool SharePlayerRig(USkeletalMesh* Mesh, USkeletalMesh* Player, UAnimSequence* Clip);
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    virtual void TickComponent(float Delta, ELevelTick Tick, FActorComponentTickFunction* Function) override;
private:
    UPROPERTY() TArray<TObjectPtr<ULHNpcAppearance>> Appearances;
    UPROPERTY(Transient) TArray<TObjectPtr<USkeletalMeshComponent>> Parts;
    UPROPERTY(Transient) TObjectPtr<ULHNpcAppearance> Active;
    bool bGesture=false;
    float Clock=0;
};
