# W5-07b — reproducible B1 cellar art candidate

The B1 sections below are historical evidence. **For the current five-style
command, output layout, budgets and W5-14 handoff, see the W5-14 section at the
end.** The original flat B1 export remains available with `--legacy-root`.

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

## W5-14 — Church, B2Damp, B3Crypt and B4Ritual

Task W5-14, contract revision 1, base
`a62e13084bb92a236343c28f25659bebec439723`. Result revision is the commit containing
this section, recorded in the attempt report/submission receipt. Owned paths are
`artsource/blender/` and this document. This is a **Blender art candidate** for the
later import/binding/lighting task; no Unreal assets or code are changed here.

All new dimensions not inherited from the kit, colors, wear, carvings, lights,
material compression and mock placements are **Prototype presentation tuning**,
W5-14, 2026-10-11, source URL null. No gameplay mechanics, historical authenticity
or reconstructed lettering is claimed. The concept was viewed only, never read
by a generation script, sampled, traced, copied or baked. No asset downloads,
image generation service, third-party textures or new installed packages were
used. Church takes the modest, warm masonry/timber direction from §2 of
`docs/plan/docs/04-art-pipeline.md` and the **left half** of the package's
`assets/concepts/temple-and-basement-concept.png`. Host W5-09g captures informed
the gap between the existing flat presentation and the requested material detail.

### Current rebuild and review

From the checkout root:

```sh
mkdir -p artsource/blender/.config
export XDG_CONFIG_HOME="$PWD/artsource/blender/.config"
BLENDER_ROOT=/home/brewerm/Downloads/blender-5.2.2-linux-x64
"$BLENDER_ROOT/blender" --background --factory-startup --python-exit-code 1 \
  --python artsource/blender/build_env.py -- --output Saved/ArtExport/env \
  --skip-renders
python3 artsource/blender/validate_styles.py Saved/ArtExport/env
"$BLENDER_ROOT/blender" --background --factory-startup --python-exit-code 1 \
  --python artsource/blender/style_review.py -- --source Saved/ArtExport/env \
  --styles Church B2Damp B3Crypt B4Ritual --samples 24 --tag final
```

One builder invocation defaults to **all five** style folders. Each contains its
own `manifest.json`, GLBs and `textures/`. GLB images are **external references**,
not embedded; move the entire style folder. `--styles Church B2Damp` (or comma
separated names) limits rebuilding. Omitting `--skip-renders` also renders all B1
pieces and its room, and a room/detail study for each new style: **21 PNG views
in a fresh export**. `--room-only`, `--render-piece NAME`, `--samples` and
`--render-tag` support narrower iterations. Tags retain prior renders; prune your
own old review images rather than accumulating unbounded iterations.

The standalone review imports the **actual exported GLBs and their shared PNGs**,
so final evidence exercises the export, including quantization and material
conversion. It supports `--room-only` and `--details-only`. These are 1280×720
Cycles **CPU**, low-sample, fixed-seed, denoised AgX art studies. They are neither
the authoritative map layouts nor Unreal screenshots. No render-only replacement
materials or reference images are used. Light powers/color grading are review
tuning, not Unreal settings or a measured in-game brightness promise.

**Existing B1 consumer compatibility:** `build/build-art.sh` and the existing
importer read the former flat `Saved/ArtExport/env/manifest.json`. They are outside
this task's write paths. Until the integration task updates them, generate the
legacy layout explicitly with `build_env.py -- --legacy-root --output
Saved/ArtExport/env --skip-renders`, or point their reader at `env/B1Cellar`.
Do not mistake a stale flat-root manifest for a new multi-style build. B1's
original generation, export, texture and manifest implementation is retained;
`geometry.py`, `materials.py` and `validate_env.py` are unchanged.

### Coverage, fitting and reuse

Read contracts: `LHVisualKit.h/.cpp`, `LHMapDressing.h`, all three map generators,
`LHB1ArtBinding.cpp`, and `docs/implementation/map-dressing.md`. Every placed
piece ID for the four requested styles is covered. Standard IDs use
`Presentation.Environment.Shared.`:

| Style | Pieces actually used by current maps, beyond the common set |
|---|---|
| Church | Arch320, Altar; descending stairs |
| B2Damp | Arch320; descending stairs |
| B3Crypt | Arch240; both signed stair variants |
| B4Ritual | Arch240, `Presentation.Environment.Basement.ArchBoss500`, Altar; ascending stairs |

