# W5-09 — B1 lighting and atmosphere evidence candidate

Task ID: W5-09.

Base revision / result revision: `54f919e4da7d86d58ddc7fa03aa0da53858de14b` / submission candidate commit (see canonical receipt).

Contract revision: 1.

Owned paths / binary assets: `Source/Lighthaven/Visual/` excluding Player and Monsters; B1 lighting/post sections of `Source/LighthavenEditor/Commandlets/LHGenerateBasementAMapsCommandlet.cpp`; `Source/LighthavenTests/World/LHLightingAuditTests.cpp`; visual tests; `docs/implementation/lighting.md`; this report. No binary assets, maps, shared gameplay schemas or reviewed-arrival data are delivered.

## Behavior changed

The B1 visual pieces now own their warm light at flame-local `(0,25,14)` cm. Build, PostLoad, construction and BeginPlay rebuild the transient component without requiring changes to saved recipes. Rebuilding a fixture reuses its light; changing to another style or a non-fixture recipe removes it. Position-seeded, game-time sine flicker is deterministic at equal simulation times and bounded to ±6%. The generator retains its original inputs to dressing, then removes B1's old point-light actors. Removing them before dressing would also remove all visible fixtures: `LHMapDressing.h` creates a Sconce for every point light below 350 cm.

B1 uses .12 cool skylight instead of .8 neutral, 59 localized amber lights and one cool player-area fill. Fog stays fixed while the child fill follows the currently possessed pawn, including respawn/repossess, and hides with no pawn. Post-process adds AO, mild flame bloom, restrained warm highlights/cool shadows and vignette. Exposure remains physical/manual EV100 2.5. All exact Prototype tuning values, performance policy and capture criteria are in [lighting.md](lighting.md).

The generator geometry/helper, gameplay authoring, route-check and floor-layout sections were compared byte-for-byte with the base revision (SHA-256 evidence in `library/gameplay-generator-source-invariance.json`). Every gameplay transform is identical by construction: no geometry, arrivals, spawns, portals, camera placements, NPCs or collision expressions changed in the generator. B2 follows its previous branch. Hub/B3/B4 generators are untouched. The original B1 fixture actor transforms and recipes also stay intact.

The floating object was **not a freestanding Torch ID accidentally bound to Sconce**. `LHMapDressing.h:138–145` explicitly creates `Shared.Sconce` at the generic light positions, all zero yaw. All B1 fixtures now receive a slim nonblocking floor stand beneath the existing assembly, including near walls: invisible full-height gameplay colliders cannot prove a cropped 120 cm visual wall supports a fixture at 250 cm. A downward WorldStatic-only trace finds an upward surface, excludes characters/loot, and rejects penetrating/vertical wall hits; bounded retries ignore rejected actors. A missing floor produces no guessed stand. This is presentation support only; no blocker or nav contribution is added.

Imported B1 walls receive a closed stone core reaching the exact fitted crop endpoints and top. The broad faces sit .1 cm inside each imported outer face. Arches receive closed uprights and a crown above their segmental opening. A separate procedural component uses the existing imported stone material with effectively infinite clip bounds, 100 cm UV units and UV-aligned orthogonal tangents for its normal map. No texture rebake or mesh reimport is needed. Typical additions: wall 12 triangles/one section, arch 36/one, stand 24/one. Totals account for these additions. Original procedural fallback and other styles remain available.

## Visual reasoning and limits

The concept's right half was viewed only as a target. It was not copied, transformed, sampled into an asset, or used as generation input. The three coordinator gameplay captures were inspected directly; their uniform bright floor and largely disconnected flame/light positions motivated localized warm pools and reduced broad fill. No new Unreal render or Blender render is presented as evidence. The existing Blender diorama has different wall height/thickness, composition and illumination and cannot predict Unreal pixel acceptance.

Independently measured baseline full-frame normalized Rec.709 exported sRGB luma, using ImageMagick RGB8 bytes and Python arithmetic:

| Existing image | Mean | Median | Share < .02 |
|---|---:|---:|---:|
| L_TempleB1-room1.png | .3553 | .3862 | .1049 |
| L_TempleB1-room2.png | .3066 | .3251 | .1018 |
| L_TempleB1-room3.png | .3405 | .3408 | .0116 |
| blender-mock-room.png | .0736 | .0180 | .6818 |

