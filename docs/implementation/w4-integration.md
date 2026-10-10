# W4-06a4 — Stage 1A integration candidate

Contract revision 1; base `a1796cc5d8318ab8832e5fbdb2ea661ec603b985`.
This submission is evidence for coordinator review, not a Stage 1A/G4 release assertion.

The production session now dispatches melee UseAbility to W4-03, Interact to W4-05,
TakeLoot to W4-04, and UseItem to the real rev-2 domain. Persisted requests use one
BeginRequest → candidate/domain → character Import → CommitRequest → owner swap
→ queued save path. Runtime attacks use only a session-local request replay latch;
no durable activation receipt, sequence increment, or save per swing. Kill settlement
advances sequence once, includes the quest observer, and initializes/persists loot RNG.
Queued/in-flight saves do not freeze movement or melee. Persisted commands and travel
still wait for durability. Saves coalesce at completed boundaries; exit captures live
resources even when no inventory/quest command occurred. Retry retains character identity.

The session populates B1 from its 17 registry markers, resolves native enemy specs,
settles deaths once, persists life generations, ticks loaded/unpaused respawn timers,
uses director distance/visibility/nav/floor/capsule safety, and captures live encounter
health. Dialogue, loot, pause, death and travel stop active simulation/recovery.
Remaining loot rehydrates as native corpse actors, including after reload; old-life
corpses remain after the next life spawns. Cleanup uses the W4-04 300s policy for gold/
torches; other item IDs are conservatively protected until their quest/unique policies
are reviewed. UI pauses while inspecting a container, preventing expiry during a transfer.

Death publishes a zero-HP checkpoint and the death UI. Respawn requires durability and
the existing hash-matching automated church review, settles W4-04 full-pool recovery,
saves, then opens the church map. Inventory/equipment/XP remain intact. Physical arrival
still goes through the existing capsule/marker checks. B2–B4 gameplay travel and service
commands remain unavailable in this Stage 1A submission; their catalog definitions are
included for closure and validation, not a claim of a playable Stage 1B route.

B1 Nevanis/Shovanis are ALHInteractableMarker actors at the original grid positions
(-25,6,0)/(-25,14,0), multiplied by the existing 100cm conversion. Literal instance IDs:
`32a8de01573448aa9fa6de27056614c2` and `c451f507b1d442788b34a5d097516bc8`.
Definition, area and GUID join the deterministic generator fingerprint. No registry
arrival, geometry or lighting change. No binary map or ReviewedArrivals.tsv is submitted.
The placement validator requires each quest/service-referenced NPC exactly once in its
catalog area and checks enemy/encounter boss/respawn agreement, including Balork B4,
OrdinaryRepeat and 900s. Existing cross-kind GUID checks remain in force.

## Mechanics and compatibility

Numbers use retained R-03 and rules/world ledgers, with no fresh web retrieval.
Starting HP30/MP10/gold100, level1/XP0/pools0, Attack10/Dodge10/no spells, kit Dirk1,
vest1/pants1/torches3 are explicitly Prototype because R-03 marks classic grants missing.
Six individual items are automatically equipped where a slot exists. Item mechanics,
known requirements and damage use W4-02 provenance. Slot capacity40 is an authored
Prototype (missing slot limit); do not infer historical backpack capacity.

Four synthetic answer IDs select a fixed all-16 finite creation outcome, zero unspent
creation points, total80, maximum22. These are an explicit Prototype selection policy
for the representative four-N/A chart, not a recovered RNG/question distribution.
Chart80/16: <https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html>,
retained retrieval2026-10-07; maximum22:
<https://web.archive.org/web/20020602203835/http://www.t4cbible.com:80/start.html>,
retained2026-10-07. Per-level grants5/15 keep the existing confirmed getting-started
provenance. All200 cumulative classic Required XPs rows are consumed from MakeBibleRuleset;
classic/PDF disagreement remains in provenance notes, with explicit Modernized selection
so the runtime does not silently reject Disputed rows. Source:
<https://web.archive.org/web/20021211182332/http://www.t4cbible.com:80/xp.html>,
retained2026-10-07. MaxGrowthAwards is table size minus one.

