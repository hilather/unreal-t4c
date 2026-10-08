# A-02 — Church and basement environment kit

Production handoff candidate for **W5-01**, dated 2026-10-08, against `cd6e73b139e117f779e1aafa5c4d4bdf45f6ecec`. This is a text specification, not an imported kit or dressed map. W5-01 depends on the G3 layout freeze; preparation here does not open that gate. W5-05's assigned map owners place and light the kit serially per map. No map, shared header, binary asset or asset lease is changed by A-02.

Use [module dimensions and assembly](modules.md) for modeling, [opened references and rights](references.md) for evidence, and the [player spec](../player/README.md) and [A-01 creatures](../creatures/README.md) as scale rulers. Governing documents are [art pipeline](../../../plan/docs/04-art-pipeline.md), [game design](../../../plan/docs/01-game-design.md), [art register](../../art-register.md), [controls](../../controls.md), [layout conventions](../../layout/README.md) and each [hub](../../layout/hub-temple-district.md), [B1](../../layout/b1.md), [B2](../../layout/b2.md), [B3](../../layout/b3.md), [B4](../../layout/b4.md) file.

**Every design number in this package is a Prototype proposal (P)**: dimensions, offsets, counts, colors, ratios, material parameters, LODs, texture budgets and lighting starts. Provenance for new values is `A-02 / LH_Prototype_v1 / 2026-10-08 / source_url: null`. W0-04/V-01/A-01/controls values remain **inherited P**; listing them does not authorize resizing their layout or actors. Dates, image metadata, source labels and asset IDs are evidence identifiers, not design tuning. No source-tile-to-centimetre conversion is established.

## Visual direction and reconstruction boundary

