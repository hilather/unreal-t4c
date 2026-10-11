# W5-05 procedural map dressing candidate

Base `15f568908b01c4a948089cfe12184debda48308f`, contract revision 1. All dimensions, colors, placements and budgets below are **Prototype presentation tuning**, `LH_Prototype_v1`, W5-05, 2026-10-10, source URL null. No mechanics values or historical art claims are introduced.

The three map generators call `LHMapDressing::Dress` after gameplay authoring and lighting coverage. Church/B1Cellar/B2Damp/B3Crypt/B4Ritual styles replace visible cube floor and cutaway wall surfaces with customized kit recipes. Geometry dimensions are adapted to existing rectangle-union strips; actor scales remain unit. The floor union, voids, wall collision, smooth ramps, nav contribution and aperture colliders remain the original generated actors. Visible graybox surfaces are hidden, with shadow casting disabled, after replacement succeeds. This intentionally preserves reviewed collision rather than introducing a second collision layer. All new recipes clear their collision arrays; kit meshes also have no navigation influence. No new blocking dressing can intersect an arrival, NPC, portal, turn or encounter pad.

Sloped 300 cm ramps receive signed kit stair recipes fitted to their existing run/rise. Ordinary/boss lintels receive kit arches; the hub's 320 cm apertures and B1/B2 entry baffle mouths also receive arches. Existing hub pew and altar-table presentation receives benches and an altar, retaining the original gameplay colliders. Broad room-floor corners receive a deterministic barrel/crate/table/bench/debris sequence, capped at 48 props per map. Local lights at fixture height receive nonblocking sconces; fill/ceiling lights above 350 cm do not. B4 receives a nonblocking altar at `(1600,7800,0)` in the ritual area. No closed preview door leaves are placed.

The generators validate the complete placed kit set before saving. Per-piece triangle and base-section ceilings remain the kit's recipe budgets. Additional conservative map ceilings are 6,000 pieces, 2,000,000 triangles and 12,000 base sections. These are CPU construction ceilings, not measured frame-time or GPU draw-call targets. No source marker ID, arrival transform, portal, encounter, NPC or spawn transform is edited.

## Fixed capture views

Each generated map has three native `CameraActor`s, named `LH_Capture_1` through `LH_Capture_3`, tagged `LH.Capture`, FOV65, unit scale. They do not autoactivate or change normal gameplay cameras. The capture script explicitly uses `EnableCheats`, `ViewActor <name>`, then `Camera Default`. Engine `UCheatManager::ViewActor` selects the actor as the view target; Default camera mode uses its camera component. Capture validates the logged camera-manager `ViewTarget.Target`, rather than assuming a pawn teleport selected the right camera. Three room views are always produced; `--overview` is retained as a compatibility option. Fifteen independent launches retain 120 warmup frames, 1280×720, 15-second lifetime and 55-second timeout. Exported sRGB Rec.709 mean, median and near-black fraction (<.02) are printed and written to `Saved/LightingCapture/<timestamp>/luminance.tsv`.

| Map | Camera positions (cm) | Look-at points (cm) |
|---|---|---|
| Hub | (1450,100,1100); (-1100,100,700); (-3700,6800,900) | (700,1200,100); (-600,500,0); (-3300,7400,100) |
| B1 | (1400,-2400,900); (-2500,200,900); (5000,600,1000) | (800,-1600,0); (-1800,1100,0); (4300,1300,0) |
| B2 | (1600,-6200,1000); (-2400,400,1100); (6500,5000,1100) | (900,-5200,0); (-1800,1000,0); (5800,5800,0) |
| B3 | (3900,-200,1000); (-600,1600,1000); (5000,5600,900) | (2900,700,0); (400,2500,0); (4600,6100,0) |
| B4 | (1100,100,900); (1700,2500,1100); (3100,7200,1000) | (600,900,0); (1000,3700,0); (2400,7500,0) |

No lighting settings change. Inherited host target means are hub .28 / B1 .27 / B2 .24 / B3 .19 / B4 .18. New material brightness must be measured on the host; these inherited values are not observations of dressed maps. LightingAudit retains its original exposure, light inventory, units and coverage assertions.

## Named remaining placeholders and limitations

