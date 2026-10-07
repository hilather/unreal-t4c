# W0-04 — map reference notes

Visual inspection date: 2026-10-07. Inspector opened **all five original map PNGs** below with the image viewer at original detail. No map image was copied, edited, generated, or imported into the repository. Dimensions were also read with ImageMagick `identify`.

Package root, abbreviated **P** below: `/home/brewerm/Downloads/lighthaven-unreal-design-and-agent-waves/lighthaven-plan`.

| Evidence ID | Opened package-relative path | Image dimensions |
|---|---|---|
| M1 | `assets/references/maps/LHDungeon1.png` | 1399 × 845 px |
| M2 | `assets/references/maps/LHDungeon2.png` | 1999 × 1141 px |
| M3 | `assets/references/maps/LHDungeon3.png` | 1757 × 1226 px |
| M4 | `assets/references/maps/LHDungeon4Classic.png` | 1230 × 804 px |
| M5 | `assets/references/maps/LighthavenClassic.png` | 2705 × 1848 px |

All paths resolve beneath **P**, outside cooked content. Per-file provenance and SHA-256 verification are in [art-register.md](art-register.md). The host's map index is <https://t4cfantasy.com/Bible/classic/Maps.php>; the package's source ledger is [lighthaven-evidence.md](../plan/research/lighthaven-evidence.md). This inspection did not revisit remote pages. Neither these images nor the host's “Classic” label establish an exact client/server patch, original cartographer, or commercial reuse permission.

## Reading conventions and evidence limits

- **Clearly visible** describes pixels or readable annotations in the opened file. **Interpretation** describes a proposed geometry, route, or gameplay meaning. **Unreadable / unresolved** identifies what the picture cannot settle.
- Positions use **image left/right/up/down**, not compass directions. Approximate pixel anchors `(x, y)` start at the image's top-left. They identify regions for another reader; they are neither surveyed coordinates nor proposed Unreal transforms. Do not infer centimetres, slope, step height, collision, or world compass bearing from them.
- Stair destinations inferred from the floor sequence are marked separately from the actual printed labels. The stair sprites do not provide a reliable physical ascent vector or paired facing orientation. A view of treads is not evidence that a portal goes up rather than down.
- Black outside the dungeon floor and within its holes is absent depicted floor. It is not a shortcut. Cutaway wall tops can obscure a doorway's threshold; the visible floor silhouette alone does not certify navigation clearance.
- Region names below are descriptive W0-04 reading aids, not adopted room IDs or a replacement for the integrator's world ledger. No enemy placement, safe-room guarantee, chest rule, or measured player clearance is established by the maps.

## M1 — Lighthaven Dungeon Level 1

### Clearly visible

Four principal rectangular-in-plan chambers are visible in total: a middle chamber and three outer chambers. The lower-left chamber has `Entrance`; the upper-left chamber has `Nevanis` and `Shovanis`; the lower-right chamber has `Level 2`. All use warm brown cutaway masonry, repeated brown floor tiles, scattered small dark props/debris and pale patches. The title reads `Lighthaven Dungeon Level 1`.

| Region | Approximate image location | Visible layout and connections |
|---|---|---|
| Entrance chamber | Centre `(310, 660)`; stair near `(273, 607)` | Broad chamber, without a large internal wall. A narrow passage leaves its image upper-right side and runs up-right to the middle chamber's lower-left side. |
| Middle chamber | Centre `(681, 487)` | Broad room with small projecting side bays. Separate narrow passages connect its lower-left edge to Entrance, its upper-left edge to the NPC room, and its lower-right edge to the Level 2 room. The three arms are visually distinct. |
| NPC chamber | Centre `(279, 268)` | Broad room, with `Nevanis` around `(221, 291)` and `Shovanis` around `(325, 239)`. Passage exits the room's lower-right side toward the middle room. There is no drawn direct passage to the Level 2 chamber. |
| Level 2 chamber | Centre `(1119, 690)`; stair/label near `(1120, 655)` | Passage arrives from image upper-left. A small inner masonry enclosure surrounds/partly screens the stair area. Walkable-looking floor is drawn around the enclosure; the exact enclosure entrance is partly hidden by wall and label. |

