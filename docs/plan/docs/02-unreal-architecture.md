# Unreal architecture: Lighthaven church and four basement floors

Status: implementation specification; no Unreal project, executable, packaged build, or benchmark is supplied by this document. Engine and plugin versions must be selected and pinned at implementation kickoff.

Companion specifications: [game design](01-game-design.md), [world and encounters](03-world-and-encounters.md), [art pipeline](04-art-pipeline.md), [agent waves](05-agent-waves.md), [validation](06-validation-and-delivery.md), [Lighthaven evidence](../research/lighthaven-evidence.md), [character rules evidence](../research/character-rules-evidence.md), and [contracts](../contracts/README.md).

## 1. Architectural decision

Build a Windows single-player Unreal Engine game with an elevated, rotatable 3D camera, modern walking/running, controller support, and character-stat-driven targeted combat. The first playable area is a small Lighthaven church precinct, the church interior, and the four basement floors. Include minimal source-grounded graybox routes to the actual town trainers, merchants, and mage tower required by the selected starting build paths. Completing the wider town is later work. Enemy identities, placement, and historical character formulas come from the research and rules documents; this document must not silently substitute familiar generic RPG rules.

The game runs entirely on the player's machine. There is one local character and one authoritative local simulation. Do not implement a login server, dedicated server executable, matchmaking, friends, database service, Go backend, PostgreSQL, containers, orchestration, or network protocol in Stage 1.

Preserve a credible migration path by placing mutations behind explicit gameplay commands, using normal Unreal gameplay ownership, and separating persistent records from presentation. This reduces future replacement work; it does not make multiplayer a switch that can be turned on. Prediction, RPC validation, replication, reconciliation, interest management, server persistence, and testing under latency remain future projects.

## 2. Toolchain and dependencies

| Area | Stage 1 choice | Boundary |
| --- | --- | --- |
| Engine | Stable UE5 version available to the team, pinned in `Build/Toolchain.md` and project association | Select after checking C++ compiler, plugin, and asset compatibility; no automatic engine upgrades |
| Gameplay | C++ with Blueprint presentation and data-authored content | Formula, ownership, inventory, rewards, and save decisions belong in C++ |
| Abilities | Gameplay Ability System (GAS), narrowly scoped | Basic attacks, the verified starting spells, attributes, costs, cooldowns, death, and status tags |
| Input | Enhanced Input | Keyboard/mouse and gamepad use the same semantic actions |
| Characters | `ACharacter` and `UCharacterMovementComponent` | No custom locomotion physics or experimental movement plugin in this slice |
| UI | UMG widgets with C++ view models/presenters | Screens observe state and submit commands; widgets do not award XP or set stats |
| AI | AIController, Blackboard, Behavior Tree, standard navmesh | One shared decision tree with data variations where possible |
| Persistence | Versioned `USaveGame` records and a local save subsystem | Platform save API boundary; no direct registry or database dependency |
| Assets | Blender, Unreal import/retarget tools, material instances | Source art kept separately from derived Unreal assets |
| Tests/profiling | Unreal C++ Automation/Functional Tests and Unreal Insights | No Python gameplay implementation, test mirror, or benchmarking framework |
| Source control | Git and Git LFS | LFS for `.uasset`, `.umap`, and suitable large source art; one writer per binary asset |

GAS is chosen now because targeted attacks, spell eligibility, attributes, resource costs, buffs, and eventual authority handling benefit from one consistent execution model. Keep the adoption small: one player ability component, one component per enemy, a small attribute set, and a few generic execution classes. Do not copy the entire Lyra architecture or implement a bespoke second ability framework alongside GAS. If the team lacks GAS experience, the foundation spike must prove one complete attack before content expands.

Epic documents abilities, effects, attributes, tags, and network-related behavior in GAS [E1–E3]. These facilities are building blocks, not automatic game-specific correctness. The choices and limits above are project recommendations.

## 3. Code and content ownership

Begin with one runtime module, one editor module for validators, and a dedicated test module where appropriate for the pinned engine. Use folders to establish seams; do not create a plugin for every class.

