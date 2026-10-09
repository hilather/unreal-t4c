# Native contracts — schema revision 1 frozen (2026-10-08)

The integrator accepted D01–D17 in [schema-rev1-freeze.md](schema-rev1-freeze.md). S-02 applies the six accepted Core changes below. The native layout is marked rev 1 frozen as directed; the coordinator must still confirm the full host Automation suite before recording G0's schema requirement as complete. This document makes no gate, package or play claim. Linux first; Windows checks remain deferred.

1. `FLHRulesetRef.HashAlgorithm`: FName, default None; published rulesets require SHA256.
2. `FLHRequestId.Epoch` and `FLHSessionRecord.RequestEpoch`: invalid GUID defaults. Authority requires epoch match and positive receipt sequence; retention follows D06.
3. `FLHCreateCharacterRequest.PreviewToken`: invalid GUID default. `ELHLootTransferKind` is Unspecified/Item/Gold; loot Kind defaults Unspecified. A pure Core payload helper rejects unspecified kind, invalid item/gold sentinel combinations and unresolved/nonpositive quantity. Authority checks and settlement remain future work.
4. `ULHAbilityDefinition.ExecutionClass`: soft UGameplayAbility class; `ImpactSeconds`: unresolved FLHNumber by default.
5. `ULHDefinition.PresentationId`: default None; Visual retained as pre-freeze fallback. `ULHPresentationDefinition` derives from UPrimaryDataAsset, stores Id and soft UObject Visual, and fixes primary type to Presentation.
6. Cooldown/effect records gain invalid entity Owner; save header gains PayloadCodec default None (required codec LHCanonicalBinary1). No animation/recovery state is serialized.

Core header banners now identify the frozen revision. Codec/hash vectors, limits, RNG adapter/history, epoch/token/reward authority, owner-scoped restore and presentation delegates belong to Wave 2/4. Declarations and payload validation do not implement those behaviours. Illustrative JSON is not a loadable save or exact wire format. The accepted decision document governs the semantic invariants; the original W0-03 rationale below is retained as historical context where it describes unresolved implementation work.

## Files and ownership

`Source/Lighthaven/Core/LHIdentity.h`, `LHValues.h`, `LHDefinitions.h`, `LHSaveSnapshot.h`, and `LHCommands.h` are the canonical frozen native layout. Subsequent shared header changes require integrator review. Public module dependencies cover these declarations and proposed GAS/input seams; editor-only DataValidation dependencies live in LighthavenEditor. LighthavenTests is the editor Automation module, including the S-02 Core schema fixtures. The game target loads only Lighthaven. No server target exists.

The `.uproject` has EngineAssociation `5.8`, runtime/editor/editor-test modules, GameplayAbilities and EnhancedInput enabled, and DataValidation limited to Editor targets. JSON forbids literal comments: the first `_DraftComment` member carries the required draft warning while preserving JSON syntax. Unknown-member acceptance and plugin descriptor compatibility need actual UE validation. Build/IncludeOrder settings use `Latest` from the pinned installation; replace with explicit supported enumerators during integrator build validation. No version upgrade is authorized. No Config, map, binary asset, ledger or toolchain file is changed.

## Identity and value types

