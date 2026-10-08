# S-01 — Schema revision 1 freeze proposal

Decision candidate, 2026-10-08. Source baseline: `7bf9e264e904b3123f00f248b9e29be65bc87a5f`. Contract revision: 1. **Not an applied freeze, migration, implementation or gate approval.** The integrator can accept or amend each numbered decision separately; any amendment must update the final change list before dependent work starts. Only this document is owned by S-01.

The W0-03 warning that Unreal is absent is historical: `g0-linux-report.md` records UE 5.8.3 Linux editor/game compilation, and later combat/integration handoffs record further builds. Those are inherited evidence, not checks performed by S-01. This proposal addresses G0's schema-freeze requirement; it cannot establish the other G0 requirements or G1 play approval. Linux is first; Windows checks remain deferred. No network research, code edits, binary assets or new authentic mechanics claims are included.

Source citations below are repository-relative `file:line` anchors at the baseline. `Core/`, `Rules/`, `Abilities/`, and `Framework/` abbreviate `Source/Lighthaven/` subdirectories. Implementation document names are beneath `docs/implementation/`. Presentation specifications are `art/creatures/README.md` (A-01) and `art/player/README.md` (A-02) beneath that directory. The architecture references are `docs/plan/docs/02-unreal-architecture.md` §§4–7; the JSON files beneath `docs/plan/contracts/` remain examples, not a second runtime schema.

## Coverage of the original integrator list

| contracts-v1.md decision | Proposed disposition / decisions below |
|---|---|
| 1. Native freeze, UHT/build, descriptor/plugins/dependencies | D01; accept semantic freeze only after integrator applies changes and obtains actual validation. |
| 2. Names, alias/types, area/entrance/slot/entity/run IDs | D02–D03, D15; freeze identity rules now, physical landings later. |
| 3. Hash/checksum/bytes/limits/migrations/reward mapping/replay | D04–D07, D11, D17. |
| 4. XP/debt, policies, concrete fields/units, absence/status | D08–D10. |
| 5. Execution/visual types, refunds, save boundaries, services | D12–D14, D16. |
| 6. Creation authorization, gold loot, quantities/overflow | D05, D06, D11. |
| 7. Visual evidence, portal pairing/transforms, coordinator ledgers | D15, D01; no speculative landing freeze or ledger edits. |
| Wave 1 / A-01 / A-02 additions | D07 RNG adapter, D08 Rules behavior, D13–D14 presentation, D16 lifecycle/re-entrancy and pause. |

## D01 — What does accepting the freeze establish?

**Current state:** Core headers still say draft/uncompiled at line 1. `Core/LHSaveSnapshot.h:249` sets schema 1, but there is no serializer. `Core/LHCommands.h:149` supplies only an interface. `g0-linux-report.md` separates observed builds from failed root runtime and unperformed package checks; `w1-integration.md` and `combat-foundation.md` record later compilation and host-only regressions.

**Recommend:** retain schema number 1 for these pre-publication changes. The integrator applies the small header changes below, builds editor/game and UHT on pinned UE 5.8.3 Linux, and records semantic acceptance separately from runtime gate evidence. Review descriptor `_DraftComment`, Editor TargetAllowList, GAS/EnhancedInput/DataValidation and InputCore dependencies against the actual build; do not upgrade the engine. Replace obsolete draft banners only after those checks. No published v1 save is assumed to exist: illustrative JSON and synthetic fixtures are not supported save files. If the coordinator discovers published saves, treat incompatible changes as a migration/revision decision before proceeding.

**Impact:** documentation/build review only apart from the explicit changes in later decisions. Coordinator updates status/toolchain ledgers outside S-01 ownership. Schema acceptance alone must not be reported as completion of G0.

## D02 — Canonical identity, names, ownership and cross-area movement

**Current state:** `Core/LHIdentity.h:9,17,25` wrap character/request/reward GUIDs; `:33` stores content as FName, `:41,49` area/entrance, `:58` entity `(RunId, Area, InstanceId)`, `:69` spawn life `(Area, SpawnSlot, LifeGeneration)`. `Core/LHDefinitions.h:171` stores an authored spawn slot. Nothing validates spelling, duplication or travel identity yet.

**Recommend:** IDs use ASCII, case-sensitive canonical registry spelling even though FName comparisons are insensitive. Dotted content grammar is `[A-Za-z][A-Za-z0-9_]*(\.[A-Za-z][A-Za-z0-9_]*)+`; no whitespace, slashes, filenames or implicit case folding. Validate raw input spelling before constructing FName and use the registry's spelling on output. Reserve prefixes `Ruleset`, `Item`, `Spell`, `Attack`, `Enemy`, `Encounter`, `Area`, `Skill`, `Quest`, `Boss`, `Effect`, `Question`, `Answer`, `CreationPolicy`, `Offer`, `Flag`, `Topic`, `Presentation`. Reservation does not create an asset class. Gameplay tags retain the independent `LH.*` namespace. An alias is an explicit one-to-one import mapping, never an automatically lowercased key. Reject duplicate keys, aliases that collide and unknown mandatory references.

Freeze playable area keys `Area.LighthavenTempleDistrict`, `Area.TempleB1`…`Area.TempleB4`; reserve `Area.Frontend` for frontend content, excluded from playable world state. Entrances use area plus a local dotted canonical name; reserve `Temple.SafeSpawn` in the hub and `Entry`, `Descent`, `SafeSpawn` for floor authoring. A reserved name does not promise a portal exists. Serialize GUIDs as four FGuid 32-bit components in the byte codec (D04); text diagnostics use lowercase 8-4-4-4-12 form. Zero GUIDs are invalid, except explicitly inactive optional fields. Character and campaign RunId are minted once by authority, independent of names/accounts.

