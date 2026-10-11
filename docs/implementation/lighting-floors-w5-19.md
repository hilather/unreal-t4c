# W5-19 — chapel and basement lighting

Task ID: W5-19. Contract revision: 1. Base revision: `ad5fb14354f77280a387086fea2233e53fe19f2a`. Source result revision: `5629019fd6c1571142b5266dde110890f6f963fc`; the subsequent documentation commit is the submitted candidate identified by its receipt. Source and validation details below are an evidence candidate, not visual acceptance.

Owned paths / binary assets: `Source/Lighthaven/Visual/LHFloorLighting.*`, lighting lifecycle hooks in `LHVisualKit.cpp`, lighting-only generator calls and `Commandlets/LHFloorLightingAuthoring.h`, lighting tests under `Source/LighthavenTests/{Visual,World}/`, this report and `lighting.md`. No binary assets delivered. Player, Monsters, Npc, gameplay, collision geometry, arrival transforms, materials and B1 lighting tuning are unchanged.

## Evidence and visual decisions

Source-backed mechanics: none changed. Every new lighting value is **Prototype presentation tuning**, provenance W5-19 visual comparison against the provided host captures and W5-14 Blender mock-ups, retrieved locally 2026-10-11; no mechanics authenticity or rendered validation is implied.

Inputs reviewed:

- Real packaged session: `/home/brewerm/.herdr-projects/unreal-t4c/scratch/captures/w5-17c-18c/rooms/`.
- Mood reference: `/home/brewerm/.herdr-projects/unreal-t4c/scratch/captures/w5-14-blender/{Church,B2Damp,B3Crypt,B4Ritual}_room_final.png`.
- B1 implementation and history: `Visual/LHB1Lighting.*`, `lighting-b1-w5-09.md`.

The chapel's floor is sunlit while its high walls and NPC faces are dark. B2 has neutral wet highlights without much separation between torch pools. B3's pale floor receives too much broad light. B4's dark material needs both more exposure and red-amber illumination; its existing sigil texture is barely visible. B1 provides the reference for small warm pools, restrained ambient light and readable gaps.

Baseline metrics were recomputed from the supplied PNGs using full-frame exported sRGB Rec.709 luma `(.2126R+.7152G+.0722B)/255`; near-black means strictly below `.02`. These are **existing capture measurements**, not screenshots of this change. No cropping, gamma conversion or edited image was used.

| Floor | Baseline means, rooms 1 / 2 / 3 | Baseline near-black shares, rooms 1 / 2 / 3 | Predicted new means, rooms 1 / 2 / 3 | Requested mean band |
|---|---|---|---|---|
| Church | .2556 / .3163 / .3176 | .1759 / .1190 / .1113 | .30 / .29 / .29 | .25–.35 |
| B1 reference, unchanged | .2373 / .2245 / .2056 | .0847 / .0437 / .0432 | unchanged | reference |
| B2 damp | .2322 / .2494 / .2523 | .1160 / .0084 / .0212 | .19 / .21 / .21 | .15–.25 |
| B3 crypt | .2714 / .2909 / .3698 | .1892 / .1912 / .1202 | .18 / .20 / .23 | .15–.25 |
| B4 ritual | .0634 / .0464 / .0443 | .2527 / .2428 / .1644 | .17 / .15 / .15 | .12–.22 |

Predictions carry **at least ±.05 uncertainty** and are visual tuning hypotheses, not a lux-to-sRGB conversion. The unchanged camera includes intentional exterior void. A lower near-black share is the goal on every view, including B2 room 2's already small .0084 share; this cannot be established by NullRHI. No threshold is relaxed and no predicted mean is reported as an observed pass.

## Behavior changed

`LHFloorLighting::FindProfile` selects four separate styles. It returns null for B1; `LHB1Lighting.*`, its existing fixture path, room fill coordinates, exposure, sky, fog and grade remain untouched.

`ALHVisualPiece::Build` binds a separate transient `ULHFloorFixtureLightComponent` to non-B1 sconces/torches, plus church and ritual altars. The light is at the visible flame's local `(0,25,14)` position. Rebuilding reuses the component, resets its base profile, and removes it if the recipe no longer emits. Registration waits until the world is initialized, preserving the B1 runtime fix. Game-world receipts log creation, PostLoad and BeginPlay. Fixture flicker uses the existing deterministic game-time ±6% function, seeded by position; altar keys are steady. No fixture light casts shadows or affects navigation.

