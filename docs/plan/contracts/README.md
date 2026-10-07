# Contract examples for implementation agents

These are planning examples, **not executable Unreal assets or validated import formats**. The Wave 0 integrator translates the contracts into versioned native structs/DataAssets and freezes their headers before parallel implementation. Keep one canonical native runtime schema; do not introduce a JSON microservice or a second rules engine just because the planning package uses JSON.

## Files

- `enemy-roster.json`: all eleven planned definitions, confirmed versus provisional floor placement and presentation families.
- `prototype-policy.json`: explicit single-player adaptations, deferred unknowns and chosen canonical names.
- `save-record.example.json`: illustrative identity and progression/world-state fields; not a playable save.

Unknown numeric mechanics are `null` or omitted with `unresolved` status. Never convert a missing field to zero through default deserialization. A shipping-content validator rejects unresolved required values. The prototype can proceed with separately declared `prototype_tuning` entries after agent review.

## Required native contracts

| Contract | Required semantics |
|---|---|
| `CharacterId` | Stable local GUID independent of name, Steam ID, Xbox ID or future account ID |
| `ContentId` | Stable namespaced ID for item/enemy/spell/quest/rules/map definitions |
| `EntityId` | Stable placed/spawned instance identity within an area/run |
| `SpawnLifeId` | Area + spawn-slot ID + incrementing life sequence |
| `RewardId` | Stable unique ID per life/objective and reward purpose |
| `CommandResult` | Accepted/rejected with code, no partial mutation on rejection |
| `RulesetRef` | ID, schema/version, content hash, explicit migration policy |
| `GrowthAward` | Level transition, stat inputs, HP/MP grants, rule version and relevant RNG record |
| `SaveSnapshot` | Coherent transaction sequence, character/world records and schema/content versions |

Use canonical five attributes `Strength`, `Endurance`, `Agility`, `Intelligence`, `Wisdom`. Source `AGI` and any interface “Dexterity” synonym map to Agility explicitly. Never infer field order from an array printed by a website; named fields prevent WIS/INT transposition.

## Command ownership

The player/UI issues intents such as `CreateCharacter`, `AllocateAttributePoints`, `TrainSkill`, `LearnSpell`, `EquipItem`, `UseAbility`, `Interact`, `TakeLoot` and `RequestTravel`. Local authority validates and commits. No UI or animation callback modifies authoritative XP, gold or health directly.

Exact C++ API names are owned by the architecture integrator. These semantic names are not an excuse to implement ten independent command buses. Prefer a small explicit interface with typed requests and results.

## Minimum provenance per mechanical value

`source_url`, `source_baseline`, `retrieved_date`, `status`, `value`, `notes`. For prototype tuning, `source_url` may be null and `notes` must say that the designer chose it. Changing a rules field that affects saved growth/items requires a migration or new character profile; cosmetic changes do not.

## Save and reward constraints

Persist learned skills, allocations, item instances, historical growth, current resources, XP debt, area states and quest flags as values. Never serialize pointers or assume map actor names stay stable. One committed gameplay snapshot includes both sides of a reward transaction. Re-applying the same `RewardId` must be harmless.

The example does not require a full event-sourced database. A small bounded growth history, world snapshot and reward/lifecycle state are sufficient. Periodic safe compaction may retain the outcome/high-water marks needed for idempotency instead of an unbounded list of every rat ever killed.

Persist mana-regeneration fractional progress and specify pause/offline behavior. Quiver equipment is a first-class slot; the selected Wooden Arrows quiver is unlimited. Avoid generic ammunition consumption defaults that contradict that data.
