# T4C character rules and visual-reference evidence

Research date: 2026-10-07. This document separates source-supported facts from implementation decisions and unresolved parity details. The aim is a single-player Unreal prototype with classic-style progression; it is not a claim that every T4C server uses identical rules.

## Findings that affect the design

T4C development is not a conventional locked class tree. An operator guide explicitly describes builds as the consequence of attribute and skill spending. Present Warrior, Archer, Mage, and Priest as explanations or suggested builds; do not make them mutually exclusive character classes. [R01]

Official support documents five attributes—Strength, Endurance, Agility, Intelligence, Wisdom—and awards five attribute points plus fifteen skill points per level. Points can be saved. Trainers consume skill points and commonly charge money. Attribute allocation is previewable until applied; committed allocations are permanent under the documented baseline. HP gained on a level-up depends on Endurance at that time. [R02]

That historical dependence is not universal today: the official Abomination relaunch announcement explicitly enables retroactive HP/MP growth. It also lists a starter kit of cloth pants, cloth vest, a rusty dagger, three torches, and 100–150 gold. Treat those as Abomination settings, not definitive original-release values. [R03]

## Verified versus provisional

| Topic | Evidence status | Implementation consequence |
|---|---|---|
| Five primary attributes; +5 attribute/+15 skill points per level | Verified in official support [R02] | Core progression contract |
| Builds emerge from choices rather than class restrictions | Verified operator description [R01] | Build suggestions never gate gear or spells |
| Four questions, five answers each, followed by rerolls | Verified operator onboarding [R04]; classic roll examples corroborate [R05] | Implement the flow; keep RNG/table fidelity tracked separately |
| Eight possible question themes | Verified operator onboarding [R04] | Data-driven question pool, four selections per character |
| Exact roll distribution, seeds, rounding, correlation | Not established by sources consulted | Do not infer an exact algorithm from maximum-stat tables |
| Endurance-at-level-up affects HP | Verified official classic-style description [R02] | Store awarded growth and the relevant stat snapshot |
| Intelligence and Wisdom affect mana growth | Documented in operator guidance and chart [R01, R05] | Configurable growth provider; numerical parity requires reference cases |
| Retroactive HP/MP recomputation | Explicit server-specific feature [R03] | Separate ruleset option; never silently enable it in historical mode |
| Spell learning consumes the same skill-point budget as skills | Verified official guidance [R06] | One spendable skill-point balance |
| Spell prerequisites and sight restrictions | Verified official guidance [R07] | Validate learned prerequisites and target visibility |
| Exact starter HP/MP, initial skill values, initial unused points | Unresolved | A named prototype default is acceptable; not a confirmed classic rule |
| Rusty Dagger versus Rusted Dirk item identity | Unresolved naming/server difference [R03, R12] | Use stable IDs and do not merge names merely because they sound similar |
| Death penalties | Verified as server-configured [R08, R09] | Versioned death-policy data; no universal percentage |
| No level loss after death XP loss | Verified official support [R10] | Track XP debt without reducing earned level or awarding duplicate points |
| Rebirth benefits and detailed requirements | Outside this researched slice | Persist a versioned rebirth record placeholder; do not fabricate values |

## Character creation and progression specification

Recommended prototype flow: local profile → character name → appearance → four-question affinity flow → stat roll/re-roll → review → confirm → spawn inside the temple. Appearance must not grant hidden attributes. Store answers, resulting stats, generation ruleset ID, and generation version in the character record.

The operator onboarding lists eight question themes and allows repeated rerolling. Its recommended physical build uses two Strength/Endurance answers and two Agility-oriented answers; its magic recommendation mixes Intelligence and Wisdom. These are affinity choices, not a class selection. [R04]

The classic-reference chart gives useful golden maximum examples in `STR, END, AGI, WIS, INT` order: all Strength answers `(22,22,16,12,12)`; all Agility `(22,16,22,12,16)`; mixed two Wisdom/two Intelligence `(14,14,14,20,20)`. These are examples of reachable maxima, not a proof of the roll distribution. The same chart lists low-level HP gains of 6–8 at Endurance 1–19 and 7–9 at 20–39; mana contributions of 3–5 at Intelligence 1–29, 4–6 at 30–59, and zero extra Wisdom contribution below 60. [R05]

