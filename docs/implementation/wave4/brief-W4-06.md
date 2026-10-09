# Task W4-06: Wave 4 integration (session, encounters in maps, Balork, return)

Implement W4-06 from `docs/plan/docs/05-agent-waves.md` (integrator). Runs **last**, as two same-scope submissions:
- **W4-06a (Stage 1A slice):** after W4-01..05 merge (W4-07 may be in flight). Church + creation + B1 (item 8 Stage 1A checklist).
- **W4-06b (G4):** after W4-07 and W4-06a merge; base: main then. Ranged/magic, services, B2–B4, Balork, return.

Read: `research/w4-bible-lookup.md`; Wave 4 + Gate G4; `06-validation-and-delivery.md` §§2–3 (V03–V19); `w3-integration.md`, `world-systems.md`, `w2-integration.md`; Wave 4 docs (`ai.md`, `abilities.md`, `rewards-lifecycle.md`, `enemy-data.md`, `quests-ui.md`, `services.md`, `schema-rev2.md`); `hub-graybox.md`, `basement-b1-b2.md`, `basement-b3-b4.md`; `schema-rev1-freeze.md` D06, D12, D15, D16.

## What exists
- `Framework/LHWave2Session.*` is the sole `ILHCommandHandler`; item 1's eight commands (incl. W4-08's UseItem) are `LH_UNSUPPORTED`. `Bind/InstallDerived` set GAS attributes, never an attack. GAS health/mana changes **never** reach `Complete` (the snapshot).
- `Framework/LHWave2Profile.cpp`: synthetic "Wave2Prototype" (test bow, 0 gold); `Compatibility().MaxGrowthAwards=3`. `LHWave2Closure.cpp` hashes profile + registry into `CatalogHash`.
- `ALHPlayerState::HandleAvatarDeath` only clears the avatar (no respawn). Nothing spawns enemies in playable maps.
- B1 `Nevanis`/`Shovanis` are tagged `ATargetPoint`s without GUIDs (B1 generator, `Mark(...)`). B4 has `B4.BalorkArena` (TargetPoint) and an altar.

