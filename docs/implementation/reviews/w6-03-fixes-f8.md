# W6-03fixg: F8 analysis and scope checkpoint

Evidence candidate; **F8 is not fixed in this submission**. Base `50db2c099fd6079da150e69eaa36d3428f534beb`. No Core, saved-shape, UI, Visual, generator or binary changes are proposed for integration by this attempt. This report follows the worker instruction to stop and report when the necessary change crosses owned paths; it is not a request to apply a policy silently.

## Replay lifetime findings

`Framework/Wave4/LHStage1Session.cpp:172–196` accepts UseAbility only for `Complete.Session.RequestEpoch`, retains every accepted GUID/digest/result and rejects new requests at4096. Replay lookup precedes capacity rejection. Neither kill settlement, `AcceptBoundary`, `Flush`, nor `OnSave` invalidates the request epoch. Ordinary durable settlement therefore does **not** make a cached activation ineligible to replay. `LHWave2Session.cpp:125` only clears the runtime cache for Fresh binding; it does not implement campaign epoch rollover. Character authority initialization issues the bootstrap epoch; Import/Export and save/load preserve it.

D04 defines the digest; it supplies no request ordering or expiry. D06 in `schema-rev1-freeze.md` explicitly identifies the arbitrary-GUID eviction problem and requires fresh epoch plus empty receipts to be committed atomically and durably at a completed boundary with no outstanding UI/async intents or callback publication, before issuing fresh intents. D16 allows runtime-only activation receipts, but does not say completed or saved activations are stale. `contracts-v1.md`, `schema-rev2.md` and `persistence.md` retain epoch matching and replay semantics. Rev2 adds UseItem without changing that lifetime policy.

The actual gameplay issuer is **outside this contract**: `Source/Lighthaven/UI/LHUIPresenter.cpp:190–206`. `FreshRequest()` uses a random `FGuid::NewGuid()` and the snapshot epoch; `UseAbility()` submits that request and `RetryCommand()` retains/reuses its closure. The controller goes through this presenter. The separate controller-issued random request at `LHPlayerController.cpp:320` is travel, not attack. A GUID therefore carries neither issuance order nor a trustworthy age. The request has no sequence/timestamp field. Ordering random GUIDs would reject fresh attacks unpredictably and would not implement an honest recent window.

## Why no eviction patch is submitted

- FIFO/LRU eviction, clearing on kill/save, or dropping cached results after impact would accept an evicted GUID as a new activation under the same epoch. That violates the brief's exact replay protection.
- Keeping an unbounded set of evicted GUIDs merely moves the memory growth from results to tombstones. A finite probabilistic filter either allows duplicate execution or eventually rejects fresh gameplay intents; it is not the requested long-session fix.
- Rotating only the in-memory epoch violates D06's durable publication boundary and affects all persisted commands, receipts and epoch-bound Light inputs. A proper campaign rotation must coordinate the outstanding presenter retry lifecycle, freeze new intent issuance until durability, preserve failure/retry behavior, and test rollback. Framework currently has no owner handshake for the presenter's outstanding intent/retry state. Adding a campaign rotation while assuming that state away would change a project invariant without evidence.

The smallest proposed solution is a **runtime UseAbility window with authority-issued ordered IDs**, leaving the campaign epoch and durable receipts alone. It needs a narrow additional UI ownership grant; it does **not** need a Core or saved-shape change:

1. Add an ability-intent issuance method to the UI/session seam in `UI/LHUIPresenter.h`, defaulted for mock owners, and have `UI/LHUIPresenter.cpp::UseAbility` call it. The Framework implementation issues a GUID encoding a runtime incarnation nonce and checked monotonic ordinal, together with the existing campaign epoch. The UI retains the entire issued request for retry.
2. Framework keeps at most4096 recent issuance slots/results, plus constant-size nonce/counter/floor metadata. Validate epoch, incarnation, issued ordinal and floor before the domain function. Below-window and previous-incarnation IDs return `InvalidRequest` without execution; retained accepted IDs compare digests and return the original result with `bReplay`, or `ReusedRequestId` for differing payload. Rejected-but-issued intents can retry while in the window. Explicitly document the below-window rejection as the task-authorized stale rule. Do not silently turn arbitrary caller GUIDs into new ordered requests.
3. Encode nonce/ordinal into existing GUID words; no serialized field or schema revision. Check exhaustion rather than wrapping; settle/fresh-bind nonce lifetime must prevent requests from a previous runtime incarnation from appearing new after reload. Reject unissued/forged/out-of-range IDs. Require exact native tests for over4096 accepted activations, recent replay, changed digest, below-window same/changed digest, rejected retries inside/outside window, unissued IDs, old epoch, fresh binding and reload.
4. Update the owned Integration fixtures to use that same issuer, then run SessionInventoryRollback and both map variants of the full suite. Do not insert reloads or free rewards to get the inventory fixture past F8.

