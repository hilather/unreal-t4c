# W3-05 integration candidate

Base: `fcd13ce22d7e169bf5589fda8203dbb036721605`; contract revision 1.
Core revision 1 remains unchanged. No map or asset bytes are authored or committed.

The native five-area registry is the Wave 3 catalog: hub, B1, B2, B3, B4;
nine entrances and eight directed reciprocal portal edges. The catalog hash now
includes ordered area/map identities, entrance transforms, portal endpoints and
spawn IDs/enemy/anchor definitions. Old Wave 2 catalog saves are incompatible and
are rejected by the save store, rather than silently reinterpreting hub `Entry`.
The mechanical profile remains the existing explicitly Prototype profile.

Creation checkpoints at hub `Temple.SafeSpawn`; Continue opens the saved area's
map and places the capsule at the saved registry entrance. Frontend remains the
default game entry; every playable map is explicitly listed in MapsToCook.
Portal interaction selects the nearest portal within 250 cm with a visibility
check and sends a run/epoch-qualified command to W3-04. This distance is Prototype
interaction presentation tuning, not an authentic T4C mechanic.

Session hooks supply completed-action capture, persistent command/movement blocking,
checkpoint sequence publication, canonical authority import and collision-checked
avatar placement. Destination installs remain subject to the W3-04 unique reviewed
marker and world validation checks. Travel does not award anything. Failed travel
uses W3-04 source restoration; persistence Retry invokes its retry state machine.
Startup with invalid/unreviewed arrival returns to the frontend, retains the durable
save and reports the refusal; it does not create an arrival checkpoint.
W4 still owns combat, lifecycle settlement and enemy population; no AI is spawned
by these adapters. Live enemy blocking overlap also rejects arrival.

## Automated arrival approval

`Lighthaven.Integration.Wave3.ArrivalSafety` loads all five real maps and logs each
of the nine IDs with transform hash and separate count/transform/capsule/floor/nav
results. Both marker actor transform and SafeArrivalTransform must match registry.
The largest currently defined player capsule is read from ALHCharacter's CDO
(35 cm radius, 90 cm half-height). A 2 cm lift avoids treating floor contact as
penetration. Ground tracing allows 5 cm height error and requires upward normal
Z >= 0.7. These tolerances are Prototype validation policy, not historical values.

Each loaded world uses Editor mode without BeginPlay. Components register first,
then all static-mesh compilation finishes, transforms update and physics states
are recreated. Before queries, the test initializes the navigation system for the
loaded world (InitWorld alone creates it without world initialization), then calls
`FNavigationSystem::Build`. In UE 5.8.3 that native delegate invokes
`UNavigationSystemV1::Build`, which calls `EnsureBuildCompletion` for every nav data
set before returning. No console Exec or arbitrary ten-second sleep remains.

Positive controls require a Pawn-blocking registered query primitive with physics
state, a real physics scene, a downward hit on a separately authored flat-floor
probe, main navigation data and valid populated-tile bounds. The latter uses the
Engine navigation interface's `ComputeNavDataBounds`: Recast `GetBounds` delegates
to `FPImplRecastNavMesh::GetNavMeshBounds`, which accumulates only Detour tiles
with headers, not allocated empty tile slots. This avoids an out-of-scope module
dependency change. Empty nav data therefore fails loudly. Any failed world control
forces every arrival in that map to FAIL, excluding it from reviewed evidence.

Flat-floor controls in registry order are hub nave (800,1200,0), B1 (900,900,0),
B2 (1100,1000,0), B3 entry room (3500,500,0), B4 entry room (600,600,0), in cm.
B1/B2 reuse the generators' route controls; other points sit inside authored floor
rectangles. Graybox generators use BlockAll for collision, including Pawn and
Visibility. Floor queries now use Pawn, matching capsule movement semantics.
Registry SafeTransform locations are foot/ground positions, not capsule centers;
the capsule center remains ground + half-height + 2 cm. Flat slabs have center
Z=-10 and thickness 20 cm (basement A uses equivalent meter-scale cubes), so their
tops are Z=0. The +10/-15 cm trace window, <=5 cm height error and normal Z>=0.7
remain unchanged; no increased tolerance or placement fix is justified by source.

