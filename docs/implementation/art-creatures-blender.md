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

# W5-08f — bat family and slime from the packaged lineup

Task ID **W5-08f**, contract revision **1**, actual isolated base
`a701b606ccebcb36a9ee9d826c7f8402d3599637`. The brief mentions `a62e130`;
this candidate uses the assigned checkout/submission base, without resetting it.
The result is the commit containing this appended section, identified in the
attempt report and submission receipt. Owned changes are Blender/Python scripts
under `artsource/creatures/` and this document. No generated binary is committed.
This is an evidence candidate, not engine integration or a passed project gate.

## Visual decisions and provenance

The supplied B1 packaged-build contact sheet, individual bat close/gameplay
captures and slime close capture were inspected at
`/home/brewerm/.herdr-projects/unreal-t4c/scratch/captures/w5-10e-lineup/`.
All **11** entries were reviewed: the brief's “other seven” is interpreted as the
**six** creatures beyond the four bats and slime. Rat, both goblins, giant spider,
atrocity and Balork have no plain defect requiring another edit in this pass and
retain their artwork. The supplied `slime-gameplay.png` is occluded by a near
surface and cannot support a gameplay before/after comparison.

The package's starter and deep-dungeon concept sheets supplied visual targets
only. No reference image was copied, traced, sampled into a material or used as a
mesh input. No asset/tool installation or download occurred. All new dimensions,
colors, poses, material precision and review lighting are **Prototype presentation
tuning: W5-08f / LH_Prototype_v1 / 2026-10-11 / source_url null**. No source-backed
mechanics changed. The current task's explicit bones/glow request supersedes the
older undead spec's visual prohibition; its size/clearance limits remain intact.

The common bat has broader membrane chords, shallow camber and a held-open wing
plane. Leading spars, thumbs, wing bones and wing-tip anchors follow the authored
wing geometry. Smaller idle/move strokes avoid turning the whole web edge-on.
Reduced fur-card density, ear tessellation and membrane subdivisions fund the
larger readable shapes without adding triangles. Heads and cupped ears remain
separate visible masses. All existing bone names, parent relationships, action
names/durations and exported anchors survive; the ordinary bat's original eight
bones and slime's two bones are retained, rather than claiming new sockets.

Variant identities are broad baked cues: warm brown bat; dark slate dungeon bat
with a wide pale dorsal chevron; larger brown giant bat with broad ochre leading
edges; bone-pale undead bat with missing membrane wedges, a dark recessed thorax,
four exposed curved ribs, a spinal ridge and faint blue eyes. Tears are real
missing geometry, not transparency. Undead eyes have one additional material
slot sharing the existing base/normal/ORM images; emission uses the baked blue
base-color eye islands at strength **0.18**. There is **no fourth texture**.

Slime now has one dominant off-center dome, connected lower shoulders, shallow
skirt folds and a grounded irregular rim. Its rest peak rises from **21.11 cm**
to **36.38 cm** within the 40 cm ruler. Antiphase lateral squash and vertical
breathing create idle/move wobble while root-weighted skirt vertices stay put.
The darker green core, broad bright rim and low-roughness highlights remain
opaque PBR surface cues, not refraction, emission or internal volume.

## Measured export budgets

Both sides were actually rebuilt with provided **Blender 5.2.2 LTS
`d13f752e3b9c`**, factory startup, CPU Cycles and checkout-local
`artsource/creatures/.config`. Before files came from an unchanged copy of the
assigned base scripts. Values below are **before → after**, in bytes except
triangles. Mesh bytes count unique GLB buffer views referenced by primitive
attributes/indices; texture bytes count the three embedded PNG payloads.

| Creature | Triangles | Mesh buffers | Embedded textures | Complete GLB |
|---|---:|---:|---:|---:|
| Bat | 7,150 → 5,550 | 651,820 → 468,384 | 773,161 → 742,794 | 1,463,116 → 1,249,916 |
| Dungeon bat | 7,150 → 5,550 | 647,452 → 467,292 | 774,532 → 639,291 | 1,479,128 → 1,163,876 |
| Giant bat | 7,150 → 5,550 | 640,120 → 468,956 | 711,643 → 501,767 | 1,408,860 → 1,027,160 |
| Undead bat | 7,150 → 5,234 | 648,648 → 384,796 | 743,938 → 516,733 | 1,449,856 → 960,248 |
| Green slime | 5,184 → 3,680 | 192,044 → 148,908 | 256,121 → 216,333 | 459,864 → 376,940 |

