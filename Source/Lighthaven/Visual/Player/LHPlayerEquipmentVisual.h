#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/LHSaveSnapshot.h"
#include "LHPlayerEquipmentVisual.generated.h"
class ULHPlayerVisualComponent;
class UStaticMeshComponent;
class ULHCombatComponent;
UCLASS()
class LIGHTHAVEN_API ULHPlayerEquipmentVisual : public UActorComponent
{
    GENERATED_BODY()
public:
    void Present(ULHPlayerVisualComponent* Visual,const FLHCharacterRecord& Character,ULHCombatComponent* Combat,bool Enabled,bool Drawing);
    void SetEnabled(bool Enabled);
    virtual void OnComponentDestroyed(bool DestroyingHierarchy) override;
    static FName Equipped(const FLHCharacterRecord& Character,ELHEquipmentSlot Slot);
private:
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Meshes;
};
