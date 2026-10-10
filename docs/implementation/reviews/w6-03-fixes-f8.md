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
