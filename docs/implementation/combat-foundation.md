# W1-03 combat foundation

W1-03b compiled the editor and game targets against installed UE 5.8.3. Automation and gameplay have not been executed in this attempt; no play or gate success is asserted.

## Ownership and entry points

`ALHPlayerState` owns `ULHCombatComponent` (a GAS ASC) and `ULHAttributeSet`; the possessed pawn is its avatar. `ALHEnemyCharacter` owns the same component and can serve as a dummy. There are no initial HP, mana, weapon, range, cost or combat defaults that pretend to be researched values. Install canonical resources and W1-02 derived values through GAS numeric base attributes, then call `InitializeAvatar(Pawn)` on PlayerState or `InitializeAfterRestore()` on an enemy. These explicitly initialize actor info, cancel old actions and grant one instanced basic attack ability. The minimal GameMode selects LH PlayerState/GameState; GameState exposes the current area identity. Integrator must select this GameMode and W1-01 pawn/controller classes.

W1-01 must expose the PlayerState ASC through its character's `IAbilitySystemInterface`, call `InitializeAvatar` after possession and canonical restore, and `ClearAvatar` before unpossession, death/travel teardown. Resolve controller-selected stable target IDs to a live `ULHCombatComponent`, then call `RequestBasicAttack(Target)` on the acting PlayerState component. Return its `ELHCommandReason` to the command/UI layer. This narrow native seam does not implement the shared eleven-command handler or request replay persistence. Request identity/replay belongs to that later authority layer.

W4 must construct `FLHBasicAttackConfig` from validated ability/weapon/rules definitions, retain field provenance, and call `ConfigureAttack(Config, RequirementInput)`. Configure is ignored while an action is pending; cancel first to replace equipment/eligibility context. `RequirementInput` must reflect canonical base/effective attributes, level, learned skills/spells, and equipped compatible quiver. This foundation executes melee only; no arrow consumption or spell implementation is added. Configure the dedicated combat `FRandomStream` explicitly with `SetCombatRandomState`; there is no implicit clock seed or cosmetic RNG. Persist/restore its state and `GetRemainingCooldown` / `RestoreRemainingCooldown` at completed-action boundaries. RNG serialization into Core's opaque state bytes remains an integrator decision.

## Pipeline

1. `RequestBasicAttack` rejects non-standalone worlds, missing/destroying actors, dead source/target, active actions, cooldown, unresolved/nonfinite/negative tuning, missing random state, W1-02 eligibility failure, invalid combat parameters, insufficient mana, self-targeting, range and blocked visibility trace. LOS traces between actor origins, ignoring source/target collision. W4 may need a definition-driven aim-point/path policy for other attack families.
2. GAS activates `ULHBasicAttackAbility`. It is instanced per actor; the builtin ServerOnly execution policy means local authority executes in this standalone game, including unpossessed dummy/enemy actors. The component is nonreplicated; activation disallows remote requests. No RPC, replication or prediction implementation is supplied.
3. GAS `CommitAbility` establishes the ability commit boundary. The native component revalidates and atomically records the pending action, charges mana and starts its active-simulation cooldown. No GameplayEffect assets are required; the ability's default GAS cost/cooldown effects are empty, and this small ASC helper owns the native cost/cooldown mutation. Do not attach separate GAS cost/cooldown effects to this class without replacing the helper, or costs would be charged twice.
4. Movement velocity stops at commitment for characters. A world timer fires after explicit `ImpactSeconds`; zero delay directly calls the same impact path. No animation notify controls damage. Input suppression during windup is W1-01's responsibility: stop-on-commit does not prevent new movement input.
5. Impact checks the exact `ActivationId + ImpactIndex` against the active action, consumes its single callback before any event, then revalidates actors, source/target life, target restore/life revision, range and LOS. Failed impacts consume the callback without drawing RNG. Successful validation draws two explicit normalized rolls from the combat stream and invokes `LH::Rules::ResolveCombat` with live GAS derived stats and configured weapon bounds.
6. Only an accepted hit reaches the target's private damage function. The target rejects repeated identities for its current life, clamps HP to zero, and sets its death-event latch before attribute delegates fire. One lethal transition cancels its active action and broadcasts `OnDeath(Id)` once. The attacker broadcasts `OnImpact(Id, Result)` for resolved hits/misses. W4 must bind death to its encounter life/reward transaction and enforce persistent reward idempotency; this component does not mint loot or XP.

