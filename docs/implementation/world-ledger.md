# W0-01 — World ledger

Text evidence snapshot: 2026-10-07; [Lighthaven evidence](../plan/research/lighthaven-evidence.md), [world design](../plan/docs/03-world-and-encounters.md), [enemy contract](../plan/contracts/enemy-roster.json), and [rules ledger](rules-ledger.md). Baseline: LH_Prototype_v1 referencing T4C Fantasy Classic, **historical patch unspecified**. `confirmed` means confirmed within that source, not original-server parity. `disputed`, `missing`, `prototype`, `modernized`, and `needs-visual-check` preserve uncertainty or authorship. Every row has a status.

Sources S1–S9 and M1–M5 resolve to the URLs/baselines below. All have retrieved_date **2026-10-07**, inherited from package research, not this worker's fetch. World rows use their fact as `value`, source key as `source_url`, the register baseline as `source_baseline`, and qualifications as `notes`. Local design rows have source_url `null`, baseline LH_Prototype_v1, snapshot date 2026-10-07. No image was inspected by W0-01. Image-derived facts summarized in the package are retained as candidate traces with **needs-visual-check**, pending W0-04; they are not fresh visual findings.

## Areas and trace checkpoints

| Area/fact (value) | status | source_url | notes |
|---|---|---|---|
| Lighthaven church temple dungeon has four floors B1, B2, B3, B4 | confirmed | S1; S2 | Separate from graveyard Tomb/Crypt, Ancient Temple/Crypt and Lighthaven Cave. No fifth floor or generic cemetery substitution. |
| L_LighthavenTempleDistrict: church interior, forecourt and essential town service route | modernized | null | Authored Stage 1 hub extent; peripheral services grayboxed; full city later. Safe church spawn is project policy, exact original starting tile null. |
| Church upper-right of built-up town image; nave/pews/red carpet/altar; dungeon access in left wing; Samaritan outside that side | needs-visual-check | M5 | Source image positions, not compass directions. Roof elevations, scale and collision are reconstruction. |
| L_TempleB1: four principal chambers, entry lower-left → central hub; branches to upper-left healer room and lower-right enclosure/descent | needs-visual-check | M1 | Candidate room IDs B1.Entry/Hub/Healers/Descent are authored. Candidate edges Entry–Hub, Hub–Healers, Hub–Descent; no direct healer–descent edge described. Loot marks are not spawn marks. |
| L_TempleB2: lower-left entry → internal-void/ring chamber → junction → dogleg → large upper-right hall → rightmost descent | needs-visual-check | M2 | Upper-left optional branches/dead ends; preserve nonwalkable void. Exact apertures and connections need trace; do not invent maze links. |
| L_TempleB3: lower-middle entry, interconnected western/upper-central/eastern regions; lower-left partitions, upper-left dead-end branch, upper-right descent | needs-visual-check | M3 | Exact door graph unresolved; validate broad circulation, B4 access and return without teleport. |
| L_TempleB4: lower-left entry → central hall; upper-left/lower-right side branches; upper-right labelled Balork complex | needs-visual-check | M4 | Altar/partitions/darker masonry described by package. No further descent label; boss anchor must be traced, not inferred from pixels. |
| World scale, exact door widths/Unreal transforms | missing | M1–M5 | null; source pixels and archival NPC coordinates are not centimetres. Grid100 cm and wall200/400 cm in world design are proposed reconstruction values only. |

## Staircase links and portal direction

The four-floor sequence is source-supported; precise landing anchors and historical portal direction require visual/client confirmation. The **paired, bidirectional travel policy** below is authored for hub → B1 → B2 → B3 → B4 → return acceptance (world design and G3), not proof of original teleporter scripts. Entrance names other than the example Temple.SafeSpawn remain proposals for W0-03/W3-04; no IDs are frozen here.