Keep exact growth formulas behind a pure C++ rules interface. Record `LevelBefore`, `LevelAfter`, effective growth-input stats, HP/MP awarded, growth-provider version, and relevant random draws. An editor designer must be able to inspect a character's growth history. Do not recalculate all historical HP from current Endurance unless the selected ruleset explicitly enables retroactive growth.

A level-up event must be atomic. Crossing multiple XP thresholds in one reward grants each level's points exactly once. Reloading a save, dying, or regaining XP debt must not re-award levels. Whether stat allocation at the same level-up influences that level's growth is unresolved: add it to the parity test protocol rather than guessing.

Suggested labels:

- `ClassicReferenceCandidate`: source-backed constants, historical growth, unresolved values explicitly listed.
- `PrototypePlaytest`: declared temporary values for a playable test before exact verification.
- `VerifiedReference_<server>_<version>`: used only after the target server/version and its fixtures have been established.

The prototype should display the active ruleset in development/debug screens. Changing the ruleset must require an explicit migration or new character, not silently change saved characters.

## Starter skills and spell acquisition

The classic skill reference places Attack training with Murmuntag/Ortanalas in Lighthaven, Dodge with Kalastor, and Archery with Kalastor/Ortanalas. Attack, Dodge, and Archery are passive resolution skills; the entry-level rows have no attribute prerequisites. Stun Blow begins at level 3 with Strength 25 and Agility 30. These are source-supported data candidates, not proven rules for every server. [R11]

Do not replace the Dodge statistic with a dodge-roll ability. Smooth running can coexist with classic hit resolution. A controller dodge move, stamina, invulnerability frames, freely moving while casting, or an action-aiming replacement would be separate combat changes requiring explicit design decisions.

Use these compact candidate spell rows from the classic reference; spell cost is a one-time learning cost while mana is a casting cost. [R13]

| Spell | Level | WIS | INT | Skill points | Gold | Mana | Teacher |
|---|---:|---:|---:|---:|---:|---:|---|
| Light | 2 | 15 | 18 | 5 | 233 | 10 | Kilhiam, temple |
| Fire Dart | 2 | 15 | 21 | 5 | 532 | 1 | Iraltok, mage tower |
| Heal Light | 3 | 19 | 15 | 9 | 897 | 2 | Moonrock, temple |
| Stone Shard | 4 | 20 | 17 | 6 | 1,328 | 2 | Uranos, mage tower |
| Dust Devil | 6 | 21 | 21 | 7 | 2,388 | 2 | Shovanis, dungeon |

The teacher locations create a scope dependency: a temple-only scene cannot honestly offer the complete original low-level spell-acquisition route without also providing access to the mage tower. Preferred stage-one implementation: make a small navigable Lighthaven service route around the church, mage tower, and trainers. A debug teaching console is acceptable for development, but is not the normal player route.

Official spell guidance distinguishes physical and mental effects, says spells do not make a normal miss roll, describes resistance mitigation, and requires an unobstructed path. It also gives the prerequisite chain Fire Dart → Flaming Arrow → Fire Bolt. Do not interpret “cannot miss” as permission to cast through walls. Collision interception details should be confirmed in play before claiming perfect fidelity. [R07]

## Starting inventory and death

Do not assume the Abomination starter “Rusty Dagger” is the classic `Rusted Dagger`. A classic weapon reference lists a zero-requirement Rusted Dirk at 1–4 base damage, whereas Rusted Dagger requires Strength 24 and has 6–10 base damage. Wooden Club and Rusted Short Sword are zero-requirement alternatives at 2–4 damage. A mistakenly mapped starter dagger can make newly rolled characters unable to equip their only weapon. [R12]

For a declared prototype starting kit, use a verified equipable zero-requirement weapon, clothing, a small torch supply, and explicitly configured gold. The chosen numeric kit must be labelled a prototype decision until the reference server/version is pinned. Include a starter-gear validation assertion for every allowed creation roll.

Official support says death XP loss is server dependent. Realmud's settings table lists PvE losses of 10% gold, 5% backpack, 0% equipped, and 10% XP, but the meaning of each item percentage and the XP denominator still needs verification. Do not implement item percentages as a guaranteed fraction of inventory without evidence. [R08, R09]

