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

## W5-09b — registered before the game world's scene exists

Task ID: W5-09b. Base revision: `653ed5e4d3d2a25c144e657f87d34a7414f4cede`.
Result revision: the candidate recorded in the canonical submission receipt.
Contract revision: 1. Attempt: `attempt-ebac2890f68c2a51e5a96e1b432543e116c7d2646aa122106792186916326a08`.

The coordinator's W5-09 packaged captures measured B1 means
**.0062/.0008/.0006**, near-black shares **.92/.9995/.9995**. They supersede
the unmeasured predictions earlier in this document. The visible emissive flames
and faint pawn pool do not establish that the torch components illuminated the
scene. The original audit proved component inventory/settings, not registration
at the correct point in a loaded game world's lifecycle.

### Observed cause and engine path

An instrumented baseline preserved the original registration behavior. With
freshly regenerated B1/B2, `UnrealEditor-Cmd Lighthaven.uproject
/Game/Lighthaven/Maps/L_TempleB1 -game -nullrhi -seconds=3` exited 0 and logged
every fixture at creation, PostLoad and BeginPlay:

| Stage | Fixtures | Game world | Initialized | Scene exists | Registered |
|---|---:|---:|---:|---:|---:|
| ConfigureCreated | 59 | 1 | 0 | 0 | 1 |
| PostLoad | 59 | 1 | 0 | 0 | 1 |
| BeginPlay | 59 | 1 | 1 | 1 | 1 |

All 59 had 1800 lm, 700 cm attenuation, visible=1, hidden=0, ownerHidden=0,
Movable mobility, affectsWorld=1, inverseSquared=1, shadows=0 and unlimited draw
distance (0). For example `LHVisualPiece_210` was at `(200,-2000,250)`, with its
light at `(200,-1975,264)` and local offset `(0,25,14)`. The full fixture receipts
are in `library/game-baseline-excerpt.log` in this attempt's worker output.

The installed UE5.8.3 source establishes why that early registration loses light
delivery in a render-capable game:

- `Engine/Private/Components/ActorComponent.cpp:2512–2527`: `ExecuteRegisterEvents`
  calls `OnRegister` first, setting registration, then creates the render state
  only if the world already has a scene (and the application can render).
- `Engine/Private/World.cpp:2472,2597`: `InitWorld` allocates the scene, then marks
  the world initialized. The observed torch PostLoad registration precedes both.
- `Engine/Private/Actor.cpp:6235` and `ActorComponent.cpp:1982`: ordinary actor
  component registration and explicit registration skip already registered
  components. W5-09's `Configure` also registered only newly created components.
- `ActorComponent.cpp:2698`: marking a render state dirty requires an existing
  render state; flicker intensity updates cannot repair this case.
- `Engine/Private/Components/LightComponent.cpp:977`: adding the actual light to
  the scene occurs in `CreateRenderState_Concurrent`.

These paths are relative to `/home/brewerm/Downloads/unreal/Engine/Source/Runtime/`.
NullRHI intentionally yields renderState=0 even for valid lights. It proves the
early registration sequence here; the missing renderer registration follows from
the source path and remains subject to the required host rendered retest. No
NullRHI result is described as a pixel or packaged rendering observation.

### Behavior changed and numeric acceptance check

Owned paths changed: `Source/Lighthaven/Visual/LHB1Lighting.cpp/.h`,
`Source/Lighthaven/Visual/LHVisualKit.cpp`,
`Source/LighthavenTests/World/LHLightingAuditTests.cpp`,
`docs/implementation/lighting.md`, and this document. No binary assets or schemas.

The fix attaches and positions the transient light before registration, defers
manual registration until the world is initialized, and retries an existing
unregistered component. Normal actor registration can now include the loaded
torch when the game scene exists. Runtime logs include each fixture's lifecycle,
world and relative positions, units/intensity/radius, visibility, world influence,
mobility, shadows and distance limits. Editor-world logging is suppressed.