| Link/value | status | source_url | notes |
|---|---|---|---|
| Church left wing ↔ B1.Entry | needs-visual-check | M5; M1 | Package candidate stair connection; exact church departure/B1 arrival orientation unresolved. |
| B1.Descent Level 2 stair ↔ B2.Entry | needs-visual-check | M1; M2 | Departure candidate lower-right B1; arrival candidate lower-left B2. |
| B2 terminal Level 3 stair ↔ B3.Entry | needs-visual-check | M2; M3 | Departure candidate rightmost B2; arrival candidate lower-middle B3. |
| B3 upper-right extension ↔ B4.Entry | needs-visual-check | M3; M4 | Arrival candidate lower-left B4; stair aperture and reverse anchor unresolved. |
| B4 return through B3/B2/B1 to church; no fifth-floor/cave link | prototype | null | Existing design return flow; no invented shortcut portal. Historical one-way/two-way trigger semantics remain missing. |
| Bidirectional paired portals, stable area/entrance IDs and safe landing | modernized | null | Architecture §§6–7: validate destination, snapshot source then arrival, recover source on failed load; travel grants no rewards. Validate both directions in packaged G3. |
| Safe loaded entrances and accessible healer retreat | prototype | null | Existing architecture/world safety policy; collision, clearance, enemy reach and facing require W3 play checks. |

## Eleven-definition floor coverage

Source_url for every row: **S3**, source_baseline **T4C Fantasy Classic Monster Weaknesses, patch unspecified**, retrieved_date **2026-10-07**. C = confirmed placement within selected source; P = provisional project placement; — = no selected placement, **not proof of universal absence**. Each ID remains independent even if visual families overlap. Floor placements below retain their original secondary-source statuses. Bible source spawn counts/positions, XP-column semantics, loot probabilities, aggro/resistances and ordinary timing remain **missing: null**. Bible HP/level/melee damage and listed loot are now recorded in the monster-data table below; they do not establish floor placement. Do not fill gaps with skeletons, zombies, mummies or Neerya's tarantula.

| Definition | B1 | B2 | B3 | B4 | status | notes |
|---|---|---|---|---|---|---|
| Enemy.BrownRat | C | C | C | C | confirmed | Rat family; eligible rat-errand identity must be explicit. |
| Enemy.Bat | C | C | — | — | confirmed | Distinct from Dungeon/Giant/Undead Bat. |
| Enemy.DungeonBat | — | P | — | — | prototype | Temple presence confirmed, **source floor null**. Existing contract chooses B2 temporarily; replace when baseline floor verified in W4-02. |
| Enemy.GreenSlime | C | C | C | C | confirmed | Independent slime definition. |
| Enemy.GiantBat | — | C | C | C | confirmed | Separate gameplay definition despite bat family. |
| Enemy.UndeadBat | — | C | — | — | disputed | R-03 Bible ArakasQuest (2026-10-09) confirms B2; S9 secondary also lists B3, which Bible does not establish or exclude. Select Bible B2; do not union sources. |
| Enemy.GiantSpider | — | C | — | — | confirmed | B2, not tarantula/Gustave B1 content. |
| Enemy.Goblin | — | — | C | — | confirmed | Separate from Goblin Warrior. |
| Enemy.GoblinWarrior | — | — | C | — | confirmed | Shared presentation family does not merge stats/rewards. |
| Enemy.Atrocity | — | — | C | C | confirmed | Separate definition. |
| Enemy.Balork | — | — | — | C | confirmed | B4 confirmed by Bible classic drops.html and live Monster1 (retrieved2026-10-07); exact arena anchor needs M4 visual check; XP reward semantics/gold null. |

Floor coverage from this matrix: B1 rat/bat/slime; B2 rat/bat/slime/Giant Bat/Undead Bat/Giant Spider plus **provisional Dungeon Bat**; B3 rat/slime/Giant Bat/Goblin/Goblin Warrior/Atrocity; B4 rat/slime/Giant Bat/Atrocity/Balork. Roster parity cannot be claimed with unresolved Dungeon Bat placement.

## Bible roster / monster-data reconciliation (R-02)


XP +1/+50/+100 are confirmed **printed columns**; their meaning and unconditional reward remain missing. Damage is melee-only, distinct from spell damage. Gold below is live-era evidence; no classic gold column was captured. Loot lists are membership, not probabilities; dash means no listed drop, not proven empty loot. Basement floor locations remain missing in the Bible except Balork.

