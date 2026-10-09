# W4-01 enemy runtime and encounter director

Native C++ equivalents of the plan's shared Behavior Tree/Blackboard are used, per Matt's Wave 4 decision: this worker authors no binary assets. `LHEnemyRuntime.h` was published first in commit `9cd8178`. The runtime is not wired into the session/game mode; W4-06 owns that integration.

## Runtime spec and D09 allowlist

`FLHEnemyRuntimeSpec` contains definition/presentation IDs, capsule dimensions, maximum combat pools and derived combat attributes, attack config and requirement input, boss flag and the AI fields below. W4-02 owns actual catalog values and A-01 capsule adoption. No gameplay statistic or AI tuning defaults are supplied here. Required fields must be resolved, finite, nonnegative (strictly positive for dimensions/ranges/AI keys), with non-Missing/non-Disputed provenance. Formula and eligibility validation reuse `LH::Rules`. Unknown presentation IDs reject. `ApplyParameters` checks the entire array on a copy before committing; unknown/duplicate/missing/unresolved keys reject. Counts must be positive int32 integers, leash >= aggro, home tolerance < leash.

| Required key | Unit | Meaning |
|---|---|---|
| AI.AggroRadiusCm | cm | LOS acquisition radius |
| AI.LeashRadiusCm | cm | Bound about authored home; boss uses B4.BalorkArena center |
| AI.MoveSpeedCmPerSec | cm/s | Character navigation speed |
| AI.PathRetryLimit | integer attempts | Consecutive failed path requests before return; also bounds home-path failures |
| AI.MaxActivePursuers | integer enemies | Floor chase/attack cap; minimum cap among participating enemies wins |
| AI.DecisionIntervalSeconds | seconds | Minimum active time between decisions; one decision per boundary, no catch-up loop |
| AI.HomeToleranceCm | cm | Home arrival/navigation acceptance tolerance |
| AI.SpawnSafetyDistanceCm | cm | Minimum player distance for respawn, with LOS suppression |

W4-02 must label missing AI tuning Prototype, rather than assert historical behavior. R-03 `w4-bible-lookup.md` marks respawn safety distance missing and recommends Prototype 1000 cm; this runtime requires the catalog to provide it explicitly. No new Bible retrieval or historical number is introduced. Balork delay belongs to W4-04: Bible 15:00, as recorded in R-03 and Matt's answers. There is no permanent-defeat boss exception.

## States and navigation

Idle → Acquire → Chase → Attack → ReturnHome → Idle; Dead is terminal. Acquisition tests a single bound player, distance, LOS, safety and pursuer capacity. No player actor search occurs every frame. `TickActiveSimulation` drives interval decisions; controller PostPhysics tick only guards cached actor references and cached NoCombat bounds before TimerManager impacts. A dead player or frozen simulation cancels enemy actions and movement. Pause stops world movement/impact timers and active-clock advancement.

Navigation requests require a full nonpartial synchronous path. Every path segment is checked against NoCombat bounds expanded by the actual capsule, and every path point must stay within the leash. Navigation speed and agent dimensions use the spec. Failures from rejected requests or navigation completion count toward the retry budget. Exhaustion enters ReturnHome; home-path exhaustion stays stopped in ReturnHome until unloaded, without issuing more path requests. Returning enemies do not reacquire until reaching the home tolerance. Live reload restores health at the authored ground anchor plus actual capsule half height (`AnchorKeepHealth`), rather than storing transient placement.

Bosses require the generated `B4.BalorkArena` target point (runtime actor name, or an explicit same-name tag), using its position as leash center. A boss marker outside this leash is rejected. There are no phases, summons, arena locks or victory-on-travel behavior.

Tagged `ATriggerBox` bounds (`LH.Safety.NoCombat.*`) are cached during Populate, including NoCollision boxes. NoCombat crossing, entering, or attack segments reject; path validation uses conservative expanded world AABBs. The PostPhysics guard cancels a windup when the player moves into a zone, and rolls back enemy movement that crossed a protected boundary or leash to its last safe point. These generated volumes must remain static while loaded; repopulate to refresh altered bounds. Conservative AABBs may reject otherwise navigable routes near rotated volumes; generated safety boxes are axis aligned. No obstruction is fabricated as a successful route.

## Director bindings and lifecycle

