# W5-08c — scripted creature visual refinement candidate

Task ID: **W5-08c**. Contract revision: **1**. Base revision: `a8a579aa0375c18fe0b575dc0f6606d9e0fce25a`.
Result revision: the commit containing this document; exact candidate is recorded in the submission receipt and attempt `report.md`.
Owned paths: `artsource/creatures/`, the removed legacy uppercase creature directory, and this document. No committed binary assets. This report is an evidence candidate, not visual acceptance or a passed project gate.

## Approach and provenance

Moved the entire creature source tree to lowercase and updated its commands and links. Original deterministic geometry and procedural materials now live in [models.py](../../artsource/creatures/models.py), with [baking/rigging/export](../../artsource/creatures/build.py), [exported-file previews](../../artsource/creatures/preview_exports.py), [byte-level GLB validation](../../artsource/creatures/validate_glb.py) and [rebuild instructions](../../artsource/creatures/README.md). Per-creature seeds are 508/509/510. No root ignore file or shared schema/header was changed.

Blender **5.2.2 LTS, d13f752e3b9c**, at `/home/brewerm/Downloads/blender-5.2.2-linux-x64`, runs headlessly with factory startup, CPU Cycles and checkout-local `XDG_CONFIG_HOME`. No installation, download, engine import or system change was performed. Construction is reproducible from Python; pixel-identical rendering across hardware is not promised.

The concept at `/home/brewerm/Downloads/lighthaven-unreal-design-and-agent-waves/lighthaven-plan/assets/concepts/starter-characters-and-creatures-concept.png` was opened for visual comparison on **2026-10-10**. It was never copied, sampled into a texture, or supplied to the geometry scripts. The three [creature specifications](art/creatures/README.md), their rat/bat/slime pages and [visual-monsters.md](visual-monsters.md) supplied presentation constraints. All new geometry, colors, shader scales and animation refinements are **Prototype authored presentation values, W5-08c, 2026-10-10, source_url null**. No gameplay mechanics or source-backed numbers changed.

## Shapes, materials and budgets

| Creature | Actual exported triangles | Joints | Embedded textures | Material slots |
|---|---:|---:|---|---:|
| Brown rat | 7,428 | 11 | base / tangent normal / ORM, each 1024² | 1 |
| Bat | 5,320 | 8 | base / tangent normal / ORM, each 1024² | 1 |
| Green slime | 5,616 | 2 | base / tangent normal / ORM, each 1024² | 1 |

**Rat:** longitudinal body sections taper through the shoulder into a pointed wedge skull; separate haunches, angled slender legs, articulated pink toes and pale claws replace peg feet. Ears are thin cupped surfaces with rolled rims. A three-bone tapering tail has baked ring/scale detail. 1,690 rooted tapered hair cards supplement directional color and normal detail, with a dark underside and warmer back. Blade tangents follow the body slope so the coat continues over the shoulder instead of disappearing into it. These are opaque geometric cards, not a high-poly groom or an alpha-card cloud.

**Bat:** the body is small relative to the wings. Four curved, scalloped membrane panels per side spread from a wrist and have geometric finger spars, an elbow/leading arm and thumb hooks. A wing and wingtip joint on each side blend membrane weights; the digits are **not independently articulated skeletal fingers**. Procedural fine veins and mottling are baked into the membrane. Cupped pointed ears, a short wedge skull, brow ridges, flat nose, small fangs and a fur ruff replace the previous round face. A deliberate 6% resting-span reserve accommodates the flap inside the 80 cm envelope.

**Slime:** an off-center height field combines unequal lobes, streaming folds, a wavy spreading skirt and shallow bubble forms. Darker green cavities, procedural bubble rims and varied roughness suggest inner depth. The broader low shape deliberately uses less than the nominal 40 cm resting-height allowance. It remains an **opaque approximation**: bubble rings are surface shading, not actual suspended refractive inclusions. Neither slime nor membrane exports physical transmission/subsurface scattering. A runtime material pass would be needed for true interior depth and backlighting.

