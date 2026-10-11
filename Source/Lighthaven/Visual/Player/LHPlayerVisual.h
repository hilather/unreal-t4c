#pragma once
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Core/LHSaveSnapshot.h"
#include "Visual/LHVisualKit.h"
#include "Abilities/LHCombatComponent.h"
#include "LHPlayerVisual.generated.h"

enum class ELHPlayerPose : uint8 { Idle, Walk, Run, Melee, Bow, Cast, Hit, Death };
struct LIGHTHAVEN_API FLHPlayerPart
{
    FName Name, Parent;
    FTransform Transform;
    FLHVisualRecipe Recipe;
};
class USkeletalMesh;
class USkeletalMeshComponent;
class UAnimSequence;
class UMaterialInterface;
namespace LHPlayerVisual
{
    LIGHTHAVEN_API bool ResolveAppearance(const FLHCharacterRecord& Character, int32& Body, int32& Hair, int32& Skin);
    LIGHTHAVEN_API FString AssetRoot(const FLHCharacterRecord& Character);
    LIGHTHAVEN_API FName Action(ELHPlayerPose Pose);
    LIGHTHAVEN_API FString MeshAssetPath(const FLHCharacterRecord& Character, FName Part);
    LIGHTHAVEN_API FString ActionAssetPath(const FLHCharacterRecord& Character, ELHPlayerPose Pose);
    // Pure presentation recipes. No random stream, rules mutation or collision.
    LIGHTHAVEN_API TArray<FLHPlayerPart> Build(const FLHCharacterRecord& Character);
    LIGHTHAVEN_API uint32 Fingerprint(const TArray<FLHPlayerPart>& Parts);
}
UCLASS()
class LIGHTHAVEN_API ULHPlayerVisualComponent : public USceneComponent
{
    GENERATED_BODY()
public:
    ULHPlayerVisualComponent();
    UFUNCTION(BlueprintCallable, Category="Lighthaven|Art")
    static bool ConfigureImportedMesh(USkeletalMesh* Mesh, UMaterialInterface* Clothing, UMaterialInterface* Skin);
    UFUNCTION(BlueprintCallable, Category="Lighthaven|Art")
    static bool CompactImportedMesh(USkeletalMesh* Mesh);
    bool UsesImportedBody() const { return ImportedBody != nullptr; }
    USkeletalMeshComponent* BodyMesh() const { return ImportedBody; }
    void Present(const FLHCharacterRecord* Character, ULHCombatComponent* Combat, float Speed, float Seconds);
    void SetPresentationEnabled(bool Enabled);
    ELHPlayerPose Pose() const { return CurrentPose; }
    USceneComponent* Socket(FName Name) const;
    const TArray<TObjectPtr<ALHVisualPiece>>& Pieces() const { return RenderPieces; }
    virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    void Rebuild(const TArray<FLHPlayerPart>& Parts);
    void Bind(ULHCombatComponent* Combat);
    void RebuildImported(const FLHCharacterRecord& Character);
    void SampleImported();
    void ClearImported();
    void Committed(const FLHAttackEvent& Event);
    void Finished(const FLHAttackEvent& Event, ELHAttackOutcome Outcome);
    void Unbind();
    UPROPERTY() TArray<TObjectPtr<USkeletalMesh>> BodyAssets;
    UPROPERTY() TArray<TObjectPtr<USkeletalMesh>> HairAssets;
    UPROPERTY() TArray<TObjectPtr<UAnimSequence>> ActionAssets;
    UPROPERTY() TArray<TObjectPtr<UMaterialInterface>> SkinAssets;
    UPROPERTY() TObjectPtr<USkeletalMeshComponent> ImportedBody;
    UPROPERTY() TObjectPtr<USkeletalMeshComponent> ImportedHair;
    int32 ImportedIndex=0;
    UPROPERTY() TArray<TObjectPtr<ALHVisualPiece>> RenderPieces;
    UPROPERTY() TArray<TObjectPtr<USceneComponent>> Anchors;
    TWeakObjectPtr<ULHCombatComponent> BoundCombat;
    FDelegateHandle CommitHandle, FinishHandle;
    FLHHitIdentity ActionIdentity;
    ELHPlayerPose CurrentPose=ELHPlayerPose::Idle, ActionPose=ELHPlayerPose::Idle;
    uint32 BuiltFingerprint=0;
    bool bBuilt=false, bEnabled=true, bAction=false, bWasAlive=true;
    float Phase=0, Clock=0, ActionAge=0, ImpactDelay=0, ReleaseRemaining=0, FlinchRemaining=0, DeathAge=0, PreviousHealth=-1;
};
