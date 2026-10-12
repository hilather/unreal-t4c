# W5-22c room polish

Task ID: W5-22c. Contract revision: 1. Base revision: `6c5b276e9211e6261e7adb2f767d206530bb8af5`. Result source/test revision: `d4ff37165f0a1ae4ecf29c950637ca3fa35f9522`; the following report-only submission commit is identified by `candidate_oid` in the submission receipt. This report is an evidence candidate, not rendered acceptance or an integration decision.

Owned paths / binary assets: lighting and dressing source under `Source/Lighthaven/Visual/` and `Source/LighthavenEditor/Commandlets/`, corresponding lighting/dressing tests under `Source/LighthavenTests/{World,Visual}/`, `lighting.md` and this report. No binary assets delivered.

## Visual evidence and assumptions

Inputs are the real W5-19 packaged-session captures at `/home/brewerm/.herdr-projects/unreal-t4c/scratch/captures/w5-19/rooms/`, inspected locally on 2026-10-12 UTC. Full-frame exported sRGB Rec.709 luma is `(.2126R+.7152G+.0722B)/255`; near-black is strictly below `.02`. Images are neither cropped nor gamma-linearized. `library/baseline-luminance.tsv` records the reproduced measurements. These are earlier host captures, not screenshots of this change.

B4 has broad amber illumination across the basalt floor, reducing the separation between its small warm fixture pools. B3's floor is largely readable but its cutaway void contributes much of the near-black area. A point light cannot illuminate absent geometry. The B3 pass therefore separates floor coverage from distant background haze. B1's storage props are sparse around the walls. The B1 arch recipe uses a back-edge pivot, while its source lintel and adjoining walls are centered in thickness.

Source-backed mechanics: none changed. All new numbers are **Prototype presentation tuning**, based on the supplied images and the existing kit. There is no mechanics authenticity claim. No engine, plugin, asset pack or texture was downloaded. Bones, chains, broken pillars and hedges are absent from the imported catalog; they are not fabricated in C++.

## B4 and B3 lighting

| Setting | B4 prior → candidate | B3 prior → candidate |
|---|---|---|
| Manual EV100 | 1.5 → 2.25 | 4 unchanged |
| Sky intensity | .20 → .10 | .12 unchanged |
| Fixture lm / radius cm / K | 1500 / 450 / 2400 → 1800 / 375 / 2300 | 900 / 375 / 3100 unchanged |
| Diffuse room fill lm / radius cm / Z cm | 14000 / 1500 / 750 → 6500 / 1400 / 750 | 4500 / 1450 / 800 → 3500 / 1600 / 750 |
| Room fill linear RGB | .82,.49,.32 → .62,.44,.38 | .63,.74,1 unchanged |
| Player fill lm / radius cm | 650 / 750 unchanged | 600 / 750 unchanged |
| AO intensity | .55 unchanged | .65 → .50 |
| Shadow grade RGB | 1.02,.99,.98 → .88,.92,1.02 | .99,1,1.03 unchanged |
| Haze linear radiance | .16,.095,.065 unchanged | .28,.36,.50 → 1.4,1.8,2.5 |
| Haze start distance / opacity | 0 / .15 unchanged | 0 → 4000 cm / .12 unchanged |

Raising B4 EV by .75 reduces exposure. Narrower fixtures retain concentrated warm keys; the altar keeps its red-amber 3000lm key but contracts from 650 to 500cm radius. The existing floor sigils and basalt materials remain; this tuning changes their incident light, not their textures. Player-area fills keep their original intensity, radius and possession/respawn lifecycle. No fixture or fill casts shadows, no light affects navigation, and Lumen GI/reflections remain disabled.