Ruleset `Ruleset.Stage1Prototype` rev4 and `LHGameplayCatalog4` reject G3-era saves
as accepted by Matt. Closure includes profile, world registry, native item/combat data,
enemy/reward/AI rows, encounter policies, abilities, service offers, quest and runtime
policy versions. Rules schema remains rev2; no migration pretending compatibility.

Nevanis full HP/free/no MP, NPC250cm LOS, corpse200cm LOS and post-death full pools are
R-03 missing-field Prototype proposals. Healing identity:
<https://www.t4cbible.com/ArakasQuest>, retrieved2026-10-09, live version unstated.
Rat quest15/2500 uses W4-05's inherited secondary ledger S5
<https://t4cfantasy.com/Bible/Classic/QuestAR.php>, retained2026-10-07; no Bible numeric
rat row was recovered by R-03. Catalog enemy source values/disputes and authored combat
model remain as documented in enemy-data.md and abilities.md; no play authenticity claim.

## Required domain follow-up / known limitations

**Rules/Character owner blocker:** the existing profile exposes only FLinearFormula.
The adapter currently evaluates HP floor(7+END/20), MP floor(4+INT/30+WIS/60), no growth
random term. HP matches the proposed midpoint within covered END; MP differs from
4+floor(INT/30)+floor(WIS/60) when fractional components add to one. At the initial16/16
and first Stage 1A level it agrees, but it is not acceptable as a general Bible band
adapter. There is also a legacy fixed100 carry-capacity adapter, rather than the known
STR×500/(STR+100) law. Neither formula replacement is authorized as authentic. Before
full progression approval, Rules/Character must support band growth and nonlinear
capacity, with Import replaying historical growth under the same frozen law. Those
paths are outside this worker's write scope; they were not patched. No schema change
is needed for a native formula-policy extension, but the catalog/ruleset must change.

Stage 1B must wire services and earned ranged/spells, all deeper floors, Balork return,
economy route checks, targeted HUD feedback and completion notices. HUD currently
provides live player pools, melee availability, quest count, save/death status, dialogue
and loot rows; no live selected-target health seam is supplied by the read-only W4-05
controller. Receipt/runtime-intent capacity4096 rejects Busy rather than rotating an
epoch with outstanding UI intents; coordinator/domain design is needed before a long
soak can claim indefinite sessions. A rejected director kill is logged and latched by
W4-01; automatic failed-settlement recovery is not provided by that interface. Saved
corpse positions use the registry anchor on reload (no saved death-position field),
matching the available schema, not the exact lethal position. Native NPC markers have
no newly authored character visuals. Physical reachability, ordinary respawn safety,
combat balance, simultaneous lethal events and save-failure play remain host checks.

## Stage 1A host checklist

- [ ] Build editor/game on the integrated revision; regenerate all five maps with existing generators; do not commit worker-generated binaries.
- [ ] Re-run automated arrival review on the host and LHValidateWorld (zero errors), including the new B1 NPC GUIDs and unchanged arrival hashes.
- [ ] Fresh character → church: kit/equipment, HP30/MP10/gold100, Attack/Dodge10, no debug gear; legal melee available with zero gold.
- [ ] Samaritan accept → B1 → legal rat kills → gold/item loot → Nevanis heal (free/full HP/no MP).
- [ ] Kill15 eligible rats using real120s loaded/unpaused respawns and safe placement → Samaritan turn-in2500 XP once.
- [ ] Quit/reload after creation, acceptance, each kill/loot/heal, death, respawn and turn-in; no duplicated XP/gold/claims/lives. Reload remaining corpse loot.
- [ ] Damage/mana/fraction/cooldowns survive travel and quit; no extra regeneration tick. Pause/travel stop AI and all clocks; save writes do not freeze movement.
- [ ] Die → durable death UI → safe church respawn; inventory intact. Inject save failure, retry, stale callback and simultaneous lethal hits.
- [ ] Refuse floor change with a pending action; inventory-full transfer rolls back; repeated requests replay. Packaged Linux run with screenshots/logs retained by coordinator.

