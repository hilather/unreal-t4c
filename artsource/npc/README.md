# W5-16 — the fifteen NPCs

Original scripted Blender role costumes on the W5-12 player bodies and 22-bone rigs. `roster.json` records all fifteen placed identities, current/historical roles, exact repository evidence, appearance choices and material colours. Nevanis and Shovanis are in B1; the remaining thirteen are in the hub. All costumes, gender presentation, age, proportions, colours, props, textile detail and timing are **Prototype presentation design**, W5-16 / LH_Prototype_v1, source URL null. No mechanics, services, quests, placement, collision or Unreal assets change here.

The package concept and W5-13b hub screenshot were inspected only as visual targets. No reference pixels, extracted geometry, generated-image service, asset downloads or third-party materials enter this pipeline. The existing `../player/models.py` and `animation.py` are reused directly without writing into that directory. These are stylized game figures, with remaining portrait/cloth fidelity limits documented in the handoff.

## Reproduce

Run from the checkout root using installed Blender **5.2.2 LTS**, hash `d13f752e3b9c`. Pin **8 threads** for source construction; changing threads can change the player voxel-remesh floating-point result. Construction seed is 51212 in the reused player source; textile/render seed is 516. Python bytecode, configuration and generated files stay in this owned directory.

```sh
mkdir -p artsource/npc/.config artsource/npc/output
XDG_CONFIG_HOME="$PWD/artsource/npc/.config" \
 /home/brewerm/Downloads/blender-5.2.2-linux-x64/blender \
 --background --factory-startup --threads 8 --python-exit-code 1 \
 --python artsource/npc/build.py
python artsource/npc/validate_glb.py
XDG_CONFIG_HOME="$PWD/artsource/npc/.config" \
 /home/brewerm/Downloads/blender-5.2.2-linux-x64/blender \
 --background --factory-startup --threads 8 --python-exit-code 1 \
 --python artsource/npc/validate_blender.py
XDG_CONFIG_HOME="$PWD/artsource/npc/.config" \
 /home/brewerm/Downloads/blender-5.2.2-linux-x64/blender \
 --background --factory-startup --threads 8 --python-exit-code 1 \
 --python artsource/npc/audit_geometry.py
```

`build.py -- --only Fali` is an iteration shortcut and **replaces the manifest with a partial roster**. Run a full build before validation/delivery. Builds overwrite ignored `output/`; generated GLBs, PNGs and JSON reports are not committed. No `.blend` is required.

Render each hero in a separate invocation of the same Blender prefix (substitute any suffix from the roster):

```text
--python artsource/npc/preview.py -- --view hero --only BrotherKiran
--python artsource/npc/preview.py -- --view lineup
--python artsource/npc/preview.py -- --view chapel
--python artsource/npc/preview.py -- --view gestures
```

Each of fifteen hero sheets includes an idle front and talk rear view. Lineup, chapel and a front gesture comparison add three PNGs; two 512² textile images bring the standard total to **20 PNGs**. All review images use actual exported GLBs, 1280×720 CPU Cycles, twelve samples, denoising and fixed seed. Studio sheets are distinct from the warm chapel review. Chapel camera uses the game's 12 m arm, 55° downward pitch, 45° yaw and 45° horizontal FOV. Chapel geometry/lighting is original review staging, **not a game map or Unreal screenshot**.

## Import the shared kit

Use `NPC_Kit_A.glb` and `NPC_Kit_B.glb` as import inventories. Each contains every used module for its bind body **once**, plus one skeleton and the two common actions. Garments overlap in these inventories: select modules listed in `manifest.json` → `npcs[].mesh_keys`; never display the complete kit. The fifteen `NPC.Name.glb` files are assembled review/export assets and an integration oracle. Importing fifteen complete independent skeletal characters would waste the sharing scheme and likely exceed the target budget.