B3 broadens cold diffuse coverage while reducing energy and moving the fill down 50cm. AO is reduced modestly to lift recessed stone gaps. Stronger cold nonvolumetric haze begins at 4000cm, beyond the usual 900–1800cm gameplay camera boom, rather than adding a global black-level offset. The distant-background effect is an inference from installed UE5.8 `HeightFogCommon.ush` (depth integration and capped transmittance) and `HeightFogPixelShader.usf` (exposure); it is not an observed rendered result. Haze start distance explicitly resets to zero when switching from B3 to another style.

## Predicted captures

Predictions are tuning hypotheses, with uncertainty of at least ±.04 mean and ±.05 near-black share. They are not derived by converting direct lux to sRGB. Only same-camera host recaptures can establish the requested B4 .12–.22 mean and B3 well-under-.10 near-black / at-most-.22 mean.

| Floor / rooms 1, 2, 3 | Observed prior mean | Observed prior near-black | Predicted mean | Predicted near-black |
|---|---|---|---|---|
| Church | .230 / .314 / .198 | .038 / .014 / .040 | .25 / .32 / .22 | .025 / .010 / .025 |
| B1 | .237 / .226 / .206 | .086 / .043 / .043 | .24 / .23 / .21 | .085 / .043 / .043 |
| B2 unchanged | .184 / .201 / .178 | .131 / .016 / .035 | .184 / .201 / .178 | .131 / .016 / .035 |
| B3 | .168 / .185 / .191 | .239 / .218 / .136 | .19 / .21 / .21 | .06 / .05 / .04 |
| B4 | .324 / .275 / .296 | .132 / .136 / .047 | .18 / .15 / .16 | .15 / .16 / .07 |

B4 deliberately retains darker gaps and distant void; a near-black decrease is not its acceptance target. B3 haze must be reviewed for unwanted visible separation at long camera depths and for the intended reduction in empty-background black.

## Numeric audit

`Lighthaven.World.FloorLoadedGameLightingAudit` reconstructs actual fixture components in fresh loaded Game worlds. Fixed room grids trace real floor receivers. The extension reports per-room and aggregate current/prior mean, minimum, tenth percentile, gap/pool means, contrast, perimeter mean, and relative exposure-weighted `lux * 2^-EV`. Prior W5-19 component values are evaluated at identical light XY positions and receivers. Pool/gap membership stays tied to the prior fixture field so changing a radius cannot improve the comparison merely by reclassifying receivers. The audit excludes temperature, finite-source integration, material response, sky, fog, AO and tone mapping; exposure weighting is a comparative proxy, never pixel luminance. B1's existing numeric audit is retained.

B4/B3 tuning was built in both targets, installed into all five maps, and checked before the lower-priority dressing work began. That first focused run completed **3/3 tests successfully with warnings**, exit 0. It loaded generated maps with 108 B3 fixtures / 91 room fills and 87 B4 fixtures / 74 room fills; no new light actors were added. Values below are calculated direct illuminance from actual loaded components, not rendered image metrics.

| Floor | Samples | Mean lux prior → candidate | Minimum lux | 10th percentile lux | Pool/gap contrast | Exposure-weighted mean |
|---|---:|---|---|---|---|---|
| B3 | 582 | 10.2709 → 11.0111 | 3.8854 → 4.4494 | 6.4732 → 7.0231 | 1.446 → 1.417 | .64193 → .68819 |
| B4 | 890 | 23.9544 → 9.3080 | 8.3135 → 2.8196 | 16.1551 → 5.2952 | 1.278 → 1.496 | 8.46917 → 1.95677 |

B3's mean increases 7.2%, minimum 14.5%, and lower decile 8.5% without changing exposure. B4's exposure-weighted mean decreases 76.9% while its local pool/gap ratio increases. The unchanged church/B2 grid means remain 20.4842 / 11.2922 lux. All individual room grids and altar/NPC receivers are recorded in `library/lighting-focus-audit.txt`. These four B3/B4 numeric grids are independently authored receiver regions, not masks for the three camera images.

## Dressing