The test invokes reflected `K2_ProjectPointToNavigation` with 25 cm horizontal
and 50 cm vertical extent, enforcing horizontal displacement <=25 cm. Each existing
`ARRIVAL …` line retains its original fields and appends world-control results and
semicolon-delimited failure reasons. `ARRIVAL_CONTROL` reports each loaded world's
positive controls. No real geometry result is inferred from compilation or pointer
package lookup; host review remains required.

`build/review-arrivals.sh` refuses root and LFS pointers, removes stale walk evidence,
runs only the headless walk-test with the documented W3-04b startup flags, and reads
its completed automation report plus `Saved/ArrivalSafety.tsv`. It atomically refreshes
`Config/Lighthaven/ReviewedArrivals.tsv` using only PASS rows. A partial completed run
writes only passing IDs and exits 1; startup/incomplete evidence leaves the list unchanged.
The list is absent in this candidate: no geometry walk passes are claimed.
The generators read the list and set reviewed only when ID and canonical-transform
SHA-1 hash match; moving a transform invalidates the approval automatically. SHA-1
here is a deterministic change fingerprint, not a security signature. Never edit the
list or markers by hand. Geometry changes without transform changes require rerunning
the walk-test; the fingerprint covers transforms, not arbitrary map geometry.

## Packaged content check

All cooked runtime session startups check package existence for the frontend and five
registry maps, blocking character commands if content is incomplete. Launch the Linux
Development package with `-LHCheckPackagedContent -nullrhi -unattended -log`: it logs
per-map package lookup and exits with status 0 only if all required packages exist.
This runs in the game runtime, avoiding reliance on the editor-only test module.
`Lighthaven.Integration.Wave3.PackagedContent` supplies an editor preflight only;
a successful editor lookup does not establish that the package contains the maps.

## G3 packaged checklist — coordinator host work, pending

- [ ] Hydrate LFS maps. Build editor and game with `UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game`.
- [ ] Run `UE_ROOT=/home/brewerm/Downloads/unreal bash build/review-arrivals.sh`; require nine PASS IDs and commit the generated reviewed list.
- [ ] Regenerate hub, basement A, basement B on the host with their existing scripts (W3-04b memory-DDC/nullrhi/home-screen flags if startup needs them). Only coordinator commits maps through LFS.
- [ ] Run LHValidateWorld on all five regenerated maps with zero errors. This includes the cross-map duplicate-ID scan, all required markers, reciprocal portals and reviewed entrance checks.
- [ ] Run full Lighthaven Automation, including Wave3 traversal/reload, review hash and actual arrival walk on regenerated maps.
- [ ] Package Linux with the explicit six-map command below (the existing wrapper's argument-free default is still the G1 map set, outside this task's owned paths); run packaged `-LHCheckPackagedContent`, retain exit status and per-map log.
- [ ] Create new character at hub, interact to descend hub → B1 → B2 → B3 → B4, and return B4 → B3 → B2 → B1 → hub. No direct B4-to-hub portal is invented.
- [ ] Quit/reload at hub and on each floor, including each descent/return entrance; confirm saved identity, valid capsule placement and unchanged rewards/timers.
- [ ] Exercise missing package, invalid/blocked spawn and corrupt area data: refusal or durable source restoration; no silent hub reset, no arrival save on failure.
- [ ] Check door/stair collision and navigation using the largest enemy placeholder on all connecting apertures; walk-test player capsule coverage does not certify enemy doorway clearance.
- [ ] Retain packaged gameplay evidence. Windows packaging/launch is deferred per project memory, not passed.

Native traversal/reload tests exercise the production save store/codec/travel adapter
with in-memory A/B storage, eight registry edges and independent reloads at source
and arrival. They assert unchanged gold/XP and safe missing-destination/blocked-arrival/
corrupt-world outcomes. They do not exercise OpenLevel or physical disk/process crashes.

## Validation observations

Final build/test observations are appended after execution; G3 remains pending real
map review, regeneration, validator and packaged traversal regardless of compilation.

Exact Wave 3 package command (required because the wrapper supplies `-map`):

```sh
UE_ROOT=/home/brewerm/Downloads/unreal bash build/package-linux.sh \
  /Game/Lighthaven/Maps/L_Frontend \
  /Game/Lighthaven/Maps/L_LighthavenTempleDistrict \
  /Game/Lighthaven/Maps/L_TempleB1 /Game/Lighthaven/Maps/L_TempleB2 \
  /Game/Lighthaven/Maps/L_TempleB3 /Game/Lighthaven/Maps/L_TempleB4
```

