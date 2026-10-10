# W5-06 evidence candidate

Task ID: W5-06. Base revision: `37737f20376c2d2d46fe596112a634e731fbb5e4`. Contract revision: 1. Result revision: submission commit (see receipt).

Owned changes: `Source/Lighthaven/Visual/LHVisualKit.*`, `Source/LighthavenTests/World/LHMapDressingTests.cpp`, generator capture vantages only, and dressing documentation. No binary assets submitted.

## Design and behavior

Design (a): retain the exact fitted `BuiltRecipe` and actor transform. A persistent `Placement` scene root saves position; the `Visual` procedural mesh is a transient default subobject with both `RF_Transient` and a transient property. Mesh sections and dynamic materials therefore cannot enter the map package. `PostLoad` builds geometry for editor package loads; `OnConstruction` supports authoring; `BeginPlay` rebuilds for PIE and packaged games. `Dress` remains entirely generator-side, including label-dependent fitting and graybox visibility changes. Geometry generation and collision recipes are unchanged. No marker/gameplay transform changes.

The engine BasicShapeMaterial is loaded with constructor `FObjectFinder` and retained by the nontransient `MaterialParent` UPROPERTY on the actor/CDO. This is a hard package dependency for cooking; no soft path lookup is introduced at runtime. The kit uses generated box vertices, no runtime mesh asset. Cook verification is reported separately below.

Framing audit: 144 rays per camera, horizontal FOV65, aspect16:9, pixel-center sampling, 6000cm maximum ray length. Rays intersect exact oriented recipe boxes and retained/visible static-mesh local bounds, bypassing collision because dressing must remain nonblocking. Static bounds are a conservative approximation for non-box retained engine primitives; this is a geometry coverage check, not a rendered-black-pixel test.

B4 source diagnosis: `FLinearColor(FColor::FromHex(...))` converts dark sRGB palette values to low linear albedo. Original main stone `514B42`, accent `733D35`, and floor override `695640` all absorb substantially more light than other styles. Change B4 stone/floor to `A69C89` and accent to `B78370`. These are Prototype presentation tuning (`LH_Prototype_v1`, W5-06, 2026-10-10, source URL null), not authentic mechanics. No light, exposure or post-process settings change. Host luminance/visual judgment remains necessary; palette diagnosis does not establish absence of local shadow issues.

## Validation

Final-source build command: `UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game`, exit0, editor19.18s and game4.70s, both `Result: Succeeded`. Initial fresh builds also succeeded (editor249.40s/game221.65s). UID1000; installed Linux UE5.8.3.

All three generator scripts ran twice (before/after camera changes), each process exit1, each commandlet result0. Final errors are the three unrelated baseline LFS pointer maps (Dev_Combat, Dev_Movement, L_Frontend). No generator error is called a clean process pass. Generator-time dressing totals match W5-05 exactly.

| Map | Bytes | Decimal MB | Pieces | Triangles | Base sections | First load + rebuild seconds |
|---|---:|---:|---:|---:|---:|---:|
| LighthavenTempleDistrict | 5,202,289 | 5.202 | 358 | 122988 | 706 | 0.642 |
| TempleB1 | 2,745,543 | 2.746 | 270 | 68520 | 536 | 0.339 |
| TempleB2 | 8,385,105 | 8.385 | 833 | 222048 | 1662 | 0.829 |
| TempleB3 | 3,883,384 | 3.883 | 427 | 108840 | 853 | 0.367 |
| TempleB4 | 1,849,889 | 1.850 | 221 | 47340 | 438 | 0.290 |

Total: **22,066,210 bytes / 22.066 MB / 21.044 MiB**, below25MB. Every map has zero dressing blockers. Timings are single headless-editor first LoadPackage calls in a fresh process, including I/O, dependent asset load and PostLoad mesh reconstruction, not isolated CPU mesh timings or cooked performance. Subsequent Maps tests reused packages, so their near-zero load timings are excluded.

Standalone final `Lighthaven.World.Dressing`: expected10/observed10 Success, no warnings/failures/notRun/inProcess, exit0, test duration4.507558s. All five Maps tests verify sections exist immediately after load, before their explicit deterministic rebuild; all meshes have RF_Transient; existing fingerprint/budget/collision/identity assertions pass. Initial audit expected10/observed6 Success and4 Fail (four map-level framing tests, five deficient cameras), exit255.

| Map | Camera | Before hits /144 | Before % | After hits /144 | After % |
|---|---|---:|---:|---:|---:|
| Area.LighthavenTempleDistrict | LH_Capture_1 | 144 | 100.00 | 144 | 100.00 |
| Area.LighthavenTempleDistrict | LH_Capture_2 | 144 | 100.00 | 144 | 100.00 |
| Area.LighthavenTempleDistrict | LH_Capture_3 | 144 | 100.00 | 144 | 100.00 |
| Area.TempleB1 | LH_Capture_1 | 130 | 90.28 | 130 | 90.28 |
| Area.TempleB1 | LH_Capture_2 | 131 | 90.97 | 131 | 90.97 |
| Area.TempleB1 | LH_Capture_3 | 116 | 80.56 | 142 | 98.61 |
| Area.TempleB2 | LH_Capture_1 | 124 | 86.11 | 124 | 86.11 |
| Area.TempleB2 | LH_Capture_2 | 120 | 83.33 | 142 | 98.61 |
| Area.TempleB2 | LH_Capture_3 | 57 | 39.58 | 136 | 94.44 |
| Area.TempleB3 | LH_Capture_1 | 140 | 97.22 | 140 | 97.22 |
| Area.TempleB3 | LH_Capture_2 | 142 | 98.61 | 142 | 98.61 |
| Area.TempleB3 | LH_Capture_3 | 111 | 77.08 | 144 | 100.00 |
| Area.TempleB4 | LH_Capture_1 | 141 | 97.92 | 141 | 97.92 |
| Area.TempleB4 | LH_Capture_2 | 144 | 100.00 | 144 | 100.00 |
| Area.TempleB4 | LH_Capture_3 | 116 | 80.56 | 144 | 100.00 |