This is a concrete implementation proposal, not an applied contract change. Needed additional paths: `Source/Lighthaven/UI/LHUIPresenter.h`, `Source/Lighthaven/UI/LHUIPresenter.cpp`; optionally the UI mock test owners if the defaulted seam proves insufficient. The worker owns neither UI file. No schema migration is requested. An integrator could instead choose D06 campaign rollover, but must supply and test its UI/durability handshake rather than simply erase receipts.

## Validation

Validation observations are appended below. Existing-suite runs, when completed, characterize the unchanged baseline only; they cannot establish F8 repair or inventory rollback coverage.

Linux UE5.8.3, uid1000. `UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game` initially stalled in UBA before completed actions; interrupted exit130. A retry with checkout-local UBA RootDir and executor flags also stalled and was interrupted exit130. Final retry additionally set `UnrealBuildAccelerator.SharedMemoryTempFile=true` (a relative writable backing-file path) and `AllowDetour=false` in ignored `Saved/UnrealBuildTool/BuildConfiguration.xml`: exit0; editor `Result: Succeeded`,73.04s; game `Result: Succeeded`,154.58s. No system/engine/HOME changes or downloads.

Initial pointer-suite probe omitted a writable LocalDataCachePath, logged no writable DDC nodes and attempted fallback; interrupted exit130 before automation results. Corrected invocation is in the canonical attempt `library/run.sh`, with checkout-local XDG config and LocalDataCachePath, memory DDC/InstalledNoZenLocalFallback, nullrhi/unattended/nosound/nop4/noshaderworker, HomeScreen disabled, and the explicit analytics privacy override. No host GUI/play or Windows validation is inferred.

| Run | Completed | Success | Success with warnings | Fail | Not run / in process | Assertion duration | Exit |
|---|---:|---:|---:|---:|---:|---:|---:|
| Full suite, pointer maps (`PointerFull`) |196|181|4|11|0 / 0|36.8884239197s|255|

Pointer failure names (exact):

- `Lighthaven.Integration.G4.DeathAndChurchRespawn`
- `Lighthaven.Integration.G4.DoorClearance`
- `Lighthaven.Integration.G4.EarnedMagicRoute`
- `Lighthaven.Integration.G4.EarnedMeleeRoute`
- `Lighthaven.Integration.G4.EarnedRangedRoute`
- `Lighthaven.Integration.G4.SessionFloorRouteAndReload`
- `Lighthaven.Integration.G4.SessionInventoryRollback`
- `Lighthaven.Integration.Wave3.ArrivalSafety`
- `Lighthaven.Integration.Wave4.B1SpawnOnContinue`
- `Lighthaven.Integration.Wave4.B1SpawnOnTravelArrival`
- `Lighthaven.World.LightingAudit`

All pointer-run failures have missing/unloadable generated-map evidence; inventory stops at the generated-hub precondition, not inventory filling. Summary counts and failing assertion excerpts are retained in `library/pointer-summary.txt`. No full index.json is kept in worker output.

All five playable maps were regenerated locally with the three generator scripts, `UE_ROOT=/tmp/f8-engine LH_NO_ZEN=1`; the temporary engine-path wrapper delegates to the real UE5.8.3 command editor and appends the same analytics/HomeScreen/DDC privacy options. This allows using the existing scripts without modifying their source. Every commandlet reports result0 and SavePackage moves the expected hub/B1/B2/B3/B4 outputs. Each wrapper script exits1 after reporting unloadable baseline-pointer packages. Generation evidence is in `library/gen-{hub,a,b}-excerpt.log`; no map or ReviewedArrivals file will be submitted.

