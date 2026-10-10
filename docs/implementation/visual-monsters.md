# W5-03 procedural monster presentation

Owner decision 2026-10-10 replaces the imported mesh/rig proposal with reproducible native geometry. This candidate contains no downloaded art, textures, binary assets, animation notifies, skeletons or new mechanics. `ALHEnemyCharacter` selects `ULHMonsterVisual` by **definition ID**, independent of the legacy presentation ID. The existing runtime validator still checks the presentation ID; catalog additions must update its allowlist separately. An unknown definition accepted with a known presentation ID logs `W5-03 UNKNOWN MONSTER <ID>: magenta placeholder`, builds a magenta cube and carries `LH.Monster.UnknownPlaceholder`. `Build` returns false for this fallback; the character retains its gameplay spec. Invalid dimensions reject before replacing the assembly.

## Derived relationships and roster

Geometry and palettes derive from [A-03 E01–E11](art/placeholders/README.md), via the baseline `AI/LHEnemyPresentation.cpp` recipe transcription. All eleven species have intentional bodies; that old renderer remains untouched for other consumers but is no longer called by the character. No reference image is copied or loaded. Documentary anatomy/reference relationships and limitations remain in the linked A-01 creature specifications; this procedural interpretation does not assert historical model authenticity.

| Definition | Recipe / source specification | Distinguishing silhouette and palette | Pieces |
|---|---|---|---:|
| Enemy.BrownRat | E01 / [Brown Rat](art/creatures/brown-rat.md) | Low brown body, back stripe, pink-brown ears, continuous tail and snout | 11 |
| Enemy.Bat | E02 / [Bat](art/creatures/bat.md) | Small brown suspended body, pointed warm tan wings | 8 |
| Enemy.DungeonBat | E03 / [Dungeon Bat](art/creatures/dungeon-bat.md) | Square gray-olive wings, pale shoulder V | 8 |
| Enemy.GreenSlime | E04 / [Green Slime](art/creatures/green-slime.md) | Grounded dark skirt and two asymmetrical green mounds | 3 |
| Enemy.GiantBat | E05 / [Giant Bat](art/creatures/giant-bat.md) | Broader brown membranes, pointed tips and pale stripes | 10 |
| Enemy.UndeadBat | E06 / [Undead Bat](art/creatures/undead-bat.md) | Broken square gray wings and bone-colored patches | 10 |
| Enemy.GiantSpider | E07 / [Giant Spider](art/creatures/giant-spider.md) | Gray abdomen, eight two-segment legs, red mouth | 20 |
| Enemy.Goblin | E08 / [Goblin](art/creatures/goblin.md) | Rust skin, bent legs, ears, dark belt and short spear | 14 |
| Enemy.GoblinWarrior | E09 / [Goblin Warrior](art/creatures/goblin-warrior.md) | Larger red body, pale shoulder yoke and broad polearm blade | 18 |
| Enemy.Atrocity | E10 / [Atrocity](art/creatures/atrocity.md) | Purple-gray shoulder hump, long arms, ochre claws and spine | 20 |
| Enemy.Balork | E11 / [Balork](art/creatures/balork.md) | Large red demon, spread marked wings, horns, centered shaft; eight ritual halo blocks | 27 |

Rest dimensions and sRGB palette reproduce A-03 centimetre recipes. The root sits at minus the existing capsule half height, grounding the assembly without scaling the capsule, changing navigation, modifying range or adding blockers. Wide wings/tails intentionally extend beyond the narrow gameplay capsule per A-03; boss routing constraints in A-01 still apply. This is not a swept-bound/doorway clearance certification.

## Kit and lifecycle

Cube geometry uses `ALHVisualPiece::Build(FLHVisualRecipe)` with one decorative box, engine material, empty collision list and default 2000-triangle/two-section budget. Each cube creates 12 triangles in one section. Spheres, cones and cylinders use engine BasicShapes mesh/materials (hard CDO mesh references expose cook dependencies), with no collision, overlap generation or navigation influence. Kit actor roots attach to the visual component, are movable, and are destroyed on rebuild/end play. Other parts are owned instance components and are destroyed on rebuild. The checksum uses the kit's explicit-field `Fingerprint()` plus primitive type and weapon role; it excludes pointers and RNG. No other visual kit file is edited. No additional kit API request is required.

The component changes child transforms only. Character collision remains owned by the existing runtime/corpse lifecycle: `MarkCorpse` still ignores Pawn and disables capsule navigation exactly as before. The animation's dead pose itself does not touch collision. Visibility can be toggled with `SetPresentationEnabled`; its tick/sampling is disabled while combat event subscriptions remain attached and never write gameplay state.

