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
namespace LHPlayerVisual
{
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
    void Committed(const FLHAttackEvent& Event);
    void Finished(const FLHAttackEvent& Event, ELHAttackOutcome Outcome);
    void Unbind();
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