Focused `Lighthaven.Integration.G4.SessionInventoryRollback` (`InventoryGenerated`), generated maps: exit255; **1 completed,0 succeeded,0 succeeded with warnings,1 failed,0 not-run/in-process**; assertion duration58.9418220520s. Diagnostic reports **378 completed kills /400 requested**, reason18 (`Busy`), and `Action request capacity reached; save and reload before issuing more attacks.` It reproduces F8 exactly. Inventory filling and full-inventory loot rollback assertions are still **unreached**, so no later inventory defect can be established by this run. There is no reload workaround in the fixture or implementation.


Full `Lighthaven` (`GeneratedFull`), generated maps: exit255; **196 completed,187 succeeded,8 succeeded with warnings,1 failed,0 not-run/in-process**; assertion duration165.1994934082s. The sole failing test is `Lighthaven.Integration.G4.SessionInventoryRollback`, with the same378/400 farm, reason18 and capacity message. All195 other tests pass. `library/generated-summary.txt` records counts, exact failure name and the failure/diagnostic excerpts. The focused and full runs establish no additional failure beyond F8 on these generated maps; neither reaches the inventory rollback assertions.

All five playable `.umap` pointers were restored after testing. Final `git diff --check` passes; only the two owned documentation files differ. Worker-output artifacts remain below1MiB (well below the40MiB cap). No source, schema, `.uasset`, `.umap`, or ReviewedArrivals changes are included.

## Handoff

Task ID: W6-03fixg (relaunch of fixf).
Base revision / result revision: `50db2c099fd6079da150e69eaa36d3428f534beb` / Deliverable candidate on this attempt branch; exact candidate OID in submission receipt and canonical report.
Contract revision: 1.
Owned paths / binary assets: this review report and `docs/implementation/g4-automated.md`; no submitted binary assets or source changes.
Behavior changed: none. Replay safety retained; F8 remains present. Analysis and a concrete UI/session-seam scope proposal submitted at the required ownership checkpoint.
Source-backed mechanics: no mechanics numbers changed.
Provisional tuning introduced: none.
Build/editor/cook/package/play checks actually run: successful editor/game builds and headless pointer/generated automation described above; local map generation. No cook/package/graphical play.
Results and evidence paths: this document; canonical attempt `report.md`, `library/*-summary.txt`, failing-run excerpts, command scripts. No full report JSON retained in worker output.
Checks not run and concrete missing prerequisite: new over4096/replay/stale-window native regressions require the proposed issuer/policy implementation and additional UI ownership; no such implementation is submitted. Full-inventory rollback assertions require fixing F8 first. Graphical/hardware/gamepad checks require their sessions; Windows deferred.
Known defects or remaining decisions: F8 remains; choose/authorize the two UI seam files for the proposed window, or implement a fully tested D06 campaign rotation handshake. No claim that every possible Framework-only rotation implementation is impossible; the proposed minimal runtime-window implementation crosses the specified ownership boundary. No further inventory defect established because earning still stops before inventory filling.
Next task and integration notes: coordinator review of proposal and scope, then implementation and required native regressions; rerun focused inventory and full suites. Do not treat this evidence submission as task repair, gate approval, capacity release or integration. No push, merge, system installation or SQLite edits.

## W6-03fixh: compact session receipts without combat capacity refusal

Evidence candidate for coordinator review, not gate approval. Base `2255d09270a1a6629cfe23e998757194547a9dd5`. Implements the coordinator's chosen unbounded runtime policy, superseding fixg's proposed ordered-window solution. No UI, Core, saved shape, epoch rotation or map-generator change.

`FLHAbilityReplayLog`, used directly by `FLHWave2Session::Execute(UseAbility)`, retains every accepted GUID mapped to a 256-bit BLAKE3 hash of the UTF-8 canonical `LHSave::RequestDigest`. There is no entry capacity refusal or eviction. Canonical encoding failure rejects without activation. Non-accepted requests remain unrecorded. Matching duplicates reconstruct Request, Accepted, None, sequence0 and bReplay=true; changed payloads return ReusedRequestId without invoking activation. Inspection of ExecuteUseAbility confirms it returns only a reason; the session's original acceptance sets no additional result fields. No historical full-result window is necessary. RuntimeAbilities.Reset remains exclusively at Fresh binding, as before; Continue preserves it. Existing blocked/paused/invalid epoch checks retain their prior order.