Keep `FLHEntityId.Area` as immutable **origin/identity area**, not current location. An inventory instance retains its entire identity when carried across floors or equipped. Container membership locates it; `Character.ActiveEntrance` locates the player. Authored objects instantiate their GUID under the current run and authored area; duplicate placed objects get new InstanceId and SpawnSlot GUIDs. Definition data cannot embed a real campaign RunId: authoring validation treats a portal's zero RunId as a template and runtime materializes it before accepting commands. A future typed portal-template record may replace this convention. Spawn life is implicitly scoped by enclosing World.RunId; cross-run reward keys include that run explicitly. Generation starts at zero, advances once on committed respawn, never on reload, and cannot wrap. Save collections enforce uniqueness across all areas, inventory and corpse/object ownership, not just within one array.

**Impact:** documentation/validators only; keep Core identity layout. This avoids invalidating carried items on travel or replacing stable IDs with actor paths. Runtime target selection's actor-path tie order is session-local (`Framework/LHPlayerController.cpp:158`), not a persistent identity rule.

## D03 — Ruleset alias and primary asset types

**Current state:** `Core/LHDefinitions.h:89–94` builds primary identity from stored Id and `PrimaryType()`. Types are Ruleset (`:107`), Item (`:125`), Ability (`:143`), Enemy (`:162`), Encounter (`:181`), Area (`:195`). `Core/LHValues.h:74` stores ruleset ID/revision/hash. `Rules/LHPrototypeRuleset.cpp` constructs an incomplete ledger fixture, while `Rules/LHBibleRules.h:22` exposes separate documentary tables without a runtime ruleset reference.

**Recommend:** approve the import alias `LH_Prototype_v1` → `Ruleset.LH_Prototype_v1` for the planning policy/example only. Export the canonical key; no other automatic aliases. Preserve the primary types above: both `Spell.*` and `Attack.*` use primary type `Ability`; the full dotted content ID remains the primary asset name. Add type `Presentation` for D13. Prefix/type mismatches reject. Skill/quest/offer/effect classes are future extensions, not silently mapped to Item or Ability.

Do not call `MakeBibleRuleset()` a playable ruleset or substitute it for the prototype fixture. It contains documentary disputes and unresolved combination/creation/initial-resource policy. A future Bible-derived runtime profile needs its own `Ruleset.*` ID, explicit selection policies, revision and hash. No hash of source-file bytes or fake populated hash makes incomplete content playable.

**Impact:** documentation/registry/Asset Manager configuration; D13 alone adds a Core class/type. No silent rename of research baseline text `LH_Prototype_v1` inside provenance.

## D04 — Hash algorithms and exactly which bytes are canonical?

**Current state:** `Core/LHValues.h:79` has an unqualified hash string; `Core/LHSaveSnapshot.h:224` has a payload digest, and `:255–257` length/algorithm/checksum. No codec or canonicalization exists. Architecture §7 requires bounded integrity checking before trusting the payload.

**Recommend:** use SHA-256, lowercase 64-hex digest; algorithm token `SHA256` and codec token `LHCanonicalBinary1`. This is corruption detection, not authentication. Add `FName HashAlgorithm` to FLHRulesetRef and `FName PayloadCodec` to FLHSaveHeader, both invalid/None until explicitly filled. Unknown algorithms/codecs reject before allocation.

Freeze a portable explicit codec, independent of reflection/property iteration, UObject memory and platform archives:

- Integers: signed 32/64-bit two's-complement little endian matching the declared native field; lengths/counts: unsigned 32-bit little endian after limit checks. Bool: byte 0/1. Enum: length-prefixed ASCII symbolic token, never ordinal. Doubles: finite IEEE-754 binary64 little endian; normalize negative zero to positive zero. No NaN/infinity. FGuid: A, B, C, D as unsigned little-endian 32-bit words. Strings/FNames: unsigned byte length then UTF-8, no terminator/BOM; FName uses canonical registry spelling or an empty string for None. Reject malformed UTF-8; preserve valid display-name text exactly without implicit Unicode normalization. Soft references encode canonical package/object paths, never loaded pointers.
- Structs: unsigned field count, then `(field-name string, encoded value)` in ascending ASCII field-name order. The version-specific allowlist supplies each field's type. Every declared wire field is present; missing/duplicate/unknown fields reject v1. No reflection-dependent order. FTransform is a codec struct with TranslationXYZ, RotationXYZW and ScaleXYZ, all binary64; require finite values, positive scale and unit quaternion within absolute squared-norm tolerance 1e-6 (proposed engineering tolerance); canonical quaternion sign uses first nonzero component in W,X,Y,Z positive. Do not recalculate authoritative transforms during hashing.
- Arrays: count then elements. Semantically ordered arrays retain order: question answers, XP thresholds, growth awards (FromLevel order), roll inputs (StreamId order for independent streams), and diagnostics. Keyed sets sort by canonical encoded key bytes and reject duplicates: definitions by Id, areas by Area, encounters by life tuple, inventory/objects/corpses by entity, equipment by slot token, skills/spells/flags/appearance by ID, policies/parameters by Key, claims by GUID, RNG by StreamId, cooldowns by owner then ability, effects by owner then effect/source, receipts by request GUID. Field provenance alternatives sort by FieldPath then their complete encoded bytes; identical duplicates reject. Set-vs-sequence classification is part of the codec, not the caller's discretion.
- FLHInteger/FLHNumber encode Resolution and Provenance; Value encodes zero when Unresolved so unused storage does not change identity. Resolved zero remains distinct through Resolution. Unresolved transform encodes a fixed identity transform and its resolution marker. An unresolved authored policy encodes empty Value. Runtime required values still reject unresolved; canonicalization does not authorize them.

Save payload is precisely the codec struct `{Character, World, Session}`. PayloadLengthBytes counts those encoded bytes. PayloadChecksum is SHA-256 of exactly those bytes, excluding Header. A fixed preamble contains 6 ASCII bytes `LHSave`, unsigned 32-bit envelope byte length, then bounded canonical FLHSaveHeader bytes, then payload; no compression or trailing bytes in v1. Sequence is positive and header/character/rules/run references must cross-validate after checksum. Header has no authenticity guarantee; modifying header metadata is not prevented by a payload checksum. Maximum header bytes are D05.

