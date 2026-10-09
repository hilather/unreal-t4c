# Wave 4 briefs: complete the basement gameplay loop (drafts, not launched)

Drafted 2026-10-09 against main 1977d3d (Waves 0–3 at 1109cf2); updated the same day for Matt's answers (below; main 11de294). Hold until G3 passes or Shepherd says go, and until R-03 (Bible lookup → `docs/implementation/research/w4-bible-lookup.md` + ledgers) is merged. Full brief per task = `brief-W4-0N.md` + `_env.md`; launch specs in `LAUNCH.md`.

| Task | Owner area (exclusive paths) | Depends on | Group | Est. size |
|---|---|---|---|---|
| W4-08 | Schema rev 2: `Core/` + listed codec files in `Persistence/` (UseItem request, reason codes, rev1→rev2 migration, `schema-rev2.md`); one-line stubs in `LHWave2Session.h`, `LHCharacterAuthority.cpp`, UI test double | G3, R-03 | 0 (alone) | M |
| W4-01 | AI: `AI/`, `LHEnemyCharacter.*`, `Lighthaven.Build.cs` deps | W4-08, G1 | A | L |
| W4-03 | Abilities: `Abilities/`, `Rules/` (melee, bow+quiver, 5 spells, UseItem domain effect (mana potion), regen, cooldowns, D14 events) | W4-08, W2-02 | A | L |
| W4-04 | Rewards/persistence: `Rewards/`, `Persistence/` (kill/loot/respawn/death transactions, digests, receipts, reward IDs); starts from main after W4-08 | W4-08, W2-01 | A | L |
| W4-02 | Data: `Data/` (11 enemies, 77 encounter rows, items, roster matrix) | W4-01 interface, W4-04 + W4-03 types | B | M |
| W4-05 | Quests/UI: `Quests/`, `UI/`, `Input/`, `LHPlayerController.*` (in-slice quests only; bat-wing quest deferred; UseItem input) | W4-01, W4-03, W4-04 | B | L |
| W4-07 | Services: `Services/`, `Character/` (train/learn/buy/sell, offer catalog, B1 NPC placement instructions) | W4-03, W4-04 | B | M |
| W4-06 | Integration: rest of `Framework/`, `World/`, three map generators, `Validation/`, `Config/` ini files, Integration tests. **Split into W4-06a (Stage 1A) and W4-06b (G4).** | all of the above | C1, C2 | XL (2 × L) |

Ownership notes: `Lighthaven.Build.cs` and `LighthavenTests.Build.cs` (coordinator-owned module definitions in the plan) are delegated to W4-01 for dependency lines only (AIModule, NavigationSystem). `Character/` goes to W4-07 and `Rules/` to W4-03. Every write list in `LAUNCH.md` is disjoint; Framework files are enumerated per owner.

Interface rule used throughout: domain owners write pure transaction functions on a snapshot copy (`ELHCommandReason`), and W4-06 alone wires them into `FLHWave2Session` with one receipt wrapper (W4-04 helpers). UseItem (schema rev 2) is a persisted command like Buy; UseAbility is runtime-only until settlement (D16): no save per swing, and W4-06 narrows `IsBlocked()` so a save in flight never freezes movement. Balork respawns after 15:00 like other lives; its completion is an Interact topic at Brother Kiran (travel never grants progress) and the completion reward is single-claim. UI talks only to `ILHUIReadOwner`/`ILHUISessionOwner` extensions; AI talks to rewards through director bindings set by W4-06.

