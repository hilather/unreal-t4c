# W1-INT integration candidate

GameMode selects LHCharacter, LHPlayerController, LHPlayerState and LHGameState.
Project startup selects Dev_Combat; packaging lists Dev_Combat and Dev_Movement.
Enhanced Input defaults select EnhancedPlayerInput/EnhancedInputComponent;
InputCore was already present at the base revision.

Possession installs explicit dev-map resources (only in generated dev worlds),
then initializes PlayerState-owned GAS actor info with the pawn as avatar.
LHCharacter forwards IAbilitySystemInterface to PlayerState. Unpossession,
controller teardown and combat death clear the avatar/cancel pending actions.
Canonical restore must install canonical attributes first and call InitializeAvatar;
the dev initializer is not a save/restore path.

Attack input and RequestSelectedAttack share the native RequestBasicAttack entry
point. Authority validates/commits, schedules the explicit impact and applies
one damage; controller never writes health or calls ResolveImpact. Successful
requests clear movement, and pending actions suppress new pawn movement each
tick. OnAttackRequested now reports accepted submission; consumers must not
apply another attack. Command rejection and authoritative hit/damage output
are printed in the Output Log. Interact is an intent-only Wave 2+ stub.
No canonical request/target IDs, replay, loot, XP or persistence are implemented.

Enemy selection uses combat ValidateAttack plus independent life, selection
range and visibility checks. Cooldown, pending action and insufficient mana
permit selection while submission still rejects. Selection range is explicitly
200 cm for this melee dev fixture, matching the configured combat range.
Noncombat providers retain the control-target interface checks. Range is measured
between actor centers. An integrator API request for future definition-driven
weapons: expose a pure target life/range/LOS eligibility query from combat;
ValidateAttack currently short-circuits before range during cooldown/actions.
If future definitions change range, synchronize SelectionRange until that query
exists. This fixture does not infer navigation reachability for combat dummies.

The generator uses LHGameMode directly and spawns LHEnemyCharacter at all four
markers with the native capsule half-height offset. Tagged enemies receive
synthetic resources at BeginPlay. Nonblocking cylinders make their locations
visible; these remain visible after death and are not corpse/lifecycle assets.
No regenerated umaps are delivered. Existing maps must be regenerated on host
before evaluating the wiring.

All introduced numbers are **Prototype**, authored W1-INT on 2026-10-08, with
no historical source claim: dev HP/mana 100/100, mana cost 2, cooldown 3 s,
impact 1 s, range 200 cm, fixed damage 10, deterministic RNG seed 123,
guaranteed hit, zero attribute prerequisites, zero armor/resistance/scaling,
minimum damage 0 and quantum 1. FLHNumber/FLHInteger inputs carry Prototype
provenance. These are debug fixtures, not legal progression or authentic T4C
mechanics. Integration tests use 100 HP/10 mana and a lethal 10 HP target.
Other movement/room numbers retain W1-01/W1-04 provenance.

## Host validation and manual G1 checklist for Matt

Run as the normal host user (worker is root and cannot initialize Unreal):

```bash
UE_ROOT=/home/brewerm/Downloads/unreal bash build/generate-dev-maps.sh
UE_ROOT=/home/brewerm/Downloads/unreal bash build/run-tests.sh Lighthaven
```

Expected new tests: Lighthaven.Integration.ControlsCombat and
Lighthaven.Integration.ContextClearsMovement. Fixtures use globally unique
transient worlds, CreateWorld initialization once, actor initialization once,
explicit PlayerState/controller possession, and cancel/destroy cleanup. Combat
advances the actual ability timer using a scoped frame counter, checks no early
damage, one impact/death, duplicate rejection and dead-target exclusion. Context
test verifies actual held axes, pending pawn input and velocity clear, and neutral
release gating. Headless fixtures do not exercise hardware input or rendering.

1. Regenerate both maps, inspect their WorldSettings game mode, build navigation,
   and check that four tagged enemy capsules sit on the floor. Commit reviewed
   maps through LFS separately. Restart PIE for fresh prototype resources.
2. In Dev_Combat, use WASD and Shift to walk/run. Tab/Q cycle, left click a dummy,
   F cancels; right mouse attacks. Approach each dummy within 200 cm. Confirm
   the pillar blocks selection/attack at floor (600,820,0), while floor
   (760,1000,0) permits a clear shot to TargetLOS (600,1000,0). Both are within
   200 cm. Moving outside range before impact
   prevents damage. Check pending attacks stop movement; fresh movement requires
   release and re-press (hold D through attack completion and key repeats; no
   movement may resume). Left stick must return below 0.2 then deflect again.
   Camera middle-drag and wheel remain functional.
3. Repeat with left stick/L3, RB/LB cycling, R3 cancellation and right trigger
   attack; test right-stick camera and D-pad zoom. Verify keyboard and gamepad
   both reach the same authority result. Observe request reasons and one timed
   hit/damage log per accepted attack, no damage from animation rate, 3 s cooldown,
   and one enemy death after ten 10-damage hits. Dead dummy must stop cycling;
   visible ruler remains intentionally. No health HUD/animation is claimed.
