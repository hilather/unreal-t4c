# W5-07b — reproducible B1 cellar art candidate

Task ID: W5-07b. Base revision: `e0f98f3fedabe05e9dbdf7f412d6ebd3b6e63332`. Result revision: the submission commit containing this document, recorded in the submission receipt. Contract revision: 1.

Owned paths: `artsource/blender/` and this document. No binary assets committed. No Unreal code, maps, collision, navigation, gameplay data, project settings or installed software changed. This replaces W5-07's blocked source-path handoff; lowercase `artsource/blender/` is explicitly authorized by W5-07b.

Behavior changed: a deterministic headless Blender workflow now builds twelve B1Cellar meshes for eleven used presentation IDs (two signed stair variants), bakes nine shared 512px textures, exports GLBs plus a manifest, checks their Blender re-import, and renders individual pieces and a mock room. This is an art candidate for the subsequent Unreal import task, not an imported/gameplay result or G5 acceptance.

Source-backed mechanics: none. Every dimension inherited from the presentation kit and all new geometry detail, color, light power, material, camera and mock placement choices are **Prototype presentation tuning**, W5-07b, 2026-10-10, source URL null. No HP, XP, loot or other game rules are introduced.

## Rebuild

Run from the repository root, with the supplied Blender 5.2.2 LTS (`d13f752e3b9c`). The directory under `artsource/blender/` keeps Blender configuration local and ignored. `Saved/` is already ignored by the repository; source-local generated files have additional rules in `artsource/blender/.gitignore`.

```sh
mkdir -p artsource/blender/.config
BLENDER_ROOT=/home/brewerm/Downloads/blender-5.2.2-linux-x64
XDG_CONFIG_HOME="$PWD/artsource/blender/.config" \
  "$BLENDER_ROOT/blender" --background --factory-startup --python-exit-code 1 \
  --python artsource/blender/build_env.py -- \
  --output Saved/ArtExport/env --samples 24
python3 artsource/blender/validate_env.py Saved/ArtExport/env
```

`--output` defaults to `Saved/ArtExport/env`. A fresh directory receives twelve `.glb` files, `manifest.json`, `textures/` with nine PNGs, and `renders/` with thirteen 1280×720 PNGs. The GLBs contain geometry but reference **external shared PNGs**: retain the complete `textures/` sibling directory when moving/importing them. They are not self-contained GLBs.

Optional iteration flags: `--room-only`, `--render-piece Wall400`, `--render-tag first`, and `--skip-renders`. They still regenerate/export/re-import all meshes. A different render tag retains earlier room images for comparison; it is not a versioned mesh export. Existing renders are not removed automatically. The submitted evidence keeps fifteen PNGs total.

An independent rebuild and binary comparison:

```sh
XDG_CONFIG_HOME="$PWD/artsource/blender/.config" \
  "$BLENDER_ROOT/blender" --background --factory-startup --python-exit-code 1 \
  --python artsource/blender/build_env.py -- \
  --output Saved/ArtExport/env-repeat --skip-renders
python3 artsource/blender/validate_env.py Saved/ArtExport/env \
  --compare Saved/ArtExport/env-repeat
```

The seed is 7507, with stable per-piece offsets. `geometry.py` creates beveled irregular blocks, warped flagstone joints, stepped stone, separate timber planks/staves, metal hoops/nails, rubble and a static emissive torch flame. `materials.py` uses periodic shader coordinates and real Cycles CPU EMIT/NORMAL bakes. The stone adds mineral variation, pores and sparse cracks; timber uses warped grain. Ordinary mesh UVs and the `Tint` vertex color are exported. `build_env.py` owns composition, exports, manifests and render checks; `validate_env.py` independently inspects the exported bytes.