| Monster | Level | HP | XP +1 / +50 / +100 (semantics missing) | Classic melee min–max | Live melee min–max | status | source_url | retrieved_date |
|---|---|---|---|---|---|---|---|---|
| Brown Rat | 1 | 27 | 45 / 42 / 42 | 4–5 | 2–5 | disputed | https://web.archive.org/web/20020202144834/http://www.t4cbible.com:80/monster.html; https://www.t4cbible.com/Monster | 2026-10-07 |
| Bat | 1 | 27 | 42 / 37 / 32 | 4–5 | 2–5 | disputed | https://web.archive.org/web/20020202144834/http://www.t4cbible.com:80/monster.html; https://www.t4cbible.com/Monster | 2026-10-07 |
| Dungeon Bat | 2 | 41 | 75 / 69 / 68 | 3–7 | 3–7 | confirmed | https://web.archive.org/web/20020202144834/http://www.t4cbible.com:80/monster.html; https://www.t4cbible.com/Monster | 2026-10-07 |
| Green Slime | 2 | 41 | 74 / 69 / 68 | 4–7 | 3–7 | disputed | https://web.archive.org/web/20020202144834/http://www.t4cbible.com:80/monster.html; https://www.t4cbible.com/Monster | 2026-10-07 |
| Giant Bat | 3 | 55 | 107 / 94 / 93 | 4–8 | 4–8 | confirmed | https://web.archive.org/web/20020202144834/http://www.t4cbible.com:80/monster.html; https://www.t4cbible.com/Monster | 2026-10-07 |
| Undead Bat | 3 | 55 | 94 / 93 / 93 | 4–7 | 4–8 | disputed | https://web.archive.org/web/20020202144834/http://www.t4cbible.com:80/monster.html; https://www.t4cbible.com/Monster | 2026-10-07 |
| Giant Spider | 4 | 69 | 146 / 124 / 122 | 4–10 | 4–10 | confirmed | https://web.archive.org/web/20020202144834/http://www.t4cbible.com:80/monster.html; https://www.t4cbible.com/Monster | 2026-10-07 |
| Goblin | 5 | 84 | 192 / 161 / 157 | 5–12 | 5–12 | confirmed | https://web.archive.org/web/20020202144834/http://www.t4cbible.com:80/monster.html; https://www.t4cbible.com/Monster | 2026-10-07 |
| Goblin Warrior | 12 | 199 | 706 / 527 / 499 | 10–23 | 10–23 | confirmed | https://web.archive.org/web/20020202144834/http://www.t4cbible.com:80/monster.html; https://www.t4cbible.com/Monster | 2026-10-07 |
| Atrocity | 5 | 84 | 231 / 161 / 157 | 5–12 | 5–12 | confirmed | https://web.archive.org/web/20020202144834/http://www.t4cbible.com:80/monster.html; https://www.t4cbible.com/Monster | 2026-10-07 |
| Balork [+karma] | 15 | 508 | 2025 / 1553 / 1452 | 13–29 | 13–29 | confirmed | https://web.archive.org/web/20020202144834/http://www.t4cbible.com:80/monster.html; https://www.t4cbible.com/Monster | 2026-10-07 |

