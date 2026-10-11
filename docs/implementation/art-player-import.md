# W5-13 — Blender player import and presentation binding

Evidence candidate for coordinator review. This report records observed worker checks; it does not approve integration, visual acceptance, or a project gate.

## Handoff

- **Task ID:** W5-13.
- **Base / result revision:** `52eb9960460ddb467c59d1149b63803cea66ab09` / the candidate containing this document, recorded in the submission receipt and external `report.md`.
- **Contract revision:** 1; digest `5f8291c39248d10f8a6aa7b215022cb57965c5854784a585d4d2a591905d8c98`.
- **Owned paths:** `build/build-art.sh`, `Source/Lighthaven/Visual/Player/`, `Source/LighthavenTests/Visual/Player/`, this document. Generated player packages are local evidence, not committed binaries. The committed JSON receipt is in `Source/Lighthaven/Visual/Player/import-receipt.json`.
- **Behavior changed:** Imports W5-12 bodies/hair/atlases/actions; resolves the authority's appearance combinations; samples skeletal presentation with a complete procedural fallback; attaches equipment to named sockets; prevents a detached GAS avatar from selecting death for an alive saved character.
- **Source-backed mechanics:** Existing appearance policy (`Character/LHCharacterAuthority.cpp`), combat `Committed`/`Finished` events and saved health are consumed read-only. No rule, capsule, movement, collision, combat settlement, save schema, or authority code changes.
- **Provisional tuning:** Existing W5-12 authored proportions, colors and animation anchors remain Prototype presentation values. Existing 0.18 s recovery, 0.2 s hit window, walk/run selection and 90 cm visual floor offset are preserved. No authentic HP/XP/loot values are introduced.
- **Checks actually run:** Native editor/game builds; Blender export and GLB validation; full art pipeline; bounds/budget inventory and same-input skip; local map generation; full headless automation; arrival review; Linux B1 cook. Final results below.
- **Checks not run / prerequisites:** Windows build/package/launch is deferred under Linux-first project policy. No packaged executable launch or rendered regression capture was performed. Rendered grip, foot sliding and likeness acceptance require an RHI/display capture run and visual review; `-nullrhi` evidence cannot establish them.
- **Known defects / remaining decisions:** Source portrait/cloth fidelity and animation polish retain the W5-12 limitations. Rendered equipment fitting and transition polish remain for visual review. No art-source changes were required to import the rigs.
- **Next task:** Coordinator review of the evidence/code and reproduction of the import into the asset-owning checkout, then LFS publication of approved player packages and a current-camera arrival/equipment capture. Restore/hydrate content before reproduction; the Git candidate contains no map or asset binaries.

## Import and reproducibility

Build both native targets first using the farm recipe in `wave4/_env.md`. The importer calls editor-only native helpers on `ULHPlayerVisualComponent`; these use `USkeletalMesh::SetMaterials` to persist the material cache and retain runtime `MaterialSlotName` values. Accessing `ImportedMaterialSlotName` from game code fails because it is editor-only.

```sh
export UE_ROOT=/home/brewerm/Downloads/unreal
export BLENDER_ROOT=/home/brewerm/Downloads/blender-5.2.2-linux-x64
bash build/build-art.sh
# Isolate the new step when the environment/creature imports already exist:
LH_ART_PLAYER_ONLY=1 bash build/build-art.sh
# Reuse already exported, independently validated inputs:
LH_ART_SKIP_EXPORT=1 bash build/build-art.sh
```

The player export receipt hashes Blender's version, player source scripts, appearances manifest, and all eight output files (two GLBs and six PNGs). The import receipt hashes the importer, engine version, appearances manifest, actual input files, native helper source and compiled editor module. An unchanged receipt must match every output package's SHA-256 and byte size before skipping import/save. Changed read-only packages require the existing explicit `LH_ART_REIMPORT=1` convention; unchanged packages keep their bytes and permissions.

The engine's PythonScript commandlet enables Python for the invocation only; no project plugin/config change is made. Scratch conversion files and export receipts live in `Saved/ArtExport/`. Source GLBs are untouched: an import-only copy reflects glTF Z, including mesh positions/normals/tangents/winding, joint transforms, inverse bind matrices and animation translation/quaternion keys. This follows the creature pipeline's full-rig basis conversion and applies metre-to-centimetre conversion once through Interchange.