## G4 host checklist (06b; deferred)

- [ ] Fresh melee, ranged and magic builds through real training/vendor/spell offers; unlimited stock and source-positioned NPCs, no debug grants.
- [ ] Real normal-kill budget funds bow+quiver and first damage spell; zero-gold melee recovery and mana potion UseItem replay work.
- [ ] Full B1–B4 descent/return, doors/stairs/largest enemy clearance, blocked spawn and pause/unload timers.
- [ ] Balork kill → 900s active respawn → second kill → reload/travel → Brother Kiran return; one unique completion claim; arrival grants nothing.
- [ ] Death/respawn, mid-action floor change, full inventory, simultaneous lethal hits, save failures/repeated interactions, exit/resume on every floor.
- [ ] Packaged Linux gameplay validation; Windows remains deferred. Neither checklist is passed by source compilation.

## Worker validation

Final commands, exit codes, durations, counts and evidence are appended after execution.

Read-only regression follow-up: `Lighthaven.Data.RuntimeSpecsValidate` appends the
entire item catalog to PrototypeProfile.Items, now already populated, so Initialize
rejects duplicate IDs (all11 fixture setup assertions fail). W4-02/Data test owner
should use a dedicated synthetic fixture or stop appending duplicates. W4-04 Rewards
fixtures also use the production profile: FullInventoryLootIntact assumes the old
2-slot inventory, PlayerDeathSettlesOnce expects20 HP, and RenewableRatRoute expects
zero starting gold. Their owners must isolate fixture tuning or update assertions;
this worker did not edit those out-of-scope paths. Owned Wave2 allocation fixtures
now earn the Bible first-level1000 XP before spending points, and use Dirk rather
than the removed free test bow/quiver; no existing integration tests were deleted.

Automation limitations: the live melee fixtures invoke the real session and GAS attack,
but reset cooldowns between swings to avoid real-time waiting in a transient world.
The errand fixture advances the real respawn transaction by120s with a synthetic safe
callback; it does not certify playable navigation/safety or normal-cadence balance.
Resource/death tests inject live GAS pool values and exercise boundary capture/recovery;
they do not establish a packaged enemy-damage/spell/death route. Enemy cooldown/RNG
continuation on reload is not integrated; the director's runtime constructor resets
those per life. The selected-target and Light-effect restore seams remain Stage 1B work.

Observed final-source checks (UID1000, UE5.8.3 Linux):

- `UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game`: exit0,
  11.100s shell time; editor/game each `Result: Succeeded` (6.78/3.33s UBT).
- Direct headless `Automation RunTests Lighthaven; Quit`, worktree XDG_CONFIG_HOME,
  memory DDC/nullrhi/unattended/nosound/nop4, HomeScreen disabled, export
  `Saved/AutomationFinal`: exit255,81.059s. Expected/observed151 completed tests:
  142 Success,5 SuccessWithWarnings,4 Fail,0 unfinished. All five new Wave4 tests
  succeeded, as did updated Wave2/3 integration and world pairing tests. Failures
  are exactly the four read-only Data/Rewards fixture regressions identified above.
- Direct generator commandlets saved hub/B1/B2/B3/B4 locally (each commandlet result0,
  process1 from startup pointer/cache diagnostics); no maps submitted. An initial
  build overlapped source edits and retained an old basement-A generator object.
  Its initial B1 map/validator result was discarded. A touched generator source and
  `Build.sh LighthavenEditor Linux Development -Project=... -WaitMutex -NoUBA`
  explicitly recompiled it: exit0,9.451s (9.25s UBT). This was a build/output issue,
  not a source marker defect. The final basement-A commandlet used the flags below,
  exit1,15.794s, commandlet result0, and passed B1/B2 floor/route/stair controls.
  Final B1 fingerprint `5439d409319ff27d478785b16d52bf1e`; B2
  `3a1ebf5c804d33ad1f13a798c4509315`. The corrected map contains both NPC definitions
  and LHInteractableMarker; the old B1 fingerprint is not final evidence.
