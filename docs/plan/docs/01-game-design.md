# Lighthaven: first playable design

Planning date: 2026-10-07. Working project name: **Lighthaven Prototype**. Deliverable: a Windows single-player Unreal game, designed to permit a later multiplayer implementation. This package is the design and implementation brief, not a compiled game.

## 1. Product decision

Recreate the character-building appeal and recognizable starting location of The 4th Coming with modern 3D presentation, smooth running, responsive input, readable interfaces and an elevated camera. Make the first complete experience character creation → Lighthaven temple → four basement floors → Balork → return and save. The user has selected Unreal, and has explicitly deferred multiplayer servers.

The broader requested city remains in scope as the next delivery increment. The first release gate covers the temple district, church interior and all four dungeon floors. Other city buildings can initially be distant shells; finishing the whole town must not delay proving that combat and character development work.

## 2. What fidelity means

Treat three things independently:

| Layer | Direction | Completion evidence |
|---|---|---|
| Character rules | Match a declared T4C baseline; preserve stat-driven builds | Source-linked rule table plus hand-calculated parity cases |
| World | Reconstruct temple-dungeon connectivity and recognizable town landmarks | Source map overlay and walkthrough |
| Presentation | Improve geometry, materials, movement and UI | Playable camera/input and art review |

Do not describe a mixture of community-server values as “the exact original.” Reference material in this package includes server-specific Classic guides. Each numeric rule carries a source, baseline/version label and status: `verified_for_source`, `corroborated`, `provisional`, or `unresolved`.

The development profile is `LH_Prototype_v1`. Its release notes enumerate departures from the selected historical rules. A `ClassicParity` label is allowed only after the relevant rows are verified. This is a fidelity requirement, not a requirement to freeze every workflow while research continues: movement, maps, art, UI shells and persistence contracts can proceed against explicit provisional fixtures.

## 3. Scope by increment

### Stage 1A — first playable loop

- Local character selection, creation, deletion confirmation and continue.
- Human character appearance selection; no classes that lock future development.
- Five base attributes, XP, level, allocatable attribute/skill points.
- Church interior, forecourt and correct dungeon entrance; floor one complete.
- Samaritan introduction, rat errand, healer interaction and first trainer.
- Movement, melee, targeting, loot, inventory, equipping, death/return and save/load.
- Keyboard/mouse and controller operation through every essential screen.
- Original downloaded images used as visual references and, where technically useful, labeled temporary stand-ins.

### Stage 1B — complete church descent

- All four temple-dungeon floors with the approved roster and a Balork encounter.
- Working early spell learning/casting; one ranged weapon path as well as melee.
- Minimal graybox service routes to the actual town trainers, vendors and mage tower, so these builds can learn and buy what they need without debug grants. The wider city's finished art remains Stage 2.
- Attribute requirements, skill training and enough equipment to demonstrate build choices.
- Quest and boss state persist through travel, death, reload and exit.
- Improved 3D character/enemy models and church/dungeon environment replace temporary images.
- A packaged Windows build and measured performance evidence.

### Stage 2 — finish Lighthaven city

- Expand from the temple district to the source-grounded streets, river edge, bridges, shops, inn/tavern and trainer locations.
- Add relevant city NPC interactions and vendors rather than a town full of inert facades.
- Treat the cemetery crypt, Lighthaven cave, mage-island interiors and later quest destinations as separately accepted subfeatures; do not accidentally conflate them with the church dungeon.
- Preserve save compatibility with Stage 1 characters.

### Deferred

Hosted worlds, replication implementation, account authentication, databases, server fleets, cross-play, Xbox packaging, Steam publishing, guilds, player trade, PvP, auctions, other islands and playable Seraph rebirth. Reserve identifiers for rebirth state but implement no speculative rebirth framework in Stage 1.

## 4. Character creation and early progression

### Frontend flow

1. Title: New character, Continue, Characters, Settings, Quit.
2. Select a save slot; enter a local display name. No online uniqueness check or account login.
3. Choose human body/presentation, hair, skin and starter appearance from a small compatible set. Appearance grants no mechanical bonuses.
4. Complete the reference character-creation process. Research supports a question-and-reroll approach; use the exact weighting and roll ranges only after verification. The UI must expose the final five-stat result before acceptance.
5. Confirm final attributes, initial learned skills/spells and starter possessions from the selected rules profile.
6. Commit a complete character snapshot, then enter the church safe spawn.