Each GLB is split into body, Cropped and Tied skinned meshes using the original rig/bind matrices. Only the body copy carries the eight clips. Hair retains its separate, identical body-specific skeleton and uses leader pose by bone name. A/B bind lengths are never silently shared. All six meshes get the four public sockets. Unreal sanitizes exported `Socket.Weapon.R`-style bone names into underscores; sockets keep the requested dotted names and bind to those sanitized bones. Blade/arrow sockets rotate local +X into the converted bone's local +Z; bow/quiver already extend along +Z.

Meshes and clips retain Interchange's stable paths. For example:

```text
/Game/Lighthaven/Art/Player/player_a/body/player_a-body/SkeletalMeshes/SK_body
/Game/Lighthaven/Art/Player/player_a/body/player_a-body/SkeletalMeshes/SK_bodyidle
/Game/Lighthaven/Art/Player/player_a/Cropped/player_a-Cropped/SkeletalMeshes/SK_Cropped
```

`MeshAssetPath`/`ActionAssetPath` construct these paths and the importer checks them. This avoids deleting/renaming animation packages held by rooted AnimSequencer controllers during re-import. Body and hair each preserve one LOD, source MeshDescription and all authoring data; the native storage helper clears only rebuildable cached `SoftVertices`, following the creature import model.

One small `M_Player` samples Base/Normal/ORM and multiplies Base by a linear Tint. Two clothing instances and six skin-tone instances share six 512 px textures. Normal/ORM disable sRGB; normal green is flipped for the external OpenGL bake. Imported duplicate material/texture packages are removed after native material binding.

## Runtime and upright spawn

The resolver accepts exactly the current body/face pairing, two hairstyles, three skins and StarterLinen outfit; a missing face follows the authority's matching-face normalization without modifying the record. Invalid appearances or any missing body, selected hair, skin material or required clip fall back to the procedural figure. Appearance IDs are included in the rebuild fingerprint, so records with identical proxy geometry can still change validity or asset selection.

The CDO holds both bodies, all four hair meshes, all sixteen clips and all six skin instances, preserving cook reachability. All skeletal components have collision and overlap disabled and cannot affect navigation. Existing decorative equipment geometry attaches to `Socket.Weapon.R`, `Socket.Bow.L`, `Socket.Arrow` and `Socket.Quiver.Back`; bow strings retain the existing decorative draw recipe. Arrow visibility remains limited to a committed bow presentation.

Pose mapping is Idle→idle, Walk→move, Run→run, Melee→melee, Bow→bow, Cast→cast, Hit→hit, Death→death. Sampling never emits gameplay notifies or calls impact/settlement. Instant attacks sample the authored contact/release anchor (0.5 s melee, 0.7 s bow/cast) and fit recovery to the existing 0.18 s window. Positive authoritative ImpactSeconds may sample the visual lead-in. The authority still owns every impact and completion.

The procedural death rotation is applied only to the fallback. Imported death is solely the baked skeletal pose, avoiding a second 90-degree collapse.

The code-level cause reproduced for the lying-idle condition is `ULHCombatComponent::IsAlive()`: it returns false when the GAS avatar is detached, even when saved character health is positive. The old presentation treated this lifecycle gap as death and rotated its floor root towards -90 degrees. Presentation now uses GAS health when this owner actually is the avatar, otherwise resolved saved health (unresolved/no record remains the prior alive default). Avatar gaps clear stale action/flinch state; restored alive presentation clears collapse. `HandlePlayerDeath` synchronizes saved resources before detaching, so resolved dead saved health still selects death. This changes presentation only.

The regression spawns a real `ALHCharacter` at capsule-centre Z=90, detaches GAS, confirms `IsAlive()==false` while saved health is 100, then checks idle, floor-root Z=0, world up, and imported head above pelvis. It also exercises invalid-appearance fallback, dead saved health, and restored alive orientation. This is headless reproduction of the faulty predicate, not a pixel comparison of the supplied host capture.

## Measured evidence

Observed native confirmations (uid 1000, installed UE 5.8.3): editor exit 0 / `Result: Succeeded` / 71.39 s; game exit 0 / `Result: Succeeded` / 50.39 s. Both used the two checkout-local XML configurations and `-WaitMutex -DisableAdaptiveUnity -UBASharedMemoryTempFile=true -NoUBA`, with checkout-local XDG/UBA directories. No HOME override or system configuration change was made. Non-adaptive unity compile units were built. UBA action-result-store warnings did not change the observed successful target results.

Both bodies were generated locally with Blender 5.2.2 LTS, factory startup, three threads, and the W5-12 scripts. Player GLB validation exited 0. Environment/creature sources were also exported locally and independently validated; their imports reported `B1_IMPORT_UNCHANGED` / `CREATURE_IMPORT_UNCHANGED`. Full `LH_ART_SKIP_EXPORT=1 bash build/build-art.sh` exited 0. The player step generated 49 packages under the 10,000,000-byte cap. A subsequent same-input player invocation exited 0 and reported both `PLAYER_EXPORT_UNCHANGED` and `PLAYER_IMPORT_UNCHANGED`, with all package hashes preserved.