The common set is Wall400, Floor400, Stair600x120, Sconce, Barrel, Crate, Table,
Bench and Debris. All four exports also include both ordinary arches and both
signed stair variants as reusable extras. Church adds its requested
`Presentation.Environment.Church.Pillar` and `.DoorLeafPreview`; these are
existing catalog IDs but are **not currently placed** by the generators.
Altar is **Shared.Altar**, not Church.Altar.

Existing B1 common envelopes/pivots remain as documented above. New frozen
canonical bounds in centimeters:

| File | Minimum XYZ | Maximum XYZ | Pivot / status |
|---|---|---|---|
| Altar.glb | −100,−80,0 | 100,80,120 | ground center; Shared.Altar |
| Pillar.glb | −30,−30,0 | 30,30,400 | ground center; Church.Pillar |
| DoorLeafPreview.glb | 0,−5,0 | 160,5,300 | hinge edge; Church.DoorLeafPreview |
| ArchBoss500.glb | −300,−20,0 | 300,0,400 | opening base center; Basement.ArchBoss500 |
| Carpet.glb | −200,−1050,0 | 200,1050,1 | auxiliary, base center; **no presentation ID** |

The carpet matches the generator's 400×2100×1 cm `Temple.RedAisle` visual extent.
Its manifest explicitly says `piece_id: null`, `binding_status:
auxiliary_unbound`. That primitive is skipped by Dress; the later task must
explicitly bind it, with base Z=0 at the aisle's XY center. This does not propose
a new shared catalog/schema ID. The ordinary arches keep 240/320×300 cm clear
rectangles; the boss arch keeps **500×360 cm**, within its 600×20×400 cm envelope.

Every mesh has identity object transforms and pivot zero. Authoring meters are
kit centimeters/100; glTF uses `(X,Z,-Y)`. Normals use tangent +Y/OpenGL. Keep
NoCollision and no navigation contribution, and retain the existing smooth ramp
colliders. Shared stairs, ordinary arches and props call the original B1 geometry
builders with their original seeds. Different stone families recolor those
shapes without remodelling them. The six timber/iron PNGs and common stone-normal
PNG are identical across the new folders and can be deduplicated at import;
per-style budgets nevertheless count **every copied byte**, without assumed
deduplication savings. Matching prop GLBs and matching geometry can also be
deduplicated by the integrator where material binding permits it.

**Fitting is still an integration responsibility.** The existing B1 binder now
tiles/crops around fitted recipe bounds rather than uniformly stretching full
walls and floors; the old W5-07b fitting description predates that change.
Church's current wall caps are **80 cm high**, below the plaster beginning at
155 cm. Simply applying B1's crop will show only masonry. Upper plaster/framing
needs an explicit art/occlusion placement decision; the tall mock walls do not
prove that the current hub will look like the mock. B2 has 120 cm caps, which
retain the low moss treatment. B3/B4 boundary boxes span **Z −100…400 cm**; a
tile fitted from −100 shifts the first niche down by 100 cm. A B1-style 120 cm
cutaway would remove the niche heads. B1 solid backing must not be copied behind
crypt recesses unaltered: its 10 cm core would partially fill the 3.4 cm niche
screen. Review the actual map heights, backing and visibility before binding.

### Church

Warm sandstone courses support aged ivory plaster, pegged rough timber and small
irregular exposed masonry patches. Square timber pillars with stone shoes and
caps, simple plank benches, an iron-strapped door leaf, a modest slab altar with
a narrow runner, and a muted red aisle make a small starter temple. Upright
pillar/door UV grain runs vertically. No cathedral towers, gothic tracery,
stained-glass spectacle or roof extension is introduced. The mock is an interior;
it does not claim to reproduce the concept's shoreline, roofs, vegetation or
larger district composition.

### B2Damp

Cool dark slate with olive mineral/moss mottling and a darker green bottom course
uses the existing wall shape. Its signature Floor400 adds two shallow irregular
pooled stains inside the canonical bounds. Shared albedo/normal images support a
low-roughness wet material with reduced normal strength; no water simulation,
transparency or gameplay water is added. Amber fixtures contrast with restrained
cool fill. Timber storage props retain the common B1 geometry.

