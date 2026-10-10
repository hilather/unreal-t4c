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

## W5-10 creature import and presentation candidate (blocked validation)

Base: `0fb6dc50c59ff2d4cf2c218796480e1044290227`. This section describes a
source/tooling candidate, **not an observed successful creature import**.

`build/build-art.sh` now builds and validates both creature batches after B1,
then invokes an embedded Unreal Python importer. Destinations are
`/Game/Lighthaven/Art/Creatures/<kind>/SK_<kind>` and
`A_idle/move/attack/hit/death`. The importer checks skeleton compatibility and
rest bounds against each source validation JSON within 0.5 cm. It fails if
Interchange does not produce exactly one combined skeletal mesh. A small
skeletal master and per-creature instances bind explicit sRGB base, normal
(with green flipped), and linear ORM textures. Generated package inventories
report bytes per creature and reject totals over **25,000,000 bytes**.

B1 and creature receipts now live under their respective `Content/` art roots.
The coordinator must commit these JSON receipts alongside the LFS packages.
An unchanged signature plus every output package SHA-256 skips all import/save
operations. Changed inputs against read-only lockable packages stop unless the
asset owner explicitly sets `LH_ART_REIMPORT=1`; that opt-in grants owner write
permission on existing packages. A legacy checkout without a committed receipt
requires one authorized rebuild. Receipts are written only after a successful
import. Byte stability across repeated Blender exports and Interchange
reimports has **not been measured**; the skip path preserves bytes when the
generated GLB/signature and inventory match. No receipt or binary is in this
worker candidate.

**Axis checkpoint:** the environment's vertex-only reflection cannot be copied
to skeletal GLBs: joint transforms, inverse bind matrices and animation tracks
must receive the same basis change. This candidate uses native glTF skeletal
conversion and rejects a bounds mismatch. Native conversion correctness,
animation handedness, winding and multi-part weapon combining remain untested.
If this checkpoint fails, extend the import-only conversion consistently for
the whole rig (or export FBX from a disposable Blender scene); do not change
the visual lane's authored exports or relax the bounds tolerance.

`ULHMonsterVisual` optionally loads eleven meshes and five clips per mesh as
hard CDO references, retaining their transitive skeleton/material/texture cook
dependencies when packages exist before cook. An absent mesh, absent clip or
incompatible skeleton retains the procedural recipe. `Build(...,false)`
explicitly exercises that branch in the collision/fallback test. Imported
components have no collision, overlaps or navigation influence. Animation
position is manually sampled: source attack contact frame 30 maps to the
authority's impact delay. Animations never invoke damage or root motion.
Authored dimensions are retained; capsule sizes and gameplay code are unchanged.

**Weapon limitation:** the candidate retains the source's skinned weapon parts
and named bones in a combined mesh. It does not yet create separately removable
weapon components attached to sockets. This is a remaining W5-10 requirement,
not a claim of completed weapon attachment.

Development builds expose `lh.CreatureLineup [index 0..10] [close]`; Shipping
excludes the command implementation. It creates generic presentation actors at
the current player's room, cycles visual attacks, and selects a review camera.
There is no enemy runtime specification, AI or reward authority on these
actors. Use it in a lit B1 room. `build/capture-creature-lineup.sh <Development
binary>` requests gameplay and closer images for all eleven IDs under ignored
`Saved/CreatureCapture/`. The camera is a review approximation (55-degree
overview / 40-degree close), not a measured match to the gameplay spring arm.
Room clearance, packaged command startup timing and visible captures remain
unvalidated.

Checks actually run in this attempt:
- Shell syntax for both scripts, embedded Python AST parsing and
  `git diff --check`: passed.
- B1 Blender export and validator: 12 meshes / 9 textures, **4,841,071 bytes**
  including manifest. The following Unreal process exited 1 because the game
  module `Lighthaven` could not be found; creature build/import was not reached.
- First editor build failed before compilation: UBA attempted to create
  `/home/brewerm/.herdr-farm-homes/codex-sol/.epic` on a read-only filesystem.
  The documented project-local configuration and UBA root avoided that initial
  error, but the retry remained at the UBA action queue and was interrupted.
  No compiler success or runnable test module is claimed.

