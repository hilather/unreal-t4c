# W4-04 rewards, encounter lifecycle and persistence

Contract revision 1, base `58d4245d1506d52261cb2d3177d972a85eafc176`.
Candidate domain implementation; session/actor wiring belongs to W4-06. No Core,
schema version, migration or binary asset changes.

## Interfaces and transaction order

`Rewards/LHRewardTypes.h` publishes `FLHKillRewardSpec` (`Experience`, `GoldMin`,
`GoldMax`, `ItemDropChance`, `Loot`, `RespawnSeconds`, `SafetyDistanceCm`,
`RespawnPolicy`, `bBoss`) and `FLHKillFacts` (`Area`, `Life`, `Enemy`,
`bKillerIsPlayer`). All numeric spec fields are resolved provenance-bearing Core
values, with no silently supplied mechanical defaults. OrdinaryRepeat and
PermanentDefeat are explicit policies; Balork uses OrdinaryRepeat.

`SettleKill` copies the snapshot, locates the latest slot life, rejects old/future
or already settled lives, validates spec bounds and collisions, grants XP through
an imported character authority copy, draws finalized corpse contents from the
persisted `RNG.Loot` stream, stores dead state/timer/reward, claims first boss defeat,
then invokes `ILHKillObserver::OnSettledKill`. It validates the completed candidate
before replacing the original. Observers may change only their candidate snapshot:
no delegates, actor effects, I/O or external counters before commit. A rejection
rolls back XP, growth RNG, loot RNG, corpse, boss and observer snapshot edits.
Notifications follow acceptance, outside this function. An already committed life
returns InvalidLifeState without calling observers or drawing RNG again.

Character authority helpers internally increment sequence. Domain functions restore
the input sequence after export, so W4-06 owns exactly one transaction sequence
increment (receipt commit for commands, explicit checked increment for kill/death).
They never write receipts. Preserve the authority copy's growth IDs/roll evidence.
Kill facts are trusted local director input, not caller-owned command intent;
non-player kills reject. The director must validate the actual lethal impact.

`PopulateArea` initializes generation-zero, full-health registry records only when
no encounter, object or corpse records exist. W4-06 calls it only on first visit;
reload/travel must hydrate existing records rather than reconstructing population.

`AdvanceRespawns(Area, Seconds, Safe, NewLives)` is the high-water transition helper:
only loaded/unpaused elapsed simulation is supplied, unsafe due slots wait, and
permanent defeats never advance. It cannot infer catalog max HP or campaign RunId
from an area-only DTO. **Integration must use the complete overload**
`AdvanceRespawns(Area, Run, Seconds, Safe, MaxHealth, NewLives)`, which atomically
restores full health and pre-mints the next kill ID. The shorter helper's returned
lives require HP/ID completion on the same unpublished candidate; do not save or
spawn actors directly from that intermediate result. No saved-shape change needed.
Overflow or unresolved health aborts without appending NewLives. Safe callback
implements player-distance/view/nav/floor/capsule checks with the catalog safety
radius; no geometry decision is faked in this domain. Zero elapsed time does not
respawn a slot. Travel stores remaining seconds; off-floor/paused/closed time is zero.

Initialize `RNG.Loot` at run creation in W4-06, using `UE.FRandomStream`, revision 1,
four little-endian bytes of the authority seed. Missing/unsupported stream rejects
settlement; no restart reseeding or callback-local RNG. Each finalized corpse uses
one bounded gold draw, an item event draw when the table is nonempty, a weighted
selection and quantity draw on success. Persist updated seed and contents together.
For the ledger's uniform listed-item interpretation, W4-02 supplies equal weights;
for midpoint guaranteed gold, supply equal GoldMin/GoldMax. This is an explicitly
Prototype interpretation, not reconstructed historical loot probabilities.

## Reward and entity identities

D06 uses canonical `LHReward1` SHA-256 and first 16 digest bytes mapped to four
little-endian FGuid words. Helpers reject invalid source tuples; collisions reject.

| Helper | SourceKind / purpose | SourceKey |
|---|---|---|
| EnemyLifeRewardId | EnemyLife / KillSettlement | Area, SpawnSlot, LifeGeneration |
| GrowthRewardId | Growth / LevelGrowth | CharacterId, ToLevel |
| QuestTurnInRewardId | Quest / QuestTurnIn | QuestId, Stage |
| BossUniqueRewardId | Boss / BossUnique | BossId, Objective=`PermanentObjective` |

All include RunId. Quest/boss source-key fields are in ASCII order. Boss death and
completion observers use the same BossUnique claim as the defeat entitlement;
quest completion rewards may use a distinct QuestTurnIn claim. Later boss lives
award ordinary XP/loot but preserve defeated/mark/dialogue/unique state. W4-05 must
not conflate defeat claim with a second cash/XP completion payout: keep the quest
reward and its turn-in claim separately, as appropriate to the authored objective.

`CorpseContainerFor` hashes UTF-8 `LHCorpse1:` + the enemy reward GUID's 32 hexadecimal
digits (Unreal Digits format), then maps the digest with the same GUID mapping.
One drop item uses distinct `LHCorpseItem1:` prefix. These are entity identities,
not reward claims. Partial item transfer retains the source ID and mints the
inventory split ID from `LHPartialLoot1:` + epoch Digits + request Digits. Full
transfer retains original item origin/identity across travel. Collisions reject.