| Monster | Classic listed loot | Live listed loot | Live gold | Bible location | Respawn | status | source_url | retrieved_date |
|---|---|---|---|---|---|---|---|---|
| Brown Rat | Torch | Torch | 1-5 | | | confirmed | https://web.archive.org/web/20011014151611/http://www.t4cbible.com:80/drops.html; https://www.t4cbible.com/Monster1 | 2026-10-07 |
| Bat | Torch, Light Heal | Torch, Light Heal | 1-5 | | | confirmed | https://web.archive.org/web/20011014151611/http://www.t4cbible.com:80/drops.html; https://www.t4cbible.com/Monster1 | 2026-10-07 |
| Dungeon Bat | - | - | 3-11 | | | confirmed | https://web.archive.org/web/20011014151611/http://www.t4cbible.com:80/drops.html; https://www.t4cbible.com/Monster1 | 2026-10-07 |
| Giant Bat | - | - | 5-16 | | | confirmed | https://web.archive.org/web/20011014151611/http://www.t4cbible.com:80/drops.html; https://www.t4cbible.com/Monster1 | 2026-10-07 |
| Undead Bat | Decaying Bat Wings | Decaying Bat Wings | 5-16 | | | confirmed | https://web.archive.org/web/20011014151611/http://www.t4cbible.com:80/drops.html; https://www.t4cbible.com/Monster1 | 2026-10-07 |
| Giant Spider | Torch, Light Heal | Torch, Light Heal | 7-22 | | | confirmed | https://web.archive.org/web/20011014151611/http://www.t4cbible.com:80/drops.html; https://www.t4cbible.com/Monster1 | 2026-10-07 |
| Green Slime | - | - | 3-11 | | | confirmed | https://web.archive.org/web/20011014151611/http://www.t4cbible.com:80/drops.html; https://www.t4cbible.com/Monster1 | 2026-10-07 |
| Goblin | Goblin Leather Armor, Light Heal, Iron Ring | Goblin Leather Armor (dr), Light Heal, Iron Ring, Heal Pot | 8-27 | | | disputed | https://web.archive.org/web/20011014151611/http://www.t4cbible.com:80/drops.html; https://www.t4cbible.com/Monster1 | 2026-10-07 |
| Goblin Warrior | Goblin Blade (dr), Iron Key, Light Heal | Goblin Blade (r), Iron Key (dr), Light Heal | 21-66 | | | disputed | https://web.archive.org/web/20011014151611/http://www.t4cbible.com:80/drops.html; https://www.t4cbible.com/Monster1 | 2026-10-07 |
| Atrocity | Light Heal | Light Heal, Iron Key | 8-27 | | | disputed | https://web.archive.org/web/20011014151611/http://www.t4cbible.com:80/drops.html; https://www.t4cbible.com/Monster1 | 2026-10-07 |
| Balork | Flowing Black Robe (dr), Light Heal; boss-table location: AR LH Temple Dungeon Level 4 | Flowing Black Robe (dr), Light Heal | (blank) | AR LH Temple Dungeon Level 4 | 15:00 | confirmed | https://web.archive.org/web/20011014151611/http://www.t4cbible.com:80/drops.html; https://www.t4cbible.com/Monster1 | 2026-10-07 |

Balork: **confirmed** Temple Dungeon Level4, Flowing Black Robe (demi rare) and Light Heal; live respawn15:00 (15 minutes), gold **missing** (blank). Timer origin, pause/offline behavior and drop probabilities are missing. R-03 owner decision supersedes permanent defeat: use900s respawn; completion reward stays single-claim. Rarity labels dr/r/vr have no recovered numeric probabilities.


## Required town and basement services

The skill/spell/item prices and minima are source candidates recorded in the rules ledger. Availability, affordable acquisition and actual proximity transactions must be implemented; named decorative NPCs do not satisfy the route.

