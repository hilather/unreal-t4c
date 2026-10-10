# W5-08d — scripted creature polish candidate

Task ID: **W5-08d**. Contract revision: **1**. Base revision: `70e84a5940a253205b17ddd80eb1f4a404db94a7`.
Result revision: the commit containing this document; the exact candidate is recorded in the attempt report and submission receipt.
Owned paths: `artsource/creatures/` and this document. No committed binary assets, engine content, source gameplay code or shared contracts. This is an evidence candidate for coordinator review, not visual acceptance or a passed project gate.

## Construction and provenance

The [rebuild instructions](../../artsource/creatures/README.md), [rat/slime geometry and materials](../../artsource/creatures/models.py), [bat anatomy](../../artsource/creatures/bat_polish.py), [bake/rig/export pipeline](../../artsource/creatures/build.py) and [export preview script](../../artsource/creatures/preview_exports.py) are deterministic headless Blender source. The optional [source preview](../../artsource/creatures/preview_source.py) makes unbaked drafts for short feedback loops; final evidence uses imported GLBs.

Blender **5.2.2 LTS, d13f752e3b9c**, at `/home/brewerm/Downloads/blender-5.2.2-linux-x64`, was run with `--background --factory-startup --python-exit-code 1`, checkout-local `XDG_CONFIG_HOME`, CPU Cycles and bounded thread counts. No installation, asset download or system change was performed. Seeds remain rat **508**, bat **509**, slime **510**. Construction signatures repeat on this Blender build; bit-identical renders across hardware or Blender versions are not promised.

The concept at `/home/brewerm/Downloads/lighthaven-unreal-design-and-agent-waves/lighthaven-plan/assets/concepts/starter-characters-and-creatures-concept.png` was opened for visual comparison on **2026-10-10**. It was never copied or loaded into any geometry/material generator. The [creature specifications](art/creatures/README.md), individual rat/bat/slime pages and [visual-monsters.md](visual-monsters.md) supplied the presentation constraints. Every new dimension, color, sculpt operation, shader parameter and preview-light setting is **Prototype presentation tuning: W5-08d, LH_Prototype_v1, 2026-10-10, source_url null**. No source-backed mechanics changed.

## Visible changes and budgets

| Creature | Exported triangles | Joints | Embedded textures | Material slots |
|---|---:|---:|---|---:|
| Brown rat | 7,856 | 11 | base / tangent normal / ORM, each 1024² | 1 |
| Bat | 7,150 | 8 | base / tangent normal / ORM, each 1024² | 1 |
| Green slime | 5,184 | 2 | base / tangent normal / ORM, each 1024² | 1 |

**Rat.** Shoulder and thigh masses now blend into a continuous body/neck/head surface. A build-time 3.5 mm voxel union, smoothing and reduction produce about 1,750 skin triangles; no remesh modifier or high-poly sculpt is exported. The final surface receives explicit blended head/body/leg weights and 1,600 surface-rooted narrow guard-hair cards. Later iteration shortened the distal muzzle 18%, added rounded whisker pads and buried the eyes more naturally. The rat retains four bare clawed feet and the three-bone tail. Ears now recess behind the rim, with a pink cup and thin furred back. Broad coat values give a darker saddle, warmer flanks, paler underside and narrow lighter dorsal tips. Tail rings affect both baked color and normal detail.

**Bat.** A larger tapered furry torso replaces the small round body. The head has surface-rooted cheek/forehead fur, inset eyes and integrated brow folds. Feedback reduced the round forehead, shrank and buried the muzzle pads, darkened that local fur and shortened the small nose leaf. Small teeth, thin cupped ears with branching cartilage ridges/tragus and exposed ivory thumb hooks remain visible in the hero view. The established four-panel membranes, scallops, spars, vein material and wing weighting are retained. There are still four wing joints, not separately rigged skeletal fingers.

**Slime.** All procedural ring dots and separate surface bubble spheres were removed. The replacement is a continuous low field of rounded unequal lobes with small flattened edge drips. Thickness-like color makes the core denser/darker and the thin rim lighter; four irregular soft inclusion highlights avoid a repeated dot pattern. A second iteration darkened the core, separated the lobes and tightened specular highlights. A final rim correction lowered the perimeter below its neighboring ring and separated underside normals to remove an angular black lip artifact. The shader is still opaque: these are surface color/depth cues, not actual suspended refractive bubbles or internal volume.

All creatures use one double-sided baked PBR material. The inherited bake remains diffuse-color-only base color, tangent-space normal, and packed **geometric AO / roughness / metallic=0**. Cycles AO uses a 6.5 cm distance and 32 samples, baked at 8 samples. The standard glTF material settings group binds the packed ORM to both occlusion and metallic/roughness. UV margin remains 0.0005 fractional, with 2 px bake dilation. Fur-card islands and near-budget triangle counts need later mip/LOD profiling; the small atlases are not a substitute for that check.

## Rigs, actions and measured envelopes

The existing five tracks remain at 30 fps: **idle 60f/2s, move 24f/0.8s, attack 42f/1.4s, hit 18f/0.6s, death 60f/2s**. Anticipation continues through f29, contact is **f30 = 1s**, and recovery ends at f42. Death holds through f45–f60. Floor roots remain constant. These are presentation clips, not damage notifies, authority timing changes or measured gait speeds.

Both source bounds and independently re-imported GLB bounds were sampled at every integer frame. The final imported-action union is below, in centimetres, rounded:

| Creature | X min / max | Y min / max | Z min / max |
|---|---|---|---|
| Rat | −66.19 / 20.00 | −12.78 / 13.82 | 0.10 / 25.81 |
| Bat | −22.18 / 15.44 | −39.96 / 39.96 | 0.10 / 132.06 |
| Slime | −42.80 / 44.10 | −41.07 / 44.67 | 0 / 23.65 |

Rest tops are **24.72 / 122.59 / 21.11 cm** respectively. As in W5-08c, the written sizes are treated as clearance envelopes rather than a requirement to fill every axis. The slime is deliberately lower than the nominal 40 cm rest peak in response to the polish brief. The rat's shortened muzzle leaves unused front clearance. Maximum allowed animated heights remain 35 / 145 / 60 cm. No capsule, nav, reach, actor-scale or selection policy changed.

Units remain metres in Blender/glTF, +X forward and +Z up. The importer must calibrate centimetres. The integer-frame audit uses the existing 0.5 mm envelope tolerance and −0.7 mm minimum floor allowance; it does not certify continuous swept clearance or terrain contact.

## Preview conditions and feedback

Nine final renders were completed and visually inspected: `rat_hero.png`, `rat_gameplay.png`, `rat_actions.png`, and the corresponding `bat_` and `slime_` names. All are **1280×720, 12-sample denoised Cycles CPU** renders from the exported files, with no image compositing or concept pixels.

Hero views use the separate studio setup. Gameplay uses procedural dark stone-like slabs, a **warm point key** at (1.5, −1.4, 2.4) m, 650 W and 0.12 m radius, plus a restrained cool 120 W area fill and ambient fill. The first darker setup hid too much of the rat; the final review lighting exposes its body/tail while retaining dark stone values. These are authored review lights, not measured Unreal torch settings.

The gameplay camera remains the inherited **12 m boom, 55° down pitch, 45° yaw, 45° horizontal FOV, 90 cm floor focus**. Mesh scale and gameplay framing were not enlarged for readability. Action sheets show independently evaluated/frozen idle f15, move f6, attack f30, hit f9 and death f60. They are pose inspections, not full-motion playback. Individual `--view hero|gameplay|actions` invocations avoid long combined render runs and overwrite the same filenames.

The feedback loop included: enlarging anatomical masses; replacing visible rat part seams with fused weighted skin; relocating fur roots onto final surfaces; recessing ear bowls; shortening and rounding the rat snout; reducing the bat's balloon-like forehead and pale rabbit-like muzzle pads; removing slime rings/warts; lowering/separating slime lobes; darkening its core; and checking the rat under two dark-floor lighting levels. Drafts are disposable and excluded from the final evidence package.

Honest concept comparison: the rat is a much more coherent stylized animal, but its short groom still reads as authored strokes rather than the concept's dense tangled fur. The bat has a strong wing/ear/thumb silhouette and a better compact face, but its expression and fur are still more stylized than the concept's natural anatomy. The slime has the clearest dark-core/light-rim separation and the requested low pooled silhouette, but its opaque shell cannot reproduce the concept's refractive internal depth. This pass does **not** establish that all three have reached the owner's “looks great” bar; final visual acceptance remains an explicit review decision.

At the fixed gameplay camera the rat's lit front mass, pointed head and thin tail separate from the stone, while the dark back and far tail remain the weakest parts. It is still a small target. The bat's near membrane/ears/body remain visible but its far wing is strongly foreshortened in this still; motion and orbit readability are not certified. The slime has the strongest small-scale value separation and recognizable irregular footprint. These limitations are retained in the actual gameplay images rather than changing creature scale or choosing only a flattering facing.

## Checks and limitations

Actually run on the final exports: full scripted bakes/exports; Blender re-import skin/action assertions; all source integer-action envelope checks; unchanged [GLB byte validator](../../artsource/creatures/validate_glb.py); and the new [Blender export audit](../../artsource/creatures/validate_blender.py). The byte validator passed triangle budgets, finite/unit vertex data, skin weights/indices, clip names/durations, constant roots, loop endpoints, death hold and occupied-UV AO variation/texture wiring, including decoded PNG CRC/filter checks. The final Blender audit reported **627 imported pose samples, zero envelope violations**, and matching construction SHA-256 signatures across two repeats of every creature (vertices, topology, transforms, weights and bones).

Python syntax checks and `git diff --check` also passed. Evidence summaries are `validation.json`, `glb-validation.json` and `export-pose-audit.json` in the attempt's `library/`, with final image/export SHA-256 values in `evidence-manifest.json`; generated GLBs/atlases remain in ignored local `artsource/creatures/output/`. No GLB, texture, render or reference image is committed.

One exploratory combined preview process exited **143** after writing the rat hero, without a diagnostic cause; shorter individual-view invocations are used for final evidence. Socket startup notices, Blender's `Material.use_nodes` deprecation warning and shared texture-sampler warnings also occurred; completed exports/re-imports returned zero despite those notices.

Not run: Unreal import/editor/build/cook/package/play (outside this task's authorized paths and requiring a separate import/runtime/material/unit-calibration task); Windows packaging (deferred). Also unperformed: continuous/subframe clearance, animation playback review, stair/terrain contact, start/stop blending, production sockets, LOD/mip testing, grayscale/orbit/zoom extremes, selected-target ring visibility and crowded-room profiling. No engine absence is inferred. The rigs remain reduced, and opaque slime/membrane shading is a concrete visual limitation.

Next task: coordinator reviews the nine PNGs, the remaining concept gap and size assumptions before deciding whether another finish pass or the separate import/material task should follow. Preserve gameplay definitions, collision and authority-owned impact timing. No project gate or verified task success is claimed here.