Base color is sRGB. Normal and ORM are linear data. Normal maps use the glTF/OpenGL tangent +Y convention. ORM channels are R=1 neutral occlusion, G=baked roughness, B=0 for stone/timber and 0.85 for iron. Neutral occlusion is intentional for shared tileable textures, not a claim of baked per-object AO. The periodic shaders repeat continuously; they contain no concept/reference image pixels. Each object's stone variation is vertex color rather than a separate texture. The untextured flame uses an emissive material and no exported light.

## B1 coverage and canonical dimensions

Read sources: `Source/Lighthaven/Visual/LHVisualKit.cpp` (builders at lines 18–68 and catalog/recipes at 110–202), `Source/LighthavenEditor/Commandlets/LHMapDressing.h`, `LHGenerateBasementAMapsCommandlet.cpp`, and `map-dressing.md`.

B1 uses `Presentation.Environment.Shared.` followed by Wall400, Floor400, Stair600x120, Arch240, Arch320, Sconce, Barrel, Crate, Debris, Table and Bench. The smaller wall/floor/stair variants, Door240/320, Torch, Altar and Pillar are not placed by B1's dressing path and are intentionally not exported. The catalog's Pillar is actually `Presentation.Environment.Church.Pillar`, not Shared.Pillar.

Each mesh has object origin `(0,0,0)`, unit object scale, no object rotation and no collision. Authoring coordinates are the numeric kit X/Y/Z divided by 100: X length/run, Y lateral, Z up. Standard Blender export produces glTF meters `(X,Z,-Y)`. The validator inverses that mapping to measure canonical centimeter bounds; the Blender re-import restores authoring coordinates. The later Unreal task must verify its chosen importer's axis/centimeter and normal-map conversion with these asymmetric pieces.

All IDs below use the Shared prefix. Dimensions are exact canonical **visual bounding extents**, rounded for display; the manifest retains bounds and precision. Both arches preserve at least the existing rectangular 300cm-high aperture, with shallow segmental stone above it. Stair dimensions retain the original kit’s inclined-slab bounding envelope below the path: their run/width/signed rise is 600/300/±120, rather than an axis-aligned 600×300×120 box.

| File / short ID | Dimensions cm (X×Y×Z) | Pivot convention | Triangles | Materials | GLB bytes |
|---|---:|---|---:|---:|---:|
| Wall400.glb | 400×20×400 | lower end of clear face; Y −20…0 | 3,720 | 1 | 301,720 |
| Floor400.glb | 400×400×20 | top corner; Z −20…0 | 3,000 | 1 | 243,644 |
| Stair600x120.glb | 603.922×300×139.612 | first top path endpoint | 2,940 | 1 | 238,848 |
| Stair600x120Descending.glb | 603.922×300×139.612 | first top path endpoint | 2,940 | 1 | 238,852 |
| Arch240.glb | 400×20×400 | opening base center; Y −20…0 | 2,472 | 1 | 201,752 |
| Arch320.glb | 400×20×400 | opening base center; Y −20…0 | 2,232 | 1 | 181,912 |
| Sconce.glb | 25×40×46 | mounting reference, not base | 706 | 3 | 61,496 |
| Barrel.glb | 62×62×100 | ground center | 2,866 | 2 | 236,624 |
| Crate.glb | 82×82×80 | ground center | 2,640 | 2 | 216,000 |
| Debris.glb | 93.885×47.850×12 | original cluster ground origin | 1,080 | 1 | 88,824 |
| Table.glb | 160×90×80 | ground center, long axis X | 1,080 | 2 | 90,148 |
| Bench.glb | 160×45×45 | ground center, long axis X | 840 | 2 | 70,792 |
| **Total unique geometry** | | | **26,516** | | **2,170,612** |

The descending file maps to the same Stair600x120 presentation ID with `variant: descending`, keyed in the manifest as `Presentation.Environment.Shared.Stair600x120:descending`. Its bounds are X −3.922…600, Y ±150, Z −139.612…0; the ascending bounds are X 0…603.922, Y ±150, Z −19.612…120. Sconce bounds are X ±12.5, Y −6…34, Z −20…26 and its projection is +Y. Debris bounds retain the original five yawed boxes: X −44…49.885, Y −23.526…24.324, Z 0…12.

