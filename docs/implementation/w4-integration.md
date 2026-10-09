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