Church surroundings use existing imported Church `Floor400` and `Wall400` meshes. The main chapel/wing region is X[-1800,2400], Y[-1600,3000]; the mage-compound region is X[-5200,-1200], Y[5600,8800]. Paving top is Z=-24, above existing water at -45 and below the playable flat floor. Rectangle subtraction leaves every original floor/ramp footprint open with an 8cm margin; the descending ramp receives additional southwest camera clearance so the apron cannot cover its lower floor in projection. Existing floor colliders, water, walls, services and routes are unchanged. Low 80cm perimeter walls are admitted only outside playable footprints and camera shadow envelopes. The existing nonvolumetric church haze remains.

The generated candidate has **64 apron assemblies and 14 outline assemblies**, adding **313,676 imported/support triangles** and 106 sections. Whole-church totals are 441 pieces, 1,768,108 triangles and 1,010 sections, below the existing 2M/12k caps. This is a bounded geometry cost, not measured GPU performance. Side and foreground outlines supply a visible setting while retaining the chapel's required cutaway. A hedge is not available in the kit.

B1 dressing uses the existing rubble mesh at previously rejected fixture-adjacent storage sites. Original seed support, broad-room depth, padded footprints, wall-end margins, stairs, 350cm protected-anchor clearance and fixture clearances remain. New candidates additionally undergo imported-triangle camera rays to the existing 50cm walkable grid, including foot/head endpoints. Any candidate that blocks a tested endpoint is rejected; the existing sight ceiling is not relaxed. Seven previously rejected sites were eligible for a low-mesh retry. Quarter turns and, inside former table footprints only, ±25cm tangent shifts yielded three safe additions; four remained rejected. Final B1 clutter is **23 pieces across five existing kinds**, up from the measured 20-piece baseline. This 15% increase is modest partial set dressing; the sparse appearance is not claimed resolved without a new capture.

The E01 room-3 structural arch moves locally +10cm in Y to center its imported [-20,0] thickness on the retained wall/lintel. Its floor Z, clear opening and 120cm foreground cutaway are preserved. The B1 baffle originally used a floor-edge anchor; its corrected actor Y=-980 centers the same mesh on the actual neighboring wall at Y=-990. These are visual actor transforms only; source lintel and gameplay transforms do not move.

## Handoff

Behavior changed: B4/B3 lighting and post-process tuning, B1 low rubble additions and arch seams, and an imported Church ground apron/low perimeter outline. Original gameplay geometry and markers remain unchanged. The generated hub layout TSV is byte-identical to its pre-surroundings version.

Build/editor/cook/package/play checks actually run (Linux Development, NullRHI automation):

| Check | Observed result | Wall seconds |
|---|---|---:|
| Final `LighthavenEditor` target | `Result: Succeeded`, exit 0 | 64.300 |
| Final `Lighthaven` target | `Result: Succeeded` / up to date, exit 0; full runtime compile also passed earlier | 2.896 |
| `LHGenerateHubMap` with surroundings | exit 0 | 41.213 |
| `LHGenerateBasementAMaps` with final B1 poses | exit 0 | 25.764 |
| `LHGenerateBasementBMaps` with final B3/B4 lighting | exit 0 | 21.966 |
| Focused lighting | 3/3 success with warnings, exit 0 | 142.458 |
| Final focused dressing | 17/17 success with warnings, exit 0 | 40.046 |
| Final full `Lighthaven` suite | **241 total: 239 success with warnings, 2 failed, 0 not run/in process**, exit 255 with written report | 620.092 |
| Standalone arrivals | 1/1 automation test; **9/9 entrances PASS**, exit 0 | 198.477 |
| `LHValidateWorld` | **five maps, 0 errors**, 18 warnings, exit 0 | 184.864 |
| Isolated respawn retry | 1/1 success with warnings, exit 0; 16.363s test execution | 57.421 |