| Type | Responsibility and consumer | Required invariant |
|---|---|---|
| FLHCharacterId | Local profile identity; W2-01/02 | Valid GUID assigned at creation, independent of name/platform/account. |
| FLHContentId | Registry key; W1-02, W2-02, W4-02/03 | Nonempty case-canonical namespaced FName, independent of asset path. Existing `LH_Prototype_v1` maps via an integrator-approved alias to proposed `Ruleset.LH_Prototype_v1`. No silent renaming. |
| FLHAreaId | Area definition key; W3 map owners/W3-04 | Proposed `Area.LighthavenTempleDistrict`, `Area.TempleB1` through `Area.TempleB4`; map filenames are presentation/location references. |
| FLHEntranceId | Area-qualified arrival key; W3-04 | Nonempty LocalId within Area; `Temple.SafeSpawn` remains proposal from example. Proposed B1..B4 local Entry/Descent names need freeze after trace. |
| FLHEntityId | Stable instance key; W2-02/W3/W4-01/04 | Run GUID + area + instance GUID, persisted across unload/load; never array offset, actor label or pointer. Newly duplicated authored objects get fresh instance IDs. |
| FLHSpawnLifeId | Encounter life key; W4-01/04 | Area + authored SpawnSlot GUID + nonnegative generation. Zero means initial life. Increment exactly once on permitted respawn; never on reload. |
| FLHRewardId | Claim key; W2-02/W4-04/05 | Mint once per source life/objective and reward purpose, persist before publishing reward. Reapplying the same ID is harmless. A shared boss death/dialogue unique purpose resolves to the same claim. GUID creation alone does not establish this invariant; reward owner enforces mapping. |
| FLHRequestId | Local intent idempotency key; W2-02/03, W4-07 | Fresh GUID plus current authority-issued Epoch for a new intent, retained for retries. Identical ID/payload returns original result; differing payload rejects. |
| ELHProvenanceStatus | Research status; W1-02/W4 data/editor validators | Missing default; Confirmed means documented only for named baseline. VerifiedT4C reserved for genuinely verified baseline evidence, never assigned automatically to ledger rows. Disputed/Modernized/Prototype preserved distinctly. |
| FLHFieldProvenance | Field-level evidence; W0-03/W1-02/W4 | FieldPath, source URL/baseline/date, status, recorded value and notes. Empty URL permits authored tuning; notes identify choice. ISO retrieval date is inherited, not a fresh fetch. Contradictory entries may coexist for one path. |
| ELHValueResolution | Presence, separate from provenance; all consumers | Unresolved never interpreted as zero; a known zero must explicitly be Resolved with provenance. |
| FLHInteger / FLHNumber | Nullable integer/real mechanical value; W1-02/W2/W4 | Resolution gates access to Value. Stored zero under Unresolved has no semantic meaning. Finite real values, units, bounds and provenance validated before use. `ValueAsRecorded` is evidence text, not another authoritative numeric store. |
| FLHAttributeBlock | Named five-attribute DTO; W1-02/W2-02 | Strength/Endurance/Agility/Intelligence/Wisdom only. AGI/Dexterity aliases map to Agility. No arrays, no permanent class lock, no temporary modifiers. Allocation requests use the same shape but each delta must be explicitly resolved, including zero. |
| ELHMigrationPolicy / FLHRulesetRef | Rules/content compatibility; W1-02/W2-01 | Nonzero published revision, stable ID and content hash; Reject default, RequireExplicitMigration or NewCharacterRequired alternatives. Formula-affecting changes cannot silently recalculate history. |
| FLHRngState | Required gameplay RNG stream state/roll input; W1-02/W2-01/W4-04 | Stream ID, algorithm/revision and bounded opaque state bytes. No cosmetic stream contamination; empty/unknown algorithm invalid when RNG is required. |
| FLHGrowthAward | Historical level entitlement; W1-02/W2-02 | One award ID per new level transition, actual growth inputs, HP/MP increments, points, ruleset and relevant RNG input. No duplicate award during XP debt recovery or replay; never derive old growth from current attributes. |

FName is case-insensitive: validators must enforce one canonical spelling and reject alias collisions. Identity defaults are invalid sentinels, not guessed mechanics. Loader validation must reject duplicates in array-backed keyed collections; no reflected nested maps or pointer keys are required.

## Definition shells