No tuning changes were needed to fix registration. The 59 torches, 2000 K,
1800 lm ±6% flicker, 700 cm radius, 12 cm source radius, .12 sky, pawn fill,
EV100 2.5, AO/haze/grade, cut caps and stands are retained. No shadow-casting torch
or Lumen is added. No generator, gameplay, arrival, collision or navigation
placement expression changes.

`Lighthaven.World.B1LoadedGameLightingAudit` loads an independently
instanced serialized B1 package as `EWorldType::Game`. Before initialization it
inspects `PersistentLevel->Actors` directly and requires all 59 torch components
to be unregistered. `TActorIterator` filters uninitialized actors and cannot be
used for that checkpoint. After initializing components and dispatching actor
BeginPlay, it checks all 59 registered lights and their settings/positions;
render-capable execution also requires their render states. The temporary world
and context are destroyed after the test.

The test computes nominal unoccluded direct lux from the registered visible
lights, with white input tint, physical lumen conversion, inverse-square falloff,
squared attenuation window and surface cosine. [lighting.md](lighting.md)
records the equation, exact UE source references and approximation limits. All
59 floor/vertical receiver pairs must have lit pools (at least 19/80 lux), four
centres at least 10 lux, and four selected corners under 6 lux. The floor probes
are at z0 below each flame; vertical probes are 1 m sideways and 64 cm below it.
They are receiver planes, not a claim that each freestanding torch has a wall.
These checks use nominal unticked intensity; the existing flicker regression
separately bounds modulation to ±6%.

The independent native reconstruction of the old 12 landmark + 47 grid lights
uses their original 900/1500/1800 lm profiles, z250 and radius1400. Its centre
estimates are 15.92–29.85 lux versus 11.43–19.77 for W5-09b; selected corners are
4.54–10.88 versus 2.69–5.46. Near the first fixture, total floor/vertical values
are 33.48/108.39 new versus 42.56/127.58 old. The old host capture mean was
approximately .3 at EV100 2.5. Retaining the reduced broad fill predicts
**approximately .20, uncertainty at least ±.05**, aiming at .15–.25. Temperature,
materials, sky/fill, fog, exposure and tonemapping prevent an exact lux-to-sRGB
mapping. The full-frame near-black requirement remains <.05. A host capture,
not this nominal estimate, must decide visual acceptance.

Source-backed mechanics: none changed. All photometric thresholds and capture
predictions are **Prototype presentation tuning**, provenance W5-09b / installed
UE5.8.3 source and the coordinator's W5-09 and legacy-grid observations, retrieval
2026-10-10, source URL null. They are not authentic T4C mechanics.

### Validation and reproduction

Observed environment: uid1000, installed UE5.8.3 Linux, no host display variables.
All builds use fixed unity (`-DisableAdaptiveUnity`) with
`-UBASharedMemoryTempFile=true -NoUBA`; both B1 tests and the lighting audit remain
inside `Module.LighthavenTests.cpp`. UBA reported unsuccessful cache-result stores
but the target builds completed with `Result: Succeeded`; no compile error was
hidden or treated as success.

The diagnostic-only baseline game run was exit0 / 30 seconds. The completed
regression against the original registration code was **1 expected / 1 Fail,
59 fixture-specific early-registration errors**, exit255 / 38 seconds, test
duration .781 seconds. No other assertion failed. Its historical report name is
`Lighthaven.World.LightingAudit.B1LoadedGameWorld`. The delivered sibling name
is `Lighthaven.World.B1LoadedGameLightingAudit`: nesting below the existing
simple-test name concealed the original five-map audit in discovery, so the
identifier was corrected before the final full run.

During test development, an incomplete initial run passed without examining
pre-init actors, and an added count check then failed with 0 examined instead of
59. Replacing `TActorIterator` with the level's actor array produced the meaningful
59-error baseline above. These earlier runs are retained in
`library/baseline-test-development.json`, not presented as evidence that the
baseline lifecycle passed. The fixed focused run observed 17/17 Success before
the name correction, including the new game-world test; the final full inventory
supersedes that focused inventory.

