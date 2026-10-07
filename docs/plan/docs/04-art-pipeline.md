# Visual references and artwork production

## 1. What is supplied

The package contains downloaded map and character/monster reference images, a provenance manifest, and enhanced visual-development concepts. Consult [asset manifest](../assets/asset-manifest.json) for exact filenames, sources and checksums. These files are starting material. A small JPEG of a monster is not a rigged 3D model, texture set or walk animation.

Original reference → readable temporary proxy → improved concept → model/material/rig → Unreal import → animation integration → in-game review is the production sequence.

Keep original downloads unchanged. Treat asset availability and permission as separate metadata: no reusable commercial license was established for the reference images during this research. Record that status for later distribution decisions; do not describe an AI-enhanced derivative as automatically cleared. This does not prevent the design and visual-reference work in this package.

## 2. Art direction

Grounded, earthy medieval fantasy with readable stylization. Use warm brown masonry, rough timber, aged plaster, restrained cloth and leather, dark subterranean recesses and selective warm torchlight. Upgrade lighting and materials without converting the small starter temple into a monumental gothic cathedral.

Preserve the silhouettes and recognizable colors of reference monsters. Distinguish improved texture detail from a redesign: a rat stays a small quadruped, the bat family stays winged, slime remains amorphous, and Balork retains the researched demon identity. The reference index shares graphics among several monsters; separate gameplay definitions must survive a shared mesh.

The supplied concepts are visual proposals. They do not verify floor topology, NPC costumes, historical scale or original lighting. The floor-plan source images and extracted room/portal graph govern reconstruction.

## 3. Reference ingest

For each source asset record: stable ID, source page URL, direct image URL, retrieved date, filename, dimensions, SHA-256, subject, provenance/permission status, baseline/version, and allowed project purpose. A server gallery proves what the server displays, not what shipped in every original client.

Save originals under `Reference/` in the eventual project, outside automatically cooked content. Store generated concepts under `ArtSource/Concepts/`. Import only needed runtime images to an explicitly labeled `Content/Lighthaven/Art/Prototype` folder. Avoid accidentally cooking every downloaded map into the game.

Images with missing directions or poses may be used as UI thumbnails or billboard placeholders at the earliest integration stage. Do not fake an eight-direction animation set by mirroring an asymmetric weapon or stretching a single image. For running, use a simple compatible skeletal proxy until the proper character is authored.

## 4. Asset work packages

| Package | Minimum deliverables | Reuse |
|---|---|---|
| Human player | Modest starter outfit, editable body/presentation variants, idle/walk/run/attack/cast/hit/death | Shared humanoid skeleton and compatible equipment attachments |
| Temple NPCs | Priest/healer/trainer/quest-giver appearances | Player humanoid rig where suitable; verified costume reference or labeled extrapolation |
| Rat family | Mesh, fur/material, locomotion/attack/hit/death | Brown Rat definition; later rat variants optional |
| Bat family | Mesh, wing rig, hover/fly/attack/hit/death | Bat, Giant Bat, Undead Bat, Dungeon Bat use separate data/size/material variants |
| Slime | Mesh or deformable setup, locomotion/attack/hit/death | Green Slime only in this slice |
| Spider | Mesh, leg rig, movement/attack/hit/death | Giant Spider |
| Goblin family | Mesh, equipment, humanoid or dedicated rig, required animations | Goblin and Goblin Warrior definitions |
| Atrocity | Mesh, distinctive limbs/shape, rig and full combat set | Do not substitute a generic undead humanoid without review |
| Balork | Recognizable demon model, boss scale, full combat set and VFX | One boss definition; no unsupported second phase required |
| Church kit | Walls, doors, beams, floor, roof, altar, benches, modest fixtures | City expansion shares architectural kit |
| Basement kit | Straight/corner/T wall modules, floor, stairs, doors/archways, torch, restrained debris | All four levels |

Exact variants and their floor placement come from [world specification](03-world-and-encounters.md). Do not add skeletons because they fit the visual theme: the cemetery crypt is a different content location.

## 5. From downloaded image to improved model

1. Inspect each reference at native resolution. Note silhouette, relative proportions, stance, palette and uncertainty. Never hallucinate detail as recovered original data.
2. Generate or draw an enhanced concept preserving those anchors. Label invented rear/side anatomy and costume details.
3. Create coherent front/side/back modeling reference where needed. Generated turnarounds require manual consistency checks; they are not measurements.
4. Build the mesh in Blender, use clean topology, UVs and deliberate material slots. Bake normal detail from a sculpt only where worthwhile.
5. Rig and animate; check foot/wing contacts, root placement, attack reach and the intended camera distance.
6. Export through a pinned, tested Blender→Unreal format/toolchain. Record units, skeleton, orientation and import settings. Unreal uses centimeters; validate with a one-meter cube.
7. Import into an assigned content folder, create materials/LODs/collision, then test in the target scene. Grounded enemies need collision and navigation behavior distinct from purely visual bat flight.
8. Bind visuals through `PresentationId`/soft references, never through the combat definition's class name. Art replacement must not change HP, hit chance or reward data.

Do not use a generated image as a normal map or a PBR roughness map without material-authoring work. Do not claim a concept sheet is animation-ready.

## 6. Initial budgets and review criteria

Budgets are provisional measurement targets, not Unreal limitations: player/NPC LOD0 approximately 20–40k triangles, common monster approximately 5–20k, boss approximately 30–60k. Start with 1–2K common textures and share atlases/materials where visually acceptable. A distant rat generally needs less detail than the character-creation camera.

Prioritize silhouette and animation over unnecessary surface density. Establish a lower quality setting without depending on the highest-cost lighting features. Shadows, translucency, overdraw and skeletal work must be measured in a crowded room before multiplying enemies.

Review each family at actual gameplay zoom, not only in a close-up beauty render. Tests: silhouette distinctness, animation readability, target-ring visibility, color accessibility, wall-occlusion readability, scale through doors, and no collision change when swapping LODs.

## 7. Definition of done for an art task

- Original reference ID and concept ID linked; any departures described.
- Source files, export settings and final runtime files all present.
- Correct units/pivot/skeleton/material setup; no missing dependencies.
- Required animation states and transitions play in-engine.
- Runtime data references the intended asset, with placeholder fallback where appropriate.
- Screenshot/video from the actual target map and camera; no substitute render presented as gameplay.
- In-engine measured cost, reproducible import notes and provenance status updated.

One agent owns each shared skeleton, animation Blueprint, master material and map at a time. Dependent artists submit assets into their own family folders. Merge serialized Unreal assets only through explicit ownership, not line-based conflict tools.

## 8. Concept prompts

The actual prompts and generation metadata are saved in `assets/concepts/prompts.md`. Further work should request one family per image for clear iteration. For example: preserve reference rat body/head/tail proportions; improve material/lighting; neutral readable three-quarter pose; no labels or extra limbs; transparent background when producing an isolated cutout. A concept must still pass inspection before import or modeling.
