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