Ruleset ContentHash covers the canonical resolved **mechanical closure**: algorithm-version tokens, ruleset ID/revision, parameters/policies/thresholds and each referenced mechanical definition sorted by ID, including eligibility, effects, economic/reward/loot data and provenance. Exclude ContentHash itself, migration policy, display text, presentation IDs/visual paths, art budgets and transient runtime state. Preserve resolution and disputed documentary status; a required unresolved field still blocks runtime activation. A rules adapter must publish this allowlisted closure before issuing saves, with golden bytes/digests. ContentRevision is SHA-256 of the complete gameplay definition catalog encoded by the same rules, including area topology/landing transforms, excluding cosmetic bindings. Request digest hashes the codec struct containing command type plus **every request field**, including ID/epoch/token, with domain string `LHRequest1`. Alias normalization occurs once at import; hash canonical IDs afterward.

**Impact:** Core additions named above; serializer/hash/validator implementation belongs to W2-01 and integrator. Reasons: cross-platform reproducibility, field-order independence and no accidental save invalidation from mesh swaps. Acceptance freezes this byte contract; W2-01 must produce golden vectors, corruption/truncation/duplicate-field tests and Linux round trips before durable saves are relied upon.

## D05 — Bounded parsing, collections, counters and diagnostics

**Current state:** all Core save collections are unconstrained TArrays; `Core/LHSaveSnapshot.h:250,255` uses int64 sequence/length and `Core/LHIdentity.h:74` int64 generation. Rules uses checked addition, e.g. `Rules/LHRules.cpp:208` for XP. No load limits exist.

**Recommend:** these are **proposed engineering limits**, not T4C mechanics or gameplay tuning. Per generation: 16 MiB total file, 64 KiB header, at most 15 MiB payload; nesting depth 16, at most 100,000 aggregate collection elements, general string ≤4,096 UTF-8 bytes, ID/token ≤128 bytes, display name ≤128 bytes. Enforce both total bytes and collection counts before allocating; never trust counts times element size without checked arithmetic.

| Collection | Proposed hard maximum |
|---|---:|
| Playable areas | 5 |
| Encounters / objects / retained corpses, each per area | 4,096 / 4,096 / 1,024 |
| Inventory items / equipment bindings / learned skills / spells | 512 / 8 / 256 / 256 |
| Growth awards | 4,096 (also ≤ frozen ruleset level-transition count) |
| Quests / bosses / permanent claims / unlocked portals | 256 / 64 / 4,096 / 256 |
| Recent accepted requests per active epoch | 4,096 |
| RNG streams / bytes per state / historical roll states per award | 4,096 / 64 / 16 |
| Cooldowns / durable effects | 1,024 / 1,024 |
| Appearance IDs / creation answers | 16 / 4 |
| Mechanical/policy fields per record / provenance alternatives per field | 64 / 16 |
| Diagnostics | 64 strings, ≤512 bytes each, ≤32 KiB combined |

Maxima are inclusive; reject over-limit loads/commands, never silently discard growth/claims/items. Diagnostics alone may drop oldest entries and are nonauthoritative. Counts do not imply starter grants, level caps or training limits. Runtime quantity is resolved positive int64 for buy/sell/item/gold transfer; allocations allow explicit zero deltas but require positive total. Gold, XP/debt/pools/generation/sequences are nonnegative signed int64; positive transaction sequence starts at 1. Reject on checked overflow, including generation/sequence exhaustion, quantity × price and debt addition; no wrap/saturation. Resource/timer doubles must be finite and convert safely to GAS float bounds. At capacity return structured failure or perform safe lifecycle compaction/epoch rotation (D06), never lose reward protection. Authoring validation must prove the selected slice fits the caps before content ships.

**Impact:** documentation/codec/validators only; no Core arrays resized or gameplay values guessed. W2-01 owns actual enforcement and failure visibility.

## D06 — Reward minting and bounded replay retention

**Current state:** FLHRewardId only holds a GUID (`Core/LHIdentity.h:25`). Encounter/corpse store kill reward (`Core/LHSaveSnapshot.h:116,127`); unique claims live at `:194`; receipt contains ID/digest/sequence at `:220–225`. Combat's runtime AppliedHits (`Abilities/LHCombatComponent.h:71`) does not provide persistent reward idempotency. Requests are arbitrary GUIDs, so discarding old receipts would make old intents appear new.

**Recommend reward mapping:** authority derives a deterministic GUID from SHA-256 of domain `LHReward1` plus canonical run ID, source kind, source key and reward-purpose token. Use first 16 digest bytes as four little-endian FGuid words; reject all-zero/collision against a different source tuple rather than rerolling. Source keys: ordinary enemy life `(Area, SpawnSlot, LifeGeneration)`, growth `(CharacterId, ToLevel)`, quest `(QuestId, objective stage)`, boss `(BossId, permanent objective)`. Purposes: `KillSettlement`, `LevelGrowth`, `QuestTurnIn`, `BossUnique`. If boss death/dialogue grant the same entitlement both use the same boss-objective key and `BossUnique`, not their trigger names. Separate unrelated rewards have separate purpose keys. Persist source lifecycle, claim, XP, RNG result and finalized loot in one completed transaction before publishing notifications; do not mint a fresh GUID on a retry. Reward IDs are claims, not item or attack IDs.

Keep permanent unique claims and every historical growth award for v1, bounded by D05. Ordinary life retention uses the **existing latest encounter generation/state** as a high-water record: reject settlement of a lower generation; for the current generation honor bRewardCommitted. Keep finalized corpses while transferable contents remain. Retire an old corpse only after empty/claimed or explicit cleanup transaction discards its contents, and no live action references it. Never reset latest generation on map reload. Cleanup and future compaction cannot recreate a claim; replay of an unavailable older target is rejected.