Native regression `Lighthaven.Review.W603.AbilityReplayLifetime` exercises the same helper used by Execute:5000 accepted activations; replay of first and last; original result reconstruction; changed digests for both; activation callback count stays5000; rejected retry; reset. It also casts Light through a real session, checks replay and changed payload, checks Continue binding, and checks Fresh clearing by reinstalling the same epoch fixture and accepting the original request anew. Light learning/requirements are isolated native fixture setup, not production freebies or inventory workarounds. The first synthetic test run failed because its target IDs were incomplete; fixed fixture canonical validity, then rebuilt and passed. Focused final replay run:1 completed,0 success,1 success-with-warnings,0 failures,0 not-run/in-process,0.4528588057s, exit0. Warning is existing GameplayCueNotifyPaths configuration.

### Memory

An entry's payload is16-byte FGuid +32-byte FBlake3Hash =48 bytes, with no FString allocation and no FLHCommandResult. Container links, allocation slack, sparse bits and hash buckets are additional. A native TMap probe using this exact key/value type and insertion order measures `GetAllocatedSize()` (container allocations, not allocator bookkeeping or process RSS):

| Accepted IDs | Payload bytes | Measured container bytes | MiB allocated |
|---:|---:|---:|---:|
|10,000|480,000|648,336|0.6183|
|100,000|4,800,000|6,000,092|5.7221|
|1,000,000|48,000,000|75,441,704|71.9468|

Measured amortized allocations are64.83,60.00 and75.44 bytes/entry respectively; they vary with TMap growth/slack. These are Linux UE5.8.3 measurements, not a contractual allocation guarantee. The request/digest temporaries are freed after each call. Memory grows for the accepted session lifetime by explicit policy. Ordered IDs or a coordinated D06 campaign rotation remain possible later refinements, outside this change. Hash comparison has the usual cryptographic collision assumption; no probabilistic eviction filter is used.

### Validation and further inventory finding

Linux UE5.8.3, uid1000. Initial two UBA builds stalled and were interrupted (exit130), despite the prescribed checkout-local XML. The engine's log still reported memfd backing. A temporary `/tmp/fixh-build-engine` wrapper passed `-UBASharedMemoryTempFile=true -NoUBA` directly to the real Build.sh; no engine or system file was changed. UE5.8 still uses its local UBA executor under NoUBA, with detouring disabled. Editor/game both Result: Succeeded,72.79s/122.28s. Final build after correcting fixture and adding memory probe:editor13.00s/game2.44s, exit0. The temporary wrapper and commands are retained in the canonical library. No large downloads.

Headless tests use the brief's checkout-local XDG_CONFIG_HOME, LocalDataCachePath, memory DDC/InstalledNoZenLocalFallback, nullrhi, unattended, nosound, nop4, noshaderworker, HomeScreen disabled and analytics/privacy overrides. All five playable maps were locally regenerated through the three existing scripts with a temporary command-editor wrapper adding those options. All three commandlets returned0 and saved expected maps; script exit1 followed baseline-pointer package errors. No fake packages, map edits or generated binaries are proposed for submission.

Focused `Lighthaven.Integration.G4.SessionInventoryRollback`:exit255;1 completed,0 success,0 success-with-warnings,1 failed,0 not-run/in-process;90.7994995117s. Farming400 kills now completes, legal vendor purchases fill40 slots, a real item-drop corpse is obtained, and full-inventory loot transfer reaches its assertions. The InventoryFull rejection and no-storage-write checks pass. Exactly two assertions fail:

- `exact live state rollback incl corpse and request journal`
- `independent durable rollback`

No fixture reload, synthetic drop, free reward or skipped assertion was introduced. This is a newly reached further defect, documented without patching around it. These equality failures alone do not establish inventory/corpse corruption. Source inspection identifies a candidate cause: Persist calls SyncResources before the domain rejects InventoryFull; SyncResources mutates Complete with component cooldowns/RNG/recovery and encounter clocks, while the fixture's reference S is the pre-call cached Snapshot. The last clock/Flush can also leave runtime state newer than durable state. Root cause and exact differing fields remain unverified; a follow-up should compare pre-call live/durable baselines and changed fields before choosing an atomicity repair or fixture boundary correction.