The new catalog test covers all eleven distinct mappings; the imported-assets
test requires real packages and checks source rest rulers and compatible clips.
Exact import bounds checks are in the importer. Full-suite counts, generated
maps, arrival review 9/9, cook, package and host display captures require a
successful build/import and remain unverified. Existing map and arrival bytes
were not edited. Windows validation remains deferred by the Linux-first
decision. See the attempt report for final command exits and evidence.

## W5-10b — compiled creature binding; import budget remains blocked

Task ID: W5-10b.
Base revision / result revision: `2ddebf7e823ae8d26b4a2ebd1aeaa8ea3b5927d0` / submission candidate (see attempt receipt).
Contract revision: 1.
Owned paths / binary assets: `build/build-art.sh`, `build/capture-creature-lineup.sh`, `Source/Lighthaven/Visual/Monsters/`, `Source/LighthavenTests/Visual/Monsters/`, and this document. Generated maps and art are local validation inputs only; no binaries are submitted.

Behavior changed: the import-only glTF conversion reflects the entire rig, including node translations/quaternions/matrices, inverse bind matrices, animation translations/quaternions, vertices, normals, tangents and triangle winding. Authored exports remain untouched. Native conversion originally reflected the asymmetric slime's Y bounds: actual approximately [-44.669, 41.075] versus authored [-41.075, 44.669] cm. The fixed importer checks every body's signed rest bounds within the original 0.5 cm tolerance. The independent conversion check reflected every one of the eleven GLBs twice and recovered its original JSON transforms and binary data.

Goblin, goblin warrior and Balork now import a body skeletal mesh and a separate `SM_Weapon`. The importer verifies rigid 100% weighting to `Weapon_R` or Balork's `Weapon_Main`, applies that bone's inverse bind matrix to produce bone-local geometry, and creates `WeaponSocket` on the body. The native component attaches an independently removable, non-colliding static mesh to that socket. An editor-only implementation of `ConfigureWeaponSocket` is necessary because UE 5.8 marks socket names read-only in Python; its game implementation returns false. Body and weapon participate in the inspection/cleanup collection, so collision tests no longer inspect an empty collection when imported art is active. Missing required weapons/sockets retain the procedural fallback.

The imported-assets test now reconstructs the weapon's reference-pose vertices through its socket bone and checks the complete body-plus-weapon signed rest bounds against the authored validation JSON. Collision tests remove and rebuild each weapon and exercise the explicit procedural branch. No gameplay, capsule, AI, reward or save implementation changed.

A reload regression exposed that direct Python assignment to UE 5.8's transient skeletal Materials field did not persist its material references. Force-saving alone did not fix it. `ConfigureCreatureMaterial` calls native `SetMaterials`, which updates the persistent `SkeletalMaterialsInfo` cache. A separate reload commandlet then checked the correct MI on all eleven bodies (exit 0); the imported-assets test now asserts this binding. Both latest targets rebuilt with the helper: editor `Result: Succeeded`, exit 0, 64.74 s; game `Result: Succeeded`, exit 0, 81.47 s. The preceding material-regression test build also succeeded in 59.69 s.

Source-backed mechanics: none changed.
Provisional tuning introduced: no gameplay tuning. Numerical tolerances and rest rulers are presentation evidence from authored creature exports; the 0.5 cm ruler remains unchanged. Rigid weighting uses a 1e-6 numerical tolerance.

Build/editor checks actually run: both pinned UE 5.8.3 native targets build. The ignored XML was installed in both `Saved/UnrealBuildTool/BuildConfiguration.xml` and `$XDG_CONFIG_HOME/Unreal Engine/UnrealBuildTool/BuildConfiguration.xml`, using the prescribed contents. Commands from the checkout root:

```sh
export UE_ROOT=/home/brewerm/Downloads/unreal
export XDG_CONFIG_HOME="$PWD/Saved/BuildEnvironment/config"
export UBA_ROOT="$PWD/Saved/UBA" HOME="$PWD/Saved/home"
mkdir -p "$HOME"
bash "$UE_ROOT/Engine/Build/BatchFiles/Linux/Build.sh" LighthavenEditor Linux Development "-Project=$PWD/Lighthaven.uproject" -WaitMutex -DisableAdaptiveUnity -UBASharedMemoryTempFile=true -NoUBA
bash "$UE_ROOT/Engine/Build/BatchFiles/Linux/Build.sh" Lighthaven Linux Development "-Project=$PWD/Lighthaven.uproject" -WaitMutex -DisableAdaptiveUnity -UBASharedMemoryTempFile=true -NoUBA
```