- Final direct `-run=LHValidateWorld` on corrected five maps:21.149s, process1,
  **commandlet result0**, five-map world identity/portal/entrance validation passed,
  no placement errors. Nonzero process status remains from unrelated LFS-pointer
  Dev_Combat/Dev_Movement/L_Frontend packages and startup cache diagnostics.
- Final direct `Automation RunTests Lighthaven.Integration.Wave3.ArrivalSafety; Quit`
  after corrected B1 generation: exit0,32.473s; expected/observed1 completed test,
  SuccessWithWarnings,0 Fail. All nine IDs PASS, including capsule/floor/nav controls.
  This writes only Saved evidence; Config/Lighthaven/ReviewedArrivals.tsv was not
  updated or submitted. Coordinator still refreshes the reviewed list via its script.
- `git diff --check` and submission-script `bash -n`: exit0.

Final generation/validator/arrival commands use UnrealEditor-Cmd and worktree project,
`XDG_CONFIG_HOME=$PWD/Saved/BuildEnvironment/config`, `-DDC-ForceMemoryCache -nullrhi
-unattended -nosound -nop4 -ini:Engine:[ConsoleVariables]:HomeScreen.EnableHomeScreen=0`,
plus `-ddc=InstalledNoZenLocalFallback -LocalDataCachePath=$PWD/DerivedDataCache` to
avoid the sandbox's unwritable Zen/home cache. Timings use Bash TIMEFORMAT, not an
installed /usr/bin/time (that utility is absent). Full command/log/timing/report
artifacts are in this attempt's worker-output library.

No cook/package, visual judgment, OpenLevel gameplay traversal, normal-cadence combat
balance, real disk/process crash, controller hardware or Windows checks were run.
They require coordinator host execution with regenerated/hydrated frontend/content
and the Rules/Character corrections. The candidate is submitted with these blockers;
no release/G4 success is asserted.

# W4-06b — G4 integration candidate

Base `4e69d68a3c0de8008fec88b2f9caf394782a5bda`, contract revision 1.
The Stage 1A observations above describe the previous attempt, not this revision.
This section supersedes its B1/melee-only runtime restrictions; neither host checklist
is marked passed by this candidate.

Train/Learn/Buy/Sell now resolve the actual GUID-bearing NPC and dispatch the native
service domain through Persist (receipt replay before spatial/resource checks, live
resource capture, temporary Import, CommitRequest, swap, queued durable save). Offer
views expose native prices and one-unit intents, and only resolved sales of unequipped
items. Unlimited vendor stock remains unpersisted. HUD lists the native ability catalog,
learned spell availability, equipment-dependent bow/melee availability, selected enemy
pools and Balork completion notice. The command domain still enforces cast requirements,
mana, range, LOS and cooldown. Self-target Light/Heal resolve to the player combat owner.
All registry floors populate their actual encounter slots; gameplay travel uses the
existing reviewed portal/arrival path without the Stage 1A destination filter. Balork
continues through the existing quest observer, OrdinaryRepeat 900s lifecycle and unique
boss claim; only Brother Kiran's return topic completes the objective. Arrival does not.
Existing generator NPC positions/identities satisfy service placement: no generator,
lighting, entrance geometry or binary map change is included.

Catalog closure is LHGameplayCatalog5 and runtime policy explicitly includes service,
all-floor ability routes and enemy continuation. Earlier catalog saves are incompatible;
no migration is claimed. Character formulas are unchanged by instruction: HP linear
floor(7+END/20), MP floor(4+INT/30+WIS/60), capacity100. Matt's MP-band/nonlinear STR
policy must land in Rules/Character, replay historical growth using the frozen law,
then update ruleset revision and closure; do not reinterpret saves under another law.