**Recommend request retention:** add `FGuid Epoch` to FLHRequestId and `FGuid RequestEpoch` to FLHSessionRecord. Epoch is an authority-issued campaign command epoch, not caller time. Every issued request carries the current epoch; reject other epochs before lookup/mutation. Keep all accepted receipts in that epoch; identical ID/digest returns accepted None, original committed sequence and bReplay=true, differing digest rejects ReusedRequestId. Those are all outcome fields currently present in FLHCommandResult; creation identity is read from the authority's selected profile, not synthesized again. Rejections do not mutate and need not be durable receipts; a rejected request may be retried against changed state until accepted.

Rotate epoch only at a completed boundary with no outstanding UI/async intents or callback publication: commit fresh epoch plus empty receipts atomically, durably save it, then issue fresh intents. If the old generation is loaded, its epoch/receipts and world roll back coherently; local save rollback is not anti-cheat. At 4,096 receipts stop new mutations until safe rotation/save succeeds. UI must not manufacture new IDs to retry an accepted but uncertain intent. Profile creation uses an authority-issued bootstrap epoch and retains the accepted create receipt in the created profile; at most one creation confirmation is in flight. W2-01 must recover an existing committed profile/receipt after an uncertain response instead of allocating a second character slot.

**Impact:** exactly two Core GUID fields plus replay validation; no unbounded tombstone database. Epoch invalidation makes bounded retention safe for arbitrary GUID intents. Reward generation is an integrator helper shared by lifecycle/progression/quest owners; no UI minting.

## D07 — Gameplay RNG and historical roll encoding

**Current state:** `Core/LHValues.h:85–91` has opaque state; `Abilities/LHCombatComponent.h:40–41` copies an FRandomStream. `Abilities/LHCombatComponent.cpp:113` draws hit/damage fractions only after impact validation. `Rules/LHRules.h:93` returns AcceptedRolls aligned with awards; Core RollInputs are not filled by Advance (`Rules/LHRules.cpp:241`).

**Recommend:** freeze state token `UE.FRandomStream`, AlgorithmRevision 1, engine implementation pinned to 5.8.3. State is exactly four little-endian bytes representing the current seed's 32-bit bit pattern, read with GetCurrentSeed; restore by Initialize with that bit pattern converted to signed int32, then SetCombatRandomState. Initial seed/reset behavior is not authoritative: never Reset on gameplay streams. GetFraction outputs and state evolution require native golden-vector tests; do not serialize FRandomStream memory. A future engine change must prove vector equivalence or migrate/retain this implementation. Stream IDs are canonical dotted keys, e.g. `RNG.Combat.<InstanceGuidDigits>`, `RNG.Creation`, `RNG.Growth`, `RNG.Loot`; independent actor streams must be uniquely associated through immutable instance IDs. No shared cosmetic stream and no reseed on load.

Historical explicit rolls use separate token `LH.NormalizedRollPair`, revision 1, state exactly 16 bytes: Health then Mana finite binary64 little endian, each in [0,1]. Put one such record with StreamId `RNG.Growth` in each award's RollInputs from AcceptedRolls. These are documentary inputs, not resumable gameplay streams; Session.GameplayRng rejects this token. Creation retains its authority-selected outcome and generator inputs; it must not pretend that Bible representative roll rows define a random distribution. Missing/unknown state when required rejects; valid current seed zero is allowed. Snapshot RNG after all settled draws, and preserve before-state/accepted outcomes where history requires it. Rejected validation consumes no RNG; accepted misses consume the same two combat draws as hits.

**Impact:** documentation and RNG adapters only; existing Core shape is sufficient. W2-02 must encode roll history before publishing a progression transaction. Dev seed 123 is prototype fixture data, not a default save seed.

## D08 — XP balance/debt and growth history

**Current state:** `Core/LHSaveSnapshot.h:88–89` separates balance/debt. `Rules/LHRules.cpp:174–244` rejects negative values, requires balance ≥ earned-level threshold, pays debt first, advances only above earned level, checks exact roll count and returns per-level history. Architecture §4's below-threshold wording needs normalization to this representation.

**Recommend:** accept current Rules convention unchanged. Balance is retained cumulative threshold progress, debt is additional nonnegative loss. Death adds the approved penalty to debt without subtracting it from balance; it preserves earned level, HP/MP history and points. New XP first pays debt, remainder increases balance. Example using synthetic numbers only: balance 150, debt 30, gain 20 gives 150/10 with no grant; next gain 15 gives 155/0, and only thresholds above previously earned level can grant awards. Never encode loss both as reduced balance and debt. Penalty magnitude/denominator remains rules content, not schema.

Validate old history, IDs and level transitions across the whole save: Advance checks duplicate IDs only within its supplied batch. Preserve earned base maxima and append growth, never replay grants or recompute past HP/MP from current stats. The finite selected threshold table establishes its cap; its last row is not inferred from a missing documentary Next value. The proposed generic linear growth model remains Prototype. Bible HP bands, mana components and disputed XP rows stay documentary until approved combination/timing policies exist. Pure Rules result is applied only on accepted diagnostic; empty rejection output is not a valid character.

**Impact:** documentation/validation only; no Core/Rules arithmetic change. Migrate any actual older reduced-balance save by an explicit policy using original earned threshold/history, with fixtures; do not guess that deficit on ordinary load.

## D09 — Policy vocabulary and concrete runtime fields

**Current state:** `Core/LHDefinitions.h:20,29` provides generic mechanical/policy fields. `Rules/LHRules.h:95–100` already has typed basis and quiver policy; `Rules/LHRules.cpp:253` rejects *every* eligibility Policies entry. Combat config (`Abilities/LHCombatComponent.h:10`) supplies typed timing and bounds absent from ability definitions. Generic Rules and Bible tables are different inputs, not interchangeable assets.

**Recommend:** empty Eligibility.Policies is the complete supported extension vocabulary for v1. Feed typed `FRequirementPolicy` from a validated rules adapter, with explicit Base or Effective provenance and boolean BowRequiresQuiver; do not populate generic strings the current code will reject. No universal Base/Effective answer is inferred from the Bible. The adapter selects and records this policy for the published ruleset.