```text
Source/Lighthaven/
  Core/              stable IDs, command results, typed state records
  Rules/             pure character/combat formulas and eligibility
  Framework/         GameMode, GameState, PlayerState, Controller, Character
  Character/         progression, equipment, inventory, creation
  Abilities/         GAS integration and execution calculations
  AI/                controllers, tasks, enemy lifecycle
  World/             areas, spawn IDs, portals, interactables
  Persistence/       SaveGame records, migrations, local save subsystem
  UI/                presenters and narrow widget bindings
Source/LighthavenEditor/Validation/
Source/LighthavenTests/
Content/Lighthaven/
  Maps/              Frontend, TempleDistrict, TempleB1..B4, developer test maps
  Data/              Rulesets, Enemies, Items, Spells, Encounters, Areas
  Characters/        Player, NPCs, Monsters
  Environment/       Church, Basement, TownProxy
  UI/
  Audio/
  Tests/
ArtSource/           editable meshes, textures, rigs; approved runtime sources
Reference/          source register and research only; excluded from cooking
Config/
Build/
Docs/Implementation/
```

Player and world records must not contain live actor pointers, widget references, animation state machines, or a serialized entire Unreal object graph. Stable definition IDs resolve content; stable instance IDs identify things that can change.

| Unreal class/system | Responsibility | Does not own |
| --- | --- | --- |
| `ALHGameMode` | Spawn and respawn policy, validate entry to an area, local authoritative game flow | UI, a durable database, a global collection of every asset |
| `ALHGameState` | Current area/session facts relevant to the simulated world | Filesystem saving or character creation widgets |
| `ALHPlayerState` | Character runtime components, player GAS owner, progression and inventory ownership | Camera or raw player input |
| `ALHCharacter` | Physical avatar, movement, collision, animation, GAS avatar | The only copy of long-lived character progress |
| `ALHPlayerController` | Input interpretation, target selection, UI routing, submit character commands | Combat formula implementation |
| `ULHSaveSubsystem` | Snapshot lifecycle, slot enumeration, validation, migration, error handling | Live combat scheduling |
| `ULHAreaStateSubsystem` | The currently loaded area's mutable encounter/interactable state | Account or matchmaking services |
| `ULHDefinitionRegistry` | Resolve stable content IDs and validate required definitions | Save mutable character state in data assets |

The player GAS component belongs to PlayerState and uses the possessed Character as avatar; enemies keep it on their character. Initialize actor info explicitly after possession and restore, and cancel avatar-bound abilities on death or map exit. Persisted state crosses travel through a plain snapshot owned by the save/session subsystem; do not assume PlayerState survives ordinary level loading. Epic's framework documentation explains the differing lifetimes of GameInstance, GameMode, PlayerState, and Pawn [E4].

## 4. Rules and content fidelity

Define `FRulesetId`, `RulesetRevision`, and a field-level provenance/status record for every externally researched mechanic. A prototype tuning value must be marked `Prototype`, not `VerifiedT4C`. Keep the distinction visible in development tooling and the implementation handoff.

Use pure C++ functions, dependent only on typed input data and explicitly provided random rolls, for derived statistics, eligibility, hit resolution, damage, experience advancement, and creation-point accounting. They can use Unreal core types but must not depend on a world, animation, UI, or frame delta. Return a calculation result with enough detail for development diagnostics. Animation completion must not decide damage magnitude or stat growth.

Historical progression may depend on attributes at the time a level is earned. Consequently persist earned base health/mana or equivalent cumulative values, and a compact per-level growth record containing the level, relevant base attributes, ruleset revision, and permanent increment. Recompute the current derived presentation from those authoritative earned values plus equipment/effects. Do not reconstruct a character's entire past from its current attributes. Implement historical dependencies only when verified; retain a schema capable of recording them.

