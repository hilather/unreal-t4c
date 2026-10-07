# W0-01 — Rules ledger

Evidence snapshot: 2026-10-07. This is a text reconciliation, not a runtime definition or a claim of observed original-client behavior. Sources below were retrieved by the package researchers on that date; this worker has not freshly fetched them. See [character evidence](../plan/research/character-rules-evidence.md), [design](../plan/docs/01-game-design.md), and [contracts](../plan/contracts/README.md).

## Selected baseline and provenance convention

Use the existing development profile **LH_Prototype_v1**. Its reference candidate is the T4C Fantasy **Classic** guide (historical patch unspecified), supplemented by Dialsoft support for documented general mechanics. D4O supplies creation-flow evidence, not proof of Classic RNG parity. No exact historical version/server can be selected from this evidence. Official version **1.16** supports only the cited bow/quiver requirement; it does not date the entire composite ruleset. Do not call this profile ClassicParity or VerifiedReference.

`confirmed` means documented for the named source baseline, not independently validated or universal. `disputed` preserves contradictions; `missing` means value `null`/unresolved; `modernized` is an authored interface/presentation change; `prototype` is temporary project tuning. These labels interpret, rather than modify, the planning contract's verified_for_source/provisional/unresolved labels.

Each table has the contract fields `source_url`, `source_baseline`, `retrieved_date`, `status`, `value`, and `notes`. Source keys resolve to literal URLs and baselines in the register below; no source key asserts a fresh fetch. In all tables **retrieved_date = 2026-10-07** (inherited package retrieval, or package snapshot date for local design); rows inherit this field. Prototype rows with source_url `null` cite the local design that chose them in notes. Unknowns are never zero defaults. Replacement points identify the dependent research/implementation task, not authorization to guess.

## Creation, attributes and progression

| Mechanic | source_url | source_baseline | status | value | notes |
|---|---|---|---|---|---|
| Builds | R01 | Nostalgia operator | confirmed | Spending determines build; no permanently locked class | Suggested Warrior/Archer/Mage/Priest labels may explain choices, never gate development. |
| Creation question flow | R04; R05 | D4O onboarding; Classic chart | confirmed | Four questions, five answers each; eight themes; repeated rerolls | Exact pool text/selection weighting remains unresolved. |
| Local creation flow | null | LH_Prototype_v1 design | modernized | Profile/name → cosmetic human appearance → questions → roll/re-roll → review → confirm → church | Design §4; commit once, cancel without consuming slot; persist answer IDs, accepted named stats and generation version. Appearance has no bonus. |
| Creation RNG | R05 | Classic chart, patch unspecified | missing | null | Distribution, seed, correlations, rounding and minima unknown. W1-02 requires fixtures or a separately labelled prototype generator. |
| Reachable maximum examples | R05 | Classic chart | confirmed | All STR: STR22 END22 AGI16 WIS12 INT12; all AGI: STR22 END16 AGI22 WIS12 INT16; two WIS/two INT: STR14 END14 AGI14 WIS20 INT20 | Examples only, not probability tables or starter defaults. Named fields prevent WIS/INT reversal. |
| Canonical attributes | R02 | Dialsoft advancement support | confirmed | Strength, Endurance, Agility, Intelligence, Wisdom | Five attributes. |
| AGI / Dexterity mapping | null | LH_Prototype_v1 native contract | modernized | AGI and interface Dexterity → Agility | contracts/README.md; never add a sixth stat. Native order uses named fields, not source array order. |
| Start level, HP/MP, unused points, gold | R03; R05 | Abomination settings versus Classic candidates | missing | null | Example save's level 1 and XP 0 are illustrative, not recovered rules. W1-02/W2-02 must resolve or declare defaults. |
| Level entitlement | R02 | Dialsoft advancement support | confirmed | +5 attribute points; +15 skill points per earned level; points may be saved | Preview allocation; committed allocation permanent in source. |
| Atomic level-up / debt recovery | R10; local architecture §4 | Dialsoft no-level-loss; LH consistency policy | modernized | Each newly earned level awarded once; recovering XP debt grants no repeat points | Preserve earned level, allocations, training and growth through death/load; multi-threshold awards include every new level once. |
| HP growth dependence | R02 | Dialsoft classic-style advancement | confirmed | Endurance at level-up influences awarded HP | Persist growth input and award. Numeric formula/temporary-buff treatment unresolved. |
| Growth table samples | R05 | Classic chart | confirmed | END1–19: HP6–8; END20–39: HP7–9; INT1–29: mana contribution3–5; INT30–59:4–6; WIS<60: zero extra WIS contribution | Source samples only; not a complete formula, RNG law, initial HP/MP, or guarantee at all levels. |
| MP growth dependence | R01; R05 | Nostalgia guidance; Classic chart | confirmed | Intelligence and Wisdom contribute | Full formula, rounding and award timing null; W1-02 fixtures needed. |
| Retroactive growth | R02; R03 | Historical dependence versus Abomination relaunch 2024-08-16 | disputed | Historical growth retained for reference candidate; Abomination enables retroactive HP/MP | Do not recompute historical maxima from current attributes or silently mix profiles. |
| Stat application timing | R02; R05 | Support/chart | missing | null | Whether same-level allocations affect current award and effective versus base growth stats unresolved; test END19→20. |
| XP curve | null | Package unresolved parity field | missing | null | Thresholds, formula, rounding and cap not supplied. W1-02 research or named prototype table before progression. |
| Derived stats | R02; R05; R07 | Support + Classic candidates | missing | Full max-HP/MP, accuracy, avoidance, damage bonuses, armor/resistance, capacity and speed formulas: null | Keep earned HP/MP, current resources, base attributes, equipment and temporary modifiers separate; no inferred D&D-style formulas. |

