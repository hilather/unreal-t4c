# W1-01 controls and camera

Native controls candidate, contract revision 1. No binary input assets created. `ULHInputConfig::Initialize` is the single runtime factory to replace with authored assets later; construction is idempotent and contexts/actions are GC-owned. Gameplay uses Enhanced Input; UI and Creation are separate navigation contexts without movement/attack. Request events are deliberately unconnected seams because combat-foundation.md and framework combat code are absent on base fefe25de9545d276c2dee5da7b0428e3db11d115.

| Semantic action | Keyboard/mouse | Gamepad |
|---|---|---|
| Move | WASD | Left stick |
| Look / RotateCamera | Middle-drag | Right stick |
| Zoom | Wheel | D-pad up/down |
| ToggleRun | Left Shift | L3 |
| Interact | E | South |
| Attack | Right mouse | Right trigger |
| TargetNext | Tab | RB |
| TargetPrev | Q | LB |
| CancelTarget | F | R3 |
| OpenCharacter | C | North |
| OpenInventory | I | View |
| Pause | Escape | Menu |

Left click selects by visibility hit-test. UI/Creation: arrows, D-pad or left stick navigate; Enter/South confirms; Escape/East backs out. Navigation dispatches semantic direction events; widget focus and repeat timing belong to the UI owner.

Assumed mapping departures from V-02: Q is previous target (Shift is run, avoiding Shift+Tab conflict); unchorded D-pad zoom; F/R3 explicitly cancel rather than toggle lock. Character has a direct North binding in addition to journal tabs. Quick slots, contextual chord zoom and spells are future UI/ability integration, not implemented here.

All tuning is **Prototype**, authored from 01-game-design §5 and layout/README.md (V-01), not historical mechanics or verified play: walk/run 220/450 cm/s; capsule radius 35 cm and half-height 90 cm; boom 1200 cm bounded 900–1800; default yaw 45° bounded 0–90° (±45°); pitch −55° bounded −60…−50°; focus at capsule center (~90 cm above floor); spring-arm collision probe 12 cm; rotation rate 90°/s per unit; zoom wheel 100 cm/unit, held D-pad 500 cm/s; selection range 2000 cm. Camera FOV is explicitly 45° **horizontal** under engine defaults; V-01's proposed vertical FOV awaits visual/aspect-axis review. No source tile conversion is implied.

Movement uses CharacterMovement and normalized analog input in camera-yaw space; run is a toggle. No root-motion locomotion asset or teleport movement exists. Run movement restrictions during attack/cast must be imposed by the combat commitment owner, not by speculative controller effect code.

Targets opt into `ILHControlTarget` and provide authoritative alive and reachable answers; absent/unknown providers fail closed. Candidate requires a valid actor distinct from avatar, inclusive range and unobstructed avatar-center→target-center Visibility trace (first hit may be target). Mouse additionally requires first cursor hit. Reachable means provider-approved navigational accessibility; this is not inferred from distance/line of sight. Cycle sorts by distance then actor path for deterministic session-local ties, wraps both directions, acquires nearest on Next when selection is missing; Previous from missing chooses last. Dead, obstructed, unreachable, destroyed or out-of-range targets are excluded and selection revalidated each tick and request. Actors/paths are transient presentation references, never save identities. Integrator must translate target to canonical stable ID and revalidate in authority.

`OnAttackRequested` / `OnInteractRequested` submit intent only. They do not apply effects, assign abilities, mint canonical requests or modify HP/XP. `OnScreenRequested` changes to UI first; session/UI owner must pause simulation, focus the screen and explicitly restore Gameplay via `SetControlContext` when safe. `OnMenuInput` routes navigation/Confirm/Back. Clearing consumes pending movement input, zeros held axes and stops CharacterMovement while retaining physical PlayerInput state. Every context change, application deactivation and unpossess clears; foreground-window polling handles viewport loss. Remapping ignores pressed boolean keys until release; a separate neutral-axis check gates movement (stick neutral threshold 0.2, Prototype). Host must check PIE/standalone viewport behavior and release gating with a real controller.

