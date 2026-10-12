# W5-23 — original Blender props, mesh effects and icons

Task ID: **W5-23**. Base revision: `6c5b276e9211e6261e7adb2f767d206530bb8af5`.
Result revision: the submission commit containing this document; exact candidate
OID is recorded in the submission receipt. Contract revision: **1**.
Owned paths: `artsource/props/` and this document. No binary assets committed.
This is an **evidence candidate for the subsequent import task**, not an Unreal
integration result or a gate pass.

Behavior changed: reproducible Blender source builds the fourteen catalog items,
a dropped sack, corpse loot badge, closed/open chest, eleven static mesh effects,
and twenty-one transparent 128px icons. It exports named mesh components, a
shared baked material atlas, a measured manifest, independent byte validation,
Blender re-import validation and CPU render evidence. Runtime behavior is unchanged.

Source-backed mechanics: none added or modified. Source IDs come from
`Source/Lighthaven/Data/Items/LHItemCatalog.cpp` and `LHAbilityCatalog.cpp`.
All dimensions, geometry, colors, lighting, icon composition and suggested
animation envelopes below are **Prototype presentation tuning**, W5-23,
LH_Prototype_v1, 2026-10-11, source URL null. They are not historical T4C values.
No mechanical research or Bible-derived numbers are asserted.

## Rebuild and material contract

See [the exact commands](../../artsource/props/README.md). Blender **5.2.2 LTS,
`d13f752e3b9c`**, fresh process, `--background --factory-startup --threads 8`,
checkout-local `XDG_CONFIG_HOME=…/artsource/props/.config`. Construction and
Cycles seed: **523**. Build, then render icons, then validate. A new build clears
the icon entries in the manifest so stale renders cannot pass full validation.
Run builds serially. `--output` supports a separate ignored repeat directory.

Output is `artsource/props/output/`. GLBs have embedded geometry and **external
shared images**, so preserve the `textures/` sibling directory. Every mesh has
identity object transforms and a canonical zero origin; glTF transforms Blender
metres `(X,Y,Z)` into `(X,Z,-Y)`. Convert metres to centimetres exactly once in the
Unreal importer and follow the existing player import's handedness convention.
All visuals should have no collision or navigation contribution.

`Props.WornAtlas` is one opaque, two-sided material shared by the ordinary
surfaces: worn iron/highlight/rust, ashwood, leather, linen/thread, parchment,
charcoal cloth, old bone, blue glazed ceramic, dry membrane, stone, brass, soot
and sealing wax. Each surface uses a padded cell of one original 4×4 palette
atlas. Base pigment and normal maps are actual CPU Cycles bakes of procedural
nodes. **No photo, concept pixel, scan, external texture or asset pack is used.**
The 256px base map is sRGB; the tangent normal and ORM maps are linear data.
Normal convention is glTF/OpenGL +Y. ORM has **R=1 neutral AO**, per-pigment
roughness in G and metalness in B; it is not a per-object occlusion bake. Do not
bake the studio lighting into the Unreal material. Fine atlas noise and small
stitches are expendable at gameplay distance. Review mip bleeding across the
palette cells when setting the Unreal texture group.

Effect materials are texture-free Principled glTF materials with emission and,
where specified, alpha blending. Translate them into suitable Unreal material
instances, explicitly checking two-sided translucent rendering and sorting.
Emissive material alone does not promise useful world illumination in Unreal.
No light actors, Niagara systems or effect timelines are exported.

## Importer asset table

Sizes below are measured **X×Y×Z bounding extents in centimetres**, rounded to
0.1cm. The manifest retains exact source-space minima/maxima, node/material
names, SHA-256 values and bytes. `ground` means an inventory/world-display
origin, not a clothing rig or an equipment socket. Ordinary clothes are static
pickup/icon meshes; they do not replace the player's skinned outfit.

