# W2-04b integration candidate and G2 checklist

Task ID: W2-04b. Contract revision: 1. Base revision:
`66cd1e0fb8c6d041b87a064c928e1f373a56d44c`.
Result revision is the submitted candidate in the receipt/report. This is source
and compilation evidence for review, **not an observed G2 runtime pass**.
The previous W2-04 canonical-identity checkpoint is resolved by W2-01d:
the production profile assigns `RequestDigest = LHSave::RequestDigest` and
`GrowthId = LHSave::GrowthId`. No test fingerprint is used by the live adapter.

## Wiring requirements — implemented source

`ULHSessionSubsystem` owns one `FLHWave2Session` for the game instance. It initializes
and configures W2-01 once, retains its actual store through a public accessor,
and owns event subscriptions. The subsystem retains adapters across controller
travel; controllers retain them for the presenter lifetime. Deinitialization clears
travel/exit callbacks. Native tests instantiate the exact same session adapter with
an injected W2-01 storage implementation; no separate integration implementation.

ALHPlayerState creates one character authority component beside its ASC. Frontend
GameMode uses that PlayerState, and its controller binds the live session before
constructing its presenter. New creates a fresh bootstrap authority. The three
frozen command overloads route only to the bound authority; unrelated command kinds
fail closed. Questions, appearance, allocation and equipment reviews are owner
reads. Allocation/equipment reviews execute against a detached service copy with
fixed probe IDs: no new request IDs, preview tokens, reward IDs, live mutations or
save writes occur. This can fail closed if a saved receipt collides with a probe ID.
The frontend presenter receives the bootstrap request epoch before preview/confirm.

Accepted, non-replayed commands export into the session's complete snapshot. Export
preserves unrelated world/session/header records. Creation completes the explicitly
empty Prototype world, safe entrance, codec/hash metadata and effect policy, then
imports that completed snapshot into the authority before further command exports.
A queued save is flushed on the next controller tick, after synchronous publication
unwinds. One initial creation command schedules one initial write; accepted replay
and the presenter's latched confirm schedule none. Save/travel suppression prevents
new mutation commands while queued/writing/travelling. Combat settlement is not
available for this temporary movement entry, so the session-bound player attack
path fails closed and receives no dev combat resource fixture. Direct dev-map G1
sessions without a canonical character retain their existing fixture behavior.

Creation travels to Dev_Movement only on its matching W2-01 Succeeded event. The sole in-flight snapshot/character and
command suppression identify the completion; a retry may raise the durable
sequence after an initially valid file had a failed readback. The adapter imports
that returned high-water sequence before admitting later commands. Failed
and Unreadable details remain visible through the presenter; save events log kind,
GUID, sequence and detail. RetrySave retries the immutable pending snapshot, keeping
the committed identity and receipt. No new creation is issued. Confirmed quit waits
for durability; failure keeps the executable open, and successful retry completes
that confirmed quit. A normal quit after the last completed action is already durable
needs no duplicate write.

Continue selects by GUID (also shown in profile status), enumerates unreadable
profiles, requires explicit recovery acknowledgment, then loads and imports the
complete save. An invalid profile is loaded once for W2-01's concrete Unreadable
event; files are preserved. No replacement character is automatically created.
Authority import rejects invalid resources/history/equipment and repeated reward
IDs. D06 identity mapping remains under W2-01, and the unchanged character owner
rejects a growth ID collision against existing history. This Wave 2 profile accepts
no encounter/corpse/source lifecycle records: the registry validator rejects them
rather than dropping them or altering D06 atomic lifecycle/high-water semantics.
World settlement and rewards remain their later-wave owners' work.

Gameplay possession reconstructs the fresh PlayerState authority from the complete
snapshot. It installs canonical current resources and recomputed derived GAS values
before InitializeAvatar, bypassing dev-map resource initialization. Inventory,
equipment, base attributes, historical maxima, growth inputs/RNG and receipts remain
canonical value records; only transient adapter bindings refer to actors. C/I/Escape
open the native journal with live presenters. Menus pause simulation; the controller
ticks while paused to flush completed action saves. Journal closure is deferred to
controller tick so a Slate event does not destroy its own widget, and reentry uses
the existing neutral/release movement gate.

