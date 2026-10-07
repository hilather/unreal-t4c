# Lighthaven temple research and map evidence

Research date: **2026-10-07**. Scope: T4C's Lighthaven temple and its four basement floors. This is an evidence record for a modern single-player reconstruction; it is not a claim to possess original server scripts, spawn tables or a licensed asset collection.

## Findings that determine the playable scope

- **Lighthaven is the correct starting town.** Published map indexes distinguish its **four-floor Temple Dungeon** from **Lighthaven's Cave**, the **graveyard Tomb/Crypt**, and the separate **Ancient Temple/Crypt**. Do not merge these places into one dungeon. [S1, S2]
- The requested church basement can become a complete four-floor introductory adventure, culminating in **Balork**. Floor one alone is a useful early milestone; stopping there would omit much of the documented temple enemy roster. [S3, M4]
- A reconstruction must not insert cemetery skeletons, mummies or zombies into the temple merely because the user calls it “catacombs.” The selected classic location table assigns those enemies elsewhere. [S3]
- This evidence supports a **classic-inspired baseline**, not an exact historical patch. The strongest accessible host labels pages “Classic,” but its wider index includes later additions. Neerya and other servers demonstrably change quests and rewards. Pin a chosen ruleset in implementation data and label unresolved values. [S1, S3, S7]

## Enemy coverage

The following compact matrix transcribes location facts from T4C Fantasy's **Classic Monster Weaknesses** reference [S3]. B1–B4 denote depth below the church, not the engine's world-coordinate layer. “Unknown” means the source names the temple without a floor.

| Enemy definition | Documented temple floors | Confidence |
|---|---|---|
| Brown Rat | B1, B2, B3, B4 | High within this reference |
| Bat | B1, B2 | High |
| Dungeon Bat | Unknown | Presence high; floor unresolved |
| Green Slime | B1, B2, B3, B4 | High |
| Giant Bat | B2, B3, B4 | High |
| Undead Bat | B2 | High; other guides also list B3 |
| Giant Spider | B2 | High |
| Goblin | B3 | High |
| Goblin Warrior | B3 | High |
| Atrocity | B3, B4 | High |
| Balork | B4 | High; also labelled on M4 |

**Implementation decision:** create all eleven enemy definitions. Treat the Dungeon Bat's floor placement as an explicit prototype decision until verified. Use B2 for Undead Bat in the selected baseline; do not silently combine every server's variant. Spawn counts, positions, respawn rates, aggro rules and numeric combat statistics were not established by this research. Derive prototype values separately and mark them provisional.

## Church and early NPCs

The classic NPC directory [S4] identifies:

| NPC | Location / useful identity |
|---|---|
| Brother Kiran | Lighthaven Temple; priest of Artherk |
| Kilhiam | Lighthaven Temple; disciple of Artherk |
| Moonrock | Lighthaven Temple; spell trainer |
| Lighthaven Samaritan | Outside / near temple; introductory quest giver |
| Nevanis | Temple Dungeon B1; priest |
| Shovanis | Temple Dungeon B1; spell trainer |
| Sigfried | Town weapon merchant |
| Fali | Town potion merchant |
| Rolph | Town armour merchant |
| Uranos | Mage tower; bat-wing quest NPC |

The newcomer quest guide [S5] places Nevanis in the western B1 room and explicitly describes his healing service. Map M1 places Shovanis in the same room. “Western” is historical guide terminology: use the marked map room as the location authority, because image-space upper-left and world compass directions are not equivalent.

## Quest facts and scope choices

The classic quest guide [S5] supports two immediately relevant loops:

1. **Samaritan's Errand:** accept near the temple, kill fifteen rats below it, return for **2,500 XP**. Nevanis offers a recovery point on B1.
2. **Decaying Bat Wings:** obtain wings from Undead Bats on B2, take them to **Uranos** at the mage tower, receive a **Skull Dagger and 2,500 XP**. Visiting Marsac Cred in Windhowl is optional in the guide.

The same guide puts **Tomb Raider** in the cemetery mausoleum rather than the church. That quest is outside this slice. Dark Fang's gold/healing interaction belongs to the wider town, and need not block the church prototype.

**Implementation decisions:** replace keyword-only input with selectable dialogue topics while preserving the original topic IDs. Do not carry a multiplayer turn-in cooldown into the single-player quest without a gameplay reason. If the mage tower is outside the first playable church district, store the bat-wing objective and introduce its turn-in in the town expansion; do not move Uranos into the church and present that as historical fact. A new final objective to defeat Balork is an authored slice-completion objective unless a specific historical quest chain is verified separately.

### Server variation that must stay separate