Final-source checks observed in this worker (UID 1000, UE 5.8.3 Linux):

- `UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game`:
  exit 0, 12 seconds combined shell wall time; editor `Result: Succeeded`
  (9.33 s UBT), game `Result: Succeeded` (2.05 s UBT). The preceding full
  compile also built both targets successfully (54.02 / 105.26 s UBT).
- Direct headless `UnrealEditor-Cmd` with `Automation RunTests Lighthaven; Quit`,
  `-DDC-ForceMemoryCache -nullrhi -unattended -nosound`, HomeScreen disabled,
  worktree XDG_CONFIG_HOME, report and absolute log paths: exit 255 in 60 s.
  Exported final report: **79 Success, 1 Fail, 0 notRun/inProcess**.
  Only ArrivalSafety failed: all nine rows have count/transform/capsule/floor/nav
  zero because real map worlds could not load from the LFS pointers. No arrival
  approval is inferred. TraversalAndReload, SessionCheckpoints, ReviewTransformHash
  and editor PackagedContent preflight succeeded. The latter finds package filenames
  even for LFS pointers; it does not establish loadable or packaged map content.
- `bash -n build/review-arrivals.sh` and `git diff --check`: exit 0.
- `UE_ROOT=/home/brewerm/Downloads/unreal bash build/review-arrivals.sh`: exit 1
  at the LFS-pointer preflight, before editor startup or list creation.
- `git lfs pull --include='Content/Lighthaven/Maps/L_*.umap'` returned 0 but
  explicitly skipped checkout because LFS is not installed for this repository;
  an explicit filter-process retry behaved identically. `git lfs checkout` also
  refused checkout. All tracked maps remain pointer files, unchanged.

Evidence is retained in this attempt's worker-output `library/`: final build log/
timing, final automation index/log/timing, nine-ID arrival TSV and script rejection.
No map generation, successful geometry walk, LHValidateWorld, cook, packaged content
invocation or gameplay check was performed. Missing real LFS maps is the concrete
prerequisite; coordinator host actions above remain required. Startup refusal and
checkpoint tests use native seams and do not establish real map/OpenLevel behavior.

## W3-05b collision/navigation test correction

Base `07442a0a765510d78b8f184ca6bc5fdc2e7446be`, contract revision 1.
Only the arrival test and this document change; review script parsing already accepts
appended diagnostic fields. No placement changes, maps, or reviewed lists are committed.

Final-source `UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game`
returned 0 in 10 seconds combined shell time: editor `Result: Succeeded` (7.81 s UBT,
explicitly compiled LHWave3ArrivalTests.cpp), game `Result: Succeeded` (1.94 s UBT).
UID was 1000. `bash -n build/review-arrivals.sh` and `git diff --check` returned 0.
`UE_ROOT=/home/brewerm/Downloads/unreal bash build/review-arrivals.sh` returned 1
in under one second at the hub LFS-pointer preflight, before editor/list creation.
Full automation results and timing are recorded below after the final run.

Real-map floor, physics repair, nav build and tile-control results remain unobserved:
all five worker maps are LFS pointers. Coordinator must run review-arrivals.sh on
hydrated maps and investigate any specific FAIL without relaxing these checks.
No LHValidateWorld, regeneration, cook, package, or gameplay checks are claimed.
Windows remains deferred. G3 is not passed by this correction.

Final rebuilt-source full headless `Automation RunTests Lighthaven; Quit` returned
255 in 61 seconds, using memory DDC/nullrhi/unattended/nop4/nosound, disabled
HomeScreen, worktree XDG_CONFIG_HOME and exported report/absolute log paths.
Exported report: **80 Success, 1 Fail**, no unfinished tests. Only ArrivalSafety
failed: five explicit `world=0` control errors and nine FAIL rows, with all controls
zero and `reason=world-load;...;floor-trace-miss;nav-projection-failed;`.
The final logs include floorhit/floordelta/floornormal, confirming final diagnostics
were executed. This verifies failure reporting on pointers, not real-map safety.
Evidence: this attempt's worker-output `library/automation-current/` report,
`automation-current-editor.log`, timing files, `ArrivalSafety.tsv`, and build logs.