The evidence report identifies Strength, Endurance, Agility, Intelligence and Wisdom, with five attribute points and fifteen skill points per level under the documented baseline. Preserve unspent pools. Confirmed creation flow is four affinity questions followed by roll/re-roll; the exact probability distribution remains unresolved. Persist chosen question/answer IDs, accepted rolled attributes, and generation version. A prototype allocation mode must be declared separately, not presented as exact classic creation. Suggested builds explain legal choices but must not create fixed warrior/mage classes. A rebirth record may be reserved in the schema, with zero rebirths for Stage 1; the rebirth gameplay itself is out of scope.

Keep total earned level separate from current XP balance/debt: the researched no-level-loss death behavior can place XP below the current earned level's threshold. Recovering that deficit must not award the level's points again. Grant multiple newly earned levels through one validated progression operation, recording each growth increment. Exact growth timing and numeric formulas remain data/specification decisions requiring verified fixtures.

Suggested definition families:

| Definition | Important fields |
| --- | --- |
| Ruleset | Stable ID, revision, verified/prototype status, creation policy, formula parameters, rounding policy, experience curve |
| Item | Stable ID, category, required attributes/skills, granted modifiers, stack rules, equipment slot, presentation references |
| Spell/attack | Ability ID/class, eligibility, cost, range, cooldown, targeting policy, cast/move cancellation policy |
| Enemy | Identity, attributes, attack definitions, rewards, loot table, AI parameters, presentation set, provenance |
| Encounter | Area ID, spawn ID, enemy definition, placement, respawn policy, elite/boss flags |
| Area | Area ID, map reference, entrance IDs, valid destination portals, safe fallback transform |

Use `UPrimaryDataAsset` definitions with soft references for optional visuals. Override or explicitly map IDs so a file rename does not change a save's content identity; maintain aliases/migrations when changing published IDs. Asset Manager primary IDs and asynchronous asset handling support this pattern [E5]. Validate that all referenced assets are included in a packaged cook; an editor-only successful load is insufficient.

## 5. Commands, combat, and movement

For mutations expose explicit commands such as `CreateCharacter`, `AllocateAttributes`, `TrainSkill`, `LearnSpell`, `BuyItem`, `EquipItem`, `ActivateAbility`, `Interact`, `TakeLoot`, and `UsePortal`. The command entry point resolves the acting character from its owner/context; it does not trust a caller-provided owner identity. Commands carry stable target/item IDs and a local request ID where duplicate submission is possible. Return structured rejection reasons suitable for UI feedback. Training/learning validates the actual trainer, proximity, prerequisites, shared skill-point pool and gold, then commits all deductions and knowledge changes together. Spell learning cost is distinct from casting mana cost.

There is no socket or RPC transport in Stage 1. Commands execute on the game thread through the local authoritative gameplay owner, validate the whole operation, update runtime state once, and publish state-change events. UI and actors may request actions but cannot bypass eligibility, inventory capacity, range, line-of-sight, or life-state checks. Avoid `GetPlayerController(0)` inside rules and enemy behavior; pass actor context or interfaces.

Resource recovery is an explicit gameplay policy: Stage 1 includes passive mana regeneration and purchasable mana potions, with rate/edge semantics recorded in the design. Store fractional regen progress with resource state; opening a menu or loading a save cannot grant an extra regeneration tick. Bow eligibility includes an equipped compatible quiver; the selected Wooden Arrows quiver is unlimited and is not decremented per attack. An equipment slot for quivers must exist before the ranged route is accepted.

The combat pipeline is:

1. Controller selects a target and requests an action.
2. Gameplay validates actor/target existence, life state, cooldown, resource, equipment, and action eligibility.
3. GAS starts the ability and establishes its cost and cancellation policy.
4. At the rule-defined impact point, validate range/line-of-sight again according to the attack definition, draw explicit random input, and invoke the rules calculation.
5. Apply the result once; death generates one reward/lifecycle transaction.
6. Publish events for hit animation, effects, sound, combat text, and UI.

Choose and document whether cancelled casts refund resources before adding spells. Keep `ActivationId + ImpactIndex` as a hit identity, and reject duplicate effect callbacks. Cosmetic animation notifies may trigger audiovisual timing; gameplay timing requires a path that still works when no visible animation is updating. Player and enemy attacks use the same validated damage/reward primitives.

