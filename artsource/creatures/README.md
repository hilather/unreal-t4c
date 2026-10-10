# Scripted creature polish

Original geometry and procedural materials are authored reproducibly with headless Blender. The plan concept is a visual comparison target only; no image is read into the generation pipeline. `models.py` builds the rat and slime, with the bat in `bat_polish.py`; `build.py` handles the bake, rigs, five actions, exports and renders. Fixed per-creature seeds: rat 508, bat 509, slime 510. Tested Blender: 5.2.2 LTS (`d13f752e3b9c`). All new dimensions, colors, light settings and sculpt parameters are Prototype presentation values: W5-08d, 2026-10-10, `source_url: null`.

From the checkout root:

```sh
mkdir -p artsource/creatures/.config
XDG_CONFIG_HOME="$PWD/artsource/creatures/.config" \
  /home/brewerm/Downloads/blender-5.2.2-linux-x64/blender \
  --background --factory-startup --threads 12 --python-exit-code 1 \
  --python artsource/creatures/build.py -- --no-render
python artsource/creatures/validate_glb.py > artsource/creatures/output/glb-validation.json
XDG_CONFIG_HOME="$PWD/artsource/creatures/.config" \
  /home/brewerm/Downloads/blender-5.2.2-linux-x64/blender \
  --background --factory-startup --threads 12 --python-exit-code 1 \
  --python artsource/creatures/validate_blender.py
```

For a single bake, use `-- --only rat --no-render` (also accepts `bat` or `slime`). Run builds serially because they merge `validation.json`. For final evidence directly from the exported files, render each creature and view in a short invocation:

```sh
XDG_CONFIG_HOME="$PWD/artsource/creatures/.config" \
  /home/brewerm/Downloads/blender-5.2.2-linux-x64/blender \
  --background --factory-startup --threads 12 --python-exit-code 1 \
  --python artsource/creatures/preview_exports.py -- --only rat --view gameplay
```

Repeat for each of `rat`, `bat`, `slime` and each of `hero`, `gameplay`, `actions`. `--all-views` still renders all three views together; `--hero-only` remains supported. No view flag defaults to gameplay. Previews overwrite the same nine filenames. Each is 1280×720, CPU Cycles, 12 samples plus denoise. Gameplay uses the unchanged 12 m/55°/45° camera, a procedural dark slate slab floor, warm 650 W point key and cool 120 W area fill. Hero and action views use the separate studio setup. These lights are review conditions, not measured Unreal lighting.

`preview_source.py -- --only rat` (via Blender) provides a fast unbaked `_draft.png` for form/material iteration. It is not export evidence; remove the three draft PNGs before collecting final artifacts. Small rendering/texture differences across hardware are possible; deterministic construction does not promise bit-identical PNGs.

The ignored `output/` holds three GLBs, baked 1024px textures, Cycles CPU hero/gameplay/action previews and build validation JSON. Generated assets are uncommitted and overwritten on regeneration. The task report records the exact outputs actually produced and their limitations.

Meshes use metres, +X forward, +Z up, scale one and floor roots. Unreal import must convert metres to centimetres. Each creature has one baked material: base colour in sRGB; tangent normal and ORM in non-colour space. ORM red stores actual Cycles ambient occlusion, green roughness and blue metallic. The exported material binds this same packed texture as glTF occlusion and metallic/roughness inputs. Procedural surface detail is baked to normals. The production pipeline uses neither concept pixels nor downloaded assets.

Rat body, haunches, shoulders, neck and muzzle are fused with a build-time voxel remesh, smoothed and reduced to about 1,750 triangles, then given explicit blended bone weights. The coat roots 1,600 tapered geometric cards on that final skin. It has a darker saddle, warmer flanks, paler underside and narrow lighter dorsal tips; the cupped pink ears retain thin furred backs. Tail rings are subtle baked color/normal detail. There is no alpha-groom dependency or exported sculpt modifier.

The bat has a fuller torso, surface-rooted forehead/cheek fur, a compact darker muzzle, tiny nose leaf and teeth, thin ridged ears and exposed hooked thumbs. Its existing membrane panels, spars, four wing joints and baked vein pattern are preserved. Fingers do not have independent bones.

Slime uses a low asymmetric union of rounded lobes with small flattened rim drips. A dense dark green center, lighter thin rim and four irregular soft inclusion highlights replace all repeated ring dots and surface bubble spheres. Slime depth and membrane translucency remain **opaque baked approximations**, not physical transmission, refraction or actual internal bubbles. Runtime subsurface/transmission behavior remains separate material work.

Five NLA tracks per creature at 30fps: idle (60f), move (24f), attack (42f), hit (18f), death (60f). Attack contact is f30; recoil ends at f42. Death holds from f45 through f60. Floor root transforms stay constant. Keys are visual prototypes, not damage events or measured movement speed.

`validate_glb.py` inspects actual GLB bytes using only Python's standard library. It checks the 3–8k triangle budget; finite vertex and animation values; skin bindings, joint indices and normalized weights/normals; exactly five named clips with expected durations; constant root transforms; idle/move loop endpoints; and the death hold. Embedded PNGs must be 512 or 1024 square, have valid CRCs and decode successfully. The validator verifies that AO is bound to ORM and that its decoded red channel has meaningful variation at interior UV samples from exported mesh triangles, including a robust percentile spread that rejects variation limited to texture background or rare outliers. This proves exported channel wiring and data variation; it cannot establish whether AO is physically correct or whether a creature matches the concept. Visual review and the Blender bake implementation provide separate evidence. Unreal import, in-game appearance and anatomical skin quality remain separate checks.