Predicted new room means: roughly .20/.18/.21, with uncertainty at least ±.05. These are visual-tuning hypotheses, not measured results. The .15–.25 mean/<.05 near-black acceptance criteria remain unchanged. Room1/2 already fail near-black due largely to exterior void; lowering interior fill alone cannot guarantee a full-frame pass. The final cool fog radiance (.10,.14,.20), still capped to .12 opacity, aims to soften distant void to a faint blue-grey (predicted sRGB luma approximately .02–.04 at saturated fog opacity); that prediction depends on the engine fog/exposure path and also needs host measurement. The coordinator must measure the same views and inspect players, enemies, loot, dark corners, stands, arch openings and cut edges. A gameplay-surface-only metric may explain a failure but does not replace the full-frame criterion.

Performance policy: 59 attached torch lights (12 former landmark positions + 47 former coverage positions), one pawn fill, one skylight; **zero shadow-casting torch/fill lights**. No Lumen/ray tracing changes; the existing renderer disables dynamic GI/reflections. Fog is nonvolumetric, with no volumetric froxel or shadow-light allocation. The existing character `SpellLight` is an additional point light while the Light ability is active, with its original shadow setting; it is outside this profile and untouched. Light overlap/GPU cost has not been profiled on the 1050 Ti. Small light radii limit overlap but unshadowed light can bleed through walls; AO cannot replace true occlusion.

Source-backed mechanics: none changed. Provisional tuning introduced: all lighting, grade, haze, cap inset and stand dimensions/trace bounds are Prototype presentation choices, source W5-09 / concept visual comparison, retrieval 2026-10-10, source URL null. No value is represented as authentic T4C mechanics.

## Validation

Engine available here is UE 5.8.3 Linux, uid1000. Both targets use non-adaptive unity with `-DisableAdaptiveUnity -UBASharedMemoryTempFile=true -NoUBA`. The first editor compile found a UE5.8 `TObjectPtr<APawn>` conditional deduction error in the new fill code; corrected with explicit `const APawn*` and rebuilt. No adaptive exclusions were used; the generated unity test translation unit contains both B1 test files.

Initial local generation ran `LHGenerateHubMap`, `LHGenerateBasementAMaps`, `LHGenerateBasementBMaps`, `LHGenerateDevMaps` and `LHGenerateFrontendMap`: exits 1/1/1/1/1, wall times 24/22/20/20/16 seconds. Every requested map was written. Their only logged errors were AssetRegistry `PACKAGE_FILE_TAG` errors scanning the remaining original LFS pointer maps before generation. B1 dressing reports 270 pieces, 1,235,002 submitted triangles, 564 base sections and zero blockers: +2,724 triangles and +164 sections over the imported-kit baseline, including all 59 stands. The first five-map `LHValidateWorld` returned 0 (28 seconds), logging validation passed for all five maps.

The focused `Lighthaven.Visual+Lighthaven.World.LightingAudit` run observed **17 expected / 17 Success**, zero failed/not-run/in-process, exit0, 37 seconds wall / 4.429 seconds reported test duration. It covers fixture light lifecycle, exact 59+1 point-light inventory, unchanged other-floor lighting, deterministic flicker, pawn movement/respawn/possession fill behavior, cap fit/aperture, floor-contact including a penetrating wall, and collision/nav invariants. An earlier 16-test run preceded compilation of the added fill lifecycle test; the 17-test rerun supersedes it. The later full delivery run also covers the final cap-tangent and haze-radiance changes.

Build/editor/cook/package/play checks actually run on the delivery code:

| Check | Observed result | Time |
|---|---|---:|
| Non-adaptive unity LighthavenEditor | `Result: Succeeded`, exit0 | 67.45 s engine / 68 s wall |
| Non-adaptive unity Lighthaven | `Result: Succeeded`, exit0 | 29.49 s engine / 30 s wall |
| Final `LHGenerateBasementAMaps` | B1/B2 generated, exit0, no logged errors | 19 s wall |
| Full headless `Lighthaven` | 211 expected / 211 observed Success: 203 clean + 8 with warnings; 0 failed/not-run/in-process; exit0 | 257.646 s test / 290 s wall |
| Standalone arrival review | 9/9 PASS, exit0; reviewed list unchanged | 72 s wall |
| Final `LHValidateWorld` | Five maps validated, zero errors, exit0 | 21 s wall |