Every individual base/normal/ORM PNG also stays below its corresponding baseline
byte size; all remain **512×512, RGB, 8-bit PNG containers**, three per creature.
The [budget audit](../../artsource/creatures/audit_polish.py) records each channel
and rejects growth, altered rig hierarchies, changed action names or extra maps.
Slime and giant-bat baked pigment is quantized to 6-bit levels in sRGB and normal/
ORM data to 7-bit levels in linear space before saving. This reduces compressed
entropy without changing dimensions or runtime texture count. Precision is
recorded in build validation. The initial higher-entropy attempt was rejected;
final images were reviewed after quantization. These export measurements do not
certify the size of reimported Unreal assets or the remaining 25 MB cook budget.

Source animation checks and independent GLB re-import checks use unchanged spec
rulers and tolerances. Final rest spans are about **77.46 / 96.63 / 162.03 /
102.75 cm**, respectively; all live/death samples fit the **80 / 100 / 170 /
110 cm** lateral ceilings and their fore-aft/height rulers. Slime's idle peak is
**39.65 cm** and move peak **40.74 cm**, below its 60 cm animated limit. The
five tracks retain idle **2.0 s**, move **0.8 s**, attack **1.4 s**, hit **0.6 s**,
death **2.0 s**, with the f30 attack pose and f45–f60 dead hold.

## Feedback renders and observed limits

[preview_polish.py](../../artsource/creatures/preview_polish.py) imports the
exported GLBs. Final evidence consists of **13 PNGs**: hero/gameplay for all five,
one common-scale warm bat-family comparison, and bat/slime action sheets.
All are **1280×720, 12-sample denoised Cycles CPU** renders. Gameplay uses the
unchanged 12 m camera, 55° down pitch, 45° yaw/FOV and 90 cm focus height; no
creature is enlarged for the camera. Procedural slate slabs, a 650 W warm point
key and restrained cool fill approximate the capture's stone-floor/torch setting.
They do not reproduce Unreal exposure, shadowing, texture mips or post-processing.
Hero lights are reduced to 35% of the inherited studio power and the bat camera
is elevated to expose the membranes. Family framing uses common mesh scale and
uniform warm/cool directional light; it is an overview, not the game camera.

Iterations broadened the wing plane twice, reduced the overly tall first ear
pass, darkened the ordinary bat's washed-out membranes, restored smaller slime
shoulders/skirt hollows to a too-smooth first dome, and reduced texture entropy.
Failed draft images were overwritten and are excluded from the evidence set.
The final bat/slime action sheets inspect five sampled poses, not motion video.

Concept proximity remains partial: bat anatomy is rounded and stylized, fur is
sparse and membranes relatively planar, with angular undead tear slots; slime is
glossy opaque jelly with a more
saturated rim than the naturalistic concept. Broader wings and broad value marks
carry gameplay identity; tiny eye/rib detail is mainly visible in hero views.
A fresh Unreal lineup is still required to judge warm-light recognition, motion,
zoom/orbit extremes, mip behavior and the owner's final visual quality bar.

## Checks and integration boundary

Actually run: final bakes/exports and source integer-frame bounds; GLB byte
validation (finite attributes, skin weights/joints, normalized normals, clip
names/durations, stationary roots, loop endpoints, death hold, PNG decoding and
occupied-UV geometric AO variation); strict before/after byte/hierarchy audit;
independent Blender re-import with **1,045 integer-frame pose samples, zero
envelope violations**, and matching geometry/bone construction signatures across
two repeats per creature; Python compilation; `git diff --check`; and visual
inspection of the final evidence. The undead validator additionally requires a
faint emissive material, shared base atlas and cold-blue eye pigment. The slime
AO check initially caught a too-convex dome; geometry was corrected without
relaxing the validator. Final occupied slime AO is 223–255 with 17 distinct values.