The generator invokes `LHFloorLightingAuthoring::Apply` **after dressing**. Old light transforms still produce the identical fixture and geometry recipes. It then removes old landmark point/spot actors, raises existing basement coverage lights into soft room fills, configures sky/post-process, and adds one atmosphere actor. There is no placement, collision or gameplay change. The chapel receives ten warm room fills plus four cooler exterior fills; they cover the chapel, service buildings and exterior approaches. They are fixed world lights, never attached to NPCs. Sunlight becomes a restrained cool 12 lux at EV4 instead of 3000 lux at EV10, allowing wall sconces and bounce to contribute. The pre-existing directional sun retains its shadows; **all fixture, altar, room and player fill lights are shadowless**.

| Prototype profile | Church | B2 damp | B3 crypt | B4 ritual |
|---|---:|---:|---:|---:|
| Manual EV100 | 4 | 2.5 | 4 | 1.5 |
| Sky intensity | .20 | .16 | .12 | .20 |
| Sky linear RGB | .65,.75,1 | .56,.76,.72 | .62,.72,1 | .62,.67,.83 |
| Fixture lumens / radius cm / kelvin | 2400 / 500 / 2800 | 1000 / 400 / 2900 | 900 / 375 / 3100 | 1500 / 450 / 2400 |
| Room fill lumens / radius cm / height cm | 20000 / 1800 / 650 | 6500 / 1500 / 800 | 4500 / 1450 / 800 | 14000 / 1500 / 750 |
| Room fill linear RGB | 1,.78,.52 | .55,.76,.72 | .63,.74,1 | .82,.49,.32 |
| Player fill lumens, radius 750 cm | 900 | 450 | 600 | 650 |
| Player fill linear RGB | 1,.82,.62 | .68,.85,.82 | .76,.83,1 | .9,.7,.52 |
| Haze max opacity | .10 | .12 | .12 | .15 |
| Haze linear radiance | .18,.145,.10 | .08,.14,.13 | .28,.36,.50 | .16,.095,.065 |

All local lights use movable inverse-square lumen units. Diffuse room fills have specular scale zero so B2's floor does not gain a new grid of wet hot spots. The shorter fixture falloff retains localized warm highlights. Church exterior fill tint is `.67,.81,1`. Church altar key is 4000 lm / 700 cm, B4 altar is 3000 lm / 650 cm with red-amber linear `(1,.3,.1)`; both sit 180 cm above the altar actor pivot. B4's sigils keep their existing material and gain incident light; they have not been made emissive.

`ALHFloorAtmosphere` reapplies the saved style on construction/load/play. Its player-area light follows the currently possessed pawn at +220 cm Z, hides when unpossessed and follows a replacement pawn after respawn; the fog stays fixed. Ambient room fills illuminate unoccupied areas independently of the player.

Post-process fixes physical exposure at ISO100, f/2, reciprocal shutter `2^EV/4`, bias0 and matching bounds. All four styles explicitly disable Lumen GI/reflections and motion blur, use world-space AO radius100 (.55 church/B4, .65 B2/B3), bloom .18/threshold1, saturation .93, vignette .16 and small per-channel highlight/shadow gains. Nonvolumetric haze density .008, height falloff .25 and the opacity caps above soften cutaway black without volumetric cost. B3 uses higher haze radiance to approximately compensate for its 1.5-stop exposure difference from the B1 reference; the .12 opacity cap is retained. This aims to reduce black cutaway void while the lower direct fill darkens stone. No engine, plugin, texture or material was installed or generated.

## Numeric audit

`Lighthaven.World.FloorLoadedGameLightingAudit` loads each serialized non-B1 map into a fresh Game world. It checks that fixture reconstruction occurs before initialization without premature registration, then checks initialized registration, world ownership, visibility, shadows and render state when a render-capable RHI exists. Under NullRHI, render states and pixels cannot be verified.

The direct-light estimate uses the existing B1 audit's UE5.8 lumen conversion and attenuation formula:

`E = lm * 10000/(4*pi) * max(0,1-(d²/R²)²)²/(d²+1) * max(0,N·L)`

Distances are centimetres. The new audit additionally applies the light's linear RGB luminance. Temperature, finite-source integration, materials, sky, fog, exposure and tonemapping remain excluded. It is a physical delivery regression, not image analysis.

