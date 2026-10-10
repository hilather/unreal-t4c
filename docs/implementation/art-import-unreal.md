# W5-07c — B1 Unreal import and native binding evidence candidate

Task ID: W5-07c. Base revision: `62790e2e827972f29f81f9a72ce433826f55a622`. Result revision: submission commit/receipt. Contract revision: 1.

Owned paths: `build/build-art.sh`, `Source/Lighthaven/Visual/` excluding Player/Monsters, visual tests, `Source/LighthavenTests/World/LHMapDressingTests.cpp`, `docs/implementation/wave4/_env.md`, and this report. Binary assets are generated locally only and excluded from the commit. No maps or reviewed-arrival list belong to the deliverable.

Behavior changed: Blender export → independent export validation → the existing engine PythonScript commandlet → automated AssetTools/Interchange import. No custom commandlet/module/plugin dependency is needed. The command enables PythonScriptPlugin for its invocation only. Imported meshes live at `/Game/Lighthaven/Art/Env/B1/SM_<export-name>`; nine shared textures and four material instances use one small project master. Base is sRGB, normal is normal compression with green flipped from OpenGL +Y, ORM is linear masks with R/AO, G/roughness, B/metallic. Vertex tint is multiplied into base color; the flame slot uses the same master with a warm emission parameter. Nanite is explicitly disabled. No lights, exposure or post-process settings change.

The native glTF parser maps `(X,Y,Z)` to `(X,Z,Y)`; Blender exports `(author X,author Z,-author Y)`. Import-only GLB copies reflect glTF Z, reverse triangle winding and reflect normals/tangent Z and tangent handedness. Original source GLBs remain untouched. Every imported bounding box is checked against the manifest in centimeters within 0.2cm; this catches axis, unit and pivot mistakes including asymmetric sconces and signed stairs. Interchange's temporary per-piece graphs/texture copies are removed after material rebinding, and the complete imported folder is measured against 15,000,000 bytes.

`ALHVisualPiece` carries native CDO hard references to all twelve static meshes. Each mesh hard references the shared material instances, which reference the master and textures. This gives the cooker a reference chain even for the descending variant not necessarily placed in a selected map. Missing assets resolve to the original procedural path. Only B1Cellar recipes bind imported meshes; other styles and IDs keep their procedural visuals. The procedural source remains available, hidden on an imported binding, to retain existing blocker/recipe reconstruction behavior. Imported instanced meshes are transient, NoCollision, generate no overlaps and never influence navigation. Saved recipes rebuild them during PostLoad/construction/BeginPlay without a map format change.

## Fitting rule

Wall/floor fit comes from the first `BuiltRecipe.Geometry` box, reversing the existing generator's center subtraction and dimension fitting. Canonical 400cm meshes tile along wall length/height or floor X/Y at unit scale. A masked master clips the excess at the exact fitted envelope, in actor coordinates expressed through world-space dot products. Thus a 120cm cutaway keeps stone course/UV size rather than compressing a 400cm wall. Thickness may scale only within 0.85–1.15; unsupported thickness or more than 256 tiles uses the existing fallback. Unit-scale actor yaw handles Y-long walls. Instances share materials and batches; triangle accounting includes every submitted tile, even its clipped-away portion.

Signed stairs derive run/rise and midpoint from the refitted first slab. A descending path picks Stair600x120Descending. The nearest 600cm piece rotates by the difference between its canonical and fitted slope and clips to the shorter path, retaining UV density and normal slab thickness rather than anisotropic stretching. Longer-than-600cm paths or non-300cm widths fall back. Props and arches retain canonical origin. The saved generic wall and arch actors on a lintel both remain; this task does not remove either or alter the generator. Sconce orientation remains the saved zero yaw.

Provisional tuning: the 15% thickness limit, 256-instance cap, 6,000 triangles/three sections per imported mesh, 0.2cm import tolerance and material clipping epsilon 0.01cm are Prototype presentation/validation choices (W5-07c, 2026-10-10, source URL null). Geometry dimensions and texture families come from W5-07b's manifest. No game mechanics values are introduced.

## Validation

Blender export validator: 12 meshes, nine textures, 4,841,071 bytes including manifest, exit 0. Both native targets built with `-UBASharedMemoryTempFile=true -NoUBA`: final editor build 13.10 seconds; game build 170.24 seconds; both report `Result: Succeeded`.

Clean import produced the following 26 packages (4,454,854 bytes total). The initial commandlet returned 1 because its AssetRegistry scanned the checkout's pointer maps; the Python import itself completed successfully. After local map generation, the unchanged end-to-end rerun returned 0; all 26 output SHA-256 values and the receipt were unchanged. Idempotence here is an explicit verified skip of import/save when inputs and output hashes match; native forced-reimport byte stability has not been established.