* Scenic water/island boundary, church red aisle, descent arrow bars/heads, thin legacy tread strips, B3/B4 pale/dark/stain floor overlays remain engine primitive presentation. The kit does not supply water, signage, colored area overlays or arbitrary outline trim.
* Existing hub NPC primitive assemblies and B1 healer cube silhouettes retain their separate presentation owner; attached pieces and the `NPC.*` label prefix are excluded from surface replacement. Enemy presentation remains the roster owner's work.
* Hidden roof/roof slopes, invisible structural wall/floor/ramp colliders, nav bounds, fade metadata, portal/arrival/encounter authoring markers and other diagnostic actors remain generated primitives. They supply gameplay/authoring rather than new art. Existing floor voids remain intentional.
* Very narrow rectangle-union strips stretch a Floor400 recipe; this preserves exact bounds but does not promise uniformly sized paving. Cutaway walls retain their existing cap height; arches are taller, nonblocking presentation. In-play occlusion and camera readability require visual review.
* Sconces are visual proxies at existing light positions, not newly emitted light. Room props are decorative, without interaction or blocking collision. Their visibility and placement still require host visual judgment.

## Checks and integration

`Lighthaven.World.Dressing.Maps` expands to five map tests. It rebuilds serialized recipes, checks fingerprints, measured kit budgets and map totals, requires zero dressing blockers, checks the canonical entrance/spawn/portal inventories and transforms, freezes all 13 hub and two B1 healer NPC IDs/transforms against the base, and requires three capture actors. These checks do not replace arrival walk-tests, enemy aperture/leash clearance, navigation, rendering, clean cook or packaging.

The coordinator must regenerate all five maps, rerun `build/review-arrivals.sh` with the reviewed list unchanged (9/9), regenerate again to embed review flags if needed, run `LHValidateWorld`, the full suite, capture and judge all fifteen images, then clean cook/package. Generated maps and ReviewedArrivals.tsv are never submitted. Windows is deferred. Actual worker results are recorded below and in the attempt report; G5 visual acceptance belongs to the coordinator.

## Worker observations (Linux UE 5.8.3, UID1000)