Full-suite test execution time is 424.030s; the wall time above includes startup/shutdown. Both final failures are named below; a green full suite is not claimed. Final gameplay camera tests retain blocked-ray counts Church/B1/B2/B3/B4 = **78/81/93/55/99**, with **zero new blocked rays and zero fully hidden points** across all five. Church's separate continuous envelope check also includes apron surfaces, which the generic dressing sight test excludes as floors.

The initial full suite was 241 total / 239 success with warnings / 2 failed (309.450s wall): missing creature receipts and `B1PerimeterClutter` because the first strict retry admitted no new rubble. The latter was corrected by safe pose selection, then passed both the focused and final full suites. An earlier editor build caught a `TObjectPtr`/`auto*` deduction error; using the explicit static-mesh pointer type corrected it before both successful target builds. No failed check is treated as passed.

The required farm XML requests non-adaptive unity and disables UBA executors; command arguments include `-DisableAdaptiveUnity -UBASharedMemoryTempFile=true -NoUBA`. UE5.8 still named its executor wrapper UBA, with `[NoUba]` actions and nonfatal action-store warnings; the target success results above are observed. Both configured XML locations and checkout-local XDG/DDC/UBA paths are used. HOME is not reassigned. Observed uid: 1000. 442 Content assets were hydrated solely from SHA-256-verified local LFS objects for validation; none were fetched from the network.

All **451 tracked Content files**, including **442 LFS pointers** and every `.umap`, were restored at **2026-10-12 02:59:27 UTC**, before the final 20-minute budget window. Each raw Git blob hash matches the base; Content and Config diffs are empty. `git diff --check` passed. Only the nine owned source/test/document files are delivered.

Results and evidence paths: ignored `Saved/W522/`; canonical attempt output `report.md` and trimmed `library/`. `library/commands.jsonl` records exact command arguments, exit codes and timings.

Checks not run and concrete missing prerequisite: new rendered Unreal views, packaged visual review and GPU timing require the coordinator's host package/display session. This worker cannot render Unreal. Cook/package creation was not run in this source-only lane. Windows packaging/launch remain deferred under Linux-first policy. NullRHI does not prove pixel brightness or in-play player/enemy/loot readability.

Known defects or remaining decisions:

- `Lighthaven.Visual.Monsters.CreatureImportedAssets` fails on eleven absent `artsource/creatures/output/*-validation.json` receipts: rat, bat, slime, goblin, giant_spider, balork, goblin_warrior, atrocity, dungeon_bat, giant_bat, undead_bat. This missing local prerequisite is also documented by W5-19. Creature source/output is outside this task's ownership; no receipt was fabricated.
- `Lighthaven.Integration.G4.DeathAndChurchRespawn` failed `session descent` at `LHWave2Tests.cpp:1311` in the final suite after passing in the earlier suite. Its real save completed approximately 14.86s after test start, immediately after failed-world cleanup. The fixture's `Flush` stops waiting after 10s. A pending-save rejection is the likely cause; the exact asynchronous delay is unproven. The isolated retry subsequently passed (1/1, exit 0), without changing source or saved fixtures. This supports an intermittent timing explanation but does not erase the full-suite failure; the coordinator should monitor it.

Remaining visual uncertainty: Image predictions remain unverified, including whether B4 is perceptually the darkest floor and B3 meets the near-black target. The B1 clutter increase is modest. B1 sight rays cover the existing 50cm grid and default 1200cm camera; they do not prove every continuous pawn position, every zoom, or animation silhouette. Church exclusions use conservative continuous floor envelopes at the same camera orientation. Shadowless fixtures can leak through walls; target-GPU light/geometry cost is not measured here. Distant B3 geometry beyond 4000cm receives haze as well as the cutaway void.

Next task and integration notes: coordinator reviews the submitted source, regenerates and commits its own maps, reruns arrival review and LHValidateWorld, and captures the same three rooms per floor in the Linux package. Compare both mean and near-black metrics, inspect player/enemy/loot readability and camera-side dressing. No push, merge, release of capacity or gate acceptance is performed by this worker.

# W5-22d — ritual readability and church exterior

