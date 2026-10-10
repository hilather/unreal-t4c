# W6-03fixe: corpse attack sight and repeated G4 evidence

Evidence candidate, not gate approval. Base `019f31b79efcdd21dbe5906ad1fb0cc8cd892fa0`; Linux UE5.8.3, uid1000. Windows deferred. No Core/schema, generator, Visual, binary asset, or reviewed-arrival changes are submitted.

## F6 measurement before the fix

A temporary diagnostic in `InRangeAndSight` replaced the visibility test with a single-hit trace and logged source/target centers, distance, configured range, hit actor/component/class, start penetration, and a 1cm Visibility overlap at the source center. The original fixture and collision behavior were retained for this measurement. `DiagnosticG4Retry` completed eight tests (four pass, four fail); all four failures were the earned combat command with OutOfRange. Diagnostic logs are in the canonical attempt library. Temporary production logging/overlap work is removed from the submitted source.

| Rejected fixture | Player center (cm) | Enemy center (cm) | Distance / range (cm) | Blocking actor |
|---|---|---|---|---|
| EarnedMagicRoute | (1650,1500,27) | (1500,1500,27) | 150.000 / 200.000 | LHEnemyCharacter_7 |
| EarnedMeleeRoute | (750,1500,27) | (600,1500,27) | 150.000 / 200.000 | LHEnemyCharacter_5 |
| EarnedRangedRoute | (1650,1500,27) | (1500,1500,27) | 150.000 / 200.000 | LHEnemyCharacter_7 |
| SessionInventoryRollback | (3450,400,27) | (3300,400,27) | 150.000 / 200.000 | LHEnemyCharacter_8 |

Every hit was `CollisionCylinder`, actor class `LHEnemyCharacter`, blocked=1, startPenetrating=0; the source-center overlap returned no blocking geometry. The requested and resolved targets were the same living generation1 actor, playerHP30. These measurements establish sight blockage, not excessive distance or a wall at the trace start. The overlap measures the **start point**, not clearance of the entire player capsule; this spatial-command fixture still makes no locomotion/path-feel claim.

The blocker actors are earlier initial-spawn actors retained while generation1 occupies the spawn. `MarkCorpse` explicitly retains Visibility blocking for loot. Excluding retained corpses clears the four early OutOfRange failures; the permanent real-collision regression independently demonstrates a retained corpse blocks a raw Visibility ray while the next-life attack succeeds.

## Submitted behavior

Combat sight adds only `ALHEnemyCharacter::IsCorpse()` actors to its ignored actors. The original finite distance/range check and solid Visibility trace remain. Capsule Visibility remains enabled: cursor targeting, nearest-corpse selection, TakeLoot's life lookup, and corpse identity are preserved. No collision channel/schema expansion, range increase, navigation bypass, enemy RNG change, reward grant, or reload workaround was added.

`Lighthaven.Review.W603.LiveGenerationLookup` now also uses real capsule collision with a corpse between an attacker and the next life. It checks raw Visibility can hit the corpse, combat validation accepts through it, FindByLife still resolves the corpse, and a living blocker still causes OutOfRange. The earned fixture reports completed farming kills and the session status when rejected.

## Four required stability runs

All five playable maps were locally generated using the three scripts in `build/`, with `UE_ROOT=/home/brewerm/Downloads/unreal`, `LH_NO_ZEN=1`. Each script exited1 because baseline LFS pointer packages were unloadable; commandlets reported result0 and SavePackage moved all five generated maps into Content. Map pointers are restored before submission. No hand map/arrival edits.

| Run | Completed | Success without warnings | Success with warnings | Fail | Not run / in process | Assertion duration (s) | Exit |
|---|---:|---:|---:|---:|---:|---:|---:|
| G4Run1, focused | 8 | 3 | 4 | 1 | 0 / 0 | 159.968200684 | 255 |
| FullRun1 | 196 | 187 | 8 | 1 | 0 / 0 | 173.540390015 | 255 |
| FullRun2 | 196 | 187 | 8 | 1 | 0 / 0 | 170.553985596 | 255 |
| G4Run2, focused | 8 | 3 | 4 | 1 | 0 / 0 | 173.582931519 | 255 |

The sole failure in **every** run is `Lighthaven.Integration.G4.SessionInventoryRollback`: `legal earned combat command`, reason18 (Busy), after **378 completed kills of 400 requested**. Every other test passes. Earned Magic completes its 200-kill farm and full route; Melee and Ranged complete their 60-kill farms and full routes. All four DeathAndChurchRespawn results pass. Comparing exported JSON states finds no differences between the two full runs, two focused runs, or each focused run and the matching full-suite G4 tests. No shared-state leak is reproduced, so no speculative isolation/owner change is submitted. This checkout discovers196 tests; the brief's coordinator host201 count is not this checkout's observed count.

