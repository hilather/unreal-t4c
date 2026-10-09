# Task W4-01: enemy AI, encounter runtime and enemy base actor

Implement W4-01 from `docs/plan/docs/05-agent-waves.md` (Wave 4, AI owner). **Group A**: parallel with W4-03 and W4-04. **Base:** main with R-03 and W4-08 (schema rev 2) merged. Dependencies: G3, G1 combat (both on main). W4-02 (enemy data) starts after your interface lands, so publish the interface header in an early commit and keep it stable.

Read: Wave 4 + Gate G4 in `05-agent-waves.md`; `02-unreal-architecture.md` §§3, 5, 6 (AI states, lifecycle); `03-world-and-encounters.md` §§4–6; `docs/implementation/combat-foundation.md`; `world-systems.md` (markers, area state, "W4 owns explicit population"); `basement-b1-b2.md`, `basement-b3-b4.md` (nav bounds, safe zones, Balork extent); `art/creatures/README.md` + per-species files and `art/placeholders/README.md` (capsules, placeholder shapes, dead poses); `schema-rev1-freeze.md` D09 (AI keys need a registered allowlist), D12, D14, D16.

## What exists
- `Framework/LHEnemyCharacter.*`: ACharacter + `ULHCombatComponent`; `BeginPlay` calls `LHDevCombat::InitializeForMap` (synthetic dev tuning). No AI controller, no spec, no life identity.
- `Abilities/LHCombatComponent.*`: `RequestBasicAttack`, `ValidateAttack`, `OnDeath` (once per life), `OnImpact`, `IsActionPending`, `RestoreRemainingCooldown`. Reuse it for enemy attacks; do not fork combat.
- `World/LHWorldMarkers.h`: `ALHSpawnMarker` (Area, SpawnId, EnemyDefinitionId, `ResolveLife`). All 77 slots are authored in `World/LHAreaRegistry.cpp` and placed by the generators; no enemies spawn at runtime today ("no AI is spawned by these adapters", `w3-integration.md`).
- B1 safe zones: `ATriggerBox` actors tagged `LH.Safety.NoCombat.Required` (no collision). B4: `ATargetPoint` `B4.BalorkArena`. Each floor has a NavMeshBoundsVolume; runtime navmesh is not baked in the committed maps.
- `Lighthaven.Build.cs` has no `AIModule`/`NavigationSystem`.