Enemy combat RNG now captures in existing GameplayRng records as
RNG.Enemy.S<spawn-GUID>.Life<life-generation>, UE.FRandomStream revision1/4-byte state;
loaded enemies restore this and their owner-scoped cooldown map at hydration. New lives
use the constructor seed and no prior-life remainder. One RNG per slot replaces older
saved life state when captured. Off-floor records do not tick. AI chase/decision state
and positions remain reconstructed; this is combat RNG/cooldown continuation only.
Light presentation is a native caster point light, 600cm Prototype radius from R-03;
3000 intensity is explicitly Prototype presentation tuning pending visual review.
No generator lighting code was changed.

## Required follow-ups / limitations after 06b

- Light duration restore remains blocked by the read-only Persistence validator, which
  rejects every DurableEffects entry. Proposal to W4-03/04: allow Effect.SpellLight with
  owner, remaining active seconds, stacks1, no modifiers; expose a validated restore
  accessor in combat; include policy in closure. Migrate empty effects as no Light.
  Current Light expires on world time and clears with avatar/travel; pause-duration
  semantics need domain review. This candidate does not encode duration as a cooldown
  or mint an unapproved durable effect.
- Corpse reload still uses registry anchors. Proposal to Core/Persistence/Rewards:
  add a canonical finite DeathTransform to corpse records, capture lethal actor position,
  schema/content migration or rejection for old saves, validate against area bounds,
  hydrate at that transform. No diagnostic/RNG field is repurposed as a transform.
- Capacity4096 Busy retains all receipts and runtime activation latches, preserving replay.
  Session status now explains runtime save/reload recovery or the durable epoch-policy
  blocker. Runtime reload resets its transient latch; durable receipts remain full after
  reload. Coordinator/UI/Persistence must agree an explicit epoch rollover with stale
  intent invalidation and completed-boundary durability before indefinite soak approval.