Evidence is in this attempt's `library/`: `budget-audit.json`,
`glb-validation.json`, `source-validation.json`, `export-pose-audit.json`,
`evidence-manifest.json` and the 13 render PNGs. Generated GLBs/atlases stay ignored
under `artsource/creatures/output/`; they can be regenerated from source. Several
superseded render processes were deliberately interrupted while iterating. One
multi-view loop exited 143 without a diagnostic; its remaining views were rerun
individually. Only completed final renders enter the manifest. Blender's socket startup notices,
material deprecation and shared-sampler warnings did not prevent final export,
import, rendering or audits.

**Concrete downstream issue found by read-only code inspection:** the current
Unreal import script creates `M_Creature` without an emissive input
(`build/build-art.sh`, master-material block), and
`ULHMonsterVisual::ConfigureCreatureMaterial` replaces every material slot with
the same `MI_Creature`. The import cleanup then removes other imported materials;
`LHCreatureArtTests.cpp` expects this uniform binding. Therefore a normal current
reimport would discard the undead eye emission even though it is present in the
validated GLB. The coordinator's separate import task must preserve an eye-only
material instance sharing the three atlases, add an emissive parameter/input,
make persistent binding/cleanup slot-aware and adjust the material test. Those
paths are outside this worker's ownership and were **not edited**.

Not run: Unreal import/editor/build/cook/package/play, post-import material size,
new B1 captures, socket alignment, collision/nav/stairs, continuous or subframe
clearance, animation blending/playback, LOD/mips or crowd performance. Missing
prerequisite is a separately authorized Unreal integration/material pass and a
new packaged lineup of these exports, not an assumed absent engine. Windows is
deferred by project decision. Next task: review these artifacts, integrate the
five regenerated meshes plus the eye material correctly, measure the real
import/cook budget, and recapture the same B1 lineup before judging in-game polish.

Reproduction from the checkout (repeat `--only` for each changed creature):

```sh
export BLENDER_ROOT=/home/brewerm/Downloads/blender-5.2.2-linux-x64
export XDG_CONFIG_HOME="$PWD/artsource/creatures/.config"
"$BLENDER_ROOT/blender" --background --factory-startup --threads 5 --python-exit-code 1 \
  --python artsource/creatures/build.py -- --only bat --no-render
python3 artsource/creatures/validate_glb.py --only bat
"$BLENDER_ROOT/blender" --background --factory-startup --threads 2 --python-exit-code 1 \
  --python artsource/creatures/validate_blender.py -- --only bat
"$BLENDER_ROOT/blender" --background --factory-startup --threads 5 --python-exit-code 1 \
  --python artsource/creatures/preview_polish.py -- --only bat --view hero
"$BLENDER_ROOT/blender" --background --factory-startup --threads 5 --python-exit-code 1 \
  --python artsource/creatures/preview_polish.py -- --only bat --view gameplay
"$BLENDER_ROOT/blender" --background --factory-startup --threads 5 --python-exit-code 1 \
  --python artsource/creatures/preview_polish.py -- --family
python3 artsource/creatures/audit_polish.py --before /tmp/w5-08f-baseline/output \
  --after artsource/creatures/output --report /tmp/w5-08f-budget-audit.json
```

For a later machine, reconstruct the comparison directory from the specified
base revision's scripts before running the last command; `/tmp` is not an input
asset dependency. No gameplay rule implementation or external image is required.

# W5-20 — creature animation life candidate

Task ID **W5-20**, contract revision **1**, base
`4faca9576012caf51e18cb774f452e0cb8a689aa`. Result revision is the commit
containing this appendix; the attempt report and submission receipt record its
exact object ID. Owned paths: `artsource/creatures/` and this document. No
Unreal assets, gameplay code, textures, GLBs or renders are committed. This is
an animation/source evidence candidate for review, not integration, visual
acceptance, an imported-budget pass or a project gate.

## Authored motion and preserved interfaces

[animation_life.py](../../artsource/creatures/animation_life.py) replaces all
eleven creatures' idle, attack, hit and death poses on their existing rigs.
[build.py](../../artsource/creatures/build.py) samples the eased pose curves at
30 fps through the established NLA/glTF exporter. The original move callbacks
and sample density are retained. Bones, hierarchy, rest sizes, removable weapon
meshes, action/track names and attachment anchors are retained. No gameplay
collision, capsule, attack reach, timing authority or damage notify changed.