For other definition policy arrays, freeze only the following key/value pairs when relevant; require Resolved/provenance, reject duplicates and unknown keys/values. These tokens are recommendations, not already interpreted code:

| Definition scope / key | Allowed rev1 value / semantics |
|---|---|
| Ability `TargetPolicy` | `LiveOtherEntity`: distinct live source/target |
| Ability `ImpactValidation` | `RangeAndLOS`: recheck at native impact |
| Ability `CancellationRefund` | `NoRefundAfterCommit`: precommit rejects cost-free; committed cost/cooldown retained |
| Ability `MovementPolicy` | `SuppressUntilImpact`: inhibit movement while pending; cosmetic recovery never locks |
| Encounter `RespawnClock` | `LoadedFloorActiveSimulation`: stop paused, off-floor and closed |
| Encounter `LiveReload` | `AnchorKeepHealth`: restore lifecycle/health, reset live placement to anchor |
| Encounter `RespawnPolicy` | `OrdinaryRepeat` or `PermanentDefeat`; boss must explicitly select permanent |
| Item `QuiverConsumption` | `Unlimited`: no per-shot decrement for selected Wooden Arrows |
| Ruleset `ExperienceAccounting` | `RetainedBalanceSeparateDebt` |

Use existing named RangeCm, CooldownSeconds, ManaCost and RespawnSeconds fields. Add `FLHNumber ImpactSeconds` to ULHAbilityDefinition so combat does not require an undocumented mechanical field. No RecoverySeconds mechanical field: recovery is cosmetic (D14). Generic combat coefficients and AI/effect fields need a per-adapter registered key/type/unit allowlist before execution; no arbitrary expression evaluator or second rules engine. Use cm, seconds, HP/MP units, integer XP/gold/points/quantities, chance/normalized-roll fraction [0,1], and named attribute fields. Do not port sample max values into formulas. Required unresolved parameters still block that action/profile.

**Impact:** one Core timing field; vocabulary/adapter documentation otherwise. This deliberately freezes implemented semantics without inventing spell refund rules, enemy AI coefficients or a service stock model.

## D10 — No requirement versus missing; provenance discipline

**Current state:** `Core/LHValues.h:15,42,52` separates presence from provenance. `Core/LHDefinitions.h:43–48` has no collection resolution markers for prerequisite arrays. `Rules/LHRules.cpp:249–255` requires every minimum and resolved basis, rejects extension policies. Bible lookups intentionally accept documentary Disputed data (`Rules/LHBibleRules.cpp:9`); generic runtime and combat reject Missing/Disputed.

**Recommend:** an explicitly validated definition with all five attribute minima and MinimumLevel resolved zero, empty RequiredSkills/RequiredSpells/SkillMinimums and empty Eligibility.Policies means no prerequisites. Those empty prerequisite lists are intentional only on an explicitly registered complete definition; an absent definition/required scalar is missing and rejects. Authoring import must require these arrays to be present; an omitted JSON array cannot silently become empty. Unresolved zero storage is never no requirement. A resolved enemy LootResolution plus empty Loot is deliberately no item loot; unresolved means unknown even if array empty, and GoldReward is independently resolved.

Confirmed means documented for named baseline, VerifiedT4C requires actual evidence beyond this task, Prototype/Modernized retain their distinctions. Each mechanical authored value needs field provenance; no runtime code upgrades status. The Bible API may inspect disputed documentary selections, but runtime adapter must make a reviewed explicit selected profile and preserve disagreement. Do not pass Bible diagnostic acceptance directly into gameplay authority. Named attribute spellings remain Strength/Endurance/Agility/Intelligence/Wisdom; AGI/Dexterity imports explicitly alias to Agility, never reorder INT/WIS by positional arrays.

**Impact:** documentation/import validation only; no new null sentinel or Core minimum enum needed. A later revision may add per-list resolution if incomplete authoring must be serialized as runtime data.

## D11 — Creation-roll authorization and gold-only pickup

**Current state:** `Core/LHCommands.h:34–40` accepts a caller Creation record with no authorization; `Rules/LHRules.cpp:118` RollCreation validates a caller-selected table outcome but does not own RNG/authority. `Core/LHCommands.h:128–134` TakeLoot has only Container/Item/Quantity; `Core/LHSaveSnapshot.h:129` separately retains gold.

**Recommend creation:** add `FGuid PreviewToken` to FLHCreateCharacterRequest. Local authority issues an opaque token mapped to exact accepted answers, generation policy/revision, authority RNG input, rolled attributes, unspent budget and ruleset hash. Confirm validates token and byte-equivalent normalized Creation, name/appearance legality, then commits once under RequestId. Caller-selected attributes/table index/RNG bytes are never sufficient authorization. Reroll supersedes the previous token for that form. Tokens are session-local, not saved as canonical character state; an unconfirmed form after restart requests a new preview. Accepted Creation and its generator evidence are saved. Do not reconstruct Classic creation probabilities from representative Bible charts; unresolved true generation blocks historical mode. A separately declared prototype generator may be approved without changing the command seam. Maintain bootstrap replay rules in D06 across profile creation failures.

**Recommend loot:** add `ELHLootTransferKind { Unspecified, Item, Gold }` and Kind default Unspecified to FLHTakeLootRequest. Item kind requires a valid owned source item and resolved quantity >0 within remaining stack; Gold kind requires Item be the all-invalid/default entity sentinel and Quantity be the resolved positive amount of gold. UI requests the displayed authoritative remainder to take all; authority revalidates it and may reject stale quantity. Reject Unspecified, mixed Item/Gold payloads, zero/negative/unresolved quantities and overflow. No magic Item.None ID or quantity-zero means-all semantics. Gold does not consume inventory capacity; capacity applies to item transfer. Each command commits matching source subtraction/destination addition and receipt together. Finalized corpse bClaimed means both RemainingItems empty and RemainingGold resolved zero; gold-only corpses are not automatically claimed. Full inventory item rejection leaves all state unchanged. Partial item/gold transfers retain source remainder.