| Package under B1/ | Bytes |
|---|---:|
| `MI_B1_flame.uasset` | 5,083 |
| `MI_B1_iron.uasset` | 5,066 |
| `MI_B1_stone.uasset` | 5,077 |
| `MI_B1_timber.uasset` | 5,088 |
| `M_B1.uasset` | 14,061 |
| `SM_Arch240.uasset` | 142,865 |
| `SM_Arch320.uasset` | 146,346 |
| `SM_Barrel.uasset` | 150,861 |
| `SM_Bench.uasset` | 116,425 |
| `SM_Crate.uasset` | 165,656 |
| `SM_Debris.uasset` | 115,932 |
| `SM_Floor400.uasset` | 173,491 |
| `SM_Sconce.uasset` | 113,452 |
| `SM_Stair600x120.uasset` | 167,605 |
| `SM_Stair600x120Descending.uasset` | 168,208 |
| `SM_Table.uasset` | 124,085 |
| `SM_Wall400.uasset` | 175,318 |
| `Textures/T_iron_basecolor.uasset` | 221,664 |
| `Textures/T_iron_normal.uasset` | 359,350 |
| `Textures/T_iron_orm.uasset` | 94,626 |
| `Textures/T_stone_basecolor.uasset` | 304,082 |
| `Textures/T_stone_normal.uasset` | 768,965 |
| `Textures/T_stone_orm.uasset` | 94,591 |
| `Textures/T_timber_basecolor.uasset` | 297,634 |
| `Textures/T_timber_normal.uasset` | 424,879 |
| `Textures/T_timber_orm.uasset` | 94,444 |

Scoped automated run: `Lighthaven.Visual+Lighthaven.World.Dressing`, 24 expected / 24 observed, all Success, exit 0. Both B1-specific tests pass: twelve actual mesh bounds/pivots, nine actual texture color-space/compression/green-channel settings, signed stair selection, fitted cutaway tiling, recipe rebuild, and other-style fallback. All five map dressing checks pass. All five framing checks pass, checking three views each (15/15); these are clipped bounding-envelope occupancy tests, not rendered screenshots.

`build/review-arrivals.sh` with the checkout-local DDC environment override returned 0, 9/9 PASS. The SHA-256 of `Config/Lighthaven/ReviewedArrivals.tsv` stayed unchanged. Five playable maps plus the dev/frontend maps were generated only for local testing; generator exits were 1 from the pointer packages scanned before each generation. Dressing generation reported B1: 270 pieces, 1,232,278 submitted triangles, 400 base sections, zero blockers.

Earlier runs exposed and corrected a suffix-match bug (`normal` contains `orm`), duplicate Interchange dependencies inflating packages to 23.7MB, a rooted-material-expression deletion assertion, and double initialization of the new test world. The first full suite was interrupted by editor telemetry's blocked domain; the next full attempt reached the new world-fixture crash before writing a complete report. They are not counted as full-suite passes. Corrected full headless suite: 209 expected / 209 observed, 208 Success (201 clean plus seven with warnings), one Fail, zero not-run/in-process; engine exit 255; reported test duration 476.401 seconds. The sole failure is `Lighthaven.Integration.G4.SessionInventoryRollback`: “exact live state rollback incl corpse and request journal” and “independent durable rollback” equality assertions. It reproduced in both full attempts that reached it. The rejected loot command and no-storage-write checks were not the reported failures. Its test is in `Source/LighthavenTests/Integration/LHWave2Tests.cpp:1630`/1633 and session/save implementation is outside this task's ownership; no fix there was made. These results do **not** establish a full-suite pass or prove the failure predates this task. All B1 import tests, all visual tests, all five dressing checks and 15 camera checks passed in the completed full run.

Linux B1 cook uses `-run=Cook -TargetPlatform=Linux -Map=L_TempleB1 -SkipZenStore` with the local filesystem DDC. The first attempt failed on the sandbox-unavailable Zen oplog (`Failed to delete oplog on ZenServer`); the loose-file retry has emitted all 26 B1 packages, including both stair variants, all nine textures, four instances and the master. Cooked B1 files plus sidecars total 5,284,052 bytes. The retry finished with exit 0: 529/529 dependency packages, zero errors and 19 warnings; commandlet duration 2,275.10 seconds (37m55s), dominated by first-time Vulkan shader compilation. This confirms Linux cook inclusion, including the descending variant. Source asset hashes remain unchanged. It is not a packaged executable launch.

## Remaining limits and integration

NullRHI tests cannot establish visual quality, masked-material shader appearance, normal-map shading or GTX 1050 Ti frame timing. Crop planes do not add solid cut caps, so wall cutaway edges and cropped stair ends need the later in-engine visual pass. Rotating a canonical stair rotates its individual tread surfaces slightly; smooth original gameplay colliders remain authoritative. Material graph topology is created once; changing that topology requires a fresh local import/master version rather than deleting rooted expressions during a commandlet. Current same-input re-runs avoid saves entirely using the receipt plus output hashes; this does not claim engine-native reimport is byte deterministic.