## Integrator requests

- GameMode must select `ALHCharacter` and `ALHPlayerController`; no GameMode edit is owned here.
- Configure EnhancedPlayerInput and EnhancedInputComponent defaults in Config (or approved framework initializer). Current controller reports an ensure and declines binding if its input component is not Enhanced Input. **Required build fix:** add `"InputCore"` to Lighthaven.Build.cs dependency modules. EnhancedInput is present, but direct `EKeys`/`FKey` use requires InputCore linkage. The observed build links fail with undefined EKeys symbols. Build.cs is outside this worker scope; integrator owns this prerequisite.
- W1-03b binds request seams to request-attack/interaction authority, owns commitment movement inhibition and GAS avatar initialization. Providers must expose life/reachability from authority; future target registry may replace world scanning.
- W2-03/W4-05 bind screen/menu events, simulation pause, logical focus, held-button release, target feedback and device-disconnect pause. Character creation uses Creation context. No widget or actual pause implementation is claimed.

## W1-04 rooms and host validation

Dev_Movement: measure walk/run, diagonal speed, slopes, stairs, door clearances, ramp transitions; test mouse middle orbit/right stick, both orbit and pitch limits, min/max zoom, spring-arm wall collision, ceilings and camera-facing wall cutaways. Spring-arm collision alone does not implement roof/wall fading; world/presentation owner supplies it. Verify at both 720p/1080p.

Hold movement and attack while opening each menu, switching Creation/UI/Gameplay, alt-tabbing, losing viewport focus, unpossessing, pausing/resuming and changing device. Reenter with buttons still held; require release before movement/action. Test target death/removal, blocked line of sight, exact range edge, provider-unreachable candidates, equal-distance cycling/wrap, cursor ground click, and attack/interact validity changing between selection and submission. No selection auto-walk or damage should occur.

Coordinator command: `bash build/run-tests.sh Lighthaven.Controls`. Expected native tests: `InputConfigParity`, `ContextClearsMovement`, `TargetOrderAndFilter` under `Lighthaven.Controls`. Pure state tests cover the movement state used by context switch, not actual viewport/CharacterMovement behavior. No editor/tests executed by root worker; host execution and manual checks remain necessary.

## G1-FIX3 release and focus handling

Attack/context clears retain raw key/axis state. A zero semantic action (including
opposing keys or canceled mappings) cannot unlock movement while WASD is down
or the left stick exceeds the Prototype 0.2 neutral threshold. Engine-requested
focus flushing records held movement keys/axes until actual release/neutral input
events; repeat events cannot unlock them. If released outside the application and
no release event is delivered, press and release that key once after returning.
Pending-action movement events clear axes and reinstate release gating.

QA on Hyprland/Wayland at 0f1a975 reported native IsForegroundWindow remained
false until a click. Local UE 5.8.3 source shows FSceneViewport::IsForegroundWindow
 delegates to FLinuxWindow, which compares LinuxApplication's current active
window; LinuxApplication activates that window on mouse-down. This is consistent
with stale engine/SDL activation bookkeeping, amplified by the project's sole
native-foreground gate. This worker cannot reproduce a compositor session, so
the underlying Wayland event defect is not conclusively isolated.

The controller now accepts Slate viewport keyboard focus OR native foreground
status, bounded by application deactivation/reactivation delegates. Reactivation
reinstalls the current input context with held-button suppression. It never
forces OS focus or synthesizes clicks. Missing viewports are permitted only for
transient input-test worlds. Host must repeat workspace/alt-tab checks on Wayland
and X11; Windows remains deferred. If Slate focus and application activation also
remain stale, engine/compositor recovery still requires the click workaround.
Evidence: Engine/Source/Runtime/Engine/Private/Slate/SceneViewport.cpp HasFocus,
IsForegroundWindow; ApplicationCore/Private/Linux/LinuxWindow.cpp
IsForegroundWindow; LinuxApplication.cpp mouse-down ActivateWindow path.