Base color is a diffuse-color-only Cycles bake; tangent normals bake procedural bump on the production geometry. **ORM red is real geometric AO**, evaluated by Cycles' Ambient Occlusion shader against the assembled mesh (6.5 cm distance, 32 AO samples, 8 bake samples); green is material roughness and blue is zero metallic. The standard Blender glTF settings group connects this exact packed image to exported `occlusionTexture` as well as metallic/roughness. Atlas color spaces are sRGB for base and Non-Color for normals/ORM. AO is no longer a neutral constant.

Smart UV padding was reduced after the first atlas gave most space to gaps around tiny fur islands. Final fractional padding is 0.0005, bake dilation 2 px. This preserves far more body detail, but fine islands still require mip/LOD review in Unreal. Materials are double-sided. Animal components intersect rather than forming a watertight sculpt; the slime's main shell is continuous, with separate shallow bubble surfaces.

## Rigs, timing and size evidence

All exports retain **idle 60f/2s, move 24f/0.8s, attack 42f/1.4s, hit 18f/0.6s, death 60f/2s**, at 30 fps. Attack explicitly anticipates through f29, contacts at **f30 = 1s**, and recovers by f42. Keys provide presentation only, never damage events. Idle/move endpoints match; death is held unchanged through f45–f60; floor-root channels remain constant.

World-axis translations/rotations are converted into bone-local coordinates. Rat gains jaw and tail-chain motion; bat gains jaw and distal wing flex. Rat death is authored at every integer frame through f30 with body-level floor correction, and bat smoothly descends to the floor. This moves the rig body, never clamps mesh vertices or changes the floor root. The final death pose is cached for the hold. Linear keys avoid Bezier overshoot. The rat's bite extension and bat's hit turn were restrained after sampled bounds exceeded their boxes.

Final union of all **integer source-action samples** (cm; rounded):

| Creature | X min / max | Y min / max | Z min / max |
|---|---|---|---|
| Rat | −66.19 / 22.20 | −12.78 / 13.86 | 0.10 / 26.02 |
| Bat | −22.18 / 12.98 | −39.96 / 39.96 | 0.10 / 132.06 |
| Slime | −42.56 / 43.87 | −39.67 / 43.07 | 0 / 33.10 |

The script asserts rest and animated envelopes. Assumption: specified sizes are clearance envelopes, not a requirement to fill every dimension. Rat rests at 25.006 cm (0.06 mm above nominal, within the explicit 0.1 mm rest tolerance); bat rests at about 122.6 cm rather than 145 cm, with its torso around 100 cm; slime rests at 29.6 cm rather than 40 cm to satisfy the low-pool direction. Maximum animated limits remain 35/145/60 cm. These undershoots need art-owner review if nominal heights must be exact. No capsules, selection, LOS, navigation, reach or actor scale were changed.

These checks are not continuous swept-clearance certification or full imported-animation bounds checks. Units remain metres in Blender/glTF, +X forward and +Z up; Unreal import must calibrate centimetres.

## Renders and visual feedback

Nine final **1280×720**, **12-sample denoised Cycles CPU** PNGs are rendered from the **re-imported GLBs**, using the shared preview function. Hero views are orthographic three-quarter/front-quarter inspections. Gameplay views retain the 12 m boom, 55° down pitch, 45° yaw, 45° horizontal FOV and 90 cm floor focus. Contact sheets freeze evaluated geometry at idle f15, move f6, attack f30, hit f9 and death f60; they are representative poses, not full-motion playback.

| Creature | Hero | Gameplay | Contact sheet |
|---|---|---|---|
| Rat | `rat_hero.png` | `rat_gameplay.png` | `rat_actions.png` |
| Bat | `bat_hero.png` | `bat_gameplay.png` | `bat_actions.png` |
| Slime | `slime_hero.png` | `slime_gameplay.png` | `slime_actions.png` |