The no-level-loss rule means a character can have XP below the threshold for their already-earned level and must regain the deficit before progressing. Preserve stat allocations, training, and earned levels. [R10] For early usability testing, a no-item-loss death profile can be a clearly declared prototype setting; it is not an asserted T4C constant.

## Mana recovery and the first bow

Official support confirms passive mana recovery over time, mana potions, a quest amulet, and the Meditate skill, but gives no exact natural regeneration interval or formula. [R16] The classic consumables reference lists Potion of Mana as restoring 25 MP and Mana Elixir as restoring 50 MP; both weigh 2. [R17] The item-location reference lists Potion of Mana at 50 gold from Fali or Yolak. [R18] Fali is the relevant Lighthaven seller; the NPC index places Fali at `(2900,1068,0)` and Yolak in Windhowl. [R19]

Stage 1 can therefore support renewable purchased mana potions plus free passive recovery. If the exact regeneration rate remains unknown, a workable **provisional** configuration is `1 MP per 5 seconds of active unpaused simulation while alive`, clamped to the current maximum, with no offline accrual. This is project tuning, not recovered T4C timing. Persist fractional progress or explicitly reset the timer on load; never grant an immediate extra tick for reopening a save. Test zero gold/zero mana recovery without restarting. Consumption should be one validated command; full-mana rejection and excess restoration clamping are explicit prototype policies until confirmed.

The classic weapon table provides a zero-attribute-requirement Ashwood Flatbow at 1–3 base damage, weight 7, buy price 29; Wooden Arrows is an **unlimited** quiver with no Strength/Agility requirement, +1 damage, weight 3, buy price 100. [R20] Sigfried's Lighthaven shop lists both. [R21] Archived official version 1.16 notes explicitly mention needing both a bow and a quiver for ranged attacks. [R22]

Use a reusable equipped quiver for this entry-level path; do not consume individual arrows. Reject fire without the bow or quiver, explain the missing component, and test that firing does not decrement Wooden Arrows. Exact bow damage composition, equipment-slot conflict rules, and initial Archery skill remain source-baseline work. The 129-gold purchase total is a useful progression-economy test, not a reason to grant a free bow or increase starting gold silently.

## Online character and monster reference images

These image URLs were obtained from links on the listed source pages. They are visual references, not established reusable game assets or complete animation packs. Downloading or referencing them does not establish distribution rights. Keep provenance and replace or obtain permission before a distributable build uses source artwork.

| Subject | Direct image URL | Source/status |
|---|---|---|
| Bat family, including Undead Bat | https://t4cfantasy.com/images/skins/app20002.jpg | Opened; monster index maps names [R14] |
| Rat family, including Brown Rat | https://t4cfantasy.com/images/skins/app20003.jpg | Opened; index mapping [R14] |
| Skeleton family | https://t4cfantasy.com/images/skins/app20012.jpg | Opened; not evidence of temple placement [R14] |
| Atrocity family | https://t4cfantasy.com/images/skins/app20026.jpg | Opened; index mapping [R14] |
| Green Slime family | https://t4cfantasy.com/images/skins/app20005.jpg | Opened; index mapping [R14] |
| Giant Spider family | https://t4cfantasy.com/images/skins/app20007.jpg | Opened; index mapping [R14] |
| Balork demon family | https://t4cfantasy.com/images/skins/app20013.jpg | Opened; index mapping [R14] |
| Warrior illustration | https://www.d4o.de/en/images/game-infos/first-steps/class-warrior.png | Opened from onboarding page [R04] |
| Mage illustration | https://www.d4o.de/en/images/game-infos/first-steps/class-mage.png | Opened from onboarding page [R04] |
| Archer illustration | https://www.d4o.de/en/images/game-infos/first-steps/class-archer.png | Downloaded and visually inspected: promotional painting [R04] |
| Character-creation promotional art | https://www.d4o.de/en/images/game-infos/first-steps/character-creation.png | Downloaded and inspected: fantasy dice illustration, not original creation UI [R04] |