### B3Crypt

Pale neutral limestone, dry roughness and dusty fragments contrast immediately
with B2. Wall400 contains two genuinely shallow arched memorial recesses, with
blank inset tablets and a central backing screen. The thin 20 cm wall contract
limits their depth; they are not walk-in alcoves. No skeletons, new creature
population or invented inscriptions are added. Geometry for paving, stairs,
arches and timber furniture remains shared.

### B4Ritual

Charcoal basalt, restrained red-brown mineral inlay, a divided shallow floor
medallion with recessed radial gaps, and a marked slab altar distinguish the
ritual floor. Carvings have actual separated stone sectors, rather than an
emissive decal. Inlay is nonemissive oxide, deliberately subdued after the first
render showed overly orange lines. The wide boss frame preserves the original
500×360 cm opening. Sigils are original abstract presentation, not source-backed
writing or a new ritual mechanic.

### Materials, budget and validation evidence

New style materials are real 512px Cycles CPU EMIT/NORMAL bakes from original
periodic node recipes. Church adds baked plaster mottling and crossed cloth
warp/weft albedos, sharing existing normal/ORM images. To fit the 4 MB ceiling,
new baked RGB samples are quantized to **5 bits per channel**, ORM to **4 bits**,
then saved as ordinary 8-bit PNG with lossless level-9 compression. Resolution
stays 512×512. This is lossy color/normal precision, not a claim of lossless bake
compression. B1 is exempt from this change and retains its exact original PNGs.
ORM R is neutral occlusion, G roughness, B metallic; shared textures do not encode
per-object AO. Wet and oxide have deliberate constant PBR factors; flame remains
a simplified static emissive shape with no exported light.

The new manifests record per-piece seeds (legacy base 7507 or signature base
7514, plus index×101), IDs/variants, bounds, dimensions, triangle counts, material
slots, hashes, bytes and external dependencies. `validate_styles.py` independently
freezes required inventories and canonical bounds, calls the original GLB/PNG
validator, and adds positive signed-volume checks for custom walls, floors,
altars, pillars, door, carpet and boss arch. This caught two inward puddle side
shells; the original bad export fails the new check and the corrected export
passes. All actual GLBs also go through Blender re-import checks for triangle
counts, bounds, origin/scale and loaded 512px images.

The pre-existing B1 export is **4,369,302 bytes** including manifest. Applying a
new 4 MB limit to it would contradict byte preservation. W5-14 therefore applies
the **4,000,000-byte complete-export cap to each of the four new styles**, and
reports B1 unchanged as an explicit pre-existing exception. Renders are evidence
and excluded from runtime payload. Complete observed final measurements and
repeat-export results are recorded below and in this attempt's report.

| Style | GLBs | 512px textures | Unique exported triangles | GLB bytes | Texture bytes | Manifest bytes | Complete bytes |
|---|---:|---:|---:|---:|---:|---:|---:|
| B1Cellar, unchanged | 12 | 9 | 20,696 | 1,701,596 | 2,655,012 | 12,694 | **4,369,302** |
| Church | 16 | 11 | 26,044 | 2,149,164 | 1,224,813 | 18,806 | **3,392,783** |
| B2Damp | 12 | 9 | 20,688 | 1,699,184 | 1,011,392 | 14,096 | **2,724,672** |
| B3Crypt | 12 | 9 | 21,332 | 1,754,056 | 1,022,794 | 14,097 | **2,790,947** |
| B4Ritual | 14 | 9 | 24,960 | 2,053,140 | 987,102 | 15,873 | **3,056,115** |

These are measured **asset-export** totals from the final `--skip-renders` build,
including the auxiliary carpet and reusable extras. They are not packaged Unreal
sizes, scene triangle counts or draw-call measurements. The new four together
are 11,964,517 bytes before import deduplication. Exporting with render metadata
adds a small amount to each manifest, still comfortably inside each new limit.

### Feedback loop and observed checks