Final-source editor/game build succeeded (exit0; editor18.93s, game3.27s). Final headless full suite exported182 expected/182 Success states:177 succeeded without warnings,5 succeeded with warnings,0 failed/notRun/inProcess; process exit0, test duration63.243s. All five Dressing map tests and LightingAudit succeeded. The initial completed run found a new test assumption error (B1's two existing healer NPCs were omitted); the corrected test freezes those identities and transforms too.

| Map | Pieces | Measured triangles | Base sections | New blockers |
|---|---:|---:|---:|---:|
| Hub | 358 | 122988 | 706 | 0 |
| B1 | 270 | 68520 | 536 | 0 |
| B2 | 833 | 222048 | 1662 | 0 |
| B3 | 427 | 108840 | 853 | 0 |
| B4 | 221 | 47340 | 438 | 0 |

The local arrival-review script completed9/9 (exit0), with the reviewed-list SHA256 unchanged. After the final B1/B2 arch addition/regeneration, full-suite ArrivalSafety also succeeded; its nine PASS ID/transform-hash rows match the unchanged reviewed list exactly. Final LHValidateWorld logged five-map identity/portal/entrance validation passed and commandlet result0. Its **process exit was1**, because startup also logged four errors: the Installed DDC has no writable node (memory fallback used), and Dev_Combat, Dev_Movement and L_Frontend remain unrelated baseline LFS pointers. This is not a clean commandlet exit.

The headless editor-game camera smoke exited0 and logged view-target selections1/2/3 in the hub. Actual capture returned1 at preflight because this worker has no X11/Wayland display. No rendered screenshots, dressed-map luminance, final camera framing, cooked performance or G5 visual acceptance are claimed. Serialized local procedural maps are approximately60–277 MiB each; clean cook, cooked size and load-time measurement remain open. Generated maps were restored to their original LFS pointers before submission.

Initial Automation/editor runs were interrupted by sandbox rejection of engine telemetry to datarouter.ol.epicgames.com. Reruns used the normal engine privacy override `-ini:EditorSettings:[/Script/UnrealEd.AnalyticsPrivacySettings]:bSendUsageData=False`, alongside the house-rule headless flags. No permanent project/system setting changed. Exact commands, interrupted/final logs, JSON reports, map sizes and arrival evidence are in this attempt's output `library/`.

## W5-06 lean serialization

Visual pieces retain fitted recipes and placement in saved maps. The procedural mesh is a transient default subobject attached to a persistent scene root; mesh sections and dynamic materials are rebuilt in PostLoad (editor load), OnConstruction (authoring), and BeginPlay (PIE/game). Generator-side Dress still performs label-based fitting and hides the same original surfaces. BasicShapeMaterial remains a hard UPROPERTY reference loaded by the constructor, so cooking can follow its dependency. Geometry coverage now has a headless 16×9/FOV65/60m capture audit. See [W5-06 evidence](map-dressing-w5-06.md) for sizes, load timing, camera changes, test results and remaining limits.

## W5-17 full-height far walls and church dressing (evidence candidate)

Base `ffc2ba7de210f6eabe9cbd87438237f854f5aeee`. Presentation tuning is
Prototype, `LH_Prototype_v1`, 2026-10-11; no mechanics values introduced.

Axis-aligned wall segments receive a 300 cm fitted Wall400 recipe only when a
continuous conservative projection of the proposed wall onto **every** authored
floor envelope is clear. The camera direction comes from pitch -55/yaw45,
arm1200, capsule center90/head180; northeast is behind the player. Testing all
floor slabs includes opposite sides of interior partitions, neighboring rooms,
and ramp elevation bounds. The retained solid wall footprint is excluded from
walkable centers. Any ambiguity retains a cutaway capped at 120 cm; rotated district
edges also retain cutaways. B3/B4 originally author visible 500 cm walls, so
unsafe segments are explicitly reduced to 120 cm presentation. All fitted
room walls start at ground level, rather than including the collider's
100 cm underground skirt. This rule is deliberately conservative rather than
promising two tall sides on every irregular room. Original actors, colliders,
nav contributions and gameplay transforms are untouched. Imported wall tiling
retains its upper section rather than stretching a low masonry rim.

The nave RedAisle is now a Church.Carpet recipe at `(800,1050,.1)` with its
existing 400 x 2100 cm extent. The imported carpet has its own ground-centered
pivot. Two imported 400 cm supports sit outside the north nave face at
`(200,2440,0)` and `(1400,2440,0)`, checked against all floor shadows. Entry leaves
use the existing 160 cm hinge-pivot asset, fold outward from `(640,-15,0)` and
`(960,-15,0)` at yaw 180 / 0, flat against their respective exterior jambs, and are clipped to 60 cm as foreground cutaways.
They do not close the 320 cm passage. Cropping preserves the original plank
scale. Full-height foreground leaves would occlude the aisle and are not used.
These support and open-leaf placements are prototype interpretations of the
nave/entry layout, requiring host visual review.

Non-B1 visual sconces within 375 cm of a raised wall mount on its inward face
at 180 cm. B1 sconces retain their original height/transform because their
transient torch-light components emit real light. No authored light, exposure,
post-process or lighting recipe changes. Generator logs identify each raised
wall reached by a point light's existing attenuation sphere and whether that
wall intersects a ray from that light to a room-side center probe. This is a
geometric light inventory, not rendered shadow or luminance evidence.

Capture cameras retain their original transforms unless the target ray crosses
a new raised wall; a blocked capture moves to a pitch -55/yaw45/1200 cm offset
from its original target and logs both positions. Generation rejects a still
blocked replacement. The existing fifteen-camera framing audits remain active.

Five GameplaySight cases independently use cropped imported envelopes as a broad phase, then intersect actual
LOD0 mesh triangles (or procedural oriented boxes), sampling authored flat and sloped floor planes on a
50 cm lattice. Centers overlapping authored obstacles expanded by 36 cm are
excluded; each remaining center receives camera-to-feet and camera-to-head
rays. This checks geometric walkable centers rather than actual nav projection;
arrival safety retains the separate physics/nav tests. The audit includes
existing props, fixtures, cutaways and arches as well as newly tall walls; it
fails rather than silently filtering inherited occlusion. Procedural fallback intersections remain conservative. The audit does not
model transparency, shader displacement, transient support meshes or gameplay
boom retraction, and does not claim rendered pixel visibility. Floor/stair
surfaces and the ground carpet are excluded from occluders. Exact observed
results and remaining limitations follow after local checks.

### W5-17 observed checks and unresolved acceptance

Final non-adaptive unity Linux UE 5.8.3 editor/game builds succeeded (exit0;
editor20.30s after the final leaf placement, game41.43s for unchanged runtime
sources). UID1000. `build/build-art.sh` exited0; all five environment imports
and creature import reported unchanged packages. The wrapper also rebuilt player
art locally; tracked player art was restored from the cached base LFS objects
before automation, matching the W5-15c test setup. No downloads or system changes.
All three final generators exited0, producing all five playable maps.

| Map | Pieces | Triangles | Base sections | Dressing blockers | Tagged tall walls |
|---|---:|---:|---:|---:|---:|
| Hub | 363 | 1,454,432 | 904 | 0 | 11 |
| B1 | 269 | 634,048 | 488 | 0 | 35 |
| B2 | 833 | 1,824,516 | 1,472 | 0 | 126 |
| B3 | 427 | 1,470,128 | 650 | 0 | 26 |
| B4 | 221 | 1,006,288 | 440 | 0 | 12 |

Final full automation exported **232 tests: 227 Success (with warnings), 5 Fail,
0 not run**, exit255, test duration282.854s. The five failures are exactly
`Lighthaven.World.Dressing.GameplaySight.Area.` followed by
`LighthavenTempleDistrict`, `TempleB1`, `TempleB2`, `TempleB3`, `TempleB4`.
Every new-wall/support/leaf ray assertion passed; the stricter assertion against
**all** dressing failed. The kit catalog, auxiliary Carpet binding, all five map
budgets/inventories, both framing families (fifteen cameras each), LightingAudit,
and ArrivalSafety succeeded. No capture target ray crossed a raised wall; all
fifteen capture positions stayed at the earlier table's coordinates.

| Map | Grid centers | Ray endpoints | Base blocked | Candidate blocked | New walls/supports/leaves blocked |
|---|---:|---:|---:|---:|---:|
| Hub | 13,318 | 26,636 | 349 | 348 | 0 |
| B1 | 6,382 | 12,764 | 113 | 113 | 0 |
| B2 | 14,498 | 28,996 | 145 | 105 | 0 |
| B3 | 8,243 | 16,486 | 3,287 | 581 | 0 |
| B4 | 10,334 | 20,668 | 2,393 | 325 | 0 |

The base comparison loads the exact cached committed playable maps with the same
triangle audit. Its new-piece inventory assertions intentionally fail because
base maps contain no new walls/carpet/supports/leaves; the blocked endpoint
counts above are observations, not a claim that the base passed. Remaining hits
include existing full arches, barrels/crates/benches/tables/debris, altar and
fixtures. B1 clutter and floor braziers occupy collision-free walkable space;
removing or relocating emitting B1 fixtures would also change lighting, which
this task forbids. Non-B1 fixture mounting reduces some inherited occlusion but
does not clear the global requirement. Detailed per-piece hit counts are in the
attempt's `library/sight-comparison.json`. **W5-17 is not acceptance-complete:
the full suite and all-dressing no-occlusion criterion remain failed.**

A separate read-only PythonScript commandlet inspected point-light-derived
components (including spotlights and transient B1 torch components) against the
actual tagged wall envelopes derived from retained source dimensions. It exited0.
`library/all-light-wall-links.json` identifies the light actor/component, source
position, wall label/bounds, sphere reach and center-probe intersection for every
matched pair. Hub/B1/B2/B3/B4 have 9/102/1,041/368/146 matched pairs and
1/8/89/76/21 intersected center probes. All matched lights have `CastShadows=false`:
these walls can potentially catch their illumination, but the component settings do not produce
shadow blocking from those local lights. Sphere/probe tests are conservative;
spot cones, ambient skylights, runtime movement/flicker, and rendered luminance
are not measured. No lighting, exposure or post-process settings were edited.

Final arrival evidence has nine PASS IDs/transform hashes matching the unchanged
ReviewedArrivals bytes exactly. Final `LHValidateWorld` exited0 and logged five-map
validation passed, **0 errors**,18 warnings,2.44s commandlet time. These are local
checks, not integration or gate approval. Rendered capture exited1 at preflight:
no X11/Wayland display. No screenshots, rendered light/shadow review, play,
package or cook results are claimed. Windows remains deferred. Tracked Content
and map files are restored to their exact base pointer/blob bytes before submit;
generated binaries and packages are not part of this candidate.

Initial iterations included an obsolete catalog-count failure (fixed by treating
Carpet as an auxiliary Church recipe), a triangle-audit variable-shadow compiler
error (fixed), and two occluded endpoints from outward entry leaves (fixed by
folding flat against jambs). The final table supersedes those earlier placements.
The first local orchestration script was interrupted after successful generation
by an edit while running; checks were rerun with stable commands. A diagnostic
Python attempt could not read a private recipe property; the completed helper
uses retained source dimensions instead. All final failures are retained openly.

Next: coordinator review of this partial candidate, scoped cleanup of inherited
arch/prop/fixture occlusion and a real nav-projected grid, then regeneration,
full automation/arrival validation and host review of fifteen rendered captures.
Do not interpret the passing new-piece assertions as completion of the broader
no-occlusion requirement or visual acceptance.
