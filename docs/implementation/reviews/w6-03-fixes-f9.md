# W6-03fixi: F9 session rejection atomicity

Evidence candidate for coordinator review; no task verification, gate approval, integration or capacity release is claimed. Base `54f919e4da7d86d58ddc7fa03aa0da53858de14b`. Contract revision 1. Result is the Deliverable candidate on this attempt branch; exact OID is in the submission receipt and canonical worker report.

## Measured defect and baseline distinction

Diagnostics were added before changing production code. Initial focused `MeasureFocused` failed both original equality assertions after real earning, filling 40 inventory slots with legal purchases, and finding an actual catalog item-drop corpse. InventoryFull rejection and no-storage-write assertions passed. Live rejection changed `Session.ManaRegenFractionalSeconds.Value` from approximately `0.900007` to `3.000007`, and `Session.Cooldowns.Num` from 5 to 4. Durable-before versus durable-after had no differing fields.

The retained helper `Review/LHSnapshotDiff.h` now normalizes both snapshots with the production Encode/Decode codec, recursively compares reflected leaves, emits doubles at 17 significant digits, reports array lengths/tail entries and exports native structs. This avoids treating canonical keyed-array sorting or generated envelope checksums as gameplay changes. The equality oracle remains the original full canonical byte comparison; no fields are excluded from the rollback assertions.

With unchanged production code, two canonical focused runs passed, then `CanonicalFocused3` failed both original assertions. Its exact live mutation was:

| Field | Before rejection | After rejection |
|---|---|---|
| `Session.ManaRegenFractionalSeconds.Value` | 1.5000076219439507 | 3.6000076234340668 |
| `Session.Cooldowns.Num` | 4 | 3 |
| `Session.Cooldowns[3]` in canonical order | Player `Attack.Melee.Basic`, ExportText remainder 1.500000 seconds | Removed |

The removed cooldown owner has run `01A127D85CF87DEBAEA3CBA610A3A0D8`, instance `01A127D85CF97BE780F0E6ADD6137517`, area `Area.LighthavenTempleDistrict`. The other cooldown rows were unchanged. No inventory, corpse contents, request journal or gameplay RNG changes were reported for this earned-loot rejection. The deterministic native baseline separately demonstrated that the same pre-validation capture can also change `Session.GameplayRng`'s `RNG.Combat.State[0..3]`, fractional recovery and cooldowns.

The durable snapshot did **not** mutate on rejection. In the failing canonical run, its timer values were already 2.1 simulated seconds behind the live snapshot before the request. The following are the exact pre-existing differences, all under canonical `World.Areas[0]` (`Area.TempleB1`). The same differences remained after rejection; `durable-rejection` emitted none.

| Field below `World.Areas[0]` | Pre-request live | Pre-request durable |
|---|---:|---:|
| `Encounters[3].RespawnRemainingSeconds.Value` | 117.89999999850988 | 120 |
| `Encounters[13].RespawnRemainingSeconds.Value` | 69.599999964237213 | 71.699999965727329 |
| `Encounters[14].RespawnRemainingSeconds.Value` | 40.199999943375587 | 42.299999944865704 |
| `Encounters[16].RespawnRemainingSeconds.Value` | 8.6999999210238457 | 10.799999922513962 |
| `Corpses[0].CleanupRemainingSeconds.Value` | 117.2999998703599 | 119.39999987185001 |
| `Corpses[1].CleanupRemainingSeconds.Value` | 129.89999987930059 | 131.99999988079071 |
| `Corpses[2].CleanupRemainingSeconds.Value` | 249.59999996423721 | 251.69999996572733 |
| `Corpses[3].CleanupRemainingSeconds.Value` | 58.499999828636646 | 60.599999830126762 |
| `Corpses[4].CleanupRemainingSeconds.Value` | 33.299999810755253 | 35.399999812245369 |
| `Corpses[5].CleanupRemainingSeconds.Value` | 163.49999990314245 | 165.59999990463257 |
| `Corpses[6].CleanupRemainingSeconds.Value` | 220.19999994337559 | 222.2999999448657 |
| `Corpses[7].CleanupRemainingSeconds.Value` | 188.69999992102385 | 190.79999992251396 |
| `Corpses[8].CleanupRemainingSeconds.Value` | 79.499999843537807 | 81.599999845027924 |
| `Corpses[9].CleanupRemainingSeconds.Value` | 1.799999788403511 | 3.8999997898936272 |
| `Corpses[10].CleanupRemainingSeconds.Value` | 297.89999999850988 | 300 |
| `Corpses[11].CleanupRemainingSeconds.Value` | 100.49999985843897 | 102.59999985992908 |