- Balork completion UI reads bCompleted (the quest domain's field); unique boss claim is
  granted on first defeat, while church return records completion with no second numeric
  reward. No invented return XP/gold/item is added.
- Physical reachability, spawn safety, largest enemy clearance, ordinary-cadence economy,
  simultaneous lethal/save failure play, target readability and Light appearance remain
  host checks. The older Data/Rewards fixture notes above are historical; those tests passed in this attempt.

## 06b checks and evidence

Appended after final-source validation. Acceptance fixtures earn budgets with real
Brown Rat specs, live session melee settlement and corpse gold transfers; they reset
cooldowns and advance the real respawn transaction with a synthetic safe callback.
NPC actors are synthetic at player range, not physical map reachability evidence.
Potion fixture spends live mana by injection to isolate replay/consumption; no claim
of a normal-cadence magic/potion playthrough. Balork fixture resets cooldowns and
advances899+1 seconds, not a real fifteen-minute wait or physical safe-spawn check.

G4 host checklist above remains required: regenerate/review/validate five maps, real
training/purchases/all-floor traversal, Balork repeat/reload/return, crash/save failures,
Linux packaged play and screenshots/logs; Windows deferred. Stage 1A checklist remains
required as regression coverage. No gate is certified by this source candidate.

### Production service blocker (W4-07 owner)

Source evidence: `Services/LHServiceAuthority.cpp` Ready(FLHInteger) accepts only
Confirmed/Prototype; Execute(BuyItem) applies it to the item StackLimit. Every native
item in `Data/Items/LHItemCatalog.cpp` intentionally uses Modernized StackLimit1
(individual-instance slice inventory policy). Thus the real bow/quiver/potion purchases
return UnresolvedRules before cost/commit. Dedicated Services tests use synthetic
Prototype stack limits and do not catch this production cross-domain mismatch.
Do not relabel the native stack limit or grant debug items in the session to evade it.
W4-07 must accept the reviewed Modernized inventory policy consistently (while rejecting
Missing/Disputed), or coordinate another explicit policy with W4-02. These are read-only
paths in this contract. Production purchase tests remain failing evidence. Services,
ranged acquisition and potion progression cannot be declared playable until this lands.
Further dependent gameplay work stops at this ownership checkpoint; this candidate is
submitted as instructed, with G4 blocked. Service fixture guards prevent null-item
crashes after a failed purchase. Earlier exploratory crashes were fixture defects,
not evidence of an observed production crash.

Save/transaction/exit/travel capture now waits for all loaded-floor enemy and player
pending/publishing actions, rather than only the player. Movement/combat still runs
while saves queue. This enforces a completed combat boundary; prolonged continuous
combat can defer durability until a quiet boundary or pause cancels enemy windups.

Observed 06b runtime-source validation (UID1000, UE5.8.3 Linux):

- Direct Build.sh editor/game Linux Development, worktree project, -WaitMutex -NoUBA,
  XDG_CONFIG_HOME=$PWD/Saved/BuildEnvironment/config and UBA_ROOT=$PWD/Saved/UBA:
  both exit0, Result:Succeeded. Runtime-final editor UBT53.89s; game UBT101.93s.
  Final Balork-fixture-only editor rebuild exit0, UBT37.80s, shell38.380s.
- Full headless Lighthaven suite before the final reload-fixture Bind correction:
  expected/found/completed156;145 succeeded,5 succeededWithWarnings,6 failed,
  zero notRun/inProcess. Exit255, shell106.459s, report testDuration62.023s.
  Failures: Wave3.ArrivalSafety and World.LightingAudit (all five local .umaps are
  unhydrated LFS pointers); Wave4.ProgressionRouteAffordable, ServicesThroughSession,
  UseItemThroughSession (production purchase UnresolvedRules, reason3); and
  Wave4.BalorkSingleClaim (return/replay fixture remained travel-frozen after Continue).
  The final fixture calls Bind before the new avatar, matching actual arrival binding;
  its targeted rerun is appended below. All Data/Rewards tests passed, superseding the
  older four-fixture regression notes. Both200-rat budgets reached the purchase seam.
- Final suite command: UnrealEditor-Cmd worktree Lighthaven.uproject,
  -ExecCmds="Automation RunTests Lighthaven; Quit" -DDC-ForceMemoryCache
  -ddc=InstalledNoZenLocalFallback -LocalDataCachePath=$PWD/DerivedDataCache
  -nullrhi -unattended -nosound -nop4
  -ini:Engine:[ConsoleVariables]:HomeScreen.EnableHomeScreen=0
  -ini:EditorSettings:[/Script/UnrealEd.AnalyticsPrivacySettings]:bSendUsageData=False
  -ReportExportPath=$PWD/Saved/Automation06bHandoff.
  The last override is process-local: a preceding run was interrupted by sandbox
  rejection of Unreal telemetry to datarouter.ol.epicgames.com, before report completion.
  No global/system privacy setting was edited. Earlier default UBA runs failed with
  transient compiler-input nulls and read-only home cache; NoUBA actions with worktree
  config/cache succeeded. Initial in-progress header edits also invalidated UHT output;
  the completed final rebuilds supersede those exploratory failures.
- git diff --check and submission script bash -n: exit0.

No map generation/arrival-review refresh/LHValidateWorld, cook/package, graphical play,
visual review or Windows execution in this attempt. Coordinator must hydrate/regenerate
maps before the two map-dependent tests and host checklists can be evaluated. Only the
native runtime and synthetic transaction boundaries were exercised here. Library logs
and automation JSON are retained in the attempt worker-output, with report.md handoff.

Final-source targeted `Lighthaven.Integration.Wave4.BalorkSingleClaim` rerun with the
same headless flags/analytics override and export `Saved/Automation06bBalorkFinal`:
expected/found/completed1;0 succeeded,1 succeededWithWarnings,0 failed,0 notRun/inProcess;
exit0, shell44.174s, testDuration0.544s. This supersedes the full-suite Balork fixture
failure. Two live kills,899+1s respawn, one unique claim, reload, church return and replay
assertions passed. Remaining observed failures are the three production purchase seams
and the two pointer-map-dependent checks. No combined hypothetical pass count is claimed.