## Decisions needed from Matt (or the coordinator)
1. **Stage 1 starting profile.** Adopt the rules-ledger "Separate prototype proposals" (HP 30, MP 10, gold 100, Attack 10/Dodge 10, Rusted Dirk + cloth + 3 torches, growth formula, ratio hit formula) plus the Bible XP table as the playable Prototype profile? The current profile is a synthetic test (2 slots, 0 gold, test bow, 4 levels). W4-06 has a placeholder for this answer.
2. **Enemy numbers.** Approve the ledger "Enemy runtime interpretation" Prototype row (first printed XP column, uniform melee damage, one attack per 2 s, midpoint gold, 10 % single drop, missing gold → 5) as Stage 1 tuning, plus Prototype AI radii/speeds/pursuer cap chosen by W4-02.
3. **Roster rows still speculative.** Dungeon Bat floor is null in the source (B2 provisional, 2 slots); Undead Bat B2 (S3) vs B2+B3 (S9), B2-only selected. No roster-parity claim is possible while these stay open.
4. **Balork.** Permanent defeat (the Bible's live 15:00 respawn is not used); arena anchor still needs the M4 visual check; gold blank and drop odds missing (Flowing Black Robe "dr", Light Heal); no phases or summons; completion text must not claim the historical quest chain; the A-01 capsule must pass the 500×360 boss door. Confirm or adjust.
5. **Potion use command.** Map mana-potion use to `UseAbility` with a new `Consumable.*` ability ID prefix (registry/doc decision, no Core change), or add a `UseItem` command in schema rev 2? Recommended: the prefix.
6. **Vendor stock.** Unlimited and not persisted (no schema change) for the slice; a persisted stock/buyback model would be schema rev 2 (D17).
7. **Unsourced rules needing a Prototype value:** Nevanis healing (amount/cost/restrictions null; proposed free full heal), Heal Light magnitude and the Light effect (missing), training price units (Prototype 10/10/15 g per rank, 1 point per rank, cap 100), respawn safety distance, loot cleanup time, HP/MP after death respawn, interaction distances.
8. **Bat-wing quest** (Undead Bat wings → Uranos → Skull Dagger + 2500 XP): drop chance null. Defer, or author a Prototype chance?
9. **Save compatibility.** The W4 catalog changes `CatalogHash`, so G3-era saves are rejected (same precedent as W3-05). Accept?
10. **Plan deviations to acknowledge:** AI is a native C++ state machine, not Behavior Tree/Blackboard assets; `Data/Enemies|Encounters` are native catalogs, not DataAssets (workers can't author binaries; matches existing registry/profile practice); W4-06 is split into 06a/06b.

## Answers (Matt, 2026-10-09; recorded in `status-ledger.md`)
1–4. **Use the T4C Bible's numbers** (adjustable later) for the Stage 1 starting profile, enemy numbers, the speculative roster rows and Balork. R-03 looks every value up and writes `docs/implementation/research/w4-bible-lookup.md` (value → Bible value or `missing` → source → proposed Prototype) and updates `rules-ledger.md`/`world-ledger.md`/`ruleset-bible-v1.md`. Wave 4 starts after R-03 merges; every brief that sets numbers takes them from that table and the ledgers, with a labelled Prototype only where the table says `missing`. W4-06's starting profile = the Bible's starting character (ledger "Separate prototype proposals" only where missing) with the Bible XP table.
4 (Balork). **Respawns after 15 minutes** (the Bible's live 15:00), not a permanent defeat. The boss-completion reward/flag stays **single-claim** across reload, respawn and floor travel (G4). Briefs W4-01/02/04/05/06 updated; `prototype-policy.json` `boss_respawn: false` is superseded.
5. **A real UseItem mechanic** (no `Consumable.*` prefix) → new command + contract change → **schema rev 2**, new task **W4-08** (group 0, alone, before group A). W4-03 owns the item-use domain effect, W4-06 wires it into the session, W4-05 sends it from inventory/hotbar.
6. Vendor stock: unlimited and not persisted (as drafted).
7. Unsourced values: workers may set labelled Prototype values **only after checking `w4-bible-lookup.md` and the ledgers** (now in `_env.md` and the affected briefs).
8. Bat-wing quest: **deferred**. It needs a future quest mechanic where *completing the quest* grants the item. W4-05 implements only the in-slice quests (Samaritan rats, Balork return, healer/services topics).
9. G3-era saves becoming unloadable after the W4 catalog change: **accepted**.
10. Plan deviations **accepted**: native C++ AI state machine, native catalogs instead of DataAssets, W4-06 split into 06a/06b.

Open after the answers: Undead Bat/Dungeon Bat floors and any other value R-03 marks `missing` stay labelled Prototype/provisional, so no roster-parity claim yet; W4-08's `schema-rev2.md` is a proposal the coordinator must accept (including its D09 note that a boss may use `OrdinaryRepeat`) before Group A starts.

## Plan items not mapped to existing code
- "Enemy base Blueprint, shared behavior tree/blackboard" and `Content/Lighthaven/Data/*` assets: none exist and none are authored. Native equivalents are specified instead.
- `ULHDefinitionRegistry` (architecture §3) and a typed D13 presentation resolver don't exist. Native catalogs resolve IDs; enemies use code-drawn A-03 placeholders keyed by PresentationId; the real resolver is deferred to Wave 5.
- `Docs/Implementation/asset-locks.md` doesn't exist. Map ownership stays "generated by commandlets, committed by the coordinator".
- "Exclusive scheduled map edits" (W4-06) becomes generator edits plus host regeneration. The only known generator change is converting the B1 Nevanis/Shovanis TargetPoints into GUID-bearing `ALHInteractableMarker`s.
- `01-game-design.md` puts a "first trainer" in Stage 1A, but the waves plan's 1A list doesn't. The briefs follow the waves list; a trainer in 1A needs W4-07 + W4-06b.
- Carried forward, not Wave 4 work: gamepad untested (G1 decision), Windows deferred, launch render stall (known limitation).
