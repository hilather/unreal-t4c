#include "Framework/LHPlayerController.h"
#include "Framework/LHInteractionReach.h"
#include "Framework/LHSessionSubsystem.h"
#include "UI/LHFrontendWidget.h"
#include "Rewards/LHEncounterLifecycle.h"
#include "UI/LHGameplayHud.h"
#include "Framework/LHCharacter.h"
#include "Framework/LHPlayerState.h"
#include "Framework/LHEnemyCharacter.h"
#include "Framework/LHDevCombatFixture.h"
#include "Abilities/LHCombatComponent.h"
#include "Abilities/LHAbilityCatalog.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "World/LHWorldMarkers.h"
#include "World/LHWorldTravelSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/CoreDelegates.h"
#include "UnrealClient.h"
#include "InputKeyEventArgs.h"
ALHPlayerController::ALHPlayerController() { bShowMouseCursor=true; PrimaryActorTick.bTickEvenWhenPaused=true; bShouldPerformFullTickWhenPaused=true; }
void ALHPlayerController::BeginPlay()
{
    Super::BeginPlay();
    DeactivateHandle=FCoreDelegates::ApplicationWillDeactivateDelegate.AddUObject(this,&ALHPlayerController::ApplicationDeactivated);
    ReactivateHandle=FCoreDelegates::ApplicationHasReactivatedDelegate.AddUObject(this,&ALHPlayerController::ApplicationReactivated);
    EstablishGameplayInput();
}
void ALHPlayerController::EndPlay(const EEndPlayReason::Type Reason)
{
    CloseJournal();
    if(HudWidget && GetWorld() && GetWorld()->GetGameViewport()) GetWorld()->GetGameViewport()->RemoveViewportWidgetContent(HudWidget.ToSharedRef());
    HudWidget.Reset();
    if (LiveSession) LiveSession->Resume=nullptr;
    JournalPresenter.Reset(); LiveSession.Reset();
    FCoreDelegates::ApplicationWillDeactivateDelegate.Remove(DeactivateHandle);
    FCoreDelegates::ApplicationHasReactivatedDelegate.Remove(ReactivateHandle);
    if (auto* State=GetPlayerState<ALHPlayerState>()) State->ClearAvatar();
    ClearHeldMovement();
    Super::EndPlay(Reason);
}
void ALHPlayerController::OnPossess(APawn* Pawn)
{
    Super::OnPossess(Pawn);
    if (!JournalWidget) EstablishGameplayInput();
    if (auto* State=GetPlayerState<ALHPlayerState>())
    {
        auto* Subsystem=GetGameInstance()?GetGameInstance()->GetSubsystem<ULHSessionSubsystem>():nullptr;
        auto Session=Subsystem?Subsystem->Session():nullptr;
        if (Session && Session->HasCharacter())
        {
            // Bind reconstructs canonical authority and GAS before actor info. Never run the dev resource fixture here.
            if (!Session->Bind(State)) return;
            LiveSession=Session; LiveSession->bInGameplay=true;
            LiveSession->Resume=[this]() { bCloseJournalRequested=true; };
            if(!JournalPresenter) JournalPresenter=MakeShared<FLHUIPresenter>(*Session,*Session,*Session);
            if(auto* Viewport=GetWorld()->GetGameViewport(); Viewport && !HudWidget)
            {
                SAssignNew(HudWidget,SLHGameplayHud).Presenter(JournalPresenter.Get());
                HudWidget->SetVisibility(EVisibility::HitTestInvisible);
                Viewport->AddViewportWidgetContent(HudWidget.ToSharedRef(),10);
            }
        }
        else LHDevCombat::InitializeForMap(State->GetCombatComponent(), Pawn);
        State->InitializeAvatar(Pawn);
    }
    if (LiveSession && !LiveSession->bWorldTravelFrozen)
    {
        FString Error;
        if (!GetGameInstance()->GetSubsystem<ULHSessionSubsystem>()->PlaceSessionArrival(GetWorld(),Error))
        {
            LiveSession->AbortGameplayArrival(Error);
            UE_LOG(LogTemp,Warning,TEXT("Session arrival refused: %s"),*Error);
            UGameplayStatics::OpenLevel(GetGameInstance(),TEXT("/Game/Lighthaven/Maps/L_Frontend"));
        }
    }
    ClearHeldMovement();
}
void ALHPlayerController::OnUnPossess()
{
    if (auto* State=GetPlayerState<ALHPlayerState>()) State->ClearAvatar();
    ClearHeldMovement(); SelectedTarget.Reset(); Super::OnUnPossess();
}
void ALHPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    if (!InputConfig) { InputConfig=NewObject<ULHInputConfig>(this); InputConfig->Initialize(); }
    auto* E=Cast<UEnhancedInputComponent>(InputComponent);
    if (!ensureMsgf(E,TEXT("Integrator must configure EnhancedInputComponent as the default input component"))) return;
    E->BindAction(InputConfig->Action("Move"),ETriggerEvent::Triggered,this,&ALHPlayerController::Move);
    E->BindAction(InputConfig->Action("Move"),ETriggerEvent::Completed,this,&ALHPlayerController::StopMove);
    E->BindAction(InputConfig->Action("Move"),ETriggerEvent::Canceled,this,&ALHPlayerController::StopMove);
    E->BindAction(InputConfig->Action("Look"),ETriggerEvent::Triggered,this,&ALHPlayerController::Look);
    E->BindAction(InputConfig->Action("Zoom"),ETriggerEvent::Triggered,this,&ALHPlayerController::Zoom);
