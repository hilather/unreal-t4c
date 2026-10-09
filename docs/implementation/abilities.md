# W4-03 native abilities and recovery

Native catalog interface is `Abilities/LHAbilityCatalog.h`: `LHAbilities::Catalog()` and `Find(FLHContentId)`. Stable IDs are listed below. W4-07 uses the learning requirements and distinct gold/point prices; W4-02 supplies `FLHCombatItemData` through `FLHCombatItemLookup`. No assets or Core schema changes.

| ID | Level / WIS / INT | Learn points / gold | MP | Effect |
|---|---|---|---|---|
| Attack.Melee.Basic | unrestricted Prototype | 0 / 0 Prototype | 0 Prototype | equipped melee weapon |
| Attack.Ranged.Bow | unrestricted Prototype | 0 / 0 Prototype | 0 Prototype | equipped bow + compatible quiver |
| Spell.Light | 2 / 15 / 18 | 5 / 233 | 10 | transient illumination, 600 seconds |
| Spell.FireDart | 2 / 15 / 21 | 5 / 532 | 1 | Prototype uniform integer 8–23 damage |
| Spell.HealLight | 3 / 19 / 15 | 9 / 897 | 2 | Prototype +10 HP, clamped |
| Spell.StoneShard | 4 / 20 / 17 | 6 / 1328 | 2 | Prototype uniform integer 13–21 damage |
| Spell.DustDevil | 6 / 21 / 21 | 7 / 2388 | 2 | Prototype uniform integer 8–18 damage |

