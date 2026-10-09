# Task W4-04: rewards, encounter lifecycle, loot and persistence transactions

Implement W4-04 from `docs/plan/docs/05-agent-waves.md` (reward/persistence owner). **Group A** (parallel: W4-01, W4-03). **Base:** main with R-03 and **W4-08** merged (it edits the codec files you extend). Commit headers (items 1–2) early; keep them stable (W4-02/03/05/07 use them).

Read: `research/w4-bible-lookup.md`; `02-unreal-architecture.md` §§5–7; `03-world-and-encounters.md` §6; `world-ledger.md` "Balork and completion semantics", "Respawn and renewable progression"; `persistence.md`; `world-systems.md` (area state); `schema-rev1-freeze.md` D04, D05, D06, D09, D12, D15; `contracts-v1.md` (encounter, corpse, reward ID, receipt rows); `prototype-policy.json` (`loaded_area_unpaused_simulation`, `death_penalty: none_during_initial_development`; its `boss_respawn: false` is superseded by Matt: note it); `schema-rev2.md`; `rules-ledger.md` (item 6 row).

## What exists
- DTOs: `FLHEncounterRecord` (Life, State, CurrentHealth, RespawnRemainingSeconds, KillReward, bRewardCommitted), `FLHCorpseLootRecord`, `FLHBossRecord`, `FLHQuestRecord`, `FLHWorldRecord::ClaimedUniqueRewards`, `FLHRequestReceipt`.
- `Persistence/LHSaveCodec.*`: `EnemyLifeRewardId`, `GrowthRewardId`, `RewardIdFromDigest`; request digests allowlisted only for CreateCharacter, AllocateAttributePoints, EquipItem, UseItem (`DispatchRequest`); `LHIdentityTests.cpp:136` asserts TrainSkill unsupported. `LHSaveValidation.cpp` validates encounters/corpses/quests/bosses.
- `World/LHAreaStateSubsystem` (Hydrate/StoreArea/PersistInto, high-water), `ALHSpawnMarker::ResolveLife`. No population, settlement, loot rolls or respawn ticks.
- `Character/LHCharacterAuthority`: `GrantExperience` (multi-level, growth IDs), `AddItem`, `RemoveItem`, Import/Export validation (items must be in the profile catalog; `InventorySlots` caps entries). Work on a copy (`auto Copy=Authority; Copy.Import(S); …; Copy.Export(S)`).

## Work
1. **Persistence helpers (commit first, in `Persistence/`).**
   - Canonical request wires + digests for `TrainSkill`, `LearnSpell`, `BuyItem`, `SellItem`, `UseAbility`, `Interact`, `TakeLoot` (D04 rules; vectors in tests). Flip that TrainSkill assertion.
   - Receipt helpers for W4-06's wrapper (domain functions write none), same semantics as `FLHCharacterAuthority` Begin/Commit (epoch check; same ID+digest replays with original sequence + `bReplay`; different digest rejects `ReusedRequestId`; 4,096 cap; sequence overflow), e.g. `bool LHSave::BeginRequest(const FLHSaveSnapshot&, FName Command, const UScriptStruct*, const void*, FLHCommandResult& Out, FString& Digest)` and `void LHSave::CommitRequest(FLHSaveSnapshot&, const FLHRequestId&, const FString& Digest, FLHCommandResult& Out)`.
   - D06 reward IDs: `QuestTurnInRewardId(Run, Quest, Stage)`, `BossUniqueRewardId(Run, Boss)`.