## Combat, equipment, death and recovery

| Mechanic | source_url | source_baseline | status | value | notes |
|---|---|---|---|---|---|
| Physical hit/damage | R11; R12 | Classic skill/weapon references | missing | Hit/avoidance probabilities, damage composition, armor, resistance, crit/stun behavior, speed, rounding: null | Attack/Dodge/Archery are passive resolution skills, not collision-only hits or dodge-roll invulnerability. W1-02 requires sourced fixtures or prototype policy. |
| Spell targeting | R07 | Dialsoft spell system | confirmed | No normal miss roll; resistance mitigates; unobstructed path and learned prerequisites required | Physical/mental distinction documented. Fire Dart → Flaming Arrow → Fire Bolt chain; later spells outside starter set. Collision interception details unresolved. |
| Spell effect magnitudes/range/timing | R13; R07 | Classic spells; support | missing | null | Learning/casting costs below do not establish damage/heal magnitude, cooldown, cancellation or moving-cast rules. W1-02/W4-03 replacement. |
| Target controls and movement | null | LH design §§5, architecture §5 | modernized | Mouse/controller targets, elevated 3D camera, smooth walk/run | Authored presentation; no stamina, parkour or invulnerable dodge in slice. |
| Movement/commitment tuning | null | LH_Prototype_v1 | prototype | Walk220 cm/s; run450 cm/s; stop moving during attack/cast commitment | Existing design/prototype-policy, not historical speed. Replace after W1-01/W4-03 play review; interruption/refund details unresolved. |
| Weapon requirements | R12 | T4C Bible weapons, version unspecified | confirmed | Rusted Dirk: zero requirement, base damage1–4; Rusted Dagger: STR24, damage6–10; Wooden Club/Rusted Short Sword: zero requirement, damage2–4 | Names are separate items; base weapon numbers do not specify final hit damage. |
| Starter identity | R03; R12 | Abomination versus weapon reference | disputed | Rusty Dagger ≠ established identity of Rusted Dirk/Rusted Dagger | Abomination cloth pants/vest, rusty dagger, 3 torches, 100–150 gold is server-specific; do not import as Classic starting kit. |
| Starter kit | null | LH_Prototype_v1 | prototype | Item.RustedDirk candidate; clothing/torch/gold quantities null | Existing design/prototype-policy. W2-02 validates equipability for every allowed roll and declares remaining kit before runtime use. No new numeric defaults here. |
| Requirement evaluation | R11; R12; R13 | Classic tables | missing | Whether base or modified stats qualify; equip-slot conflicts; armor/weight rules: null | Named item/spell minima do not resolve this. W1-02/W2-02 must specify. |
| Ashwood Flatbow | R20; R21 | Classic weapons; Bible traders | confirmed | No attribute requirement; base damage1–3; weight7; price29 gold; Sigfried | Initial Archery value and final bow damage composition unresolved. |
| Wooden Arrows quiver | R20; R21; R22 | Classic table; traders; official v1.16 notes | confirmed | Unlimited; no STR/AGI requirement; +1 damage; weight3; price100 gold; bow AND equipped quiver required | Separate quiver slot, no per-shot decrement. 129 gold total is an affordability check, not starting-gold evidence. |
| Death losses | R08; R09 | Dialsoft; Realmud settings | disputed | Server-dependent; Realmud PvE: gold10%, backpack5%, equipped0%, XP10% | XP denominator and item selection semantics null; percentages are not guaranteed fractions of inventory. |
| No level loss / XP debt | R10 | Dialsoft support | confirmed | Earned level retained; regain deficit before further progression | Debt accounting representation is native schema work; never award level points again when recovering. |
| Initial death profile | null | LH_Prototype_v1 | prototype | No death penalty during initial development | prototype-policy.json; replace in W2-02/W4-05 after selected baseline's denominator/item-loss/respawn research; do not present as authentic. |
| Passive mana recovery | R16 | Dialsoft support | confirmed | Natural recovery, potions, quest amulet, Meditate exist | Exact natural rate and Meditate/amulet parameters null; latter paths need not replace free natural recovery. |
| Mana regeneration rate | null | LH_Prototype_v1 | prototype | 1 MP / 5 seconds alive, unpaused simulation; clamp to max; no offline accrual | Existing policy/design; persist fractional timer to avoid reload ticks. W4-03 replaces rate with verified timing or reviewed tuning. |
| Mana consumables | R17; R18; R19 | Classic consumables/item/NPC references | confirmed | Potion of Mana +25 MP, weight2, 50 gold from Fali or Yolak; Mana Elixir +50 MP, weight2 | Fali is local; Yolak is Windhowl. No Elixir price/local vendor inferred. |
| Potion edge policy | null | LH_Prototype_v1 | prototype | At full mana reject without consumption; excess restore clamps | Existing policy; W4-03 verifies source semantics or retains declared tuning. Validate/consume once. |
| Monster HP/XP/loot and economy | null | enemy-roster.json unresolved data | missing | HP, damage, XP reward, drops/weights, attack timings: null | Quest reward rows are not ordinary creature XP. W4-02/04 need explicit data; no guessed authentic numbers. |