## Loot, cleanup and death

`ExecuteTakeLoot` accepts only finalized, unclaimed corpses in the active area and
current run, with alive player, valid payload kind, available positive quantity,
catalog item and checked gold arithmetic. It works on a copy; inventory capacity
failure leaves both sides intact. Gold does not consume slots. Partial stacks use
distinct IDs; source/destination are changed together. `bClaimed` means no items
and zero gold. W4-06 validates actual reachability/LOS/200cm range before dispatch:
a snapshot contains no player/corpse positions. Source item identity area is never
changed to the active area.

`AdvanceLootCleanup` takes the same active clock and caller predicates for
unique/quest items and in-flight container references. Protected contents and
referenced containers do not expire. Ordinary contents expire at the persisted
300s timer; claimed/unreferenced records can be retired. Encounter generation and
reward high-water remain, so cleanup cannot re-award XP. Capacity stops settlement
at 1024 corpses until safe cleanup; never evict protected content. An empty loot
list must be explicitly authored; dash evidence is not proof of guaranteed empty.

`SettlePlayerDeath` requires snapshot CurrentHealth==0, a resolved registry-matching
recovery checkpoint, and explicit safety-reviewed evidence from W4-06. Set active
entrance to recovery, store Session.SafeRespawn and restore derived full HP/MP.
Inventory/equipment/gold/XP/debt/earned level/pools stay unchanged. Positive restored
HP is the snapshot idempotency boundary; duplicate death callbacks reject. W4-06
must deduplicate avatar callbacks and must not set HP to zero again for a stale
callback after acceptance. Respawn avatar only after the completed death snapshot
has been saved; report save failures and retain the canonical inventory owner.
The boolean is evidence from automated arrival review/runtime marker, not user or
UI approval. Do not treat fixture `true` as real map safety evidence.

## Receipt wrapper

`LHSave::BeginRequest(S, Command, exact StaticStruct, &Request, Result, Digest)`
returns true only for a fresh legal request. False means rejection **or accepted
replay**: return Result without executing domain work. Epoch mismatch rejects before
lookup. Same ID/digest replays original sequence with bReplay; changed digest returns
ReusedRequestId. At 4096 receipts or exhausted sequence return Busy. W4-06 performs
safe epoch rotation only at a completed, durably saved boundary with no outstanding
intents. Never evict receipts or manufacture new retry IDs.

On true: copy S, execute domain, validate/import completed candidate, call
`CommitRequest(Candidate, Request.Request, Digest, Result)`, and replace S only if
Result is accepted and not replay. Save/publish through existing wrapper. Commit
rechecks epoch, digest, sequence and cap. Header sequence starts positive for existing
characters; bootstrap creation may start at zero. The allowlist now encodes all
fields for TrainSkill/LearnSpell/BuyItem/SellItem/UseAbility/Interact/TakeLoot, ordered
by ASCII names, enum Kind as symbolic Item/Gold. Exact command/type mismatch rejects.
UseAbility's digest helper does not authorize per-swing durable saves; D16 remains.

## Evidence and prototypes

Values are input from R-03 and the ledgers, not newly researched by this worker.
Balork 15:00 = 900s is documented by <https://www.t4cbible.com/monster1>, retrieved
2026-10-09, live Original Content, server/version unstated. Its active-clock origin
is Prototype. Matt's 2026-10-09 recurring boss decision supersedes the planning
`prototype-policy.json` boss_respawn=false; completion remains single-claim.

R-03 explicitly marks ordinary respawn, safety distance, loot probabilities,
cleanup and post-death pools missing. Existing labelled recommendations: ordinary
120s, safety1000cm plus outside view/relevancy, item event10% with one uniform listed
item, guaranteed midpoint gold (missing Balork gold Prototype5), corpse300s,
full HP/MP reviewed church recovery, no-development-penalty. Replace these in the
catalog/economy/death policy after normal-play review. Only corpse300s and full
recovery are authored inside this domain; other values are supplied specs.
No historical death-loss percentages, bat-wing quest or monster parity claim.

Native tests use the existing synthetic Wave2 profile and fixture reward XP10/gold2,
drop100%/unit quantity solely to expose state invariants. RenewableRatRoute uses the
actual B1 registry's12 rat/3 bat/2 slime slots over two lives to reach15 rat kills and
30 gold, not an economy or Bible-profile gate. W4-06 repeats with W4-02 catalogs and
normal gameplay. See attempt report for observed build/test counts and limitations.

## Observed candidate checks

UE 5.8.3 Linux Development editor and game targets each reported `Result: Succeeded`.
The final headless `Lighthaven` run reported101 tests:98 success,1 success with
warnings (`Lighthaven.Abilities.DeadTarget`),2 failures,0 not-run. All10 rewards and
22 persistence tests succeeded, including both new persistence tests. The failures
were `Lighthaven.Integration.Wave3.ArrivalSafety` and `Lighthaven.World.LightingAudit`:
all five playable `.umap` files are baseline LFS pointers, so world loading failed.
No map safety, cook/package, actual combat/death play or real-data economy pass is
claimed. Full logs, JSON and independent seven-vector digest evidence are in the
attempt's worker-output library. The first two automation runs crashed in an early
test fixture due to positional indexing after canonical set reordering; fixtures
now resolve areas/slots by stable identity across reload.