| Type | Responsibility and consumer | Required invariant |
|---|---|---|
| ELHEquipmentSlot | Equipment vocabulary; W2-02/W4-03 | Quiver first-class; Unspecified invalid for equip. Other proposed slots need integrator agreement. No per-shot depletion for Wooden Arrows. |
| FLHMechanicalField | Named unresolved numeric parameter; W1-02/W4 data | Key has frozen meaning/unit and one resolved value; not executable formula text. Shell parameters expanded to concrete fields where rules implementation needs stronger types. |
| FLHPolicyField | Named enum-like authored policy; W1-02/W4 | Explicit resolution and provenance; empty/unresolved never silently chooses default behavior. |
| FLHEligibility | Definition requirements; W1-02/W2-02/W4-07 | Named attribute/level minima, skill requirements/minima and spell prerequisites. Missing required data rejects execution; explicit policies distinguish no requirement and unresolved requirement, base/modified evaluation and quiver needs. |
| FLHLootEntry | Authored drop candidate; W4-02/04 | Stable item, resolved quantities/weight, provenance. Definition candidates are not rolled loot; validation must establish distribution semantics. |
| FLHEntranceDefinition | Safe landing shell; W3-04 | Area-qualified ID, explicit transform resolution, provenance. Untraced identity transform is not an approved landing. |
| FLHPortalDefinition | Travel endpoints; W3-04 | Stable instance, resolvable source/destination entrances; reverse edge authored explicitly. |
| ULHDefinition | Common UPrimaryDataAsset boundary; registry/editor/W4 | Explicit stored Id and fixed primary type; `GetPrimaryAssetId` never uses filename. Soft Visual reference only; no mutable player/world state. Registry validates prefix/type and global uniqueness. |
| ULHRulesetDefinition | Creation/formula/rounding/XP policy shell; W1-02 | Id equals Reference.Id, hash/revision frozen, parameters/policies and XP thresholds unresolved until sourced or reviewed prototype. |
| ULHItemDefinition | Item eligibility/stack/slot/modifier/economy shell; W2-02/W4-07 | Stack, buy and sell prices distinguished; permanent modifiers separate from temporary effects. Category is a proposed tag. |
| ULHAbilityDefinition | Unified Spell/Attack shell; W1-03/W4-03/07 | Learning gold/skill costs distinct from cast mana. Range/cooldown/effect/cancellation/refund/target policies explicit. ExecutionClass is a soft generic class shell; integrator narrows it to UGameplayAbility before implementation. Primary type Ability; Spell and Attack IDs remain separate namespaced keys. |
| ULHEnemyDefinition | Independent researched variant; W4-02 | Eleven roster identities remain independent. HP/rewards/AI parameters unresolved, loot has explicit resolution so empty cannot mean missing. No inferred stat parity from shared visuals. |
| ULHEncounterDefinition | Authored slot/placement/respawn shell; W3/W4-01/02 | Slot unique within area, enemy resolves; transform unresolved until authored. Boss/elite booleans are authoring flags, not numeric evidence. Ordinary/boss clock/reset policies are explicit policy entries. |
| ULHAreaDefinition | Map/entrance/portal/fallback shell; W3-04 | Soft UWorld map resolves and cooks; fallback references a valid resolved entrance. No fifth temple floor. Registry maps its Id to FLHAreaId.Content. |

Visual and execution references are soft; packaged-cook inclusion remains an Asset Manager configuration and validation task outside this ownership. Proposed class flags allow data asset authoring, but no assets are created.

## Save schema v1