## Starting knowledge and acquisition

Initial learned Attack/Dodge/Archery values, initially learned spells, initial skill-point pool and skill-training gold prices are **missing**, value **null**, source_url R11/R13, source_baseline Classic guide, retrieved_date 2026-10-07. These acquisition rows are source-supported candidates, not automatic starting grants. Trainers commonly consume skill points and money (R02); learning spells shares the skill-point pool (R06). Learning and casting costs are separate. All transactions require the actual trainer, proximity, prerequisites, sufficient gold/points, and one atomic commit (authored architecture policy).

| Skill | source_url | source_baseline | status | value | notes |
|---|---|---|---|---|---|
| Attack | R11 | Classic Skills | confirmed | No entry attribute prerequisite; Murmuntag/Ortanalas, Lighthaven | Gold price/training increments/scaling null; W4-07 resolves. |
| Dodge | R11 | Classic Skills | confirmed | No entry attribute prerequisite; Kalastor, Lighthaven | Passive avoidance statistic; costs/starting value null. |
| Archery | R11 | Classic Skills | confirmed | No entry attribute prerequisite; Kalastor/Ortanalas, Lighthaven | Costs/starting value null; bow/quiver still required. |
| Stun Blow | R11 | Classic Skills | confirmed | Level3, STR25, AGI30 | Early candidate beyond basic starting trio; cost/teacher null in package summary. |

Spell rows inherit source_url **R13**, source_baseline **T4C Fantasy Classic Spells, patch unspecified**, retrieved_date **2026-10-07**, status **confirmed** (candidate values for that source). `value` is the tuple below; `notes` states location. No spell is presumed initially learned.

