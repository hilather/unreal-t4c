# W3-03 — B3/B4 graybox generator

`LHGenerateBasementBMaps` replaces only `/Game/Lighthaven/Maps/L_TempleB3`
and `/Game/Lighthaven/Maps/L_TempleB4` in a fresh editor process. It serializes
runtime/engine actors, map-contained material instances, and engine primitive
meshes. No editor-module actor class or external reference image enters a map.
Generated binaries belong to the coordinator's LFS submission, not this change.

```bash
UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game
UE_ROOT=/home/brewerm/Downloads/unreal bash build/generate-basement-b-maps.sh
UE_ROOT=/home/brewerm/Downloads/unreal bash build/run-tests.sh Lighthaven
```

The wrapper rejects UID 0 before loading environment configuration, rejects
arguments and read-only existing LFS maps, then invokes UnrealEditor-Cmd with
`-run=LHGenerateBasementBMaps -unattended -nullrhi`. It selects
`-DDC=NoZenLocalFallback` with `Saved/DerivedDataCache` as its writable local
filesystem cache and passes `-nocrashreports` for unattended generation.
This avoids writes to the read-only engine/user cache and needs no Zen server.
A failed asset load, registry
validation, count/roster check, actor creation, map save or manifest write returns
1. Saves are sequential, not atomic: a B4 failure may leave a new B3 map. Run
without unsaved editor work; regeneration replaces manual edits.

## Geometry and evidence

The input contracts are [layout conventions](layout/README.md), [B3](layout/b3.md),
[B4](layout/b4.md), [world ledger](world-ledger.md),
[map evidence](map-reference-notes.md), [world systems](world-systems.md),
[A-02](art/environment/README.md), [A-04](art/lighting/README.md),
[creature clearance](art/creatures/README.md) and [placeholders](art/placeholders/README.md).
All dimensions/transforms remain **Prototype**, inherited V-01 /
LH_Prototype_v1 / 2026-10-07 / source_url null. Exact primitive decomposition,
20 cm slabs, floor marks, plinth height, palette implementation and manifest
format are W3-03 / LH_Prototype_v1 / 2026-10-08 / source_url null assumptions.
Materials inherit A-02; lights inherit A-04, both 2026-10-08 / source_url null.
No mechanics values or historical floor placements are newly researched here.

The commandlet contains baked rectangle-union floor strips and boundary wall
segments in centimetres. They are the union of the listed rooms, orthogonal
corridor sweeps and square turn pads, with flat floor removed under ramps and
landings. There is no large rectangular backing slab spanning absent floor.
Boundary walls lie outward from clear floors; their bottoms extend to Z=-100 cm
to close the descending-stair edge. Wall tops are Z=400 cm. Ordinary corridor
mouths have 240×300 cm apertures; corridors are 320 cm clear. Stair runs are
300 cm with 320 cm landings, smooth 20 cm collision slabs and no decorative
step collision. No roof is added.

B3 includes all eleven labelled regions (C3Cell is a partition inside
LowerWest), ten corridors, C3Cell west door, West stubs, East recess and
LowerRight divider. The reconstructed cycle is Entry–LowerWest–West–
UpperCentral–East–LowerRight–Entry. UpperCentral–BranchNear–BranchEnd,
West–WestBay and LowerRight–Descent are separate branches. The explicit
[20,27]×[14,24] U void remains absent and bounded, as do other union gaps.

B4 includes seven regions, C01–C04, the D05–D07 complex loop, one 20 cm shared
partition band per pair, flush threshold floors, the East baffle and its blocked
south plinth, a two-tier altar entirely within [14,18]×[70,74] U and top Z=120 cm,
and sparse nonblocking pale wall forms. The central dark patch is nonhazardous.
There is no side-chamber shortcut, arena lock, fifth floor or post-boss portal.
C04 is 550 cm clear; C04/D05/D06/D07 doors are 500×360 cm. Their specified
600 cm room-side pads and Hall apron are free of solid decoration. Balork's
440 cm wings/484 cm conservative transit-turn diameter and R100/HH155 cm
capsule fit those nominal dimensions. Ordinary routes clear the 180 cm Atrocity
arm envelope and 170 cm Giant Bat wings; their collision capsules are smaller.
These comparisons do not establish actual animation/collision/navigation.
Balork is not assigned transit through C01–C03 or the ordinary return stair.
AI must enforce the authored boss extent and test retreat/LOS exploits.