## W3-05c loaded-world physics and navigation

Base `4769fe1af9f84a65a4ff912d64f2caa77a52ce6b`, contract revision 1.
Only the arrival test and this document change. No generator/placement fixes,
map binaries, reviewed lists, schema changes, or altered safety tolerances.

UE 5.8.3 UEditorEngine::OnAssetLoaded initializes Inactive worlds via
InitializeNewlyCreatedInactiveWorld, with CreatePhysicsScene(false).
Changing WorldType afterward and checking IsInitialized skips the needed setup.
The physics-only intermediate run demonstrated initialized=1, scene-before=0,
scene-after=1 on every map, and physics/floor/capsule controls all passed;
navigation still failed. Its logs showed missing UNavigationObjectRepository and
AsyncLoadLock (0x20). The repository is a world subsystem selected at initialization.

The final test sets WorldTypePreLoadMap to Editor around LoadPackage, following
UE's editor map-loading pattern, then initializes once. It also creates a missing
physics scene defensively before registration and retains compilation/body repair.
After finishing all asset compilation, it disables the reflected per-world
bWaitForAsyncLoadingBeforeBuildingNavigationAutomatically property before navigation
initialization. UE otherwise requires at least 16 core-ticker frames/two seconds to
unlock; this synchronous automation command cannot advance that ticker. The
property change is confined to the destroyed test world, with no global/config
change. A missing property fails closed. Explicit native Build still builds Recast
and waits for completion; valid computed nav bounds still prove populated tiles.

UID: 1000. Initial build command:
`UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game` returned 0;
editor Result: Succeeded (144.80 s), game Result: Succeeded (128.74 s).
Final same-command rebuild returned 0 in 9 shell seconds: editor Result: Succeeded
(6.85 s), game Result: Succeeded (1.86 s). git diff --check and bash -n
build/review-arrivals.sh returned 0.

Real maps were generated with the editor scripts, XDG_CONFIG_HOME set to
$PWD/Saved/BuildEnvironment/config and UE_ROOT=/home/brewerm/Downloads/unreal:
`bash build/generate-hub-map.sh`, `LH_NO_ZEN=1 bash build/generate-basement-a-maps.sh`,
`bash build/generate-basement-b-maps.sh`. Each process returned 1 from unrelated
LFS-pointer asset-registry errors; each commandlet reported result 0 and saved its
owned maps. B1/B2 native floor/route/stair controls passed. Hub was also regenerated
with the direct memory-DDC headless commandlet invocation (result 0, process 1).
Initial pre-build hub script launch returned 1 before map generation.
Commandlet execution times reported by UE: hub 0.56 s, basement A 0.81 s,
basement B 0.47 s; total script elapsed time was not independently timed.
Direct hub invocation: XDG_CONFIG_HOME=$PWD/Saved/BuildEnvironment/config
/home/brewerm/Downloads/unreal/Engine/Binaries/Linux/UnrealEditor-Cmd
$PWD/Lighthaven.uproject -run=LHGenerateHubMap -DDC-ForceMemoryCache
-nullrhi -unattended -nop4 -nosound
-ini:Engine:[ConsoleVariables]:HomeScreen.EnableHomeScreen=0.

`UE_ROOT=/home/brewerm/Downloads/unreal bash build/review-arrivals.sh` returned 0
in 58 shell seconds on final source. This runs XDG_CONFIG_HOME=$PWD/Saved/BuildEnvironment/config,
-DDC-ForceMemoryCache -nullrhi -unattended -nop4 -nosound,
-ini:Engine:[ConsoleVariables]:HomeScreen.EnableHomeScreen=0, and
Automation RunTests Lighthaven.Integration.Wave3.ArrivalSafety; Quit.
Exported report: one completed Success, reviewed 9/9. All five world controls:
initialized=1, scene-before=1, scene-after=1, levels=1, physics=1, floor=1,
navdata=1, tiles=1 (tiles is a boolean populated-bounds control, not a tile count).
The physics-only intermediate review returned 1 in 61 s (one completed Fail).

Exact final arrival lines:

