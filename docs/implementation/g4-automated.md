# W4-15b automated evidence candidate (partial)

Base `c3c0eb2d9336305dd86be322f07d1ec7088ff4f9`, contract revision 1.
This candidate extends automated coverage but **does not finish W4-15b or approve
G4**. The remaining earned-build, inventory and GameInstance recovery work below
is explicit. No production code, generators, schema or mechanics changed.

## Checklist mapping

All names below have prefix `Lighthaven.Integration.`.

| Checklist | Test and assertions | Limit / remaining work |
| --- | --- | --- |
| Fresh creation; legal purchase/replay/save failure/RetrySave | `G4.LegalPurchaseReplayAndRetrySave`: creation presenter, HP30/MP10/gold100, real potion purchase, repeated request, failed storage, widget RetrySave, independent reload | Existing W4-15 coverage retained |
| Hub → B1–B4 → return, reload every floor, arrival grants nothing | `G4.SessionFloorRouteAndReload`: actual session RequestTravel, generated portals, travel/save adapter, encounter population, exact independent durable arrival on every floor, unchanged XP/gold at each arrival | Portal positioning isolates command validation; it does not walk the connecting corridors. Reload decodes a separate store; it does not restart GameInstance |
| Occupied arrival refused | Same test: blocking player capsule, refusal reason, no arrival write, source retained | Install hook performs a real placement query, but does not call private production Place; recovery hook acknowledges restoration without loading source geometry |
| Floor change during pending action | Same test: catalog enemy GAS windup on every basement departure, travel refused, adapter token and write count unchanged; action completes through timers before retry | Player attacks have zero impact delay; enemy one-second windup is the real reachable pending boundary |
| Enemy aperture clearance | `G4.DoorClearance`: player on every authored lintel / B1–B2 flat route; enemy envelope selected from the floor's registry/encounter rows and runtime home/leash; special B4 opening count five | A leash disk is a conservative permitted region, not a nav-path proof. Geometry and NoCombat can further restrict it. B1/B2 polygon-strip routes remain probes, not exhaustive aperture enumeration |
| Player stairs | Same test: actual CharacterMovement, force input for a controllerless fixture, native capsule, supporting collision and Walking mode at the endpoint; seven routes across B1–B4 | Stops before terminal portal walls; does not cross levels by locomotion, exercise controller feel, or test enemy stairs |
| Simultaneous lethal damage | `G4.DeathAndChurchRespawn`: two real populated encounters, same-frame catalog GAS attacks, normal clock/cooldown progression, lethal HP, exactly one death publication | Attackers are positioned to isolate simultaneous impacts. This proves once-only combat publication, not once-only durable death settlement |
| Death → church | Same test asserts as far as this fixture permits: no fabricated dead checkpoint and RequestRespawn refuses an undispatched death | **Incomplete**: FRuntime has no GameInstance-owned ULHSessionSubsystem. The production player-state death bridge resolves that owner; it cannot reach durable death/recovery here. No direct HandlePlayerDeath call or dead-snapshot edit substitutes for it. Requires a real GameInstance/session fixture, including church placement |
| Balork claim, clock, reload and travel | `G4.SessionFloorRouteAndReload`: real generated B4 Balork, repeated session melee commands at normal supported cooldown times, combat settlement, one claim; pause and travel freeze stop900s clock; active899s leaves1s; next1s enters RespawnPending; independent save preserves claim; subsequent travel captures the clock and exact arrival reload; authored Kiran return, replay and refusal of second completion | Combat isolates commands: AI decision time is not advanced during the melee loop, so this is **not opposed combat or a full earned melee build**. Safe respawn is unobserved: the loaded fixture intentionally lacks navigation; real IsSpawnSafe reports `navigation projection missing`, center(1200,6600,157), r100/hh155. Never replace safety with a synthetic true callback. No second life/second kill is claimed |
| Fresh earned melee, ranged and magic routes | No complete replacement implemented | **Incomplete, not manual-only**. Existing `Wave4.ServicesThroughSession`, `ProgressionRouteAffordable`, `BalorkSingleClaim` use synthetic floor installs, direct respawn advancement or cooldown restores and do not satisfy this checklist. Need legal earning/training/equipment/consumption plus opposed combat and real safe respawns through all floors |
| Full-inventory loot rollback | No replacement implemented | **Incomplete, not manual-only**. Existing domain capacity fixtures do not establish legal session earning/filling and corpse transfer rollback. Requires a legal inventory-filling route; no debug grants used to manufacture it |

## Region selection and collision evidence

The old global Balork envelope is removed. Every slot resolves through
`LHEncounterData::ForSpawn` and `LHEnemyData::Find`; ordinary home is its registry
anchor, while boss home is the generated `B4.BalorkArena` marker, exactly as the
director defines it. Only slots whose leash disk intersects the aperture probe or
route segment contribute capsule dimensions. The component-wise envelope encloses
the eligible capsules; it can be conservative when radius/height maxima differ.
Aperture logs include map/label, underside Z, capsule dimensions, blocker, slot
alias, home and leash. No failed eligible sweep is suppressed or expected as an
error. Player probes remain independent of enemy selection.