| Spell | value: min level / WIS / INT / learning skill points / learning gold / casting MP | notes: teacher and source position |
|---|---|---|
| Light | 2 / 15 / 18 / 5 / 233 / 10 | Kilhiam, temple |
| Fire Dart | 2 / 15 / 21 / 5 / 532 / 1 | Iraltok, mage tower |
| Heal Light | 3 / 19 / 15 / 9 / 897 / 2 | Moonrock, temple |
| Stone Shard | 4 / 20 / 17 / 6 / 1328 / 2 | Uranos, mage tower |
| Dust Devil | 6 / 21 / 21 / 7 / 2388 / 2 | Shovanis, B1 healer room |

Keep mage tower access in Stage 1B; neither free debug teaching nor relocated teachers proves a legitimate build route. Price scaling, availability and exact prerequisite interpretation remain research tasks for W4-07.

## Source register

Every URL/baseline below is inherited from the package's R register; retrieved_date 2026-10-07. No new external evidence is asserted.

| Key | source_url | source_baseline |
|---|---|---|
| R01 | https://www.t4cnostalgia.com/game/how-to-play | Nostalgia operator guidance |
| R02 | https://support.t4c.com/knowledgebase.php?article=12 | Dialsoft character advancement |
| R03 | https://steamcommunity.com/app/523750/announcements/ | Official Abomination relaunch, 2024-08-16; feed, not pinned post |
| R04 | https://www.d4o.de/en/game-infos/first-steps/ | D4O operator onboarding |
| R05 | https://t4cfantasy.com/Bible/Classic/Questions.php | Classic creation/growth tables; patch unspecified |
| R06 | https://support.t4c.com/knowledgebase.php?article=25 | Dialsoft spell learning |
| R07 | https://support.t4c.com/knowledgebase.php?article=24 | Dialsoft spell system |
| R08 | https://support.t4c.com/knowledgebase.php?article=14 | Dialsoft experience loss |
| R09 | https://support.t4c.com/knowledgebase.php?article=29 | Realmud server settings |
| R10 | https://support.t4c.com/knowledgebase.php?article=15 | Dialsoft no level loss |
| R11 | https://t4cfantasy.com/Bible/Classic/Skills.php | Classic skills; patch unspecified |
| R12 | https://www.t4cbible.com/Weapon | Community weapon reference; version unspecified |
| R13 | https://t4cfantasy.com/Bible/Classic/Spells.php | Classic spells; patch unspecified |
| R16 | https://support.t4c.com/knowledgebase.php?article=23 | Dialsoft mana recovery |
| R17 | https://t4cfantasy.com/Bible/Classic/Misc.php | Classic consumables |
| R18 | https://t4cfantasy.com/Bible/Items.php | Fantasy item locations |
| R19 | https://t4cfantasy.com/Bible/Classic/NPCs.php | Classic NPC locations |
| R20 | https://t4cfantasy.com/Bible/Classic/Weapons.php | Classic weapons/quivers |
| R21 | https://www.t4cbible.com/traders | Community weapon traders |
| R22 | https://next.t4c.com/board/viewtopic.php?id=94 | Official archived version 1.16 notes |

## Open questions / research still required

- Select a precise historical server/patch with permitted client observations before asserting parity; composite LH_Prototype_v1 remains development-only.
- Capture creation RNG fixtures, initial level/resources/knowledge/points/gold and exact kit identity. R05 maxima cannot reconstruct probabilities.
- Resolve full derived/growth formulas, randomness/rounding, stat-allocation timing, temporary modifiers, and historical versus retroactive policy.
- Establish XP thresholds and death denominator/item selection/recovery/checkpoint rules; exercise debt without duplicate entitlements.
- Verify hit/armor/resistance/speed, spell magnitudes/ranges/cancellation, equipment requirement stat basis and slot/weight rules.
- Verify trainer learning versus repeated-training costs, skill increments, scaling and starter values. Demonstrate affordable magic/ranged progression with renewable rewards.
- Verify mana rate, potion full/overflow semantics and timer behavior; explicitly retain prototype replacements when evidence is unavailable.
- Supply separately labelled enemy/reward/drop/respawn data through W4-02/04; unresolved required values must fail content validation, not become zero.
- W0-03 integrator owns native schemas, ruleset revisions and migrations; this ledger does not freeze shared contracts or pass G0.