Core/schema revision 1 is unchanged. UI edits are binding/lifecycle/status/retry
adapters. Persistence edits only expose the existing store and read-only access to
existing frozen value encoders; no serializer, validation, storage or reward behavior
is changed. Helpers use file-unique namespaces. No TArray element is supplied to
that array's Add/Insert. No binary asset was generated or modified.

## Explicit Prototype profile and closure

The task authorizes the existing synthetic profile when no better ledger profile
exists. Assumption: its two fully described toy items are the declared initial
Prototype kit, allowing legal UI equipment without a debug grant. All enabled
numeric inputs carry Prototype provenance. Creation uses four Question.Test0..3
answers, one finite outcome with five attributes of 10 and 10 unspent points
(total budget 60; maxima 20); level 1, HP20/MP10, skill/gold zero, two inventory
cells, capacity100. XP thresholds 0/100/300/600; growth HP2/MP1 plus two normalized
rolls scaled by2, Base basis, +5/+15 grants. Stats constants are1 (capacity100),
coefficients zero. The toy bow/quiver have weight1/stack1 and zero requirements;
bow grants Strength2/Accuracy3 and requires its equipped compatible quiver.
Seed123 is an explicit Prototype finite-table seed. Combat and mana regeneration
are disabled policies for this temporary canonical entry. None is an authenticity
or verified-in-play claim; no new mechanics research was performed.

`LHWave2Closure` publishes the resolved mechanical closure and complete enabled
gameplay catalog as explicit ASCII-sorted codec structs with canonical keyed sets,
ordered question/threshold/outcome sequences, symbolic policies, values and full
provenance. It composes the existing frozen value encoders through read-only
accessors. SHA256 is computed from those bytes, with ruleset ID/revision included
and ContentHash/migration/cosmetic bindings/transient state excluded. Catalog includes
the mechanical closure and the one authored temporary entrance/map/checkpoint.
Additional eligibility policies/skill minimums are not enabled by this synthetic
profile; extending the profile requires a closure adapter/compatibility revision.
The identity checkpoint has no alternate command/reward codec.

## Required native integration tests — implemented, runtime NOT RUN

`Lighthaven.Integration.Wave2.*` adds six native Automation tests:

- IndependentRebuild: two GUID-distinct equal-name characters through live presenters,
  deferred single saves, full runtime/world destruction, independent restore and
  canonical snapshot equality, GAS values installed before avatar initialization.
- EquipmentAllocationRebuild: side-effect-free legal review, allocation and quiver/bow
  equip commands, each completed save, retained growth history using canonical IDs,
  destruction/rebuild and full inventory/attributes/resources/world/session comparison,
  including a second untouched character after the mutations.
  The growth settlement is an explicitly trusted native fixture, not packaged G2 evidence.
- RecoveryAndUnreadable: corrupt newest slot, acknowledgment before loading the earlier
  canonical snapshot, then both slots bad, visible error, no new character or writes,
  retained bad bytes and successful independent loading of the unaffected character.
- SaveFailureRetry: failed initial write, visible failure, no travel, latched repeated
  confirm without duplicate write, stable committed identity, failed quit remains open,
  then matching retry durability completes the confirmed quit.
- ReadbackRetryHighWater: a valid write whose readback fails remains visible and
  blocks travel; retry succeeds with a raised envelope sequence and the next
  command advances from that durable high-water value.
- CanonicalClosure: frozen canonical struct prefix, hash coverage, hash/migration
  exclusion, definition-set permutation, mechanical/provenance mutation sensitivity,
  and catalog hash agreement. Full closure byte/digest host observation remains open.

Fixtures use globally unique transient worlds, one initialization, controller
InitInputSystem, destruction per runtime, the actual authority component and actual
W2-01 store/codec. No timers are advanced by these tests, so no synthetic timer tick
or GFrameCounter mutation is needed. Worker UID is 0: no Automation/editor execution
was attempted. The coordinator must capture the full suite's exact pass/fail lines;
existing 56/56 host results are inherited evidence, not this worker's observation.
Profile reads are cached between save events so Slate refresh does not re-read
and hash every file on every frame.

## G2 packaged Linux checklist for the coordinator

All items below are **NOT RUN**. Windows equivalents are deferred.
The live adapter and native tests are now implemented. Compilation does not establish
the runtime behaviors or pass G2.

1. Build editor and game with
   `UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game`.
   Require both `Result: Succeeded`. As a non-root host user run
   `bash build/run-tests.sh Lighthaven`, retaining exact pass/fail lines.