Fixed 100 cm room grids are independent of fixture placement; floor traces reject blocked or missing receivers. A warm-light gap is below .25 direct fixture lux and a pool is at least 1 lux. The audit reports floor means/minima, gap/pool means and shares, real floor receivers below fixtures, vertical sconce receivers, chapel NPC-facing probes/tall wall and B4 altar/nearby ritual-floor probes. The grid must contain both gaps and pools; unoccupied gaps must receive average direct fill of at least .25 lux. These are conservative structural bounds, not visual acceptance thresholds. Existing B1 loaded-game photometry and visual tests remain in place.

The focused NullRHI run completed all five selected tests successfully (warnings present, no failures), including the unchanged B1 loaded-game audit. The final full-suite run reproduced the direct-light results below and passed every lighting test. Values exclude player fill and are **calculated from actual loaded components and floor receivers**, not rendered means.

| Floor | Fixture keys / room fills | Valid fixed-grid samples | Mean / minimum direct lux | Gap / pool mean direct lux | Warm gap / pool share |
|---|---:|---:|---:|---:|---:|
| Church | 21 / 14 | 250 | 20.484 / 6.831 | 17.621 / 26.565 | .604 / .312 |
| B2 | 112 / 95 | 1490 | 11.292 / 4.289 | 9.874 / 14.136 | .551 / .328 |
| B3 | 108 / 91 | 582 | 10.271 / 3.885 | 8.673 / 12.539 | .491 / .342 |
| B4 | 87 / 74 | 890 | 23.954 / 8.314 | 20.205 / 25.821 | .224 / .669 |

Each floor additionally has one player fill and one skylight; Church alone retains one directional sun. Fixture totals include altar keys. Per-fixture traced floor-pool means are 36.124 / 15.989 / 10.856 / 21.739 lux, respectively. The vertical sconce probes 25.5 cm behind the flame report 2939.518 / 1290.809 / 1163.108 / 1972.838 lux; these intentionally near-source probes only detect missing/misplaced wall illumination and cannot predict a wall's pixel value. B4's fixed altar-top probe receives 313.935 lux, with nearby ritual-floor mean 26.418 lux. All 13 authored hub NPC face probes passed individually: 2.734–20.312 direct lux (minimum JagarKar); the fixed tall north-wall probe measured 122.604 lux. These estimates omit visibility occlusion because the contributing fixture/fill lights intentionally cast no shadows. The complete per-NPC and per-room output is in `library/floor-numeric-audit.txt`.


## Handoff and integration

Provisional tuning introduced: all four profiles and the predicted image means above; source URL null, replacement through measured host captures.

Build/editor/cook/package/play checks actually run: UE5.8.3 on Linux, observed uid1000, non-adaptive unity via the farm XML configuration, `-DisableAdaptiveUnity -UBASharedMemoryTempFile=true -NoUBA`, checkout-local XDG configuration/DDC/UBA. Both final build targets report `Result: Succeeded`.

| Final check | Observed result | Exit | Wall time |
|---|---|---:|---:|
| LighthavenEditor Linux Development | Succeeded | 0 | 92.599 s |
| Lighthaven Linux Development | Succeeded | 0 | 85.316 s |
| LHGenerateHubMap | Church generated | 0 | 28.073 s |
| LHGenerateBasementAMaps | B1 regenerated; B2 semantic hash unchanged, previously generated map bytes preserved | 0 | 26.144 s |
| LHGenerateBasementBMaps | B3 and B4 regenerated | 0 | 67.176 s |
| Full `Lighthaven` automation | **238 expected/observed; 237 Success with warnings, 1 Fail, 0 not-run/in-process** | 255 | 306.822 s |
| Standalone ArrivalSafety | 1 Success with warnings; **9/9 arrivals PASS**, all hashes equal committed review | 0 | 39.767 s |
| LHValidateWorld | **Five maps passed, 0 errors**, 18 warnings | 0 | 27.608 s |

Full-suite test duration was 266.611 s. The one failed test is **`Lighthaven.Visual.Monsters.CreatureImportedAssets`**. Its eleven errors are missing local `artsource/creatures/output/<kind>-validation.json` files for rat, bat, slime, goblin, giant_spider, balork, goblin_warrior, atrocity, dungeon_bat, giant_bat and undead_bat. The test reads source rest-bounds evidence at `LHCreatureArtTests.cpp:90`; the imported mesh/material/animation checks did not report failures. This missing generated evidence is outside the lighting lane. It was not fabricated, bypassed or changed, and the full suite is **not** claimed green. Coordinator must supply/reproduce the matching creature export validation and rerun this test. `Lighthaven.Integration.G4.SessionInventoryRollback` passed in this run.