Final measured type totals after re-import against the final compiled module:

| Unreal class | Bytes |
|---|---:|
| SkeletalMesh | 3,003,990 |
| AnimSequence | 2,563,186 |
| Texture2D | 1,712,017 |
| PhysicsAsset | 49,396 |
| Skeleton | 40,146 |
| MaterialInstanceConstant | 37,462 |
| Material | 8,099 |
| Total | 7,414,296 |

All six rest-bounds checks passed. Maximum absolute min/max error: **0.00000667572021484375 cm**, against the requested 0.5 cm limit. The receipt contains each expected/actual bound and every package's SHA-256/size.

Local generator commands `LHGenerateHubMap`, `LHGenerateBasementAMaps`, `LHGenerateBasementBMaps` exited 0 in 25, 35 and 36 s respectively, producing all five playable maps. The full headless command used Local filesystem DDC, null RHI, privacy/home-screen settings, `Automation RunTests Lighthaven; Quit`, `-TestExit=Automation Test Queue Empty` and `Saved/AutomationReportFinal`.

Final full suite: **217 expected / 217 observed / 217 Success with warnings / zero Fail / zero unfinished**, engine exit 0, reported test duration **450.0894775390625 s**. Failure names: none. All six `Lighthaven.Visual.Player` tests passed. Warnings include the headless SDL/Wayland initialization warning; they are reported as warnings, not clean successes. The preceding full run had 216 Success and one Fail, `Lighthaven.Visual.Player.CombatEventsAndSettlementGuard`, solely the old procedural “Nock drawn back” local-coordinate assertion. The final version tests the imported named arrow attachment and retains that old coordinate check for fallback; damage/mana/cooldown/RNG/impact and reward/save identity assertions passed.

`build/review-arrivals.sh` exited 0 (80 s), **9/9 PASS**. Reviewed TSV SHA-256 before/after: `e56cc528fcbdee98194c878edf84347d5a71f49eb6c23cf927e47634a724e5fc`, byte-identical. Generated map/content pointer restoration and final cook results are recorded in the completed evidence below.

The first B1 Linux cook exited 0 in **1,912 s** (cold shader cache). Every one of the 49 player package paths was present under `Saved/Cooked/Linux/Lighthaven/Content/Lighthaven/Art/Player`; their cooked packages and sidecars totaled **2,943,957 bytes**. After final native changes, the full art script was run again and exited 0, preserving the unchanged environment/creature imports and re-importing the player against the final module signature. Source package serialization sizes changed slightly on re-import; the committed receipt and table above describe that final import.

Final-receipt confirmation: unchanged-input player script exit 0 with `PLAYER_EXPORT_UNCHANGED` / `PLAYER_IMPORT_UNCHANGED`; six player tests rerun against that receipt: **6 Success with warnings, zero Fail, zero unfinished**, engine exit 0 (50 s wall time, 0.5277974605560303 s reported test duration). This includes the upright-spawn/imported-head test, all twelve appearances and actions, fallback, attachment, movement/collision, combat and reward settlement guards. Shell syntax, importer Python AST parsing and `git diff --check` also passed.

Final warm B1 Linux cook: **exit 0, 105 s, 714 packages cooked / zero remaining**. All 49 final-receipt player package paths were present; player cooked package/sidecar total was **2,943,261 bytes**. No packaged launch is inferred from this cook.

Scope cleanup completed after the last engine process exited: all tracked Content files (including generated `.umap` files) and `Config/Lighthaven/ReviewedArrivals.tsv` were restored byte-for-byte to the base revision. `git diff` reports no changes in those paths. New player packages were moved to ignored local evidence and copied into the attempt library (under 8 MB); none is committed. Only the four authorized path groups remain changed.

## W5-13b: session-backed captures and no-session preview

Direct room captures previously launched without a character record. A ready zero-health GAS avatar therefore selected death, and the empty appearance could not bind the imported player. The no-record presentation now supplies the existing A/Cropped/LightWarm/StarterLinen choices, ignores missing-session combat resources/events, and stays upright in idle. Saved-character and attached-avatar health still select death normally; no gameplay resources are written by presentation.

