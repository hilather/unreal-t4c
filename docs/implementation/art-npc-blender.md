# W5-16 — scripted Blender NPC kit

**Evidence candidate for coordinator review, not a verified task or Unreal gate pass.** The Blender source builds all fifteen placed NPCs using the existing player bodies and rig, seven role outfit designs and shared props. Individual exports support visual review; two deduplicated kits support the later import task. The result is a coherent stylized improvement over the block figures, with substantial naturalistic face/cloth fidelity still missing relative to the starter concept.

## Handoff

- **Task ID:** W5-16, hub NPCs, Blender visual lane.
- **Base revision / result revision:** `e021ca86e54b7af998a771dea72a5b9dab4c7f0a` / the submission candidate containing this file. Exact candidate revision is recorded in the external attempt report and submission receipt, avoiding a self-referential commit hash.
- **Contract revision:** 1, digest `683befd2f62830e71e0a95e462b8eb64cf58e2ba04e43c7ac8e7d25daa1c2385`.
- **Owned paths / binary assets:** `artsource/npc/` and this report. Source/text only in Git; GLBs, textile PNGs, renders and validation JSONs are ignored generated output. No shared player source, game code, map, Unreal asset or project memory file was changed.
- **Behavior changed:** Adds deterministic construction, role/appearance recipes, fitted modular costumes/props, two stationary actions per NPC, GLB assemblies and import inventories, export and pose validation, and Cycles review staging. Game presentation remains pending the later import/binding task.
- **Source-backed mechanics:** None introduced or changed. Existing roles/services are read from repository evidence below. No HP/XP/loot or authentic costume claims. Jagar Kar's historical teaching role does not create a current offer; Rolph's stock remains unresolved; Uranos's bat-wing quest remains deferred.
- **Provisional tuning introduced:** All new dimensions, age/gender presentation, skin/hair/cloth factors, costume designs, props, timing, folds, weights and review lights are **Prototype presentation values**, W5-16 / LH_Prototype_v1, source URL null. Two-second clips are visual timing, not a researched rule.
- **Build/editor/cook/package/play checks actually run:** Blender construction, two shared textile bakes, 17 GLB exports; independent actual-GLB checks; all-frame reimport pose audit; two independent source reconstructions; Cycles CPU renders; Python syntax and Git whitespace/scope checks. Detailed observations below. No Unreal build/editor/cook/package/play command ran.
- **Checks not run and concrete missing prerequisite:** Unreal skeletal import, material instances, module assembly/merging, actor binding, talk transitions, actual map/camera review, GPU/draw-call cost and cooked size require the subsequent authorized `Source/`/`Content/` import task. This visual lane owns neither path. Windows packaging remains deferred; this report does not infer that the current host lacks an engine.
- **Known defects or remaining decisions:** Faces, hair, beards and cloth are miniature-like, below the concept's naturalism; stiff shoulders and sparsely folded robes remain visible close up. No lip sync or cloth simulation. Arbitrary subframe intersections and runtime transitions are unproven. Modular material/draw cost and the roughly 8 MB Unreal budget need measured review. Visual acceptance remains the coordinator's decision.
- **Next task and integration notes:** Review the images, then assign codex-sol to import the two kits, share geometry/textures/material masters, bind `DefinitionId` and replace both hub attached proxies and B1 unattached boxes. Preserve marker identity, transforms, yaw, collision and services. See [the source README](../../artsource/npc/README.md) for units, scale, bind poses, colours, module selection and validation commands.

## References and role assignments

Visual targets inspected at their original paths:

- `/home/brewerm/Downloads/lighthaven-unreal-design-and-agent-waves/lighthaven-plan/assets/concepts/starter-characters-and-creatures-concept.png`
- `/home/brewerm/.herdr-projects/unreal-t4c/scratch/captures/w5-13b/rooms/L_LighthavenTempleDistrict-room1.png`

The concept is style guidance only; no pixels or meshes were taken from it. Materials and geometry are original procedural work plus direct reuse of the repository's player source. Art direction follows [plan §2](../plan/docs/04-art-pipeline.md). No external assets or tools were downloaded.