The map labels `C1`, `C2`, and `C3` appear beside small fixtures/patches, including multiple instances of a code. They refer to the printed item legend, not to monster spawn IDs. The NPC room has `C1`; the middle room has two `C2` labels; the descent room has `C1` and two `C3` labels.

### Interpretation for the world ledger

The three-branch topology in the plan is strongly supported: **Entrance ↔ middle ↔ NPC chamber**, and **middle ↔ Level 2 chamber**. A visit to the labelled NPC room need not pass through the Level 2 room. That geometric separation is useful for a healer branch; its actual combat safety is an implementation choice.

| Portal reading | Image anchor | Destination / direction confidence |
|---|---|---|
| `Entrance` stair | `(273, 607)` | Printed label visible; pairing upward to church follows the known floor sequence, not a printed `Church` label. Approach from the open chamber to its upper portion. |
| `Level 2` stair | `(1120, 655)` | Printed destination visible; descent from B1 to B2 is high-confidence sequence interpretation. Corridor arrival is from upper-left; precise tread-facing/trigger location is unresolved. |

### Unreadable / unresolved

No named world cardinal directions, map grid, dimensions, enemy roster, spawn coordinates or NPC interaction radii are printed. The small dark central furnishings cannot all be identified reliably. The inner Level 2 stair enclosure's exact doorway and path width require tracing and blockout review. The image does not establish whether the label positions are exact NPC standing locations.

## M2 — Lighthaven Dungeon Level 2

### Clearly visible

The title reads `Lighthaven Dungeon Level 2`. The lower-left `Entrance` is separated from the upper-right `Level 3` stair by a room with an internal void, a central region, a bent narrow corridor, and a long hall. Optional-looking rooms extend toward image left/up-left. Brown masonry remains the dominant kit; dark red stains are more conspicuous than on M1.

| Region | Approximate image location | Visible layout and connections |
|---|---|---|
| Entrance chamber | `(259, 985)`; stair `(228, 936)` | Lower-left room with `Entrance` and `C2`. A narrow passage leaves image upper-right toward the ring room. |
| Ring / void chamber | `(559, 835)` | Floor surrounds a conspicuous black rectangular-in-plan hole near `(557, 841)`, bounded by low walls. Two `C1` annotations are on opposite sides. The route continues from the room's upper-right side through a narrow neck toward a small `C3` bay. |
| Central junction and lower bay | Main room `(1000, 639)`; `C3` bay `(801, 722)` | Large `C4` chamber with an image lower-left projection carrying `C3`, reached from the ring. Its image upper-left connection opens into another broad room; its upper-right side meets the bent corridor. |
| Upper-left junction room | `(645, 494)` | `C1` annotation. Passage from its lower-left side reaches a leftmost dead-end room. A separate passage from its upper-right side reaches the upper branch. Its lower-right portion meets the central `C4` room. |
| Leftmost dead end | `(297, 611)` | One visible connecting passage, toward image upper-right. Marked with two `C5` labels and one `C2`. |
| Upper branch | `(884, 320)` | An elongated hooked region, with `C3` in its upper-left end and `C2` near its lower-right portion. It is reached by the narrow passage running up-right from the upper-left junction room. The close approach to the large upper-right hall is separated by black space/wall; no cross-connection is drawn there. |
| Bent corridor | From about `(1115, 603)` through `(1300, 507)` and `(1090, 414)` to `(1203, 349)` | Narrow zigzag around a large black gap. It connects the central region to the lower-left side of the long upper-right hall. Preserve its changes of direction. |
| Long upper-right hall | Centre `(1382, 342)` | Large uninterrupted rectangular-in-plan stretch with a narrower continuation toward image lower-right. It connects to the terminal stair chamber, rather than ending at the `C2` upper branch. |
| Terminal stair room | Centre `(1785, 551)`; stair `(1786, 519)` | `Level 3` label on a stair near the upper-left interior part of the final room. The hall approaches from image upper-left. |

