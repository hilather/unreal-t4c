# W4-15c automated evidence candidate

Base `fe5b3aa39f46633dd41b3cadae089daf581e7ca1`, contract revision 1.
This candidate completes the durable death/church fixture and adds executable
acceptance tests for earned builds, safe respawns and session inventory rollback.
**Earned-route execution is blocked by a production target-resolution defect;
G4 is not approved.** No production code, generators, schemas or mechanics changed.

## Checklist mapping

All names below have prefix `Lighthaven.Integration.`.

| Checklist | Tests / actual assertions | Limit or outstanding work |
| --- | --- | --- |
| Fresh creation; legal purchase/replay/save failure/RetrySave | `G4.LegalPurchaseReplayAndRetrySave`: presenter creation, HP30/MP10/gold100, real purchase, replay, injected storage failure, widget RetrySave, independent reload | Existing W4-15 coverage retained |
| Hub → B1–B4 → return; reload every floor; arrival grants nothing | `G4.SessionFloorRouteAndReload`: session RequestTravel, generated portals, save adapter, encounter population, exact independent stored arrival on every floor, unchanged arrival XP/gold | Positions isolate command validation; connecting corridors are not walked. Reload uses an independent store, not a process restart |
| Occupied arrival refused | Same route test: real blocking capsule placement query, refusal, no arrival write, source checkpoint retained | Recovery hook acknowledges restoration without reloading source geometry |
| Floor change during pending action | Same route test: real catalog enemy GAS windup on each basement departure, refused travel, unchanged adapter token/write count, timer completion then retry | Player attacks have zero impact delay; enemy windup is the reachable pending boundary. Already implemented, retained |
| Enemy aperture clearance | `G4.DoorClearance`: generated lintels and B1/B2 flat routes; eligible runtime capsules selected by registry home/leash; five special B4 openings | Leash intersection is conservative; no enemy nav-agent stair traversal claim |
| Player stairs | Same clearance test: native CharacterMovement and capsule, seven generated stairs, grounded endpoints | Stops before portal walls; no controller feel or cross-level locomotion claim |
| Simultaneous lethal damage and durable death | `G4.DeathAndChurchRespawn`: two populated encounters request same-frame catalog attacks; supported timers cause death; exactly one publication **and one transaction boundary**; dead snapshot independently reloads exactly; later impacts cannot advance sequence | Positioning isolates impacts. No HandlePlayerDeath call, health edit or fabricated checkpoint |
| Death → church; inventory preserved | Same test: real initialized GameInstance-owned session/save/travel subsystems, real local player/controller, HUD dead state, presenter Respawn, asynchronous durability, generated reviewed church, production `PlaceSessionArrival`, exact arrival transform, HUD alive, item identities/quantities preserved, exact independent recovery reload | Load adapter is synchronous; OpenLevel callback is replaced with a fixture callback. Production placement, safety checks and death bridge execute. Not a graphical/process-restart test |
| Fresh earned melee/ranged/magic routes | `G4.EarnedMeleeRoute`, `EarnedRangedRoute`, `EarnedMagicRoute`: fresh creation, generated nav worlds, session portal route, rat kills/gold transfers, real skill training; ranged vendor bow/quiver and equipment; magic earned INT allocation and FireDart learning; floor combat/Balork and Kiran return | **Blocked** at first stale respawn target (defect below); training, deep floors, Balork and completion assertions are present but not reached. Do not claim any complete earned route |
| Safe respawns and second Balork life | Earned fixtures build real navigation and use active session/AI clocks. Melee route includes 900 active seconds, new boss generation, second kill and one unique claim | Ordinary respawns reach live new generations; session cannot attack a new generation when a retained old corpse resolves first. Second Balork life/kill assertions remain unreached. Existing route separately observes missing-navigation refusal and the 899/900 boundary |
| Full-inventory loot rollback through session | `G4.SessionInventoryRollback`: earn 400 rat kills, spend earned gold on vendor bows to the real 40-slot limit; obtain an actual RNG corpse item; session transfer must reject InventoryFull; exact live/durable rollback including corpse and request journal, no write | **Blocked** during earning by the same respawn target defect. Filling/rollback assertions are implemented but not reached; no direct inventory edits or synthetic drops |