| File stem / manifest suffix | Attachment / pivot / axes | Size cm | Triangles | GLB bytes | Surface intent |
|---|---|---:|---:|---:|---|
| RustedDirk | Socket.Weapon.R; grip zero, blade +X | 58.0 × 5.8 × 19.5 | 1,364 | 131,424 | Iron/edge/rust, bound leather |
| AshwoodFlatbow | Socket.Bow.L; grip zero, limbs ±Z | 72.2 × 8.5 × 145.1 | 1,798 | 136,708 | Ashwood, flax string, horn tips |
| WoodenArrows | Socket.Quiver.Back; strap anchor zero, +Z | 17.9 × 17.9 × 81.0 | 1,170 | 102,316 | Leather, wood, pale feathers |
| Torch | Socket.Bow.L (proposed); grip zero, haft +Z | 13.5 × 12.3 × 76.5 | 572 | 53,936 | Charred wood/iron + Ember/Core |
| PotionOfMana | Ground; bottle base, upright +Z | 16.4 × 12.9 × 26.0 | 898 | 69,224 | Opaque blue glaze, pewter, wax |
| ClothVest | Ground; lower hem; front +X | 30.5 × 47.5 × 49.0 | 974 | 78,888 | Worn flax, leather mending |
| ClothPants | Ground; cuffs; front +X | 22.9 × 40.0 × 69.6 | 1,214 | 74,660 | Worn flax, leather ties |
| LightHeal | Ground; lower roll center; face +X | 5.4 × 25.4 × 33.0 | 236 | 25,808 | Parchment, ink, brass/wax |
| DecayingBatWings | Ground; tied-wing center; span ±Y | 24.3 × 60.5 × 9.5 | 290 | 35,540 | Dry membrane, bone, cord |
| GoblinLeatherArmor | Ground; lower hem; front +X | 30.5 × 60.1 × 52.1 | 1,106 | 96,820 | Leather, iron scales, horn |
| IronRing | Ground; ring bottom; signet +Z | 3.8 × 1.1 × 4.2 | 352 | 26,776 | Worn iron, pale signet |
| GoblinBlade | Socket.Weapon.R; grip zero, blade +X | 80.7 × 5.8 × 27.7 | 1,320 | 121,896 | Iron/edge, rust, wrapped grip |
| IronKey | Ground; tooth end; shank +Z | 9.4 × 3.5 × 23.9 | 500 | 38,808 | Iron, bright teeth, rust collar |
| FlowingBlackRobe | Ground; hem; hood face +X | 42.1 × 116.9 × 140.0 | 1,454 | 107,584 | Charcoal cloth, flax piping |
| LootSack | Ground center; gathered top +Z | 46.2 × 40.0 × 40.0 | 812 | 61,212 | Flax, rope, leather patch |
| CorpseLootBadge | Ground below diamond; upright +Z | 35.7 × 35.7 × 72.0 | 296 | 20,868 | LootSilver + brass ring |
| ChestClosed | Floor center; latch +X | 106.6 × 71.5 × 68.9 | 2,800 | 312,456 | Ashwood, iron straps/rivets |
| ChestOpen | Same floor center; latch/opening +X | 137.2 × 71.5 × 150.6 | 2,800 | 315,340 | Same atlas; dark empty interior |
| FireDart | Projectile body origin; travel +X | 77.0 × 20.4 × 23.6 | 134 | 22,944 | Ember + FireCore |
| FireImpact | Impact origin; burst +Z | 75.1 × 70.8 × 40.8 | 254 | 42,200 | Ember + Spark + FireCore |
| StoneShard | Projectile center; travel +X | 39.4 × 14.6 × 18.0 | 40 | 6,952 | Opaque atlas stone/fracture |
| StoneShatter | Impact origin; shatter +Z | 66.2 × 42.4 × 37.9 | 160 | 25,724 | Opaque atlas stone |
| DustDevil | Ground origin; spin +Z | 63.3 × 64.0 × 78.5 | 448 | 72,092 | Dust + opaque stone grit |
| HealLight | Ground origin; rise +Z | 78.2 × 78.3 × 61.3 | 616 | 95,804 | HealGold |
| Light | Orb center; halo in XZ | 63.8 × 22.0 × 63.8 | 544 | 84,056 | LightPearl |
| MeleeSpark | Contact origin; orient +Z to hit normal | 75.1 × 45.0 × 40.8 | 176 | 29,232 | Spark + FireCore |
| SlashArc | Swing center; plane XY, bulge +X | 37.9 × 83.4 × 0.0 | 64 | 11,184 | Spark; two-sided |
| ArrowFlight | Nock zero; shaft/tip +X | 72.0 × 5.1 × 6.6 | 54 | 6,832 | Wood/bone/feathers/metal |
| ImpactPuff | Impact origin; rise +Z | 37.0 × 35.1 × 24.0 | 140 | 22,424 | Puff; translucent |

