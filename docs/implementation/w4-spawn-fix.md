# W4-13 — B1 runtime spawn and arrival checkpoint

Task ID: W4-13. Base revision: `42eb0a7037acd1ca1babe18ca0a27cbcf1d7dbd4`. Contract revision: 1. Result revision: submission candidate (recorded in the attempt receipt/report).

Owned changes: `Source/Lighthaven/AI/`, `Source/Lighthaven/Framework/`, `Source/Lighthaven/World/`, `Source/LighthavenTests/Integration/`, this document. No binary assets owned or submitted; no generator edits.

## Root causes

D1: `SpawnRecord` already adds the runtime half-height to the floor-contact marker. The failure is earlier than `ApplyRuntimeSpec`: UE 5.8.3's `Engine/Private/LevelActor.cpp` tests `DontSpawnIfColliding` with the class-default actor (`Template`) before allocating the deferred actor. The default ACharacter capsule is larger than the Brown Rat's 25 cm half-height. At the runtime capsule's center, the default capsule penetrates the authored floor. `SpawnActorDeferred` returns null, so runtime capsule sizing never runs. This is independent of navigation and is not solved by merely adding the runtime half-height again. The engine emits this rejection at Log level; the application previously silently returned.

D2: `LHSessionSubsystem` installs the coordinator's arrival snapshot and calls `StartEncounters`, which populates **the live session copy** and queues a separate population boundary. `LHTravelCoordinator::OnDestinationLoaded` then saves **its original unpopulated Arrival**. `OnSaveCompleted` publishes that empty snapshot through `CheckpointDurable → InstallTravel`, replacing the populated live session. The empty arrival persists; subsequent Continue is the first durable population. This is a snapshot ownership bug, not a missing spawn marker.

## Changes

Spawning now checks NoCombat bounds and a blocking overlap with the actual species capsule, at the unchanged marker XY and marker Z + runtime half-height + 2 cm. Only after those checks does it use deferred AlwaysSpawn, apply the runtime spec, and finish spawning. It never searches or shifts horizontally, so collision adjustment cannot cross a safety box. The 2 cm clearance follows the existing player arrival convention; it is geometric tolerance, not a mechanics/stat change. Boss home uses the same clearance. Respawn retains D09 distance, LOS, NoCombat, nav projection, collision and floor gates.

Every actual spawn refusal reports a Warning with marker/life, definition, area, generation, failed check and relevant values. Spec health rejection includes current/max HP. `SpawnLife` reports the specific failed respawn safety check. Eligibility polling intentionally does not log on every simulation tick; it does not attempt a spawn.

An additive optional `PrepareArrival` host hook transforms the coordinator-owned arrival snapshot before installation and saving. Production binds it to the session's atomic `PopulateEncounterCheckpoint`, shared with Continue hydration. It creates native registry encounters and deterministic reward IDs without an extra transaction increment; travel's existing arrival increment covers them. Existing encounters/life generations/health, receipts and save shape remain unchanged. Missing callbacks remain compatible with existing pure travel-host fixtures. Continue's legacy empty-floor population still queues its own completed boundary. No duplicate records are appended.

## Validation

Observed baseline comparison: temporarily substituted the base revision's `LHEncounterDirector.cpp` (only the IsSpawnSafe declaration signature adapted for the additive diagnostics argument) and `LHTravelCoordinator.cpp`, retaining the new tests and session helper. Editor build exit 0, UBT 20.87 s. The two named tests both failed (0 succeeded, 0 succeededWithWarnings, 2 failed, 0 notRun/inProcess; exit 255; report duration 1.224296 s). Continue had 17 records and 0 actors. Travel's first durable arrival had 0 records and 0 actors; an explicit repeat hydration recovered 17 records but still 0 actors. This directly reproduces D1 and D2 against loaded B1. Restored fixed copies before final builds/tests.

Observed fixed targeted run: both named tests succeeded (0 succeeded, 2 succeededWithWarnings, 0 failed/notRun/inProcess; exit 0; 0.481597 s report duration). The expected blocked-marker Warning was captured by Automation. The original engine spawn call returned null at rat marker BF36087D7EB24F6EA2E7C07103873227, center (300,300,25). Independent floor collision control ignored all spawned enemies and proved the default half-height 88 cm overlaps at the fixed center (300,300,27), while the actual half-height 25 cm capsule produced 17 actors. Every encounter resolved through FindByLife/FindByEntity, retained its XY anchor, and passed controller SelectTarget. Travel readback contained 17 lives; final Continue test additionally reads back its population boundary. Duplicate arrival callbacks/repeated hydration preserve 17 records and do not advance Continue's population transaction again.

Final `UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game`: exit 0, both targets Result: Succeeded; editor UBT 35.25 s, game UBT 31.61 s. UID was 1000. Build-local XML/config/cache paths were used; no system changes. UE 5.8 always labels its executor UBA, even with detouring disabled. Initial exploratory builds were interrupted during PCH compilation (exit 130); one omitted-cache-root invocation failed (exit 6, read-only worker home). These are superseded by final successful builds.