G4Run1/FullRun1/FullRun2 used the temporary rejection diagnostics with the corpse fix; G4Run2 used the final combat implementation without that observer. Both versions retain the same finite range and blocking-sight semantics. A subsequent fixture-only status diagnostic is checked in the final targeted run below.

## Next defect: activation replay-cache capacity

`Framework/Wave4/LHStage1Session.cpp` rejects a new UseAbility request once `RuntimeAbilities.Num() >= 4096`, setting `Action request capacity reached; save and reload before issuing more attacks.` This cache retains accepted runtime activations rather than one entry per kill, so the 400-kill inventory fixture hits it before returning to the vendor. The four runs stop identically at378 kills. It does not reach inventory filling or the full-inventory loot rollback assertions. No claim of passing those assertions is made. This is a separate long-session capacity defect, not attack range/sight or cross-test state. Changing replay/epoch lifetime policy or inserting reloads to hide the cap is intentionally left to a separate task.

## Commands, environment and validation

Exact headless invocation is retained as `library/f6-run.sh`. It takes filter and report name and invokes UnrealEditor-Cmd with Automation RunTests, Quit, nullrhi/unattended/nosound/nop4/noshaderworker, memory DDC and InstalledNoZenLocalFallback, checkout-local config/cache, report export, HomeScreen disabled, NoAnalytics/NoCrashDialog, and `-ini:EditorSettings:[/Script/UnrealEd.AnalyticsPrivacySettings]:bSendUsageData=False`.

Executed filters/names: `Lighthaven.Integration.G4 G4Run1`, `Lighthaven FullRun1`, `Lighthaven FullRun2`, `Lighthaven.Integration.G4 G4Run2`. Each library subdirectory contains index.json and each matching .log contains editor output. No graphical play observation is inferred from automation.

Initial standard build was interrupted (exit130) without completed actions. A direct -NoUBA retry failed exit6 trying to create the read-only worker home's `.epic`; another -NoUBALocal retry was interrupted exit130. Engine source explains UE5.8 always selects UBA, using non-detoured actions when disabled. Checkout-local Saved build configuration disables both executor flags and sets UnrealBuildAccelerator.RootDir to Saved/UBA. No HOME/system/engine edits. Diagnostic editor build then succeeded exit0,176.88s. Fixed standard editor/game build both succeeded exit0,62.44s/206.54s. After removing observers, standard editor/game build succeeded exit0,13.96s/62.69s. Final status-diagnostic standard editor/game build succeeded exit0,39.15s/4.61s. Logs are retained in library.

Two early test probes were terminated by sandbox rejection of Epic datarouter analytics; -NoAnalytics alone did not disable editor usage telemetry. No complete run is claimed for those probes. One prematurely launched full process was interrupted exit130 before any test completed to keep suite runs sequential. Those attempts do not count toward the four runs above.

Final submitted-source targeted checks:

- `Lighthaven.Review.W603 ReviewFinal`: 3 completed: 1 success without warnings, 2 with warnings, 0 failures/not-run/in-process; assertion duration0.126743749s; exit0. All three tests pass, including the real-collision corpse/living-blocker regression.
- `Lighthaven.Integration.G4.SessionInventoryRollback InventoryCapacityFinal`: 1 completed: 0 success, 1 failure, 0 not-run/in-process; assertion duration58.5613059998s; exit255. Session status explicitly reports `Action request capacity reached; save and reload before issuing more attacks.` Completed378/400 kills, reason18.
- All five pointers restored; `git diff --check` exit0. No maps or ReviewedArrivals changes remain.

## Handoff

Task ID: W6-03fixe.
Base revision / result revision: `019f31b79efcdd21dbe5906ad1fb0cc8cd892fa0` / submitted Deliverable candidate.
Contract revision: 1.
Owned paths / binary assets: LHCombatComponent.cpp; Integration/LHWave2Tests.cpp; Review/LHW603Tests.cpp; g4-automated.md; this report. No binary assets.
Behavior changed: retained corpses excluded from attack sight; loot Visibility and life targeting preserved; permanent collision regression and actionable route-limit diagnostics.
Source-backed mechanics: no mechanics numbers changed.
Provisional tuning introduced: none; regression coordinates are test geometry, not tuning.
Build/editor/cook/package/play checks actually run: editor/game builds and headless automation above; no cook/package/graphical play.
Results and evidence paths: this document; canonical attempt report.md and library/.
Checks not run and concrete missing prerequisite: graphical/hardware/gamepad checks require a graphical/hardware session; Windows deferred. No validator rerun claimed. Inventory rollback assertions remain unreached because of the4096 activation limit.
Known defects or remaining decisions: SessionInventoryRollback stops at378/400 kills with Busy; long-session activation replay cache needs a separate policy/fix. Source-center overlap does not certify full player-capsule clearance or locomotion realism.
Next task and integration notes: coordinator review of corpse-sight fix and repeated evidence; separate task for activation replay-cache capacity followed by the still-unreached inventory rollback test. No push, merge, gate approval, capacity release or SQLite edits.