Next task: coordinator reruns the art script on the host, reviews/imports the actual binary assets under LFS, reproduces the Linux cook and exercises package/launch dependency inclusion, and assigns the subsequent visual/lighting pass. Windows packaging stays deferred. This report is an evidence candidate, not a G5 acceptance or task-success declaration.

Submission preparation: all eight locally regenerated `.umap` files were restored to their original Git LFS pointer bytes; the 26 untracked imported `.uasset` files were removed after testing/cook. `Config/Lighthaven/ReviewedArrivals.tsv` remained byte-identical (`e56cc528fcbdee98194c878edf84347d5a71f49eb6c23cf927e47634a724e5fc`). Only owned source/tool/test/document paths are submitted. `bash -n build/build-art.sh` and `git diff --check` pass. Trimmed evidence is in the canonical attempt output `library/`; no binary assets or full logs are included there.

### W5-07d — unity test namespace collision (2026-10-10)

The anonymous namespace in `LHRulesTests.cpp` exposed generic `Flags` to later engine headers in the same unity translation unit, producing `-Werror,-Wshadow`. All generic test flag constants now have file-specific names. The remaining anonymous helper namespaces (rules, Bible rules, UI presenter) are named per file; helper imports are scoped to test bodies, including the formerly global world-test import. Test registrations, flag values and assertions are unchanged.

Validation used UE 5.8.3, uid 1000, and fresh project intermediates: move `Intermediate/Build/Linux` aside under ignored `Saved/` before the build. Ignored `Saved/UnrealBuildTool/BuildConfiguration.xml` sets `BuildConfiguration.bUseUnityBuild=true`, `bUseAdaptiveUnityBuild=false`, `bAllowUBAExecutor=false` (also the deprecated `bAllowUBALocalExecutor=false`) and `UnrealBuildAccelerator.SharedMemoryTempFile=true`. The generated `Module.LighthavenTests.cpp` included every test `.cpp`, including both implicated files; no adaptive exclusions were observed.

```sh
export UE_ROOT=/home/brewerm/Downloads/unreal
export XDG_CONFIG_HOME="$PWD/Saved/BuildEnvironment/config"
export UBA_ROOT="$PWD/Saved/UBA"
# Clean fallback build, following slow/interrupted wrapper attempts:
for target in LighthavenEditor Lighthaven; do
  bash "$UE_ROOT/Engine/Build/BatchFiles/Linux/Build.sh" "$target" Linux Development \
    "-Project=$PWD/Lighthaven.uproject" -WaitMutex -DisableAdaptiveUnity \
    -UBASharedMemoryTempFile=true -NoUBA
done
# Subsequent wrapper confirmation:
bash build/build-linux.sh --game
```

Clean fallback: editor `Result: Succeeded` (exit 0, 146.76 s); game `Result: Succeeded` (exit 0, 130.43 s). Wrapper confirmation: both `Result: Succeeded` (exit 0 overall; 14.55 s editor, 24.82 s game). UE 5.8 still uses its UBA executor with detouring disabled under `-NoUBA`; nonfatal action-result-store warnings occurred.

After locally generating the five playable maps and running `build/build-art.sh` with the pinned Blender, the scoped headless invocation was:

```sh
"$UE_ROOT/Engine/Binaries/Linux/UnrealEditor-Cmd" "$PWD/Lighthaven.uproject" \
  '-ExecCmds=Automation RunTests Lighthaven.Visual+Lighthaven.World.Dressing+Lighthaven.Rules; Quit' \
  '-DDC=(Local)' "-LocalDataCachePath=$PWD/Saved/DerivedDataCache" \
  -nullrhi -unattended -nosound -nop4 -NoCrashDialog \
  '-ini:EditorSettings:[/Script/UnrealEd.AnalyticsPrivacySettings]:bSendUsageData=False' \
  '-ini:EditorSettings:[/Script/UnrealEd.CrashReportsPrivacySettings]:bSendUnattendedBugReports=False' \
  '-ini:Engine:[ConsoleVariables]:HomeScreen.EnableHomeScreen=0' \
  "-ReportExportPath=$PWD/Saved/W507dAutomationReport" '-TestExit=Automation Test Queue Empty'
```

Observed exit 0: 38 successful tests (Rules 14, Visual 14, World.Dressing 10), zero failed/not-run/in-process; one success with a GameplayCueNotifyPaths fallback warning in `Lighthaven.Visual.Monsters.CatalogDeterminism`. Map generation and art import returned 1 from baseline pointer-package AssetRegistry errors; the generated playable maps and 4,454,854-byte art receipt existed, and scoped tests exercised them successfully. Map pointers were restored and generated art moved under ignored `Saved/` before submission. No package, rendered review, Windows build, or gameplay gate is claimed by this compile fix.
