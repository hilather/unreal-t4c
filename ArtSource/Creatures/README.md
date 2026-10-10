# Scripted creature pilot

Original geometry and procedural material authoring in `build.py`; no image inputs or downloads. Blender version tested: 5.2.2 LTS, d13f752e3b9c. Fixed seed: 508. The task's explicit uppercase owned path is used despite the general lowercase-directory memory.

From the checkout root:

```sh
mkdir -p ArtSource/Creatures/.config
XDG_CONFIG_HOME="$PWD/ArtSource/Creatures/.config" \
  /home/brewerm/Downloads/blender-5.2.2-linux-x64/blender \
  --background --factory-startup --python ArtSource/Creatures/build.py
```

The ignored `output/` contains three GLBs, nine baked 512px textures, nine 1280×720 Cycles CPU renders, and `validation.json`. Generated binary assets are intentionally uncommitted. Existing output files are overwritten. Keep the sandbox probe outside this regular regeneration count.

Meshes use metres, +X forward, +Z up, scale one, floor roots. Importers must convert metres to Unreal centimetres. Each creature has one baked material, base color in sRGB, tangent normal and ORM in non-color space. ORM is baked emission with white AO, species roughness and zero metallic: it does **not** contain occlusion from a high-poly sculpt. The normal map bakes procedural bump detail on the production mesh. There is no high-poly fur groom.

Five NLA tracks per creature: idle (60f), move (24f), attack (42f), hit (18f), death (60f), 30fps. Attack contact is f30; recoil reaches neutral at f42. Death holds unchanged from f45 to f60. Floor root transforms stay neutral. These are visual prototype keys, not damage events. Motion loops need runtime blending; no measured speed/stride agreement is supplied.

`validation.json` records triangle counts, imported action names, file sizes and sampled source-animation bounds in centimetres. The script asserts an imported armature, armature modifier and at least five imported actions. These checks do not certify Unreal import, anatomical skin quality or visual approval. See the task report for shortcomings.