### Interpretation for the world ledger

The main reading is **Entrance ↔ ring room ↔ central junction ↔ bent corridor ↔ long hall ↔ Level 3 room**. The central junction also leads to the upper-left junction, which branches to the leftmost room and hooked upper branch. This supports the plan's optional branch structure. The ring has visible floor on both sides of its central hole; representing that hole as filled floor would lose the local choice of route.

`Entrance` near `(228, 936)` is interpreted as the return to B1. `Level 3` near `(1786, 519)` explicitly names the onward destination. The semantic down/up distinction comes from the floor sequence; the picture does not certify stair-facing orientation. The upper `C2` branch should not be connected to the long hall merely because their walls almost meet on the image.

### Unreadable / unresolved

The stair enclosure/door threshold and some narrow mouths are partly masked by wall height. Exact widths, line-of-sight obstructions and whether every apparent floor neck is freely traversable remain untested. The red stains and pale patches do not identify enemies or guaranteed loot. The map contains no named NPCs.

## M3 — Lighthaven Dungeon Level 3

### Clearly visible

The title reads `Lighthaven Dungeon Level 3`. `Entrance` appears in the lower-middle chamber, and `Level 4` at the far image upper-right end of a narrow terminal arm. Broad connected-looking regions wrap around a large black central gap. The left/lower-left region contains more internal partitions than M1–M2. The longest upper-left arm terminates in a broad room; no next-floor label appears there.

| Region | Approximate image location | Visible layout and connections |
|---|---|---|
| Entrance and lower-middle region | Stair near `(967, 855)`; open region extends down-right toward `(943, 1090)` | `Entrance` and `C1` in the upper part, `C2` toward image left. Partial wall ends divide the stair area from the broad lower circulation space, without enclosing it as an isolated square room. |
| Lower-left connection | `C11` area around `(701, 1102)` | Broad floor neck connects the lower-middle region to the lower-left complex. It skirts the lower edge of a smaller black opening. |
| Partitioned lower-left complex | Approximate span `(90, 780)` to `(744, 1164)` | `C1`, `C3`, `C5` and `C11` annotations; a conspicuous small interior `C3` enclosure near `(483, 987)` and an open outer route around it. Several wall stubs divide adjacent spaces. The lower-left `C5` room has a large red floor patch. |
| Western / middle-left rooms | Approximate span `(15, 516)` to `(786, 847)` | Broad western room labelled `C3`, smaller upper-left recesses and wall baffles, and a central connecting region labelled `C3` near `(527, 577)`. A small image-right projecting bay below that region has no destination label. |
| Upper-central region | `(829, 488)` | Broad room with `C1` and `C3`. Open-looking connection continues down-left to the western region; a narrowing extension bends toward image lower-right. |
| Upper-left terminal branch | End near `(353, 317)`; intermediate `C2` near `(531, 415)` | A long broad arm leaves the upper-central region toward image upper-left. Its far end has `C1`; an intervening partial wall divides its near portion. No separate stair is visible at its endpoint. |
| Upper/right bend around central gap | Roughly `(1082, 604)` to `(1043, 700)` | Narrower offset route from the upper-central region down-right, then around the central black gap toward the east room. Small wall stubs prevent treating the whole bend as a single open rectangle. |
| East room and branch hinge | `C2` around `(1151, 725)`; hinge around `(1385, 776)` | Large room with an interior walled baffle/box carrying `C2`. An open-looking neck connects toward the right-side branching region. |
| Lower-right return arm | `C5` near `(1192, 1011)`, `C4` near `(1299, 1065)`, `C6` near `(1552, 841)`, `C3` near `(1641, 876)` | Long diagonal region down-left of the far-right arm, with multiple partial wall dividers. It connects visually back toward the lower-middle region, forming the eastern portion of the broad circuit. |
| Level 4 terminal arm | Stair near `(1597, 687)`; endpoint around `(1624, 679)` | Narrow arm extends image upper-right from the east-side hinge. `Level 4` is printed above the stair. |