## Registry identities, safety and travel

The generator consumes the W3-04 registry directly: 22 B3 and 13 B4 spawn
markers, with its existing aliases, GUIDs, enemy definitions and ground pivots.
B3 roster: rat4/slime3/GiantBat4/Goblin6/GoblinWarrior3/Atrocity2.
B4: rat3/slime2/GiantBat4/Atrocity3/Balork1. No provisional floor rows are
assigned to either floor; all authored counts/positions are still prototype.
The generator rejects an unassigned species. Undead Bat's disputed B3 source,
Dungeon Bat's provisional B2 row and decorative pale forms add no encounters.
Markers do not activate enemies, set HP/loot/respawn, or implement boss defeat.

Arrival transforms and identities come from the registry. `bSafetyReviewed`
stays false: geometry alone cannot certify damage, corpse or enemy-reach safety.
The 6×6 U arrival reserves contain no generated spawns or props. Ground pivots
must receive the actual pawn capsule half-height in the travel/session adapter.

| Source | Destination | Departure feet (cm), yaw | Arrival feet (cm), yaw |
|---|---|---|---|
| B3 Entry | B2 Descent | (3200,1360,80), 90 | (3200,500,0), -90 |
| B3 Descent | B4 Entry | (4700,7060,-80), 90 | (4700,6100,0), -90 |
| B4 Entry | B3 Descent | (-210,600,80), 180 | (650,600,0), 0 |

Each ALHPortal retains the registry GUID and explicit directed destination.
The existing B2 Descent reverse actor belongs to W3-02, outside this ownership.
Portals have serialized noncolliding 100×300×300 cm interaction bounds above
the departure feet and a nonblocking direction bar. They initiate no overlap
travel. The integrator must connect explicit interaction to the W3-04 adapter;
bounds alone are not a functioning interaction or a safety enforcement system.

## Presentation and deterministic regeneration

A-02 palette starts are embedded map-contained instances of the engine
BasicShapeMaterial: warm stone #78634B on B3, deep stone #514B42 on B4,
floor #695640, pale repairs #9B9484, branch stain #733D35 and dark #333333.
B3 pale repair and branch-end marks, B4 dark hall patch, pale forms and stepped
altar are visual landmarks, with no loot or encounter semantics.

All 17 B3 and 13 B4 A-04 lights use the README/SVG positions. Point lights are
Movable, inverse-square, initially unshadowed; T=600 lm/600 cm/2200 K,
W=1000 lm/800 cm/2200 K, N=1200 lm/850 cm/6500 K. B3 C3Cell uses
275 lm/300 cm. B4 F01 is a down-facing 3200 lm/1700 cm/6500 K spotlight,
50°/65° cones. Skylight intensity is 0.32/0.26, specified engine
`/Engine/EngineResources/GrayTextureCube`, real-time capture off, lower-hemisphere
black disabled. Exposure min=max 2, bias 0. No directional basement light.
The engine gray cubemap is a fixed reproducible calibration choice rather than
an authored new cubemap; host review must reconcile its luminance with other
floors. Renderer settings/local exposure and visual calibration remain owned
by the integrator. Null RHI cannot establish light leakage, budgets or appearance.
Full-height opaque wall meshes do not implement A-04 runtime cutaway/fade;
that remains a presentation integration dependency.

Actor names, transforms, authored IDs and lighting/material inputs are fixed;
no RNG, coordinate-derived GUID or array-order identity is minted. A successful
run writes sorted generated-actor manifests to `Saved/BasementB/L_TempleB3.txt`
and `L_TempleB4.txt` for two-run comparison. Engine map metadata/GUIDs can differ,
so byte-identical .umap hashes are not the determinism criterion. Room IDs are
also editor-only TargetPoint labels, separate from entrance and spawn IDs.

