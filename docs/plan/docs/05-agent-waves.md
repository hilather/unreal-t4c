# Agent implementation waves

Status: a work plan for agents and Unreal-capable artists/integrators. No tasks below have been implemented or validated by preparing this package. Task IDs are stable handoff references.

Read first: [game design](01-game-design.md), [Unreal architecture](02-unreal-architecture.md), [world and encounters](03-world-and-encounters.md), [art pipeline](04-art-pipeline.md), [validation](06-validation-and-delivery.md), [Lighthaven evidence](../research/lighthaven-evidence.md), [character rules evidence](../research/character-rules-evidence.md), [asset manifest](../assets/asset-manifest.json), and [contracts](../contracts/README.md).

## Execution contract

The deliverable is a Windows single-player vertical slice: create a valid character, enter the Lighthaven church, descend through all four basement floors, fight the researched enemy roster including the final encounter, progress/equip/use supported starting abilities, return, and continue from a coherent save. A bounded church exterior and source-positioned graybox service routes to necessary trainers, merchants and the mage tower are included; completing the whole town is subsequent work. Multiplayer services and production console work are later projects.

Every task must read the design, architecture, current research ledger, world specification, and relevant asset status before editing. Confirm unknown historical values as unknown. A temporary value is permitted only when recorded as prototype tuning with a replacement point. Do not add unresearched enemies or claim pixel-exact/map-exact reconstruction from incomplete images.

Use one coordinator/integrator, parallel implementers with exclusive ownership, and an independent reviewer at each significant gate. Agents may choose their configured models; this plan does not assume a model name or harness feature. The coordinator owns contracts, shared configuration, `.uproject`, module definitions, gameplay tags, shared data schemas, and merges unless a task explicitly transfers that ownership.

Text changes may use isolated Git branches/worktrees. `.uasset`, `.umap`, animation graphs, materials, input mapping assets, and other binary content have one writer at a time. Track ownership in `Docs/Implementation/asset-locks.md`; use LFS locks when available. A worktree does not make binary merges safe. Split work into separate assets or serialize it. Never solve a binary conflict by choosing an arbitrary branch's file.

Each task delivers a small patch, evidence, and a handoff note:

```text
Task ID and base commit:
Files/assets owned and changed:
Dependencies/contract revision:
Behavior implemented:
Historical values verified / prototype values introduced:
Tests/editor/package checks actually run, exact result:
Checks not run and why:
Known risks and next dependent task:
```

## Global stop and completion rules

- No Unreal installation/toolchain: code/schema work may continue, but mark build, editor, import, and packaging checks **not run**. Do not invent `.uasset` binaries or report a text stub as a playable asset. Resume the affected gate on a real Unreal-capable machine.
- Missing formula or contradictory historical evidence: implement the seam and mark prototype behavior, then resolve in the ledger before claiming parity. Do not fabricate test fixtures to endorse guessed mechanics.
- Missing asset rights/provenance: keep it in the reference workflow; use an approved placeholder for distributable builds until its status is resolved. Do not block unrelated gameplay engineering.
- Overlapping binary ownership, incompatible schema changes, corrupt saves, duplicated rewards, or broken travel: stop that integration and resolve before the next dependent wave.
- Do not broaden to multiplayer, a full MMORPG backend, browser streaming, the rest of Arakas, or a full-city art pass to avoid an unfinished Stage 1 gate.
- A task is complete only when its acceptance evidence exists. Source written, code compiled, editor tested, cooked, packaged, and manually played are separate statuses.
- Tests use native Unreal/C++ facilities. New Python benchmark frameworks, runtime game logic, or large text-inventory test suites are not authorized by this plan.

## Wave 0 — Close assumptions and establish a runnable project

Parallel work starts on research/specification and toolchain preparation. Shared contracts are approved by the integrator before code branches depend on them.