#define LH_BIND(Name, Method) E->BindAction(InputConfig->Action(Name),ETriggerEvent::Started,this,&ALHPlayerController::Method)
    LH_BIND("ToggleRun",ToggleRun); LH_BIND("Attack",Attack); LH_BIND("Interact",Interact);
    LH_BIND("TargetNext",NextTarget); LH_BIND("TargetPrev",PrevTarget); LH_BIND("CancelTarget",CancelTarget);
    LH_BIND("OpenCharacter",OpenCharacter); LH_BIND("OpenInventory",OpenInventory); LH_BIND("Pause",PauseMenu);
    LH_BIND("OpenAbilities",OpenAbilities); LH_BIND("Hotbar1",Hotbar1); LH_BIND("Hotbar2",Hotbar2); LH_BIND("Hotbar3",Hotbar3); LH_BIND("Hotbar4",Hotbar4); LH_BIND("Hotbar5",Hotbar5); LH_BIND("Hotbar6",Hotbar6); LH_BIND("HotbarItem",HotbarItem);
    LH_BIND("Confirm",Confirm); LH_BIND("Back",Back);
#undef LH_BIND
    E->BindAction(InputConfig->Action("Navigate"),ETriggerEvent::Triggered,this,&ALHPlayerController::Navigate);
    E->BindAction(InputConfig->Action("SelectMouse"),ETriggerEvent::Started,this,&ALHPlayerController::SelectMouse);
    SetControlContext(ActiveContext);
}
void ALHPlayerController::ClearHeldMovement()
{
    // Do not erase physical key state: a flushed held key looks released next tick.
    Movement.Clear(); bAwaitMoveRelease=true;
    if (auto* C=Cast<ALHCharacter>(GetPawn())) { C->ConsumeMovementInputVector(); C->GetCharacterMovement()->StopMovementImmediately(); }
}
bool ALHPlayerController::MovementInputsReleased() const
{
    return FlushedMovementKeys.IsEmpty() && !IsInputKeyDown(EKeys::W) && !IsInputKeyDown(EKeys::A)
        && !IsInputKeyDown(EKeys::S) && !IsInputKeyDown(EKeys::D)
        && FMath::Abs(GetInputAnalogKeyState(EKeys::Gamepad_LeftX))<0.2f
        && FMath::Abs(GetInputAnalogKeyState(EKeys::Gamepad_LeftY))<0.2f;
}
void ALHPlayerController::FlushPressedKeys()
{
    // Engine focus-loss flushing also manufactures neutral state. Remember affected
    // keys/axes until an actual device release event, including after focus regain.
    for (const FKey& Key : {EKeys::W,EKeys::A,EKeys::S,EKeys::D})
        if (IsInputKeyDown(Key)) FlushedMovementKeys.Add(Key);
    for (const FKey& Key : {EKeys::Gamepad_LeftX,EKeys::Gamepad_LeftY})
        if (FMath::Abs(GetInputAnalogKeyState(Key))>=0.2f) FlushedMovementKeys.Add(Key);
    ClearHeldMovement();
    Super::FlushPressedKeys();
}
bool ALHPlayerController::InputKey(const FInputKeyEventArgs& Params)
{
    if (Params.Event==IE_Released || (Params.Event==IE_Axis && FMath::Abs(Params.AmountDepressed)<0.2f))
        FlushedMovementKeys.Remove(Params.Key);
    return Super::InputKey(Params);
}
void ALHPlayerController::ApplicationDeactivated()
{
    bApplicationActive=false; bHadFocus=false; ClearHeldMovement();
}
void ALHPlayerController::ApplicationReactivated()
{
    bApplicationActive=true;
    SetControlContext(ActiveContext);
}
void ALHPlayerController::SetControlContext(ELHInputContext Context)
{
    ClearHeldMovement(); Movement.SwitchContext(Context); ActiveContext=Context;
    if (!InputConfig) { InputConfig=NewObject<ULHInputConfig>(this); InputConfig->Initialize(); }
    if (auto* Local=GetLocalPlayer()) if (auto* S=Local->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
    {
        for (int32 I=0; I<3; ++I) S->RemoveMappingContext(InputConfig->Context(ELHInputContext(I)));
        FModifyContextOptions Options; Options.bIgnoreAllPressedKeysUntilRelease=true;
        S->AddMappingContext(InputConfig->Context(Context),0,Options);
    }
}
void ALHPlayerController::PlayerTick(float DeltaSeconds)
{
    Super::PlayerTick(DeltaSeconds);
    if (bCloseJournalRequested) { bCloseJournalRequested=false; CloseJournal(); }
    if (LiveSession) LiveSession->Flush();
    if (JournalPresenter) { JournalPresenter->Refresh(); if(JournalPresenter->Hud().bDead && !JournalWidget) OpenScreen("Death"); }
    if (!HasLiveMovementAvatar()) { ClearHeldMovement(); return; }
    auto* V=GetWorld() ? GetWorld()->GetGameViewport() : nullptr;
    // Slate viewport keyboard focus can be valid while SDL native foreground
    // bookkeeping is stale after a Wayland workspace switch. No viewport exists
    // in transient automation worlds; those still exercise the real input tick.
    const bool Focus=bApplicationActive && (!V || (V->Viewport &&
        (V->Viewport->HasFocus() || V->Viewport->IsForegroundWindow())));
    if (!Focus) { if (bHadFocus) ClearHeldMovement(); bHadFocus=false; return; }
    if (!bHadFocus) { SetControlContext(ActiveContext); bHadFocus=true; }
    if (bAwaitMoveRelease && MovementInputsReleased()) bAwaitMoveRelease=false;
    if (SelectedTarget.IsValid() && !ValidTarget(SelectedTarget.Get())) SelectedTarget.Reset();
    if (ActiveContext==ELHInputContext::Gameplay) if (auto* C=Cast<ALHCharacter>(GetPawn()))
    {
        auto* State=GetPlayerState<ALHPlayerState>();
        if (!LiveSession && State && State->GetCombatComponent()->IsActionPending()) { C->ConsumeMovementInputVector(); return; }
        const FRotator Yaw(0,C->CameraYaw(),0);
        C->AddMovementInput(Yaw.Vector(),Movement.Held.Y);
        C->AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y),Movement.Held.X);
    }
}
void ALHPlayerController::Move(const FInputActionValue& V) { SubmitMovement(V.Get<FVector2D>()); }
bool ALHPlayerController::HasLiveMovementAvatar() const
{
    const auto* State=GetPlayerState<ALHPlayerState>();
    const auto* Combat=State ? State->GetCombatComponent() : nullptr;
    if(LiveSession) return !LiveSession->IsBlocked() && GetPawn();
    return GetPawn() && Combat && Combat->IsAlive() && Combat->GetAvatarActor()==GetPawn();
}
void ALHPlayerController::SubmitMovement(FVector2D Axis)
{
    if (!HasLiveMovementAvatar()) { ClearHeldMovement(); return; }
    if (Axis.IsNearlyZero() && MovementInputsReleased()) bAwaitMoveRelease=false;
    auto* State=GetPlayerState<ALHPlayerState>();
    if (!LiveSession && State && State->GetCombatComponent()->IsActionPending()) { ClearHeldMovement(); return; }
    if (bHadFocus && !bAwaitMoveRelease && ActiveContext==ELHInputContext::Gameplay)
        Movement.Held=Axis.GetClampedToMaxSize(1);
}
void ALHPlayerController::StopMove(const FInputActionValue&) { Movement.Clear(); }
void ALHPlayerController::Look(const FInputActionValue& V)
{
    if (!bHadFocus || ActiveContext!=ELHInputContext::Gameplay) return;
    if (auto* C=Cast<ALHCharacter>(GetPawn()))
    {
        // Mouse2D only contributes while middle dragging; right stick remains independent.
        FVector2D D=V.Get<FVector2D>();
        const FVector2D Stick(GetInputAnalogKeyState(EKeys::Gamepad_RightX),GetInputAnalogKeyState(EKeys::Gamepad_RightY));
        D=IsInputKeyDown(EKeys::MiddleMouseButton) ? D : Stick;
        C->RotateCamera(D*GetWorld()->GetDeltaSeconds()*90.f);
    }
}
void ALHPlayerController::Zoom(const FInputActionValue& V) { if (!bHadFocus || ActiveContext!=ELHInputContext::Gameplay) return; if (auto* C=Cast<ALHCharacter>(GetPawn())) { const bool Pad=IsInputKeyDown(EKeys::Gamepad_DPad_Up) || IsInputKeyDown(EKeys::Gamepad_DPad_Down); C->ZoomCamera(V.Get<float>()*(Pad ? GetWorld()->GetDeltaSeconds()*5.f : 1.f)); } }
void ALHPlayerController::ToggleRun() { if (auto* C=Cast<ALHCharacter>(GetPawn())) C->ToggleRun(); }
float ALHPlayerController::EffectiveSelectionRange() const
{
    float Range=SelectionRange;
    if(LiveSession && JournalPresenter) for(const auto& Ability:JournalPresenter->AbilityCatalog())
        if(const auto* Row=LHAbilities::Find(Ability.Id); Row && Row->RangeCm.Resolution==ELHValueResolution::Resolved && FMath::IsFinite(Row->RangeCm.Value)) Range=FMath::Max(Range,float(Row->RangeCm.Value));
    return Range;
}
bool ALHPlayerController::ValidTarget(AActor* A) const
{
    if (!IsValid(A) || A==GetPawn() || !GetPawn()) return false;
    if (auto* Enemy=Cast<ALHEnemyCharacter>(A))
    {
        auto* State=GetPlayerState<ALHPlayerState>();
        if (!State) return false;
        if(LiveSession) { return Enemy->IsAlive() && FVector::DistSquared(A->GetActorLocation(),GetPawn()->GetActorLocation())<=FMath::Square(EffectiveSelectionRange()) && LineOfSightTo(A); }
        // Reuse authority's life/range/LOS checks. Temporary action/resource rejection
        // does not remove a selectable target; submission still validates everything.
        const auto Reason=State->GetCombatComponent()->ValidateAttack(Enemy->GetCombatComponent());
        if (Reason!=ELHCommandReason::None && Reason!=ELHCommandReason::ActiveAction &&
            Reason!=ELHCommandReason::Cooldown && Reason!=ELHCommandReason::InsufficientMana) return false;
        if (!Enemy->GetCombatComponent()->IsAlive()) return false;

    }
    else
    {
        if (!A->Implements<ULHControlTarget>()) return false;
        if (!ILHControlTarget::Execute_IsControlTargetAlive(A) || !ILHControlTarget::Execute_IsControlTargetReachable(A,GetPawn())) return false;
    }
    if (FVector::DistSquared(A->GetActorLocation(),GetPawn()->GetActorLocation())>FMath::Square(EffectiveSelectionRange())) return false;
    FHitResult Hit; FCollisionQueryParams Params; Params.AddIgnoredActor(GetPawn());
    const bool Blocked=GetWorld()->LineTraceSingleByChannel(Hit,GetPawn()->GetActorLocation(),A->GetActorLocation(),ECC_Visibility,Params);
    return !Blocked || Hit.GetActor()==A;
}
void ALHPlayerController::Cycle(int32 Direction)
{
    if (ActiveContext!=ELHInputContext::Gameplay) return;
    TArray<FLHTargetCandidate> Candidates; TMap<FString,AActor*> Actors;
    for (TActorIterator<AActor> It(GetWorld()); It; ++It) if (ValidTarget(*It))
    {
        const FString Key=It->GetPathName(); Actors.Add(Key,*It);
        Candidates.Add({Key,FVector::DistSquared(It->GetActorLocation(),GetPawn()->GetActorLocation()),true,true,true});
    }
    const auto Order=LHControls::OrderedTargets(Candidates,FMath::Square(EffectiveSelectionRange()));
    const FString Key=LHControls::CycleTarget(Order,SelectedTarget.IsValid() ? SelectedTarget->GetPathName() : FString(),Direction);
    SelectedTarget=Actors.FindRef(Key);
}
void ALHPlayerController::NextTarget() { Cycle(1); }
void ALHPlayerController::PrevTarget() { Cycle(-1); }
void ALHPlayerController::CancelTarget() { SelectedTarget.Reset(); }
void ALHPlayerController::SelectMouse()
{
    if (ActiveContext!=ELHInputContext::Gameplay || !bHadFocus) return;
    FHitResult Hit; GetHitResultUnderCursor(ECC_Visibility,false,Hit);
    SelectedTarget=ValidTarget(Hit.GetActor()) ? Hit.GetActor() : nullptr;
}
bool ALHPlayerController::SelectTarget(AActor* Target)
{
    SelectedTarget=ActiveContext==ELHInputContext::Gameplay && ValidTarget(Target) ? Target : nullptr;
    return SelectedTarget.IsValid();
}
ELHCommandReason ALHPlayerController::RequestSelectedAttack()
{
    if (LiveSession)
    {
        if(LiveSession->IsBlocked() || ActiveContext!=ELHInputContext::Gameplay) return ELHCommandReason::Busy;
        if(!JournalPresenter) return ELHCommandReason::UnresolvedRules;
        auto* Enemy=Cast<ALHEnemyCharacter>(SelectedTarget.Get());
        const auto Result=JournalPresenter->UseAbility(Enemy?Enemy->GetEntityId(LiveSession->Snapshot().World.RunId):FLHEntityId{});
        return Result.Reason;
    }
    if (ActiveContext!=ELHInputContext::Gameplay) return ELHCommandReason::InvalidRequest;
    auto* State=GetPlayerState<ALHPlayerState>();
    auto* Enemy=Cast<ALHEnemyCharacter>(SelectedTarget.Get());
    if (!State || !Enemy)
    {
        UE_LOG(LogTemp, Display, TEXT("LH attack request: NotFound (no selected combat target)"));
        return ELHCommandReason::NotFound;
    }
    const auto Reason=State->GetCombatComponent()->RequestBasicAttack(Enemy->GetCombatComponent());
    UE_LOG(LogTemp, Display, TEXT("LH attack request: %s"), *StaticEnum<ELHCommandReason>()->GetNameStringByValue(static_cast<int64>(Reason)));
    if (Reason==ELHCommandReason::None) { ClearHeldMovement(); OnAttackRequested.Broadcast(Enemy); }
    return Reason;
}
void ALHPlayerController::Attack() { RequestSelectedAttack(); }
// Wave 2+: intent notification only, no interaction transaction exists yet.
void ALHPlayerController::Interact()
{
    if (LiveSession && !LiveSession->IsBlocked() && ActiveContext==ELHInputContext::Gameplay && GetPawn())
    {
        ALHPortal* Nearest=nullptr; double Distance=250.0*250.0;
        for (TActorIterator<ALHPortal> It(GetWorld());It;++It)
        {
            const double D=FVector::DistSquaredXY(It->GetActorLocation(),GetPawn()->GetActorLocation());
            if (D<=Distance && LHInteractionReach::Contains(GetPawn()->GetActorLocation(),It->GetActorLocation())) { Nearest=*It; Distance=D; }
        }
        if (Nearest)
        {
            FHitResult Hit; FCollisionQueryParams Params; Params.AddIgnoredActor(GetPawn());
            if (GetWorld()->LineTraceSingleByChannel(Hit,GetPawn()->GetActorLocation(),Nearest->GetActorLocation()+FVector(0,0,60),ECC_Visibility,Params) && Hit.GetActor()!=Nearest) return;
            const auto S=LiveSession->Snapshot(); FLHRequestTravelRequest Q;
            Q.Request.Epoch=S.Session.RequestEpoch; Q.Request.Value=FGuid::NewGuid();
            Q.Portal=Nearest->Materialize(S.World.RunId); Q.Destination=Nearest->Destination;
            LiveSession->Execute(Q); ClearHeldMovement(); return;
        }
    }
    if(LiveSession && !LiveSession->IsBlocked() && ActiveContext==ELHInputContext::Gameplay && GetPawn())
    {
        auto Visible=[this](AActor* A){ FHitResult H; FCollisionQueryParams P; P.AddIgnoredActor(GetPawn()); return !GetWorld()->LineTraceSingleByChannel(H,GetPawn()->GetActorLocation(),A->GetActorLocation()+FVector(0,0,60),ECC_Visibility,P) || H.GetActor()==A; };
        ALHInteractableMarker* Npc=nullptr; double Best=250*250;
        for(TActorIterator<ALHInteractableMarker> It(GetWorld());It;++It) { const double D=FVector::DistSquaredXY(It->GetActorLocation(),GetPawn()->GetActorLocation()); if(It->DefinitionId.Value.ToString().StartsWith(TEXT("NPC.")) && D<=Best && LHInteractionReach::Contains(GetPawn()->GetActorLocation(),It->GetActorLocation()) && Visible(*It)) { Best=D; Npc=*It; } }
        if(Npc) { ShowTargetScreen("Dialogue",Npc->Materialize(LiveSession->Snapshot().World.RunId)); return; }
        ALHEnemyCharacter* Corpse=nullptr; Best=200*200;
        for(TActorIterator<ALHEnemyCharacter> It(GetWorld());It;++It) { const double D=FVector::DistSquared(It->GetActorLocation(),GetPawn()->GetActorLocation()); if(It->IsCorpse() && D<=Best && Visible(*It)) { Best=D; Corpse=*It; } }
        if(Corpse) { ShowTargetScreen("Loot",LHRewards::CorpseContainerFor(LiveSession->Snapshot().World.RunId,Corpse->GetLife())); return; }
        return;
    }
    if (!SelectedTarget.IsValid()) Cycle(1);
    if ((!LiveSession || !LiveSession->IsBlocked()) && ActiveContext==ELHInputContext::Gameplay && ValidTarget(SelectedTarget.Get())) OnInteractRequested.Broadcast(SelectedTarget.Get());
}
void ALHPlayerController::OpenScreen(FName Screen)
{
    if(LiveSession && GetPlayerState<ALHPlayerState>() && GetPlayerState<ALHPlayerState>()->GetCombatComponent()->IsActionPending()) return;
    SetControlContext(ELHInputContext::UI); OnScreenRequested.Broadcast(Screen);
    if (!JournalPresenter || !GetWorld()->GetGameViewport()) return;
    if (GetPlayerState<ALHPlayerState>()->GetCombatComponent()->IsActionPending()) return;
    if (!JournalWidget)
    {
        SAssignNew(JournalWidget,SLHFrontendWidget).Presenter(JournalPresenter.Get());
        GetWorld()->GetGameViewport()->AddViewportWidgetContent(JournalWidget.ToSharedRef(),100);
    }
    const ELHUIScreen Destination=Screen==TEXT("Inventory")?ELHUIScreen::Inventory:Screen==TEXT("Abilities")?ELHUIScreen::Hud:Screen==TEXT("Pause")?ELHUIScreen::Pause:Screen==TEXT("Death")?ELHUIScreen::Death:Screen==TEXT("Dialogue")?ELHUIScreen::Dialogue:Screen==TEXT("Loot")?ELHUIScreen::Loot:ELHUIScreen::CharacterSheet;
    JournalWidget->Open(Destination);
    JournalPresenter->SetPaused(true);
    SetPause(true);
    FInputModeUIOnly Mode; Mode.SetWidgetToFocus(JournalWidget); SetInputMode(Mode);
}
void ALHPlayerController::CloseJournal()
{
    if (JournalWidget && GetWorld() && GetWorld()->GetGameViewport()) GetWorld()->GetGameViewport()->RemoveViewportWidgetContent(JournalWidget.ToSharedRef());
    JournalWidget.Reset(); if(JournalPresenter) JournalPresenter->SetPaused(false); SetPause(false); EstablishGameplayInput();
}
void ALHPlayerController::OpenCharacter() { OpenScreen("Character"); }
void ALHPlayerController::OpenInventory() { OpenScreen("Inventory"); }
void ALHPlayerController::OpenAbilities() { OpenScreen("Abilities"); }
void ALHPlayerController::PauseMenu() { OpenScreen("Pause"); }
void ALHPlayerController::Navigate(const FInputActionValue& V) { const auto D=V.Get<FVector2D>(); OnMenuInput.Broadcast(FMath::Abs(D.Y)>FMath::Abs(D.X) ? (D.Y>0 ? "Up" : "Down") : (D.X>0 ? "Right" : "Left")); }
void ALHPlayerController::Confirm() { OnMenuInput.Broadcast("Confirm"); }
void ALHPlayerController::Back() { OnMenuInput.Broadcast("Back"); }