| Type | Responsibility and consumer | Required invariant |
|---|---|---|
| FLHQuestionAnswer | Creation evidence pair; W2-02 | Stable question/answer definition IDs, legal pair validated by generation policy. |
| FLHCreationRecord | Accepted creation inputs; W2-02 | Preserve accepted named attributes, answers, generation policy/revision and RNG inputs. Four-question flow or explicitly labelled prototype allocation, never conflated. |
| FLHItemInstance | Permanent inventory/rolled item data; W2-02/W4-04 | Stable instance/definition, resolved quantity and permanent rolled values only. |
| FLHEquipmentBinding | Slot → owned item; W2-02 | Unique slots/instances, referenced item in inventory, omitted binding means empty slot; no duplicated equipped totals. |
| FLHLearnedSkill | Skill training state; W2-02/W4-07 | Unique skill definition key and resolved trained value; learn/train costs not baked into state. |
| FLHCheckpoint | Safe recovery/respawn location; W2-01/W3-04 | Resolved valid entrance and transform. No unsafe origin fallback. |
| FLHCharacterRecord | Canonical progress; W2-01/02 | Display/appearance/creation, base stats, earned level, XP balance/debt, points, earned base HP/MP, current resources, skill/spell knowledge, inventory/equipment, gold, history and active entrance (includes area). No GAS aggregates. Rebirth zero is a Stage 1 reserved structural policy. |
| ELHEncounterLifeState / FLHEncounterRecord | One spawn life's lifecycle; W4-01/04 | Unresolved invalid for playable saves; life/health/remaining active-time respawn/kill claim committed coherently. Dead lives never become alive merely on unload. |
| FLHCorpseLootRecord | Finalized death roll and remaining transfer; W4-04 | Container/life/reward link; finalized contents and gold never rerolled. Claimed agrees with empty transferable remainder; full inventory rejects without changing either side. Cleanup timer may remain unresolved until policy is selected. |
| FLHObjectRecord | Persistent doors/chests/pickups; W3/W4-04 | Stable instance/definition; distinct opened/collected/unlocked facts; contents persisted. |
| FLHQuestRecord | Quest stages/counters/claims; W4-05 | Accepted/completed/rewarded independently tracked. Rat count is not corpse count. Turn-in uses stable claim, flags preserve future branch semantics. |
| FLHBossRecord | Defeat/mark/dialogue/claim facts; W4-05 | Permanent defeat independent of marks and quest completion; no implicit historical Balork chain. |
| FLHAreaRecord | Per-area mutable snapshot; W2-01/W4-04 | Unique area; encounter/object/corpse identities validated. |
| FLHWorldRecord | Campaign world and unique claims; W2-01/W4-04/05 | Run ID scopes entities; unique reward set, quests/bosses/portal unlocks distinct. Whole character+world commit, never XP without corresponding lifecycle/loot. |
| FLHCooldownRecord | Remaining gameplay cooldown; W2-01/W4-03 | Persist remaining duration, no load-based reset or wall-clock accrual. |
| FLHDurableEffectRecord | Canonical approved effect inputs; W2-01/W4-03 | Only allowed durable effects; restore definition/inputs then rebuild modifiers. No serialized GAS handles/specs/aggregates. |
| ELHEffectSavePolicy / FLHSessionRecord | RNG/timers/effects/checkpoint/request retention/diagnostics; W2-01 | Explicit completed-action-boundary policy proposed; no active cast saved. Fractional mana timer persists; diagnostics bounded and nonauthoritative. |
| FLHRequestReceipt | Committed intent replay evidence; W2-01/W4-07 | ID + canonical payload digest + committed sequence. Original accepted outcome recoverable; retention must not permit replay to remutate. Not an event-sourced database. |
| FLHSaveHeader | Integrity/compatibility envelope; W2-01 | Magic/schema/sequence/build/rules/content/character/length/checksum. Payload length/checksum computed by serializer. Structural zero sequence/length is not a mechanic. Reject malformed/future/incompatible envelope before large allocation. |
| FLHSaveSnapshot | One immutable coherent DTO; W2-01/04 | Character/world/session from same completed transaction. No actor pointers, no UI/animation graphs, no USaveGame implementation supplied. |

Proposed XP convention: ExperienceBalance is nonnegative retained progress; ExperienceDebt is a separate nonnegative deficit. New XP pays debt first, then progresses balance; earned level never decreases and debt recovery does not award points. Exact threshold/penalty conversion awaits rules owner and integrator. This avoids encoding the same deficit twice using both a negative balance and positive debt.

Architecture envelope fields extend the incomplete planning example. JSON `null` ↔ Unresolved, resolved zero ↔ explicit Resolved(0). World active map/entrance normalize to Character.ActiveEntrance with area registry resolving map; fractional mana/RNG normalize to Session. No conflicting copies are maintained. The accepted D04 specifies canonical codec bytes; implementation remains W2-01.

W2-01 must implement bounded parsing/counts/bytes, checksum canonicalization, schema migrations, A/B generations, newest-valid fallback, one in-flight save and visible failures. Proposed saves occur only after completed commands/actions; cooldowns and approved effects persist. Restore definitions/migrate → initialize base/earned state → inventory/equipment → durable effects → clamp resources → enable simulation. Travel saves source checkpoint before loading, validates destination then saves arrival; crash/failure restores source. Death settles once and commits checkpoint before respawn. None of these behaviors have been implemented or tested here.

Reward retention: ordinary kill claim lives in encounter/corpse records; permanent claims live in ClaimedUniqueRewards. Compaction may retire old lives only when all replay/loot paths can no longer reference them. Bound request/growth/reward arrays without dropping evidence that would permit duplicate awards. Concrete limits and high-water strategy remain integration decisions.

## Command seam

`ILHCommandHandler` provides eleven typed `Execute` overloads and one `FLHCommandResult`. It is a native interface bound to local owner context, not a UObject bus or transport. Results carry request ID, accepted/rejected disposition, reason, committed sequence and replay flag. `ELHCommandDisposition` defaults rejected; `ELHCommandReason` supplies stable UI-facing codes. Accepted requires None; rejected must provide a non-None reason and apply no mutation. All validate/commit/publish happens on the game thread in the later authority implementation. Caller never supplies an acting character ID.