Use normal CharacterMovement for grounded movement, collision and slopes [E6]. Stage 1 has walking and running with animation blending and an optional run toggle; no dodge invulnerability, parkour, mounted movement, or stamina resource unless approved as an explicit rule change. Running speed and attack/cast movement restrictions are prototype balance decisions documented separately from historical rules. Do not use repeated `SetActorLocation` teleports for locomotion. Avoid root-motion-driven combat translation initially.

Camera defaults to an elevated perspective with bounded zoom and rotation. Target selection supports mouse hit tests and controller cycling, excludes dead/unreachable candidates as specified, and uses clear ground/outline feedback. Mouse and controller need equivalent access to attacks, spell slots, interaction, equipment, stat allocation, dialogue, pause, and saves. Use Enhanced Input contexts for gameplay, UI, and creation; cancel stuck movement when focus or contexts change [E7].

## 6. Church, basement areas, and enemy lifecycle

Use `L_Frontend`, `L_LighthavenTempleDistrict`, and `L_TempleB1` through `L_TempleB4`. The hub contains a bounded exterior church precinct, church interior, and a minimal connected service route. Graybox the source-positioned mage tower and necessary trainers/vendors so Fire Dart/Stone Shard, attack/archery training and required gear can be earned through normal play. Do not move Iraltok/Uranos into the church or ship a debug free-teaching menu. Simple graybox service interiors may live in this same modest map. Use roof/wall occlusion control; do not finish the entire town before the basement loop works. The remaining city begins as explanatory geometry outside the playable boundary. Each basement floor is a separate small map with explicit staircase portals. World Partition, origin shifting, and continuous-world streaming are unnecessary here.

Separate church-interior reconstruction from confirmed map evidence in the content ledger. The map implementation consumes the world specification's topology, staircase links, landmarks and roster coverage. Architecture does not infer which floor contains each species. Four floors must remain four floors even if art production temporarily uses the same kit across them.

Every persistent enemy encounter uses an authored `SpawnId`, and each life has a `LifeGeneration`. Every persistent door, chest, boss, and quest object has an `InstanceId`. Level duplication must create fresh IDs except deliberate references; an editor validator rejects duplicates across all five playable maps. Do not derive identity from array position, display name, actor label, or runtime pointer.

AI uses explicit idle/patrol, acquire, chase, attack, return-home, and dead states through a shared Behavior Tree. Decisions are event/timer driven, not an expensive full-world scan every frame. Range, perception, aggression, flee/leash behavior, and attack timing are enemy data. Navmesh and collision accommodate the player plus the largest included enemy through stairs/doors; unavailable paths must terminate or retry with a bounded policy. Epic provides the AIController/Blackboard/Behavior Tree/navigation pattern used as a starting point [E8].

Safe hub boundaries and stair entrances prevent accidental unavoidable hits during load. Define encounter persistence explicitly: live enemies may reset to their authored anchor on map reload while keeping persisted health/lifecycle as the chosen policy; dead/rewarded enemies must not reappear merely because their map unloaded. Ordinary encounters use a data-defined respawn policy, with prototype timings labelled until verified. Persist remaining active-simulation time; the canonical prototype policy pauses respawn timers while their floor is unloaded or play is paused, and never advances them while the game is closed. A permitted respawn increments `LifeGeneration` and receives a fresh reward identity. This permits required rat kills, training costs and progression without debug grants or a permanently exhausted dungeon. Bosses and unique rewards do not automatically reset. Boss defeat, dialogue/quest stages, reward claims, and portal unlocks are separate persistent facts; the same reward cannot be granted by both death and subsequent dialogue.

## 7. Save format and crash behavior

The save is a coherent snapshot of character and relevant world state. Minimum fields:

```text
Header: magic, schema version, sequence, build ID, ruleset ID/revision,
        content revision, character ID, payload length/checksum
Character: name, appearance IDs, creation answers/roll version,
           base attributes, earned level, XP/debt, unspent point pools,
           health/mana, learned skills/spells, inventory item instances,
           equipment, currency, level-growth history, active area/entrance
World: per-area encounter lifecycle/respawn remainder, corpse loot, pickups/doors,
       boss flags, quest stages, claimed unique reward IDs
Session: required gameplay RNG state, mana-regen fractional timer, persisted effect policy,
         safe respawn checkpoint, bounded diagnostics
```

Store canonical values, not current GAS aggregate values with temporary modifiers baked in. On restore: load definitions, migrate records, create owned components, initialize base/earned values, restore inventory/equipment, restore valid durable effects, set clamped current resources, then enable input/AI. Transient effects, cooldowns, and combat-in-progress require an explicit documented save policy; do not accidentally clear every cooldown on reload. The slice can save only at completed action boundaries, with no active cast, and persist remaining cooldown duration where needed.

Inventory changes, loot transfer, currency, XP, death, and boss flags are logical all-or-nothing runtime operations. Validate first; apply the whole operation on the game thread; emit notifications afterward. Take saves only between completed operations. A full inventory leaves the loot in its original container. A kill transaction marks that enemy life dead and rewarded, assigns XP, and finalizes corpse loot together. Persist the finalized roll so reloading does not generate a different drop for the same saved corpse.

Use two full save generations, A and B, with monotonic sequence numbers. Write an immutable snapshot into the older slot; await completion; validate the stored envelope on subsequent load; keep the previous valid generation. Load the newest complete, validated, compatible generation, falling back with a visible recovery message. A checksum detects corruption; it is not anti-cheat or a signature. Do not claim that a platform write is atomic or that two-slot recovery survives every possible disk failure. Bound saves by byte size and collection counts before allocating on load.

Only one read/write operation is in flight per character. Coalesce dirty save requests without overwriting an active snapshot. `AsyncSaveGameToSlot` is Epic's recommended general approach for reducing save hitches; its implementation still performs serialization on the game thread [E9]. Keep snapshots small and profile actual save stalls. A failed write keeps the old generation, preserves dirty state, and tells the player progress is not safely saved.

For travel: finish/cancel current action by policy, freeze interaction, save a coherent source checkpoint, load the destination, validate its entrance and state, then save the arrived checkpoint. Suppress unrelated autosaves while travel is incomplete. If destination loading fails, restore the source checkpoint; a crash before arrival commit also resumes there. Never grant travel rewards. If the pre-travel save fails, present retry/cancel rather than pretending the transition is durable.

For death: settle the kill/death transaction, apply the specified death consequence once, record the safe respawn checkpoint, commit, and respawn. Do not destroy the only copy of inventory with the avatar. Boss and quest reward claims have explicit stable claim IDs. A failed save after death is reported, not hidden; a player may choose to retry or leave knowing the last durable checkpoint.

Offline saves are user-controlled and cannot establish trustworthy future multiplayer progression. Reverting to an older local save may undo progress. Stage 1 protects consistency and avoids accidental duplication; it does not implement anti-cheat. Future online characters should start from a server-authoritative record or a deliberately reviewed import policy.

## 8. UI, graphics, and validation targets

Required screens are frontend, character creation with legal-build validation, character selection/continue, HUD, character attributes, inventory/equipment, spell selection, interaction/dialogue, pause/settings, death/respawn, and save/recovery feedback. Creation preview is cosmetic; committing the form creates one canonical character record. Resubmitting the same confirmation must not consume points twice or produce ghost save slots.

Use stylized, readable 3D meshes and lighting that preserve reference silhouettes while fitting a coherent art direction. Runtime art status follows the separate asset manifest. Reference screenshots are not a substitute for rigged meshes, animation coverage, collision, materials, or import validation. Start with placeholders and approved reference-derived sprite previews if appropriate, then replace presentation through stable definition IDs without changing character rules.

Create an explicit reference PC specification during kickoff. Proposed target: 1080p at 60 fps on that machine, with a 30 fps fallback quality mode evaluated if required. This is a target, not measured performance. Record CPU/GPU frame times, memory, shader/load stalls and worst included encounter through Unreal Insights [E10]. Use conventional scalable lighting and modest texture/material budgets first; high-end rendering options require demonstrated benefit and a measured budget.