Every new curve, angle, translation and clearance allowance is **Prototype
presentation tuning: W5-20 / LH_Prototype_v1 / 2026-10-11 / source_url null**.
There are no new source-backed mechanics. The supplied W5-19 lineup sheet and
Balork close capture were opened as visual context; their pixels were not used
as materials or geometry. No downloads or external image inputs were used.

| Creature | Attack construction and secondary motion |
|---|---|
| Rat | Low recoil/coil, open jaw, short whole-body dart and bite closure at contact; three tail joints whip through delayed arcs after impact, forefeet and rear legs counter the body. |
| Bat, dungeon bat, giant bat, undead bat | Rise and pitch back into the bite preparation, dive through contact, down-snap the wings, let the tips drag behind and rebound; body size changes the swoop amplitude. |
| Slime | Compress first, draw into a tall narrow crest, slam into a broad low pool at contact, then overshoot upright and settle; no topology or shader changes. |
| Goblin | Draw the upright spear into a two-handed guard, shift back, plant the leading foot and thrust through the contact pose before recovering. |
| Goblin warrior | Raise the polearm behind the head, brace, chop down through contact, carry the blade past the strike angle and recover. |
| Giant spider | Lift the front leg pairs, rear the thorax, counter-rotate the abdomen, then stamp the forelegs while the head/fangs lunge. |
| Atrocity | Lift both claws, coil the shoulders across the hips, rake both hands down with the shoulder turn, curl the claws and recover. |
| Balork | Deep opposing hip/chest coil, stepped brace, broad two-handed axe sweep opening toward the reviewed camera at impact, further shoulder overshoot, flared wings and delayed tail/wing-tip recovery. |

Weapon-bearing attacks analytically solve the existing arm chains against a
single rigid weapon transform. Both wrists and hand orientations follow that
transform; the result is baked into existing channels. No runtime IK or new
bones are required. This is a stylized two-bone solve, not a certification of
anatomical skinning or perfect sole contact.

Idle includes breathing/weight shifts, staged head looks, goblin ear twitches,
claw flexes, spider palp/foreleg twitches and wing/tail lag. Hit now peaks at f3,
with delayed head/appendage recoil, counter-recoil and recovery. Death uses a
slow loss of balance followed by faster collapse, a small settle and a stable
final pose. Bat corpses roll and tuck the wings; rat rolls onto its side;
goblins fold with the weapon laid down; spider legs curl; atrocity sprawls;
Balork kneels and slumps. The retained rigs have no independently animatable
rat/bat ear bones or full membrane/finger articulation.

The five clips keep **idle 60f/2s, move 24f/0.8s, attack 42f/1.4s, hit 18f/0.6s,
death 60f/2s**, with frame zero included in the bake. **Attack contact remains
f30 / 1.0s**. A brief contact/follow-through hold precedes recovery to f42.
The previous text's literal anticipation hold through f29 is superseded: f28
is the last coil pose and f29 is committed travel into the f30 contact pose.
This does not move the contact anchor or authority-owned impact time. Death
now settles until f45 and holds exactly f45–f60. All floor-root channels remain
constant, and idle/move endpoints match. Source keys and pose review establish
an animation anchor; an Unreal timing/collision test is still separate.

## Measured attack clearance and byte accounting

The attack-only `ATTACK_RULERS` in
[roster.py](../../artsource/creatures/roster.py) explicitly accommodate the new
swings. Rest, idle, move, hit and death retain their prior rulers. The sampled
bounds include body, wings, tails and carried weapons; they are presentation
clearance, **not** damage reach or a request to enlarge collision. Long goblin
weapons and Balork's transverse axe now extend much further forward during a
swing. Integration needs to review visual wall/neighbor intersections with
these stated extents.

Sampled **attack** bounds in centimetres, all 43 integer frames per creature.