| Request type | Intent and consumer | Validation boundary |
|---|---|---|
| FLHCreateCharacterRequest | Name/appearance/accepted creation; W2-02/03 | Validate roll authority/token policy, no trusted caller stat injection; same confirmation produces one character/slot. |
| FLHAllocateAttributePointsRequest | Named deltas; W2-02/03 | Resolved nonnegative integers, affordable total, permanent allocation committed once. |
| FLHTrainSkillRequest | Trainer, skill, requested points; W4-07 | Actual trainer/proximity/offer, prerequisites, pool and gold; no partial charge. |
| FLHLearnSpellRequest | Trainer and spell; W4-07 | Actual source-positioned teacher, legal costs and prerequisites; cast cost distinct. |
| FLHBuyItemRequest | Vendor, offer, quantity; W4-07 | Offer prices come from authority, capacity and funds checked together. |
| FLHSellItemRequest | Vendor, owned instance, quantity; W4-07 | Ownership, sell policy, quantities and equipment constraints. |
| FLHEquipItemRequest | Owned instance/slot and unequip flag; W2-02 | Legal slot/prerequisites, quiver rules, atomic swap; unequip uses same seam. |
| FLHUseAbilityRequest | Definition and stable target; W1-03/W4-03 | Life state, knowledge/equipment, resource/cooldown/range/LOS, cancellation/impact policy. Activation+ImpactIndex dedup belongs to later ability layer. |
| FLHInteractRequest | Stable object and topic; W3/W4-05 | Valid target, proximity, topic/quest state, no direct UI rewards. |
| FLHTakeLootRequest | Container, item and quantity; W4-04 | Source contents, ownership/capacity, intact rejection; explicit Item/Gold kind and positive resolved quantity; Gold requires the default item sentinel. |
| FLHRequestTravelRequest | Portal and expected destination; W3-04 | Resolve authoritative portal edge, validate destination and life/action state, source/arrival checkpoint durability. Never travel rewards. |

Conceptual usage example: submit `FLHLearnSpellRequest` with fresh Request GUID and current Epoch, Trainer's stable entity ID and `Spell.FireDart` to the owner-bound handler. Handle rejected `InsufficientGold` with no deductions; retry an identical payload with the same ID after an uncertain response. A new purchase/learning intent uses a new ID. No request lets the UI choose reward quantity, vendor price or acting owner.

## Proposed shared gameplay tags

No Config changes or registered tags yet. Integrator freezes spelling and ownership before use.

| Namespace/examples | Consumer/purpose |
|---|---|
| `LH.State.Alive`, `.Dead`, `.Casting`, `.Attacking`, `.Travelling` | W1-03/W4-03 authoritative lifecycle/action gates |
| `LH.Ability.Attack.Melee`, `.Ranged`, `LH.Ability.Spell`, `LH.Ability.Consumable` | W1-03/W4-03 ability classification |
| `LH.Cooldown.Attack`, `LH.Cooldown.Spell` | W4-03 policy-defined cooldown groups |
| `LH.Equipment.Weapon.Melee`, `.Bow`, `LH.Equipment.Quiver.Unlimited` | W2-02/W4-03 equipment eligibility |
| `LH.Item.Weapon`, `.Quiver`, `.Armor`, `.Consumable.Mana`, `.Quest` | W2-02/W4-07 item categories |
| `LH.Effect.Durable`, `.Transient`, `LH.Damage.Physical`, `.Mental` | W4-03 policy classification; not guessed resistance formulas |
| `LH.Interaction.Trainer`, `.Vendor`, `.Healer`, `.Loot`, `.Portal` | W3/W4 services/interaction gates |

Tags classify runtime facts; they do not replace stable content IDs or persistent boss/quest flags.

## Unresolved ledger rows and representation