**Impact:** Core token, enum and Kind field only; authority/preview implementation in W2-02 and loot owner W4-04. Explicit kind prevents accidentally interpreting a malformed item identity as gold.

## D12 — Ability class narrowing and effect/cooldown boundary policy

**Current state:** `Core/LHDefinitions.h:134` is TSoftClassPtr<UObject>, although ULHBasicAttackAbility exists. `Core/LHSaveSnapshot.h:15–17` has CompletedActionBoundaryOnly; cooldown/effect records at `:198–215` have no owner; Session arrays at `:235–237` do not specify scope. Combat stores runtime world-time cooldown (`Abilities/LHCombatComponent.cpp:141–149`), and publishes synchronous delegates during impact (`:116–117`).

**Recommend:** narrow ExecutionClass to `TSoftClassPtr<UGameplayAbility>` with forward declaration and validated subclass resolution. Keep explicit native cost/cooldown as the single owner for the current basic attack; attaching additional GAS cost/cooldown effects would double-charge. Reject unapproved ability policies rather than inheriting cast refund defaults.

Completed-action boundary means no pending activation, impact timer, queued transfer, death/reward settlement, travel transition or notification publication stack, across all actors included in the snapshot. It does **not** mean every cooldown is zero. For explicit save/travel, settle or cancel active attacks by the selected no-refund policy, unwind callbacks, freeze simulation, then snapshot; autosave coalesces until a valid boundary. Do not write during OnDeath/OnImpact. Cosmetic recovery may still be playing and is not serialized.

Persist all gameplay cooldown remaining seconds, fractional mana carry and approved durable effect definition/inputs/stacks/remainder. Active-simulation clock stops paused/closed; ordinary encounter timers also stop off-floor. No wall-clock catch-up. Add `FLHEntityId Owner` to FLHCooldownRecord and FLHDurableEffectRecord; Source remains the effect source. Materialize a stable player entity using campaign RunId, hub origin area and CharacterId GUID as InstanceId (D02); enemy owner uses its stable encounter entity. Cooldown key is owner+ability; in v1 each ability owns its own cooldown, shared tag groups remain later work. Durable effect key owner+effect+source is unique, stacks combine there; multiple independent identical-source instances need a future instance ID. Validate owners/source references, allowing a settled historical source identity without requiring its live actor. Persist off-floor enemies' canonical cooldowns/effects as values too.

Restore definitions/migrate → owned components → base/earned attributes → inventory/equipment → allowed durable effects → clamped current resources → RNG/cooldowns/timers → fresh actor-info life initialization → enable simulation/input with release gating. Never serialize GAS handles/specs/aggregates, runtime AppliedHits, pointers, timers or animation graphs. V1's approved durable-effect allowlist starts **empty** because no executable durable effects exist; reject a nonempty array until a reviewed definition/adapter supports it. Transient audiovisual effects are discarded. Any future temporary effect with gameplay consequences must get a save/rebuild policy before being enabled; do not erase a damaging debuff as a save benefit. No implicit poison/buff implementation follows from the record shell.

**Impact:** narrowed class and two owner fields in Core; completed-boundary/effect allowlist is documentation/authority policy. Owner fields prevent enemy cooldown resets or misapplying an enemy effect to the player after load.

## D13 — Presentation registry and appearance selections

**Current state:** `Core/LHDefinitions.h:91` has generic Visual but no PresentationId; A-01 `art/creatures/README.md` proposes one Presentation.Enemy mapping per eleven Enemy IDs. A-02 `art/player/README.md:35–51` proposes body/face/hair/skin/outfit IDs and compatibility, not assets. `Core/LHSaveSnapshot.h:84` and `Core/LHCommands.h:39` store AppearanceIds. No typed resolver exists.

**Recommend:** add FLHContentId PresentationId to ULHDefinition and add ULHPresentationDefinition : UPrimaryDataAsset with stored FLHContentId Id, `TSoftObjectPtr<UObject> Visual`, and GetPrimaryAssetId returning type Presentation plus Id.Value. Retain ULHDefinition.Visual as deprecated authoring-only fallback for pre-freeze fixtures; published playable definitions resolve exclusively through PresentationId. Do not derive presentation identity from filenames or infer gameplay from a mesh. Generic soft UObject within the presentation binding deliberately permits primitive assembly/Blueprint first and mesh bundles later; validate supported binding classes in the resolver before use, keep the registry itself typed. No new hard asset dependency.

Freeze A-01's exact `Presentation.Enemy.<RosterSuffix>` mapping for BrownRat, Bat, DungeonBat, GreenSlime, GiantBat, UndeadBat, GiantSpider, Goblin, GoblinWarrior, Atrocity, Balork. Shared meshes retain separate binding records. Approve A-02's `Presentation.Player.Body.A/B`, `Face.A/B`, `Hair.Cropped/Tied`, `Skin.LightWarm/MediumWarm/DeepWarm`, `Outfit.StarterLinen` as catalog keys, not observed assets or class grants. AppearanceIds canonically contains one body, compatible face, one hair, one skin, one outfit. Authority expands bundled face from body at submission then persists the explicit compatible face; UI need not add a face selector. Reject incompatible/duplicate/unknown categories; presentation style never changes equipment, learned spells, capsule or stats.

Cosmetic binding changes preserve IDs, saves, collisions and mechanical hashes. Missing visual assets retain a labeled family/modest human proxy and emit bounded diagnostic; unknown mandatory gameplay IDs reject. Registry resolves assets asynchronously and Asset Manager cook validation proves inclusion. Import/rights/nav/camera acceptance remains W4/W5 work; no reference image is authorized for shipping by this decision.

**Impact:** one Core field and a small registry asset class; resolver/content mapping outside Core. This supplies A-01's replacement seam without freezing a skeleton/material/animation implementation into persistent saves.

## D14 — Attack commit, cancel, finish and cosmetic recovery events