| Service/value | status | source_url | source position / notes |
|---|---|---|---|
| Brother Kiran, priest; Kilhiam, Light; Moonrock, Heal Light | confirmed | S4; R13 in rules ledger | Lighthaven Temple; exact transforms need M5 trace. |
| Church/forecourt tutorial and safe arrival | modernized | null | Authored dialogue/arrival; source exact spawn tile null. |
| Lighthaven Samaritan rat errand | confirmed | S4; S5 | Outside/near temple. Accept, fifteen rats below temple, return for 2500 XP, within Classic guide baseline. |
| Samaritan exact forecourt side/interaction anchor | needs-visual-check | M5 | Package says outside left-side dungeon wing; no compass or precise transform inferred. |
| Nevanis healing | confirmed | S4; S5 | B1 “western room” in guide; map-marked room controls placement, not assumed world west. R-03 Bible ArakasQuest2026-10-09 confirms healing request. Restrictions, amount and cost missing; Prototype full HP/free for alive injured players, no level cap; Dark Fang restriction does not apply. |
| Nevanis and Shovanis shared healer chamber anchor | needs-visual-check | M1 | Upper-left image branch in package description; verify room/clear interaction space. |
| Shovanis, Dust Devil training | confirmed | S4; R13 in rules ledger | Temple Dungeon B1; price/prerequisites in rules ledger. |
| Iraltok Fire Dart, Uranos Stone Shard/bat-wing turn-in | confirmed | R13; S4; S5 | Mage tower; keep there, never relocate into church. |
| Mage tower route | needs-visual-check | M5 | Small offshore area above-right of church on image. Actual bridge/ground access null until traced; any temporary route must be recorded as reconstruction. |
| Attack: Murmuntag/Ortanalas; Dodge: Kalastor; Archery: Kalastor/Ortanalas | confirmed | R11 in rules ledger | Lighthaven town trainers; exact individual building/coordinates unresolved in text summary. Printed costs Attack10/Dodge10/Archery15 confirmed; currency/points-per-rank/scaling missing; see R-03 lookup. |
| Trainer building and reachable street route | needs-visual-check | M5 | Package says training building below-left of church; do not assign individual trainers to an invented room. |
| Sigfried, Ashwood Flatbow and Wooden Arrows quiver | confirmed | S4; R20; R21 in rules ledger | Town weapon shop; prices29/100 gold, unlimited quiver. |
| Sigfried precise shop anchor | needs-visual-check | M5 | Nearby above-left of church in package summary. |
| Fali, Potion of Mana | confirmed | S4; R17–R19 in rules ledger | Town potion seller, archival (2900,1068,0); +25 MP for50 gold, not Unreal transform. Yolak is Windhowl, not a local replacement. |
| Fali/Rolph building anchor | needs-visual-check | M5 | Below-left of church per package. Rolph town armour merchant confirmed S4; upgrade stock/prices unresolved. |
| Bat-wing route | confirmed | S5; S8 | Undead Bat B2 (S3), wings → Uranos at mage tower → Skull Dagger +2500 XP. R-03 Bible ArakasQuest2026-10-09 confirms B2 and reward; drop chance/count remain missing (plural name is not a count). Deferred by owner; optional Marsac Cred need not be staged. |
| Selectable dialogue topics preserving topic IDs | modernized | null | Existing world design replaces keyword-only input. Single-player turn-in omits multiplayer cooldown by design; historical cooldown not inferred. |
| Gustave ten-corpses/tarantula quest excluded | confirmed | S7 | Neerya-specific content, separate from selected fifteen-rat Samaritan baseline. |

## Balork and completion semantics

| Fact/policy (value) | status | source_url | notes |
|---|---|---|---|
| Balork final encounter is on B4 | confirmed | S3 | Exact labelled complex anchor needs M4 visual review. |
| Defeat Balork → return to church → save finishes slice | modernized | null | Authored Stage 1 objective; no verified historical quest chain/reward attached. Mark's original effects/quest prerequisites are missing, value null. |
| Balork respawns900s after defeat; completion reward single-claim | confirmed (timer); modernized (reward policy) | https://www.t4cbible.com/monster1 | R-03 live check2026-10-09:15:00. Owner decision supersedes earlier permanent-defeat prototype; timer origin and loaded/unpaused clock are Prototype. |
| Persist defeated/mark state, return objective, quest stages and reward claims separately | modernized | null | Architecture + world design. Unique completion reward claim cannot be granted by both death and dialogue or duplicated by travel/reload. Numeric completion reward null. |
| Boss mechanics, extra phases/summons/area attacks | missing | S3 | null; not established, no invented phases in fidelity profile. |
| Samaritan accepted/completed/rewarded states and single turn-in | modernized | null | Persist fifteen eligible kill count independently of corpse cleanup; source reward2500 XP is distinct from rat kill XP. |

## Respawn and renewable progression