## Work
1. **Build deps.** Add `AIModule` and `NavigationSystem` (public, so tests link) to `Source/Lighthaven/Lighthaven.Build.cs`; touch `LighthavenTests.Build.cs` only if required. Dependency edits only.
2. **Interface header (commit first): `Source/Lighthaven/AI/LHEnemyRuntime.h`.** `struct FLHEnemyRuntimeSpec`: enemy ContentId, PresentationId, capsule radius/half-height, `FLHBasicAttackConfig Attack`, max health (`FLHNumber`), boss flag, and the AI parameters. Publish the **D09 allowlist** as `LHAI::AllowedParameterKeys()` with name, unit and meaning for each key (for example `AI.AggroRadiusCm`, `AI.LeashRadiusCm`, `AI.MoveSpeedCmPerSec`, `AI.PathRetryLimit`, `AI.MaxActivePursuers`) plus `bool LHAI::ApplyParameters(const TArray<FLHMechanicalField>& AIParameters, FLHEnemyRuntimeSpec& InOut, FString& Error)` (the `ULHEnemyDefinition::AIParameters` shape) that rejects unknown keys, duplicates and unresolved required values, and `bool LHAI::ValidateSpec(const FLHEnemyRuntimeSpec&, FString&)`. W4-02 fills these values; you supply no historical numbers.
3. **Enemy base actor.** Rework `ALHEnemyCharacter` so that: a spec drives capsule, combat config and placeholder presentation (A-03 engine-primitive assembly keyed by PresentationId, collision off on visuals, capsule is the only blocker); it carries its `FLHSpawnLifeId` and exposes `GetEntityId(RunId)`, `GetLife()`, `IsAlive()`, `IsCorpse()`. Keep dev maps working: with no spec, keep the current `LHDevCombat` path so `Lighthaven.Integration.ControlsCombat` and the Abilities tests stay green.
4. **AI controller (native C++ state machine, not BT assets).** States Idle → Acquire → Chase → Attack → ReturnHome → Dead, event/timer driven (no per-frame full-world scans). Acquire within aggro radius with LOS; chase via navigation; attack through `RequestBasicAttack` only; leash past radius (Balork bounded to `B4.BalorkArena` + leash) and return to anchor; **bounded path retries** (give up → ReturnHome, never spin); never enter or attack into `LH.Safety.NoCombat.*` volumes; stop on player death, travel freeze and pause. Plan wording asks for a "shared behavior tree/blackboard"; record in your doc that the native FSM is used because workers cannot author binary assets.
5. **Encounter director: `ULHEncounterDirector` (UWorldSubsystem, `AI/LHEncounterDirector.*`).** Spawns one enemy per `ALHSpawnMarker` whose area record life is Alive, at the authored anchor with the persisted health (D09 `AnchorKeepHealth`), and spawns nothing for Dead/RespawnPending/PermanentlyDefeated lives. Balork respawns like any other life (the Bible's 15:00 respawn, Matt 2026-10-09); no permanent-defeat special case. Inputs come through bindings set by W4-06 (no Framework/Rewards includes):
   - `TFunction<const FLHEnemyRuntimeSpec*(const FLHContentId&)> ResolveSpec`
   - `TFunction<bool(const FLHSpawnLifeId&, const FLHHitIdentity&, AActor* Killer)> SettleKill`, called **exactly once per life** from `OnDeath`, then the actor stays as a corpse (A-03 dead pose) until told to despawn.
   - `void Populate(const FLHAreaRecord&)`, `void CaptureLive(FLHAreaRecord&) const` (current health of live enemies at a completed boundary), `void SpawnLife(const FLHSpawnLifeId&)` (respawn), `void Despawn(const FLHSpawnLifeId&)`.
   - `void TickActiveSimulation(float Seconds)` plus a `TFunction<void(float)> AdvanceRespawns` binding, driven only while the floor is loaded and unpaused, and `bool IsSpawnSafe(const FGuid& SpawnId) const` (player distance/visibility predicate for W4-04).
   - `FindByLife`, `FindByEntity` for the controller and loot.
   Cap concurrent pursuers (Prototype parameter from the spec; plan suggests 1–3). Two or more enemies may engage one player; each settles its own life once; enemies must not park in doorways and block all exits (leash/return frees them).
6. Doc `docs/implementation/ai.md`: states, parameter allowlist, bindings, safe-zone and leash rules, nav assumptions, open items (A-01 capsule adoption is W4-02's data; your actor applies it).

## Acceptance (native Automation, `Lighthaven.AI.*`)
- `Lighthaven.AI.SpecValidation`: unknown/duplicate/unresolved keys reject; valid spec builds.
- `Lighthaven.AI.StateMachine`: acquire inside radius with LOS, no acquire through a wall, chase, attack via combat component, leash → ReturnHome at anchor, Dead is terminal.
- `Lighthaven.AI.BoundedPathRetry`: unreachable target ends in ReturnHome after N retries; no tick spin.
- `Lighthaven.AI.SafeZone`: enemy never attacks a player inside a NoCombat volume.
- `Lighthaven.AI.TwoEnemiesOneSettlement`: two enemies engage; each death calls `SettleKill` once; simultaneous lethal hits on one enemy settle once.
- `Lighthaven.AI.DirectorPopulateAndCapture`: Alive/Dead/RespawnPending/PermanentlyDefeated records spawn the right actors; `CaptureLive` round-trips health; reload never resurrects Dead.
- `Lighthaven.AI.PausedAndUnloadedNoTick`: no respawn advance while paused or with zero active time.
- Existing `Lighthaven.Abilities.*`, `Lighthaven.Integration.ControlsCombat` and dev maps keep passing.
Use small synthetic test worlds with an in-test navmesh; real-map nav walks are host checks (list them in your doc).

## Own only
`Source/Lighthaven/AI/` (new), `Source/Lighthaven/Framework/LHEnemyCharacter.h`, `Source/Lighthaven/Framework/LHEnemyCharacter.cpp`, `Source/Lighthaven/Lighthaven.Build.cs` (dependency lines only), `Source/LighthavenTests/LighthavenTests.Build.cs` (only if needed), `Source/LighthavenTests/AI/` (new), `docs/implementation/ai.md` (new).
Read-only: `Abilities/` (W4-03 owns it this wave), `World/`, `Framework/LHDevCombatFixture.*`, generators. Do not wire the director into the session or game mode (W4-06 does).