The source audit found exactly **15 placed identities**: thirteen calls in `LHGenerateHubMapCommandlet.cpp:389–401`, plus Nevanis and Shovanis in `LHGenerateBasementAMapsCommandlet.cpp:520–521`. No extra placed `NPC.*` identities were found. All apparel, physical appearance and props below are **our design**; a role citation does not authenticate the costume. In particular, a smith-like apron on Rolph and guard-like equipment on Murmuntag are visual interpretations of merchant/trainer roles, not new occupations or services.

| NPC | Source-backed role | Assigned appearance (original design) | Evidence at base revision |
|---|---|---|---|
| BrotherKiran | Priest of Artherk; temple guide | Cream habit, oxblood stole, grey beard, ring staff. | docs/plan/research/lighthaven-evidence.md:38; Source/Lighthaven/Quests/LHQuests.cpp:23 |
| Fali | Potion merchant | Sage coat, russet lapels, linen wrap, three ceramic flasks. | docs/plan/research/lighthaven-evidence.md:45; Source/Lighthaven/Services/LHServiceCatalog.cpp:30 |
| Iraltok | Fire Dart teacher | Bald fire scholar in clay red, ochre shoulder cape and copper-brown beard. | docs/plan/research/character-rules-evidence.md:64; Source/Lighthaven/Services/LHServiceCatalog.cpp:22 |
| JagarKar | Town resident; historical martial teacher, no current offer | Broad ochre tabard, leather shoulders, sheathed sword. No guard or service added. | docs/implementation/art/placeholders/README.md:668; docs/implementation/services.md:23; docs/implementation/ruleset-bible-v1.md:405,411 |
| Kalastor | Dodge / Archery trainer | Moss hood and fitted woodland jerkin with back bow and quiver. | docs/implementation/world-ledger.md:105; Source/Lighthaven/Services/LHServiceCatalog.cpp:26-27 |
| Kilhiam | Disciple of Artherk; Light teacher | Blue-grey habit, pale stole and auburn tied hair. | docs/plan/research/lighthaven-evidence.md:39; Source/Lighthaven/Services/LHServiceCatalog.cpp:20 |
| Moonrock | Temple spell trainer; Heal Light teacher | Deep sage habit, linen head wrap and prayer book; no stole. | docs/plan/research/lighthaven-evidence.md:40; Source/Lighthaven/Services/LHServiceCatalog.cpp:21 |
| Murmuntag | Attack trainer | Tall drill instructor, oxblood tabard and leaf-bladed spear; guard-like design only. | docs/implementation/world-ledger.md:105; Source/Lighthaven/Services/LHServiceCatalog.cpp:25 |
| Nevanis | Basement 1 priest and healer | Ochre habit, pale stole, white long beard, prayer book and travelling healer satchel; no ceremonial staff. | docs/plan/research/lighthaven-evidence.md:42,49; Source/Lighthaven/Quests/LHQuests.cpp:30 |
| Ortanalas | Attack / Archery trainer | Grey-haired ranger, pale tunic, short green mantle and bow. | docs/implementation/world-ledger.md:105; Source/Lighthaven/Services/LHServiceCatalog.cpp:25,27 |
| Rolph | Armour merchant; current stock unresolved | Smith-like leather apron, felt cap and belt hammer; smith trade is visual inference. | docs/plan/research/lighthaven-evidence.md:46; docs/implementation/services.md:21 |
| Samaritan | Rat-errand quest giver | Weathered ochre hood and travel cape, satchel and unarmoured boots. | docs/plan/research/lighthaven-evidence.md:41; Source/Lighthaven/Quests/LHQuests.cpp:17-21 |
| Shovanis | Basement 1 spell trainer; Dust Devil teacher | Dusty plum hooded scholar, light binding and belt grimoire. | docs/plan/research/lighthaven-evidence.md:43; Source/Lighthaven/Services/LHServiceCatalog.cpp:24 |
| Sigfried | Weapon merchant; bow/quiver vendor | Russet long coat, olive cap, back bow and sheathed display sword. | docs/plan/research/lighthaven-evidence.md:44; Source/Lighthaven/Services/LHServiceCatalog.cpp:28-29 |
| Uranos | Stone Shard teacher; bat-wing quest deferred | Teal-grey hooded elder, ash beard, brass staff and book. | docs/plan/research/lighthaven-evidence.md:47; Source/Lighthaven/Services/LHServiceCatalog.cpp:23; docs/implementation/services.md:14 |

