# W5-12 scripted starter players

Original Blender source for two clothed humanoids, reviewed against the **left half** of the plan's `starter-characters-and-creatures-concept.png`. The image is a visual target only; these scripts never load it, sample its pixels, or use downloaded assets. All new measurements, materials, animation poses and lighting are **Prototype presentation tuning**, W5-12, LH_Prototype_v1, source URL null. This is Blender delivery, not an Unreal replacement or a gate pass.

Use Blender **5.2.2 LTS** (`d13f752e3b9c`) from the checkout root:

```sh
mkdir -p artsource/player/.config
XDG_CONFIG_HOME="$PWD/artsource/player/.config" \
 /home/brewerm/Downloads/blender-5.2.2-linux-x64/blender \
 --background --factory-startup --threads 8 --python-exit-code 1 \
 --python artsource/player/build.py
python artsource/player/validate_glb.py > artsource/player/output/glb-validation.json
XDG_CONFIG_HOME="$PWD/artsource/player/.config" \
 /home/brewerm/Downloads/blender-5.2.2-linux-x64/blender \
 --background --factory-startup --threads 8 --python-exit-code 1 \
 --python artsource/player/validate_blender.py
```

Build serially; `build.py -- --only player_a` or `player_b` builds one asset. `--geometry-only` constructs/rigs/animates without a texture bake or export. Builds overwrite ignored `output/`. Fixed construction and Cycles seed: 51212. Voxel unions and reduction are applied during construction, not exported modifiers. Floating-point/render differences across Blender versions or hardware are not promised bit-identical.

For final evidence run these commands through the same Blender prefix:

```text
--python artsource/player/preview.py -- --only player_a --view hero
--python artsource/player/preview.py -- --only player_b --view hero
--python artsource/player/preview.py -- --only player_a --view gameplay
--python artsource/player/preview.py -- --only player_b --view gameplay
--python artsource/player/sheets.py -- --view actions
--python artsource/player/sheets.py -- --view appearances
```

Seven final evidence PNGs, 1280×720, CPU Cycles 16 samples plus denoise. Hero views use studio lamps; gameplay uses the creature pipeline's 12 m, 55° pitch, 45° yaw, 45° FOV camera, dark procedural stone and warm 650 W point light plus restrained fill. These are Blender review conditions, not Unreal captures. Sheets freeze **only skinned meshes**, excluding imported bone display shapes. `preview.py -- --draft` builds an unbaked source preview for iteration; it is not export evidence.

## Appearance and runtime binding contract

The source authority is `Source/Lighthaven/Character/LHCharacterAuthority.cpp:128–158`, not the older proxy's omissions. There are **12 valid combinations**, not 24: Body A must use Face A, Body B must use Face B; missing face values are normalized. The only outfit is StarterLinen. Both bodies accept Cropped/Tied hair and LightWarm/MediumWarm/DeepWarm skin. `appearances.json` enumerates all twelve and the actual presentation IDs.

Each GLB contains the clothed `body` mesh, `Hair.Cropped`, and `Hair.Tied`, with one 22-bone armature. **Hide the unused hairstyle immediately after import**; glTF has no runtime appearance-selection logic here. Faces belong to their matching body; there is no independently permitted face swap. A single mesh per body serves all six choices. Hair is fitted to each body from common source construction, not six duplicated body exports.

Two material slots per body use the **same three 512² images**: `ClothingAtlas` and `SkinTint`. The latter multiplies the neutral skin areas of the base texture by a linearized sRGB factor; it also tints lips, but not eyes, hair, fabric or metal. Exact source palette: LightWarm `BD8E72`, MediumWarm `8B5A40`, DeepWarm `51362C`. Default factor is LightWarm. Reuse material instances for tones; do not create extra texture sets. Body A/B use separate three-image atlases because their cloth color and UV packing differ. Both are within the task's 512px limit.

Base pigment is baked through emission without scene lighting. Normal is an actual tangent-space Cycles bake of the procedural surface detail. ORM is actual Cycles geometric AO in red (distance 7.5 cm, 32 AO samples), roughness green, metallic blue. The same ORM image is wired into glTF occlusion and metallic/roughness. Alternate Tied hair is displaced during the bake to avoid overlap with Cropped; its AO contains self-occlusion but omits head-contact occlusion. The body is baked with Cropped hair in place. All materials are opaque; no hair cards with transparency, physical skin shader or cloth simulation.