2. **Reward types (`Source/Lighthaven/Rewards/LHRewardTypes.h`).** `FLHKillRewardSpec` (XP, gold min/max, item drop chance, `TArray<FLHLootEntry>`, RespawnSeconds, SafetyDistanceCm, RespawnPolicy OrdinaryRepeat/PermanentDefeat, boss flag), `FLHKillFacts` (area, life, enemy definition, killer is player), `class ILHKillObserver { virtual ELHCommandReason OnSettledKill(const FLHKillFacts&, FLHSaveSnapshot& InOut) = 0; }`, run in SettleKill's transaction (W4-05's rat counter/boss flow implement it).
3. **Lifecycle (`Rewards/LHEncounterLifecycle.*`).**
   - `PopulateArea(FLHAreaRecord&, const FLHAreaDefinition&, TFunctionRef<FLHNumber(const FLHContentId&)> MaxHealth)`: on first visit only, Alive generation-0 records at full health for every registry spawn; never on reload or over existing records.
   - `SettleKill(FLHSaveSnapshot& InOut, const FLHKillFacts&, const FLHKillRewardSpec&, const FLHCharacterProfile&, TArrayView<ILHKillObserver* const>)`, one all-or-nothing transaction: reject a lower generation or a committed current one (idempotent); mint `EnemyLifeRewardId`; grant XP via an authority copy; finalize corpse loot via the gameplay RNG stream (persisted, never rerolled) in a container with life-deterministic EntityId (`FLHEntityId LHRewards::CorpseContainerFor(const FGuid& Run, const FLHSpawnLifeId&)`, W4-05 uses it); set `bRewardCommitted`; set Dead + RespawnRemaining (Balork too: Bible 900 s); a boss's first defeat also sets `FLHBossRecord.bDefeated` and the `BossUnique` claim exactly once, later lives settle ordinary kill rewards only (PermanentDefeat: supported, unused); run observers.
   - `AdvanceRespawns(FLHAreaRecord&, double ActiveSeconds, TFunctionRef<bool(const FGuid&)> IsSpawnSafe, TArray<FLHSpawnLifeId>& NewLives)`: active-simulation time only; due-but-unsafe slots wait; a new life increments generation once, with a fresh reward ID; a boss respawns too; its claim stays.
4. **TakeLoot.** `ELHCommandReason LHRewards::ExecuteTakeLoot(FLHSaveSnapshot&, const FLHCharacterProfile&, const FLHTakeLootRequest&)`: item or gold kind (`LHValidateLootTransferPayload`), ownership/quantity/capacity; full inventory rejects, both sides intact; corpse `bClaimed` matches an empty remainder. Tests compose it with item 1 helpers (replay). Ordinary-loot cleanup policy: item 6. Unique quest items stay recoverable.
5. **Player death.** `LHRewards::SettlePlayerDeath(FLHSaveSnapshot&, ...)`: `none_during_initial_development` penalty (no XP/item/level loss); active entrance becomes `Session.SafeRespawn`; HP/MP after death per the lookup (Prototype if `missing`); settled once before respawn; never lose the only inventory copy (W4-06 wires it).
6. **Values** from the lookup/ledgers; a declared Prototype (provenance + replacement point) only where `missing`: respawn time (fallback 120 s), respawn safety distance, loot interpretation (ledger row), cleanup duration.
7. **Sustainability check.** Native simulation over B1 slots (12 rats, 3 bats, 2 slime), fixture specs: respawns let a fresh character reach 15 rat kills and accumulate gold (W4-06 repeats it with real data).
8. Doc `docs/implementation/rewards-lifecycle.md`: transaction order, reward ID table, policies, interfaces, helper usage.

## Acceptance (`Lighthaven.Rewards.*`, `Lighthaven.Persistence.*`)
- `Lighthaven.Persistence.NewCommandDigests`: frozen vectors for the 7 new commands; mismatched type rejects.
- `Lighthaven.Persistence.RequestReceiptReplay`: replay returns original sequence; different payload or wrong epoch rejects.
- `Lighthaven.Rewards.OneKillOneReward`: repeated settle/OnDeath callbacks → one XP event, one finalized loot, one observer call.
- `Lighthaven.Rewards.FullInventoryLootIntact`: rejection leaves corpse and inventory byte-identical.
- `Lighthaven.Rewards.SaveReloadNoDuplicate`: settle → Encode/Decode → reload → no duplicate corpse, loot, XP or claim; no loot reroll.
- `Lighthaven.Rewards.RespawnGenerationAndClock`: the spec's active respawn time respawns with generation+1, new reward ID; paused/unloaded time excluded; unsafe slot waits; travel never resets the timer.
- `Lighthaven.Rewards.BossRespawnSingleClaim`: Balork respawns after 900 s active time (generation+1); boss flag/claim granted once across reload, respawn, a second kill and travel.
- `Lighthaven.Rewards.PlayerDeathSettlesOnce`, `Lighthaven.Rewards.RenewableRatRoute`.

## Own only
`Source/Lighthaven/Rewards/` (new), `Source/Lighthaven/Persistence/`, `Source/LighthavenTests/Rewards/` (new), `Source/LighthavenTests/Persistence/`, `docs/implementation/rewards-lifecycle.md` (new).
Read-only: `Core/`, `Character/`, `World/`, `AI/`; no session wiring (W4-06). Schema rev 2 is W4-08's: don't change envelope, version or migration; extending the request allowlist and reward helpers is in scope.