## Measured export budget and sharing

The kit has **40 used fitted modules**: 23 in body A and 17 in body B, totalling 40,416 triangles. The reconstruction audit also includes unused A Tied hair, making 41 source templates. Both bodies reuse the player mesh construction, skin weights and exact 22-bone rest hierarchy; A/B bind lengths remain distinct. Seven outfit designs have ten body-fitted meshes. Skin, hair, cloth and trim vary by material factors, and stature is a scene-parent transform.

| NPC | Body / stature | Visible triangles | Assembled GLB bytes | Textures |
|---|---:|---:|---:|---|
| BrotherKiran | A / 1.0 | 13,120 | 1,561,748 | shared 2 × 512² |
| Fali | B / 0.98 | 11,227 | 1,415,840 | shared 2 × 512² |
| Iraltok | A / 0.97 | 11,311 | 1,421,780 | shared 2 × 512² |
| JagarKar | A / 1.035 | 10,285 | 1,327,908 | shared 2 × 512² |
| Kalastor | B / 1.02 | 11,315 | 1,414,000 | shared 2 × 512² |
| Kilhiam | B / 1.0 | 12,264 | 1,523,272 | shared 2 × 512² |
| Moonrock | B / 0.96 | 11,648 | 1,461,656 | shared 2 × 512² |
| Murmuntag | A / 1.08 | 10,179 | 1,319,368 | shared 2 × 512² |
| Nevanis | A / 0.96 | 11,672 | 1,441,612 | shared 2 × 512² |
| Ortanalas | A / 1.01 | 9,926 | 1,299,300 | shared 2 × 512² |
| Rolph | A / 1.02 | 10,286 | 1,326,068 | shared 2 × 512² |
| Samaritan | B / 0.97 | 10,743 | 1,360,744 | shared 2 × 512² |
| Shovanis | A / 0.95 | 11,961 | 1,458,996 | shared 2 × 512² |
| Sigfried | A / 1.015 | 11,116 | 1,384,004 | shared 2 × 512² |
| Uranos | A / 1.025 | 12,315 | 1,488,736 | shared 2 × 512² |

| Import inventory | Modules | Triangles | GLB bytes |
|---|---:|---:|---:|
| NPC_Kit_A.glb | 23 | 22,258 | 2,329,496 |
| NPC_Kit_B.glb | 17 | 18,158 | 2,008,540 |

The two import inventories total **4,338,036 bytes (4.14 MiB)**, including the two images embedded twice, both skeletons and four sampled clips. The fifteen assembled review GLBs total **21,205,032 bytes (20.22 MiB)**; importing all of them independently defeats sharing. The GLB validator proves exact geometry equality for every used module between named assemblies and the corresponding kit. Select modules from `manifest.json`; the complete kits contain overlapping outfit alternatives and must not be displayed directly.

Shared exported geometry/index buffer views occupy **3,337,788 bytes**, with 45,519 exported vertices across the two kits (UV/material boundaries split vertices); animation buffers occupy **114,436 bytes**. Shared embedded PNG payload is **336,529 bytes**. A planning estimate for the two 512² BC7/BC5 textures with full mip chains is approximately **0.67 MiB**. This leaves plausible room under roughly 8 MB for imported meshes, four short clips, two skeletons, master materials/instances and serialization overhead, but it is **not a measured cooked Unreal result**. Material-slot and modular draw overhead need profiling or mesh merging at import/runtime.

Textile pigment is an unlit Cycles emission bake of original procedural grain; the normal tile is a real tangent-space Cycles bake. Both are shared by the cloth-like semantic slots. Other opaque materials use untextured colour/roughness/metallic factors. There is deliberately no geometry AO bake, ORM claim, per-character texture set, translucent hair or skin microdetail. The budget favours silhouette and shared geometry over portrait detail.

## Actual validation observations

Installed Blender was **5.2.2 LTS**, hash `d13f752e3b9c`, with `--background --factory-startup`, checkout-local `XDG_CONFIG_HOME`, CPU Cycles and fixed seeds. Source construction is pinned to eight threads; render/pose work used four or eight. No installation or system change was made.