```text
ARRIVAL Area.LighthavenTempleDistrict/Temple.SafeSpawn PASS hash=15325072514A4188181BB893ADDD76053B815884 count=1 transform=1 capsule=1 floor=1 nav=1 radius=35.0 halfheight=90.0 controls=1 physics=1 knownfloor=1 navdata=1 tiles=1 floorhit=1 floordelta=0.00 floornormal=1.00 reason=ok
ARRIVAL Area.LighthavenTempleDistrict/Temple.Descent PASS hash=3F82F0B2AB80F6B3D05D4CCA5D04EA94C7E392A8 count=1 transform=1 capsule=1 floor=1 nav=1 radius=35.0 halfheight=90.0 controls=1 physics=1 knownfloor=1 navdata=1 tiles=1 floorhit=1 floordelta=0.00 floornormal=1.00 reason=ok
ARRIVAL Area.TempleB1/Entry PASS hash=AB8B6B423CEE70994931146D09060018C82FE3C4 count=1 transform=1 capsule=1 floor=1 nav=1 radius=35.0 halfheight=90.0 controls=1 physics=1 knownfloor=1 navdata=1 tiles=1 floorhit=1 floordelta=0.00 floornormal=1.00 reason=ok
ARRIVAL Area.TempleB1/Descent PASS hash=53AC035523728DF84A22E7CB1A54309649C4663A count=1 transform=1 capsule=1 floor=1 nav=1 radius=35.0 halfheight=90.0 controls=1 physics=1 knownfloor=1 navdata=1 tiles=1 floorhit=1 floordelta=0.00 floornormal=1.00 reason=ok
ARRIVAL Area.TempleB2/Entry PASS hash=4213865433DBA97C6C30E22398F288F3E7EAAB20 count=1 transform=1 capsule=1 floor=1 nav=1 radius=35.0 halfheight=90.0 controls=1 physics=1 knownfloor=1 navdata=1 tiles=1 floorhit=1 floordelta=0.00 floornormal=1.00 reason=ok
ARRIVAL Area.TempleB2/Descent PASS hash=2035638011A03CFB4885C2E5B44EC2EB97E4AC0A count=1 transform=1 capsule=1 floor=1 nav=1 radius=35.0 halfheight=90.0 controls=1 physics=1 knownfloor=1 navdata=1 tiles=1 floorhit=1 floordelta=0.00 floornormal=1.00 reason=ok
ARRIVAL Area.TempleB3/Entry PASS hash=4DEAF2A2512AE0C5F340F1994D3D5623FB8E5EB0 count=1 transform=1 capsule=1 floor=1 nav=1 radius=35.0 halfheight=90.0 controls=1 physics=1 knownfloor=1 navdata=1 tiles=1 floorhit=1 floordelta=0.00 floornormal=1.00 reason=ok
ARRIVAL Area.TempleB3/Descent PASS hash=5DA240A639D63847C1E26DD6E828E6C57CE22892 count=1 transform=1 capsule=1 floor=1 nav=1 radius=35.0 halfheight=90.0 controls=1 physics=1 knownfloor=1 navdata=1 tiles=1 floorhit=1 floordelta=0.00 floornormal=1.00 reason=ok
ARRIVAL Area.TempleB4/Entry PASS hash=DA6D11A5C329A547D75813B928C7E4FD290AD14F count=1 transform=1 capsule=1 floor=1 nav=1 radius=35.0 halfheight=90.0 controls=1 physics=1 knownfloor=1 navdata=1 tiles=1 floorhit=1 floordelta=0.00 floornormal=1.00 reason=ok
```

Evidence is under this attempt's worker-output/library/: build-final.log/time,
arrival-final/editor.log and index.json, arrival-lines.txt, ArrivalSafety.tsv,
ReviewedArrivals.tsv, generator logs, and arrival-physics-only.log/tsv.
Generated maps and Config/Lighthaven/ReviewedArrivals.tsv were restored/removed
before submission. The evidence reviewed list is retained only outside the repo.

No full-suite rerun, reviewed-map regeneration/LHValidateWorld, cook, package,
or gameplay check is claimed. Next coordinator step: review/integrate source,
run the review on host maps, regenerate using its reviewed list, and run
LHValidateWorld on all five maps before persisting LFS maps. G3 remains subject
to that validation and remaining gate items. Windows is deferred.
