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