W4-06 sets `RunId`, `Player`, `ResolveSpec`, `SettleKill`, `AdvanceRespawns`, then calls `Populate` with a validated area record. A missing spec/settlement binding fails closed. `Populate` replaces this floor's actors and spawns only Alive records with valid positive persisted health. Dead, RespawnPending and PermanentlyDefeated records spawn nothing; no implicit enemy population is created from absent records. Marker definition and area must match the record. Spawn collision uses DontSpawnIfColliding and an authored anchor, never an adjusted rescue location.

W4-06 must restore and capture each combat component's RNG and remaining cooldown through the existing session record/component APIs before enabling AI. The actor's initial deterministic stream is seeded from slot/generation only for a newly created life; repopulation alone is not a persisted RNG restore. This task does not add a save shape or silently claim combat RNG/cooldown round-trip.

`CaptureLive` updates current health only for matching Alive actors at a completed combat boundary. Integration must ensure the entire floor has completed actions before exporting a snapshot; CaptureLive does not establish that global boundary. It preserves the record's resolution/provenance and lifecycle; it never settles a death.

`SpawnLife` is invoked only AFTER W4-04 commits an Alive new-generation respawn. It does not authorize lifecycle changes, invent generations or advance respawn timers. `IsSpawnSafe` requires a loaded/unfrozen/unpaused floor, a player, catalog resolution, player distance, no player LOS to the marker, no safety-zone overlap, a nav projection, blocking-overlap clearance and a supporting floor trace. It conservatively suppresses all LOS-visible spawns (stronger than screen visibility). W4-04 must defer/reset eligible timers according to its policy when this returns false. Marker scans occur on these boundary calls, not per-frame acquisition.

`FindByLife` includes area/slot/generation. `GetEntityId(RunId)` materializes the area-qualified spawn-slot identity; generation checks belong to FindByLife and combat's life revision. `FindByEntity` is for current actors; integrations must retain source life separately when targeting corpse loot.

Death first latches the area/slot/generation, makes the corpse nonblocking to pawns, then calls `SettleKill` once. The latch survives same-floor repopulation/despawn. Duplicate/reentrant death publication cannot settle twice. Each enemy has its own latch. The callback's killer comes from the single-player `Player` binding because the existing combat death event contains hit identity but no attacker actor. Environmental/friendly/NPC damage attribution is unsupported and requires an explicit future combat event extension. Integration must keep settlement callbacks synchronous and transactional. A false result logs an error; the director does not retry the callback (once-per-life contract). W4-06/W4-04 must retain the failed transaction, block further durable saves/travel and recover it; do not repopulate a stale Alive save as a substitute for settlement.

Actors remain corpses until `Despawn` or floor replacement. Capsule visibility stays targetable, but Pawn blocking/navigation influence are disabled to avoid corpse doorway traps. No reward, loot roll, quest claim or boss single-claim flag is implemented by AI. W4-04/W4-05 own those durable decisions. Deinitialize destroys enemies/controllers, clears bindings and stops loaded-floor advancement.

## Presentation and open checks

`LHEnemyPresentation.cpp` transcribes A-03 E01–E11 primitive recipe dimensions, centers/endpoints and palettes from `art/placeholders/README.md`; engine spheres/cubes/cylinders/cones are assembled on a floor-root at -capsule-half-height. These dimensions and colors inherit A-03 Prototype provenance (2026-10-08); they are cosmetic. All visual children disable collision, overlaps and navigation influence. Family dead poses are static reduced-motion collapses, with weapons laid on the floor. Capsule dimensions come solely from W4-02. The obsolete A-03 statements that Balork never respawns are superseded by Matt's Wave 4 decisions.

No optional pulses, nameplates, selection pointers, loot badges, dynamic lights or D14 attack cues are authored; those require their UI/presentation owner's wiring. Real-map nav rebuild/cook, all 77 anchor spawns, largest-enemy doors/stairs (especially Balork), exit passing with concurrent pursuers, safe-zone retreat during windup, and legal melee/ranged/magic full-route play remain coordinator host checks. Primitive material appearance and dead-pose bounds need visual review; no screenshot or visual acceptance is claimed.

## Validation evidence

See the attempt handoff for exact build/test commands, observed counts, logs and remaining prerequisites. Native tests are registered as `Lighthaven.AI.SpecValidation`, `StateMachine`, `BoundedPathRetry`, `SafeZone`, `TwoEnemiesOneSettlement`, `DirectorPopulateAndCapture`, `PausedAndUnloadedNoTick`. The state test builds a real small Recast navmesh; the failure test deliberately lacks navigable geometry. Existing Abilities and ControlsCombat regressions are part of the requested headless suite.
