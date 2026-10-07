# V-01 — Graybox layout specifications

Design candidate dated 2026-10-07 for W3-01, W3-02 and W3-03, based on revision `4e49fa8bad7f9fecaa57fdc9cfc70ac0084ae1c7`. These documents specify enough geometry to begin blocking out five separate maps. They are not approved transforms, historical tile reconstructions, implemented levels, or evidence that G0/G3 passed. Only the coordinator may freeze shared IDs/contracts after review.

| Map / proposed area ID | Specification | Builder |
|---|---|---|
| `L_LighthavenTempleDistrict` / `Area.LighthavenTempleDistrict` | [Church and service district](hub-temple-district.md) | W3-01 |
| `L_TempleB1` / `Area.TempleB1` | [B1](b1.md) | W3-02 |
| `L_TempleB2` / `Area.TempleB2` | [B2](b2.md) | W3-02 |
| `L_TempleB3` / `Area.TempleB3` | [B3](b3.md) | W3-03 |
| `L_TempleB4` / `Area.TempleB4` | [B4](b4.md) | W3-03 |

## Evidence and confidence

The governing local references are [map observations](../map-reference-notes.md), [art/scale proposals](../art-register.md), [world ledger](../world-ledger.md), [identity contracts](../contracts-v1.md), [world design](../../plan/docs/03-world-and-encounters.md), [architecture §6](../../plan/docs/02-unreal-architecture.md#6-church-basement-areas-and-enemy-lifecycle), [art pipeline](../../plan/docs/04-art-pipeline.md) and [Wave 3](../../plan/docs/05-agent-waves.md#wave-3--graybox-the-full-church-to-basement-route). No external page was re-fetched. Roles and floor rosters come from these ledgers, not from colors or figures in a map.

Read-only package root `P` = `/home/brewerm/Downloads/lighthaven-unreal-design-and-agent-waves/lighthaven-plan`.

| Evidence | Image opened during this task | Use |
|---|---|---|
| M5 | `P/assets/references/maps/LighthavenClassic.png` | Church, service positions and winding sand route |
| M1 | `P/assets/references/maps/LHDungeon1.png` | Four chambers, healer branch, Level 2 stair |
| M2 | `P/assets/references/maps/LHDungeon2.png` | Ring, optional branches, dogleg and Level 3 stair |
| M3 | `P/assets/references/maps/LHDungeon3.png` | Circuit-like regions, partitions and Level 4 arm |
| M4 | `P/assets/references/maps/LHDungeon4Classic.png` | Branch hall and labelled Balork complex |
| C-env | `P/assets/concepts/temple-and-basement-concept.png` | Modest masonry/timber and selective warm-light direction only |

Every image remains outside the repository. No image is an approved runtime texture. Source historical patch and rights remain unresolved as recorded by W0-04.

Each file uses these confidence codes, applying separately to the feature and its reconstruction:

- **V — clearly visible:** a readable label, shape, furnishing or floor region in the opened source; not proof of collision or original-client travel.
- **I — interpreted:** inferred continuity, region correspondence or meaning where labels/walls do not settle it.
- **P — invented for playability:** an authored prototype proposal, provenance `V-01 / LH_Prototype_v1 / 2026-10-07`, source URL null. All exact coordinates, dimensions, heights, doorway cuts, NPC standing positions, spawn positions, safety volumes and light placements are P even in a row whose source feature is V.

These are document confidence codes, not new native provenance enums. Import proposed metrics as Prototype only after review; retain missing/disputed historical evidence separately. `V/P` means visible feature, invented metrics. Tables answering `needs-visual-check` are evidence candidates for the ledger owner; V-01 does not mutate the world ledger.

## Grid, coordinates and construction

**All dimensions here and in the area files are prototype proposals, not recovered source measurements.** Use one layout unit **U = 100 Unreal cm**, with a 1 U construction grid and 2/4 U wall modules. Decimal coordinates deliberately permit off-grid clearances. There is no established “one source tile = one U” conversion.

Plan +X projects toward image down-right; +Y projects toward image up-right; +Z is height. This is an authored local orientation, not a historical compass. Each map has its own origin. A coordinate `(x,y,z)` becomes `(100x,100y,100z)` cm in its local Unreal map frame; an `(x,y)` point uses the stated floor Z. Yaw convention: +X = 0°, +Y = 90°, −X = 180°, −Y = −90°. Portal facing means arrival view direction, not a guessed stair ascent direction. Never convert the approximate source-pixel anchors in the evidence notes directly into Unreal transforms.

Room bounds describe clear floor inside the collision wall face. Lay walls outward from those bounds (default thickness 0.2 U, full collision height 4 U, P); cut only the listed apertures. A shared wall is built once. Corridor centerlines include their stated width, square unobstructed turn pads and flush floor joints. Continue openings through the whole wall thickness. Nearby rooms/corridors without a listed edge remain separated: no shortcut across a black void, no diagonal corner squeeze. Every gap/void requires blocking collision, not merely a dark material.

| Ruler / clearance | Proposal inherited from W0-04 | Application |
|---|---|---|
| Player | 1.75 U standing height; gameplay proxy about 0.7 U wide × 1.8 U tall | Appearance range must not alter door eligibility |
| Ordinary routes | Doors ≥2.4 U wide ×3 U high; corridors ≥3.2 U; stairs ≥3 U; landings ≥3.2 ×3.2 U | Unobstructed after frames, props and handrails; larger per-file clearances override |
| Common large proxies | Spider leg span 1.7 U; Giant Bat wingspan 1.7 U; Atrocity arm span 1.8 U | Use each floor's actual largest resident; clear visible limbs at corners, not only capsule |
| Full-spread Balork | Wings 4.4 U; horns 3.1 U; body proxy 2 U wide; polearm about 4 U | B4 explicitly defines boss navigation extent |
| Required Balork routes | Doors ≥5 U ×3.6 U; corridors/stairs ≥5.5 U; turns/landings ≥6 ×6 U | No assumed folded-wing animation; ordinary routes outside boss extent stay distinct |

Floor Z is local, generally 0. Separate maps do not require physically stacked world elevations. Each file identifies any short cosmetic stair/ramp approach and its slope/endpoints. Subtract each listed stair/ramp footprint from the flat floor slab, then use a smooth collision ramp under decorative treads; do not leave a flat collision cap over a descending ramp. Continuous traversal, stair headroom and actual walkability remain untested. No unseen roof or wall elevation is historical evidence.

## Identity and travel contract

`Area.X::LocalId` below is human-readable shorthand for `FLHEntranceId{Area, LocalId}`. The `::` is not a new serialized naming convention. Room labels such as `B1.Entry` are candidate design IDs; they are not entrance objects or runtime entity identities.

| Source entrance | Paired destination entrance | Required reverse edge |
|---|---|---|
| `Area.LighthavenTempleDistrict::Temple.Descent` | `Area.TempleB1::Entry` | B1 Entry → Temple.Descent |
| `Area.TempleB1::Descent` | `Area.TempleB2::Entry` | B2 Entry → B1 Descent |
| `Area.TempleB2::Descent` | `Area.TempleB3::Entry` | B3 Entry → B2 Descent |
| `Area.TempleB3::Descent` | `Area.TempleB4::Entry` | B4 Entry → B3 Descent |

`Area.LighthavenTempleDistrict::Temple.SafeSpawn` is the proposed creation/recovery entrance, not an extra inter-floor portal. B4 has only the return link. There are no cemetery, cave, fifth-floor, service-teleport or post-boss shortcut edges.

Each area specifies a departure interaction/trigger separately from its arrival transform and reserved safe floor. Use explicit interaction or rearm only after leaving the trigger to avoid immediate bounce-back on arrival. Reserve at least 6 ×6 U around each proposed arrival unless the file supplies a larger region. Keep spawn anchors, patrols, damage reach, corpse clutter and transient blockers out of that space. Empty floor alone does not prove safety: W3/W4 must validate enemy leash, ranged attacks, arrival loading and failure recovery. No invulnerability duration or combat-range constant is invented here.

W3-04 owns stable portal instance GUIDs, the two explicit directed travel records per pair, source snapshot, destination validation, arrival save and recovery. On failed destination load, recover the validated source checkpoint; do not quietly send every failure to an unvalidated origin. SafeSpawn remains a separate reviewed hub fallback. Validate the paired return direction and final facing in-game.

Encounter aliases in the area files are stable design references. W3/W4 must mint and persist distinct SpawnSlot GUIDs and record alias→GUID mappings; do not hash actor labels, list offsets or coordinates into identity. Save identity remains area + authored slot GUID + life generation. NPC/object/portal instances receive their own GUIDs, not encounter slot identities. Case-only aliases are forbidden. Moving an anchor preserves its assigned identity; duplicating an actor creates a fresh one.

## Population and safety boundaries

| Floor | Existing prototype slot budget carried forward | Total |
|---|---|---|
| B1 | Brown Rat 12; Bat 3; Green Slime 2 | 17 |
| B2 | Brown Rat 6; Bat 3; Green Slime 3; Giant Bat 4; Undead Bat 4; Giant Spider 3; **Dungeon Bat 2 provisional** | 25 |
| B3 | Brown Rat 4; Green Slime 3; Giant Bat 4; Goblin 6; Goblin Warrior 3; Atrocity 2 | 22 |
| B4 | Brown Rat 3; Green Slime 2; Giant Bat 4; Atrocity 3; Balork 1 | 13 |

The 77 candidate positions instantiate existing world-design budgets; counts and positions are not historical spawn evidence or a requirement for simultaneous combat. B2 Dungeon Bat floor remains provisional and Undead Bat uses only the selected B2 profile. Hub has no hostile encounters. No placement of skeletons, zombies, mummies or tarantulas is authorized. HP/XP/loot/aggro/respawn rules stay with existing rules and world owners; chest labels are not spawn locations. Existing renewable-rat policy is still needed to reach fifteen kills from twelve initial B1 slots.

## Presentation and builder checks

Use W0-04's proposed elevated view: pitch −55° (review 50–60° downward), diagonal yaw, boom 12 U (review 9–18 U), focus 0.9 U above ground, initially 45° vertical FOV with engine-axis confirmation. This document supplies no implemented camera. Cut away roofs and fade camera-facing wall presentation while retaining collision. Inspect both orbit limits (±45° around default) and zoom extremes.

C-env informs small-scale stone/timber, amber pools and cool ambient separation. Its courtyard roof, dock, market and single basement room do not replace M1–M5 topology. Place lights by specified landmarks, avoid opaque foreground fixtures over small targets, and preserve all clear-width measurements after dressing. Brightness/exposure values remain unset pending real camera review.

Before art lock, W3 must build and walk the candidate geometry, review an equivalent-camera overlay against the originals, exercise all service interactions on controller, verify the largest resident through each intended route, and test hub→B4→hub plus quit/reload and invalid-destination recovery. Check all maps for duplicate IDs, then explicitly cook them. The especially uncertain B3 circuit cuts, B4 partitions and final mage doorway require review. This task performed documentation/source inspection only; no engine, UHT, navigation, editor, cook, package or play validation is claimed.