## Animation and timing

| State / event | Procedural presentation |
|---|---|
| Idle / AI idle/acquire, stationary chase | Restrained breathing; bats hover-flap; slime mound breathing leaves skirt grounded |
| Velocity with AI chase/return home (or no controller) | Rat/spider scurry, bipeds step/walk, bats flap, slime ooze; actor/capsule root never moves from animation |
| `OnAttackCommitted` | Windup starts only after authority commit, using event `ImpactSeconds` rather than a hardcoded attack timer |
| Impact deadline / resolved finish | Rat/bats snap head, spider jabs mouth, slime snaps forward lobe, goblins/Balork translate the complete weapon assembly for a compact jab, Atrocity rakes one claw fan |
| `OnAttackCancelled` or invalidated finish | Clear anticipation and strike; no delayed cosmetic callback survives cancellation |
| Health decreases above zero | Brief backward hit overlay; it does not delay/replace a pending contact pose; healing does not react |
| AI Dead / `OnDeath` via `MarkCorpse` | 0.6 s collapse to flattened inert hold, suspended bats descend to the floor; dead latch ignores later attack/hit events |

Attack commits, finishes and cancellations bind once after runtime spec installation or BeginPlay; health changes bind through GAS. AI state/velocity are read in `TG_PostUpdateWork`; runtime anticipation samples the authority commit timestamp against the world clock, so a late-frame commit does not consume a whole extra delta. Contact follows the existing impact timer, including the current catalog's 1 s impact and 2 s cooldown; the component never requests attacks. Resolved misses still swing. Zero-delay attacks skip anticipation. Post-impact hold is 0.18 s and hit overlay 0.22 s, both cosmetic and interruptible. Hit overlay adds recoil without advancing damage or changing the contact clock. Normal world/component ticks stop on world pause; there are no presentation timers or catch-up gameplay operations.

All new oscillation frequencies (3/4/12/16 radians per second), 0.3 radian wing swing, 0.5–4 cm bob/step offsets, 8–10 cm anticipation/contact offsets, slime scale variation and halo dimensions/colors are **Prototype presentation tuning**: W5-03, LH_Prototype_v1, 2026-10-10, source_url null; replace after camera/route review. Source-backed mechanics are unchanged. The simpler death collapse and shape articulation interpret the proposed authored rigs rather than claim anatomically rigged jaws/fingers or exact A-01 death-envelope fidelity.

## Validation and integration limits

`Lighthaven.Visual.Monsters.*` includes four native tests:

- `CatalogDeterminism`: every catalog ID, distinct recipe fingerprints, equal part counts and rest/animated transforms within 1e-6 attachment/rotation tolerance, plus exact procedural box vertices/indices.
- `CollisionAndFallback`: all bodies/rebuilds/animation states preserve capsule dimensions, response container, collision enabled state and nav flag; every part is nonblocking/non-navigating; unknown ID logs and marks a visible fallback through both direct build and character runtime installation.
- `StateAndCombatEvents`: all species react to actual commit/cancel/finish delegates, health changes and an AI controller's Dead state; move/idle sampling, non-default impact delay and dead latching are covered.
- `PresentationInvariantSettlement`: all species run equal-seed real GAS attacks with presentation on/off; actual impact, duplicate rejection, one death notification, health, mana, combat RNG, cooldown and corpse lifecycle match. The death observers run actual `LHRewards::SettleKill` on copied canonical snapshots; encoded XP, gold, loot RNG, corpse/respawn state and boss claims must be byte-identical, with duplicate settlement refused without changing bytes. The synthetic snapshots are not a session UI/save-file playthrough.

Execution evidence belongs to the attempt report; source tests alone are not observed results. Rendered readability, 900/1200/1800 cm camera comparisons, GPU costs, full session playthrough, navigation/door/stair animation sweeps, cook/package and Windows launch are not inferred from NullRHI tests. W5-05 must exercise those integration checks. The recipe has at most 27 parts including the boss halo; separate primitive/kit parts cost multiple draws and need real-RHI profiling before claiming a performance budget. No map or reviewed-arrival artifact is changed.

Observed in this attempt on UE 5.8.3 Linux / UID1000: final editor and game builds each reported `Result: Succeeded` (82.06 / 69.12 s). Final selected NullRHI run contained 47 tests: 44 clean successes, 3 successes with existing fixture/configuration warnings, zero failures/not-run/in-process. All four `Lighthaven.Visual.Monsters.*` tests succeeded without warnings. Exact commands, exploratory failures, JSON reports and remaining checks are recorded in the worker evidence report; this is not rendered/G5 acceptance.