| Shared texture family (512×512 each) | Base color bytes | Normal bytes | ORM bytes | Total bytes |
|---|---:|---:|---:|---:|
| stone | 331,688 | 656,943 | 113,770 | 1,102,401 |
| timber | 307,681 | 418,734 | 113,762 | 840,177 |
| iron | 240,722 | 360,674 | 113,790 | 715,186 |
| **Total** | | | | **2,657,764** |

Mesh plus texture payload: **4,828,376 bytes (4.828 MB)**. The 13,210-byte manifest brings the complete asset export to **4,841,586 bytes**, below the **15,000,000-byte** limit. Render evidence is separate from this runtime payload. The validator includes the exact manifest size in its budget check and rejects unmanifested mesh/texture files. The manifest records every file's byte count and SHA-256, dimensions, bounds, triangles, materials and texture dependencies.

## Important fitting handoff

Canonical dimensions/pivots match the kit, but a static-mesh table alone cannot replace the saved fitted actors correctly. `LHMapDressing::Piece` puts wall/floor fitting into `BuiltRecipe.Geometry`, leaving actor scale at one. It subtracts `(200,-10,200)` for walls or `(200,200,-10)` for floors, then multiplies by fitted dimensions divided by `(400,20,400)` or `(400,400,20)`. The import task must reproduce that recentering/scaling on the visual component. Y-long walls receive another 90° yaw. B1's cutaway wall geometry is only 120cm high; blindly placing a canonical 400cm mesh at its existing actor would change the scene substantially.

Stairs need signed fitting around `(300,0,±60)` using horizontal run/600 and absolute rise/120. The existing generator also refits the smooth slab explicitly to maintain 20cm thickness. B1's two ramps descend along world +Y with run/rise 600/−120 and 400/−100. Use the descending visual variant, retain the original smooth colliders, and verify surface contact after fitting; nonuniform visual scaling does not reproduce the generator's explicit slab-thickness correction exactly.

Arches and props use their unmodified canonical origins. The ordinary lintel path also falls through to generic wall dressing, so an existing B1 lintel can have both an Arch240 and a fitted Wall400 presentation actor. Fixture placement currently assigns every Sconce yaw zero rather than orienting it from a wall normal. Both require review in the actual generated map.

Keep imported visuals NoCollision with no navigation contribution. Existing lifecycle and tests reconstruct procedural boxes and assume `triangles == boxes×12`, original recipe triangle budgets and procedural sections. Several new assets exceed those old box-recipe budgets. The subsequent code task must update binding/lifecycle and appropriate validation deliberately; this art task has not passed those Unreal tests or measured runtime draw calls/performance.

## Render feedback and evidence

Visual target: **right half only** of `temple-and-basement-concept`, at `/home/brewerm/Downloads/lighthaven-unreal-design-and-agent-waves/lighthaven-plan/assets/concepts/temple-and-basement-concept.png`, and art direction in `docs/plan/docs/04-art-pipeline.md` §2. The source image was inspected only; it was not copied, sampled, baked, traced into geometry, or included in outputs. Meshes and materials are original procedural constructions; no external asset pack or image download was used.

The mock room is a concept-comparison diorama assembled from exported piece geometry, including fitted/cutaway instances. It is **not B1's authoritative floor plan, an Unreal screenshot, or evidence of gameplay clearance**. It has a high oblique orthographic camera, amber point lights at sconces, restrained cool area fill, and a black environment. Individual pieces use neutral studio lighting. All final views are Cycles **CPU**, 1280×720, **24 samples**, fixed seed, denoised, AgX. The first room used 16 samples; the second used 24.