All pieces in a multi-node GLB retain their shared asset origin; do not recenter
them independently on import. Closed and open chests are distinct mesh assets,
each with `Body` and `Lid`. Both roots sit at the floor; the latch faces +X.
The open lid is baked at **−106° about source Y** around the hinge
`(-0.50, 0, 0.49)m`. It is a visual state, not a new persistent interactable.
The imported bounds slightly exceed I04's old proxy envelope because the rim,
rivets, hinge and lid are modeled: inspect wall clearance before placement.

`CorpseLootBadge` follows I05: diamond Z=6…78cm, with an equatorial ring within
X/Y ±17.84cm. The owning code decides whether a corpse has loot. Place at the
corpse front extent +24cm only on clear visible floor, otherwise suppress it and
retain the label. Empty/dead state, corpse name, cleanup and contents remain
authoritative; this art adds none of them. The sack is an optional visual proxy,
not an instruction to spawn extra loot or replace every corpse.

### Socket fitting

The existing source rig's axes are +X forward/+Z up; `Socket.Weapon.R` and
`Socket.Arrow` have source longitudinal +X, while `Socket.Bow.L` and
`Socket.Quiver.Back` have longitudinal +Z. Blender bone local +Y becomes the
imported Unreal bone's +Z. `LHPlayerVisual.cpp:137–152` already creates corrected
public sockets: Weapon/Arrow have `FRotator(90,0,0)`; Bow/Quiver use identity.
**Attach these canonical meshes to those corrected sockets without adding the
same correction again.** Public dotted names may bind underscore-sanitized
imported bone names. Keep the existing one-time handedness/metre conversion.

The bow exports `Body`, `String.Upper`, `String.Lower`, `NockedArrow`. In source
rest coordinates its string endpoints are `(-.145,0,±.72)m`, its string midpoint
is `(-.19,0,0)m`, and the arrow nock is `(-.19,-.035,0)m`. The item's icon hides
the nocked-arrow component; the bow-attack icon includes it. In a draw pose,
move the nocked arrow to `Socket.Arrow`, aligning its +X shaft with that socket;
compensate the baked nock offset. Reconstruct or transform each straight string
half from its limb tip to the current nock. The rig preview demonstrates this
using posed/rest bone matrices and midpoint deformation. These are static mesh
components, not an exported bow animation or a mechanically delayed shot.

`Torch` uses the left-hand `Socket.Bow.L` as a **proposed alternate attachment**,
with +Z along its haft and a separate `Flame` node. This does not add a torch
slot, authorize dual wielding with the bow, or change the catalog's equipability.
`WoodenArrows` has its strap anchor at zero, pouch bottom at −39cm and arrow
feathers above the opening. Preview fitting uses Body A at idle and bow draw;
Body B and all action frames still need the importer’s fit review.

## Effect and animation intent

Let `t` mean normalized **presentation** lifetime, from 0 to 1; these are suggested
visual envelopes, not durations, damage delays, travel speeds or new rules.
Bind to the existing committed/resolved attack and spell state, including misses
and cancellations. All pieces can use whole-component transforms; splitting
motes or shatter fragments further is optional importer work.