### Interpretation for the world ledger

There is a credible **broad circuit** from the entrance/lower-middle region through the lower-left complex, western rooms, upper-central region, eastern bend/room, lower-right arm and back to the lower-middle region. This is a region-level reading of continuous depicted floor; **the exact door-by-door cycle is not validated**. Preserve room to test this circuit during tracing, rather than locking a linear corridor now.

The upper-left arm reads as a dead-end branch, while the upper-right narrow arm is the onward descent. The `Entrance` near `(967, 855)` is interpreted as the return to B2; `Level 4` near `(1597, 687)` explicitly names the onward destination. The final approach to Level 4 goes image up-right. Neither annotation fixes the physical facing of a spawned player.

### Unreadable / unresolved

Wall overlap and thin openings make individual doorway decisions less certain than on M1. In particular, trace the lower-left `C3` enclosure, western partition apertures, east `C2` baffle, and the lower-right series of dividing walls before adopting a room graph. No map-only evidence establishes one-way passages, locks, movement through wall stubs, enemy species, or spawn positions. Small floor fragments are too coarse to identify as specific items or remains.

## M4 — Lighthaven Dungeon Level 4 Classic

### Clearly visible

The title includes `Classic`. This image uses a darker gray-brown floor and much more conspicuous orange wall torches than M1–M3. Red stains, pale rubble/bone-like clusters and dark debris are scattered across the floor. There is a label `Balork` in the broad upper-right complex; no onward `Level 5` label is visible.

| Region | Approximate image location | Visible layout and connections |
|---|---|---|
| Entrance chamber | `(210, 680)`; stair near `(184, 655)` | Small lower-left chamber labelled `Entrance` and `C3`. Passage exits toward image upper-right to the central hall. |
| Central hall | `(554, 501)` | Broad room with a dark irregular central floor feature. Separate narrow branches go image upper-left and lower-right; its upper-right side connects toward the Balork complex. |
| Upper-left side chamber | `(237, 347)` | Broad dead-end room, labelled `C3`, connected by a narrow passage to the hall. Torches line the far walls. |
| Lower-right side chamber | `(894, 667)` | Connected from the central hall by a narrow passage. A long internal wall/baffle divides the left/near section from the larger floor area. Two `C1` labels and pale standing shapes are visible. |
| Balork complex | Main room around `(785, 332)` | Large upper-right region, `Balork` near `(780, 367)` and `C5` in its upper portion. A stepped raised fixture with a vertical element stands near `(824, 325)`. Large pale clusters occupy the upper area. |
| Right-edge subdivisions | Roughly `(976, 441)` and `(1092, 366)` | Internal walls subdivide the complex into a lower/right `C4` area and a further upper-right rectangular enclosure with several pale upright shapes. Openings/terminations occur between these partitions; not all thresholds are clear. |

### Interpretation for the world ledger

The principal reading is **Entrance ↔ central hall**, with hall branches to the upper-left side chamber, lower-right side chamber, and Balork complex. This corroborates the package's broad topology. The raised fixture reads as an altar/pedestal-like object; its exact function is not established. The `Balork` annotation locates the encounter region, not the boss's collision footprint or exact spawn transform.

The `Entrance` stair near `(184, 655)` is interpreted as the return to B3. There is no visually documented further descent. The brighter pale upright shapes could be decorative/statue-like forms or sprites; do not identify them as living skeleton enemies. No additional species is legibly labelled here.

### Unreadable / unresolved

The central dark floor feature's function, the rightmost enclosure entrances, and precise circulation past the lower-right side-room wall require tracing/graybox review. The map does not establish a closed boss arena, encounter triggers, combat phases, door locks, or retreat restrictions. Its darker palette and drawn torches are visual evidence, not a measured Unreal lighting setup.