| ID / owner | Dependencies | Files/assets owned | Work and acceptance |
| --- | --- | --- | --- |
| W0-01 / research owner | Package sources | `Docs/Implementation/rules-ledger.md`, `world-ledger.md` | Select reference T4C ruleset/version where evidence permits; record confirmed, disputed, missing, and modernized fields. Floor-by-floor roster/links and Balork completion semantics have explicit status. No guessed HP/XP/loot numbers labelled authentic. |
| W0-02 / build owner | None | `Build/Toolchain.md`, proposed build configuration | Select installed stable UE version, compiler/SDK, Windows target, plugins and Git LFS rules. Create and compile a minimal C++ project; package/run a blank Windows test map. Record machine and engine revision. |
| W0-03 / architect + integrator | W0-01 initial schemas, W0-02 | `Core/`, schema headers, `.uproject`, module/build config, shared tags | Agree stable IDs, command results, attribute/progression DTOs, map/entrance IDs, definition schema, save schema v1. Publish interface examples and responsibility boundaries. |
| W0-04 / art/reference owner | Package art manifest | `Reference/`, `ArtSource/`, `Docs/Implementation/art-register.md` | Inventory provided/downloaded references and available approved stand-ins. Record source, author/status, intended use, derived asset relation. Establish scale, camera, silhouettes and material direction from the reference package. |

Gate G0: clean checkout builds on the pinned toolchain, a packaged executable launches, and the integrator freezes schema revision 1. If only planning/code access is available, record the missing machine/toolchain as a dependency rather than passing G0.

## Wave 1 — Character motion, rules, and combat foundation

Three implementation tracks can run together after G0. The combat/GAS integration consumes the rules track's agreed interfaces; changes to shared headers go through the integrator.

| ID / owner | Dependencies | Files/assets owned | Work and acceptance |
| --- | --- | --- | --- |
| W1-01 / controls owner | W0-03 | `Framework/LHPlayerController*`, `Framework/LHCharacter*`, `Content/Lighthaven/Input/`, camera assets | Elevated camera, bounded zoom/rotation, walking/running, mouse target selection, gamepad target cycling, interaction action. Test ramps/doors and focus loss; changing input context never leaves movement held. |
| W1-02 / rules owner | W0-01, W0-03 | `Rules/`, rules tests, ruleset data definition values | Implement creation eligibility, base/derived stats, equipment/spell requirements and verified core combat/progression calculations. Every supported formula has a boundary/rounding fixture tied to evidence; prototype fields remain tagged. |
| W1-03 / gameplay owner | W0-03, W1-02 interfaces | PlayerState/GameMode foundations, `Abilities/`, one dummy attack asset | Initialize player/enemy GAS ownership, route a basic attack through command validation, explicit impact timing and one-time damage. A destroyed target, repeated callback and failed resource check cannot damage or charge twice. |
| W1-04 / world owner | W0-01, W0-03 | `Maps/Dev_Movement`, `Maps/Dev_Combat` | Create small reproducible collision/camera and combat test rooms using standard placeholders. Own their binary maps exclusively. |

Gate G1: an actual character can run, target and attack a dummy with both input methods. The authoritative rules output explains the result; animation speed does not determine damage. A skeptical reviewer checks state ownership and duplicate hit paths. Do not add networking to demonstrate future readiness.

## Wave 2 — Create, save, load, and equip a character

Persistence, inventory/progression, and UI can work in parallel against frozen records. Integration happens on one branch once their individual work compiles.