2. Hydrate the actual LFS maps on the host first: this isolated checkout has only
   129/130-byte LFS pointers. If needed, the coordinator runs the existing editor
   generators and reviews/commits binaries separately. Startup now selects L_Frontend
   and MapsToCook includes it. Package with explicit map arguments (the script's
   default arguments still select the G1 maps):
   `UE_ROOT=/home/brewerm/Downloads/unreal bash build/package-linux.sh /Game/Lighthaven/Maps/L_Frontend /Game/Lighthaven/Maps/Dev_Movement /Game/Lighthaven/Maps/Dev_Combat`.
   Launch the real archive. Record revision, command, exit code and archive.
3. Create A and B through the UI. Record their stable GUIDs and displayed
   Prototype profile/provenance. Confirm each creation reaches Dev_Movement only
   after one successful initial save. Confirm both appear independently in select.
4. Equip A through the inventory screen and legally allocate points through the
   character sheet. Capture the completed save event and the resulting inventory,
   bindings, base/effective attributes, growth records and current resources.
   Equip the Prototype quiver before its compatible Prototype bow. The legal initial
   Prototype pool is 10 points. C/I open the journal; Escape closes it after staged
   edits are resolved. Confirm B is unchanged. Do not inject authority state or grant debug points to
   substitute for this UI check; use the declared legal Prototype starting pool.
5. Exit through confirmed Quit after durability success. Relaunch the executable
   (a genuinely new process). Continue A and B separately; compare identities and
   durable records with step 4, including restored resources and equipment.
6. With the executable closed, back up its actual save files outside the save
   directory. Identify newest A/B generation by observed save sequence, make that
   generation unreadable, then relaunch. Require a clear recovery warning and
   explicit acknowledgment; compare restored state with the prior valid generation.
7. Make both generations unreadable for one backed-up character. Relaunch; require
   a visible error and disabled continue, no replacement GUID or silent fresh
   character, and unaffected loading of the other character. Restore backups only
   with the executable closed.
8. Exercise a write failure. Require the W2-01 failure detail to remain visible,
   prevent travel/quit, activate RetrySave on the same screen and observe success.
   A confirmed pending quit completes on successful retry; otherwise initial creation
   travels on its successful retry.
   Record process logs, save event sequences and observed state; screenshots alone
   do not prove independent persistence or successful writes.

## Validation and handoff

See the attempt report for final build command, exit code, wall time, both target
result lines and evidence paths. `git diff --check` and scope review are run before
submission. A failed test-module link from calling an unexported GAS method was
corrected to use the existing exported combat API. No engine/editor/cook/package/
play/native test result is inferred from compilation. LFS maps are pointers here;
host hydration or editor generation is a concrete package prerequisite. Windows
validation remains deferred. If the first-ever failed write leaves only a corrupt file and no valid generation,
W2-01 Start intentionally refuses to overwrite the all-invalid pair on retry
(LHSaveStore.cpp:106); this adapter preserves its error/identity and does not delete
files. Integrator request to the persistence owner: define a safe policy for retry
of a store-owned failed initial generation, or document host intervention. This
non-wiring storage policy is outside W2-04b and is not changed here.
Settings execution and richer UI presentation remain
W2-03 limitations; this adapter does not implement economy, combat settlement,
world topology, respawn or migration. Next: coordinator review, host Automation,
actual Linux packaging and this G2 checklist before a gate decision.

Final build actually run: `UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game`,
exit **0**, wall time **66.395 seconds**. Editor `Result: Succeeded`, UBT total
**9.86 seconds**; game `Result: Succeeded`, UBT total **55.87 seconds**.
The exact editor timing is printed in `library/build-final.log` in the attempt
output; that log is authoritative if this summary differs. Both native test and
game targets linked. UBA warned that some action result store tasks did not succeed,
without preventing target success. Whitespace and 24-path scope checks passed.
No native runtime or gate pass is claimed.

## W2-04c — direct combat pawn assertion

`ULHCombatComponent::IsAlive()` combines a valid, non-destroying ASC avatar actor
with finite positive health. It does not distinguish the gameplay pawn from the
PlayerState: UE 5.8.3 `UAbilitySystemComponent::InitializeComponent()` calls
`InitAbilityActorInfo(Owner, Owner)` by default. The restore adapter does not bind
a pawn early. Thus restored positive health can make the owner-backed ASC alive
before `ALHPlayerState::InitializeAvatar`.