Observed corrected run: all eligible aperture/flat-route sweeps clear; all seven
CharacterMovement stair probes reach their endpoints on ground. The previous29
failures were from sweeping Balork on floors/regions he cannot reach. No geometry
fix is inferred as necessary from these probes. Balork's envelope is retained on
reachable portions of his B4 complex (including C04Door1 and the low arena
partition); remote openings are not treated as Balork routes merely because they
share a floor. No nav-agent traversal or visual-envelope claim follows.

## Clock and fixture rules

GAS timers are primed before requests. `AdvanceWorldClock` advances UWorld's
supported time-only tick and TimerManager in <=0.1s substeps. A single2.1s world
tick was observed to clamp to0.4s; advancing the timer by2.1 alone would leave
cooldowns inconsistent. There are no cooldown-map restores, manual impacts,
snapshot reward edits or direct kill/respawn settlement in the added tests.
CharacterMovement needs its native UpdatedComponent/InitializeComponent setup
in the editor fixture; it is driven through its actual TickComponent.

Ordinary encounter clock changes do not themselves schedule a durable action.
The test therefore checks the independent stored claim after the clock probe,
then relies on real floor travel's completed checkpoint and exact durable arrival
assertions. Flush with no queued action is not asserted to save transient time.

## Manual-only / deferred

Lighting/readability, visual Light/effects/animation/audio, controller feel,
graphical/package launch and in-play render stalls remain unobserved by null RHI.
Windows packaging/launch is deferred by the owner. Earned routes, inventory rollback,
GameInstance death/church and navigation-safe respawn are additional automation
work, not inherently impossible headless and not passed by this candidate.

## Execution

UE5.8.3 Linux at `/home/brewerm/Downloads/unreal`, uid1000. Final standard
`UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game` returned0:
editor `Result: Succeeded` (10.42s), game `Result: Succeeded` (5.18s).
Initial UBA builds were interrupted after stalling; exported UBT compile/link
commands were run in dependency order, then the standard build was rerun. Logs
are retained in the attempt library; exported-action success alone is not claimed
as a normal build result.

Generated five playable maps locally with LHGenerateHubMap,
LHGenerateBasementAMaps and LHGenerateBasementBMaps. All save logs are retained;
commands returned1 after baseline LFS-pointer package startup errors. No generated
map or review TSV is submitted.

Full-suite command (headless house form plus local DDC, stdout and privacy settings):

```sh
XDG_CONFIG_HOME="$PWD/Saved/BuildEnvironment/config" \
 /home/brewerm/Downloads/unreal/Engine/Binaries/Linux/UnrealEditor-Cmd \
 "$PWD/Lighthaven.uproject" \
 -ExecCmds="Automation RunTests Lighthaven; Quit" \
 -DDC-ForceMemoryCache -ddc=InstalledNoZenLocalFallback \
 "-LocalDataCachePath=$PWD/Saved/DerivedDataCache" \
 -nullrhi -unattended -nosound -nop4 -noshaderworker -nocrashreports \
 -stdout -FullStdOutLogOutput \
 '-ini:EditorSettings:[/Script/UnrealEd.AnalyticsPrivacySettings]:bSendUsageData=False' \
 '-ini:Engine:[ConsoleVariables]:HomeScreen.EnableHomeScreen=0' \
 -ReportExportPath="$PWD/Saved/W415bSubmittedReport" -log=G4Submitted.log
```

The first broad invocation was terminated by sandbox enforcement of a telemetry
request to datarouter.ol.epicgames.com after163 completions (160 success/3 fixture
failures); no complete report. Setting editor usage-data privacy false allowed a
later full invocation to export its report. Intermediate failures were fixture
setup/unsupported assertions and were corrected, not recorded as production defects.
Final submitted-source counts and report paths follow below. No cook/package,
graphical editor or hands-on play was performed. `git diff --check` passed.

Final submitted-source full-suite run: **171 completed, 165 success without
warnings, 6 success with warnings, 0 failure, 0 not run, 0 in process**;
process exit0, reported test duration78.53s. G4 has4 completed tests,
all success; DeathAndChurchRespawn explicitly warns that church recovery is
unobserved. Success means the implemented assertions passed, not that the
unimplemented checklist was fulfilled. Report: `Saved/W415bSubmittedReport/index.json`,
retained as `library/submitted-index.json` in the canonical worker output. Final
build log: `library/w415b-submitted-build.log`; suite log:
`library/w415b-submitted-tests.log`. No eligible aperture defects observed in this
run. Submit as a partial evidence candidate; the coordinator should schedule the
remaining real GameInstance/navigation/earned-route/inventory fixture work.