4. Hold movement while opening C/I/Escape (gamepad North/View/Menu), switch
   UI/Creation/Gameplay, alt-tab, and unpossess/repossess. Verify axes, pending
   movement and velocity clear; reentry with a held stick requires neutral.
   Screens/pause widgets are future work: SetControlContext(Gameplay) explicitly
   restores gameplay during this check; opening a screen alone does not pause.
5. In Dev_Movement, test door and corridor both ways, 30-degree ramp ascent and
   descent, steep 60-degree ramp rejection, each stair and landing, and low
   ceiling/boom collision. Check walk/run diagonals, min/max zoom and orbit/pitch
   limits with both devices at 720p and 1080p. Wall/roof fading is not implemented.
6. Cook/package both maps on Linux, launch the archive and confirm Dev_Combat
   startup and identical controls. Windows validation remains deferred.

G1 remains a host play decision; compilation alone does not pass it.

## Worker validation evidence

`UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game`
completed with elapsed_seconds=147.901 exit_code=0. UE 5.8.3 reported both editor and game
`Result: Succeeded` (UBT 11.86 s editor / 135.34 s game). `git diff --check`
passed. UBA reported some action result store tasks did not succeed; compilation
and linking still succeeded. Earlier attempts exposed stale UHT line macros
while headers were edited during a build, incorrect impact log field access,
and a direct test-module GAS symbol reference; final source resolves these.
No Automation/editor/commandlet/cook/package/play execution occurred in this
root worker. Logs/timing accompany the attempt evidence report.

## G1-FIX player movement life gate

Movement ingress and controller tick require the PlayerState combat component
to be alive and its avatar to equal the possessed pawn. Failure clears held axes,
consumes pending pawn input, stops velocity and reinstates release gating before
focus processing; neutral input while dead cannot unlock restored movement.
Canonical resource/avatar restoration still requires neutral followed by fresh
input. No respawn framework or death UI was added.

`Lighthaven.Integration.DeadPlayerMovement` delivers a lethal enemy attack,
checks retained possession, submits neutral then digital/analog axes, and advances
controller/CharacterMovement ticks. It also injects residual input/velocity to
exercise tick cleanup, checks restored-avatar release gating and rejects a live
but mismatched avatar. The floorless transient fixture uses flying movement to
isolate locomotion from gravity. Both devices share semantic ingress; hardware
and focused viewport validation remain host checks. All existing 24 tests retain
their assertions; the four added tests bring the suite to 28.

G1-FIX compilation: `UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game`
exited 0 against UE 5.8.3; editor (including all test sources) and game both
reported `Result: Succeeded` (UBT 237.24 s / 177.32 s). UBA cache-store tasks
reported warnings; compilation and linking succeeded. `git diff --check` passed.
No Automation, editor session, cook, package or play was run: UID 0 cannot
initialize Unreal. Coordinator must run all 28 `Lighthaven` tests on the non-root
host and retain the G1 manual/device checks; no gate result is claimed.

## G1-FIX2 transient input fixture

The fixture now calls `InitInputSystem()` after possession and checks PlayerInput.
A spawned controller without LocalPlayer startup does not receive that normal
initialization. In UE 5.8.3 `APlayerController::PlayerTick` unconditionally calls
`TickPlayerInput`, whose `check(PlayerInput)` caused the reported crash before the
product death gate ran (`PlayerController.cpp:2309,5444`). `InitInputSystem`
creates configured PlayerInput and invokes SetupInputComponent (line 756).
The product movement gate and every integration assertion remain unchanged.
DeadPlayerMovement still drives full controller and CharacterMovement ticks;
dead/mismatched avatars should clear residual input and velocity, and restored
avatars still require neutral followed by fresh movement. No hardware input,
viewport focus or observed runtime pass is claimed.

G1-FIX2 validation: required editor/game build exited 0, both UE 5.8.3 targets
reported success (195.57 s / 154.89 s). Both modified test sources compiled.
UBA cache-store warnings were logged. All existing assertion lines are unchanged;
`git diff --check` passed. Runtime automation and interactive checks remain for
the non-root host; this is a compilation-validated candidate only.

## G1-FIX3 host retest

Run all `Lighthaven` tests, including new HeldMovementThroughAttack and
HeldMovementThroughContext tests. These use real PlayerInput key events,
Enhanced Input action dispatch and PlayerTick rather than SubmitMovement setters.
Repeat focus loss/regain with a held key and a held stick, including workspace
switches without clicking. See controls.md for the engine-source investigation
and remaining Wayland uncertainty. Regenerate maps for revised LOS fixtures.
`bash build/package-linux.sh` now explicitly cooks both maps by default; launch
the archive with each map, including Dev_Movement, to verify package coverage.
AndroidFileServer is explicitly disabled because Android deployment is unused.
G1 still requires host test/play/package evidence; no new gate pass is claimed.