Cancel via `CancelAllAbilities`, or clear the avatar; EndAbility clears the timer and pending identity. A stale callback cannot mutate a later activation. Explicit initialization changes a target life revision, preventing an already committed attack from striking a restored/replaced life. Initialize only at possession/restore boundaries, never per frame. Target hit identities are runtime-only and retained for one initialized life; their set grows with hits on that life and is not a save record. Save/travel must settle or cancel actions before snapshotting.

## Cancellation and refunds

Before native commit: rejection charges no mana, cooldown or RNG and deals no damage. After commitment: cancellation, death, destruction, target death, range/LOS failure or rule rejection retain the charged mana and remaining cooldown. There is **no refund** and no second roll/impact retry. Pause naturally pauses world timers and world-time cooldowns; no offline clock is used. W4 should explicitly authorize a different policy before introducing spells. Directly setting health to zero from another system does not publish this combat death event: route combat damage through this pipeline, and let the lifecycle owner settle other death causes separately.

## Mechanics and provenance

No new T4C numeric mechanics are claimed. Hit/damage and prerequisites delegate to the W1-02 pure functions and preserve their unresolved/prototype status. Its ledger ruleset intentionally lacks usable production combat/requirement parameters, so it rejects until the definition/rules adapter supplies approved data. `FLHNumber` tuning requires resolved values and nonmissing/nondisputed provenance. Synthetic tests alone specify 100 HP, 10 mana, cost 2, cooldown 3 seconds, delay 1 second, range 200 cm and damage 10 with guaranteed hit; all are labelled Prototype, have no external source claim, and are never exported as play tuning.

## Tests and integration requests

Native fixtures under `Lighthaven.Abilities.*` are written for DestroyedTarget, DuplicateImpact, FailedResource, DeadTarget, DeathOnce and ImpactRangeAndCancellation. They exercise the GAS activation path and manually invoke the explicit impact callback, allowing duplicate/race checks without advancing a timer. They **require a transient UWorld and physics scene**, constructed in C++; no authored map/assets are required. This proves neither timer scheduling nor a rendered fight until host execution/play validation occurs.

Runtime Build.cs already exports GameplayAbilities, GameplayTags, GameplayTasks and the module include path. The test module consumes their public transitive dependencies through Lighthaven. No shared paths changed. Integrator requests: select LH GameMode and W1-01 pawn/controller; implement avatar ASC forwarding and possession/restore hooks; approve/configure production rules and definition adapter; freeze RNG save encoding and command replay; bind encounter death and reward identity. No module/plugin dependency changes were made. Compiler and linker confirmation for both targets is recorded below.

## W1-03b UE 5.8.3 compatibility and observed validation

Combat test flags now use `constexpr auto`, matching the rules tests and retaining EditorContext/EngineFilter and all six tests' assertions. The first build then exposed unresolved GAS symbols when linking LighthavenTests: inline attribute property/value getters and inherited SetNumericAttributeBase/CancelAllAbilities calls referenced GameplayAbilities directly. Attribute getters now have exported out-of-line definitions in Lighthaven, and ULHCombatComponent exports forwarding methods for those two GAS operations. They delegate to the same base implementations; cost, damage and cancellation behavior are unchanged. No Build.cs, Core, Rules or project descriptor changes were made.

Command: `UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game`. The `--game` option is necessary because the script otherwise builds only the editor target. Final command exited **0**, elapsed **120.681 seconds**; both LighthavenEditor and Lighthaven reported **Result: Succeeded** (UBT execution times 27.27 and 92.48 seconds). The initial build exited 6 after 209.390 seconds with the linker diagnostics above. UBA reported that some action result store tasks did not succeed, but both final targets succeeded. `git diff --check` passed.