Task ID: W5-22d. Contract revision: 1. Base revision: `15c11efa5809be99a456bcf2d580c29c65c1be38`. Result revision is recorded with the final validation below. This is worker evidence for coordinator review, not rendered acceptance or an integration decision.

Owned paths / binary assets: `LHFloorLighting.cpp/.h`, `LHEnvArtBinding.cpp`, lighting/dressing authoring headers under `Source/LighthavenEditor/Commandlets/`, corresponding World and floor-lighting tests, `lighting.md`, and this appended section. No binary assets are delivered. Gameplay geometry, navigation, registry transforms, spawns and services are not edited. All tuning below is **Prototype presentation**, based on the supplied host captures and existing imported kit; no source-backed mechanics change or authenticity claim.

## Capture calibration

The real packaged-session images at `/home/brewerm/.herdr-projects/unreal-t4c/scratch/captures/{w5-19,w5-22c}/rooms/` were inspected and measured on 2026-10-12 UTC. The existing ImageMagick-backed measurement function computes the full exported sRGB frame, Rec.709 channel weights, near-black strictly below .02; no crop, mask or gamma-linearization. Measurements reproduced the supplied values:

| B4 view | W5-19 mean / near-black | W5-22c mean / near-black | Mean ratio | Fitted exponent p | Proxy gain to mean .15 |
|---|---|---|---|---|---|
| room1 | .324140 / .131713 | .116730 / .246115 | .360122 | .697076 | 1.43297 |
| room2 | .274559 / .136032 | .072440 / .242228 | .263841 | .909408 | 2.22640 |
| room3 | .295933 / .047399 | .073548 / .127754 | .248529 | .950214 | 2.11708 |

The previous audit's exposure-weighted direct-light mean ratio was `1.95677 / 8.46917 = .231046`. Fit `p = log(image_mean_ratio) / log(.231046)` independently for each view; a follow-up hypothesis is `mean_d = mean_c * (proxy_d / proxy_c)^p`. This replaces the unsupported W5-22c prediction: its expected means .18/.15/.16 were actually .1167/.0724/.0735. The correction requires roughly 1.43× entrance energy but 2.1–2.2× deeper-room energy to approach .15, under this limited model.

This is a descriptive calibration to three measured pairs, **not a universal light-to-pixel conversion**. The earlier proxy averaged four independent floor grids, not the image's visible surfaces. There are only two observations per camera, no held-out validation, and exposure/sky/fill/grade changed together. New emissive strokes, distant fog, material response, UI and player framing are not represented by the proxy. Near-black shares cannot be inferred from these means. The .12–.18 mean and <.15 near-black targets remain same-camera host-capture checks.

## Source behavior and visual assumptions

B4 retains the accepted entry's EV2.25, 1800lm/375cm/2300K fixtures, 650lm player fill, AO and grading. A modest cool ambient increase (fill6500→7500lm, brown RGB .62/.44/.38→blue-grey .48/.54/.72, unchanged1400cm radius/750cm height; sky .10→.12) provides a dim floor baseline. The main readability addition is the existing carved inlay: only `MI_B4Ritual_oxide` on floor and altar recipes receives emission through the existing cooked `M_B1` master. Basalt stays nonemissive. Floor emission is linear RGB(3.2,.72,.12), altar(5,1.2,.16); `Flame=1` enables the existing emissive branch. No texture or mesh is fabricated or modified.

Full 400cm floor medallions receive local, diffuse red-amber reflection from +120cm, radius360cm. Entry medallions with world Y<2000 use325lm; deeper halls use650lm. No light is created for a clipped-off medallion. The altar key increases3000→3800lm and500→600cm radius. Every new light is movable, shadowless, non-navigation-affecting; sigils do not tick or flicker. Build/PostLoad/BeginPlay restore material overrides and reuse/remove transient components without changing actor or gameplay transforms. The 2000cm boundary is an authored regional tuning boundary, not an inferred image-space mask.