`validate_blender.py` independently imports the actual GLBs and samples all five actions at every integer frame: 209 frames per creature. It writes compact `export-pose-audit.json` with measured bounds and exact frame entries for any violations. It also constructs each creature twice and compares SHA-256 signatures of vertices, topology, transforms, weights and bones. This checks construction on this Blender build; it does not establish cross-version identity, swept/subframe clearance, absence of self-intersections, skin quality or in-game results.

## W5-08e — remaining roster

`roster.py` dispatches the eleven creature builders and records the written art
clearance rulers. The new batch is `goblin`, `giant_spider`, `balork`,
`goblin_warrior`, `atrocity`, `dungeon_bat`, `giant_bat`, `undead_bat`.
The source modules are `humanoids.py`, `spider.py`, `balork.py` and
`bat_variants.py`. New dimensions/materials/poses are Prototype presentation
values: W5-08e, LH_Prototype_v1, 2026-10-10, `source_url: null`.

Reuse the commands above, replacing `--only rat` with a batch name. The build
and both validators also accept `--batch2` to process all eight. Example:

```sh
XDG_CONFIG_HOME="$PWD/artsource/creatures/.config" \
  /home/brewerm/Downloads/blender-5.2.2-linux-x64/blender \
  --background --factory-startup --threads 3 --python-exit-code 1 \
  --python artsource/creatures/build.py -- --batch2 --no-render
python artsource/creatures/validate_glb.py --batch2 \
  > artsource/creatures/output/batch2-glb-validation.json
XDG_CONFIG_HOME="$PWD/artsource/creatures/.config" \
  /home/brewerm/Downloads/blender-5.2.2-linux-x64/blender \
  --background --factory-startup --threads 3 --python-exit-code 1 \
  --python artsource/creatures/validate_blender.py -- --batch2
```

Use `build.py -- --only goblin --geometry-only` before a bake to check the
production mesh join, rig and every integer-frame pose against the art rulers.
This mode creates neither a GLB nor textures. All new pose callbacks receive
reset bones and author every integer frame; the shared harness grounds the
body during death, holds the final corpse, and preserves the floor root.
Goblins use the explicit larger death boxes in their specifications; no transit
limit is silently enlarged. The reduced movement loops are presentation studies;
start/stop/turn blending still belongs to runtime integration.

For independent per-creature export audits, use
`validate_blender.py -- --only goblin`. It writes
`goblin-export-pose-audit.json`; `--batch2` writes
`batch2-export-pose-audit.json`. Each ordinary new mesh has a 3–10k triangle
budget; Balork has 3–20k. The pilot retains its 3–8k budget. All delivered
atlases are 1024². Base pigment is now baked through a temporary emission
connection, then the original surface is restored for normal baking. This
prevents metallic weapon surfaces from losing their base color in a diffuse
BSDF bake; it does not bake scene lighting or add emissive runtime materials.
ORM blue now preserves authored metallic values, with organic surfaces zero.
AO remains the actual Cycles geometric bake; channel wiring and occupied-UV
variation are checked on the embedded export PNGs.

Goblin and warrior exports contain `<kind>_body` and `<kind>_weapon` meshes,
weighted to the same rig and sharing one atlas/material. `Weapon_R` is their
primary attach bone, with `Weapon_Main`, `Weapon_Support`, `VFX_WeaponTip`
alias/inspection anchors. Balork exports `balork_body` and `balork_weapon`;
use `Weapon_Main` and `Weapon_Support`, plus `VFX_WeaponTip_A/B`.
These are actual exported bones, not Unreal socket assets. Hide/remove the
named default weapon mesh before attaching a replacement. Skin weights never
make weapon tips hitboxes. The renderer joins imported parts only within its
disposable evidence scene; the exported files retain removable equipment.

Generate each of `hero`, `gameplay`, `actions` with `preview_exports.py` as
above. Eight creatures produce 24 final PNGs, with unchanged gameplay camera,
dark stone floor and warm lighting. `preview_source.py` remains draft-only.
Per-kind `<kind>-validation.json` files are written in addition to the legacy
aggregate `validation.json`; serial builds are recommended. If jobs run in
parallel, aggregate the per-kind files after completion rather than treating
the concurrently merged legacy file as authoritative.

All PNGs, logs, atlases and GLBs belong to ignored `output/`. Do not commit
these or copy concept/reference pixels into an output package. The task
implementation document records actual checks and known visual limitations;
these scripts do not certify Unreal appearance, navigation or a project gate.

For the common-scale bat comparison, first build the unchanged pilot `bat`
alongside the three variants, then invoke `preview_family.py` through the same
headless Blender command. It imports all four GLBs and emits
`bat_family_color.png` and `bat_family_grayscale.png`. The latter converts shader
pigment to luminance and neutralizes review lights inside Blender; no PNG is
post-processed. This elevated overview camera is a separate comparison condition,
not the fixed gameplay camera. Hero/gameplay frames retain the original local
lamps, while action sheets use uniform directional studio lights so large rows
remain inspectable at both ends.