All new floor tests, the existing B1 lighting tests, the five dressing tests, and ArrivalSafety passed within the full run. The earlier focused suite had 5/5 Success with warnings, exit0, 16.029 s test / 88.247 s wall. The initial editor build failed with one newly introduced test iterator typing error (`auto*` over `TObjectPtr<AActor>`), corrected to explicit `AActor*`; later editor/game builds above passed. The first generation pass created all five playable maps, exit0 for each commandlet (109.056 / 46.722 / 43.419 s); final B2 semantic digest was `02d44d16bc28835dbdd761b69f1a8a74`. Repeated Wayland-before-SDL and read-only user-config warnings explain warning-bearing headless results; none is hidden as a clean result.

Reproduction: `library/commands.jsonl` contains every exact command and final exit/time; `library/reproduce-checks.py` records the orchestration. Build configuration is the XML in `wave4/_env.md`, saved to both specified checkout-local locations. Environment: `UE_ROOT=/home/brewerm/Downloads/unreal`, `XDG_CONFIG_HOME=$PWD/Saved/BuildEnvironment/config`, `UBA_ROOT=$PWD/Saved/UBA`, `DOTNET_CLI_HOME=$PWD/Saved/BuildEnvironment/dotnet`. Headless engine calls use the farm Local DDC recipe and `-nullrhi -unattended -nosound -nop4 -NoCrashDialog`, the documented analytics/crash-report/HomeScreen ini switches, plus `-run=LHGenerateHubMap`, `-run=LHGenerateBasementAMaps`, `-run=LHGenerateBasementBMaps` or `-run=LHValidateWorld`. Automation uses `-ExecCmds='Automation RunTests Lighthaven; Quit'` (or the arrival/focused prefix), `-ReportExportPath=...` and `-TestExit='Automation Test Queue Empty'`.

All **442** hydrated/generated LFS assets, including all eight map pointers, and the B2 receipt were restored at **2026-10-11 19:07:19 UTC**, approximately 37.5 minutes into this attempt, leaving over 82 minutes of the 2-hour budget. `Content/` has no diff. `Config/Lighthaven/ReviewedArrivals.tsv` remained unchanged, SHA-256 `e56cc528fcbdee98194c878edf84347d5a71f49eb6c23cf927e47634a724e5fc`. The B1 loaded-audit body is byte-identical to base, `LHB1Lighting.*` has no diff, and `git diff --check` passed. Only owned source/tests/docs are submitted.

Results and evidence paths: ignored checkout-local `Saved/W519/`; canonical attempt output `report.md` and trimmed `library/`. The baseline metric table is `baseline-luminance.tsv`. Local assets were hydrated from 442 SHA-256-verified objects already in the Git LFS store; no network download.

Checks not run and concrete missing prerequisite: new rendered Unreal screenshots, packaged visual review and GPU timing require the coordinator's host display/package run. This worker cannot render Unreal. No new Blender mock-up was necessary; existing references were inspected. Windows packaging/launch are deferred under Linux-first policy. NullRHI does not establish actor/loot readability or a near-black pixel reduction.

Known defects or remaining decisions: the full suite has the single missing creature-validation evidence failure described above. The mean predictions and all-view near-black reduction are unverified; exterior void may dominate the latter. Short unshadowed lights can leak through walls. Added fixture lights increase the component count, with limited radii and no fixture shadows; target-GPU cost remains unmeasured. The existing freestanding non-B1 sconce geometry and its grid positions are retained. The red sigils remain painted texture details, so their final readability needs the host review.

Next task and integration notes: coordinator reviews the source candidate, regenerates all five playable maps with these commandlets, runs arrival review and `LHValidateWorld`, and commits its own generated assets. Package Linux and capture the same three room views per floor with the established camera. Compare the full-frame metrics above, inspect player/enemy/NPC/loot readability, tall chapel walls and B4 altar/sigils; keep B1 as the unchanged control. Adjust only profile tuning in a follow-up if captures miss the requested bands. No push or merge is performed by this worker.