To address room1/2 cutaway void separately, B4 nonvolumetric haze begins at4000cm, cool radiance(.40,.55,.85), opacity cap .10. Nearby floor and torch receivers are before that start distance. This follows the existing B3 distant-haze mechanism; B3 itself, B2 and church interior profiles remain unchanged. No Lumen or shadow-casting fixture lights are enabled.

Church apron/outline actors retain their exact original bounds, ramp exclusions and 8cm floor margins. The existing Church Floor400 mesh repeats in800cm units instead of400cm; no different outdoor paving mesh exists in the imported catalog. The saved recipe palette selects this exterior fit without changing interior floor recipes. All tagged exterior primitive components receive channel1 only, excluding every existing warm chapel key/fill. A dedicated4lux movable blue moon uses channel1 only and zero indirect intensity; existing interior lights remain unchanged. The exterior master-material blend is .82 in the inner apron to .96 near the edge, with low cool emission (.035,.055,.095)→(.11,.17,.28), retaining some texture and normal relief while fading into blue haze color. This taper is **per assembly**, not a continuous world-position shader; possible seams remain a visual-review limitation. Global Church exposure/haze/sky are untouched. Rebuilding a former exterior piece as interior also resets its channel and emission overrides.

First generated church audit: unchanged64 apron and14 outline assemblies; added triangles313,676→205,676 (−108,000, −34.4%). Whole-hub triangles1,768,108→1,660,108,441 pieces/1,010 sections/zero dressing blockers. The apron/outline cap is tightened400k→240k. This is a CPU geometry count, not GPU timing.

## Loaded lighting evidence

The first focused run completed8/8 successfully with warnings, exit0. It loaded fresh Game worlds, rebuilt actual imported materials/lights, checked components before/after scene initialization, and sampled real traced floor receivers. B4 has87 original fixtures,74 room fills and120 new local sigil lights. The point-source estimate excludes finite source integration, temperature, sky, AO, haze, emission and tone mapping; it is not rendered luminance. The B4 comparator now freezes W5-22c, while B3 retains its earlier W5-19 comparator.

| B4 fixed receiver grid | Samples | Mean lux c→d | Lowest decile c→d | Minimum c→d | Direct-light gain |
|---|---:|---|---|---|---|
| entry X100–1100/Y100–1100 |115|13.4641→18.8435|8.7281→12.8247|5.2328→8.7242|1.3995×|
| west X−2700–−900/Y2500–2900 |95|8.8684→14.4903|4.9950→9.2713|3.4298→6.9165|1.6339×|
| middle X100–2100/Y3500–4100 |147|10.0782→16.7765|5.3288→11.3722|2.9785→4.3522|1.6646×|
| south arena X100–4100/Y5500–6700 |533|8.2772→14.3679|5.2298→9.2778|2.8196→3.8164|1.7359×|
| aggregate |890|9.3080→15.3571|5.2952→9.6933|2.8196→3.8164|1.6499×|

New sigils contribute3.8318lux to the aggregate mean,63.3% of its6.0491lux increase, and4.0008lux at the frozen torch-gap samples. Those gaps rise6.8856→13.0622lux, pools13.1643→18.9644lux; composite pool/gap ratio decreases1.912→1.452 as sigil pools light previously dark gaps. This explicitly does **not** claim the composite field preserves its old contrast; it preserves the individual torch field and adds localized floor-identity pools rather than increasing torch radii or flattening the scene through exposure. Actual perceptual separation needs the capture.

Applying the calibrated exponents to the entry/middle/south-arena gains gives rough direct-light-only mean hypotheses around .147/.115/.124. The second and third are conservative incomplete estimates: emissive inlay pixels and the new distant haze are omitted. The third grid is south of the actual altar camera view and does not include its stronger key. Do not count these as meeting the .12–.18 band; the host must measure all three views. No numerical near-black prediction is claimed from lux. Room1's former .246 near-black share includes distant cutaway void and cannot be fixed by floor lights alone.