Existing tuning below comes from world design §6/prototype-policy, except explicitly marked R-03 proposals in the reconciliation section. Prototype source_url is null, baseline LH_Prototype_v1, snapshot date 2026-10-07. W4-02/04 replace with verified baseline data or reviewed playtest tuning. Historical spawn counts, respawn rates, chest timers and loot weights remain **missing**, source S3/M1–M4, value null.

| Policy/value | status | source_url | notes |
|---|---|---|---|
| Ordinary cooldown120 seconds after death | prototype | null | Unpaused **loaded-area** simulation only; pause, unloaded floors and game closed do not tick. No respawn on load/travel reset. |
| Suppress eligible respawn in view/relevancy region or unsafe player distance | prototype | null | Safety-distance value null; author/review in W4-04, no enemy materialization on player. |
| Persist spawn slot, life generation, alive/dead state and remaining timer | modernized | null | New life increments generation and obtains fresh reward identity; old life cannot reward twice. |
| Finalize corpse loot at death and persist contents/claimed state | modernized | null | No reroll on reload. Full inventory rejects pickup intact. Kill/XP/loot and quest updates form coherent transactions. |
| Ordinary loot cleanup duration; chest timing | missing | null | null; unique quest items require recovery path; interrupted travel/full inventory must not permanently block wings. |
| Rat farming and renewable gold/XP support fifteen kills and training | prototype | null | Ordinary respawn prevents finite-dungeon softlock; affordability not yet tested. No debug grants qualify. |
| B1 slot counts: rat12, bat3, slime2 | prototype | null | Existing world-design proposal, not source spawn table; rat quest needs respawns to reach15. Positions null. |
| B2 slots: rat6, bat3, slime3, Giant Bat4, Undead Bat4, spider3, Dungeon Bat2 | prototype | null | Existing proposal; Dungeon Bat floor also provisional. No inferred source positions. |
| B3 slots: rat4, slime3, Giant Bat4, goblin6, Goblin Warrior3, atrocity2 | prototype | null | Existing proposal; no B3 Undead Bat in selected profile. |
| B4 slots: rat3, slime2, Giant Bat4, atrocity3, Balork1 | prototype | null | Existing slot proposal; R-03 selects900s Balork respawn with separate single-claim completion reward. |
| Initially one to three active pursuers; live reload reset details | prototype | null | Pursuer suggestion from world design; authoritative alive health/anchor reset details still unresolved. Dead/rewarded lives must retain state. |

## Source register and reference locations

| Key | source_url | source_baseline |
|---|---|---|
| S1 | https://t4cfantasy.com/Bible/classic/Maps.php | Fantasy Classic map directory; patch unspecified |
| S2 | https://www.t4cbible.com/armaps | Community map index |
| S3 | https://t4cfantasy.com/Bible/Classic/MonsterWeaknesses.php | Fantasy Classic location matrix; not server spawn dump |
| S4 | https://t4cfantasy.com/Bible/Classic/NPCs.php | Fantasy Classic NPC directory |
| S5 | https://t4cfantasy.com/Bible/Classic/QuestAR.php | Fantasy Classic Arakas quest guide |
| S7 | https://www.t4c-neerya.com/wiki/quetes/beyond_the_kingdom/secondaire/rats_gustave | Neerya server-specific quest |
| S8 | https://wiki.t4c.com/abo/doku.php?id=en%3Aquetes%3Aarakas%3Aautres%3Aailes_de_chauve_souris | Abomination bat-wing quest |
| S9 | https://www.l4p.fr/index.php?page=VoirMonstres | Community monster table; lower confidence, intermittent source access in package |
| M1 | https://t4cfantasy.com/images/maps/i1arakas/LHDungeon1.png | Classic-hosted B1 map |
| M2 | https://t4cfantasy.com/images/maps/i1arakas/LHDungeon2.png | Classic-hosted B2 map |
| M3 | https://t4cfantasy.com/images/maps/i1arakas/LHDungeon3.png | Classic-hosted B3 map |
| M4 | https://t4cfantasy.com/images/maps/i1arakas/LHDungeon4Classic.png | Classic-labelled B4 map |
| M5 | https://t4cfantasy.com/images/maps/i1arakas/LighthavenClassic.png | Classic-labelled town map |

