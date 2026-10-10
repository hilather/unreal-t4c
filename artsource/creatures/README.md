# Scripted concept creature pass

Original geometry and procedural materials are authored reproducibly with headless Blender. The plan concept is a visual comparison target only; no image is read into the generation pipeline. `models.py` builds the rat, bat and slime forms; `build.py` handles the bake, rigs, five actions, exports and renders. Fixed per-creature seeds: rat 508, bat 509, slime 510. Tested Blender: 5.2.2 LTS (`d13f752e3b9c`).

From the checkout root:

```sh
mkdir -p artsource/creatures/.config
XDG_CONFIG_HOME="$PWD/artsource/creatures/.config" \
  /home/brewerm/Downloads/blender-5.2.2-linux-x64/blender \
  --background --factory-startup --python-exit-code 1 --python artsource/creatures/build.py
python artsource/creatures/validate_glb.py > artsource/creatures/output/glb-validation.json
```

For a single bake, append `-- --only rat --no-render` to the Blender invocation (also accepts `bat` or `slime`). For final evidence directly from the exported files:

```sh
XDG_CONFIG_HOME="$PWD/artsource/creatures/.config" \
  /home/brewerm/Downloads/blender-5.2.2-linux-x64/blender \
  --background --factory-startup --threads 12 --python-exit-code 1 \
  --python artsource/creatures/preview_exports.py -- --all-views
```

Without `--all-views`, this refreshes only gameplay cameras. `--only rat` is also supported. Run builds serially when producing `validation.json`. Previews overwrite the same nine filenames; they do not create iteration archives. Small render and texture differences across hardware are possible; deterministic construction does not promise bit-identical PNGs.

The ignored `output/` holds three GLBs, baked 1024px textures, Cycles CPU hero/gameplay/action previews and build validation JSON. Generated assets are uncommitted and overwritten on regeneration. The task report records the exact outputs actually produced and their limitations.

Meshes use metres, +X forward, +Z up, scale one and floor roots. Unreal import must convert metres to centimetres. Each creature has one baked material: base colour in sRGB; tangent normal and ORM in non-colour space. ORM red stores actual Cycles ambient occlusion, green roughness and blue metallic. The exported material binds this same packed texture as glTF occlusion and metallic/roughness inputs. Procedural surface detail is baked to normals. The production pipeline uses neither concept pixels nor downloaded assets.

The rat's coat uses rooted, tapered geometric cards plus directional color and bump detail; no alpha-groom dependency. Thin bat membranes are double-sided, with actual finger struts and a baked fine vein pattern. Slime depth and membrane translucency are **opaque baked approximations**, not physical transmission or refraction. The slime's bubble rings are procedural shading, supplemented by shallow surface-near bubble geometry. Actual subsurface/transmission behavior in Unreal remains future material work.

Five NLA tracks per creature at 30fps: idle (60f), move (24f), attack (42f), hit (18f), death (60f). Attack contact is f30; recoil ends at f42. Death holds from f45 through f60. Floor root transforms stay constant. Keys are visual prototypes, not damage events or measured movement speed.

`validate_glb.py` inspects actual GLB bytes using only Python's standard library. It checks the 3–8k triangle budget; finite vertex and animation values; skin bindings, joint indices and normalized weights/normals; exactly five named clips with expected durations; constant root transforms; idle/move loop endpoints; and the death hold. Embedded PNGs must be 512 or 1024 square, have valid CRCs and decode successfully. The validator verifies that AO is bound to ORM and that its decoded red channel has meaningful variation at interior UV samples from exported mesh triangles, including a robust percentile spread that rejects variation limited to texture background or rare outliers. This proves exported channel wiring and data variation; it cannot establish whether AO is physically correct or whether a creature matches the concept. Visual review and the Blender bake implementation provide separate evidence. Unreal import, in-game appearance and anatomical skin quality remain separate checks.