| ID / owner | Dependencies | Files/assets owned | Work and acceptance |
| --- | --- | --- | --- |
| W2-01 / persistence owner | G1, W0-03 | `Persistence/`, persistence tests, save fixtures | Implement coherent snapshots, migrations, size bounds, A/B generations, single in-flight write, recovery UI events and failure reporting. Prove round trip, bad checksum, interrupted write simulation, future-version rejection and previous-generation fallback. |
| W2-02 / character owner | G1, W1-02 | `Character/`, item instances/definitions, character tests | Four-question/roll/re-roll creation commit, inventory/equipment, attribute allocation, level advancement, XP debt and growth history. Store creation version/answers; reject invalid items/points; recovering lost XP does not re-award a level; equipping then unequipping does not compound modifiers. |
| W2-03 / UI owner | G1, W0-03 interfaces | `UI/`, `Content/Lighthaven/UI/`, `Maps/L_Frontend` | Frontend, create/continue/select, name/appearance, legal stat setup, character sheet and inventory. Form errors preserve input; gamepad completes creation and equipment flow without a mouse. |
| W2-04 / integrator | W2-01..03 | Framework integration, shared definitions only | Wire new character to the existing `Dev_Movement` test map as a temporary entry, persist once, destroy runtime state and reconstruct from save. Wave 3 replaces the temporary entry with the hub. Confirm no live actor pointer is required in a serialized record. |

Gate G2: create two distinct local characters, equip one, change attributes through legal progression, exit the executable, relaunch and restore the correct independent record. Test an unreadable newest save. Display clear recovery/errors rather than silently creating a fresh character. This is the first mandatory packaged build after G0.

## Wave 3 — Graybox the full church-to-basement route

All four floors are included before final environment art. Parallel level work is safe only when every floor is a different owned `.umap` and shared kit assets are read-only.

| ID / owner | Dependencies | Files/assets owned | Work and acceptance |
| --- | --- | --- | --- |
| W3-01 / hub designer | G2, current world ledger | `Maps/L_LighthavenTempleDistrict`, hub-only Blueprints | Build church exterior/interior, safe spawn, first descent and source-positioned graybox service routes to the mage tower and required trainers/vendors. Keep actual NPC locations; no relocated Iraltok/Uranos or free-debug teaching. Mark reconstructed geometry. Camera and controller interaction work throughout. |
| W3-02 / dungeon designer A | G2, world ledger | `Maps/L_TempleB1`, `L_TempleB2`, floor-specific data | Graybox floors 1–2, stairs, navigation, encounter anchors and visual landmarks. Cross-check source route, reachability and portal direction. |
| W3-03 / dungeon designer B | G2, world ledger | `Maps/L_TempleB3`, `L_TempleB4`, floor-specific data | Graybox floors 3–4, final encounter space and required links. Same acceptance as W3-02; no species placement invented to fill empty space. |
| W3-04 / world systems owner | G2 | `World/`, area state/save integration, validators | Stable instance/spawn IDs, portal validation, source/destination checkpoints, load failure recovery, per-area state hydration and ID collision detection. |
| W3-05 / integrator | W3-01..04 | Map packaging list, agreed area registry | Integrate traversal and cook every playable map explicitly. No editor-only soft reference masks missing packaged content. |

Gate G3: a packaged player travels hub → 1 → 2 → 3 → 4 and back; quit/reload on every floor returns to a valid entrance/checkpoint. Missing destination/invalid spawn/corrupt area data has a tested safe outcome. Run a duplicate-ID scan over all maps after integration. Door/stair collision and the largest enemy placeholder pass navigation checks.

## Wave 4 — Complete the basement gameplay loop

The research/world owner supplies one traceable roster coverage matrix. Every verified species/variant assigned to the included basement receives a definition, placement and playable validation row. Balork is tracked separately as the final encounter. Unresolved floor placement remains explicit until settled; no task may declare complete roster parity while rows remain speculative.