Downloaded reference bytes and hashes are indexed in `assets/references/character-monster-manifest.md`. The D4O character illustrations are modern promotional artwork; they are not proof of original player models. Original player-model references were located in the classic misc image index [R15]: eight male directions at `https://t4cfantasy.com/images/items/Misc/Puppet/HumanMale1.png` through `HumanMale8.png`, and eight female directions at the corresponding `HumanFemale1.png` through `HumanFemale8.png`. These are 79×79 images with a black background, not complete animated sprite sheets. Front/back samples were visually inspected. The Goblin family image is `https://t4cfantasy.com/images/skins/app20001.jpg`; its source also names Goblin Warrior [R14].

Do not equate image families with identical monsters: Bat, Dungeon Bat, and Undead Bat may share artwork but require independent creature definitions. Build an original 3D mesh/rig from approved art direction; a single JPEG cannot be assumed to supply the run, attack, hit, and death animations needed by Unreal.

## Remaining parity work for the first agent wave

1. Record the target historical version/server or explicitly accept a composite prototype baseline.
2. Confirm start level, initial HP/MP, initial learned skills, skill values, inventory, and starting gold.
3. Collect stat-roll fixtures and define whether the prototype needs exact distribution parity or only confirmed ranges and flow.
4. Verify growth timing relative to stat application and temporary stat buffs; include an Endurance 19→20 boundary case.
5. Verify trainer prices and whether training prices vary by skill level; distinguish learning from training.
6. Verify hit, avoidance, armor, resistance, attack speed, potion, and encumbrance rules using observed examples rather than invented formulas.
7. Verify XP debt, loss denominator, respawn placement, dropped-item selection, and lost-item recovery.

Do this in the game's native C++/Unreal test facilities. A spreadsheet or Markdown evidence table is sufficient for recording facts. No separate Python simulation or benchmarking framework is needed.

## Source register

All sources accessed 2026-10-07. Modern operator documentation supports the operator's own current descriptions; classic-reference tables need version confirmation for strict original parity.

- **R01** T4C Nostalgia, How to Play: https://www.t4cnostalgia.com/game/how-to-play
- **R02** Dialsoft support, Character advancement: https://support.t4c.com/knowledgebase.php?article=12
- **R03** Official Steam announcement, Abomination relaunch, 2024-08-16 (within announcement feed): https://steamcommunity.com/app/523750/announcements/
- **R04** D4O operator, First Steps: https://www.d4o.de/en/game-infos/first-steps/
- **R05** T4C Fantasy Classic, creation/growth tables: https://t4cfantasy.com/Bible/Classic/Questions.php
- **R06** Dialsoft support, Warrior spell learning: https://support.t4c.com/knowledgebase.php?article=25
- **R07** Dialsoft support, Spell system: https://support.t4c.com/knowledgebase.php?article=24
- **R08** Dialsoft support, Experience loss: https://support.t4c.com/knowledgebase.php?article=14
- **R09** Dialsoft support, Realmud settings: https://support.t4c.com/knowledgebase.php?article=29
- **R10** Dialsoft support, No level loss: https://support.t4c.com/knowledgebase.php?article=15
- **R11** T4C Fantasy Classic, Skills: https://t4cfantasy.com/Bible/Classic/Skills.php
- **R12** T4C Bible, Weapons: https://www.t4cbible.com/Weapon
- **R13** T4C Fantasy Classic, Spells: https://t4cfantasy.com/Bible/Classic/Spells.php
- **R14** T4C Fantasy Classic, Monster images: https://t4cfantasy.com/Bible/Classic/MonsterImages.php
- **R15** T4C Fantasy Classic, Misc images / character model: https://t4cfantasy.com/Bible/Classic/MiscImages.php
- **R16** Dialsoft support, Mana recovery: https://support.t4c.com/knowledgebase.php?article=23
- **R17** T4C Fantasy Classic, consumables: https://t4cfantasy.com/Bible/Classic/Misc.php
- **R18** T4C Fantasy, item locations: https://t4cfantasy.com/Bible/Items.php
- **R19** T4C Fantasy Classic, NPC locations: https://t4cfantasy.com/Bible/Classic/NPCs.php
- **R20** T4C Fantasy Classic, weapons/quivers: https://t4cfantasy.com/Bible/Classic/Weapons.php
- **R21** T4C Bible, weapon traders: https://www.t4cbible.com/traders
- **R22** T4C Development official archive, version 1.16 notes: https://next.t4c.com/board/viewtopic.php?id=94

Source summaries are deliberately compact rather than copied wiki prose. Exact formulas not independently established above remain open questions.