void ALHPlayerController::EstablishGameplayInput()
{
    FInputModeGameOnly Mode; Mode.SetConsumeCaptureMouseDown(false); SetInputMode(Mode);
    bShowMouseCursor=true;
    // The viewport survives map travel. Reset it even before a Slate viewport is ready.
    if (auto* Viewport=GetWorld()?GetWorld()->GetGameViewport():nullptr)
    {
        Viewport->SetIgnoreInput(false);
        Viewport->SetMouseCaptureMode(EMouseCaptureMode::CaptureDuringMouseDown);
        Viewport->SetMouseLockMode(EMouseLockMode::LockOnCapture);
    }
    SetControlContext(ELHInputContext::Gameplay);
}

void ALHPlayerController::ShowTargetScreen(FName Screen,const FLHEntityId& Target)
{
    if(!JournalPresenter) return;
    JournalPresenter->OpenTarget(Screen=="Dialogue"?ELHUIScreen::Dialogue:ELHUIScreen::Loot,Target);
    OpenScreen(Screen);
}
void ALHPlayerController::Hotbar1() { if(JournalPresenter) JournalPresenter->SelectAbility(0); }
void ALHPlayerController::Hotbar2() { if(JournalPresenter) JournalPresenter->SelectAbility(1); }
void ALHPlayerController::Hotbar3() { if(JournalPresenter) JournalPresenter->SelectAbility(2); }
void ALHPlayerController::Hotbar4() { if(JournalPresenter) JournalPresenter->SelectAbility(3); }
void ALHPlayerController::Hotbar5() { if(JournalPresenter) JournalPresenter->SelectAbility(4); }
void ALHPlayerController::Hotbar6() { if(JournalPresenter) JournalPresenter->SelectAbility(5); }
void ALHPlayerController::HotbarItem() { if(JournalPresenter && LiveSession && !LiveSession->IsBlocked()) JournalPresenter->UseHotbarItem(); }