## Work
1. **Command wiring.** Replace stubs: UseAbility→`LHAbilities::ExecuteUseAbility`; UseItem→`LHAbilities::ExecuteUseItem` (persisted; sync GAS→snapshot before, mana→GAS after); Interact→`LHQuests::ExecuteInteract`; TakeLoot→`LHRewards::ExecuteTakeLoot`; Train/Learn/Buy/Sell→`LHServices::Execute` (06b). One wrapper: `LHSave::BeginRequest` (replay/epoch)→domain function on a snapshot copy→character-authority Import validation→`LHSave::CommitRequest`→swap→`Published`→save. Except **UseAbility**: runtime-only, no receipt/sequence/save at request time (D16); effects persist via settlement and boundary sync. Split gating: `IsBlocked()` (gates W4-05's movement/attack) = only travel freeze/missing content/dead-awaiting-respawn; save-in-flight gating moves to private `IsTransactionBlocked()` (persisted commands, Continue, Exit). Autosaves coalesce (D12); queued/in-flight saves never freeze movement/combat. Fill contexts (NPC id/entity, distance, LOS, combat components) from live actors.
2. **Stage 1 profile + catalog.** Rebuild the profile (new ruleset/content rev) from W4-02 items, W4-03 abilities + `MakeStage1PrototypeCombat`, W4-07 offers, W4-05 quests. Starting state (Matt Q1): the Bible's starting character per `w4-bible-lookup.md`/`rules-ledger.md` (HP, MP, gold, skills, kit, growth); the ledger's "Separate prototype proposals" only where the lookup says `missing`, labelled Prototype and listed in your doc. Thresholds: the Bible XP table; `MaxGrowthAwards` sized to it. Add every catalog to `GameplayCatalogClosure`. G3 saves become incompatible (accepted, Matt Q9); document it.
3. **Encounters in maps (runtime, not hand-placed).** Per floor, bind `ULHEncounterDirector`: `ResolveSpec`→`LHEnemyData`; first visit→`LHRewards::PopulateArea`; `SettleKill`→`LHRewards::SettleKill` with `FLHQuestKillObserver`; `AdvanceRespawns`/`IsSpawnSafe`; `CaptureLive` + GAS→snapshot resource sync at each completed boundary (kill, loot, travel capture, save, exit). Pause/travel freeze stop AI and timers. Placement stays the 77 registry spawn markers; validate against W4-02's encounter rows.
4. **Generators.** `LHGenerateBasementAMapsCommandlet`: replace the two B1 TargetPoints with `ALHInteractableMarker` `NPC.Nevanis`/`NPC.Shovanis` (freshly minted literal GUIDs) at the **existing** `B1.NPC.*` positions (06b may refine per `services.md`); deterministic fingerprints. Hub generator: only changes `services.md`/`quests-ui.md` require; don't move NPCs. B4 generator: explicit arena bound only if W4-01's leash needs one. **Never change a registry entrance transform** (un-reviews the arrival; travel blocked until host re-review); nearby geometry changes also need a coordinator re-review.
5. **Validator.** Extend `LHValidateWorld`: each offer/quest-referenced NPC interactable exists once, unique GUID; boss slot/flags/reward purpose match the ledgers (Balork B4, OrdinaryRepeat, 900 s, BossUnique claim); duplicate-ID scan covers all IDs.
6. **Death, respawn and return.** PlayerState death→`LHRewards::SettlePlayerDeath`→save→W4-05 death screen→respawn at church `SafeRespawn`. Floor change with a pending action: refused or settled per D12. Balork defeat→`Quest.BalorkReturn` objective→completed only by the church Brother Kiran Interact topic (arrival/travel grants nothing); single claim across reload, travel and Balork's 15:00 respawn.
7. Implement W4-05's new UI read/session virtuals in the session (HUD, topics, offer views, respawn, pause).
8. Doc `docs/implementation/w4-integration.md` with **two host checklists**. Stage 1A: create character→church→Samaritan accept→B1→legal-melee rat kills→loot→Nevanis heal→die/respawn→15 rats→turn in; quit/reload at each step. G4: fresh melee/ranged/magic builds; training, bow/quiver, spells earned from source-positioned services (no debug); full descent B1–B4; Balork; return; exit/resume; death; mid-action floor change; full inventory; simultaneous lethal hits; save failure; repeated interaction; packaged run.

## Acceptance (`Lighthaven.Integration.Wave4.*`)
- `…Stage1A.ErrandLoop`: session-level create→accept→15 settled rat kills (with respawns)→turn-in→save/reload→no duplicates.
- `…LiveMeleeCommand`: live session attack: UseAbility→combat→SettleKill once; XP and loot persist.
- `…ResourceSyncAtBoundary`: damage taken/mana spent survive travel/reload; no extra regen tick.
- `…DeathRespawnSafe`: death→church; inventory intact; no duplicate rewards.
- `…ServicesThroughSession` (06b): train/learn/buy/sell via session with real catalogs; double submit replays.
- `…ProgressionRouteAffordable` (06b): real-data normal kills fund the first damage spell and bow+quiver (prints kill/gold budget).
- `…BalorkSingleClaim` (06b): defeat→15:00 respawn→second kill→reload→travel→return; one claim.
- `…UseItemThroughSession` (06b): a mana potion via the session restores live mana once; replay doesn't consume twice.
- `…CatalogCrossRefs`: every offer/loot/quest item and ability resolves; 11 roster IDs map to the 77 slots.
- Existing Wave 2/3 integration tests: update for new profile; don't delete.

## Own only
`Source/Lighthaven/Framework/` files `LHWave2Session.*`, `LHWave2Profile.*`, `LHWave2Closure.*`, `LHSessionSubsystem.*`, `LHPlayerState.*`, `LHGameMode.*`, `LHGameState.h`, `LHCharacter.*`, `LHDevCombatFixture.*`, `LHArrivalReview.h`, plus new files only under `Source/Lighthaven/Framework/Wave4/` (not `LHEnemyCharacter.*` [W4-01] or `LHPlayerController.*` [W4-05]); `Source/Lighthaven/World/`; `Source/LighthavenEditor/Commandlets/LHGenerateHubMapCommandlet.*`, `LHGenerateBasementAMapsCommandlet.*`, `LHGenerateBasementBMapsCommandlet.*`; `Source/LighthavenEditor/Validation/`; `Config/` (except `Config/Lighthaven/ReviewedArrivals.tsv`); `Source/LighthavenTests/Integration/`; `Source/LighthavenTests/World/`; `docs/implementation/w4-integration.md` (new).
Read-only: all W4-01..05/07/08 paths (you replace W4-08's UseItem stub in the session). Wrong domain function? Record defect + owner in your doc; don't patch it.
