# G1 skeptical review — ownership and duplicate hit paths

Task ID: G1-SKEPTIC. Contract revision: 1. Reviewed exact candidate `bf4f19f8bfb475e1a52605515bbe8aa86fbed21f`, against `381ed313f8c181f6048e8fc14c22e0c83fe8669a`, with candidate-tree scope. Deliverable branch base: `8494186289da8971bb3d25f362c9541987aaabd9`. Result revision: the commit containing this document.

Two findings warrant owner triage before relying on the lifecycle and re-entrant callback seams in Wave 2. Both are **verified defects by source/control-flow inspection**, not design suggestions or observed runtime failures. The reproductions below are tests to write/run on the non-root host; no Unreal test, editor or play session was executed in this review. This document does not pass or fail G1 on the owner's behalf.

## Finding 1 — fresh input moves the dead player pawn

Reference: `finding:g1-dead-player-movement`. Severity: **major**. Failure class: logic. Owning area: Framework / controls and player lifecycle (W1-INT, W1-01, W1-03).

Evidence at the reviewed candidate:

- `Source/Lighthaven/Framework/LHPlayerState.cpp:22–28`: combat death clears held movement and the ASC avatar, but keeps the pawn possessed and leaves the controller in Gameplay.
- `Source/Lighthaven/Framework/LHPlayerController.cpp:102–106`: after a neutral input, fresh movement is accepted solely on focus, release and Gameplay context; there is no life/avatar check.
- `Source/Lighthaven/Framework/LHPlayerController.cpp:92–98`: movement submission to CharacterMovement checks only whether an action is pending. The dead player's pending action has been cleared; the still-possessed character receives new movement input.
- `Source/Lighthaven/Abilities/LHCombatComponent.cpp:33–37`: clearing the combat avatar cancels the action but does not disable pawn locomotion.

Reproduction/test: extend the player/controller integration fixture with a configured enemy attacker and set player HP to 10. Deliver a lethal 10-damage hit through `RequestBasicAttack` and `ResolveImpact`. Assert player combat `IsAlive()` is false and the pawn remains possessed. Submit neutral then nonzero movement; with a focused host viewport, advance controller and CharacterMovement ticks. The current code accepts held movement and moves the dead pawn. Expected: held/pending movement and velocity remain zero until canonical resources and the new live avatar are initialized. Repeat for both input devices, then confirm legitimate restored/repossessed life can move.

Smallest sufficient fix: gate movement ingress and per-tick application on the PlayerState combat component being alive **and its avatar matching the possessed pawn**. Clear/stop movement when that gate fails. Preserve release gating when the live avatar is restored. This needs no respawn framework or death UI.

Existing tests clear movement on context switches and clear the ASC on unpossession, but never kill the player and submit fresh input afterward. Enemy-death selection coverage does not exercise this path.

## Finding 2 — an old impact callback tears down a newly committed attack

Reference: `finding:g1-reentrant-impact-teardown`. Severity: **major**. Failure class: concurrency (synchronous game-thread re-entrancy). Owning area: Abilities / combat foundation (W1-03).

Evidence at the reviewed candidate:

- `Source/Lighthaven/Abilities/LHBasicAttackAbility.cpp:8`: one ability object is reused per actor.
- `Source/Lighthaven/Abilities/LHBasicAttackAbility.cpp:26–29`: the timer calls `ResolveImpact`, which publishes callbacks, then unconditionally ends the ability using the object's **current** activation fields.
- `Source/Lighthaven/Abilities/LHCombatComponent.cpp:116–117`: target health/death delegates and `OnImpact` can execute synchronously before control returns to the timer callback.
- `Source/Lighthaven/Abilities/LHBasicAttackAbility.cpp:34–36`: EndAbility unconditionally clears the shared timer and component pending action before the base class validates whether ending is valid.
- `Source/Lighthaven/Abilities/LHCombatComponent.cpp:98–100`: a replacement action has already charged its cost when these stale cleanup operations run.

Reproduction/test: use the combat fixture with a supported, explicitly Prototype zero-second cooldown, one-second impact delay, cost 2, mana 10, target HP 100 and damage 10. Bind a one-shot `OnImpact` listener that calls `CancelAllAbilities()` on the attacker and immediately requests another attack on the same live target. Drive the **actual timer**, as the integration test does; invoking `ResolveImpact` directly would miss the bug. The first hit reduces HP to 90. Cancellation synchronously ends the first activation; the replacement request succeeds, reduces mana to 6, and schedules its timer. When the old `Impact()` resumes, line 29 reads the replacement activation and ends it; lines 34–35 erase its timer and identity. The second committed attack never impacts. Expected: either reject/defer replacement activation until publication unwinds without charging it, or preserve the replacement identity/timer so its hit occurs exactly once.

Engine corroboration, read from installed UE source: `GameplayAbilities/Private/Abilities/GameplayAbility.cpp:741–767` cancels synchronously unless the ability has a scope lock; this project timer callback takes none. `AbilitySystemComponent_Abilities.cpp:1831–1850` rejects a second InstancedPerActor activation only while its spec remains active, so it can activate after explicit cancellation returns. `GameplayAbility.cpp:771–810` performs base EndAbility validation, after this subclass has already performed its own cleanup. The ASC's ability-list lock is distinct from the ability's scope lock.