- Neerya's **Beyond the Kingdom** rat quest adds **Gustave**, ten rat corpses and a tarantula near the stairs. Its page also mentions the separate fifteen-rat Samaritan quest. This is clear evidence of additional server content, not grounds to add the tarantula to the classic B1 roster. [S7]
- The Abomination bat-wing quest independently describes Undead Bats in the temple basement and a Skull Dagger plus 2,500 XP reward, but does not specify the floor. [S8]
- A French community monster table places Undead Bats on B2 and B3; that disagreement should remain visible rather than being “resolved” by inventing a universal spawn map. [S9]

## Map inspection and reconstruction notes

Five PNGs were downloaded without modification and visually inspected. They are hosted by T4C Fantasy. The files do not establish a reusable licence or the original cartographer's identity. Their labels and artwork are useful reference material; keep them out of packaged game content unless reuse is authorized.

**Orientation rule:** the descriptions below use positions on the displayed image, not world north. Geometry is inferred from the visible image and must be reviewed during blockout. Pixel dimensions are not Unreal centimetres; choose a gameplay scale, retain connections, then test camera and navigation clearance. Do not interpret empty black pixels as traversable floor.

### M1 — Temple Dungeon B1

![Temple Dungeon B1](../assets/references/maps/LHDungeon1.png)

Visible topology: **four principal chambers**, joined as a three-branch hub. The entry chamber sits lower-left; a corridor leads to the central chamber. One branch goes upper-left to the room labelled **Nevanis** and **Shovanis**. The other goes lower-right to a room containing a small interior enclosure and the **Level 2** stair. There is no visible route directly joining the two side rooms. Preserve the healer branch as a discoverable retreat from the central combat space. Small markings identify loot locations, not enemy spawn points.

Suggested room IDs (new implementation IDs): `B1.Entry`, `B1.Hub`, `B1.Healers`, `B1.Descent`. Edges: Entry–Hub; Hub–Healers; Hub–Descent. Stair edges: church–Entry; Descent–B2.Entry.

### M2 — Temple Dungeon B2

![Temple Dungeon B2](../assets/references/maps/LHDungeon2.png)

The entry lies lower-left. Its route reaches a chamber with a central nonwalkable rectangular void, then the broad central junction. The central junction connects toward upper-left branches and toward a long bent corridor leading to the large upper-right hall. The stair labelled **Level 3** occupies the rightmost terminal room. The upper-left branch has side chambers and dead ends. The main route thus adds a small local loop/obstacle and longer navigation legs without requiring an invented maze. Trace door apertures from the full image during blockout; do not flatten the internal void or the bent main corridor.

Suggested functional regions: entry, ring chamber, central junction, optional loot branches, dogleg corridor, long hall, descent. This is a navigation plan derived from M2, not a claim about original room names.

### M3 — Temple Dungeon B3

![Temple Dungeon B3](../assets/references/maps/LHDungeon3.png)

The entry is near the lower middle. The floor has significantly more interconnected chambers, internal partitions and routes around large black gaps than B1 or B2. The descent lies at the end of the narrow upper-right extension. Preserve the broad circulation route through the western, upper-central and eastern regions, the partitioned lower-left rooms, and the long upper-left dead-end branch. A first blockout must explicitly validate all visible door openings; the screenshot does not supply authoritative collision or cell coordinates.

Suggested validation: enter from B2, reach every principal region, locate the B4 stair, return to the entry without teleporting; ensure AI can navigate the same openings. Exact doorway scale and alternate-route connectivity remain blockout verification tasks.

### M4 — Temple Dungeon B4 (labelled Classic)

![Temple Dungeon B4](../assets/references/maps/LHDungeon4Classic.png)

A lower-left entry corridor reaches a large central hall. Side branches lead to an upper-left chamber and a lower-right chamber with internal partitions. The final upper-right complex contains the **Balork** label, a central raised object/altar, and subdivisions along the right edge. The floor uses darker stone and wall-mounted warm light, visibly distinct from the upper floors. There is no next-floor stair label. Stage the boss in the labelled complex; side branches provide exploration and recovery space. Do not add a fifth floor or connect this chamber to Lighthaven Cave based on the shared name.

### M5 — Lighthaven town (labelled Classic)

![Lighthaven town](../assets/references/maps/LighthavenClassic.png)

The temple is in the upper-right portion of the built-up town on the image. Its nave is identifiable by pews, red carpet and the Brother Kiran/Kilhiam/Moonrock labels. The **Dungeon** label is at its left-side wing; the Samaritan is outside that side of the church. Sigfried is nearby above-left; Fali/Rolph and the training building are below-left. The mage tower sits on a small offshore area above-right. The graveyard's **Crypt** label and **Lighthaven Cave** are well away to the left, corroborating their separate identity.

The map also includes labelled content such as striking dummies, High Priest Gunthar and Lothar's Temple. “Classic” is the host's label; it does not certify every annotation as original-release content. For Stage 1, reconstruct the church, immediate approaches and recognizable neighbouring silhouettes; keep unrelated late/additional features outside the playable boundary.