Feedback sequence: first room was too clean, bright and orderly. Second introduced stronger stone/grain detail and lower light, but lost too much prop readability. The final pass restores selective fill, recesses mortar to reveal chipped bevels, warps flag joints, clusters rubble, breaks furniture symmetry and extends the upper corridor. The final stair meshes have solid stepped stone beneath the treads instead of the original visible smooth ramp, while retaining canonical bounding extents. A close-up found mortar occluding bevels and a cropped wall preview; both were corrected before the final set. The ascending stair preview now faces its risers. One intermediate full render was deliberately stopped after three images to apply fixes. A later all-piece run ended with signal 143 after Table.png; its log gives no cause. The remaining bench/room and revised stair views were run through the same entry point’s selective flags. The combined set is evidence from those completed render operations; no uninterrupted final all-view process pass is claimed.

Renders retained: `B1_room_first.png`, `B1_room_second.png`, `B1_room_final.png`, plus `Wall400.png`, `Floor400.png`, `Stair600x120.png`, `Stair600x120Descending.png`, `Arch240.png`, `Arch320.png`, `Sconce.png`, `Barrel.png`, `Crate.png`, `Debris.png`, `Table.png`, `Bench.png` — fifteen PNGs.

Visual assessment after inspecting the final piece and room renders: **moderate concept correspondence**. The chamber composition, stone palette, paired stair routes, amber light pools and dark perimeter storage read in the intended direction. This is a reusable stylized kit, not a close reproduction of the painted detail. The final room also supports the raised landing with masonry and fits the lower landing paving to its corridor walls. Remaining gaps include cleaner/more repetitive masonry and paving than the painted target, conspicuous repeated timber grain (including grain across furniture legs), coarser debris sides, simplified static flame, no cobwebs/pottery/loose boards, no volumetric shafts, and no creatures. The upper corridor and lower landing continue beyond the render frame, as a composition choice rather than a full-layout inspection. Dense unique damage and the concept's hand-composed detail are not reproduced by this small reusable catalog. The later Unreal lighting/material pass and owner review remain necessary.

Evidence destination for this attempt: `/home/brewerm/.herdr-projects/unreal-t4c/.state/worker-output/attempt-76d65a8c6bde3c5473b3f20293ff94be42e2617cb08b618f2f4a13c933940fc5/`, with `report.md`, the fifteen images under `library/`, and a small validation summary. No GLB, texture, blend file or full log is copied into worker-output.

## Checks actually run / remaining checks

- Blender 5.2.2 source generation, nine real 512px shader bakes and twelve exports completed. All twelve actual GLBs re-imported through Blender's glTF importer with matching triangle counts, local bounds, origin and scale, and loaded 512px images.
- Independent artifact validator checks GLB headers/chunks/accessor ranges, index ranges, finite/unit normals, texture and vertex-color presence, canonical frozen bounds from the C++ kit, origin transforms, PNG CRC/decompression/dimensions, hashes, complete file inventory, per-piece counts and total bytes. Final check and comparison both exited 0: twelve meshes, nine textures, 4,841,586 bytes including manifest. All 21 GLB/texture SHA-256 values match the independent rebuild.
- A fresh-process export to `env-repeat` completed with twelve Blender re-import checks, exit 0. The final export comparison passed byte-for-byte for meshes/textures. This establishes determinism on this host and pinned Blender version, not across arbitrary versions/platforms. Fifteen retained PNG headers were checked as 1280×720; final views use 24 samples and the mock room has 149 kit instances.
- Python syntax compilation completed. No third-party Python packages are required beyond Blender's bundled Python for generation and standard-library Python 3.9+ for validation.
- An actual import initially failed because the handcrafted glTF material-node group lacked Blender 5.2's extension sockets. The source now uses Blender's native complete group helper; subsequent twelve-piece re-import checks passed. Export logs retain a repeated sampler warning because one ORM image feeds multiple channels; every such image uses the same repeat/linear settings. Blender also warns that `Material.use_nodes` will change in 6.0; this workflow is pinned to 5.2.2.