| Creature | X min / max | Y min / max | Z min / max |
|---|---:|---:|---:|
| rat | -66.25 / 20.34 | -12.35 / 21.77 | 0.10 / 30.69 |
| bat | -24.27 / 19.92 | -38.84 / 38.86 | 73.68 / 138.78 |
| slime | -42.79 / 44.07 | -41.11 / 44.58 | 0.00 / 54.37 |
| goblin | -35.11 / 148.39 | -41.23 / 45.89 | 0.10 / 184.18 |
| giant_spider | -64.51 / 85.08 | -83.13 / 84.08 | 0.10 / 91.09 |
| balork | -219.91 / 226.76 | -216.47 / 215.00 | 0.10 / 302.76 |
| goblin_warrior | -49.43 / 163.46 | -46.00 / 48.37 | 0.10 / 217.95 |
| atrocity | -64.80 / 141.47 | -107.90 / 82.82 | 0.10 / 200.60 |
| dungeon_bat | -27.67 / 22.15 | -48.42 / 48.46 | 71.70 / 149.77 |
| giant_bat | -41.67 / 32.40 | -81.15 / 81.04 | 67.01 / 193.37 |
| undead_bat | -31.02 / 24.88 | -51.49 / 51.45 | 73.15 / 157.34 |

The exact full five-clip bounds, chosen rulers and per-action source/reimport
comparisons are retained in `library/animation-after.json`. Rest and all
non-attack clips pass the original clearance boxes.

Animation byte counts below are the **actual unique glTF sampler input/output
accessor payloads**, using the same exporter for baseline and candidate. The
Unreal column is the base revision’s sum of its five `A_*.uasset` files, read
from LFS declarations where the checkout holds pointers. After-import Unreal
bytes are **not measured** for any creature.

| Creature | glTF before B | glTF after B | Delta B | Unreal before B |
|---|---:|---:|---:|---:|
| rat | 14,060 | 35,624 | +21,564 | 560,339 |
| bat | 13,040 | 25,768 | +12,728 | 537,711 |
| slime | 3,760 | 6,728 | +2,968 | 445,173 |
| goblin | 24,688 | 41,704 | +17,016 | 792,240 |
| giant_spider | 45,848 | 63,156 | +17,308 | 977,380 |
| balork | 30,520 | 52,384 | +21,864 | 895,644 |
| goblin_warrior | 24,688 | 41,704 | +17,016 | 836,040 |
| atrocity | 33,620 | 58,064 | +24,444 | 879,224 |
| dungeon_bat | 20,008 | 26,840 | +6,832 | 639,142 |
| giant_bat | 19,920 | 26,704 | +6,784 | 632,472 |
| undead_bat | 20,056 | 26,824 | +6,768 | 705,987 |
| **Total** | **250,208** | **405,500** | **+155,292** | **7,901,352** |

Animation JSON metadata increased by another **737 bytes**. The measured
payload growth is **0.155 MB**, below the requested approximately +1 MB
allowance at the exchange-file level. This does **not** establish compressed
Unreal growth. The tracked base creature assets total **23,230,812 bytes**
across 136 `.uasset` files. The 25 MB imported cap and after-import animation
bytes require a real Unreal re-import; neither is claimed passed here. No
atlas, texture resolution, geometry or material-construction changes were made.

## Visual feedback and evidence conditions

The contact-sheet renderer is
[preview_animation.py](../../artsource/creatures/preview_animation.py).
Every creature has ten attack frames **0,12,22,26,28,30,32,35,38,42**, viewed
both at the gameplay camera's **55° down / 45° yaw angle** and a **25° / 45°
three-quarter angle**, plus idle frames **0,12,27,43,60**. They are orthographic
inspection crops, not the fixed 12 m gameplay framing. Source procedural
materials and actual evaluated/deformed source meshes are rendered in Cycles
CPU with two studio sun lights. They are neither Unreal captures nor baked
atlas appearance evidence.