Changed vantages/targets (cm), no other cameras moved:

| View | Old position → target | New position → target |
|---|---|---|
| B1/3 | (5000,200,1000) → (4100,1500,0) | (5000,600,1000) → (4300,1300,0) |
| B2/2 | (-2200,300,1000) → (-1400,1400,0) | (-2400,400,1100) → (-1800,1000,0) |
| B2/3 | (4500,5400,1000) → (3600,6300,0) | (6500,5000,1100) → (5800,5800,0) |
| B3/3 | (5300,5400,1000) → (4300,6400,0) | (5000,5600,900) → (4600,6100,0) |
| B4/3 | (3500,7100,1200) → (1600,7400,100) | (3100,7200,1000) → (2400,7500,0) |

B4 source albedo Rec.709 linear luminance (calculated, not rendered): stone .07175→.33690; accent .07239→.27470; floor .10029→.33690. The floor color override is included in the fix.

Arrival review: `build/review-arrivals.sh`, exit0, 9/9 PASS. Reviewed-list SHA256 before/after `e56cc528fcbdee98194c878edf84347d5a71f49eb6c23cf927e47634a724e5fc` is identical.

Full headless suite: command in `library/headless-tests.sh`, filter `Lighthaven`, exit0. Discovered/expected199, exported/observed199 Success states:193 succeeded without warnings,6 succeeded with warnings,0 failed/notRun/inProcess. Duration102.134834s. Failure names: none. LightingAudit passes unchanged; all five Dressing Maps and five CaptureFraming map tests pass. Warnings: BowRequiresQuiver, AI.StateMachine, AI.TwoEnemiesOneSettlement, G4.DeathAndChurchRespawn, Wave3.ArrivalSafety, Wave4.B1SpawnOnContinue. Warning details are preserved in FullSuite.json.

Cook/package attempted with the five explicit map URLs. Initial script exit1: AutomationTool could not clear its read-only HOME log folder. Redirecting `uebp_EngineSavedFolder` and `uebp_LogFolder` into Saved allowed execution. UAT did not pass the environment-appended cooker flags through; the cooker crashed for lack of writable DDC, and sandbox network policy stopped its crash reporter at datarouter.ol.epicgames.com (no normal process exit). Passing flags through `-AdditionalCookerOptions` fixed the DDC crash but encountered a ZenStoreWriter fatal because localhost:8558 was unavailable; its crash reporter was also stopped by network policy. The generated Zen locator was preserved under Saved/UAT after automatic review rejected an `rm -f` command.

Final supported loose-file retry added `-SkipZenStore` to explicit cooker flags. UAT exit25 (`Error_UnknownCookFailure`), cooker exit1 after255.71s, UAT4m23s. Summary:4 errors,2898 warnings; four unique errors are the DDC memory-fallback error and unloadable Dev_Combat, Dev_Movement, L_Frontend pointers. All five maps and engine BasicShapeMaterial have real loose cooked `.umap/.uexp` or `.uasset/.uexp` files, recorded with sizes/hashes in `library/partial-cook-manifest.json`. This observes the engine material entering cook output; it does **not** establish a clean cook, finished package, launch or runtime reconstruction. Staging/packaging and packaged piece-count-on-load checks were not run to completion. Prerequisites: writable DDC graph, supported loose-file cooking or a working Zen server, and hydrated baseline maps. No permanent project/system setting or engine installation was changed.

Generated source maps were restored byte-for-byte to their HEAD LFS pointers after all checks. ReviewedArrivals.tsv remains unchanged. No map, asset or reviewed list is submitted.

Evidence directory: `/home/brewerm/.herdr-projects/unreal-t4c/.state/worker-output/attempt-b39acc4ac65027c60d2e87e2c1e33eb53e9b00c0b83a86031b43bb8909327f6a/library/`: build/generator logs, Before/After/FullSuite/ArrivalReview JSON, exact headless helper, map sizes, partial cook manifest and package logs. No screenshot or binary asset evidence is fabricated.

Checks not run: rendered captures/luminance, direct PIE/play review, completed package/packaged piece counts and Windows. Display review belongs to the coordinator; package prerequisites are above; Windows is deferred by project memory. Known remaining uncertainty: B4 palette brightness and composition need host acceptance; conservative static-mesh bounding rays are not a rendered pixel audit; timings are editor load observations, not packaged timing or GPU profiling. No new source defect was observed by the executed suite. Next task: coordinator regenerate/review the source candidate, resolve clean cook prerequisites, verify packaged dressing counts, and measure the fifteen displayed views (especially B4).

## Integration notes

Regenerate maps from the submitted source; do not use the pointer maps for visual acceptance. Coordinator owns display captures and luminance assessment. Windows remains deferred. No source-backed mechanics introduced.