Creation is atomic. Back/cancel does not consume a slot or alter another character. Rerolling replaces a pending result, not an already committed character. Store creation profile, rules version and rolled values; do not later reconstruct the initial character from a changed random generator.

If reference roll probabilities remain unknown, present the implemented creation mode as a **prototype allocation/roll mode**, explicitly documented as provisional. Do not advertise parity. Do not make a class-selection screen with Warrior/Mage/Priest as permanent choices. Optional explanatory examples may suggest allocations without enforcing classes.

### Progression model

The five attributes are Strength, Endurance, Agility, Intelligence and Wisdom. Use `Agility` as the canonical field; “Dexterity” is at most a localized display alias, never a sixth stat. Research supports five allocatable attribute points and fifteen skill points per level. See [character evidence](../research/character-rules-evidence.md) for source limitations and early spell data.

Keep these separate in code and UI:

- Base attributes and unspent points.
- Equipment modifiers and temporary effects.
- Learned skill identifiers, training values and unspent skill points.
- Current versus maximum health/mana.
- Level, total XP and the current level threshold.
- Permanent progression flags such as Balork's mark.

Historical maximum-health/mana growth can depend on attributes at the time levels were gained; some servers use retroactive growth. Persist each level-up input and granted increment. Never derive classic historical maxima solely from today's attributes. Do not silently switch growth policies on an existing save.

Point allocation is a command validated against the remaining pool, allowed increments and the selected profile. Show a preview, then apply once. Multi-level XP awards grant all intervening level entitlements exactly once. Equipment bonuses do not automatically qualify a character for learning/equipping unless that is the selected source rule.

### Minimum equipment and skills

Provide a basic melee weapon, basic bow, cloth equipment, a low-tier armor upgrade, healing consumables and a torch/light option. Source maps name some chest rewards, but chest rates and item stats remain a separately verified data set. Do not grant unverified items free solely because the UI needs samples.

Use the zero-requirement **Rusted Dirk** as the provisional starter melee candidate, not the higher-requirement Rusted Dagger. Verify every permitted creation result can equip its starter weapon. For the ranged path, the reference Ashwood Flatbow and **Wooden Arrows unlimited quiver** are separate equipped requirements. Do not consume one arrow per shot: this quiver is reusable. Source candidates are 29 gold for the bow and 100 for the quiver from Sigfried; preserve their source/profile provenance.

Inventory needs stack quantity, stable item-instance ID for non-stackable items, slot/equipment location, requirements and encumbrance only if verified and included in the profile. A failed equip/purchase leaves all state unchanged. Skill learning and training consume the correct currencies and validate trainer availability.

Stage 1 spell candidates are Light, Fire Dart, Heal Light, Stone Shard and Dust Devil. The evidence report provides source-specific minimum levels and prerequisites. Stage 1A may remain melee-led; Stage 1B must supply the actual acquisition route for Light, damage and healing spells and finish the selected early set. Fire Dart and Stone Shard need mage-tower teachers; provide their minimal service route rather than relocating them into the church. Additional spells require source data and a content acceptance task.

Mana recovery is mandatory, including when the player has no money left. Official guidance confirms natural recovery, but the exact rate is unresolved. Prototype policy: gain 1 MP per five seconds of alive, unpaused simulation, clamped to maximum, with no offline accrual and a persisted fractional timer. Mark this rate provisional. Also supply the reference Potion of Mana (+25 MP, candidate price 50 gold from Fali). Prototype use policy rejects consumption at full mana and clamps restoration; verify those edge semantics before claiming parity. Empty mana must be recoverable through normal play without restarting or reloading.

## 5. Movement, camera and combat

### Presentation choices

Default to an elevated perspective camera with zoom and limited orbit; retain the recognizable isometric reading of the world. Start with a tunable camera pitch around 50–60 degrees. It is a design starting point, not a recovered T4C camera specification.

Use smooth analog movement and a walk/run toggle. A proposed prototype walk/run pair is 220/450 cm/s, to be measured against corridor scale and combat pacing. There is no stamina bar, dodge invulnerability, jump traversal or parkour in Stage 1. These would change encounters and should be separate later decisions.