Initial editor: `Result: Succeeded`, exit 0, 198.29 s. Initial game: `Result: Succeeded`, exit 0, 245.19 s. Final editor after the socket helper/tests: `Result: Succeeded`, exit 0, 118.64 s. The game compiled the final binding in the preceding serialized build (`Result: Succeeded`, 164.62 s); final confirmation was up to date, `Result: Succeeded`, exit 0, 73.26 s including mutex wait. UE still logs its UBA executor, with compiler actions marked [NoUba]; no ten-minute build stall occurred. Deprecated configuration and action-result-store warnings were nonfatal.

All eight maps were generated locally. Generator invocations returned 1 because AssetRegistry scanned baseline LFS-pointer packages; real output maps were produced and exercised by the completed suite. B1 export validation: twelve meshes/nine textures, 4,841,071 source bytes. B1 imported packages remain 4,454,854 bytes.

`BLENDER_ROOT=/home/brewerm/Downloads/blender-5.2.2-linux-x64 bash build/build-art.sh` completed both creature export/validation batches and returned 255 at the unchanged 25,000,000-byte import gate. `LH_ART_SKIP_EXPORT=1` is an optional retry mode that still runs the independent validators and imports existing exports. A fresh validated-input import generated all eleven bodies, 55 compatible clips and three socket weapons, but its 136 packages total **55,231,963 bytes**, so the task's import budget does not pass. No approved creature receipt was written.

| Creature / shared package | Imported bytes |
|---|---:|
| rat | 5,729,211 |
| bat | 5,353,681 |
| slime | 1,904,711 |
| goblin | 4,317,973 |
| giant_spider | 6,310,820 |
| balork | 6,141,242 |
| goblin_warrior | 4,466,932 |
| atrocity | 5,081,484 |
| dungeon_bat | 5,430,430 |
| giant_bat | 5,221,497 |
| undead_bat | 5,266,919 |
| shared M_Creature | 7,063 |
| **Total** | **55,231,963** |

Interchange duplicate materials/textures are removed after rebinding. An isolated metadata experiment reduced the total only to 54,235,813 bytes (meshes 23,751,532; textures 22,245,736; animations 7,825,838; other 412,707); that experiment was discarded and is not part of the importer. No source geometry, textures or animation authoring was altered to conceal the budget failure. Interchange replacement of an interrupted partial creature directory also failed with invalid skeleton references; fresh import used an empty local directory. Forced reimport into existing assets remains unvalidated; the hash-checked skip is the byte-preserving path. Further authoring/storage optimization remains necessary.

Receipt signatures now include the external base/normal/ORM PNGs and bounds JSON, as well as GLBs, importer source and engine version. A signature check confirmed stable identical inputs and invalidation on each of these five per-creature input types. Completed but oversized imports get a scratch inventory under `Saved/ArtExport/`, never an approved Content receipt. A normal Blender rerun preserved all 55 source inputs and all 136 generated package hashes; it logged `CREATURE_IMPORT_UNCHANGED_REJECTED` and returned 255 without importing or saving. After the material correction, an additional validated-input rerun preserved the final 55 input and 136 package hashes and scratch inventory, with the same rejected-skip result. This establishes byte preservation through the rejected-candidate skip, not engine-native forced-reimport determinism or successful budget acceptance.

Scoped headless suite: 6 observed / 6 Success with warnings, zero failed/unfinished, exit 0. It exercised actual assets, exact signed rulers, socket placement, weapon removal/rebuild, collision/navigation isolation, fallback and presentation-invariant combat/settlement. First full suite: 211 observed / 209 Success with warnings / two Fail, zero unfinished, exit 255, reported duration 389.255 s. A concurrent editor cache write failed inside `DeathAndChurchRespawn`; the other failure was `SessionInventoryRollback`.