## Production defect: retained corpse wins respawn target lookup

Reproduction uses `G4.EarnedMeleeRoute` (the other two routes and inventory test
reach the same failure):

1. Create a legal fresh character; enter the generated B1 through its authored
   hub portal and the session/save adapter.
2. Kill and loot Brown Rats with session melee commands at normal cooldown times.
   Enemy decisions and GAS windups advance through the real session clock.
3. Retreat to the authored entrance; advance active time in <=0.1s substeps.
   Real navigation controls report a main nav data set and populated tiles; the
   safety policy permits ordinary respawns. Do not replace safety with `true`.
4. Select a living rat in a new life generation and issue a session attack.

Expected: the session resolves that living generation and accepts the eligible
attack. Actual: `InvalidLifeState` (reason4); the requested actor is alive but the
resolved actor is an older dead generation for the same spawn slot. The added
`TARGET requested=... generation=... alive=... resolved=... generation=... alive=...`
diagnostic records both actors and the player's live HP.

Source explanation: `ALHEnemyCharacter::GetEntityId` uses the spawn slot for
InstanceId, omitting life generation. `ULHEncounterDirector::FindByEntity` returns
the first matching actor in `Enemies`, without preferring an alive generation.
Director death handling retains corpse actors; protected/unclaimed corpse loot
can keep an older actor available across respawn. `Execute(UseAbility)` therefore
passes that corpse combat component into normal life-state validation.

The smallest follow-up should review live-target selection in the director and
preserve corpse-loot addressing/replay semantics. This worker makes **no**
production fix or shared identity/schema proposal. Do not bypass this failure by
direct attacks on the new component, deleting corpse actors, forcing cleanup,
editing snapshots, suppressing errors or granting resources. Rerun all four tests
after the production owner fixes the lookup; subsequent failures may still exist
in the currently unreachable portions of those tests.

## Fixture and provenance rules

Death owns a real `UGameInstance` initialized with `InitializeStandalone`, its
world context, subsystems and local player. World replacements preserve that
context. Controller possession occurs under the ordinary travel-freeze seam;
church placement runs explicitly after the destination is ready. File storage is
real and asynchronous; Flush pumps game-thread completions with a bounded 10s
wait. Characters receive unique IDs; generated saves stay under Saved/.

Earned tests use in-memory storage with the production save store/session and
travel adapter, plus independent stores for decoding. The fixture binds the
public gameplay clock flag that the production controller binds on possession.
No persisted state, HP, XP, gold, inventory, cooldown or RNG grant/edit is used.
Generated maps are loaded into fresh editor package instances for navigation.
Create the nav system without initializing it, disable its editor async-load wait
after finishing assets, then initialize/build the actual Recast data. Setting the
wait property after initialization leaves an existing async-load build lock.

World/GAS timers, mana component recovery and session/AI time advance together in
<=0.1s substeps. Combat approach/retreat and service/portal positions isolate the
command path. Enemy AI decisions and windups remain enabled; retreat evades normal
impact range checks. These fixtures do not prove CharacterMovement kiting,
nav-following enemy traversal, corridor accessibility or controller feel.

All mechanical values come from existing catalogs/profile and their retained
Bible/Prototype provenance. Starting kit, 40 entry slots, guaranteed midpoint
rat gold, attack timing, training prices and drop odds retain their existing
Prototype/Modernized classifications. Farming counts and iteration bounds are
scenario controls, not new mechanics or authentic progression-time claims.
Carry weight remains unlimited/deferred; this test concerns entry slots.

## Manual-only / deferred

Lighting/readability, graphical Light/effects/animation/audio, controller feel,
graphical/package launch and in-play render stalls require rendering, audio or
interactive input absent from null RHI. Windows packaging/launch remains deferred
by owner decision. Earned builds, inventory rollback and second boss life are
**automation blocked**, not manual-only. Enemy navigation stair traversal and
exhaustive corridor locomotion remain additional automation work; clearance sweeps
and player stair checks do not establish them.

