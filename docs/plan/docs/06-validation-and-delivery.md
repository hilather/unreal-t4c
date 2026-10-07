# Validation, delivery and future multiplayer boundary

## 1. Evidence levels

Use separate completion fields: `written`, `compiled`, `editor_checked`, `packaged`, `played`, `parity_verified`. A Markdown plan is not implementation. Passing C++ compilation does not verify a map, controller menu or art asset. Agent reports must identify tests actually run, skipped tests and the machine/toolchain used.

Build game tests with Unreal Automation/Functional Testing and native C++ fixtures. Use Unreal Insights for profiling. Do not introduce Python gameplay tests, benchmark harnesses or a second implementation of the formulas. Packaging helpers may use the platform's ordinary build scripts; gameplay remains in the engine/native modules.

## 2. Required functional acceptance

| ID | Scenario | Pass condition |
|---|---|---|
| V01 | New character through controller only | No mouse required; final state valid; correct safe spawn |
| V02 | Cancel, reroll, invalid name, back navigation | No partial character or overwritten slot |
| V03 | Several level-ups from one XP grant | Every pool/growth increment applied once, threshold exact |
| V04 | Historical-growth fixture | Different leveling history has the expected effect; reload preserves it |
| V05 | Failed training/equip/purchase | No partial gold, skill, inventory or point mutation |
| V06 | Melee and spell at wall/range boundary | Invalid targets/casts rejected; cost policy consistent |
| V07 | Single enemy death across repeated hit/callback | One XP grant, one loot result, one quest increment |
| V08 | Rat errand before/after acceptance and turn-in | Only eligible kills count; reward cannot repeat |
| V09 | Enter/exit all four floors repeatedly | Correct destination portal, no duplicate actor/spawn/reward |
| V10 | Death, load, quit during travel | Restore a coherent safe state, no item or boss-flag loss |
| V11 | Save failure or truncated newest save | Error shown; previous valid generation available |
| V12 | Balork defeat and reload | Boss/mark flags retain declared semantics; no duplicate unique reward |
| V13 | Missing asset/content mismatch | Useful report and safe recovery; no silent reset of progression |
| V14 | Keyboard/controller switch with menu open | Focus remains usable; prompts update correctly |
| V15 | Full fresh-character playthrough | Earn progression normally, complete intended route, resume afterward |
| V16 | Art swap | Gameplay outcomes unchanged for fixed input/RNG fixture |
| V17 | Zero mana and zero gold | Natural recovery enables a legal cast without reload; pause/load never creates extra ticks |
| V18 | Buy/equip bow and quiver | Missing quiver blocks shot; unlimited quiver is not consumed; legitimate training/purchase path works |
| V19 | Every allowed creation output | Starter melee weapon equipable; no unplayable low-stat roll |

Run boundary cases that target concrete risks, not thousands of tests mirroring every getter. Provide hand-worked expected values for numerical parity tests; never compare an implementation against a duplicate copy of itself.

## 3. World verification

Compare the graybox with the downloaded source at matching orientation. Record room IDs, portal connections, notable branches, NPC niches and boss chamber. Isometric pixels are not Unreal world coordinates; the world document specifies a reconstruction process. List intentional width/scale changes needed for modern movement.

Walk every corridor both directions with each required collision envelope. Test simultaneous enemies at doors, no reachable outside geometry, no NPC blocking required stairs, no camera occlusion at the healer, and no unreachable loot. A bat's visual altitude must not let it attack through ceilings or escape level bounds.

Publish floor-by-floor roster coverage, including explicitly provisional placements. Uncertain entries cannot disappear from the completion report. No generic skeleton/zombie population may substitute for source-supported temple enemies.

## 4. Performance gate

At Wave 0 choose and record a concrete Windows reference machine, resolution and quality preset. Proposed initial target: 60 FPS at 1080p, with CPU/GPU timing and frame-time percentiles recorded separately. This is a project target, not an already measured capability.

Record normal exploration, a crowded room, combat effects, UI open, floor transition and a save event. Set the crowd from a chosen encounter budget; do not claim “MMO-ready” from this test. Diagnose the dominant measured bottleneck before changing systems. A reference build must include quality scalability, bounded AI activation and reasonable draw/material costs.

Frame rate, load time and memory targets become binding only after the machine/content baseline is written down. Performance exceptions need a concrete finding and mitigation owner.

## 5. Deliverable for Stage 1

- Complete project source plus versioned binary assets; no missing LFS objects.
- Pinned engine/toolchain and a documented clean build/package path.
- Packaged Windows game, readme, default controls and known limitations.
- Rules baseline and all provisional deviations.
- Source/asset provenance manifest.
- A representative save fixture and upgrade/load recovery procedure.
- Native test results, measured performance notes, screenshots and a short full-loop capture.
- Agent handoff report identifying exact revision and any unexecuted verification.

No Steam publication, Xbox submission or multiplayer service deployment is part of this milestone.

## 6. Multiplayer later: preserved boundaries and actual remaining work

Already planned: single authority for gameplay mutations; input intent separate from resulting state; stable entity/content identifiers; cosmetic-only effects isolated; CharacterMovement rather than per-frame teleporting; explicit ability/attribute ownership; server-independent data definitions; persistence snapshots rather than actor pointers.

Still required later: RPC contracts and validation; actor replication/relevancy; owner-only private state; ability prediction/reconciliation; remote movement testing; dedicated-server target/package; reconnect; zone transfer authority; account authentication; database transactions; trading/duplication prevention; load testing; operations and observability.

The offline save is not a trusted online character record. A future service may reject importing offline characters, or run a deliberate migration under a defined policy. It must never accept client-supplied gold/stats as authoritative by default.

Before the multiplayer phase, audit every gameplay mutation and prove that it can execute on a server without a local player, UI, camera, renderer or local SaveGame access. Standalone authority provides a useful starting shape; it does not prove multiplayer works.

## 7. Steam and Xbox preparation

Keep controller usability and rendering scalability in the current acceptance tests. Keep platform identity separate from character ID and use Unreal's platform-aware save abstraction rather than hard-coded Windows-only paths. Platform-specific stores/achievements are later adapters.

Steam distribution and Xbox development/certification remain later work. Xbox requires approved developer access and console-specific resources. Do not create a fake Xbox configuration or claim certification from Windows controller testing.