| Asset | Material / motion and fade intent |
|---|---|
| FireDart | +X travel; `Core` and `Flame`. Pale core with three orange trailing lobes. Preserve narrow transverse size; stretch X 0.7→1.1→0.7, fade final 30%. Root is trail body, nose at +19cm. |
| FireImpact | Origin at impact; orange/pale radial splinters. Scale 0.2→1 by t=.25, opacity/emission→0 by t=1. Offset along hit normal to avoid embedding the center in the floor. |
| StoneShard | +X travel, opaque atlas stone and pale fracture. Mild roll only; scale/fade on impact, no shader glow. Center origin. |
| StoneShatter | Eight unequal opaque fragments around impact origin. Scale 0.15→1; translate along normal and sink/scale to zero at end, or adopt a dithered material fade. |
| DustDevil | Ground origin, `Swirl.0/1/2` translucent dust ribbons and opaque `Grit`. Rotate around +Z, slight opposing band speeds; scale XY .5→1 and fade first/last 20%. |
| HealLight | Ground origin; `Ring`, `Rays`, `Motes`. Sage/gold annulus, paired rising rays and motes, no medical cross. Ring expands .65→1; motes rise at most .35m over t; fade final 35%. |
| Light | Orb-center origin; `Orb`, `Halo`. Pearl/cyan orb with four rays. Hold while the authoritative spell is active, subtle scale pulse .95–1.05; fade only when that state ends. |
| MeleeSpark | Contact origin, pale gold radial slivers. Brief scale .1→1 and fade; orient along hit normal. Blood-free. |
| SlashArc | Swing-center origin, flat XY crescent (Z extent zero intentionally), +X bulge. Two-sided. Rotate into the attack plane; expand .8→1 and fade, never use its geometry as collision/range. |
| ArrowFlight | Nock origin, shaft +X to tip +72cm. Same arrow model as bow/quiver; opaque, scale one. Translate only as cosmetic feedback; authoritative resolution timing stays unchanged. |
| ImpactPuff | Impact origin; seven low-poly translucent dust lobes. Scale .3→1.3, rise up to .12m, opacity .18→0. Blood-free; do not use as a persistent floor stain. |

| Material name | sRGB tint | Emission strength | Initial alpha |
|---|---|---:|---:|
| FX.Ember | `#DC661F` | 1.4 | .82 |
| FX.FireCore | `#FFC66C` | 2.3 | 1 |
| FX.HealGold | `#BFCB90` | 1.3 | .78 |
| FX.LightPearl | `#90C9D4` | 1.3 | .80 |
| FX.Dust | `#9B8C6A` | .22 | .45 |
| FX.Puff | `#9B8C6A` | 0 | .18 |
| FX.Spark | `#DCAA56` | 1.5 | .90 |
| FX.LootSilver | `#A9C4CC` | .60 | 1 |

These strengths are the Blender/glTF inputs, not calibrated Unreal exposure
values. Reduce emission together with alpha when fading; an opaque core needs a
material that supports fade or must shrink away. The corpse badge is stateful,
not a timed transient. No bloom is baked into the icon PNGs.

## Icons, bytes and budget

Fourteen `Item.*` and seven exact ability IDs are mapped in `preview.py` and the
manifest. **`Item.LightHeal` is the scroll; `Spell.HealLight` is the spell.**
Ability IDs are `Spell.Light`, `Spell.FireDart`, `Spell.HealLight`,
`Spell.StoneShard`, `Spell.DustDevil`, `Attack.Melee.Basic`, `Attack.Ranged.Bow`.
The melee icon combines the exported dirk and slash arc; all others use the
matching item/effect models. These are filenames/mappings, not registered new
presentation IDs or starter grants.