Corrected full suite, with `-AssetRegistryCacheRootFolder=$PWD/Saved/W510bFullIsolatedCache`: **211 observed / 210 Success with warnings / one Fail**, zero not-run/in-process, exit 255, reported duration **379.838 s**. All six monster tests passed. The sole failure was `Lighthaven.Integration.G4.SessionInventoryRollback`, with the two assertions "exact live state rollback incl corpse and request journal" and "independent durable rollback". These match the earlier W5-07c report; this attempt does not establish a full-suite pass or independently prove when that defect originated. The implementation/test needing review is outside this task's ownership. The warnings include NullRHI/SDL Wayland queries.

An intermediate full run with the new material regression failed only `CreatureImportedAssets` (211 observed, 210 Success with warnings, one Fail; duration 159.258 s); it caught the actual missing persisted materials. A subsequent final-content run was interrupted by sandbox denial of an editor request to www.google.com and emitted no index; it is not completed evidence. Final full suite after native material persistence, with the privacy/home-screen arguments restored and isolated cache: **211 observed / 210 Success with warnings / 1 Fail**, zero unfinished, exit 255; duration 171.593 s. All six monster tests, including the new persisted-material assertions, passed. The sole failure was `Lighthaven.Integration.G4.SessionInventoryRollback`, with the same two rollback assertions described above. The intermediate red run passed that test, so this evidence indicates variable failure rather than an established baseline-only defect. No full-suite pass is claimed.

Arrival review: `env "UE-LocalDataCachePath=$PWD/Saved/DerivedDataCache" bash build/review-arrivals.sh`, exit 0, 9/9 PASS. `Config/Lighthaven/ReviewedArrivals.tsv` stayed byte-identical (SHA-256 e56cc528fcbdee98194c878edf84347d5a71f49eb6c23cf927e47634a724e5fc).

Linux B1 cook command (with isolated cache/home and Local DDC):

```sh
"$UE_ROOT/Engine/Binaries/Linux/UnrealEditor-Cmd" "$PWD/Lighthaven.uproject" -run=Cook -TargetPlatform=Linux -Map=L_TempleB1 -SkipZenStore '-DDC=(Local)' "-LocalDataCachePath=$PWD/Saved/DerivedDataCache" "-AssetRegistryCacheRootFolder=$PWD/Saved/W510bCookPersistentCache" -unattended -nosound -nop4 -NoCrashDialog
```

Final cook exited 0: 665 cooked packages, zero remaining. Every one of the 136 editor creature package paths has a corresponding cooked `.uasset`; none missing. The 305 cooked creature files, including sidecars, total 36,268,168 bytes. The first cook exited 0 but included only 107/136 creature packages: 29 material/texture dependencies were absent because the body bindings had not persisted. A force-save retry reproduced that deficiency. The native-setter correction resolved it. Package inclusion does not establish a rendered or packaged-launch check.

Checks not run and concrete missing prerequisite: lineup capture preflight returned 1, "Host display required"; neither DISPLAY nor WAYLAND_DISPLAY is set in this worker. No rendered screenshot, visual-quality review, frame timing or packaged executable launch is claimed. Capture startup timing and room clearance remain unvalidated. Windows build/package/launch is deferred by the Linux-first decision.

Known defects or remaining decisions: the imported editor packages exceed the mandated 25 MB by 30,231,963 bytes. The full suite retains the inventory rollback failure. No budget exception, visual acceptance or gameplay gate is declared. Art-source authoring/storage changes and the rollback implementation are outside the owned paths.

Results and evidence paths: canonical attempt `attempt-68e9fec778df3140d81065dea00fa5fc6d997ef1b38c0d5d36d261e0f2156b6f`, worker-output `report.md` and trimmed `library/` evidence. No binary assets or full automation indexes/logs are copied there.

Cleanup: all 34 tracked B1 asset/map files were restored byte-for-byte from HEAD (26 B1 assets and eight maps); generated creature packages and B1 receipt are absent from Content. ReviewedArrivals.tsv remains byte-identical. The final diff contains only the five edited code/script paths and this owned document; shell syntax, both embedded Python ASTs and `git diff --check` passed. No `.umap`, `.uasset`, texture, arrival-registry or binary result is committed. Worker-output contains only small text/JSON evidence, below 40 MiB.

Next task and integration notes: the coordinator/art owner needs a scoped budget-reduction follow-up before importing these packages under LFS; the integrator should review the rollback failure; the host visuals lane must capture and review the lineup. Compile/import/binding evidence is a candidate for review, not task-success or G5 acceptance.