Meaningful automated coverage: verified rule vectors including rounding/boundaries; creation point conservation; equipment eligibility; one-hit/one-reward invariants; full-inventory pickup rejection; save round trips, migrations, interrupted/corrupt generation fallback; floor transitions and restoration; and the boss/quest reward path. Use Unreal Automation and Functional Tests [E11]. Do not write thousands of text-scanning assertions or duplicate the runtime rules in a second language. Manual editor and packaged-game checks remain necessary for camera occlusion, controller focus, animation, readability, navigation, collision and content cooking.

## 9. Future migration seams

| Later capability | Preserved now | Work still required later |
| --- | --- | --- |
| Dedicated multiplayer | Owned command entry points, Gameplay Framework, canonical rules, CharacterMovement/GAS | Server targets, actor/property replication, RPCs, prediction, anti-cheat, latency and load tests |
| Persistent online world | Stable character/item/encounter IDs and revisioned records | Server storage, concurrency/transactions, authentication, recovery, operations |
| Steam | Windows packaged executable and platform-independent character ID | Steamworks enrollment/integration, achievements, cloud-save policy, release process |
| Xbox | Complete controller flow, scalable graphics, platform save boundary | Microsoft approval/devkit/SDK access, supported engine branch, optimization, certification |
| Larger Lighthaven/world | Area/entrance IDs and reusable content definitions | New content, streaming/zone architecture only when size justifies it |

Epic documents packaging as a build/cook/stage/package workflow and notes extra console requirements [E12]. A successfully packaged Windows slice is not an Xbox-ready or multiplayer-validated build.

## 10. Official technical references

Checked 2026-10-07. URLs may display a newer default documentation version; implementation must consult the version matching the pinned engine. Project architecture choices above are recommendations, not verbatim Epic requirements.

- **E1**: [Understanding the Gameplay Ability System](https://dev.epicgames.com/documentation/en-us/unreal-engine/understanding-the-unreal-engine-gameplay-ability-system).
- **E2**: [Gameplay Attributes and Attribute Sets](https://dev.epicgames.com/documentation/unreal-engine/gameplay-attributes-and-attribute-sets-for-the-gameplay-ability-system-in-unreal-engine).
- **E3**: [Ability System Component and Attributes](https://dev.epicgames.com/documentation/unreal-engine/gameplay-ability-system-component-and-gameplay-attributes-in-unreal-engine).
- **E4**: [Gameplay Framework](https://dev.epicgames.com/documentation/en-us/unreal-engine/gameplay-framework-in-unreal-engine).
- **E5**: [Asset Management](https://dev.epicgames.com/documentation/unreal-engine/asset-management-in-unreal-engine).
- **E6**: [Networked Character Movement](https://dev.epicgames.com/documentation/unreal-engine/understanding-networked-movement-in-the-character-movement-component-for-unreal-engine).
- **E7**: [Enhanced Input](https://dev.epicgames.com/documentation/unreal-engine/enhanced-input-in-unreal-engine).
- **E8**: [Behavior Tree Quick Start](https://dev.epicgames.com/documentation/en-us/unreal-engine/behavior-tree-in-unreal-engine---quick-start-guide).
- **E9**: [Saving and Loading](https://dev.epicgames.com/documentation/unreal-engine/saving-and-loading-your-game-in-unreal-engine) and [AsyncSaveGameToSlot](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/UGameplayStatics/AsyncSaveGameToSlot).
- **E10**: [Unreal Insights Trace Quick Start](https://dev.epicgames.com/documentation/en-us/unreal-engine/trace-quick-start-guide-in-unreal-engine).
- **E11**: [Automation Test Framework](https://dev.epicgames.com/documentation/en-us/unreal-engine/automation-test-framework-in-unreal-engine).
- **E12**: [Packaging Projects](https://dev.epicgames.com/documentation/en-us/unreal-engine/packaging-your-project).