`contracts-v1.md` (command result semantics, line 99) requires rejected commands to apply no mutation, and its corpse row requires intact full-inventory rejection. `persistence.md` (completed-action boundary, line 31) saves cooldowns, fractional recovery, RNG and encounter state at completed boundaries. The existing `StartEncounters::AdvanceRespawns` callback explicitly retains timer progress in memory until the next completed save/travel boundary. Nothing requires arbitrary live timer progress to already equal the preceding durable generation. Accordingly the test now takes an independent durable baseline before rejection and compares durable-after against that baseline, while retaining exact live-before/live-after equality and the no-write check. It does not advance clocks, force a new save or bypass the rejection to establish equality.

## Production repair and regression

`CaptureResources` writes only a supplied candidate snapshot and validates against a copy of character authority. `Persist` captures into `Next`, validates the domain, imports/commits the request there, and publishes only upon acceptance. Rejection no longer imports runtime resources into the live authority or `Complete`. `SyncResources` uses the same capture helper at legitimate settlement/save/travel boundaries. Capture is read-only with respect to combat components; dead Light is removed from the candidate without clearing runtime Light as a capture side effect. Existing death handling clears runtime Light explicitly. Derived-stat installation is preflighted before accepting a boundary.

Allocation and equipment had the same sync-before-rejection pattern. They now execute against a captured snapshot and copied authority and publish only new accepted results. Rejected commands and replays discard the candidate. No Core, saved shape, schema revision, UI, Visual, map-generator or mechanics change is included. Schema revision 2 remains frozen.

Native `Lighthaven.Review.W603.RejectedResourceCapture` deliberately sets newer runtime fractional recovery (0.375), combat seed (12345) and melee cooldown (1.25), with no world tick or order dependence. A structurally valid request for a missing inventory instance reaches the same Persist domain-rejection path. It checks exact live and independent durable rollback, no writes after Flush, untouched runtime values, rejected allocation/equipment, and a subsequent legitimate save capturing the retained resources. Its pre-fix version failed alone and in the baseline full suite. Final native validation uses the expanded regression. The earned integration fixture still farms 400 kills and obtains real loot; no reload workaround, synthetic loot or free rewards was added.

## Validation

Observed Linux uid 1000, installed UE 5.8.3. Final non-adaptive unity editor/game builds both returned 0 and `Result: Succeeded`: editor 72.59 seconds, game 138.79 seconds. `-DisableAdaptiveUnity -UBASharedMemoryTempFile=true -UBANoDetour -NoUBA`, checkout-local XDG/UBA directories and `UBA_FILE_MAPPING_DIR` were used. Ignored BuildConfiguration sets unity true, adaptive unity false and UBA detouring disabled. The generated unity test translation unit includes both LHWave2Tests.cpp and LHW603Tests.cpp; no adaptive-exclusion report occurred.

Four initial builds were interrupted (exit 130) while investigating quiet UBA output; a same-process probe later confirmed active Clang compilation. Early helper builds failed with an include path and cooldown-overload error (exit 6), both corrected before the successful diagnostic build (62.13 seconds). Canonical measurement build also succeeded (68.36 seconds). Nonfatal UBA action-result-store warnings remained. No engine/system installation or modification was made.

All five playable maps were generated locally with the three native generation commandlets. Each reported commandlet result 0 and saved its expected maps. Process exits were 1 after baseline map/art-pointer load errors; this distinction is preserved in the generation excerpts. For testing, 26 committed B1 art files (4,453,783 bytes) were copied from the host checkout only after matching each committed LFS SHA-256 and size. No downloads or new assets. All local binary changes and ReviewedArrivals output are restored before submission.