The fixed `-game` run was exit0 / 23 seconds: **59/59 unregistered with
initialized=0/scene=0 at creation and PostLoad; 59/59 registered with
initialized=1/scene=1 at BeginPlay**. See `library/game-final-excerpt.log` and
`library/game-lifecycle-comparison.json`. NullRHI renderState=0 is expected and
is not evidence of rendered acceptance.

Exact build invocation (repeat for `LighthavenEditor` and `Lighthaven`):

```sh
UE_ROOT=/home/brewerm/Downloads/unreal
XDG_CONFIG_HOME="$PWD/Saved/BuildEnvironment/config" UBA_ROOT="$PWD/Saved/UBA" \
  bash "$UE_ROOT/Engine/Build/BatchFiles/Linux/Build.sh" LighthavenEditor Linux Development \
  "-Project=$PWD/Lighthaven.uproject" -WaitMutex -DisableAdaptiveUnity \
  -UBASharedMemoryTempFile=true -NoUBA
```

The headless editor invocation uses the checkout-local DDC arguments in
`wave4/_env.md`; `library/run-editor.sh` preserves the exact common flags and
`library/build-target.sh` the timed target invocation. The common editor flags
include `-nullrhi -unattended -nosound -nop4 -NoCrashDialog`,
`'-DDC=(Local)'`, `-LocalDataCachePath=$PWD/Saved/DerivedDataCache`, the HomeScreen
disable and analytics/crash-report privacy overrides. Operations are:

```sh
# After copying library/run-editor.sh to Saved/W509b/run-editor.sh:
bash Saved/W509b/run-editor.sh generate-final -run=LHGenerateBasementAMaps
bash Saved/W509b/run-editor.sh game-final /Game/Lighthaven/Maps/L_TempleB1 -game -seconds=3
bash Saved/W509b/run-editor.sh full-final \
  '-ExecCmds=Automation RunTests Lighthaven; Quit' \
  "-ReportExportPath=$PWD/Saved/W509b/FullFinalReport" '-TestExit=Automation Test Queue Empty'
bash Saved/W509b/run-editor.sh validate-final -run=LHValidateWorld
env UE_ROOT=/home/brewerm/Downloads/unreal \
  XDG_CONFIG_HOME="$PWD/Saved/BuildEnvironment/config" \
  'UE-LocalDataCachePath'="$PWD/Saved/DerivedDataCache" bash build/review-arrivals.sh
```

The 26 art packages and eight maps were hydrated locally from existing Git LFS
objects, verifying SHA-256 and byte length before use; no network asset download
or art reimport. B1/B2 were regenerated for the checks. The full B1A generator is
unchanged from the base (SHA-256
`04f264ad17d9c28fcf84576b69c01a2f4661de04a7c4fac32a6271aab1c7fb70`).

Delivery checks actually run:

| Check | Observed result | Time |
|---|---|---:|
| Non-adaptive unity LighthavenEditor, final test identifier | `Result: Succeeded`, exit0 | 74.52 s engine / 75 s wall |
| Non-adaptive unity Lighthaven | `Result: Succeeded`, exit0 | 184.77 s engine / 185 s wall |
| Final `LHGenerateBasementAMaps` | B1/B2 generated, zero logged errors, exit0 | 27 s wall |
| Fixed B1 `-game -nullrhi` | 59 deferred before initialization / 59 registered at BeginPlay, exit0 | 23 s wall |
| Full headless `Lighthaven` | **212 expected / 212 Success: 204 clean + 8 with warnings; 0 failed/not-run/in-process**, exit0 | 283.949 s test / 312 s wall |
| Standalone arrival review | **9/9 PASS**, exit0; reviewed list unchanged | 87 s wall |
| `LHValidateWorld` | Five maps validated, **zero errors**, exit0 | 69 s wall |

Failure names in the final full run: **none**. Both
`Lighthaven.World.B1LoadedGameLightingAudit` and `Lighthaven.World.LightingAudit`
are explicitly present and Success. The eight warning-bearing tests are
`Lighthaven.AI.StateMachine`, `Lighthaven.AI.TwoEnemiesOneSettlement`,
`Lighthaven.Integration.G4.EarnedMagicRoute`, `EarnedMeleeRoute`,
`EarnedRangedRoute`, `SessionInventoryRollback` (same G4 prefix),
`Lighthaven.Integration.Wave3.ArrivalSafety`, and
`Lighthaven.Review.W603.TravelCooldownAndResult`. The previously reported
inventory rollback flake did not fail this run; this task does not claim to fix it.