Spell learning and MP cells: [Bible Spells](https://www.t4cbible.com/Spells), retrieved 2026-10-09 in R-03, live Original Content, executable version unstated. Light duration and caster target, Heal Light self/friendly targeting: [Bible descriptions](https://www.t4cbible.com/spelldesciprt), retrieved 2026-10-09. Classic measured outputs retained from [spell chart 1.20c](https://web.archive.org/web/20020202213610/http://www.t4cbible.com:80/spells.html), retrieved 2026-10-07. The output ranges are documentary measured results, not an authentic raw-damage formula. Uniform draws, neutral mitigation/power and minimum-stat interpretation are explicitly Prototype (`rules-ledger.md`, Spell magnitude arithmetic). No play parity claim. Heal magnitude is missing in R-03. Conflicting beta mana values remain in the research ledger, not selected here.

Prototype missing runtime fields: base-stat requirement evaluation, 1.5 second cast/attack cooldown; instant spell impact; spell range 1200 cm, melee catalog fallback 200 cm (physical range actually comes from item data). Physical weapon range must be resolved by the equipment catalog. No refunds after commit per D09; historical cancellation/refund behavior remains missing. Replace these authored values after fixtures and balance review. Light exposes a transient remaining-duration accessor; presentation must use it to illuminate the caster with the R-03 proposed Prototype 600 cm radius. It does not damage, heal, or populate the empty durable-effect allowlist. A renderer is an integration item, not implemented here.

`LH::Rules::MakeStage1PrototypeCombat()` supplies a typed `Stage1Ratio` variant, preserving the legacy linear default and aggregate field order. Physical chance is clamp(Attack/(Attack+Dodge), .10, .95), both zero => .50. Inclusive integer weapon draw + floor(STR/5) for melee or floor((STR+AGI)/10) for bows + quiver bonus minus armor, minimum 1. All are the ledger's Prototype Physical resolution row, nearest evidence [skills](https://www.t4cbible.com/skills), retrieved 2026-10-07; numeric formulas are missing, not sourced mechanics. No ratio approximation with linear coefficients. Spell output uses inclusive integer bounds without a second attribute bonus.

## Authority and integration

`BuildAttackConfig(snapshot, ability, itemLookup, combatParameters, out, error)` resolves equipped MainHand and compatible Quiver inventory instances. Item data carries damage bounds, range, compatible IDs and bonus; Wooden Arrows use `Unlimited` (no inventory write per shot). Fixtures should use Rusted Dirk 1–4, Ashwood Flatbow 1–3 and Wooden Arrows +1 as recorded in the ledgers. The builder rejects missing equipment and unresolved physical coefficients. Learned Attack/Archery are source rule inputs; the target's GAS Avoidance must be populated from learned Dodge by the profile/integrator. Source STR/AGI come from snapshot base attributes under the declared Prototype basis.

`ExecuteUseAbility(FLHUseAbilityContext&, request)` accepts source/target combat components, stable entity IDs, a synchronized snapshot copy, item lookup, physical coefficients and authoritative friendship. It validates knowledge, equipment, life state, mana, separate cooldown, range/LOS and targeting before the existing GAS activation commits. Hostile actions cannot target self/friends, Light targets caster, Heal Light targets self/friends. UI must never supply friendship or coefficients. No receipts, no snapshot resource write: W4-06 synchronizes live GAS at settlement and owns BeginRequest/CommitRequest. No Consumable ability IDs. Existing ConfigureAttack/RequestBasicAttack/ValidateAttack/OnDeath consumers remain supported.

`FLHUseItemContext` contains snapshot copy, item lookup, stable owner, alive state and derived MaximumMana. `ExecuteUseItem` accepts an owned instance, rejects nonconsumables with NotUsable, rejects full mana with NoEffect, clamps and consumes one only on acceptance. Empty target means self; explicit target must match owner. Equipped consumables reject; zero quantity and invalid pools reject. [Potion effects](https://www.t4cbible.com/Potions), retrieved 2026-10-09: Potion of Mana restores 25 MP. Full/clamp edge semantics are Prototype policy. W4-06 syncs GAS first, validates/imports the mutated copy, persists receipt and then publishes Mana to GAS.

D14 component delegates carry value-captured activation/impact identity, stable IDs, ability ID, life generations, commit time and impact seconds. Committed follows canonical cost/cooldown mutation; cancelled includes Explicit/SourceDeath/AvatarCleared/Travel; finished includes ResolvedHit/ResolvedMiss/ImpactInvalidated/Cancelled. Native cost is the single cost owner; no duplicate GAS cost effects. Committed cancellation keeps cost/cooldown. Finish resets pending state before publication. Commit publication guards configuration/replacement and defers finish until the committed event has published. Basic ability serial guards protect replacement timers.

## Recovery and boundaries

`AdvanceManaRegen(snapshot, aliveUnpausedSeconds, parameters)` uses `Rules::RegenerateMana` with persisted fractional seconds. Snapshot maximum is EarnedBaseMana; modifiers require a derived-max adapter in W4-06. Zero delta on menu/pause/load. Dead snapshots do not tick. Prototype natural recovery = 1 MP per 5 alive unpaused seconds (rate missing in R-03). `ConfigureManaRegen` enables the live component tick; `SetRecoveryMenuPaused` gates menus; `ManaRegenFractionalSeconds` is imported/exported by W4-06. World pause contributes no tick, offline time is never measured.

`CaptureCooldowns(component, owner, array)` replaces only that owner's records. `RestoreCooldowns` rejects duplicates/unknown catalog IDs/invalid remainders before mutating the component. Per-ability remaining seconds use world active simulation time and are restored relative to the new world clock, never wall time. Travel must cancel with Travel, capture at a completed boundary, and restore; initialization does not clear cooldowns. Off-floor enemies keep canonical records, not live ticking actors. No saved-shape changes proposed.

## Remaining integration checks

W4-06 must wire session commands, stable IDs, friendship, derived max values, target Dodge, menu gating, carry/cooldown import/export, Light presentation and completed-action save boundaries. Physical skills and equipment require real W4-02 catalogs. End-to-end floor travel and zero-gold recovery in the playable session are not claimed by isolated domain tests. All missing historical fields above remain Prototype. Linux build/test evidence and any baseline map-pointer failures are recorded in the worker report; Windows checks remain deferred.

The runtime owner must include `IsPublishingActionEvents()` as well as pending actions in completed-boundary checks. This is true through commit, impact, death and finish notification stacks, even if a listener cancelled and cleared pending state. Menus must pause world simulation for cooldown/Light clocks; the menu recovery flag independently suppresses mana ticks.

Spell configurations supply their own explicitly Prototype neutral magnitude parameters, independently of unresolved physical coefficients. Equipped attacks require caller-resolved physical parameters. The builder enables learned-skill inputs for player attacks; legacy/AI `ConfigureAttack` callers retain GAS Accuracy/DamageBonus unless they opt in to `bUseLearnedSkills`.

W4-06 must also include the combat model variant and native catalog mechanical fields/policies in its mechanical/catalog closure hash. The existing Framework closure is read-only to this task; changing a formula variant must not reuse a previous profile hash. No G4 gate or integrated gameplay success is claimed here.