Every icon is **128×128, 8-bit RGBA, transparent background**. The same lights,
orthographic view direction, half-stop exposure and Cycles 32-sample settings
are used throughout. Thin objects are rotated in the image plane, then uniformly
framed with clear padding. Runtime thumbnails should use their alpha and retain
the full item/ability label; no frame, numbers or text are baked into the icons.
The sheet renders a 36px strip for inspection alongside the 128px set. This is
the task's rendered-icon direction, not a replacement for the style guide's
other attribute, input, navigation or status glyphs.

| Icon ID / PNG filename stem | Bytes |
|---|---:|
| `Item.RustedDirk` | 10,964 |
| `Item.AshwoodFlatbow` | 10,120 |
| `Item.WoodenArrows` | 10,715 |
| `Item.Torch` | 9,121 |
| `Item.PotionOfMana` | 13,031 |
| `Item.ClothVest` | 16,279 |
| `Item.ClothPants` | 13,194 |
| `Item.LightHeal` | 13,658 |
| `Item.DecayingBatWings` | 11,085 |
| `Item.GoblinLeatherArmor` | 14,661 |
| `Item.IronRing` | 12,772 |
| `Item.GoblinBlade` | 10,978 |
| `Item.IronKey` | 11,371 |
| `Item.FlowingBlackRobe` | 10,802 |
| `Spell.Light` | 10,898 |
| `Spell.FireDart` | 8,970 |
| `Spell.HealLight` | 10,373 |
| `Spell.StoneShard` | 10,549 |
| `Spell.DustDevil` | 12,401 |
| `Attack.Melee.Basic` | 9,609 |
| `Attack.Ranged.Bow` | 11,058 |

Measured geometry: **22,586 triangles** across all 29 files
(including both chest states and duplicated arrow geometry), **2,229,708 GLB bytes**.
Shared textures: base **94,375**, normal **2,957**,
ORM **2,002** bytes; total **99,334**. Icons: **242,609** bytes.
Combined mesh + shared texture + icon payload: **2,571,651 bytes
(2.572 MB)**, below the 6,000,000-byte source-delivery target.

This is an observed **export payload**, not a measured `.uasset`, cooked or GPU
memory total. Unreal texture compression, mesh settings, material instances and
metadata determine the eventual imported size. Shared resources must be reused,
not copied per mesh. The single atlas material keeps ordinary props at one
material slot per mesh component. FX use a few additional slots and have no
texture payload. Evidence renders, manifest and logs are excluded from the
runtime payload; final output and worker evidence both remain under 40 MiB.


## Render review and remaining weaknesses

Inspected the supplied `starter-characters-and-creatures-concept.png` and
`deep-dungeon-creatures-concept.png` at the external plan package, via
`artsource/concepts/README.md`. They were visual targets only. The original
meshes use worn flax/leather, muted rust, narrow metal edges, small warm brass
accents, and reserved blue/sage spell colors to fit that family.

Feedback loop: the initial item sheet showed capped/bucket-like garments,
faceted round surfaces, similar dirk/goblin silhouettes and an open chest
needing clearer structure. Revisions opened and shaped garment neck/waist
edges, added selective smooth normals, gave the goblin weapon a hooked cleaver
silhouette, added chest rim/hinges/feet, and improved the robe's contrasting
binding. Effects were reframed to show every caption, fire saturation increased,
and the impact puff's opacity reduced. The icons gained consistent exposure
and diagonal framing for thin weapons. The first item sheet is retained for
comparison. A rig compositor attempt failed because Blender 5.2 exposed an
empty Render Result pixel buffer; it now uses temporary linear EXRs and removes
them after assembly. Two earlier concurrent sheet processes ended with signal
143 without a diagnostic; final review renders were run serially.