**Current state:** `Abilities/LHCombatComponent.h:25–26,55–56` exposes only OnDeath/OnImpact. `Framework/LHPlayerController.cpp:191` OnAttackRequested is controller-local and fires *after* RequestBasicAttack returns; a zero-delay impact may already have happened. `Abilities/LHBasicAttackAbility.cpp:21–25` handles cancellation and zero-delay timing; it ends at impact. A-01 requests commit/cancel synchronization and an interruptible cosmetic recovery after its 1-second prototype impact preset. There is no production recovery duration.

**Recommend:** add native read-only component delegates outside Core: `OnAttackCommitted`, `OnAttackCancelled`, `OnAttackFinished`, plus a value event carrying hit identity, source/target stable entity IDs, ability ID, source/target life generation, commit simulation time and ImpactSeconds. Finished carries outcome `ResolvedHit`, `ResolvedMiss`, `ImpactInvalidated`, or `Cancelled`; Cancelled additionally carries a stable reason (`Explicit`, `SourceDeath`, `AvatarCleared`, `Travel`). Identity is captured by value, never a reference into mutable pending state.

Commit event occurs exactly once after successful canonical native cost/cooldown mutation, before immediate impact or timer scheduling. Publication is re-entrant: if a cost/commit listener cancels or replaces the activation, the resumed old path must check activation serial/identity before scheduling anything. Cancel event emits once only for a successfully committed action ended without completed impact; a precommit rejection does not start windup or cancellation animation. Finished emits once for every committed action including invalidated range/LOS/life checks that currently return false without OnImpact. OnImpact remains resolved hit/miss feedback; on lethal hits target OnDeath may occur before attacker OnImpact. No presentation handler awards damage/loot/XP or authorizes requests.

Windup aligns contact to native commit+D active-simulation time. D=0 immediately resolves; animation FPS/LOD/socket callbacks do not control it. Finished-resolved begins optional cosmetic recovery; cancelled/invalidated interrupts windup without pretending a hit. Recovery completion is a presentation-local event, not a mechanical RecoverySeconds field, cooldown completion or movement lock. New activation/death/travel/life replacement interrupts recovery; ignore old identity callbacks. Component events support enemies and keyboard/gamepad equally; never animate from raw button press/OnAttackRequested. No event is persisted.

**Impact:** no Core header additions; native Abilities/Framework event adapter requires integrator implementation before live A-01/A-02 animation. Documentation alone cannot claim that these missing delegates exist.

## D15 — Entrances, portal pairing and unresolved art evidence

**Current state:** `Core/LHDefinitions.h:64–81` has explicit transform resolution and directed endpoints; `:189–193` resolves area map/fallback/portals. `Core/LHSaveSnapshot.h:68` stores checkpoint transform resolution. Map/reference/layout documents and A-01/A-02 contain proposed measurements, not executed traversal evidence.

**Recommend:** freeze area/entrance ID rules (D02), independently authored forward/reverse portal edges and source-save → destination-validation → arrival-save policy from architecture §7. Destination must equal the authoritative portal's permitted edge, not a caller teleport request. Do not freeze proposed origin/identity transforms or visual door/capsule measurements as safe locations. Required landing, fallback, soft UWorld cook inclusion, clearance and actual both-way traversal need W3 authoring/host validation. Failure restores durable source; failed pre-travel save blocks travel with retry/cancel. Death consequence and safe checkpoint settle once before respawn; avatar teardown cannot destroy the only inventory copy. No travel rewards.

**Impact:** documentation/validator only. Coordinator reconciles W0-04/world floor disputes and updates ledgers when real evidence arrives; S-01 edits none. Reserved IDs do not settle Undead Bat/Dungeon Bat placement, camera FOV or commercial rights.

## D16 — Wave 1 review findings, pause and authority seams

**Current state:** `docs/reviews/G1-skeptic.md` reviewed an older candidate and found dead movement and stale callback teardown. Current `Framework/LHPlayerController.cpp:86,103–114` checks live matching avatar and clears movement; `Abilities/LHBasicAttackAbility.cpp:29–40` snapshots activation serial and checks it before ending, `:45` validates end before clearing shared timer. Source contains fixes, not runtime proof. `Framework/LHPlayerState.cpp:22–28` clears avatar on death; controller screen change at `:197` changes input context without pausing simulation. Current controller invokes combat directly, not ILHCommandHandler request replay.

**Recommend:** retain the fixes and require the host timer-driven replacement/cancel/death and dead-player-movement regressions before Wave 2 relies on them. Apply the same serial/identity discipline to D14 commit/cancel events and reward notifications. A completed-action snapshot waits for all re-entrant publication to unwind, even when bPending has already been cleared. Selected targets remain transient; authority resolves canonical IDs and validates again. No UI health/gold/XP setter or bypass of the command handler in W2-03.

UI session owner must explicitly pause simulation for pause/menu policies; switching Enhanced Input context alone is not proof timers stopped. Restore Gameplay only after canonical restore and neutral/release gating. Keep ASC owned by PlayerState for player, by enemy for enemies; inventory/progression lives in canonical character/world authority, not pawn lifetime. Phase W2 command handler into the current direct dev-attack path without creating a competing bus. Ordinary UseAbility request acceptance commits activation cost/cooldown; it does not promise a hit. Pending attack receipts may be runtime-only until settlement; durable snapshots contain only completed boundaries. Startup A/B recovery may undo an unsaved intent coherently, and must not resurrect runtime pending casts from a receipt.

**Impact:** documentation and authority/UI integration; no extra Core changes. G1 hardware, viewport, maps, timers and package evidence remain separate host checks.

## D17 — Service offers and revision/migration boundaries

**Current state:** `Core/LHCommands.h:75–92` includes Offer in buying and vendor/item identity for selling; no service offer/stock asset exists. Train/Learn fields at `:54–70` do not distinguish UI-provided price because price is intentionally absent. Generic policies/definitions cannot safely invent missing costs or trainer placement.