- `build.py` completed all fifteen assemblies and both kits. The emitted manifest records exact sizes, triangles, module keys, hashes, slots, bones and version.
- `validate_glb.py` exited 0 for every assembly and kit. It checked finite accessors, indices, normalized weights/normals/tangents, tangent orthogonality, valid 22-joint skins/hierarchy, UV bounds, two shared 512² images, material tint factors against `roster.json`, two 61-sample clips, exact loop endpoints, stationary floor root, scene-parent scale ancestry and canonical geometry equality to the kits.
- `validate_blender.py` exited 0: all fifteen actual exported assemblies, both clips, every integer frame 0–60 (**1,830 pose samples**), no exceptions. Root transforms and foot-head positions remained stationary; maximum evaluated endpoint delta was **0.0 m**. Each pose stayed upright and above the −0.025 m audit tolerance. Maximum bounds include props: Murmuntag's spear reaches 2.4006 m; character stature is lower. This is sampled pose evidence, not proof of collision-free garments or all subframes.
- `audit_geometry.py` exited 0 on two new eight-thread reconstructions: all **41 module hashes** matched each other and the final manifest, including vertices, topology, UVs and weights; rest bones also matched. A four-thread exploratory run had altered floating-point hashes for the remeshed heads/torsos, so cross-thread or cross-Blender binary determinism is not claimed. The final eight-thread result also matches every existing first-build module.
- Python AST parsing and `git diff --check` were run. Final scope/artifact checks are recorded in the external attempt report. No GLB, PNG, texture or `.blend` is tracked.

A combined render invocation and separate long lineup attempts exited 143 without a Blender error trace; final views were rerun separately, with a four-bounce limit for the large lineup. These interruptions are not reported as successful renders.

The reproduction commands and constraints are in [README.md](../../artsource/npc/README.md). The generated manifests are evidence of what the scripts produced, not replacement game catalogs.

## Visual feedback and remaining gap

The feedback loop corrected uneven lineup lighting, staff/header overlap and the hero camera crop around Murmuntag’s spear, coif-piercing tied hair on Fali/Moonrock, duplicated clergy silhouette (Nevanis now carries a book/satchel), and exporter-baked scale that prevented exact mesh sharing. The final evidence contains fifteen hero sheets, the complete lineup, a warm chapel-distance view and a front idle/talk comparison, all 1280×720, CPU Cycles, twelve samples plus denoise. Including the two 512² textile PNGs, the output contains **20 PNGs**, below the requested cap. All characters in the renders come from final GLB exports; floor, benches, altar, captions and lamps are Blender-only review staging.

The role families and colour blocks are substantially clearer than the old block figures. Staff/stole, potion bottles, smith apron, bow/quiver, spear/tabard and hood/mantle distinguish the major roles. Some within-family pairs still depend on colour, hair/headwear and props, especially at distance. The look is coherent, earthy and deliberately simplified. **Concept match is moderate for palette/outfit intent, low for naturalistic portrait and fabric fidelity.** Squared shoulders, straight sleeve/robe forms, sculpted locks/beards, plain skin and sparse wear remain obvious in hero views. Close review also found small pale belt flecks on the ranger jerkin, deep dark mantle interiors, and an angular fold near Shovanis’s lower hem. The chapel view retains broad role separation, while facial detail and the small potion bottles contribute little at 12 m. This should not be reported as fully matching the concept or receiving final visual acceptance.

The sampled actions are floor-rooted breathing/nodding and a modest left-hand gesture; they are not conversational facial animation. Held props stay in the right hand, and the geometry has no cloth simulation. Contact/intersection-free transitions, current in-game lighting, actor rotation, target-ring/occlusion readability, service interaction and shader/cook cost still require the authorized import task. No screenshot here is an Unreal capture.

## Evidence location

Ignored working outputs: `artsource/npc/output/`. Reviewable generated assets and reports are copied to this attempt's `library/`:

`/home/brewerm/.herdr-projects/unreal-t4c/.state/worker-output/attempt-cc299b23618488b4bdb864e7159e6a3f8eb3a8caf6fe2e6c62f1383a8dfe268e/library/`

The library includes fifteen assembled GLBs, two kits, the two textile maps, eighteen render PNGs, manifest/budget/validation JSONs, and a SHA-256 manifest. Its final byte count and submission revision are recorded in the external `report.md`. Generated binaries are local evidence, not committed art assets.