Development capture startup is opt-in with `-LHCaptureSession`, optionally `-LHCaptureAppearance=B` (A by default). `ALHGameMode::InitGame` prepares the registered launched map before possession. The GameInstance subsystem builds an ephemeral `FLHWave2Session` with in-memory generation storage, then uses the production Bind → Preview → Execute(CreateCharacter) → Flush/readback path. This supplies the production starter inventory, equipment, resources, identity and provenance. The validated snapshot is installed at the map's registry fallback through `InstallTravel` / `PopulateEncounterCheckpoint`. Normal controller possession then binds the actual player state, restores GAS, places the reviewed arrival, and starts the production encounter director. Capture profiles never write to the user's save directory. These startup/HUD/logging additions are guarded out of both Shipping and Test builds.

`build/capture-map-screenshots.sh BINARY` now requests that session, checks `LH CAPTURE SESSION PASS` (positive live health, imported body, completed arrival), and hides the HUD with `-LHCaptureHideHUD`. `--appearance B`, `--keep-hud`, and `--no-session` expose the requested alternatives. Fixed `LH_Capture_1/2/3` CameraActors, `ViewActor`, FOV and screenshot framing commands remain as before. Session creation occurs before those commands, so it does not teleport the camera back to the pawn afterward. The helper self-test also checks all session/HUD argument combinations.

New native regressions require the default imported A body and upright idle with no record, including an attached zero-health combat avatar; test full-health possessed session players for A and B, starter equipment, imported body and production encounter spawning. Existing combat/death/settlement tests remain intact. The session fixture's marker review flags are synthetic test inputs; the separate arrival walk-test checks actual generated geometry.

Assumptions: the capture character uses the frontend's existing starter choices and creation answers; the launched registry fallback is the initial capture arrival; respawn remains the production church default. This is a temporary capture session, not a saved playable profile or an alternate rules implementation. Captures require a Development executable with the imported player packages cooked or available to the editor. Shipping execution is not supported by this diagnostic script.

W5-13b observed checks (UE 5.8.3 Linux, uid 1000): final Editor Development build exit 0 / `Result: Succeeded` / 245.24 s including mutex wait; Game Development exit 0 / succeeded / 163.90 s; Game Shipping exit 0 / succeeded / 430.46 s including mutex wait. Builds used the farm recipe's non-adaptive unity and UBA flags, local XDG/config directories, and no HOME override. The Development binary contains all three capture switches and the capture log marker; UTF-8/UTF-16LE/UTF-32LE scans find none of them in the Shipping executable. All capture entry declarations/definitions and HUD handling are also preprocessor-guarded out.

Both headless real B1 `-game -LHCaptureSession` starts (A and B) exited 0 and logged `LH CAPTURE SESSION PASS health=30 imported=1 possessed=1`. The focused final player suite exited 0: **8 expected / 8 observed / 8 Success with warnings / 0 Fail / 0 unfinished**, reported test duration 0.8681284785270691 s. It includes the new zero-health no-record default body and A/B production possession/encounter assertions. The initial fixture run had a duplicate transient-world-name crash; the final fixture uses unique packages with the map's short name and the final focused run supersedes it.

Local hub/A/B generation commandlets returned result 0; process exits were 1/1/0, with the first two containing the expected invalid-package-tag errors from remaining baseline pointer maps. All five playable maps were generated. Existing art packages and remaining non-playable maps were hydrated from SHA-256-matching objects already in the local LFS cache; no assets were downloaded. `build/review-arrivals.sh` exited 0 with **9/9 PASS**. Capture-script self-test, shell syntax and `git diff --check` passed.

Rendered camera/HUD/pixel checks and packaged launch are **not run**: this isolated worker has no `DISPLAY` or `WAYLAND_DISPLAY`, and produced native target executables rather than a cooked/staged package. Headless body binding is not a visual review or a packaged-content check. Windows checks remain deferred.

Final W5-13b full headless suite: **221 expected / 221 observed / 221 Success with warnings / 0 Fail / 0 unfinished**, engine exit 0, reported duration **266.715576171875 s**. All eight player tests passed in this run. The initial full attempt was interrupted by a sandbox-blocked Epic analytics request after 48 completed tests; the completed rerun used `-NoAnalytics` and the build brief's EditorSettings usage-data/crash-report privacy overrides. No pass is attributed to the interrupted run.

After the last engine process exited, all tracked Content files and `Config/Lighthaven/ReviewedArrivals.tsv` were restored byte-for-byte to the attempt's base revision, including generated `.umap` and hydrated `.uasset` pointers. Only authorized source/script/document paths remain changed. No binary or rendered evidence is committed; the attempt library contains logs, automation reports and the Shipping literal-exclusion check.