First reviews used 12 samples; final room and key-piece studies use **24 samples**.
Church's first detail cropped the pillar and exposed horizontal grain on upright
wood; framing and UVs were corrected, plaster losses made irregular, and cloth
given a woven albedo. B2's first pools looked like flat, rough stone stickers;
the revised irregular outlines, subtle normal strength and low roughness read as
pooled stains. A reflection-light experiment produced distracting white glare,
so its power was reduced before the retained final view. B3/B2 mock exits were
initially backed by wall tiles; those tiles were removed. B4's first inlay looked
too bright orange and was darkened to oxide red-brown. The independent winding
defect was corrected in the actual exported geometry, not hidden by lighting.

Visual assessment after inspecting the final views: the four kits are clearly
distinct at room distance. Church has good correspondence to the requested
**small temple material vocabulary**, but only moderate correspondence to the
painted exterior concept's richness. B2 gives a useful damp, mossy read, with
noticeably repeated puddle silhouettes. B3 reads as pale, dry memorial stone;
its thin niches remain shallow and orderly. B4's dark paving, inset lines and
altar make the ritual identity readable without emission. These are deliberately
small stylized modular kits, not close reproductions of painted environment
detail. Repeated blocks, sigils and wear, sparse clutter, simplified static
flames, inherited cross-grain on some common furniture legs, shallow recesses
and absent vegetation/volumetric atmosphere remain visible limitations. Final
in-engine lighting, occlusion, palette and owner acceptance remain open.

Checks actually run:

- The final single all-style `--skip-renders` invocation completed with **exit 0**,
  generating all 66 GLBs and performing 66 Blender importer round-trips. All five
  manifests passed `validate_styles.py`, including PNG checks, frozen geometry
  bounds, IDs/variants, external dependencies, outward shell volumes and budgets.
- B1 was exported before changing the entry point. All **22 files**, including
  the manifest, match the final B1Cellar folder byte for byte with the same
  `--skip-renders` flags. No cross-version determinism claim is made.
- Fresh independent per-style Blender processes returned **exit 0** and produced
  **96/96 identical files** for the four new styles: Church 28, B2 22, B3 22,
  B4 24. The comparison includes every GLB, PNG and manifest, followed by
  independent artifact validation. Repeat directories were removed after checks.
- POSITION/index buffer hashes for ten common shapes match across B1 and all
  four new styles. Five whole prop GLBs (Sconce, Barrel, Crate, Table, Bench) and
  seven PNGs are identical across the new styles. No size saving is subtracted
  from the reported per-style totals. Independent arch inspection found no
  vertices inside the required ordinary or boss aperture rectangles.
- The old B2 puddle GLB is rejected as a winding negative control; final B2 passes.
  The exported wet material records normal scale approximately .04 and roughness
  approximately .10, confirming the glTF conversion retains those settings.
- Final exported-asset room/detail reviews completed through Cycles CPU. Nineteen
  retained first/intermediate/final PNGs provide the feedback sequence; all are
  1280×720. Python syntax compilation and `git diff --check` passed.

An early combined iteration was terminated with exit 143 during B4 re-import;
the log did not identify a cause. A subsequent complete all-style run and all
fresh per-style repeats exited 0. An initial wet-material export also exposed
an ORM filename collision: overriding only roughness caused the glTF exporter
to repack a shared image under its old name. Wet now shares albedo/normal only
and uses constant roughness/metallic factors without that AO/ORM export path.
The final export has no unresolved Python exception. Existing Blender sampler
warnings and Blender-6 deprecation warnings remain, as in the B1 pipeline.

Evidence is under this attempt's worker-output `library/`: the 19 PNGs, five
export manifests, `export-metrics.json`, `b1-byte-identity.json`,
`repeat-checks.json`, `reuse-checks.json`, and compact validation/render logs.
Generated GLBs, textures and renders are not committed. No memory projection,
shared header, schema, generated map or file outside the owned source paths is
changed.

**Checks not run / concrete prerequisite:** Unreal import/material conversion,
native binding, actual map regeneration, arrival/nav/collision checks, build,
cook/package/play, in-game screenshots and performance. These require the later
codex-sol integration task; they are outside this Blender-only ownership, not
claimed as passed. Windows remains deferred. G5 and verified task acceptance
belong to the integrator. Next task: import/deduplicate these manifest-backed
assets, extend style-aware fitting/materials with the Church/B3 notes above,
explicitly bind the carpet and catalog extras as appropriate, and judge all four
real maps under their actual lighting and gameplay camera.
