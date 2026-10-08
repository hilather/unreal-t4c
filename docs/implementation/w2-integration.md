# W2-04 integration checkpoint and G2 checklist

Task ID: W2-04. Contract revision: 1. Base revision:
`2f6cbe7b270149781615693995d0ac90ab122197`.
This is a **blocked integration evidence candidate**, not a completed implementation
or a G2 pass. No runtime wiring, schema, configuration or binary assets changed.

## Required integrator request

The assigned brief says W2-01 provides the D04 canonical command digest and D06
reward identity functions. At this baseline, `Persistence/LHSaveCodec.h` exposes
SupportsVersion, Sha256, Validate, Encode and Decode only. Encode accepts a
FLHSaveSnapshot, not a command. There is no public command encoder/digest or
reward mapping implementation elsewhere in Persistence. A public accessor cannot
expose an implementation that is absent.

`Character/LHCharacterAuthority.h` requires injected RequestDigest and GrowthId
callbacks. Initialize explicitly rejects absent callbacks
(`LHCharacterAuthority.cpp`, Initialize). Its test profile supplies a reflection
ExportText/SHA1-derived fingerprint and a deterministic tuple stand-in; both are
explicitly test-only. Copying that fixture into production would violate D04/D06.
The save payload checksum is not the digest of a command.

Request the W2-01 owner/coordinator to implement and expose the missing canonical
command byte encoder/digest, or explicitly expand W2-04's persistence permission
beyond **a public accessor**. This requires substantive codec implementation and
native vectors, not adapter wiring. Keep Core revision 1 unchanged. The accepted
D04 contract in schema-rev1-freeze.md requires SHA256 of the LHRequest1-domain
codec struct, command type and every request field (ID, epoch, preview token,
creation evidence, provenance included), explicit allowlisted ASCII field order,
canonical set ordering and rejection of malformed input. Initial coverage must
include CreateCharacter, AllocateAttributePoints and EquipItem, with distinct
command domains and mutation/reordering vectors. Reject unsupported types.

D06 growth mapping can be implemented in an authorized Framework integrator
helper if no shared implementation is supplied: SHA256 LHReward1 plus canonical
run, source kind, growth source key (CharacterId, ToLevel), and LevelGrowth
purpose; first 16 bytes map to four little-endian GUID words. Its native vectors
and zero/collision policy must be explicit. The brief's claim of an existing
W2-01 function is not supported by this baseline.

Work stops at this ownership checkpoint. No alternate codec, fixture fingerprint,
placeholder asset, invented content hash, or startup switch is introduced.
No new mechanics research or tuning is needed for this request. The existing
synthetic profile may be used as the expressly Prototype profile once shared
identity functions and mechanical/catalog closure hashes exist.

## Wiring requirements after the checkpoint

- Attach one character authority component to ALHPlayerState. Initialize it with
  the explicit Prototype profile and canonical identity callbacks. Bind creation,
  allocation and equipment to this owner; unsupported commands fail closed.
- Give the game-instance session owner immutable complete snapshots and save
  event subscriptions. Character Export updates only its owned fields; preserve
  world/session fields in the complete snapshot. Configure persistence with
  matching rules/catalog hashes, growth limit and reference validation.
- Bind a live presenter to the frontend controller with adapters that outlive it.
  Supply question/appearance catalogs and side-effect-free allocation/equipment
  reviews. Do not mint previews, rewards or request IDs during review.
- Save creation once after synchronous authority publication settles. Wait for
  its matching Succeeded event before travel to Dev_Movement. Failure must remain
  visible, retain the committed identity and permit save retry without creation.
  Replayed accepted commands must not request a second mutation save.
- Continue enumerates by GUID, requires recovery acknowledgment, validates and
  imports the selected complete save. All-invalid preserves the files and shows
  the persistence Unreadable event; never create a replacement automatically.
- Tear down the prior runtime; reconstruct a fresh PlayerState authority,
  inventory/equipment, base attributes, historical maxima and resources. Install
  derived GAS values before InitializeAvatar. The current player controller
  calls the dev initializer before avatar initialization: a canonical restore
  must bypass that fixture so it cannot overwrite restored resources.
- Serialized state uses frozen value records and stable IDs only. Actor/component
  pointers are transient owner bindings, never persisted fields. Suppress
  commands while saving/travelling, and save only completed action boundaries.
- Expose the journal presenter in gameplay; the existing controller's context
  switching alone does not construct the character/inventory screens. Wait for
  successful durability before confirmed quit; surface failure and allow retry.

## Required native integration tests (not implemented or run here)

`Lighthaven.Integration.Wave2.*` must exercise the actual live session adapter,
not merely repeat isolated codec tests:

1. Create two GUID-distinct characters, save each, destroy their runtime owners,
   reconstruct each and compare full canonical character/world/session state.
   Verify independence even with equal display names.
2. Equip and allocate through owner commands, persist each completed action,
   rebuild, compare inventory bindings, attributes, growth history and resources.
   Check derived GAS installation precedes avatar initialization.
3. Corrupt the newest generation; load the prior generation and require a
   Recovered event and explicit UI acknowledgment before continue.
4. Corrupt both generations; require a visible Unreadable error, preserve files,
   retain the selected identity and assert no new character is created.
5. Fail initial write/readback; require visible Failed, no travel/quit and retry
   of the same committed character. Repeated confirms/replays cannot duplicate it.

Use unique transient worlds, single initialization, per-test destruction,
InitInputSystem on spawned controllers, and GFrameCounter advancement per timer
tick. Worker UID is 0, so Automation/editor execution is prohibited here.

## G2 packaged Linux checklist for the coordinator

All items below are **NOT RUN**. Windows equivalents are deferred.
The full integration and native tests above must exist before this checklist can
be used as gate evidence. A baseline build does not establish these behaviors.

1. Build editor and game with
   `UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game`.
   Require both `Result: Succeeded`. As a non-root host user run
   `bash build/run-tests.sh Lighthaven`, retaining exact pass/fail lines.
2. Ensure the editor-generated L_Frontend and Dev_Movement maps are cooked and
   frontend startup is configured. Inspect `build/package-linux.sh` map arguments;
   do not assume its current dev-map defaults include L_Frontend. Build and launch
   the real Linux package. Record revision, build command, exit code and archive.
3. Create A and B through the UI. Record their stable GUIDs and displayed
   Prototype profile/provenance. Confirm each creation reaches Dev_Movement only
   after one successful initial save. Confirm both appear independently in select.
4. Equip A through the inventory screen and legally allocate points through the
   character sheet. Capture the completed save event and the resulting inventory,
   bindings, base/effective attributes, growth records and current resources.
   Confirm B is unchanged. Do not inject authority state or grant debug points to
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
   prevent travel/quit, retry the retained completed snapshot and observe success.
   Record process logs, save event sequences and observed state; screenshots alone
   do not prove independent persistence or successful writes.

## Validation and handoff

Inspection covered persistence, character, UI and Wave 1 integration documents,
frozen contracts, START-HERE, Wave 2/G2, module headers and command/restore paths.
Build observations, exact timings and the result revision are in the attempt
report. No engine/editor/package/play result or existing host test count is claimed
as observed runtime evidence by this worker. Next task: shared canonical command
identity implementation or scope amendment, then resume W2-04 and host G2.