Final per-fixture and sample lux values are in `library/final-lux-evidence.txt`;
the test inventory/counts/warnings are in `library/full-suite-summary.json`.
Build logs, trimmed game/generator/validator receipts, red-regression evidence,
arrival IDs/hashes, run timings and exact command wrappers are in `library/`.
Raw local logs and automation JSON remain in ignored `Saved/W509b/`; they are
not copied wholesale to worker output.

All **34** locally hydrated/generated map/art files were restored to their
original LFS pointer bytes after the final run. The reviewed-arrival file stayed
byte-identical (SHA-256
`e56cc528fcbdee98194c878edf84347d5a71f49eb6c23cf927e47634a724e5fc`).
`git diff --check` passed. Only the six owned source/test/doc paths listed above
are delivered; no `.umap`, `.uasset`, reviewed-arrival data, push or main merge.

Checks not run and concrete missing prerequisite: a fresh cooked/packaged
rendered launch, luminance screenshots and GPU timing require coordinator
packaging and the host display/GPU session, which this worker does not have.
The delivered test's render-state branch requires a render-capable RHI and was
not exercised under NullRHI. Windows packaging/launch remain deferred under
Linux-first policy. These are structural/game-world observations, not a G5,
packaged rendering or verified-task-success declaration.

Known defects or remaining decisions: actual post-fix capture means, near-black
share and player/enemy/loot readability remain unmeasured. Existing unshadowed
light leakage, freestanding grid placement and black exterior void remain;
scalar direct-lux receiver probes cannot evaluate them. No lighting thresholds
were relaxed to claim visual acceptance.

Next task and integration notes: coordinator reviews this candidate, rebuilds
and packages it with the existing regenerated W5-09 B1 and imported art, then
captures the same three B1 views. Inspect the per-fixture BeginPlay receipts in
that render-capable game (including renderState=1), compare means against
.15–.25 and near-black <.05, and assess actual pools/readability. B1 regeneration
is only necessary if integrating into maps older than W5-09; this lifecycle fix
works from existing saved recipes. Source generator and gameplay transforms
are unchanged. The canonical output `report.md` is an evidence candidate.

## W5-09c — localized pools, mounted sconces and recessed masonry

Task ID: W5-09c. Base revision: `d50722362008e244fe13895161628aaa2afa19d3`.
Result revision: the candidate in the canonical submission receipt. Contract revision: 1.
Attempt: `attempt-de4b3b1b16819705778e1e0326de26a8aeda26718d5d7525964a4a1c94841f89`.
This section is an evidence candidate, not rendered visual acceptance.

### Changes and visual reasoning

The supplied W5-09b room-1 capture and the concept's right half were inspected.
The supplied capture mean `.246` and near-black share `.12` are coordinator
observations, not new measurements. The floor's warm flood, tall pole fixtures
and almost continuous wall faces are visible in that image. The concept is a
visual target only; no reference pixels are used in assets.

The B1 flame profile is now **1000 lm, 2900 K, 375 cm attenuation radius** with
physical inverse-square falloff and the existing ±6% deterministic flicker.
The 3.75 m radius bounds individual pools; lowering the flame from 264 cm to
about 104 cm increases local floor illumination, so intensity also drops.
The existing cool `.12` skylight, pawn-following 450 lm/750 cm fill, EV100 2.5,
nonvolumetric haze, AO and grade remain. All torch/fill lights are movable and
cast no shadows; no Lumen or renderer setting changes are introduced.