Smallest sufficient fix: retain the original impact/activation identity across callback publication and prevent the resumed old callback from ending or clearing a different activation. Guard subclass cleanup against stale/invalid ends before touching the timer or pending state. Alternatively, reject/defer new activation while the original impact/publication stack is unwinding. Add the timer-driven regression above, including cancellation without replacement and a target-death listener variant. No new framework is necessary.

This is not a duplicate damage result in the normal dev fixture: its 3-second cooldown and logging-only listeners mask the gap. It is a concrete failure of the public native listener/activation seams and the documented claim that a stale callback cannot mutate a later activation.

## Completed challenge checklist and limits

| Challenge / area | Inspection result |
|---|---|
| Evidence gaps | Reviewed combat, controls, integration and rules fixtures. No test covers fresh input after player death or cancellation/replacement inside an actual impact timer callback. Host hardware/focus, generated-map, cook/package and play evidence remain required. |
| Missed edge cases / duplicate hit paths | Identity, committed/pending flags and consume-before-event guard reject ordinary repeated impacts; target hit identities and death latch protect ordinary duplicate lethal hits. Destroyed/dead targets, source death, range/LOS failure and life revision changes are checked. Finding 2 concerns cleanup crossing activation ownership despite those guards. |
| Missing acceptance criteria | G1 requires both actual input methods and a timed dummy fight. Native mapping parity and synthetic fixtures cannot establish that host play gate. Generator code creates tagged enemies, but candidate documentation explicitly requires host map regeneration; no regenerated map/runtime result is inferred here. |
| Unsafe concurrency | Game-thread execution avoids parallel mutation, but synchronous GAS/attribute/native delegates re-enter state. Finding 2 identifies a specific unsafe sequence rather than recommending threading changes. |
| Unsupported claims | Documentation appropriately separates compiler results from runtime checks, labels fixture values Prototype, and states post-commit cancellation retains cost/cooldown. Retained committed cost is therefore not independently reported as a cancellation defect. The stale-callback claim is too broad for finding 2. |
| ASC ownership / lifecycle | PlayerState owns player ASC, pawn forwards it, enemy owns its ASC. Possession initializes and unpossession/teardown cancel/clear; initialization advances target life revision. Death clears the player ASC but misses locomotion ownership (finding 1). Runtime respawn/restore sequencing remains untested. |
| Rules / provenance | Runtime attack config requires resolved, finite, nonnegative fields with nonmissing/nondisputed status; generic rules reject required Unresolved fields. Debug constants are labelled Prototype. Bible APIs explicitly select documentary disputes and retain provenance; they are not silently substituted into runtime combat. Historical source values were not independently re-retrieved or verified in play in this review. |
| Save/restore architecture §7 | Core save records contain values and stable IDs, not live actors or GAS aggregates. Runtime weak targets, hit identity sets and GAS totals are not serialized records. Completed-action boundaries, RNG byte encoding, canonical resource restore and persistent reward/request idempotency remain explicitly deferred integration work. No live save implementation is claimed. |
| Rules/AI controller access | Search found no `GetPlayerController` use in Source. Rules calculations take typed inputs; no AI module or reward minting exists yet. |
| Security / test weakening | No network transport added and remote activation is disallowed by the standalone request seam. Candidate diff adds integration tests; it does not weaken existing assertions. No additional concrete security finding identified. |

Checks actually performed: candidate/base diff inspection; byte equality comparison of all 54 Source files and the five requested implementation documents against the exact candidate; line-numbered source traces; inspection of installed GAS cancellation/activation/end validation code; static review of tests, commandlet and save DTOs; searches for controller-zero access and save pointer fields; `git diff --check`. All are read-only/static checks, not gameplay tests. No compile was needed for this documentation-only deliverable and none was attempted.

Evidence artifact: worker output `library/source-evidence.txt`, SHA-256 `9e9c1e4daac890ee61d7b7e156b1d13b14dad3a08a7ddb6df23a782469626e37`. It records candidate equality, UID 0, candidate excerpts and installed engine excerpts. No identity/producer history was consulted.

## Handoff

- Task ID / contract: G1-SKEPTIC / 1.
- Base / result: deliverable base `8494186289da8971bb3d25f362c9541987aaabd9`; result is this document's commit. Reviewed candidate and comparison base are separately identified above.
- Owned paths / binary assets: only `docs/reviews/G1-skeptic.md`; none.
- Behavior changed: review documentation only; no code changes.
- Source-backed mechanics / provisional tuning: none introduced. Reproductions use explicitly synthetic Prototype values.
- Build/editor/cook/package/play checks actually run: none. Static checks and evidence are listed above.
- Checks not run / prerequisite: Automation, editor, map generation and play are forbidden in this UID-0 sandbox; a non-root host Unreal session is required. Linux packaging/play remain unobserved here; Windows is deferred per retained scope.
- Known defects / decisions: two source-validated findings with unexecuted runtime regressions; owner triage and host validation remain outstanding. No gate approval or task verification is asserted.
- Next task / integration: respective owners apply narrow fixes, then coordinator runs timer/lifecycle regressions and existing `Lighthaven` Automation on the non-root host, regenerates reviewed maps, and exercises G1 with keyboard/mouse and gamepad before Wave 2. No push or merge performed.