## Execution and evidence

UE5.8.3 Linux, `/home/brewerm/Downloads/unreal`, uid1000. Both standard targets
reported `Result: Succeeded` with `UE_ROOT=/home/brewerm/Downloads/unreal bash
build/build-linux.sh --game` on the final source. Early UBA work was interrupted;
exported UBT actions were executed in dependency order to bootstrap binaries, then
the normal build ran. One intermediate compile read a file during a fixture edit
and reported null characters; the source contained zero NULs and the stable-source
rerun succeeded. These intermediate outcomes are not production defects.

Generated the five playable maps locally with LHGenerateHubMap,
LHGenerateBasementAMaps and LHGenerateBasementBMaps. Initial generation processes
returned1 after baseline LFS-pointer startup errors but recorded real saves.
ArrivalSafety returned0 with9/9 PASS; its reviewed output was installed locally
and the hub regenerated for church placement. No map or review TSV is submitted.

Final full-suite command:

```sh
XDG_CONFIG_HOME="$PWD/Saved/BuildEnvironment/config" \
 /home/brewerm/Downloads/unreal/Engine/Binaries/Linux/UnrealEditor-Cmd \
 "$PWD/Lighthaven.uproject" \
 '-ExecCmds=Automation RunTests Lighthaven; Quit' \
 -DDC-ForceMemoryCache -ddc=InstalledNoZenLocalFallback \
 "-LocalDataCachePath=$PWD/Saved/DerivedDataCache" \
 -nullrhi -unattended -nosound -nop4 -noshaderworker -nocrashreports \
 -stdout -FullStdOutLogOutput \
 '-ini:EditorSettings:[/Script/UnrealEd.AnalyticsPrivacySettings]:bSendUsageData=False' \
 '-ini:Engine:[ConsoleVariables]:HomeScreen.EnableHomeScreen=0' \
 "-ReportExportPath=$PWD/Saved/W415cFinalReport"
```

Final submitted-source suite: **185 completed: 176 success without warnings,
5 success with warnings, 4 failure, 0 not run, 0 in process**. Process exit255;
reported test duration54.90135192871094s. G4:8 completed,4 success,4 failure.
The failures are `G4.EarnedMagicRoute`, `G4.EarnedMeleeRoute`,
`G4.EarnedRangedRoute`, and `G4.SessionInventoryRollback`, all with the same
confirmed requested-generation1-alive / resolved-generation0-dead diagnostic;
player HP30. Example melee: requested `LHEnemyCharacter_17`, resolved
`LHEnemyCharacter_0`. Rat drops/retained corpses depend on the real run RNG;
the exact actor/slot and first failing kill can vary. No RNG stream was changed.

Report: `Saved/W415cFinalReport/index.json`. Canonical worker output retains
`library/final-index.json`, `final-suite.log`, `failures.txt`, `final-build.log`,
`arrival-index.json`, `arrival-review.log`, `arrival-evidence.tsv`, and generation
logs. Final standard editor/game builds both succeeded; exact timings are retained
in `final-build.log`. `git diff --check` passed. No cook/package, graphical editor
or hands-on play was performed. Submit as blocked evidence for production follow-up;
the unreached assertions are not claimed as passed.

## W6-03fixe follow-up (2026-10-10)

The F6 measurements identify retained corpse Visibility as the early earned-route
attack blocker (150cm distance,200cm range). Combat now excludes corpses from
attack sight while preserving their cursor/loot Visibility and life lookup.
Two full runs each complete196 tests with195 passes; two focused G4 runs each
complete8 with7 passes. Earned Melee/Ranged/Magic and DeathAndChurchRespawn pass
in all four runs. The sole remaining failure is SessionInventoryRollback, which
gets378/400 farming kills before the4096 accepted-activation replay cache rejects
UseAbility as Busy. Inventory filling/rollback assertions remain unreached.
All exported test states agree across repetitions and full/focused context.
This is automated evidence, not G4 gate approval or hands-on coverage.
Measurements, exact counts, commands and remaining limits:
[W6-03fixe report](reviews/w6-03-fixes-f6.md).