B1 dressing uses the original light positions as seeds. Within 240 cm of an
actual visible wall segment, it mounts a sconce 0.5 cm outside that surface,
turns local +Y into the room and puts its origin 30 cm below the wall top.
The usual 120 cm cutaway therefore holds the plate at 90 cm and light at
104 cm; the imported flame tip remains below the wall top. A 45 cm endpoint
margin keeps plates clear of segment cuts. It does not infer support from an
invisible full-height gameplay wall. Remaining seeds become low iron braziers
with a 58 cm bowl, four 10 cm legs and 70 cm base fitted to a traced floor.
Seeds without ground are discarded. Relocated flame positions closer than
300 cm are deduplicated. Other floor styles follow the unchanged fixture path.
Fixtures have no collision or navigation contribution. Older elevated Sconce
recipes retain broad floor supports until maps are regenerated.

The wall defect has a geometry cause: W5-09's backing sat **0.1 cm** inside the
outer faces and covered the imported **3.8 cm bevels and 5 cm mortar recess**.
The source mesh builder already recesses its mortar for this reason. Backing
now sits 5 cm behind both faces; 1 cm perimeter strips close wall cut ends,
tops and bottoms without filling the broad joints. Arch backing is also
recessed while preserving its crown/opening. The imported albedo multiplied by
vertex colour, tangent normal map and linear ORM channels (R ambient occlusion,
G roughness, B metallic) remain connected as before. No texture, material asset
or UV scaling is changed. Recessed block edges and the lower, grazing warm key
are the intended remedies; their rendered response still requires inspection.

Source-backed mechanics: none changed. All dimensions, photometric values,
thresholds and predictions here are **Prototype presentation tuning** from the
W5-09c concept/capture comparison and installed UE5.8.3/source-art inspection,
retrieval 2026-10-10, source URL null. No value claims authentic T4C mechanics.

### Numeric audit and capture prediction

`Lighthaven.World.B1LoadedGameLightingAudit` retains the independent serialized
game-world loading and pre-scene registration regression. It now checks
actual floor receivers beneath the fixtures, mounted wall faces, inward
orientation, fixture spacing and zero collision/nav influence. Fixed one-metre
room sample positions establish pools and gaps independently of where lights
were placed. The audit compares their nominal unoccluded direct lux with a
negative control restoring 1800 lm and 700 cm on the same relocated positions.
This control tests the flooded profile; it is not a reconstruction of every
original W5-09b fixture transform.

Predicted full-frame sRGB-luma means for the three standard captures are
**approximately .19 / .18 / .20, uncertainty at least ±.06**. These are tuning
hypotheses, not measurements or a lux-to-pixel conversion. The acceptance band
remains **.15–.25**. The pre-existing exterior void can still dominate the
near-black share; the previous `<.05` target is not claimed passed. Rendered
player, enemy and loot readability, wall texture response and GPU cost require
a host capture. A NullRHI pass cannot establish any of those results.

### Observed numeric results and validation limits

The regenerated B1 has **51 fixtures**, including mounted sconces and broad
braziers, down from 59, plus the existing single pawn fill. B1 totals **262 pieces,
1,217,374 submitted triangles, 481 base sections and zero blockers**. B2 remains
833 pieces / 222,048 triangles / 1,662 base sections / zero blockers. The
basement generator source, area registry and reviewed-arrival file are
byte-identical to the base revision; placement changes are confined to the
B1 dressing branch and visual recipes.

The first focused numeric run detected a sconce moved inside the entry stair
side wall with its flame only 4 cm above the ramp. Generation now drops mounted
seeds with less than 70 cm clearance over actual ground. The subsequent loaded
game-world run observed these nominal direct-lux results:

| Receiver/sample group | Predicted illuminance or fraction |
|---|---:|
| Entry floor mean | 8.2700 lux |
| Hub floor mean | 9.9322 lux |
| Healer floor mean | 7.0971 lux |
| East floor mean | 5.9341 lux |
| All floor samples mean | 7.4087 lux |
| Floor directly beneath fixtures, mean | 72.7014 lux |
| Wall directly behind sconces, mean | 1221.8683 lux |
| Warm-light gaps, mean | .3504 lux |
| Occupied gap with nominal cool-fill energy, mean | 3.8605 lux |
| Occupied gap with approximate fill-colour weighting | 2.7758 lux |
| Lit pool sample fraction (>=8 lux) | .255 |
| Gap sample fraction (<1.5 lux) | .428 |
| Restored broad profile mean / gap fraction | 17.2078 lux / .028 |