## M5 — Lighthaven Classic town

### Clearly visible: church and services

The town is on the lower-right/main landmass of the image, surrounded by water and pale shoreline. A river channel separates it from the wooded upper-left mainland. The church is in the image upper-right part of the **built-up town**, not at the upper-right corner of the full picture; the mage building and Stonehenge are farther up-right in the water area.

| Landmark / annotation | Approximate image anchor | Visible placement and useful reading |
|---|---|---|
| Church nave | `(1900, 872)` | Large gray-tiled cutaway interior, red carpet on the main aisle and several wooden pews/benches to either side. Modest rectilinear building, without evidence of a cathedral-scale roof. |
| `Brother Kiran`, `Kilhiam`, `Moonrock` | `(1860, 823)`, `(1864, 855)`, `(1868, 882)` | Three readable names within the church. Labels overlap the furnishings, so exact standing positions should not be lifted from text centres. Their priest/trainer roles come from the text evidence, not the pixels. |
| `Dungeon` | `(1716, 869)` | Small wing/space on the image-left side of the church. No destination number printed here. This is spatially distinct from the cemetery `Crypt` and `Lighthaven Cave`. |
| `Samaritan` | `(1706, 927)` | Exterior church-side approach, image below-left of the nave near the dungeon-side wing. |
| `Sigfried` | `(1702, 783)` | Separate brown-walled building above-left of the church, near the town's upper shore. An interior is visible; the weapon-merchant role is text evidence. |
| `Fali & Rolph` | `(1536, 916)` | Shared labelled building below-left of Sigfried and left of the church/Samaritan approach. Visible connecting streets/ground between these buildings. Potion/armour merchant identities are text evidence. |
| `Jagar Kar`, `Ortanalas` | `(1649, 993)`, `(1651, 1025)` | Same white-plaster/timber cutaway building immediately below-left of church. This anchors the town training building referenced by the plan; which training each offers requires the rules/NPC ledger. |
| `Murmuntag` | `(1076, 1158)` | Label in the open town ground farther image lower-left of the church, below `Darkfang` and left of Kalastor's building. The map shows the name and nearby large gray form; it does not identify the form's function. Attack training is established by the package's character-rules text evidence, not the annotation itself. |
| `Kalastor` | `(1362, 1174)` | Label over the lower portion of the brown-walled building below `A Dark Figure`, farther image lower-left of the church and below-left of the Jagar Kar/Ortanalas building. Dodge and Archery training are established by the package's character-rules text evidence, not the image alone. |
| `Uranos`, `Iraltok`, `Lothan` | `(2053, 482)`, `(2245, 453)`, `(2132, 510)` | All are labelled within the separate multiroom mage building above-right across water. `C11` appears near `(2123, 449)`. Beds/partitions are drawn. Building elevation/roof form cannot be recovered from the cutaway. |
| `High Priest Gunthar`, `Araknor` | `(2002, 782)`, `(2131, 810)` | Labelled buildings/positions above and to the right of the church. Their presence does not justify substituting either for the source-specified church/B1 healer. |
| `Beggars Corner` | `(1549, 831)` | Labelled shelter/camp-like group on the shore above-left of the church and beside Sigfried's building. |
| Cemetery `Crypt` | `(433, 997)` | Glowing entrance in wooded graveyard area far image left of the town services. Separate from the church wing. |
| `Lighthaven Cave` | `(642, 1266)` | Dark opening in a rocky ridge below the cemetery and image lower-left of the service district. Separate from both Crypt and Dungeon. |

The image clearly places the church and all four local service destinations in one compact group: Sigfried above-left, Fali/Rolph left, Jagar Kar/Ortanalas below-left, and church NPCs inside. There are visible streets/ground between them. No internal shop doorway or collision clearance is certified by those open-ground pixels.