**Recommend:** freeze owner-bound command shapes, authoritative offer/price lookup, distinct learning versus cast costs, all-or-nothing train/buy/sell/equip, quiver slot and unlimited Wooden Arrows semantics. Reserve Offer.* identifiers; defer stock/restock/buyback/service assets to W4-07 through shared integrator review. Wave 2 may implement inventory/knowledge representation and disabled unresolved service actions, but cannot invent a vendor economy or relocate historical trainers. Before a persisted stock model is enabled, add its DTO and migration in a new schema revision; purely authored offer catalog extension needs content revision/compatibility review.

All schema/rules changes use explicit compatibility decisions: future schema version rejected before allocation; no silent defaults for new required fields. Rules revision/hash changes affecting growth/items require migration or new-character policy. Cosmetic changes preserving IDs do not migrate character mechanics. Aliases are version-scoped, collision-checked and applied once; unknown IDs never map to the nearest visually similar species.

**Impact:** documentation only for rev1; future shared schema changes held by integrator.

## Proposed schema rev 1 change list for the integrator

These are the only recommended pre-freeze Core layout changes. Apply before publishing v1 saves; otherwise increment schema and supply migration. Existing unrelated DTOs remain intact.

1. `LHValues.h`: add `FName HashAlgorithm = NAME_None` to FLHRulesetRef; require SHA256. Keep opaque RNG records and current XP/history types.
2. `LHIdentity.h`: add `FGuid Epoch` to FLHRequestId. `LHSaveSnapshot.h`: add `FGuid RequestEpoch` to FLHSessionRecord; require positive receipt sequence and epoch match, retain receipts by D06.
3. `LHCommands.h`: add `FGuid PreviewToken` to FLHCreateCharacterRequest; define ELHLootTransferKind with Unspecified/Item/Gold and add Kind default Unspecified to FLHTakeLootRequest.
4. `LHDefinitions.h`: forward-declare UGameplayAbility and narrow ExecutionClass to TSoftClassPtr<UGameplayAbility>; add FLHNumber ImpactSeconds to ULHAbilityDefinition.
5. `LHDefinitions.h`: add FLHContentId PresentationId to ULHDefinition; retain Visual only for pre-freeze fallback. Add ULHPresentationDefinition with Id, soft UObject Visual and fixed Presentation primary identity (D13).
6. `LHSaveSnapshot.h`: add FLHEntityId Owner to both FLHCooldownRecord and FLHDurableEffectRecord; add FName PayloadCodec = NAME_None to FLHSaveHeader. No serialized animation/recovery fields.

Documentation/implementation accompanying acceptance: publish ID/alias/type and policy registry; implement D04 codec/hash vectors and D05 limits; RNG adapter/roll history; request epoch/token/reward helper; owner-scoped timer/effect restore; D14 native presentation delegates with re-entrancy regressions. Integrator removes historical uncompiled banners after actual UHT/editor/game validation. These accompanying behaviors are requirements for their owning waves, not claims implemented by this document. Freeze acceptance record must identify applied revision, amended decisions, native checks and remaining gate prerequisites.

## What Wave 2 can rely on once frozen

- W2-01: one immutable coherent character/world/session snapshot, explicit canonical bytes/checksums/limits, A/B recovery, version rejection, owner-scoped cooldowns, independent gameplay RNG, completed-action boundaries and bounded epoch replay. It still must implement and run round-trip/corruption/fallback/save-failure fixtures; declarations alone are insufficient.
- W2-02: stable character/run/entity identities, named attributes, retained XP/separate debt and per-level immutable growth, authority-issued preview confirmation, compatible appearance catalog, canonical inventory/equipment/quiver and explicit gold transfer. No historical generator, initial resource kit or unsupported formula is implied by schema acceptance.
- W2-03: one owner-bound command interface with structured atomic rejection/replay and no caller prices/rewards, appearance choices independent of class/build, explicit pause/focus/release restoration, and presentation read events after authoritative commit. Native controller dev events remain adapters until canonical handler integration.

Opening dependent work remains the coordinator's decision after required gate evidence. This document supplies accept/amend decisions and no gate pass.

## Explicitly open after rev1 and migration rule

Keep historical creation distribution, HP/MP combination/timing, initial grants, death penalty numbers, spell/ranged/refund/target policies, AI/loot distributions, trainer cost units and floor disputes unresolved until sourced or explicitly reviewed Prototype. Existing canonical fields can receive approved content only with new ruleset/content revision/hash and compatibility review; never upgrade provenance silently. No new numeric mechanics were researched here.

Service stock/buyback/restock, shared cooldown groups, independently stacked same-source effects, broader world areas, live-position persistence, richer portal templates, more creation questions/appearance categories and rebirth gameplay need a later schema revision when they affect saved shape/meaning. Larger collection limits or an engine RNG algorithm change need compatibility review and vectors; reject unsupported saves rather than truncate. Typed mesh/skeleton bundles, rig sockets/LOD/materials, final landings and art rights remain production work; pure cosmetic binding changes preserve IDs without schema migration.

For every later incompatible change: bump SchemaVersion, retain a bounded decoder for the old version, migrate into a new validated snapshot without rewriting historical grants/finalized loot, preserve identity/replay high-water facts, then write the older A/B slot only after successful migration while retaining the original valid generation. Unsupported or ambiguous migration rejects visibly; RequireExplicitMigration or NewCharacterRequired is honored. Never reinterpret a v1 file under changed semantics with the same version/hash. No migrations from the illustrative null-filled JSON or transient fixture profiles are promised.

## Integrator decision (coordinator, 2026-10-08)

**Accepted as proposed:** D01–D17 and the six-item schema rev 1 change list above. S-02 applies the Core header changes. Schema revision 1 is **frozen** once S-02 compiles and the full `Lighthaven` Automation suite passes on the host; that completes G0's schema requirement. The accompanying behaviours (codec/hash vectors, limits, RNG adapter, request epoch/reward helper, owner-scoped restore, presentation delegates) belong to their owning Wave 2/4 tasks.