| ID / owner | Dependencies | Files/assets owned | Work and acceptance |
| --- | --- | --- | --- |
| W4-01 / AI owner | G3, G1 combat | `AI/`, enemy base Blueprint, shared behavior tree/blackboard | Acquire/chase/attack/leash/death, bounded path retries, safe-zone behavior, encounter lifecycle. Two or more enemies can engage without awarding twice or blocking all exits. |
| W4-02 / enemy data owner | W0-01 final roster, W4-01 interface | `Data/Enemies/`, `Data/Encounters/`, enemy-specific children | Populate each researched variant and floor distribution, with separate prototype tuning where exact stats are unknown. Keep roster and implemented IDs in sync. |
| W4-03 / abilities owner | G3, W2-02 | Starting spell/attack assets, ability extensions, resource recovery | Implement melee, bow plus equipped unlimited-quiver eligibility, magic, passive mana recovery and mana consumables. Test cancellation, range, zero-mana recovery with no gold, regen timer reload and cooldown persistence. Trainer services consume the resulting ability definitions through W4-07. |
| W4-04 / reward/persistence owner | G3, W2-01 | Reward/lifecycle logic, world save records, transaction tests | One kill → one XP event plus finalized loot; full inventory rejects transfer intact; save/reload does not duplicate corpses, loot or rewards. Persist encounter/respawn state and life generation; floor travel never resets a cooldown or grants a duplicate reward. Verify ordinary respawns provide a sustainable progression route. |
| W4-05 / quest/UI owner | G3, W2-03 | Dialogue/quest data, HUD/spell/pause/death widgets | Church interaction/tutorial, clear descent objective, health/mana/target feedback, supported spell selection, death/respawn and boss completion flow. No lengthy quest system outside the slice. |
| W4-06 / integrator + level owners | W4-01..05, W4-07 | Exclusive scheduled map edits | Place encounters in each owner's map, integrate the final encounter and return flow, and validate boss flags/reward IDs against the world specification. |
| W4-07 / services owner | G3, W2-02, W4-03 definition contracts | Character service commands, service data; coordinate UI bindings with W4-05 | Implement TrainSkill, LearnSpell, BuyItem and SellItem with trainer/vendor proximity, prerequisites, gold and shared skill points validated atomically. Deliver source-grounded service definitions and placement instructions: mage tower, temple, Sigfried bow/quiver, Fali mana potion. W4-06 and map owners perform the physical NPC placement. Prove successful transactions, insufficient-resource rejection, full-inventory rejection and double-click idempotency. |

Stage 1A may be reviewed when the church, creation and first-floor subset of this wave works: the Samaritan errand, healer, legal basic combat, loot and save/reload. This is an intermediate slice, not completion of all requested enemies.

Gate G4 / Stage 1B gameplay: complete the entire route on fresh melee-, ranged- and magic-oriented legal builds. Earn skill training, equipment and spells through their source-positioned services and normal progression; debug teaching, level-up commands or free item grants do not satisfy acceptance. A magic-oriented character may use its legal starter weapon before earning its first damage spell. Exercise death, floor change during pending action, full inventory, simultaneous lethal hits, save failures and repeated interaction. Final-boss rewards remain single-claim across reload and map travel. Stage 1 is mechanically complete at this gate; visuals remain provisional.

## Wave 5 — Replace placeholders and finish presentation

Art production may begin from Wave 1, but final map dressing follows G4 so visuals do not hide an incomplete route. Approved original or licensed meshes can substitute for research-only imagery without changing IDs, hitboxes or formulas.

| ID / owner | Dependencies | Files/assets owned | Work and acceptance |
| --- | --- | --- | --- |
| W5-01 / environment artist | G3 layout freeze, art register | Church/basement mesh and material kit; no map writes | Produce coherent church masonry, basement walls/floors, stairs, doors, props and readability lighting references. Import with units, collision, material bounds and texture budget validated. |
| W5-02 / character artist | G1 combat timing, approved art direction | Player appearance/rig/animation assets | Produce minimum creation variations and locomotion, attack, cast, hit and death coverage. Retargeting and equipped weapon sockets validated at gameplay camera distance. |
| W5-03 / monster artist | W4-02 roster, approved references | Distinct monster meshes/sprites/rigs and animation assets | Preserve recognizable silhouette/scale cues, create full required animation coverage, and record derived/reference relationships. Never treat one generated picture as a finished animated game character. |
| W5-04 / effects/audio/UI owner | G4 | Effects, audio, final HUD/menus | Clear impact/resource/target cues, accessible contrast, readable text, volume/input settings and controller glyphs. Cosmetic changes cannot alter damage or award paths. |
| W5-05 / map owners scheduled serially per map | W5-01..04 | Respective owned maps only | Dress and light maps; preserve encounter/portal IDs and navigation. Re-run collision, occlusion and navigation walkthroughs after art replacement. |