Unchanged comparison checks: Church interior grid mean20.4842lux; B2 aggregate11.2922lux; B3 aggregate11.0111lux, all matching the W5-22c audit. B3 profile/exposure/haze source is unchanged. The glow lifecycle tests cover component reuse, clipped-medallion removal, recipe switching, emission restricted to inlay, nonemissive basalt, preserved clip parameters, and no collision/navigation changes.

## Optional B1 infill outcome

No new B1 clutter is delivered. Two bounded trials used existing crates/barrels/benches with the original padded footprint,600cm room-depth support,350cm protected-anchor, stair and fixture clearances, plus the strict imported-triangle camera-to-foot/head rule. The first tried34 wall sites and admitted0;4 member placements across3 sites reached and failed sight. The denser trial swept at100cm along walls, used55cm inset and bench-led groups with a central crate/barrel trial:202 sites/606 member attempts,0 admitted groups,0 admitted members. Member vetoes were183 geometry/occupied footprint,110 protected anchor,257 unsupported footprint/depth strip,36 fixture clearance,20 sight. Those20 sight failures occurred at17 distinct rejected sites, listed in `library/b1-candidate-sites-2.txt`; reason counts are member counts, not mutually exclusive site counts.

The tested tall silhouettes and benches would interrupt existing camera-to-feet/head rays on the walkable lattice. These sampled candidates do not prove that every possible placement is impossible. No successful site was found under the preserved rule, so the failed exploratory infill and its nonzero-admission test were removed completely. `LHMapDressing.h` and `LHMapDressingTests.cpp` retain their exact W5-22c bytes. No sight threshold or collision rule was relaxed to admit clutter; the existing seven small rubble retries remain.


## W5-22d validation and handoff

Task ID: **W5-22d**. Base revision: **15c11efa5809be99a456bcf2d580c29c65c1be38**. Result revision: the candidate commit containing this section, recorded exactly in the canonical attempt `report.md` and submission receipt. Contract revision: **1**. This is worker evidence for coordinator review; rendered acceptance is outstanding.

Owned paths delivered: `Source/Lighthaven/Visual/LHFloorLighting.{h,cpp}`, `Source/Lighthaven/Visual/LHEnvArtBinding.cpp`, `Source/LighthavenEditor/Commandlets/LHChurchSurroundings.h`, `Source/LighthavenEditor/Commandlets/LHFloorLightingAuthoring.h`, `Source/LighthavenTests/Visual/LHFloorLightingTests.cpp`, `Source/LighthavenTests/World/LHChurchSurroundingsTests.cpp`, `Source/LighthavenTests/World/LHLightingAuditTests.cpp`, `docs/implementation/lighting.md`, and this appended section. No binary assets, shared gameplay schemas, collision/nav/spawn definitions or gameplay transforms are delivered. Source-backed mechanics: none changed. All new light, color, haze and paving settings are **Prototype presentation tuning** against the supplied host captures and existing imported kit.

| Check actually run | Observed result | Wall seconds |
|---|---|---:|
| Final `LighthavenEditor Linux Development` | `Result: Succeeded`, exit0 |99.23|
| `Lighthaven Linux Development` with final runtime source | `Result: Succeeded`, exit0 |355.29|
| `LHGenerateHubMap` after final moon isolation | exit0 |48.73|
| `LHGenerateBasementAMaps` after reverting unsuccessful B1 infill | exit0 |118.73|
| `LHGenerateBasementBMaps` with final B4 lighting | exit0 |43.57|
| Focused lighting/exterior suite |8 total:0 clean successes,8 successes with warnings,0 failures,0 not run/in process; exit0 |140.82|
| Final full `Lighthaven` suite |**243 total:0 clean successes,242 successes with warnings,1 failure,0 not run/in process**; exit255 with report |734.86|
| Arrival safety inside the full suite |1 successful automation test; **9/9 entrances PASS**, all hashes unchanged |1.707 test execution|
| `LHValidateWorld` |**five maps,0 errors,18 warnings**, exit0 |102.70|