The headless invocation is retained in canonical `library/run.sh`: checkout-local XDG/DDC, nullrhi, unattended, nosound/nop4, HomeScreen disabled, analytics/privacy overrides, ReportExportPath under ignored Saved/F9 and TestExit on empty automation queue. No full logs or index.json are retained in worker output. Table durations are Automation assertion durations, not process wall times.

Pre-fix measurement (the full suite includes the added deliberately failing native regression):

| Run | Completed | Success | Success with warnings | Fail | Not run | In process | Assertion seconds | Exit |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| MeasureFocused | 1 | 0 | 0 | 1 | 0 | 0 | 63.371861 | 255 |
| MeasureFull | 210 | 200 | 9 | 1 | 0 | 0 | 264.133301 | 255 |
| MeasureNative | 1 | 0 | 0 | 1 | 0 | 0 | 0.141279 | 255 |
| CanonicalFocused | 1 | 0 | 1 | 0 | 0 | 0 | 67.122643 | 0 |
| CanonicalFocused2 | 1 | 0 | 1 | 0 | 0 | 0 | 79.491379 | 0 |
| CanonicalFocused3 | 1 | 0 | 0 | 1 | 0 | 0 | 91.281876 | 255 |

Exact baseline failure names: `MeasureFocused` and `CanonicalFocused3`: `Lighthaven.Integration.G4.SessionInventoryRollback`; `MeasureFull` and `MeasureNative`: `Lighthaven.Review.W603.RejectedResourceCapture`. All 209 pre-existing tests passed in MeasureFull, including inventory rollback. This records the disagreement rather than inferring baseline repair from a full-suite pass.

Final candidate, locally generated maps. Expected: 1 test in each focused/native run and 210 in each full suite (209 existing tests plus the new regression). Observed:

| Run | Completed | Success | Success with warnings | Fail | Not run | In process | Assertion seconds | Exit |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| NativeFinal | 1 | 0 | 1 | 0 | 0 | 0 | 0.086057 | 0 |
| Focused1 | 1 | 0 | 1 | 0 | 0 | 0 | 95.282318 | 0 |
| Focused2 | 1 | 0 | 1 | 0 | 0 | 0 | 95.641029 | 0 |
| Focused3 | 1 | 0 | 1 | 0 | 0 | 0 | 73.912697 | 0 |
| Full1 | 210 | 201 | 9 | 0 | 0 | 0 | 213.517487 | 0 |
| Full2 | 210 | 201 | 9 | 0 | 0 | 0 | 223.100693 | 0 |

Final failures by name: none observed.

## Handoff

Task ID: W6-03fixi.
Base revision / result revision: `54f919e4da7d86d58ddc7fa03aa0da53858de14b` / Deliverable candidate; exact candidate OID in submission receipt and canonical report.
Contract revision: 1.
Owned paths / binary assets: Framework/LHWave2Session.{h,cpp}, Framework/Wave4/LHStage1Session.cpp; Integration/LHWave2Tests.cpp; Review/LHSnapshotDiff.h and LHW603Tests.cpp; this review report. No submitted binary assets.
Behavior changed: rejected resource capture no longer publishes into live state; durable rollback compares its own independent baseline; accepted boundaries still capture runtime resources.
Source-backed mechanics: no mechanics values changed.
Provisional tuning introduced: none; seeded regression resources are fixture values only.
Build/editor/cook/package/play checks actually run: editor/game non-adaptive unity builds, three native map generators, measured focused/full/native baseline runs, final native run, three final focused inventory runs and two final full Lighthaven suites, diff/scope checks.
Results and evidence paths: this review; canonical attempt report.md and library/automation-summary.txt, trimmed failing diffs, build/generation summaries and invocation scripts. Worker output stays below 40 MiB.
Checks not run and concrete missing prerequisite: no cook/package/graphical play or hands-on/gamepad session was requested/run; those observations require the respective package or interactive session. Windows validation remains deferred without a Windows machine. This submission supplies Linux automated evidence only.
Known defects or remaining decisions: final automation results are recorded above; coordinator review and host integration/retest remain outstanding. Baseline discrepancy is measured and explained, not dismissed as a fluke.
Next task and integration notes: coordinator review of rejection atomicity and the durable-baseline correction, then host rerun/integration. No push, merge, memory-projection edit, SQLite change or gate approval.