The read-only `ALHPlayerState::GetCombatAvatar()` forwards the existing GAS avatar
read and casts it to `APawn`, returning null for the default PlayerState avatar.
Keeping that forwarding call in the Framework module also avoids the previous
cross-module GAS export/link problem. IndependentRebuild now directly requires
no combat pawn before initialization for both restored characters, and requires
the exact spawned pawn after initialization. Every other assertion is retained.
No restore ordering, Abilities code, schema, tuning or binary asset changed.

Native Automation is not run in this worker: `id -u` returned **0**, and Unreal
refuses root execution. Coordinator should run `bash build/run-tests.sh Lighthaven`
as a normal host user and retain exact pass/fail lines; no runtime pass is claimed.
Build details and evidence are recorded in the W2-04c attempt report.

W2-04c build actually run:
`UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game`.
Exit **0**, wall time **232.132 seconds**; both editor and game reported
`Result: Succeeded`. Evidence: W2-04c attempt output `library/build.log` and
`library/build-time.txt`. `git diff --check` passed. Native tests, editor launch,
package and play were not run in this root worker; the previously reported
64 pass / 1 fail and package launch are coordinator evidence for the base only.

## W2-05 — G2 gameplay input / UI candidate

Base: d02fee1d215e4db7df438661260aa9c4059732c6. Contract revision 1.
Only assigned UI, Framework, Persistence wording, UI/Integration test and two
implementation-document paths changed. No Core schema, binary asset, mechanics
value, renderer or W2-06 launch-hang behavior changed.

Gameplay startup/possession now applies GameOnly and resets the persistent
viewport's IgnoreInput flag; frontend teardown restores defaults. Journal pause
and UIOnly transitions retain existing movement-release gating. Creation's modal
latch is scoped by action, so a subsequent confirmed Quit reaches the existing
pending-exit owner and takes precedence over creation travel on successful retry.
Unreadable Continue is visibly disabled, skipped by focus and refuses activation.
Appearance choices are separate named categories with missing-category feedback.
Selection/acknowledgment changes clear stale profile errors without hiding save
failures. First-save failure wording no longer claims a prior generation exists.

Added native test candidates:
- Lighthaven.Integration.Wave2.GameplayInputHandoff
- Lighthaven.Integration.Wave2.WidgetCreationFailureQuitRetry
- Lighthaven.UI.UnreadableContinueAndStaleErrors

Existing WidgetRejectedFieldsAndDuplicateSubmit now additionally confirms Quit
after accepted creation. Existing SaveFailureRetry additionally submits Quit through
the widget; all prior assertions remain. GameplayInputHandoff uses controller seams
with a real viewport client but no attached Slate viewport; actual window focus,
mouse capture, travel and gamepad behavior require host play evidence.

Automation/editor/cook/package/play NOT RUN: id -u returned 0; Unreal refuses root
execution. Coordinator must run `bash build/run-tests.sh Lighthaven` as the normal
host user, preserve exact pass/fail lines, then repeat G2 failed items 4/5/7/8 and
appearance/recovery minors in a new Linux package. Windows remains deferred.
Build observation is recorded below and in the attempt report; this candidate does
not establish that G2 has passed.

W2-05 final-source build actually run:
`UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game`.
Exit **0**; editor `Result: Succeeded`, total execution **74.11 seconds**;
game `Result: Succeeded`, total execution **145.27 seconds**. Combined wall time
approximately **220.8 seconds** (build-log creation to final write).
`git diff --check` exited 0. Evidence in this attempt's output:
`library/build-final.log` and `library/checks.txt`. An earlier build was interrupted
with exit 130 after source edits overlapped generated-header compilation; it is
not validation of the final source. Native automation is compiled, not run.

## G2 result

**Passed 2026-10-08** on `G2c-Linux-88f9265` (main `04347fc` + ledger `88f9265`): checklist items 3–8 pass, including both item-8 Quit paths. The launch render stall (3/12 launches in the re-check) is recorded as a known environment limitation (see `g2-render-hang.md`); relaunch is the workaround. Gamepad is untested and non-blocking. Six minor UI findings are queued as W3-06 (see the status ledger).