Final visual assessment: **coherent stylized candidate, moderate correspondence
to the painted concepts**, not a reproduction of their surface richness.
The potion, scroll, sack, chest and major spell silhouettes read clearly. Cloth
folds and bat membranes remain simplified; metal rust patches are visibly
constructed, chest boards remain regular, the robe is dark, and tiny weapon/
string detail is weak at 36px. The puff is still visibly faceted rather than
soft smoke. Emissive pieces have solid stylized cores rather than sophisticated
volumetric falloff. No animation, in-engine bloom, translucency sorting, UI
background variations or packaged performance has been judged here.

The gameplay sheet shows actual exported props on the existing **unbaked source
player rig**, using its idle/draw poses and the existing dark stone/warm-light
stage. Four 800×500 panels use a 12m, 55° pitch, 45° FOV perspective camera;
three use 45° yaw and the quiver panel reverses it. This is Blender socket/scale
evidence, not a runtime capture, and tests one body and two poses only.

## Checks and handoff

- Blender construction/export completed for all **29 GLBs**, with three real
  **256×256 CPU Cycles** atlas bakes. Final geometry totals **22,586 triangles**.
- Standard-library validator passed **29 meshes, 21 icons, 3 textures**,
  **2,571,651 runtime payload bytes**. It checks exact ID/file inventory, hashes,
  GLB headers/chunks/accessor ranges, finite unit normals, indexed nondegenerate
  triangles, identity transforms, source-axis bounds and manifest counts,
  atlas bindings, FX alpha/emission, PNG CRC/decompression/dimensions/alpha,
  and the 6MB/40MiB/30-PNG budgets.
- Blender re-import validation passed all **29 exported files**, comparing
  component names, triangle counts, identity roots/pivots, source-space bounds,
  materials and actual loaded 256px images. This ran again after the chest feet
  were added; it is not just validation of the first draft.
- A fresh-process rebuild and independent icon render matched **all 53 runtime
  files byte-for-byte** (29 GLBs + 3 atlas images + 21 icons) on this host/pinned
  Blender build. The first comparison exposed varying PNG timestamps/timing
  chunks; pixel chunks matched. The renderer now strips ancillary timestamp/
  text/EXIF metadata while retaining pixel/color-profile chunks unchanged, and
  the subsequent full binary comparison passed. Cross-version/hardware byte
  identity is not promised. Repeat payload was removed after comparison to keep
  the retained PNG count at **29**, including the first item sheet.
- Validator negative controls on copied artifacts rejected transformed nodes,
  external image URIs outside the atlas, transparent ordinary atlas materials,
  degenerate triangles, truncated GLBs and corrupted PNG signatures.
- Python syntax compilation and Git whitespace/scope checks passed. Final item,
  effect, icon and gameplay-rig sheets were rendered with **Cycles CPU** and
  inspected; 21 icon files have transparent and visible pixels. Blender emits
  expected 6.0 `use_nodes` deprecation notices and a shared ORM sampler warning;
  all references use matching linear sampling, and re-import passed.

Evidence is retained in this attempt's worker-output `library/`: the five
review PNGs (four final sheets plus `items-first.png`), `export/` with the complete
importable manifest/GLB/shared-texture/icon payload, and validation/comparison/
re-import summaries. `report.md` contains the required task handoff. Generated
meshes, images and renders are not Git assets. The ignored checkout output also
contains trimmed-size diagnostic logs; there are no retained `.blend` files.


Checks not run and concrete prerequisite: Unreal import, texture/material
translation, code binding, animation over complete action cycles, Body B fit,
actual inventory/HUD contrast, collision/navigation regressions, editor build,
cook/package/play and GPU/translucency cost. The separately assigned codex-sol
import/binding task must implement those changes outside this task's owned
paths before they can be tested. Windows packaging remains deferred by owner
decision. No Unreal gate is claimed passed.

Next task: import the manifest's meshes and shared images, bind catalog IDs,
preserve the corrected sockets and authoritative effect timing, add the icon
lookup, and review the result in the actual Linux build. The integrator owns
binary destinations/LFS, runtime registration and acceptance. No push, merge,
worker-capacity change or verified-task-success declaration is made here.