The four fixed grids contain 1,552 candidates, of which **1,545** hit walkable
surfaces and **7** are rejected for penetrating/non-walkable hits. The player
CDO capsule half-height is 90 cm; adding the runtime fill's 220 cm offset
puts it 310 cm above the floor in the estimate. This deliberately corrects an
earlier draft's incorrect 220 cm floor-relative assumption. Actual pawn
placement may include a small arrival clearance. The coloured estimate uses
linear Rec.709 weighting of the fill tint; torch temperature, finite source
integration, occlusion, materials, indirect light, fog and exposure remain
outside the estimate. High wall-probe lux close to the finite light is
especially approximate. It is not a claim of visible wall brightness.

The audit requires at least 25% gaps overall and 15% in each room, at least
15% lit pools overall, and a failed gap criterion for the restored broad
profile. It checks all 51 fixtures after serialized game loading and BeginPlay.
Masonry recesses, cut-cap positions, broad brazier bases and emissive flame
geometry are checked in that initialized world, where floor traces can work.
The intermediate separate masonry test incorrectly used an uninitialized
loaded editor world and reported 30 missing supports; it was moved into the
existing initialized-game audit before delivery. No runtime stand fallback was
added to conceal that test-harness failure.

The supporting `library/direct-lux-diagram.svg` visualizes the same 51 light
positions over flat z=0 planes. It is a numerical diagram, **not a Blender or
Unreal rendering**, and excludes stair heights and occlusion. The audit table
uses actual traced surfaces and is the numeric evidence to compare.

### Delivery checks and remaining integration work

Observed environment: uid1000, UE **5.8.3 Linux**, changelist 58210709. Both
builds used `-DisableAdaptiveUnity -UBASharedMemoryTempFile=true -NoUBA`,
checkout-local XDG config, UBA directory and DDC. The generated unity test
translation unit includes both `LHB1LightingTests.cpp` and
`LHLightingAuditTests.cpp`. UBA reported unsuccessful cache-result stores;
target compilation/linking nevertheless reported `Result: Succeeded`.

| Check | Observed result | Wall time |
|---|---|---:|
| Final non-adaptive unity LighthavenEditor | Succeeded, exit0; 54.38 s engine time | 55 s |
| Non-adaptive unity Lighthaven | Succeeded, exit0; 114.89 s engine time | 115 s |
| Final local `LHGenerateBasementAMaps` | B1/B2 generated, zero errors, exit0 | 22 s |
| Full headless `Lighthaven` | **213 discovered / 211 Success / 2 Fail / 0 not-run or in-process**, exit255 | 250 s |
| `LHValidateWorld` | Identity/portal/entrance validation passed for five maps, zero errors, exit0 | 22 s |

The full suite reports **203 clean successes and 8 successes with warnings**,
224.687 seconds of test duration. Both `Lighthaven.World.LightingAudit` and
`Lighthaven.World.B1LoadedGameLightingAudit` are present and Success. All five
map dressing checks and the framing checks pass. These are geometry/behavior
checks, not image acceptance. `Lighthaven.Integration.G4.SessionInventoryRollback`
passed in this run; this change does not claim to fix its previously reported
intermittent failure.

**The suite is not green.** The two observed failure names and causes are:

- `Lighthaven.Visual.B1TorchLighting`: its Sconce and Torch assertions still
  expect 2000 K; actual is the deliberate 2900 K profile. Integrator follow-up
  is in `Source/LighthavenTests/Visual/LHB1LightingTests.cpp`, line 31.
- `Lighthaven.Visual.CatalogBuildCollisionBudgets`: “Roughness exists in engine
  parent” assumes all procedural surfaces use BasicShapeMaterial. The new
  B1 brazier flame deliberately uses the existing emissive imported material,
  whose roughness comes from ORM. Its one parent-parameter assertion fails;
  geometry/budgets, recipe colour/roughness MID reads and collision assertions
  pass. Integrator follow-up is in
  `Source/LighthavenTests/Visual/LHVisualKitTests.cpp`, line 58: distinguish the
  imported emissive flame material from the engine primitive-material path.