Build a modest town church and a repeated masonry basement kit. Warm brown stone, gray church paving, rough dark timber, aged pale plaster, a red nave aisle and selective amber light follow the [art register](../../art-register.md#palette-and-materials). Keep broad readable masonry faces with restrained bevels and edge rubble. The temple must not become a tall gothic cathedral, and the basement must not become the separately labelled cemetery crypt.

| Source-evidenced observation from images opened by A-02 | Reconstruction still required |
|---|---|
| Town church has gray paving, red aisle, bench-like furnishings and an image-left Dungeon wing; Samaritan label is outside | Exact footprint, wall heights, entrances, altar function, roof, all meshes and NPC positions use the V-01 candidate and remain P. |
| B1 has a central room with separate entry, healer and onward-stair branches | Corridor widths, stair direction, safe volumes and doorway cuts are authored, not recovered collision. Preserve the proposed graph while reviewing against the source. |
| B1–B3 repeat brown masonry/floors, rubble, pale patches and dark red stains | Exact stone courses, tile size, wear, material roughness and intentional floor-specific dressing themes below are P. |
| B2 shows ring/branches and B3 a partitioned circuit-like plan | Internal cuts, usable void edges and source passability remain uncertain; do not turn black voids into shortcuts. |
| B4 appears darker, has amber wall lights and a Balork-labelled complex with a raised fixture | Boss-scale clearances, internal right-side loop, altar dimensions/function, lighting levels and navigation extents are P. Pale marks do not license new skeleton enemies. |
| Environment concept shows stone/timber/slate-like roofs and a torchlit basement | Roof openings, dock, boats, stalls, extra stairs, barrels and cobweb placements are concept proposals, not required map features. |

No new encounter, chest content, trap, gate, boss phase, portal or service is authored by decoration. Source item-legend markers are not decals to place in the game. If a source image and graybox differ, retain the explicit uncertainty and send the layout owner a concrete correction proposal; do not silently move an entrance to make a module fit.

## Presentation records and ownership

Propose case-canonical IDs `Presentation.Environment.<Family>.<Piece>`, independent of mesh path. The [module catalog](modules.md) gives suffixes and dimensions. These are handoff identifiers, not new `Area.*`, entrance IDs, entity GUIDs or runtime contracts. Current generic `ULHDefinition::Visual` does not implement an environment-kit registry; ordinary static modules can be directly referenced by owned maps after review. Gameplay-interactive doors/props require the integrator's binding and persistent identity, never an ID derived from this mesh name.

Future folders: `/Game/Lighthaven/Art/Environment/Church/`, `/Basement/`, `/Shared/`; source files eventually under lowercase `artsource/`. Proposed names `SM_LH_<Piece>`, `M_LH_Environment`, `MI_LH_<Surface>`, with collision named/documented by the adopted import workflow. Shared opaque master and trim textures have a single assigned writer. The map owner owns roof/wall cutaway, light actors and final prop placements; W5-01 supplies meshes, material instances, bounds, simple collision and placement references only. A proof assembly can be assigned later without this task writing a `.umap`.

## Floor differentiation using the same kit

All themes and their placements are **P**, informed by the observations above. Reuse the same stone/trim/floor assets; alter broad wear masks, fixture selection and restrained prop clusters. A floor must remain identifiable through layout and landmark shape in grayscale, not hue alone. Counts/intensities of dressing are deliberately not inferred from source pixels.

| Area | Recognizable kit treatment | Protected landmark / placement rules |
|---|---|---|
| Hub church | Gray slab floor, red woven aisle, pale plaster over warm stone, dark timber pews and low altar; exterior slate-like pitched roof and timber edge | Follow nave/wing plan. Red aisle remains X[6,10], Y[0,21] U; keep cross-aisles and named service pads empty. Roof eave 400 cm/ridge 600 cm is inherited P. Fade roof on interior use. |
| B1 | Dry tan/brown stone and worn floor slabs; sparse low edge rubble; clean threshold and simple wall sconce at healer branch | Keep three hub arms visible and use a flush worn-stone center motif, not a blocking pillar. Entry, healer branch and descent enclosure retain safe lanes. No added doors/locks. |
| B2 | Same brown stone with restrained darker lower-wall damp bands, a contrasting plain coping around ring void, occasional side-bay storage props | Ring and hooked side branch are the identity; leave full ring lanes and dogleg turn pads clear. Damp bands are a design extension, not an original measured material. Do not add a bridge across the void/gap. |
| B3 | Same masonry with broader pale mineral/repair patches on wall faces, repeated capped partition ends, fewer loose containers | Use flush floor wear strips to distinguish return circuit, branch end and descent arm; keep all baffle openings. No extra chest or stair in WestBay. Original pale floor patches remain ambiguous; no creature identity inferred. |
| B4 | Dark gray-brown instance of shared stone, stronger amber fixture rhythm, pale flat rubble scatter and restrained dark red stain decals; raised stone fixture as focal mass | Keep Balork's wings/torso separate from background through cool fill and value, not a red room wash. All boss door/turn pads and altar circulation stay clear; no arena lock or forced cinematic camera. |

For all floors, rubble/cobwebs/stains are fixed authored decoration, not interactive loot or damage surfaces. In corridors, stairs, turns, arrival boxes, combat lanes and NPC approaches use flush nonblocking marks only. Larger barrels, benches or pillars belong in approved alcoves/room edges **outside** the reserved circulation and visual sweep envelope. Turning collision off does not excuse a visibly impassable heap.

## Materials and palette

Starting swatches below are **P unlit sRGB base colors**, not samples recovered from the maps or final lit appearance. Roughness/metallic are P material-authoring starts. Share UV tiling and trim widths across variants; broad dirt/wear masks may vary by floor without multiplying unique texture sets. Do not bake torch highlights or source map labels into albedo.

| Surface / proposed instance | Base color | Roughness / metallic | Use |
|---|---|---|---|
| Warm masonry / `MI_LH_StoneWarm` | `#78634B` | 0.85 / 0 | B1–B3 walls, modest exterior base |
| Dark masonry / `MI_LH_StoneDeep` | `#514B42` | 0.85 / 0 | B4 instance of same texture set; keep readable floor/edge values |
| Gray church paving / `MI_LH_Paving` | `#77766D` | 0.8 / 0 | Church floor and thresholds |
| Basement floor / `MI_LH_FloorWarm` | `#695640` | 0.85 / 0 | Large readable stone slabs, same scale at all modules |
| Rough timber / `MI_LH_Timber` | `#43352A` | 0.8 / 0 | Beams, pews, doors, prop boards |
| Aged plaster / `MI_LH_Plaster` | `#B2A58A` | 0.9 / 0 | Interior/exterior upper wall faces |
| Slate-like roof / `MI_LH_Roof` | `#4C5356` | 0.75 / 0 | Reconstructed low church roof; not reflective metal |
| Woven aisle / `MI_LH_AisleRed` | `#733D35` | 0.9 / 0 | Source-supported landmark hue, reconstructed weave |
| Dull iron / `MI_LH_Iron` | `#535451` | 0.65 / 1 | Sconces, straps and hinges only |
| Pale mineral/rubble accent | `#9B9484` | 0.9 / 0 | Sparse broad value change; avoid noise across targets |

Default opaque shading for walls, floors, roofs and props. Masked cobweb cards are optional and sparse, never stacked in front of a target. No translucent water/roof requirement enters the minimum kit. Render-facing occlusion may use a reviewed fade/cutaway material, but collision/LOS/navigation remain in the world model. Pack channels consistently (proposal: R ambient occlusion, G roughness, B metallic); textures generated from concepts are not automatically authored PBR data. Use real authored normals; choose texture linear/sRGB import settings per channel and record them during actual import.

## Lighting and camera-facing geometry

Review with controls' inherited P focus ≈90 cm, boom 1200 cm (900–1800), pitch −55° (−60…−50), yaw 45° (0…90), **45° horizontal FOV**, at 1280×720 and 1920×1080. Do not use the earlier unadopted vertical-FOV proposal. Include the collision-shortened camera at walls and ceilings. Light/exposure levels are not historically known and have not been profiled.

Fixture meshes are separate from light actors. P starting references: amber flame around 2200 K, neutral/cool ambient around 6500 K; no numeric lux, lumen, exposure or attenuation value is asserted as ready. Map owner sets these in an actual scene, records the settings, and reviews both normal and lower-cost lighting. A small emissive flame remains visible when a costly shadow light is disabled. Proposed ceiling is ≤4 overlapping local lights and ≤2 shadow-casting local lights on a player at once; these are profiling targets, not observed performance. No dynamic light per ember or decorative candle.

Use wall-mounted sconces and recessed light niches outside the clear route prism. Light the B1 healer detour, B2 ring/return landmarks, B3 circuit and descent tip, B4 widened boss approach, and hub nave/wing thresholds at the positions owned by the layout files. Do not replace a navigation cue with a bright floor obstacle. The boss should remain distinguishable without making all masonry emissive.

Split roof from walls. Keep full structural collision and Visibility blocking on walls and closed void boundaries; fade/cut away render geometry only. No new click-through-wall or attack-through-wall rule follows a visual fade. Roof render children can be nonblocking where the approved graybox has no roof collider; never add a continuous camera-blocking roof in the art pass without host review. Wall fading must not remove the spring-arm's existing collision behavior. Broad trims, beams and fixtures should share their wall's visual cutaway state so floating ornaments do not obscure targets after the wall disappears. Current controls supply boom collision, not a completed fade system.

## Collision, navigation and creature clearance

Use boxes for straight walls, jambs, columns and large props; a few convex pieces for corners and smooth stair-ramp collision. No complex render-mesh collision for routine navigation. Floors are flat with continuous top surfaces; cracks, carpet, thresholds and decals introduce no step lip. Do not let texture/LOD changes modify collision. Keep intentional voids blocked with actual boundary geometry, not black paint.

Decorative props in circulation have collision disabled **and** do not affect navigation, then remain visually outside required clearances. Solid room-edge furnishings use a simple blocker matching their visible footprint and receive nav testing from the map owner. Avoid separate protruding Visibility blockers on tiny decoration; target traces must correspond to the reviewed structural/interactive geometry. Cosmetic torch flames never block target selection. Any hazard or interactive collider is a gameplay-owner addition.

| Inherited P ruler | Required unobstructed route after dressing | Implication |
|---|---|---|
| Player R35/HH90, standing art 175 cm | Common 240×300 cm minimum door, 320 cm corridor, 300 cm stair, 320² cm turn pad | Capsule is not resized by appearance. Review bows, quivers, robe and death poses separately. |
| Giant Bat / Giant Spider span 170 cm; Atrocity width 180 cm, animated top 210 cm, turn diameter 262 cm | Common routes above | Atrocity fits a 240 cm aperture aligned but cannot finish a 262 cm turn inside it; keep adjacent pads empty. |
| Balork transit 440 W×200 L×310 H cm; R100/HH155; turn diameter 484 cm; polearm about 400 cm carried within envelope | B4 C04 and D05–D07 doors **500×360 cm**, corridor **550 cm**, pads **600×600 cm** | Door arithmetic leaves 30 cm each side and 50 cm above visible envelope when aligned; 600−484=116 cm turning-diameter allowance. Arithmetic only, not a swept validation. |

Balork stays within Arena/Reliquary/Ossuary plus C04 and the specified Hall apron. No boss transit is required in C01–C03, side chambers or the B4 return stair. Do not narrow wings or assume a folded pose to use ordinary routes. A-01's optional 520 cm attack sweep fails a 500 cm doorway; compact jab is the required doorway presentation. No boss stair needs production in this kit; a future wide stair would require the inherited 550 cm width and 600 cm landing plus an explicitly changed route task.

All headroom is measured above the local ramp/tread at that point. Frames, arches, braces, sconces, wing tips and weapons count toward visual clearance. Keep safe arrival boxes at least 600×600 cm where the layout calls for them; they are not clutter/storage space. Visual/capsule arithmetic alone proves neither AI navigation, player passing, combat safety, attack reach, navmesh bake nor controller targeting.

## Texture, mesh and material budget

All targets are **P** and must be measured on the intended Linux hardware at actual camera distance. The ceilings count all parts of an assembled module, not each child independently.

| Asset group | LOD0 / LOD1 / LOD2 triangle target | Material sections |
|---|---|---|
| Floor or straight wall module | ≤2k / 800 / 200 | ≤1 |
| Doorway, arch or corner assembly | ≤4k / 1.6k / 400 | ≤2 |
| Stair flight, including decorative treads | ≤6k / 2.4k / 600 | ≤2 |
| Pillar, altar or pew | ≤4k / 1.6k / 400 | ≤2 |
| Roof slope/gable section | ≤3k / 1.2k / 300 | ≤2 |
| Small prop/fixture | ≤2k / 800 / 200 | ≤2; flame effect accounted separately |

Shared surface allocation: masonry 2048², floor/paving atlas 2048², timber/plaster trim atlas 2048², roof/red-cloth atlas 2048², prop/fixture atlas 1024²; each is base color + normal + packed masks. Floor variants use tint/masks, not duplicated sets. Optional shared stain/decal atlas ≤1024²; optional flame strip ≤512². Texture budget covers a complete first kit, not that many unique sets per floor or object. No 4K unique wall/floor textures. UV target 256 px/m for tileables and props; review 512 px/m only for altar/close interaction surfaces inside existing atlases. Trim bevels and UV edges must survive mip reduction.

Texture sizes are authored resolution targets, not a measured VRAM guarantee. Verify actual imported compression, mip residency and texture streaming alongside geometry/material/shadow cost. Start with conventional LODs; no required dependency on Nanite, Lumen, virtual textures or a new plugin. Calibrate distance/screen-size transitions in-engine. Preserve silhouette, opening boundary and collision at every LOD; a low-detail arch must not visually close its opening.

## Placeholder, replacement and acceptance

Next-wave placeholder: project-authored boxes/planes/ramps with flat named material colors, doors represented by actual open cuts and stairs by the exact ramp profiles. No reference map texture, source billboard or fake `.uasset` is needed. Replace each proxy module with the authored module at the same pivots/bounds, keeping actor/map/entrance/encounter identities. Layout owners, not the kit artist, adopt replacements in maps.

Before production acceptance, build a kit test assembly with straight/corner/T seams, every opening, every ramp profile and both human/creature rulers. Inspect floor joints and shared partitions, measure post-dressing clearances, then walk each used route in both directions. Show the biggest resident turning and passing the player, not only a capsule fitting in a doorway. Test B4 boss approach, all complex thresholds, altar bypasses and Hall retreat boundaries; existing possible ranged safe spots remain gameplay review work.

Review church roof/cutaway, thresholds and light fixtures at every camera limit; compare full-floor silhouettes against the opened maps without copying source pixels. Inspect floor-specific themes and creature/target-ring visibility in grayscale and warm/cool light. Preserve hub service pads, portal arrival zones, narrow baffles and blocked voids. Then run Linux import/collision/nav/editor/play/cook/package and performance checks, with a cooked-content rights/soft-reference audit. Windows remains deferred. Those checks require actual assets, assigned map changes, presentation systems and a non-root Unreal runtime; **none ran in A-02**. The specs pass no build or art gate.
