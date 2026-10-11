# W5-12 — scripted Blender starter players

**Evidence candidate for coordinator review.** This delivers the Blender source and locally generated export/render evidence; it does not mark W5-12 verified or pass an Unreal gate. The outfit and distant readability follow the reference, but portrait fidelity is still below the concept's target.

## Handoff

- **Task ID:** W5-12, visual lane.
- **Base revision / result revision:** `a3de69ae3f6a3cee989ac4359bbdcd87f2ebe714` / the submission candidate containing this file. The exact candidate OID is recorded in the attempt's external `report.md` and submission receipt; a file cannot embed its own commit hash.
- **Contract revision:** 1; digest `fa08a87e20af8ec83c7e5cb30ac26d1e8f0f184c597a8dce2f582b1225101d65`.
- **Owned paths / binary assets:** `artsource/player/` and this file only. No tracked binary assets. GLBs, six baked textures, seven renders and validation reports are ignored local outputs, copied to the attempt's `library/` for review. No `Source/` or `Content/` changes.
- **Behavior changed:** Adds reproducible source for two clothed bodies, fitted Cropped/Tied hair, tintable skin, armatures, eight sampled actions, four attachment bones, GLB export, validation and review cameras. Existing game presentation is unchanged until the later import task.
- **Source-backed mechanics:** No rules introduced or changed. Appearance policy, palette, capsule, camera, equipment presentation and action timing were read from the current C++ sources below. No HP/XP/loot claims and no claim of authentic T4C measurements.
- **Provisional tuning introduced:** All geometry proportions, materials, UVs, folds, rig/weight choices, animation poses, light powers and artistic timing anchors are original **Prototype presentation values**, W5-12 / LH_Prototype_v1, source URL null. Existing instant damage timing is preserved as an integration constraint.
- **Build/editor/cook/package/play checks actually run:** Blender generation/bakes/export, independent GLB validation, actual-export Blender pose/construction audit, Cycles CPU renders, Python syntax and Git scope/whitespace checks. Details below. No Unreal build/editor/cook/package/play operation was performed.
- **Checks not run and concrete missing prerequisite:** Unreal skeletal import, material instances, animation/equipment binding, attached-weapon fitting, current-camera map play, transitions, and actual cooked size require the later authorized `Content/`/`Source/` import task. Windows checks remain deferred under project memory. This report does not infer an unavailable engine installation.
- **Known defects or remaining decisions:** Portrait/cloth realism is below the reference; animation is a first pass; foot sliding, subframe collisions, garment intersections and weapon attachment offsets still need runtime review. The body-specific bind poses need explicit import/retarget handling. Visual acceptance and the ~10 MB Unreal budget remain open.
- **Next task and integration notes:** Coordinator visual review, then the assigned codex-sol import/binding task (or a further visual revision if the concept's portrait fidelity is required before import). Follow the appearance, unit, socket and instant-attack rules in [the source README](../../artsource/player/README.md). Do not reapply the old procedural pose rotations to this skeleton.

## Read before authoring

The visual target was inspected at its original package path:

`/home/brewerm/Downloads/lighthaven-unreal-design-and-agent-waves/lighthaven-plan/assets/concepts/starter-characters-and-creatures-concept.png`

Only its left-hand starter pair was used for visual comparison. No concept pixels, mesh extraction, external asset download, texture projection or image-generation service entered the pipeline. Rear costume details and hairstyle variants are original extrapolations. The art direction is [plan §2](../plan/docs/04-art-pipeline.md).

Source contracts at the base revision:

| Evidence | Applied constraint |
|---|---|
| `Source/Lighthaven/Character/LHCharacterAuthority.cpp:128–158` | Twelve valid combinations; matching body/face pairs, two hair styles, three skins, one StarterLinen outfit |
| `Source/Lighthaven/Visual/Player/LHPlayerVisual.cpp:17–53` | Body widths/palette; left-hand bow, right-hand weapon, quiver, bow arrow and fallback weapon presentations |
| `Source/Lighthaven/Visual/Player/LHPlayerVisual.cpp:95–137` | Event-based impact delay; 0.18 s release, 0.2 s hit, 0.65 s death; committed-bow arrow visibility |
| `Source/Lighthaven/Framework/LHCharacter.cpp:22–46` | 35 cm capsule radius / 90 cm half-height; 12 m isometric camera; existing movement speeds |
| `Source/Lighthaven/Abilities/LHAbilityCatalog.cpp:27–35` | Current production impact delay is zero; cooldown 1.5 s |
| `artsource/creatures/build.py`, `validate_glb.py`, `validate_blender.py` | Reused bake, GLB/NLA, channel wiring, byte/PNG inspection and reimport audit patterns |

The legacy proxy does not visibly honor every face/outfit ID; this delivery follows the authority's permitted combinations. Body A/Face A is the broader masculine starter; B/B is the narrower feminine starter. Neither is a new character-selection policy. [appearances.json](../../artsource/player/appearances.json) enumerates all twelve valid records.

## Delivered scheme and measurements

Each body has `body`, `Hair.Cropped`, `Hair.Tied`, a 22-bone hierarchy and eight actions: idle, move, run, melee, bow, cast, hit, death. Hide one hairstyle after import. The two skeletons have matching names/hierarchy but different bind lengths. Skin materials swap factors, not geometry or textures. Face A is built into body A, face B into body B; the game rejects cross-pairs.

| Actual exported measurement | Body A | Body B |
|---|---:|---:|
| All triangles, including both hair variants | 13,799 | 13,799 |
| Clothed body triangles | 11,265 | 11,281 |
| Cropped hair triangles | 1,053 | 1,049 |
| Tied hair triangles | 1,481 | 1,469 |
| Visible default triangles | 12,318 | 12,750 |
| GLB bytes | 2,139,624 | 2,178,592 |
| Embedded textures | 3 × 512² | 3 × 512² |

The two GLBs total 4,318,216 bytes (4.12 MiB), including embedded images and animation. This is **not** a cooked Unreal size measurement. Budget planning uses six 512² BC7/BC5 textures with mip chains (about 2 MiB), shared material instances and one mesh per body rather than one per combination. The ~10 MB Unreal target must still be measured after import/cook, including skeleton/animation/material overhead.

The base atlas is unlit pigment; tangent normals bake the procedural surface detail. ORM red is Cycles geometric AO (7.5 cm range, 32 AO samples), green roughness, blue metallic. Actual exported occupied-UV AO statistics are in `glb-validation.json`; both contain broad nonconstant data. Tied hair is temporarily displaced during baking to avoid overlapping Cropped hair; its AO is self-occlusion only, while the body is baked with Cropped in place. Skin is an opaque tintable surface, without physical subsurface scattering; hair uses opaque sculpted locks.

Exported attachment bones are `Socket.Weapon.R`, `Socket.Bow.L`, `Socket.Arrow`, `Socket.Quiver.Back`. Bow aiming is solved in the build and baked to ordinary FK keys, including socket orientation compensation. No IK dependency or constraint is exported. Actual equipment meshes and a nocked string are not delivered here; their offsets remain import review work. Preserve `Presentation.Player.BowArrow`'s runtime visibility rule.

Melee contact is authored at frame 15; bow/cast release at frame 21, 30 fps. These lead-ins are **not current game windups**: the ability catalog resolves at zero impact delay. The importer should start at the contact/release anchor and fit recovery to 0.18 s for today's instant events. A future positive `ImpactSeconds` may use the lead-in. Never delay damage to a visual key. Death is a grounded fall sampled over the existing 0.65 s interval, followed by a hold.

## Validation evidence

Commands are reproducible in [README.md](../../artsource/player/README.md). Actual Blender was **5.2.2 LTS**, hash `d13f752e3b9c`, using `--background --factory-startup`, CPU Cycles and checkout-local `XDG_CONFIG_HOME`. No toolchain installation or system configuration change was made.

- Both final builds completed with emitted GLBs and six 512² PNGs. Duplicate internal faces produced by construction/reduction are removed by Blender mesh validation before bake/export. Narrow belt shading corners are made flat before baking where needed to keep tangent space defined.
- `python artsource/player/validate_glb.py` exited 0 on both final GLBs: finite attributes, normalized normals/tangents/weights, valid skins/joints, 8 expected clips/durations, stationary root, three loop endpoints, death hold, four named sockets, exact tint factors, atlas sharing and occupied-UV AO wiring/variation.
- `validate_blender.py` exited 0 on the final exports: 242 integer frames per body with both hairstyle vertex sets (484 total), 813 successful checks per body, zero exceptions. Floor root transforms remained exactly constant; idle remained upright; loop endpoints and grounded poses passed; the death hold stayed within the 1-micrometre tolerance. Maximum idle heights with both hair sets were 1.7981 m (A) and 1.7172 m (B); final corpse heights were 0.4345/0.4133 m. Two independent constructions per body produced identical hashes of vertices, topology, UVs, weights and bones. The audit does not prove subframe clearance or absence of self-intersections.
- Seven final PNGs were generated from the actual exported GLBs, not source-only scenes: two hero views, two gameplay-distance views, two eight-action sheets, and one grid of all twelve appearances. All are 1280×720, CPU Cycles, 16 samples and denoising. The six atlases plus seven renders make 13 delivered PNGs, below the 25-PNG cap.
- Python syntax and Git whitespace/scope checks were run. The final staged tree includes source/text only inside the two owned paths. Generated binaries are ignored and excluded from commits.

The artifact directory is the current attempt's worker output:

`/home/brewerm/.herdr-projects/unreal-t4c/.state/worker-output/attempt-5785eb0c3fcdaca405b4002a87fddf4eb8285052a80f589755ab33c719a176e6/library/`

It contains the two GLBs, six textures, seven renders, JSON validation/build evidence and a SHA-256 manifest. The working copies are in ignored `artsource/player/output/`. No screenshot in this delivery is presented as an Unreal capture.

## Visual feedback and remaining gap

Iterations corrected the initial camera crop, exposed shoulder skin, overly rope-like hair, weak collar readability, incorrect exported skin tint, bad belt tangent corners, the bow reach/orientation, and contact-sheet lighting/captions. Final exports are upright in idle and floor-rooted in the Blender audit. That observation does **not** prove that the old in-game tilted-idle issue is fixed; the old component is untouched.

The concept match is **moderate overall**. The lighter linen top, wide belt/pouch, bracers, brown trousers and tall boots are present, and the light/dark body split reads under the 12 m warm-light review camera. The male/female bodies and hair choices are subtler at that distance. Faces, hair, sleeve shapes and fabric folds remain simplified and somewhat doll-like in hero views; they do not achieve the concept's natural anatomy, loose hair or rich worn-cloth detail. This should not be described as a fully matched or final-quality portrait asset.

The pose sheets show each required state, but no whole-body mesh audit can establish correct elbow deformation, absence of all clothing intersections, planted-foot motion at arbitrary speeds, transition quality, or equipped weapon grip. Those are explicit remaining review items. No collision, gameplay numbers, actor rotation or project gate was changed.
