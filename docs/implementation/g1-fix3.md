# G1-FIX3 / G1-FIX3b QA evidence candidate

2026-10-08. Base: `4a04641a2e68601410d729816a063febbe0ce39d`.
This report distinguishes coordinator-supplied host results on that revision
merged with main from this worker's source inspection and build. No G1 pass is claimed.

## Bug 1: packaged Dev_Movement missing

Root cause: package-linux.sh previously required one explicit map and passed only
that map to UAT. G1-FIX3 accepts multiple map arguments, joins them with `+`, and
explicitly defaults to Dev_Combat and Dev_Movement. Coordinator reports UE 5.8.3
normal-user packaging with both arguments returned BUILD SUCCESSFUL; packaged
Dev_Movement loaded with LHGameMode and exited 0. Bug 1 has host verification
supplied in this task. No package was run in this worker.

## Bug 2: held movement resumes after attack

G1-FIX3 preserves physical PlayerInput state when clearing movement, gates on
actual WASD release/stick neutral, and remembers keys flushed by the engine
until a real release event. Repeat events cannot manufacture release. Pending
combat actions clear movement and require release before fresh movement.
HeldMovementThroughAttack uses actual D key events and Enhanced Input dispatch,
checks no movement through attack completion, then requires a fresh press to move.
HeldMovementThroughContext also checks engine flush/repeat versus actual release.

Coordinator reports all tests passed except `Fresh press moves pawn`; the
preceding nonzero held-movement assertion passed. Source diagnosis found a
fixture setup defect rather than evidence of a controller delivery defect:
UE 5.8.3 PlayerController.cpp:304–345 IsLocalController returns false when there
is no NetDriver and no ULocalPlayer, unless the local-controller flag is set.
The transient fixture has neither. CharacterMovementComponent.cpp:1650–1764
consumes input but calls ControlledCharacterMove only for a locally controlled
character (or certain controllerless cases). Repeating 16 ms ticks alone would
not repair this. The fixture now calls public SetAsLocalPlayerController after
spawn, asserts the pawn is locally controlled, and allows at most five further
16 ms controller/CharacterMovement ticks after the first fresh-press tick.
The same position-change assertion and all stopped/release assertions remain.
No movement is injected by a setter to make that assertion pass. Runtime
confirmation of this diagnosis/fix is pending normal-user automation.

## Bug 3: input dead after Hyprland/Wayland workspace switch until click

Reported QA observed IsForegroundWindow false until a click. Source inspection:
FSceneViewport::IsForegroundWindow (SceneViewport.cpp:590) delegates to the
native window; FLinuxWindow::IsForegroundWindow (LinuxWindow.cpp:1150) compares
LinuxApplication's current active window. LinuxApplication.cpp's mouse-down
path calls ActivateWindow (around line 526). FSceneViewport::HasFocus
(SceneViewport.cpp:111) checks Slate's focused viewport widget independently.
These are local UE 5.8.3 engine sources under Engine/Source/Runtime.

The project previously relied solely on native foreground state; that gate
amplifies stale activation bookkeeping. G1-FIX3 accepts Slate viewport focus OR
native foreground, bounded by application activation delegates. Deactivation
clears movement; reactivation reinstalls mappings with held-key suppression.
It does not force OS focus or synthesize clicks. Engine/SDL/compositor activation
is the suspected underlying cause, but source inspection cannot establish which
Wayland event is missing. No interactive compositor reproduction was available.

Matt should repeat identical packaged and PIE workspace/alt-tab sequences on
X11, without clicking on return, recording native foreground, Slate HasFocus,
application activation/deactivation callbacks, key press/release delivery and
SDL video driver. Compare Wayland with those observations; check fresh movement
and held-key suppression separately. If Slate and application activation also
stay stale, the click workaround remains necessary. Bug 3 is not host-verified.

## Bug 4: LOS pillar fixture cannot distinguish LOS from range rejection

G1-FIX3 moves the pillar to (600,910,200), size (60,60,400), and adds floor
markers (600,820,0) blocked and (760,1000,0) clear for TargetLOS at (600,1000,0).
Actor-center separations are 180 and 160 cm, both inside the prototype 200 cm
range; the first center trace crosses the pillar, the second does not.
This is source/layout evidence, not observed gameplay. Coordinator generation
failed saving Dev_Movement because lockable LFS maps were read-only, so the
revised map still requires generation and play verification by its asset owner.

## Minors and generation prerequisite

- AndroidFileServer is explicitly disabled in Lighthaven.uproject in G1-FIX3;
  Android deployment is unused. Coordinator editor/game builds and Linux package
  succeeded with that configuration; no Android validation is claimed.
- Controls and integration checklist now describe both in-range LOS positions,
  multi-map packaging and release/re-press behavior. Gamepad checks are
  **UNTESTED and not a G1 blocker**, per Matt's 2026-10-08 06:02 ET decision;
  they remain open for a later gate.
- generate-dev-maps.sh now checks both existing target .umap files with `-w`
  before invoking the editor and stops with the exact path and `git lfs lock`
  or single-local-writer `chmod u+w` advice. It never chmods maps automatically.
  Existing UID-0 rejection remains. No binary maps are owned or modified here.
  Bash syntax passed. A normal-user mock-editor preflight test was attempted,
  but sandbox runuser failed with `cannot set groups: Operation not permitted`
  before executing the script. Read-only/writable runtime branches remain for
  host validation; root's access semantics would not model the LFS checkout.

## Handoff

Task ID: G1-FIX3b
Base revision / result revision: 4a04641a2e68601410d729816a063febbe0ce39d / submitted candidate HEAD (receipt identifies exact revision).
Contract revision: 1.
Owned paths / binary assets: Source/Lighthaven/Framework/, Source/Lighthaven/Input/, Source/LighthavenTests/Integration/, build/generate-dev-maps.sh, docs/implementation/{g1-fix3,w1-integration,controls}.md; no binary assets changed.
Behavior changed: transient controller local classification; bounded fresh-press position check; read-only map preflight; QA evidence and gamepad status.
Source-backed mechanics: none changed; UE source evidence above concerns fixture/focus behavior.
Provisional tuning introduced: no gameplay tuning; bounded test wait is five additional 0.016 s frames.
Build/editor/cook/package/play checks actually run: see final validation entry below; id -u returned 0; bash -n and git diff --check passed.
Results and evidence paths: attempt output report.md and library/build.log; coordinator results above are supplied evidence, not worker observations.
Checks not run and concrete missing prerequisite: automation/editor/map generation/cook/package/play require a normal-user Unreal process; this worker UID is 0. Interactive Wayland/X11 and gamepad hardware absent. Windows deferred.
Known defects or remaining decisions: fresh-press fix needs host test confirmation; Wayland mitigation and regenerated LOS map need host verification. No gate pass asserted.
Next task and integration notes: coordinator review candidate, run all Lighthaven tests, explicitly unlock maps as their single writer, generate/review maps, verify LOS and workspace focus, then evaluate G1. Keep gamepad open without blocking G1.

Final worker validation: required `UE_ROOT=/home/brewerm/Downloads/unreal bash
build/build-linux.sh --game` exited 0. UE 5.8.3 editor and game both reported
`Result: Succeeded` (UBT 82.34 s / 74.74 s). Integration test source compiled.
UBA reported some action-result store tasks did not succeed; targets succeeded.
`bash -n build/generate-dev-maps.sh` and `git diff --check` passed. Executing the
generator as UID 0 exited 1 with the expected normal-user requirement, before
any editor invocation. Per task instruction no Unreal tests were run as root.