## Validation and remaining work

Validation observations are recorded below after actual execution. Required
host checks remain: generate twice and compare manifests; open maps with the
actual camera at both orbit/zoom extremes; build navigation; traverse every
branch, circuit, threshold and ramp; sweep actual player/Atrocity/Giant Bat and
full Balork wings/weapon/capsule; check palettes/light leakage/overlap and fixed
exposure; test hub→B4→hub, explicit interaction, save/reload, invalid-destination
recovery, safe arrivals and permanent boss defeat with W3-04/W4. Explicitly
cook both maps and review a Linux packaged route. Windows remains deferred.
The V-01 door graphs and B4 subdivision openings remain authored reconstruction,
not historically verified topology. No G3 pass is claimed by this deliverable.

Observed in this isolated worktree (UID 1000, UE 5.8.3):

- `UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game`:
  exit 0, 549 seconds; editor `Result: Succeeded` (241.67 seconds), game
  `Result: Succeeded` (305.47 seconds). A temporary ignored Saved UBT config
  disabled UBA detours after exploratory compile failures; no engine/config
  source or shared header changed. The first exploratory builds exposed and
  corrected manifest pointer/area-ID typing errors, and were interrupted with
  exit 130 after compiler failures.
- Generator wrapper twice: exit 1 in 31/40 seconds, respectively. **Both
  SaveMap calls reported success on both runs**, with 22/13 spawn and 2/1
  directed portal markers. Engine exit 1 came from baseline Dev_Combat,
  Dev_Movement and L_Frontend maps being unhydrated LFS pointers, producing
  `Invalid value for PACKAGE_FILE_TAG`. Those out-of-scope files were untouched.
  This is not a clean generator process pass. An earlier default-cache run
  failed before generation because the Installed DDC graph had no writable
  nodes; its crash reporter attempted a network request blocked by the worker.
- Two sorted manifests (354 B3 actors, 164 B4 actors) compared identically:
  both `cmp` exits 0. All 35 spawn and three portal GUIDs were distinct.
  Temporary generated maps are retained under ignored Saved validation data,
  outside the submitted source diff; the coordinator must regenerate/hydrate
  the full checkout and commit map binaries through LFS.
- `bash -n build/generate-basement-b-maps.sh` and `git diff --check`: exit 0.
  Mock UID-0 fail-fast: exit 1 before invalid engine lookup; unexpected argument:
  exit 2; actual read-only generated B3 map guard: exit 1, then permissions restored.
- Source-geometry sampling reached all 22/13 spawn anchors from entry using a
  90 cm horizontal envelope on a 20 cm XY grid; all listed Balork retreat/patrol
  segments passed sampled 242 cm turn-radius clearance at intervals ≤10 cm.
  This independent source audit excludes ramp traversal, actual engine sweeps,
  animation, navigation and gameplay, and cannot establish G3.
- `UE_ROOT=... UE-LocalDataCachePath=<worktree>/Saved/DerivedDataCache bash
  build/run-tests.sh Lighthaven` failed during Installed cache initialization
  before running tests. A direct equivalent UnrealEditor-Cmd retry used
  `-ExecCmds="Automation RunTests Lighthaven; Quit" -TestExit="Automation Test Queue Empty"`
  plus the generator's explicit filesystem DDC, `-nocrashreports`, and
  `-ini:EditorSettings:/Script/UnrealEd.AnalyticsPrivacySettings:bSendUsageData=False`.
  It reached editor startup (reported 58.711 seconds) then SIGSEGV in
  `X11_ShowMessageBox`, called by DesktopPlatformLinux engine-installation
  discovery because the worker's ApplicationSettingsDir is read-only.
  No Automation index.json or completed tests were produced. Both process
  sessions failed at the tool boundary when their crash reporters requested
  blocked `datarouter.ol.epicgames.com`; no normal shell exit code or total
  timing was captured. Command logs are in the attempt report/library. A normal host user with a
  writable settings directory and hydrated baseline LFS maps must rerun the
  full suite. The two W3-04b test fixes mentioned in the task are expected
  coordinator work, not observed successful or failed test outcomes here.