Final images and the two validation JSON summaries belong in the attempt's worker-output `library/`. GLBs and texture atlases remain only in ignored local `output/`; no concept image, GLB, texture or render is committed. There are 18 generated PNGs locally (9 textures + 9 previews), with only the nine previews in worker output.

Feedback iterations addressed UV starvation; buried/sparse shoulder hair; coarse broad fur blades; oversized rat ears/eyes; the bat's spherical face and foreshortened wing presentation; mismatched membrane winding; an overbright lime slime with broad white reflections; and the animation envelope/floor defects described above. The slime now uses smaller studio emitters for tighter wet highlights. Lighting is a studio review setup, not an Unreal dungeon-lighting result.

Honest concept comparison:

- **Rat: moderate correspondence.** Pointed head, low brown body, bare tapering tail, thin ears and clawed feet now read clearly; the coat is continuous and much denser. It remains an angular stylized reconstruction. Limb joints, short head fur and the relatively stiff tail are less natural than the concept's anatomy and tangled groom.
- **Bat: moderate correspondence.** Large scalloped wings, struts, veins, pointed ears and furry central body now dominate the silhouette. The face and membrane remain simplified; it lacks the concept's skin folds, fine branching vein complexity, natural finger folding and convincing transmitted light.
- **Slime: strongest silhouette/material correspondence.** A low uneven pool, folds, dark green variation, wet highlights and bubble cues replace the bell. Surface rings still look painted at close range, and the opaque model lacks the concept's convincing suspended bubbles and refractive depth.

At the gameplay camera, the rat's back/tail, bat's wing outline and slime's pooled footprint/highlight pattern survive. Fine fur, claws, veins and facial detail mostly disappear at that small pixel footprint. These renders do **not** establish the full beautiful/concept-match acceptance bar; coordinator visual review remains necessary.

## Checks, limitations and next task

Actually run: full scripted CPU bakes and GLB exports; Blender re-import skin/action assertions; source-action integer-frame bounds assertions; all nine re-imported-file renders and visual inspection; independent GLB byte validation; Python syntax checks and `git diff --check`. The GLB validator checks container/accessor integrity, triangle budget, finite/unit attributes, normalized skin weights, joint bounds, exactly five named non-static clips and durations, root constancy, loop endpoints and dead hold. It decodes embedded PNGs with CRC/filter checks and samples **occupied triangle UV texels** to prove varied red AO bound to the actual ORM image. PNG filter fixtures and an in-memory corrupted-death negative check also ran.

Exploratory failures: the first custom glTF settings group caused an `Iridescence Factor` KeyError on re-import; using Blender's installed complete settings-group helper fixed it. Two exploratory Blender processes returned exit 143 without a diagnostic cause; later serial final builds/renders completed. Subsequent invocations use `--python-exit-code 1` so Python errors cannot appear as successful shell exits. Blender's socket startup notices, `Material.use_nodes` deprecation warning and shared texture-sampler warning were observed; final export/import completed despite them. The initial constant-key death validator false positive was corrected to evaluate optimized animation channels rather than require redundant keys.

Not run: Unreal import/editor/build/cook/package/play, which require the separate authorized import/runtime task and material/unit calibration; Windows packaging is deferred by project policy. Also unperformed: full animation playback review, continuous/subframe clearance, terrain contact, start/stop blending, socket integration, LOD/mipmap and crowded-room profiling, dungeon lighting/selection/grayscale/zoom-extreme review. Rigs remain reduced, with no production sockets or measured gait/speed metadata.

Next task: coordinator reviews the nine PNGs and remaining stylization/depth/size assumptions before authorizing the import/material pass. Preserve gameplay definitions, collisions and authority-owned timing; contact must bind to `ImpactSeconds`, not an animation damage notify. Rebuild generated assets from source. No project gate or final visual approval is claimed here.