The full run includes all visual tests, the B1 lighting audit, five dressing checks and 15 camera framing views. Framing checks inspect geometry envelopes, not rendered images. The cap regression checks UV-aligned unit tangents, orthogonality and handedness on actual generated masonry; the fixture regression includes a ray starting inside a tall wall and still reaching the floor. No map or gameplay transform change is inferred from image metrics.

An earlier full run before the final tangent/haze refinement completed 211 tests with 210 Success (203 clean + 7 warnings), one failure, exit255, 275.642 s test / 309 s wall. The sole failure was `Lighthaven.Integration.G4.SessionInventoryRollback`, matching the brief's known F9: “exact live state rollback incl corpse and request journal” and “independent durable rollback”. It passed in the delivery run; this task did not modify its implementation or claim to fix that flake. Both runs are retained as trimmed evidence.

Reproduction uses `UE_ROOT=/home/brewerm/Downloads/unreal`, `XDG_CONFIG_HOME=$PWD/Saved/BuildEnvironment/config`, checkout-local DDC and the pinned commands in `wave4/_env.md`. The headless common arguments are:

```sh
"$UE_ROOT/Engine/Binaries/Linux/UnrealEditor-Cmd" "$PWD/Lighthaven.uproject" \
  '-ExecCmds=Automation RunTests Lighthaven; Quit' \
  '-DDC=(Local)' "-LocalDataCachePath=$PWD/Saved/DerivedDataCache" \
  -nullrhi -unattended -nosound -nop4 -NoCrashDialog \
  '-ini:EditorSettings:[/Script/UnrealEd.AnalyticsPrivacySettings]:bSendUsageData=False' \
  '-ini:EditorSettings:[/Script/UnrealEd.CrashReportsPrivacySettings]:bSendUnattendedBugReports=False' \
  '-ini:Engine:[ConsoleVariables]:HomeScreen.EnableHomeScreen=0' \
  "-ReportExportPath=$PWD/Saved/W509/FullReport" '-TestExit=Automation Test Queue Empty'
```

Generators/world validator replace `-ExecCmds` and automation-report arguments with `-run=<commandlet>`; the focused suite uses `Lighthaven.Visual+Lighthaven.World.LightingAudit`. Arrival review uses `env 'UE-LocalDataCachePath'="$PWD/Saved/DerivedDataCache" bash build/review-arrivals.sh`.

All 34 locally hydrated/generated binary files were restored to their exact original Git LFS pointer bytes before submission (8 maps, 26 art packages). `Config/Lighthaven/ReviewedArrivals.tsv` remained byte-identical, SHA-256 `e56cc528fcbdee98194c878edf84347d5a71f49eb6c23cf927e47634a724e5fc`. `git diff --check` passed; the submitted delta contains only owned source/tests/docs. These are observed checks, not a rendered visual acceptance or G5/task-verification declaration.

Results and evidence paths: canonical attempt output `report.md` and trimmed `library/` artifacts; raw local logs remain in ignored `Saved/W509/`. Baseline art packages were hydrated from the existing local Git LFS object store after SHA-256 verification (26 packages); no network download. The `git lfs checkout` wrapper reported LFS unavailable, so object contents were read directly for local validation only. Source export with installed Blender completed without renders; no new art is delivered.

Checks not run and concrete missing prerequisite: rendered Unreal capture/packaged visual review/GPU frame timing require the coordinator's host display/package and GTX 1050 Ti run; this worker is headless. Windows packaging/launch remain deferred under Linux-first project policy. NullRHI is structural/behavioral evidence only.

Known defects or remaining decisions: actual luminance and readability are unmeasured for this tuning; exterior void may prevent the full-frame near-black target. Existing fixture grid remains visible as standing torches, not an authored wall-only arrangement. Zero-shadow lights can leak through thin walls. Floor/stair clipped ends are not capped in this wall/arch repair. Caps are fitted backing, not newly sculpted stone courses. Their final seams and arch appearance require host rendering.

Next task and integration notes: coordinator reviews the candidate, regenerates B1 to replace legacy lighting and install atmosphere/post-process, reruns review-arrivals/validation and packages/captures three room views. Retain the existing B1 imported kit assets; no art import change is required. Runtime lights/caps/stands rebuild in existing maps, but the reduced skylight and removal of old map lights require regeneration. Do not accept visual/G5 gates from the headless audit. No push, main merge or binary commit is part of this attempt.