## Reference asset manifest

Retrieval for every file below succeeded on **2026-10-07**. Original filenames retained to simplify provenance. Attribution: **T4C Fantasy, Classic Bible map collection**; underlying T4C artwork associated with the game; specific map author and reusable licence not established. Local copies are for reference review, not production-ready game assets.

| ID | Local file | Direct source URL | SHA-256 |
|---|---|---|---|
| M1 | `assets/references/maps/LHDungeon1.png` | <https://t4cfantasy.com/images/maps/i1arakas/LHDungeon1.png> | `8a47f31fa2936256816742def2e5e8ae274ddde24e10afabff751319320b2154` |
| M2 | `assets/references/maps/LHDungeon2.png` | <https://t4cfantasy.com/images/maps/i1arakas/LHDungeon2.png> | `9de7aec45987fdc770cf926f44419b9fcacb4948380988c88c5cae747adc2a30` |
| M3 | `assets/references/maps/LHDungeon3.png` | <https://t4cfantasy.com/images/maps/i1arakas/LHDungeon3.png> | `3f2d3b479fc7ce3c919d0651c7f46d885954e36064bf3ed51377a203d0b2e356` |
| M4 | `assets/references/maps/LHDungeon4Classic.png` | <https://t4cfantasy.com/images/maps/i1arakas/LHDungeon4Classic.png> | `fb910477978b37739436314ca7b3d5bbcc5f51e147c23be585650a1bfda7eeea` |
| M5 | `assets/references/maps/LighthavenClassic.png` | <https://t4cfantasy.com/images/maps/i1arakas/LighthavenClassic.png> | `e93af7ea22e3c03c5e072ecbb18d4bb4704c9e438917afbbb980372cc0f25e49` |

## Sources and evidence quality

All accessed 2026-10-07. Dates shown by search crawlers are not treated as publication dates.

| ID | Source | Evidence use / limitation |
|---|---|---|
| S1 | [T4C Fantasy Classic Maps](https://t4cfantasy.com/Bible/classic/Maps.php) | Host-maintained map directory; four temple floors and separate cave/crypt destinations; links M1–M5. Exact game patch unspecified. |
| S2 | [T4C Bible: Ar Maps](https://www.t4cbible.com/armaps) | Corroborates four temple floors, Lighthaven Cave, The Tomb and Crypt as separate map entries. Page carries Dialsoft attribution. |
| S3 | [T4C Fantasy Classic Monster Weaknesses](https://t4cfantasy.com/Bible/Classic/MonsterWeaknesses.php) | Main enemy-location matrix. Useful host documentation, not an original server spawn dump. |
| S4 | [T4C Fantasy Classic NPCs](https://t4cfantasy.com/Bible/Classic/NPCs.php) | NPC identities and locations. Published coordinates are archival world coordinates, not a suitable Unreal transform system. |
| S5 | [T4C Fantasy Classic Arakas Quests](https://t4cfantasy.com/Bible/Classic/QuestAR.php) | Early quests, healing and rewards; exact patch unspecified. |
| S6 | [T4C Fantasy Classic Monster Images](https://t4cfantasy.com/Bible/Classic/MonsterImages.php) | Artwork lookup: shared visual families for rat, bat, slime, spider, goblin, atrocity and Balork. The page demonstrates shared graphics, not equivalent combat stats. |
| S7 | [T4C Neerya: Gustave rat quest](https://www.t4c-neerya.com/wiki/quetes/beyond_the_kingdom/secondaire/rats_gustave) | Primary host documentation of a server-specific addition. Do not fold its tarantula and ten-corpse objective into the classic baseline. |
| S8 | [T4C Abomination: Bat Wings Quest](https://wiki.t4c.com/abo/doku.php?id=en%3Aquetes%3Aarakas%3Aautres%3Aailes_de_chauve_souris) | Independent server documentation corroborating reward and monster identity, without floor precision. |
| S9 | [L4P: Monsters](https://www.l4p.fr/index.php?page=VoirMonstres) | Community table; conflicting extra Undead Bat floor. Search retrieval accessible; direct page intermittently timed out. Lower confidence than selected host baseline. |

## Questions remaining for the implementation evidence gate

1. What historical patch/server ruleset should “exact character development” target? The plan can proceed with a named provisional baseline, but numerical parity requires that decision.
2. Which floors host Dungeon Bat in that baseline, and does Undead Bat extend beyond B2?
3. What are the original spawn points/counts, respawn timing, chest timing and loot weights? Images and guides do not establish these.
4. What is the exact starting character location and starter inventory for that baseline?
5. Are original game names, map layouts and images authorized for an eventual public release? Keep provenance separate from assumptions about reusable rights.
6. Can a permitted original client session validate room scale, doors, monster behaviour and NPC healing/training restrictions? If not, identify each reconstruction choice as a prototype adaptation rather than historical fact.