Both files are outside this task's write scope and remain unchanged. Their
expectations need integrator review/update before a green suite can be claimed.
The owned game-world audit covers the new temperature, imported flame parent,
flame topology/bounds and zero collision/nav behavior. No tests were disabled
or failures reclassified as successes.

Reproduction wrappers and trimmed evidence are in this attempt's `library/`.
From this checkout, the saved wrappers run:

```sh
bash Saved/W509c/build-target.sh LighthavenEditor
bash Saved/W509c/build-target.sh Lighthaven
bash Saved/W509c/run-editor.sh generate-final -run=LHGenerateBasementAMaps
bash Saved/W509c/run-editor.sh full-final \
  '-ExecCmds=Automation RunTests Lighthaven; Quit' \
  "-ReportExportPath=$PWD/Saved/W509c/FullReport" '-TestExit=Automation Test Queue Empty'
bash Saved/W509c/run-editor.sh validate-final -run=LHValidateWorld
```

The complete wrappers preserve the exact local environment and common flags.
`library/full-suite-summary.json` contains all observed test names, states,
errors and warnings; `full-suite-lux.txt` contains the numeric receipts.
The intermediate 1-test red run and 18-test focused run are retained as
trimmed evidence, superseded by the final 213-test inventory. No full automation
index or raw editor log is copied into worker output.

Checks not run and concrete missing prerequisite: fresh packaged Unreal
screenshots, pixel-luma/near-black measurements and GPU frame timing need the
coordinator's render-capable host session and rebuilt package. No display
variables were available in this worker. No Blender mock or cook/package run
was performed; the SVG is only a numerical diagram. Windows checks remain
deferred under Linux-first policy. NullRHI does not exercise the render-state
assertion guarded by `FApp::CanEverRender()`.

Known limitations/assumptions: `.19/.18/.20` capture means remain unobserved;
zero-shadow light can pass through walls; cool-fill visibility of actual
characters/loot and wall bevel/material response require rendered inspection.
The remaining braziers retain a regular seed pattern. Their new silhouette,
small cut-cap strips, arch recesses and near-flame highlights need host review.
This is a submitted evidence candidate, not a G5 or verified-task declaration.

Next task and integration notes: review this source candidate and the two
out-of-scope test expectations, then **regenerate B1/B2** before packaging.
Regeneration is required for mounted/removed fixtures; loading an old map only
updates lighting/support geometry on its old recipes. Keep the existing
imported B1 assets; no new art import or binary asset is supplied. Capture the
same three views, compare the measured means to `.15–.25`, and assess the
concept's pools, neutral shadows, readable stone and actor/loot visibility.
No push or merge is part of this attempt.

Standalone arrival review (`env UE_ROOT=/home/brewerm/Downloads/unreal
XDG_CONFIG_HOME="$PWD/Saved/BuildEnvironment/config"
'UE-LocalDataCachePath'="$PWD/Saved/DerivedDataCache" bash build/review-arrivals.sh`)
observed **9/9 PASS**, exit0, **76 seconds wall time**. Its installed DDC graph
logged an unavailable writable-node error and explicitly fell back to the
memory cache; the automation test completed Success and all nine receipts
were written. This startup fallback is retained in the trimmed log rather
than described as an error-free engine launch.

`Config/Lighthaven/ReviewedArrivals.tsv` remains byte-identical, SHA-256
`e56cc528fcbdee98194c878edf84347d5a71f49eb6c23cf927e47634a724e5fc`.
All **34** locally hydrated/generated binary files (8 maps, 26 art packages)
were restored to their exact original Git LFS pointer bytes. Local hydration
used verified SHA-256/size matches in the existing LFS object store, without
network downloads. `git diff --check` passes. The delivered delta contains
only five owned source/test files and this appended document; no `.umap`,
`.uasset` or arrival-review changes. Evidence output is below 1 MiB, under the
40 MiB limit.