Checks not run: Unreal import/material conversion, map regeneration, collision/nav/arrival tests, in-engine rendering, editor/game build, cook/package/play, GPU timings and Windows packaging. Concrete prerequisite: the separately assigned Unreal import/binding/lighting task must implement and exercise these assets in the actual project; no such changes are owned here. Windows remains deferred by project decision. The prior W5-07 tiny CPU smoke is not substituted for any of these checks.

Next task: codex-sol import/binding work using the manifest and fitting notes, then visual review of the real B1 map against the concept. Keep generated binaries uncommitted here; asset ownership/LFS/import destinations and G5 acceptance belong to the integrator. No main merge, push, release of worker capacity, or verified task-success declaration is made by this evidence candidate.

## W5-09g — warmer, larger worn masonry (2026-10-11)

This section supersedes the original geometry counts and stone palette above;
manifest IDs, dimensions, pivots, signed stair variants and nine shared 512px
textures remain the same. Source revision is `W5-09g-v1` (seed 7507). No new kit
IDs, downloads, textures made from reference images, or binary deliverables.
All choices are Prototype presentation tuning, provenance W5-09g supplied B1
captures and concept comparison, retrieval 2026-10-11, URL null.

Wall courses now average 68cm high with 78–146cm stones instead of 40cm courses
and 57–94cm stones. Individual warm tones and a darker bottom course suggest
floor grime. Unequal 65/95/70/100/70cm paving bands, wider flags, occasional
cross-jointed repairs, warped joints and worn bevels break the former uniform
rows. The shared stone albedo uses umber/ochre mineral tones; timber and iron
texture recipes stay unchanged. Bounds still normalize to the frozen canonical
contract without object scale or pivot changes.

The custom arch voussoirs had all six face windings reversed. Correct outward
faces remove that source of backface culling and inverted lighting. The independent
GLB validator now welds split normal/UV vertices and requires positive signed
volume for each arch shell. It rejects the previous exported arches as a negative
control and accepts the corrected twelve-piece export. This complements the
existing bounds, UV, normal, PNG, manifest and Blender re-import checks.

Rebuild/import remains `build/build-art.sh`. Render evidence can be regenerated
without rebaking or exporting again:

```sh
XDG_CONFIG_HOME="$PWD/artsource/blender/.config" \
  /home/brewerm/Downloads/blender-5.2.2-linux-x64/blender \
  --background --factory-startup --python-exit-code 1 \
  --python artsource/blender/render_b1_review.py -- \
  --source Saved/ArtExport/env --samples 32
```

This imports the actual exported GLBs and renders a room, a 120cm cropped wall /
flagstone / storage study, and the arch. The study's coping uses the native cap
length pattern, 6cm depth and 1.6cm joints; it is an art approximation rather than
execution of the Unreal cap code. The room retains the earlier demonstration
layout and thicker/taller walls. The study uses the true 20cm-thick kit wall.
Point lights have shadows disabled. Cycles CPU, AgX, indirect light and area fills
still differ from Unreal; these images judge shape/palette and cannot establish
Unreal exposure, pixel acceptance, performance or gameplay clearance.

Visual assessment: larger varied stones and warm gaps move toward the concept;
perimeter storage is useful at oblique distance. Remaining differences include
visible modular repetition, thinner actual B1 walls, crisp native coping edges,
simplified props/flames and no pottery or cobwebs. The existing five prop families
were sufficient for this pass, so the allowance for new pieces was not used.
The authoritative task validation and predicted Unreal means are recorded in
[lighting-b1-w5-09.md](lighting-b1-w5-09.md), W5-09g section.

Observed final export: 20,696 unique triangles (Wall400 1,560; Floor400 1,500;
Arch240 1,392; Arch320 1,152), 1,701,596 GLB bytes plus 2,655,012 texture bytes,
**4,369,302 bytes including manifest**. A fresh independent export matches all
21 asset hashes byte-for-byte. Imported B1 packages total **4,316,432 bytes**
across 26 packages, below the task's 8,000,000-byte limit. The source-only
submission excludes these generated files.