Map generation: `UE_ROOT=/home/brewerm/Downloads/unreal LH_NO_ZEN=1 bash build/generate-basement-a-maps.sh`, exit 1, commandlet result 0, successfully wrote B1 (17 anchors) and B2 (25 anchors); only baseline-pointer package errors caused the nonzero process result. Maps are safety-unreviewed locally and excluded from submission.

Two exploratory fixture runs crashed before complete reports: first, the synthetic creation session was still travel-frozen and a missing area was dereferenced; second, duplicate worlds in the shared transient package collided on UE's package-based rename. Fixed by arrival-style rebind, defensive assertions, unique world packages, and retaining the world context through subsystem teardown. Crash-report telemetry was rejected by the sandbox. Final tests use the corrected fixture and process-local privacy/no-crashreports flags. No success is attributed to incomplete runs.

Final full headless suite: **158 found/completed; 150 succeeded, 3 succeededWithWarnings, 5 failed, 0 notRun, 0 inProcess**, exit **255**; report duration **57.95268249511719 s**. Both B1 regressions succeeded with zero warnings/errors (Continue 0.335784674 s, travel 0.264750838 s). The corrected teardown supersedes the targeted run's context warnings.

Failures, all reported rather than waived:

- `Lighthaven.Integration.Wave4.ProgressionRouteAffordable`: “earned purchase” false (known service purchase seam, W4-07b).
- `Lighthaven.Integration.Wave4.ServicesThroughSession`: “buy bow” false (known service purchase seam, W4-07b).
- `Lighthaven.Integration.Wave4.UseItemThroughSession`: “real potion purchase” false (known service purchase seam, W4-07b).
- `Lighthaven.Integration.Wave3.ArrivalSafety`: hub, B3 and B4 cannot load because their local files remain LFS pointers. **B1 Entry/Descent and B2 Entry/Descent passed** capsule/floor/nav controls. This report is not an applied safety review, and no review TSV is submitted.
- `Lighthaven.World.LightingAudit`: hub, B3 and B4 null worlds from those same pointer files.

SucceededWithWarnings tests: `Lighthaven.Abilities.BowRequiresQuiver`, `Lighthaven.AI.StateMachine`, `Lighthaven.AI.TwoEnemiesOneSettlement`.

Exact full-suite command (worktree CWD):

```sh
XDG_CONFIG_HOME="$PWD/Saved/BuildEnvironment/config" \
/home/brewerm/Downloads/unreal/Engine/Binaries/Linux/UnrealEditor-Cmd "$PWD/Lighthaven.uproject" \
  -ExecCmds="Automation RunTests Lighthaven; Quit" -DDC-ForceMemoryCache \
  -ddc=InstalledNoZenLocalFallback -LocalDataCachePath="$PWD/DerivedDataCache" \
  -nullrhi -nocrashreports -unattended -nosound -nop4 \
  '-ini:Engine:[ConsoleVariables]:HomeScreen.EnableHomeScreen=0' \
  '-ini:EditorSettings:[/Script/UnrealEd.AnalyticsPrivacySettings]:bSendUsageData=False' \
  -ReportExportPath="$PWD/Saved/AutomationW413Full"
```

Targeted/baseline commands use the same flags, `Automation RunTests Lighthaven.Integration.Wave4.B1Spawn; Quit`, and report directories `AutomationW413Fixed` / `AutomationW413Baseline`. Test timings above are measured report durations, not shell durations; shell timings were not separately captured. `git diff --check` and submission-script `bash -n`: exit 0.

Evidence: attempt output `library/full-suite-index.json`, `baseline-index.json`, `fixed-targeted-index.json`, matching logs, `final-build.log`, `baseline-build.log`, `map-generation.log`, `headless-tests.sh`, and `summary.json`. The repository document and report are evidence candidates, not a verified task/gate declaration.

Checks not run: cook/package/interactive Stage 1A new-character descent and Continue, because this worker validates through the headless loaded-map harness and leaves full map generation/review/package integration to the coordinator. Windows checks remain deferred by project instruction. No marker relocation is required; no generator follow-up is requested. Known remaining defects: the three service failures and local pointer-map limitations above; packaged end-to-end verification remains outstanding. Headless test harness uses real generated B1 geometry, the production save adapter/coordinator and live session arrival methods, and the actual `Continue` method. It injects synchronous map loading rather than invoking `OpenLevel` or a packaged frontend. Packaged/interactive validation remains a coordinator follow-up.

Source-backed mechanics: no new mechanics values. Provisional tuning introduced: 2 cm runtime capsule clearance, matching player arrival tolerance; no enemy stats or encounter anchors changed.

Next task/integration notes: review additive travel hook; regenerate/review maps through existing automation, then re-run packaged Stage 1A new-character descent and Continue. Windows remains deferred. Never integrate locally generated maps or arrival review TSV from this worker.