Occluding roofs and camera-facing walls fade or cut away. Never make navigation depend on seeing through opaque ceilings. Collision belongs to deliberate simplified meshes; decorative props do not create invisible narrow traps.

### Target combat

- Mouse selects a visible target; controller uses nearest meaningful target plus cycle/lock controls.
- Attack intent enters the authoritative local gameplay layer. UI and animations never award damage directly.
- Melee checks target validity, range, line of sight where appropriate, action state and cooldown.
- Ranged attacks and spells obey profile-specific costs, prerequisites, wind-up and recovery.
- Running and attacking have an explicit interruption rule; the provisional default stops movement during the attack/cast commitment. This needs a balance review against the selected T4C baseline.
- Damage timing belongs to the ability/rules contract. Animation notifies provide presentation timing signals; a missing notify cannot make a different damage formula or duplicate a hit.
- Maintain separate streams for combat randomness and cosmetic variation. Saving/reloading must not accidentally reroll already committed rewards.

Do not turn accuracy into “the mesh touched the other mesh” if the goal is stat-driven T4C combat. Visible weapon sweeps and projectiles communicate a result resolved by the selected rules, unless a later explicit design change adopts action collision.

### Proposed control map

| Intent | Keyboard/mouse default | Controller default |
|---|---|---|
| Move | WASD | Left stick |
| Camera | Middle-drag/orbit, wheel zoom | Right stick, contextual zoom |
| Walk/run | Shift toggle or hold setting | Left-stick press |
| Select/lock | Left click, Tab cycle | Right-stick press / bumper cycle |
| Attack | Right click | Right trigger |
| Interact/loot | E | South face button |
| Spell/consumable | 1–6 hotbar | Shoulder-held radial selection |
| Inventory/character | I / C | Menu tabs |
| Pause/back | Escape | Menu/back |

Resolve overlaps through Enhanced Input contexts. All mappings are proposals, remappable and tested in a shipped build. Do not bind actions only through widget mouse events.

## 6. NPCs, quests and local play

Use dialogue choices for discoverability, optionally showing keyword equivalents. Original keyword recognition can be added as a secondary interaction, not the only controller path. Dialogue text can be newly written while retaining the verified quest predicates and rewards.

The rat errand is the first meaningful loop. It begins with Samaritan, counts eligible rat kills after acceptance, and grants the recorded reward only on turn-in. Nevanis supplies the source-grounded healing role; Shovanis is shown by the floor-one map and is a candidate trainer, with the exact offerings verified separately. Church NPCs should use confirmed names/locations from the evidence sheet, not guesses based on a “Classic” map label alone.

Balork is the Stage 1B destination. Killing him records the relevant permanent mark/quest state. Full good/evil island-access quest chains extend beyond this slice. A prototype completion notification must not falsely claim the entire original quest is completed. If implementing Balork's dialogue path, distinguish it from killing him and retain its flags; do not force future good/evil exclusivity through one boolean.

Single-player adaptation: true pause is allowed; no enemies simulate while the game is closed. Loot ownership is local. Death returns the character to the safe church spawn with a clearly configured penalty policy. The exact loss formula is unresolved until baselined; do not quietly choose permanent item loss. The default development profile may use no loss while death/restore invariants are being built, labeled accordingly. Under the documented no-level-loss rule, death-related XP debt never removes already earned levels or grants their points again when recovered.

## 7. Playthrough and acceptance

The first ten-minute test should demonstrate creation, running to the cellar, selecting a rat, taking damage, finding the healer, looting, progression, and save/reload. This is a usability test duration, not a promise that the original XP curve reaches a particular level within ten minutes.

A complete Stage 1B test must create a fresh character, legitimately earn every required progression step, descend and return through all four floors, defeat the boss under the declared prototype/classic profile, exit and resume. Debug level-ups may accelerate development but cannot substitute for this playthrough.

No forced session-length target overrides a verified XP curve. If classic grind makes the test longer, report it and offer a separately identified accelerated test profile; never silently multiply XP in the default profile.

The game is accepted only when it is playable as a packaged application, not merely when Blueprints compile. Use [validation](06-validation-and-delivery.md) and [waves](05-agent-waves.md) for the exact handoff gates.