R11/R13/R17–R21 resolve in the companion rules ledger. Original map files remain at `/home/brewerm/Downloads/lighthaven-unreal-design-and-agent-waves/lighthaven-plan/assets/references/maps/`; this worker has not inspected them. Their package hashes/provenance are in the evidence/asset manifest. They are reference material, with licence/author unresolved, not approved packaged textures.

## Open questions / research still required

- W0-04: verify all candidate image-space room/door/stair traces, church access, service buildings and mage-island access. Do not interpret image directions as compass or pixels as centimetres.
- Pin historical server/patch; establish Dungeon Bat floors and independently resolve Undead Bat B2/B3 conflict without unioning profiles.
- W0-03/W3-04: freeze area/entrance/portal IDs, both-direction destinations and safe fallback checkpoints after trace; test hub-to-B4-and-back, reload on each floor and destination-load failure.
- W3 owners: validate exact door graph, widths, healer reachability, boss retreat, camera and largest-enemy navigation; no original tile coordinates recovered here.
- Research source spawn positions/counts, HP/XP/loot, chest/cleanup rates and live-enemy restore semantics. No ordinary enemy numeric parity is claimed.
- Resolve trainer prices/starting skills, healer restrictions and bat-wing quantity/drop/recovery policy; preserve actual NPC locations. Demonstrate affordable melee/ranged/magic routes without debug teaching.
- Determine historical Balork mark/quest semantics and recurrence before any fidelity claim; current permanent defeat/return objective is authored, rewards unresolved.
- W4-04/05: test one-life/one-reward, quest single-claim, remaining timer across travel/pause/reload, renewable fifteen-rat completion and zero-gold/zero-mana recovery.
- Map artwork rights and original-client validation remain separate research questions. No build/editor/cook/package/play checks or gate passes are asserted by this text ledger.


## R-03 Wave 4 world lookup — 2026-10-09

See [one-table lookup](research/w4-bible-lookup.md) for every species' HP/XP/melee/gold/loot and shared missing speed/odds/respawn fields. Existing classic numeric/loot rows retain2026-10-07 provenance because Wayback failed this attempt; fresh live lowercase [monster](https://www.t4cbible.com/monster) and [monster1](https://www.t4cbible.com/monster1) checks agree with retained live values. Classic damage conflicts remain disputed and use the selected classic side. Live gold fills absent classic gold; numeric drop odds remain missing for every species. No archive fetch failure is evidence of absence.

Fresh [ArakasQuest](https://www.t4cbible.com/ArakasQuest) (retrieved2026-10-09, version unstated) explicitly places Undead Bats at temple level2. Dungeon Bat exact floors still missing; B2 stays Prototype. B3 Undead Bat is retained as secondary disagreement, not added. Nevanis heals in B1 on request; no fee/amount/level restriction recovered. Wings→Uranos→Skull Dagger+2500 XP is confirmed, wing count/chance missing, quest deferred.

Balork's live boss row lists B4, robe (demi rare), Light Heal, blank gold,15:00. HP508/melee13–29 are confirmed; exact arena, phases/summons, drop odds, attack speed and timer origin are missing. **Recurring900s respawn supersedes permanent defeat**, while completion reward remains single-claim. A simple melee encounter at the existing B4 anchor is Prototype script behavior, not proof of the original script.

R-03 missing-field **Prototype recommendations**, not tested rules: ordinary120s and boss900s remaining cooldowns tick only loaded/unpaused; respawn needs1000cm clearance from player, outside view/relevancy and valid nav/floor/non-overlap. Ordinary corpse loot300s on that clock with persisted contents/remaining time; unique/quest loot never expires. NPC interaction250cm and loot200cm with LOS/reachability. Nevanis restores full HP/free to alive injured players without level cap; church death recovery full HP/MP, no penalty during development. Bible does not provide these numbers. Wings1 item/10% proposal is deferred record-only; item odds never derived from qualitative rarity. Integration still needs ordinary-play economy/safety tests.