| Ledger rows still unresolved/disputed | Representation and next owner |
|---|---|
| Rules: exact historical patch, Creation RNG/question weights, starting level/HP/MP/pools/gold/skills/spells and Starter identity/kit | Missing provenance; unresolved rules parameters/creation revision and character numbers; no starting defaults. Generator policy and initial grants W1-02/W2-02. Existing server-specific kit is not imported. |
| Rules: growth formula/timing, retroactive growth conflict, XP curve, Derived stats | Unresolved parameters/thresholds and policy fields; FLHGrowthAward stores actual inputs/increments/rules/RNG. W1-02 fixtures and W2-02 progression. |
| Rules: hit/damage/armor/resistance/speed, spell magnitude/range/cooldown/cancellation/refund, requirement base/modified semantics | Ability effects/range/cooldown and eligibility policies unresolved; W1-02/W4-03. No damage formula or refunds selected. |
| Rules: Death losses/denominators, XP debt accounting, checkpoint rules | Policy fields and explicit debt/earned level/checkpoint. No-penalty development policy may be declared Prototype later; W2-02/W4-05. Proposed debt normalization requires freeze. |
| Rules: training gold/increments/scaling/initial skills, healing restrictions | Shell policies/numbers; service offers not yet modeled as assets, W4-07 extends through integrator. Candidate learning/casting prices remain distinct. |
| Rules: mana natural rate, potion edge semantics; world respawn clock/safety distance/cleanup/chest timers | Unresolved numeric/policy fields and saved remaining/fractional timers; known existing 1MP/5s, 120s respawn and potion edges require explicit Prototype provenance if authored, never defaults. W4-03/04. |
| World: Dungeon Bat floor missing, Undead Bat B2/B3 disputed, all species stats/rewards/drops | Encounter field provenance preserves placement status; Enemy health/rewards/loot/AI fields unresolved. Select B2 Undead Bat profile without source union. W4-02. |
| World: image-space traces, exact graph/door widths/transforms/service anchors/mage route, safe spawn and portal direction | Placement/entrance transform resolution defaults Unresolved; field provenance retains needs-visual-check in Notes with Missing status until review. IDs/edges proposed only; W0-04/W3 owners/W3-04 validate. |
| World: bat-wing drop/count/recovery, ordinary loot cleanup, live-enemy reload policy | Loot weights/counts/timers unresolved, finalized corpse contents stored, Encounter policies unresolved. W4-02/04/05. |
| World: Balork mark/chain/recurrence/rewards | Boss marks/dialogue/defeat/claims separate; policies and rewards unresolved. Permanent defeat/return objective are authored Prototype/Modernized choices, not historical completion. W4-05. |

No new gameplay tuning values are introduced. Structural constants such as schema revision 1, initial life generation 0 and invalid sentinels describe format/identity. A shipping-content validator must reject unresolved required mechanics and unresolved mandatory references; reviewed prototypes must explicitly resolve with Prototype evidence. Such validators remain W0 integrator/W1/W4 work.

## Historical W0-03 decision checklist (resolved by S-01 acceptance; implementation pending)

1. Review/freeze native revision 1; run actual UHT/C++ build under UE 5.8.3 Linux. Verify descriptor unknown member, Editor TargetAllowList, plugins, generated reflection, module dependencies and BuildSettings enumerators. Engine installation is available; Windows checks remain deferred.
2. Freeze canonical names/prefixes, ruleset alias and primary type mapping, area/entrance/slot/entity/run IDs and cross-area instance policy. Validate duplication and map references before cook.
3. Freeze hash algorithms/canonical bytes, payload checksum, byte/count/growth/RNG/diagnostic limits, revisions/migrations and GUID reward-purpose mapping. Decide bounded replay/ordinary-reward retention without losing idempotency.
4. Approve XP balance/debt normalization, policy vocabularies and concrete fields/units replacing generic definition shell parameters. Freeze explicit no-requirement versus missing interpretation and status access discipline.
5. Narrow ability execution/visual reference types when classes exist; decide cast refunds, effect/cooldown save policy and completed-action boundaries. Add service offer/stock schemas through shared integrator review before W4-07.
6. Decide creation roll authorization representation (validated local preview token or regeneration check); draft caller values are never authority. Freeze gold-only loot semantics, quantity validation, and maximum counters/overflow rejection.
7. Reconcile pending W0-04 visual evidence; freeze portal pairing/landing transforms only after traces and real traversal. Update coordinator-owned status/toolchain ledger after review; these paths were not edited here.

Next work: coordinator runs the full host Lighthaven Automation suite and records the freeze requirement after review. Wave 2/4 owners implement the accepted accompanying behaviours and their validation; no gate pass is asserted here.

> **Schema revision 2 (accepted 2026-10-09):** UseItem, NotUsable/NoEffect and schema 2 with rev-1 migration are defined in `schema-rev2.md`, which supersedes the matching rows here.