Gate G5: the church and all four floors meet the selected visual direction; all roster entries have intentional presentation, and any remaining placeholders are named. Reference-only assets cannot leak into the packaged content audit. Check a clean cook to detect accidentally omitted art, stale redirects and hidden dependencies.

## Wave 6 — Package, profile, and independent acceptance

| ID / owner | Dependencies | Files/assets owned | Work and acceptance |
| --- | --- | --- | --- |
| W6-01 / QA reviewer | G5 | Acceptance report, reproduction saves, native test additions only for real gaps | Run clean-install creation → legitimate training/spell/gear acquisition → full descent → final encounter → return → exit/reload. Cover melee, ranged and magic build routes; repeat core menu/combat/travel using gamepad. Record observed results against every Stage 1 criterion. |
| W6-02 / performance owner | G5 | Insights captures, scoped optimization patches | Profile the agreed reference PC at target resolution/settings; capture busy combat and worst camera/room transitions. Fix measured bottlenecks, report before/after and remaining budget misses. No Python microbenchmark substitute. |
| W6-03 / adversarial reviewer | G4, final candidate | Review findings only; fixes assigned to owners | Challenge authority bypasses, duplicated rewards, save compatibility, missing IDs/content, disconnected nav areas, controller traps and unverified historical claims. Do not request a broad rewrite without a concrete defect. |
| W6-04 / release integrator | W6-01..03 resolved | Build config, release notes, packaged output | Compile, validate assets, cook/package Windows; launch outside the editor from a clean destination. Bundle controls, known limitations, asset credits and engine/ruleset/content revisions. |

Gate G6: playable Windows artifact, reproducible build instructions, checked acceptance report, no unresolved blocker/critical defects, and explicit remaining non-blocking limitations. The report states tested hardware, engine revision and whether historical parity is confirmed or still prototype. A functioning PIE session alone cannot pass.

## Later stages, deliberately outside Stage 1

1. Complete Lighthaven town from verified maps and landmarks, expand NPC interactions and quests, and add nearby wilderness once the existing slice is stable.
2. Extend character mechanics and content according to verified evidence, including the wider skill/spell/equipment range and rebirth when in scope.
3. Design and implement authoritative multiplayer: server ownership, replication and ability prediction, dedicated builds, online persistence, character identity, security, concurrency, operations and load tests. Never import arbitrary offline saves as trusted online progression.
4. Prepare Steam distribution and then an Xbox workstream with approvals, platform tools, performance/certification and complete controller validation. Neither is required to test the Windows prototype.

## Agent task prompt template

```text
Implement task <ID> from docs/05-agent-waves.md.
Read the architecture, design, current research/rules ledger and art status.
Base commit: <commit>. Contract revision: <revision>.
Own only: <paths/assets>. Other shared files require integrator coordination.
Implement the stated behavior; do not broaden the task or rewrite adjacent systems.
Preserve stable IDs and save compatibility. Use native Unreal/C++ tests for material risks.
For binary assets, acquire the recorded exclusive ownership before opening/editing.
Run the task acceptance checks that the actual environment supports.
Return the standard handoff with actual results, missing checks and concrete blockers.
Do not claim an editor/package/gameplay test passed unless it was actually run.
```

The coordinator closes a wave only after dependent code and content work together in the integrated project. Parallel progress is useful; a pile of isolated patches is not a playable milestone.