Full `Lighthaven` with generated maps (`GeneratedFull`):exit0;197 completed,188 success,9 success-with-warnings,0 failures,0 not-run/in-process;314.1766967773s. **SessionInventoryRollback passes in full-suite context**,120.4706954956s, including all full-inventory live/durable equality and no-write assertions; replay regression passes,0.8093720078s. Thus the focused equality failures above are context-dependent and were not reproduced by this full run. They are retained as unresolved evidence, not asserted to be a consistently reproduced production rollback bug. No code or fixture changed between focused and full runs. Investigation must account for this disagreement; no gate/hands-on conclusion is inferred.

Full `Lighthaven` with pointer maps (`PointerFull`):exit255;197 completed,181 success,5 success-with-warnings,11 failures,0 not-run/in-process;66.4662322998s. All failures have missing/unloadable generated-map evidence; inventory stops at the hub precondition. Exact failing names:

- `Lighthaven.Integration.G4.DeathAndChurchRespawn`
- `Lighthaven.Integration.G4.DoorClearance`
- `Lighthaven.Integration.G4.EarnedMagicRoute`
- `Lighthaven.Integration.G4.EarnedMeleeRoute`
- `Lighthaven.Integration.G4.EarnedRangedRoute`
- `Lighthaven.Integration.G4.SessionFloorRouteAndReload`
- `Lighthaven.Integration.G4.SessionInventoryRollback`
- `Lighthaven.Integration.Wave3.ArrivalSafety`
- `Lighthaven.Integration.Wave4.B1SpawnOnContinue`
- `Lighthaven.Integration.Wave4.B1SpawnOnTravelArrival`
- `Lighthaven.World.LightingAudit`

All five playable .umap LFS pointers and ReviewedArrivals.tsv are restored. Final git diff --check passes. Canonical worker output contains only summary counts, exact failure names, relevant failure excerpts, memory measurements and invocation scripts, below 1 MiB; no full index.json or full run logs. No binary assets are included.

### W6-03fixh handoff

Task ID: W6-03fixh.
Base revision / result revision: `2255d09270a1a6629cfe23e998757194547a9dd5` / Deliverable candidate on this attempt branch; exact OID in submission receipt and canonical report.
Contract revision: 1.
Owned paths / binary assets: Framework helper and session files, Review regression, this report and g4-automated.md; no Integration fixture changes or submitted binary assets.
Behavior changed: no UseAbility capacity refusal; compact accepted receipts retain replay protection for the existing runtime lifetime; original accepted results reconstructed with replay flag.
Source-backed mechanics: none changed.
Provisional tuning introduced: none.
Build/editor/cook/package/play checks actually run: Linux editor/game build; local generation of five maps; focused native replay and inventory tests; full suites once with generated maps and once with pointer maps. Exact exits, counts and durations above. No cook/package/graphical play.
Results and evidence paths: this review and canonical report.md; library/build-summary.txt, replay-summary.txt, memory-measurements.txt, inventory-summary.txt, inventory-failure-excerpt.log, generated-summary.txt, generated-inventory-result.txt, pointer-summary.txt and pointer-failure-excerpt.log; wrapper/run scripts and generation excerpts.
Checks not run and concrete missing prerequisite: focused/full inventory disagreement requires changed-field/baseline diagnostics to establish its cause; no such diagnostic rerun claimed. Graphical/hardware/gamepad coverage requires separate sessions; Windows packaging remains deferred because there is no Windows machine. No gates are approved by this submission.
Known defects or remaining decisions: focused inventory equality failures occur despite all full-suite assertions passing; retained unresolved. Runtime receipt memory grows by accepted-ID count as authorized. Ordered IDs or D06 rotation are later refinements. No UI/Core/saved-shape changes, push, merge, system installation or SQLite edits.
Next task and integration notes: coordinator review/integration; investigate context-dependent inventory equality failures without bypassing earning or rollback checks. Restore/generated map handling and privacy invocation scripts are recorded for reproducibility. This report is an evidence candidate, not task verification, gate approval or worker-capacity release.