The [character-rules evidence](../plan/research/character-rules-evidence.md#starter-skills-and-spell-acquisition) assigns Attack to Murmuntag/Ortanalas, Dodge to Kalastor, and Archery to Kalastor/Ortanalas. Preserve a ground route to Kalastor for the distinct Dodge service, plus the labelled Attack/Archery options; this extends the service circuit farther down-left through town. The map does not show training menus, prices or prerequisites.

### Clearly visible: mage approach and shoreline

A **pale sand route is actually drawn across the water**, winding between the town's upper-right shore and the mage building's surrounding sand. It is not necessary to invent a direct bridge across the nearest blue gap to explain a route in this image.

Approximate reading waypoints, provided for reinspection rather than pathfinding coordinates:

1. Town's upper-right shoreline/neck, near `(2275, 784)`, to the eastward bend near `(2406, 719)`.
2. The strip bends back image left along the band near `(2212, 669)` and `(2050, 654)`.
3. It reaches the branching sandy area southwest of the mage building near `(1888, 617)`.
4. Sand continues up/right into the inner shoreline around `(1966, 573)` and along the building's lower edge near `(2111, 579)`; a longer outer sand arm also bends around the image-left side of the building through approximately `(1787, 466)` and `(1898, 347)`.
5. Farther north, a sand continuation near `(2068, 329)` winds toward the island labelled `Striking Dummies` / `Stonehenge`, near `(2486, 122)`.

These waypoints trace a **visually continuous-looking shore/causeway**; they do not prove passable terrain in the original client. The inner/outer branches and the final building threshold must be traced carefully before freezing a route. The image does not label the route a bridge, contain a portal arrow, or establish teleporter use. Keep the sand route's outline as the initial reconstruction hypothesis; do not mark the mage tower as inaccessible from this picture alone.

Two distinctly drawn timber bridges occur on the river at about `(535, 758)` and `(60, 1230)`, both well to the image left of the church. They connect the broader land/river routes and are not the depicted mage-building approach.

### Other readable labels and landmarks

Outside the immediate service circuit, the image includes two `Mercenary Camp` labels on the upper-left mainland; `Stonehenge` and green `Striking Dummies` at upper-right; `Lothar's Temple` to the church's right; and the following readable town labels: `Geena`, `A Guardsman`, `Halam`, `Darkfang`, `A Dark Figure`, `Amelia`, `Mithrand`, `Isulgur`, `Markam`, `Kirlor Dhul`, `Elmert Merkiss`, `Edgar`, `Vincent Swiftblade`, `Jalus`, and `Marnet Sunim`. There are tilled fields and fenced plots below the town, timber/plaster houses, brown masonry interiors, trees, rocks, and small shore shelters. These labels establish map annotations, not mandatory Stage 1 services.

`C1`, `C2`, `C11`, and `C13` appear at buildings and have legends along the bottom edge. A short name by the right-side temple/building, ending approximately `…nisien`, is too uncertain in this inspection to transcribe confidently. A lower-left forest name appears to read `Tarnian`; its initial letter is uncertain. Keep these spellings unresolved rather than inventing an NPC record.

### Interpretation and unresolved church/service details

The red aisle and benches support a recognizable small church reconstruction. The exact church entry threshold appears near the image lower-left end of the carpet, but text and cutaway walls obscure the junction with the exterior: treat its final position as a tracing task. The `Dungeon` label locates the annex, not an exact stair tile. The map is not a roof reference.

A Stage 1 service route can plausibly keep church ↔ Fali/Rolph ↔ Jagar Kar/Ortanalas ↔ Sigfried on the existing town ground, extend down-left to Kalastor and Murmuntag, and use a longer shore/causeway trip for the mage building. This is a reconstruction proposal grounded in the layout, not a verified original-client walk. Do not relocate Uranos or Iraltok to the church. Nevanis/Shovanis are only labelled on M1, not the town map; healing and spell-teaching functions must stay linked to the text evidence.

## Readable chest legends: annotations, not adopted mechanics

These transcriptions are useful because the small `C…` labels otherwise look like encounter IDs. A slash below normalizes the ornamental separator in the images. **No chest timing, gold amount, item guarantee, ownership model, or server patch was runtime-verified.** The numbers in the town legend are observed text, not prototype tuning and not authorization to insert those values into gameplay data.

| Code | Printed item meaning | Maps where this legend is visible |
|---|---|---|
| C1 | Rusted Dirk / Ashwood Flatbow | M1–M5 |
| C2 | Rusted Dirk / Ashwood Flatbow | M1, M2, M3, M5 |
| C3 | Rusted Long Sword / Ashwood Longbow | M1–M4 |
| C4 | Steel Reinforced Club / Ashwood Reflex Bow | M2–M4 |
| C5 | Rusted Short & Long Sword / Ashwood Flatbow & Longbow | M2–M4 |
| C6 | Iron Ring / Ring of Confidence | M3 |
| C11 | Rusted Long Sword / Ashwood Longbow | M3, M5 |
| C13 | Iron Ring / Torch / Potions | M5 |

M5 also has a legible heading `Chest Respawn Timers & Gold`:

| Code | Global time, as printed | User time, as printed | Gold, as printed |
|---|---|---|---|
| C1 | 8.83 Mins. | 16.67 Mins. | 34 |
| C2 | 10 Mins. | 21.67 Mins. | 56 |
| C11 | 15.83 Mins. | 30 Mins. | 70 |
| C13 | 33.33 Mins. | 58.33 Mins. | 24 |

This qualifies the package research statement that chest timing was not established: **timing values are visibly claimed by this host map**, but their correctness, historical baseline, precise semantics and relevance to a single-player adaptation remain unestablished. Keep the distinction in the world/rules ledger.

## Map-only questions ready for the integrator

| Question | Visual result | Remaining work |
|---|---|---|
| Where does the church dungeon enter? | M5's `Dungeon` is on the church's image-left wing; M1 has a labelled lower-left Entrance chamber. | Exact church stair tile/trigger and paired player facing. |
| Are B1 healers on a separate branch? | Yes: M1 Nevanis/Shovanis chamber connects to the middle room independently of the Level 2 chamber. | NPC spawn/interaction space and safety policy. |
| Does B2 contain a central hole and optional branches? | Yes: ring chamber near entry; separate leftmost and upper hooked branches. | Door apertures, traversal widths and local ring clearance. |
| Is B3 a straight route? | No: broad depicted circuit, lower-left partitions, upper-left terminal branch and upper-right Level 4 arm. | Full room/door graph and confirmation of every circuit opening. |
| Where is Balork? | M4 upper-right complex, beside a raised fixture; M4 shows no onward floor label. | Boss transform, arena bounds and passable right-edge subdivisions. |
| Are Cave/Crypt the church dungeon? | No: all three are separately labelled and spatially separated on M5. | Keep separate world/location IDs. |
| Where are essential services? | M5 church NPCs, Sigfried, Fali/Rolph, Jagar Kar/Ortanalas, Murmuntag, Kalastor and mage-building labels are readable. | Service role/price/spell rules from text evidence, not images. |
| Is a mage-building approach visible? | Yes: winding pale sand route connects-looking town shore to mage-building surroundings, with an onward Stonehenge branch. | Trace actual sandy branches and final building doorway; validate navigation. |
| Do chest annotations establish runtime rules? | They show named items and M5 timer/gold claims. | Baseline/server meaning and runtime behavior unverified. |

## Validation boundary and next use

This is a visual evidence deliverable. No Unreal asset, collision trace, navigation test, editor session, gameplay session, build, cook or package test was run for it. The supplied task context reports no Unreal installation on this host; independent engine/toolchain verification belongs to W0-02. A Wave 3 owner should trace wall/floor boundaries on a separate working document, agree stable room/portal IDs with the integrator, and test doors, all service routes, stairs, retreat paths and largest-enemy clearance in the actual graybox. Do not treat these approximate pixel anchors as a finished level specification.
