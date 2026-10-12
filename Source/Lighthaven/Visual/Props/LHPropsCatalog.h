#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LHPropsCatalog.generated.h"
class UStaticMesh;
class UTexture2D;
// Imported package contains hard references to every prop, icon and material dependency.
UCLASS()
class LIGHTHAVEN_API ULHPropsAssets : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere) TMap<FName,TObjectPtr<UStaticMesh>> Meshes;
    UPROPERTY(EditAnywhere) TMap<FName,TObjectPtr<UTexture2D>> Icons;
};
UCLASS()
class LIGHTHAVEN_API ULHPropsCatalog : public UObject
{
    GENERATED_BODY()
public:
    ULHPropsCatalog();
    UPROPERTY() TObjectPtr<ULHPropsAssets> Assets;
    static UTexture2D* Icon(FName Id);
    static UStaticMesh* Mesh(FName Id);
    static TArray<UStaticMesh*> Effects(FName Ability);
    static const TArray<FName>& ItemIds();
    static const TArray<FName>& AbilityIds();
};