## Coordinates, sockets and actions

Source geometry uses metres, +X forward, +Z up, identity object transforms and a fixed root at the floor. GLB applies its standard Y-up conversion. Unreal import must convert metres to centimetres **once** and inspect facing/up before binding. Place the visual floor root at capsule base (current capsule radius 35 cm, half-height 90 cm); do not inherit the old procedural torso/death rotation logic on top of the skeletal pose. This task does not diagnose or fix the old component's lying-flat capture in Unreal.

Both armatures have identical names/hierarchy but body-specific bind lengths. Preserve each bind pose, or explicitly retarget; do not silently substitute A's inverse bind matrices on B. Exported sockets are bones, not Unreal socket assets:

| Bone | Parent | Purpose / local bone +Y axis in rest |
|---|---|---|
| `Socket.Weapon.R` | `hand.R` | Right grip; +X shaft/blade direction |
| `Socket.Bow.L` | `hand.L` | Left grip; +Z bow long axis |
| `Socket.Arrow` | `hand.R` | Nocked arrow; +X shaft direction |
| `Socket.Quiver.Back` | `chest` | Back quiver; +Z long axis |

The bow and arrow socket axes are compensated in the draw pose. Equipment is not part of the player meshes. Bind the existing RustedDirk/AshwoodFlatbow/WoodenArrows and fallback blade/staff presentations separately. Source proxy lengths are dirk 33 cm, blade 72 cm, staff 145 cm, bow about 140 cm, arrow 60 cm. `Presentation.Player.BowArrow` visibility remains under the existing committed-bow-action condition; no baked visibility animation or projectile simulation is supplied. Grip offsets and equipment orientations still require actual attached-weapon review in Unreal.

Eight NLA clips, sampled at 30 fps:

| Clip | End frame / seconds | Visual anchor |
|---|---|---|
| idle | 60 / 2.0 | Upright breathing; exact loop endpoints |
| move | 30 / 1.0 | In-place walk; exact loop endpoints |
| run | 24 / 0.8 | In-place run; exact loop endpoints |
| melee | 24 / 0.8 | Contact frame 15 / 0.5 s |
| bow | 30 / 1.0 | Release frame 21 / 0.7 s |
| cast | 30 / 1.0 | Release frame 21 / 0.7 s |
| hit | 6 / 0.2 | Recoil peak frame 2 |
| death | 30 / 1.0 | Fall finishes on first integer sample after 0.65 s; hold 21–30 |

**The current production ability catalog has zero impact delay and a 1.5 s cooldown.** The above windups are optional visual lead-ins, not gameplay timing evidence. With current instant attacks, start at the contact/release anchor and fit the remaining recovery to the component's 0.18 s release window; play a windup only if a future event supplies positive `ImpactSeconds`. Never defer damage to these animation keys. Hit matches the existing 0.2 s window. Death samples the existing 0.65 s fall at 30 fps. Walk/run speed fit and start/stop blending are import work; default loops do not prove zero foot sliding at the current 220/450 cm/s movement speeds.

## Validation and budget

`validate_glb.py` adapts the creature pipeline's independent stdlib GLB/PNG decoder. It checks triangle budget, actual skin bindings, weights/normals/tangents, exact clip set/durations, constant root, three loop endpoints, death hold, sockets, texture dimensions/wiring and meaningful AO variation **inside occupied exported UVs**. No constant-white AO stand-in passes that check.

`validate_blender.py` separately imports the exports, samples all 242 integer frames per body with both hairstyle vertex sets, checks floor clearance and upright idle, measures capsule-relative bounds, loops, root and death hold, and constructs source geometry twice to hash vertices/topology/UVs/weights/bones. Root equality is exact; death hold permits 1 micrometre of imported interpolation noise. These checks do not certify skinning anatomy, all subframes, self-intersections, attached weapons, transitions or Unreal behavior.

Budget plan: six shared-within-body 512² textures, 2 bodies and 4 fitted hair pieces, 2 armatures, 16 short clips, 6 tint instances. Expected texture payload with BC7/BC5 and full mip chains is about 2 MiB; mesh/animation and Unreal serialization remain to be measured. Deduplicate atlas references on import; importing one texture copy per material or per appearance defeats the plan. The task's ~10 MB cooked budget is an **estimate pending Unreal import/cook**, not a measured engine result.