Evidence: attempt worker output `library/build-first.log` and `library/build.log` (attempt-13d9c0c793e653ab13833e762004630ccdd4c69fc577bed847bab2cb3b5dcd10). No Automation command was run, as instructed: the sandbox runs as root, which Unreal runtime refuses. Coordinator must run `bash build/run-tests.sh Lighthaven` under a non-root host user after review. Timer execution, collision/LOS, movement suppression, assertions at runtime and actual gameplay remain unvalidated. No engine installation/download or root-runtime bypass was attempted.

## W1-03c transient test-world lifetime

The host crash report identified `WorldSettings` name generation during fixture setup. Inspection of installed UE 5.8.3 `Engine/Source/Runtime/Engine/Private/World.cpp` shows that `UWorld::CreateWorld` already calls `InitializeNewWorld`, which creates the persistent level and its named WorldSettings. The old fixture called `InitializeNewWorld` again. The fixture now supplies its initialization options directly to `CreateWorld` and never repeats that initialization. No production combat code or test assertions changed.

All six tests (DeadTarget, DeathOnce, DestroyedTarget, DuplicateImpact, FailedResource, ImpactRangeAndCancellation) instantiate the same stack-owned, noncopyable `FFixture`. Each construction obtains a globally unique `LHCombatTestWorld` name with `MakeUniqueObjectName`, registers a Game `FWorldContext`, creates a rooted Game world in the transient package with `bInformEngineOfWorld=false`, and associates it with the context. Creation enables its physics scene, disables audio/navigation/AI and physics simulation, then calls `InitializeActorsForPlay` once before spawning the two enemies. The fixture still explicitly initializes the enemies' GAS actor info after installing synthetic attributes. It does not call `BeginPlay` or tick: these tests require neither a game mode nor automatic enemy BeginPlay, and manually resolve impacts as before. This follows the unique-name/transient-package/context/actor-initialization pattern in UE 5.8.3's `Developer/CQTest/Private/Components/ActorTestSpawner.cpp` without adding a CQTest dependency.

At every test's scope exit the destructor clears both combat avatars (cancelling abilities and impact timers), destroys target then source unless already destroying, calls `DestroyWorld(false)` to clean the world and remove its root, then removes its engine world context. DestroyedTarget already destroys its target inside the test; teardown avoids destroying that actor a second time. Objects are left for normal Unreal garbage collection, with no forced collection or manual WorldSettings spawn. No fixture can copy ownership of a world.

Host runtime confirmation remains required: the worker runs as UID 0, which Unreal refuses, and the task forbids running Automation here. Coordinator should run `bash build/run-tests.sh Lighthaven` as a normal host user and confirm all six tests finish and repeated fixture setup does not crash. Timer-driven impact and rendered gameplay are still outside this fixture's coverage.

Observed W1-03c validation: `UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game` exited **0** against installed UE **5.8.3**. LighthavenEditor reported **Result: Succeeded**, UBT execution time **131.79 seconds**, including `Compile LHCombatTests.cpp`; Lighthaven reported **Result: Succeeded**, **151.19 seconds**. UBA logged unsuccessful action-result store tasks, but both targets compiled and linked. Evidence is this attempt's `library/build.log`. `git diff --check` passed; a static comparison against base confirmed all six test bodies/assertions unchanged. No Automation/runtime result is claimed.

## G1-FIX re-entrant impact ownership

The native timer callback snapshots its activation serial, GAS activation fields and
impact identity before publishing damage/death/impact listeners. A listener may
cancel and immediately commit another attack on this InstancedPerActor object;
the resumed callback ends only its original still-active serial. EndAbility checks
the current handle/actor info and GAS end validity before clearing shared timer or
pending state. Immediate replacement was retained instead of rejecting requests
during publication, so existing synchronous listener semantics remain available.

New timer-driven regressions are `Lighthaven.Abilities.ImpactReplacement`,
`ImpactCancelWithoutReplacement` and `TargetDeathReplacement`. They check
committed costs, pending identity, replacement timer delivery, original publication
identity, one damage per activation, and duplicate rejection. The death variant
replaces onto a second live target. The zero-second cooldown used to expose
re-entrancy is explicitly synthetic Prototype tuning, not a production change.
Existing tests and assertions remain in place. Runtime execution is reserved for
the non-root host; see the G1-FIX report for compilation evidence.