Modules include head/neck, torso/arms/starter tunic, trousers/boots, fitted Cropped/Tied hair, seven garment designs (priest, scholar, coat, apron, tabard, ranger, traveller), hoods/coifs/caps, two beard lengths, and shared props. Modules are triangulated, UV mapped and skinned. Geometry is genuinely shared within a body, including vertices, topology, normals, tangents, UVs and weights; the independent validator compares exported module hashes against the two kits. A/B are separate fits using their original player bind lengths. Both have the same bone names and hierarchy. Preserve the appropriate bind pose or explicitly retarget; don't substitute A inverse bind matrices on B.

Create materials from semantic slot suffixes (`Cloth`, `Accent`, `Trim`, `Skin`, `Hair`, `Lip`, `Leather`, etc.). Per-NPC sRGB hex factors are in `roster.json`; convert to linear once. The kits have example colours. Reuse master materials/instances and override the slots on each visible component. Two **shared** 512² images provide neutral textile base pigment and tangent normal; untextured leather/skin/metal use ordinary opaque PBR factors. Roughness and metallic are scalar factors. This deliberately does **not** claim baked geometric AO, skin microdetail or a per-character normal atlas. Cloth folds, seams, cords, lapels and trim are geometry. No external procedural Blender shader is required after export.

The exporter keeps canonical mesh/skin data at scale one, then writes each NPC's uniform `scale` into its standard glTF scene-parent node named `NPC.Name.Stature`. This tiny GLB JSON transform assignment is necessary because Blender's exporter otherwise bakes even ancestor scale into skin vertices. It changes no geometry, texture, inverse-bind or animation bytes. Actual reimport checks measure the resulting assembly. For kit assembly, apply the roster scale once to the entire visual root, including props. Do not apply it again when importing an already assembled NPC GLB.

Source coordinates: **metres, +X forward, +Z up**; glTF uses its standard Y-up conversion. Convert metres to Unreal centimetres once. Marker roots are floor roots. Do **not** copy the player's −90 cm capsule offset to these plain interactable marker actors. Runtime stature variation and two body builds are visual only; marker collision remains untouched.

Actions are `idle` and `talk`, each frames **0–60 at 30 fps**, two seconds. They loop with identical endpoints. Feet/pelvis/root remain stationary. Idle breathes and turns the head slightly; talk raises the left forearm and nods, leaving the right-hand staff/spear still. No locomotion, combat, lip sync, cloth simulation, fingers or facial rig is supplied. Start talk with a short blend from idle; transition blending still needs Unreal review. Props are skinned to their intended body/hand bones; they are not separate game inventory items or new services. Four player attachment bones remain present.

Bind visual selection by `ALHInteractableMarker.DefinitionId`, preserving GUID, Area, transform and yaw. Hub markers have `Presentation.NPC.*` tags, whereas B1 markers don't; DefinitionId is the common seam. Hub legacy figures are attached child actors; B1 Nevanis/Shovanis proxies are **unattached boxes** emitted separately by `LHGenerateBasementAMapsCommandlet.cpp:522`. Remove both old presentation forms during the later import task.

## Evidence and practical limits

`manifest.json` records triangles/bytes per NPC and kit, template geometry hashes, material slots, exact rest bones, texture hashes and version. `validate_glb.py` independently checks real GLB accessors, topology, weights/joints, normals/tangents, texture dimensions/sharing, roster tint factors, clips/loops and kit geometry equality. `validate_blender.py` reimports all fifteen assembled exports and samples both actions at every integer frame, measuring floor/upright bounds, root/feet stability and endpoint positions. `audit_geometry.py` reconstructs source modules twice and compares them to the exported build manifest on the pinned host configuration.

These checks don't certify arbitrary subframes, absence of all garment intersections, animation transitions, distant GPU cost, cooked size or actual map lighting. Material slots and modular skeletal draws need profiling/possible runtime merging; the small download alone doesn't prove a fast crowded chapel. The roughly **8 MB Unreal import budget remains an import/cook measurement**, not a passed Blender gate. See the [task report](../../docs/implementation/art-npc-blender.md) for actual measurements, visual review and open work.