Full-suite test execution time was652.493s. The243 expected tests are the previous241 plus the two new rebuild regressions. The sole full-suite failure is **`Lighthaven.Visual.Monsters.CreatureImportedAssets`**, with11 errors for absent `artsource/creatures/output/*-validation.json` receipts: rat, bat, slime, goblin, giant_spider, balork, goblin_warrior, atrocity, dungeon_bat, giant_bat, undead_bat. This existing missing local prerequisite is outside task ownership; no receipts were fabricated. `Lighthaven.Integration.G4.DeathAndChurchRespawn` passed this full run. A green full suite is not claimed.

All final lighting/dressing tests passed. Gameplay sight retains Church/B1/B2/B3/B4 blocked-ray counts **78/81/93/55/99**, with **zero new blocked rays and zero fully hidden points** in each map. Church also passes its separate continuous envelope check including the apron, floor/ramp exclusions, channel isolation and zero moon indirect/volumetric/atmospheric contribution. The final triangle/piece/section counts are Church1,660,108/441/1,010; B1 635,476/271/489; B2 1,824,516/833/1,472; B3 1,421,816/405/628; B4 996,928/215/434. All have zero dressing collision blockers.

The first editor build failed on an unavailable directional-light field; replacing it with `SetIndirectLightingIntensity(0)` fixed the compile before subsequent successful builds. The farm recipe used both required XML files, non-adaptive unity, `-DisableAdaptiveUnity -UBASharedMemoryTempFile=true -NoUBA`, and checkout-local XDG/DDC/UBA paths. UE5.8.3 named its executor wrapper UBA while actions reported `[NoUba]`; the final results above are observed. HOME was preserved. Observed uid was1000. Nonfatal environment warnings include unwritable HOME configuration, netlink monitoring and headless Wayland queries. Exact commands, exits and timing are retained in `library/commands.jsonl`.

442 LFS assets were hydrated solely from existing verified local objects for validation, with no download. All **451 tracked Content files**, including every `.umap` and all442 pointers, plus the reviewed-arrival file were restored at **2026-10-12 04:40:31 UTC**. Every raw blob hash matches the base, well before the final20-minute budget window. Content/Config diffs and nonignored untracked files are empty; `git diff --check` passed. Only the ten owned source/test/document files are submitted.

Results and evidence: canonical attempt `attempt-60dce82d541435ad18d46fbf3e093657997ed0b8d6c34ae9ed222b72484bfc5b` output `report.md` and trimmed `library/`, including full-suite counts/failure details, final visual audit, arrival TSV, world validation, calibration inputs and B1 rejected-site logs. Local expanded logs are in ignored `Saved/W522d/`.

Checks not run and concrete missing prerequisite: the worker cannot render Unreal; new screenshots, packaged visual review and GPU timings require the coordinator's host package/display session. Cook/package creation was not run in this source-only lane. Windows packaging/launch remain deferred under Linux-first policy. The120 extra local shadowless lights have no measured GPU cost. Near-black shares, actual B4 means, basalt/torch separation and the church's per-assembly edge-tint seams remain visual-review items. The direct-light-only room2 hypothesis is still below .12 before omitted emission/haze; this is explicitly an unverified tuning candidate, not a target-band claim. Optional B1 additions remain unfulfilled after the bounded strict-sight search.

Next task and integration notes: coordinator reviews this candidate, regenerates and commits the five maps, repeats host validation, then captures the same three B4 and Church room1/3 views. Measure each B4 mean against.12–.18 and near-black<.15, inspect entry character and clear indoor/outdoor separation, and profile local-light cost. Retune only from that observed result if needed; do not infer rendered acceptance from the NullRHI proxy. This worker neither pushes nor merges and does not mark the task externally verified.