Final output is **28 PNGs**: eleven attack sheets, eleven idle strips and six
additional hit/death sheets (rat, bat, goblin, spider, atrocity, Balork). Most
use 280 px panels and 8 samples with denoising (1400×1120 attack, 1400×280 idle,
1400×560 reactions). Rat reactions use 320 px panels and 10 samples. All are
actual Blender renders. No image synthesis, paint-over or reference-image
compositing was performed. Rebuild/audit commands are in the
[README](../../artsource/creatures/README.md#w5-20--authored-attacks-idles-and-reactions).

The feedback iterations corrected the initial rat corpse side excursion,
ordinary-bat recoil, and Balork idle-tail excursion; reduced excessive bat
rise and spider foreleg lift; and reversed Balork's sweep so the contact
silhouette opens toward the reviewed camera instead of turning edge-on.
A later bat-corpse review replaced the hovering-looking spread-wing pose with
a side roll and wing tuck, then corrected the folding direction and brief
body travel to keep the existing corpse clearance.

Honest visual limits: the clearest differences are Balork's large open sweep,
slime's stretch/slam/rebound and the bat family's wing snap. Rat's body travel
remains short, with jaw/tail motion doing much of the work. Spider foreleg
anticipation reads, but its recovery and idle still feel restrained. Goblin,
warrior and atrocity idles retain stiff shoulders despite head/ear/claw motion;
Balork's breathing is also subtle. The bat family shares choreography rather
than four independently acted performances. Reduced rigs constrain secondary
motion, and the quick two-frame commitment can still read as a snap. These
are reviewable improvements, not a claim that every creature has reached a
final animation-quality bar. Full-speed, in-game visual acceptance remains
unperformed.

## Checks, uncertainty and integration handoff

[audit_animation.py](../../artsource/creatures/audit_animation.py) constructs
source rigs, evaluates all integer frames, exports actual temporary 30 fps
GLBs through `export_asset`, checks their animation accessors, then independently
imports them into a cleared Blender scene and samples all skinned parts.
It checks clip names/ranges, f30 source keys, finite animation data, constant
roots, idle/move endpoints, f45–f60 corpse holds, the stated envelopes and
source-versus-reimport bounds. Baseline comparison covers exact rig/geometry
hashes, triangles, exported node hierarchy/skins and rest bounds. An exact
geometry hash discrepancy is reported as a warning, never silently equated.

Blender **5.2.2 LTS, d13f752e3b9c** was used with background/factory startup,
checkout-local `XDG_CONFIG_HOME`, 2-thread audits and bounded CPU render jobs.
Two preview jobs and one narrow audit startup exited **143** with no
cause in their logs; retries as shorter jobs completed. PulseAudio socket and
material/world API deprecation warnings also appeared. No system changes,
engine/SDK installation or large downloads were performed.

Final evidence explicitly reports **11/11 creatures, 55 source clips, 2,299
source frames and 2,299 independently re-imported frames**, with zero failures
and zero geometry warnings. All baseline construction geometry hashes, rig
hashes and triangle counts match at the matching two-thread audit setting.
Source/re-import bounds differ by at most **0.0001 cm**. Four bat records were
replaced by final reruns after the corpse fix; the merged report records that
provenance. Earlier failing bounds are explicitly superseded, not counted as
passes. A different-thread smoke did exhibit tiny construction-coordinate
variation, which is why baseline comparisons use the same thread count.

Actually run additionally: Python syntax compilation for all creature scripts,
`git diff --check`, and an unchanged-file diff against the base for all six
geometry/material construction modules. The original texture/AO validators
were not rerun because these audit GLBs intentionally contain unbaked source
materials; no claim of a new atlas validation is made. Evidence is in this
attempt's `library/animation-before.json`, `animation-after.json`,
`imported-before.json`, `reruns/`, the 28 final PNGs, and `evidence-manifest.json`.
Generated temporary audit GLBs are removed; no new baked texture outputs are
needed for this animation-only evidence.

Not run: Unreal re-import, animation compression, editor/build/cook/package/
play, Windows packaging, authoritative hit-time playback, full-speed animation
review, continuous/subframe swept bounds, terrain/stair contact, start/stop or
cross-action blending, foot-lock/skin-intersection tests, production lighting
and fixed-distance gameplay readability. The concrete missing prerequisite
for the engine checks is the separate integration task with ownership of
`Content/` and runtime outputs; this Blender-only lane cannot update those
assets. Engine absence is not claimed. No project gate or imported size limit
is marked passed.

Next task: review the 28 PNGs and explicit wider attack extents, then regenerate
and re-import the five actions for all eleven existing skeletons, keeping game
capsules and authority timing unchanged. Measure actual compressed animation
bytes and the **hard 25 MB total imported-creature limit**, and replay attacks
against a target at the isometric camera before accepting the result.
