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

# W5-08e — remaining roster candidate

Task ID **W5-08e**, contract revision **1**, base
`079c9590778fef13ffba476bab3d43459ef5e7c3`. Result revision is the commit
containing this extension; the attempt report and submission receipt identify
its full object ID. Owned paths remain `artsource/creatures/` and this document.
This is a source/artifact evidence candidate for coordinator review, not visual
acceptance, engine integration, or a passed gate.

## Source and construction

The new [roster dispatch and rulers](../../artsource/creatures/roster.py),
[goblin/atrocity builder](../../artsource/creatures/humanoids.py),
[spider builder](../../artsource/creatures/spider.py),
[Balork builder](../../artsource/creatures/balork.py), and
[bat variants](../../artsource/creatures/bat_variants.py) extend the existing
pipeline. Rebuild and audit commands are in the
[creature README](../../artsource/creatures/README.md#w5-08e--remaining-roster).
The original rat, bat and slime modeling modules were not edited.

Both supplied concept sheets were opened for visual comparison on **2026-10-10**
at the package's `assets/concepts/` directory. Neither image was copied,
traced into geometry, loaded into a shader, or supplied as a mesh/texture input.
No external art, engine, SDK, library or system installation was performed.
These are new scripted constructions, not recovered T4C meshes. All new
measurements, material colors, anatomical reconstruction, procedural patterns,
rig weights, animation poses and review lighting are **Prototype presentation
tuning: W5-08e, LH_Prototype_v1, 2026-10-10, source_url null**. The creature
specifications supply presentation constraints only. No mechanics value,
collision capsule, spawn placement, combat event or roster ID changed.

The tool was the provided **Blender 5.2.2 LTS (`d13f752e3b9c`)**, run headlessly
with factory startup, `--python-exit-code 1`, checkout-local `.config`, CPU
Cycles and bounded 3–4 thread jobs. Geometry uses authored metre units, +X
forward and +Z up in Blender; the glTF exporter performs its normal Y-up
conversion, and the evidence importer converts back. Unreal centimetre import
calibration remains unperformed.

Goblin-family and Balork primary anatomy is built from overlapping muscle
volumes, voxel-unioned, smoothed, reduced and explicitly weighted. Fingers,
facial ridges, claws, equipment and membranes remain separate sculpt elements
inside the final skinned assemblies. Spider legs have eight articulated chains,
substantial proximal masses, tapered/fluted chitin segments and rooted sparse
bristles. Bat variants reuse the polished bat topology/rig with authored
proportions and broad procedural pigmentation; they are separate roster exports.
No procedural sculpt modifier or source image dependency is needed at runtime.

## Roster and identity

| Script/export stem | Gameplay definition | Distinguishing construction |
|---|---|---|
| `goblin` | `Enemy.Goblin` | Crouched iron-red body, long pointed ears, angular brow/nose, bare back, slender leaf spear |
| `giant_spider` | `Enemy.GiantSpider` | Separate rear abdomen and low front shield, eight jointed legs, red eye cluster, fangs and bristles |
| `balork` | `Enemy.Balork` | Massive oxblood demon, curved paired horns, red/dark patterned scalloped wings, cloven hooves, tail, lateral double-ended axe |
| `goblin_warrior` | `Enemy.GoblinWarrior` | Same head-height ruler, broader layered leather yoke, pale upper-back band and wider three-prong polearm |
| `atrocity` | `Enemy.Atrocity` | Broad hunched dark shoulders, forward head, long arms, yellow/ochre claw fans and swept dorsal spines |
| `dungeon_bat` | `Enemy.DungeonBat` | Clipped wing tips, slate/umber palette and broad dorsal chevron |
| `giant_bat` | `Enemy.GiantBat` | Wider wings and inner shoulder mass, brown coat and continuous ochre leading-edge band |
| `undead_bat` | `Enemy.UndeadBat` | Charcoal center, pale outer panels and one broad modeled notch on each outer trailing wing |

These retain the proposed `Presentation.Enemy.<ExactRosterSuffix>` mappings
from the individual briefs. They create no Unreal resolver or assets. The
existing floor decisions, including provisional/disputed bat placements, remain
unchanged.

## Baking, equipment and animation

Each export embeds one shared material and three **1024²** textures: sRGB base
pigment, tangent normal, and non-color **AO / roughness / metallic**. Base pigment
is baked through a temporary emission connection to avoid losing metallic
weapon color in a diffuse BSDF bake; the original surface is restored for normal
baking. The exported shader is ordinary non-emissive PBR. AO remains the actual
Cycles geometric shader bake at **6.5 cm**, **32 AO samples**, **8 bake samples**,
with the inherited UV margin and 2 px dilation. Exposed iron/bronze may be
metallic; organic skin, chitin, fur, membranes, leather and claws are nonmetal.
Membranes are opaque and double-sided. No alpha hair, transmission or subsurface
runtime material is introduced.

The two goblins export separately removable `<kind>_body` and `<kind>_weapon`
skinned meshes sharing their rig and atlas. Their primary attach bone is
**`Weapon_R`**; **`Weapon_Main`**, **`Weapon_Support`** and **`VFX_WeaponTip`**
are additional named anchors. Balork exports **`balork_body`** and
**`balork_weapon`**, with **`Weapon_Main`**, **`Weapon_Support`**,
**`VFX_WeaponTip_A`**, **`VFX_WeaponTip_B`**, **`VFX_Hit`**, **`UI_Anchor`** and
**`WingTip_L/R`**. Hide/remove the named default weapon mesh before swapping.
These are exported bones, not installed Unreal sockets; alignment and actual
attachment still need import testing. Both hands have authored grip poses;
there is no runtime support-hand IK solver. Tip bones never generate damage.

All eight have the five required tracks at **30 fps**: `idle` **60f / 2s**,
`move` **24f / 0.8s**, `attack` **42f / 1.4s**, `hit` **18f / 0.6s**, and
`death` **60f / 2s**. A stationary floor root owns no motion. Contact is authored
at **f30**, after anticipation through f29, with cosmetic recovery through f42.
Death reaches a grounded final pose by f30 and holds through f60. The new
callbacks key every integer frame; the shared grounding correction translates
the body bone from measured evaluated soles rather than clamping vertices.
The two goblin death clips use the written larger polearm death boxes
(190×100 and 210×110 cm); all live clips retain their transit boxes.

The move tracks are short in-place pose cycles. Start/stop/turn blending, runtime
attack retiming, cancellations, terrain contact and animation Blueprint/state
machine integration are **not** implemented here. Optional alerts and room-only
wide Balork swings were not added. No animation notify changes native authority.

## Evidence conditions and review limits

Final evidence is rendered from imported GLBs, including removable equipment,
using the inherited **1280×720, 12-sample denoised CPU Cycles** harness.
Hero cameras frame each creature; gameplay retains the same **12 m boom,
55° down pitch, 45° yaw, 45° horizontal FOV and 90 cm floor focus**, dark
procedural slate floor, warm 650 W point key and restrained cool fill. Actor
scale was not enlarged to make small creatures readable.

Action sheets sample actual imported idle f15, move f6, attack f30, hit f9 and
death f60. The sheet lights now give uniform directional studio coverage across
the whole row: the old local lamps left large Balork copies at either edge
almost black. Hero/gameplay lighting is unchanged. Contact sheets inspect poses,
not motion playback. The optional [bat family comparison](../../artsource/creatures/preview_family.py)
uses four unscaled imported family members under common studio illumination and
a separate overview camera, in color and grayscale. It does not substitute for
the individual fixed gameplay views.

The iteration included joined anatomy and larger limb masses, recessed visible
eyes and pointed face planes, dark/light skin separation, rigid weapon death
placement, wing-clearance reserves, spider shell darkening and relief changes,
Balork hand/shaft alignment and cloven soles, a deeper grounded boss collapse,
and a fix preventing the goblins' “iron red” skin name from being classified as
metal. Source-pose failures were corrected in the artwork instead of relaxing
clearance rulers.

These remain stylized reconstructions with a material gap from the illustrated
concepts. Smooth muscle volumes, repeated horn/spine forms and simplified facial
sculpting are most apparent in hero views. Scripted pores, grain and bristles do
not reproduce the concept's dense irregular microdetail. Fine eye/tooth/finger
anatomy is not relied on for gameplay identity. The task report records the final
per-creature visual assessment and measured validation results; no claim is made
that the owner's “looks great” bar has been accepted.

Unperformed checks require a separate authorized import/runtime task: Unreal
asset/material/socket import, centimetre calibration, editor/build/cook/package/
play, integrated lighting, collision/navigation, continuous/subframe sweeps,
stairs, crowded rooms, selection ring readability, LOD/mips and movement blends.
Windows checks remain deferred by project decision. No engine absence is
inferred. The boss encounter companion's actual B4 door/chase/corpse and reward
checks are outside this art-only task. No completion, loot, save or permanent
campaign state is implemented by these files.

## Measured W5-08e export checks

| Creature | Triangles | Bones | GLB bytes | Imported frames checked |
|---|---:|---:|---:|---:|
| Goblin | 8,146 | 23 | 2,515,624 | 209 |
| Giant spider | 9,606 | 33 | 3,586,432 | 209 |
| Balork | 14,396 | 30 | 3,424,900 | 209 |
| Goblin warrior | 8,914 | 23 | 2,512,600 | 209 |
| Atrocity | 8,023 | 27 | 3,070,424 | 209 |
| Dungeon bat | 7,150 | 13 | 3,192,824 | 209 |
| Giant bat | 7,150 | 13 | 3,006,400 | 209 |
| Undead bat | 7,150 | 13 | 3,021,672 | 209 |

The final source builds, GLB byte validation and independent Blender re-import
audits completed for all eight. **1,672 imported integer-frame samples reported
zero envelope violations**, and each creature's geometry/topology/transforms/
weights/bones signature matched across two independent constructions. These are
checks of these artifacts on this Blender build, not continuous clearance,
animation quality, visual acceptance or runtime performance certifications.
All exports contain the five named animated tracks and three 1024² PNGs; the
byte audit passed skin/normal/joint checks, stationary roots, loop endpoints,
death hold, decoded PNG checks and occupied-UV AO variation. The three armed
exports also passed the removable body/weapon node and primary bone checks.

The unchanged pilot bat was additionally rebuilt with the shared bake harness;
its 7,150 triangles, byte checks, construction repeat and 209 imported pose
samples passed. Rat and slime were not rebuilt in this task. Python syntax and
`git diff --check` passed. Source anatomy modules for all three pilot creatures
remain unchanged; shared baking and preview changes warrant ordinary review.

The final bat variants tilt their membrane mesh and corresponding wing bones
0.50 radians toward the fixed gameplay camera. A named vertex attribute retains
the original design coordinates for pigmentation, so the markings stay attached
to their intended wing regions. In the common-scale comparison, the dungeon
bat's narrow pale band, giant bat's size and ochre leading edge, and undead
bat's broad pale outer panels remain distinguishable in color and grayscale.
This is a single posed, lit comparison; orbit, motion and integrated dungeon
readability remain open. The dungeon and undead bats are still small targets.

Final evidence is limited to 24 creature PNGs plus the two family comparisons,
with compact validation and SHA-256 summaries in the attempt's `library/`.
Generated GLBs and atlases remain local, ignored and regenerable. One long
combined render loop exited 143 without a diagnostic cause; short individual
view invocations completed the final evidence. Startup socket notices and
Blender material/sampler warnings did not prevent successful export/re-import.
