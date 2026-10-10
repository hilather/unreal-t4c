# W6-03 adversarial review

Review baseline: `68bb29e28f27b886326fe2ef34261bcc2aea4935`, contract revision 1, 2026-10-10. Findings only: no fixes, production source changes, committed tests, assets or gate approvals. Linux first; Windows deferred. The supplied attempt baseline governs this review even though the status ledger's launch row names an earlier main revision. Severity describes impact; a source proof is distinguished from an executed runtime observation. Source references below are relative to `Source/Lighthaven/` unless explicitly prefixed otherwise.

## Findings

### F1 — Major: changing floors clears the player's cooldown and can restore a stale one on return

Owner: Framework/Abilities travel integration.

Reproduction: finish an attack in B1 while a positive cooldown remains, request the nearby B1→hub portal at a completed action boundary, and install/start the hub avatar without advancing active simulation. The capture contains owner `(run, Area.TempleB1, character GUID)` and its remaining attack cooldown. Destination restoration looks for `(run, Area.LighthavenTempleDistrict, character GUID)` and restores an empty map. Expected: the same player's cooldown remainder survives travel. Actual: zero; the saved B1 record remains. Travel back to B1 can restore that old remainder even after spending active time elsewhere. This is an identity mismatch, not offline clock advancement.

Proof chain: `Framework/Wave4/LHStage1Session.cpp:61–65` constructs PlayerEntity from ActiveEntrance.Area; `:74` captures under that identity; `:219` restores with the destination identity. `Framework/LHWave2Session.cpp:294–308` remaps only Light, not cooldowns. `World/LHTravelCoordinator.cpp:56` changes ActiveEntrance. `Abilities/LHResourceRecovery.cpp:16–28` removes/matches records by full owner including area; `Abilities/LHCombatComponent.cpp:199–203` clears the component map before installing the matched records. No world-time wait can repair a record that was never matched.

Temporary headless probe `Lighthaven.Review.W603.TravelCooldown` isolates this exact capture/install/restore seam with a component remainder of 1.25 seconds; it does not claim to walk a portal or earn an attack. Execution evidence is below. Smallest fix: keep the player's owner identity stable as D12 specifies (hub origin), or explicitly remap only this player's cooldown records on area transitions with collision handling. Retest actual session travel, reload at destination, and return after active time; preserve enemy owner scoping.

### F2 — Major: production save compatibility accepts undefined learned skill/spell IDs

Owner: Framework reference validator / Character catalog validation.

Save-input reproduction: start with a valid newly created current-catalog snapshot. Append `Spell.DoesNotExist` to LearnedSpells and `{Skill.DoesNotExist, resolved trained value 999}` to LearnedSkills; retain the original rules/content hashes. Encode through the production codec (which recomputes integrity), then Decode with `FLHWave2Session::Compatibility()`. Expected: authoritative reference validation rejects missing definitions with InvalidSnapshot, leaving the selected output unchanged and permitting A/B fallback. Actual: both unknown IDs survive production Decode.

Evidence: `Framework/LHWave2Session.cpp:15–26` delegates to world and Character Import; `Character/LHCharacterAuthority.cpp:446–460` checks nonempty/unique names and nonnegative ranks but performs no catalog lookup. `Persistence/LHSaveValidation.cpp:113–114` checks only skill/spell syntax and values. `World/LHAreaStateSubsystem.cpp:8–59` checks area records, not learned catalogs. `Abilities/LHAbilityCatalog.cpp:48` has no definition for the inserted spell. Compatibility hashes identify the catalog but do not validate individual references.

Temporary probe `Lighthaven.Review.W603.UnknownReferences` uses actual session creation and production Encode/Decode, not a permissive fixture callback. This is semantic corrupted-save acceptance, not a claim that arbitrary byte damage bypasses SHA256 or that local saves are anti-cheat. Smallest fix: resolve learned IDs and canonical spelling against the enabled skill/spell catalogs inside the production callback. Extend that audit to quest/boss/cooldown/object/contained-item references; their current syntax-only checks are coverage gaps, not separately demonstrated runtime defects here.

### F3 — Major: a semantically inconsistent Balork claim loads and then prevents kill settlement

Owner: Framework/Quests/Rewards save reference validation.

Save-input reproduction: in a valid current-catalog run with populated B4, put a boss row `{Boss=Enemy.Balork, bDefeated=true, UniqueClaim=(1,3,5,7)}` into World.Bosses while ClaimedUniqueRewards remains empty. Encode and Decode with production compatibility. Both succeed. Settle the alive Balork encounter life through `LHRewards::SettleKill` with its real catalog reward and current character profile. It returns InvalidRequest. Expected: reject the inconsistent save before enabling gameplay, so fallback/recovery can operate. Actual: readable malformed boss state reaches play and settlement rejects after combat kills the actor.

Evidence: `Persistence/LHSaveValidation.cpp:166–172` never validates UniqueClaim against bDefeated, the deterministic BossUnique ID or the retained unique set. `Framework/LHWave2Session.cpp:15–26` and `World/LHAreaStateSubsystem.cpp:8–59` add no such check. `Rewards/LHEncounterLifecycle.cpp:140–148` requires that relationship only during the next kill. `AI/LHEncounterDirector.cpp:111–119` marks the actor a corpse and latches PublishedDeaths before rejected settlement, then only logs the error. This can strand the Balork objective for that loaded floor/run rather than presenting save recovery.

Temporary probe `Lighthaven.Review.W603.InconsistentBossClaim` demonstrates Decode acceptance and subsequent domain rejection; it does not claim an opposed combat kill. Smallest fix: validate boss claim relationships at the authoritative load boundary (and matching quest turn-in facts), without reconstructing claims or granting rewards during load. Retest both inconsistent directions and legitimate later-generation Balork kills. Failed kill recovery is already a documented integration limitation, not a request for a separate broad rewrite.

### F4 — Minor: accepted travel returns an invalid reason and loses request correlation

Owner: Framework command handling.

Reproduction: submit a well-formed in-range portal request whose configured travel adapter accepts it; inspect the returned FLHCommandResult. Expected: Request equals the submitted ID and an Accepted result has Reason=None. Actual: Accepted with Reason=InvalidRequest and default invalid Request. `Core/LHCommands.h:25–29` supplies those defaults; `Framework/LHWave2Session.cpp:310–323` changes Disposition but never assigns Request or clears Reason, including on rejection paths. This does not prove duplicated rewards, but violates the command result contract and prevents consumers from correlating travel outcomes.

Temporary probe `Lighthaven.Review.W603.TravelResult` uses a real session/portal/spatial check and an accepting travel callback to isolate the result construction. Smallest fix: initialize result.Request before every return and set Reason=None on acceptance. Preserve asynchronous travel durability semantics; do not invent a committed sequence before durability completes.

## Coverage and sound paths

The following are source-review conclusions unless an executed check is specifically listed below. They are not package/play or full-route claims.

- **Grant paths:** production calls to GrantExperience are kill settlement and Samaritan turn-in on detached authority/snapshot candidates; AddItem is used by corpse transfer. Service buys/sells/training/learning edit snapshot copies and only publish through the session Persist wrapper (`Framework/Wave4/LHStage1Session.cpp:103–121`). Creation's starter gold/items and post-create Attack/Dodge grants belong to its one latched creation transaction; no UI gold/XP/item/knowledge setter was found. Trusted authority helpers remain public native seams, not authenticated UI commands; their presence alone is not a demonstrated bypass. No Python rules implementation was introduced.
- **Durable replay/double submit:** BeginRequest validates epoch, compares digest, returns the original sequence for identical requests, rejects changed payloads, and stops at 4096 rather than discarding receipts (`Persistence/LHRequestReceipts.cpp:29–63`). Character creation/allocation/equipment use the equivalent authority Begin/Commit logic. Persist checks replay before spatial/resource/domain mutations. UI confirmation latches accepted creation; RetryCommand retains the captured request. Runtime UseAbility has a separate session-local replay latch under D16, not a durable reward receipt. Cross-kind runtime/durable ID collision behavior was not exercised. Receipt exhaustion/epoch rollover remains the previously documented limitation, not evidence of safe indefinite operation.
- **Kill and loot coherence:** stable EnemyLifeRewardId, current generation/state and bRewardCommitted reject duplicate/lower-life settlement; RNG and finalized loot are updated on one snapshot copy and observers run before validation/publication. Loot subtracts source/adds destination atomically and publishes only after validation; inventory-full rejection leaves the candidate uncommitted. Existing domain tests are relevant but do not substitute for an earned inventory-filling route.
- **Balork legitimate state:** first defeat adds deterministic BossUnique once; later life kills require the same retained claim, do not append it again, and ordinary kill XP/loot are intentionally repeatable. Quest observer does not reopen completed BalorkReturn; Kiran completion topics disappear after completion. Generation changes do not clear permanent claims. Reload/travel copy those facts rather than minting claims. F3 concerns malformed state, not legitimate recurring reward policy. Real nav-safe second-life combat remains unobserved in this attempt.
- **Cooldown/reload:** same-owner capture/restoration and enemy RNG/life restoration exist. F1 is the player cross-area exception. StartEncounters ignores RestoreCooldowns' boolean failure (`Framework/Wave4/LHStage1Session.cpp:219`); unknown cooldown definitions are an additional load/restore coverage gap rather than an executed separate finding. Ordinary encounter and Light clocks stop with pause/travel/off-floor policies; ordinary timer-only progress is saved at the next completed boundary by design.
- **Death suspected overwrite rejected:** production `Framework/LHPlayerState.cpp:27–35` calls HandlePlayerDeath then clears the ASC avatar. SyncResources only captures ASC pools while an avatar exists. Thus RequestRespawn's snapshot-only full-pool recovery is not overwritten through the normal dead-avatar path. The existing DeathRespawnSafe test deliberately clears the avatar; a real GameInstance/church route is still open in g4-automated.md.
- **Rev1→rev2 / corruption:** version dispatch changes only SchemaVersion; frozen v1 payload layout remains delegated by v2. Bounds preflight, exact canonical re-encoding, enum/UTF8/NUL/collection limits, duplicate set keys, checksum, rules/content mismatch and future-schema refusal are implemented before assigning output (`Persistence/LHSaveCodec.cpp:195–299`, `LHSaveWireV1.inl:61–95`). A/B store selects newest valid, rejects divergent equal sequence, does opposite-slot writes and exact readback, retains dirty snapshots for retry. Accepted old-catalog rejection is distinct from migration. F2/F3 expose semantic gaps after integrity checks.
- **Catalog closure:** inspected all eleven enemy rows, encounter→enemy references, listed loot→item definitions, service→spell/item IDs, starter kit and NPC generators. Existing CatalogClosure assertions enumerate 77 slots, all loot references and 200 XP rows (`Source/LighthavenTests/Integration/LHWave2Tests.cpp:510–518`). No concrete undefined authored roster/offer ID found. Required service/quest NPC counts and portal reverse edges are validated by `World/LHWorldValidation.cpp`. This is source/catalog coverage; LFS pointer maps were not treated as loaded content.
- **NoCombat/leash/navigation:** controller rejects partial paths and checks every segment against expanded safety volumes and leash; PostPhysics restores LastSafeLocation after an illegal crossing, cancels pursuit when the target enters a safety zone. Corpse Pawn blocking/navigation influence is disabled. ReturnHome path exhaustion intentionally stays stopped until unload (documented in ai.md); no specific authored disconnected island was proved. Initial SpawnRecord validates capsule/NoCombat but does not prove a nav connection; IsSpawnSafe projects a point, which is not a connected-path proof. Real Recast connectivity, largest-agent routes, all 77 safe respawns and player pursuit/exit traps remain untested. g4-automated.md explicitly limits aperture sweeps and player stair probes; their successes are not nav-agent traversal evidence.
- **Controller menus/modals:** traced gamepad bindings to inventory/character/abilities/pause, D-pad/stick focus, South confirm, East Back, shoulder tab change, name keyboard Done/East exit, default-cancel confirmation, recovery acknowledgment and RetrySave. Control lists provide Back/quit/retry; pending allocation switches use a discard modal. No concrete unreachable/inescapable controller modal found by source inspection. Hardware feel, focus routing in the graphical viewport and rapid device switching were not observed. Equipment selections refer to the retained presenter View (`UI/LHUIPresenter.h:125`), not a temporary snapshot.
- **Historical claims:** compared native item/enemy/ability/service/profile provenance against rules-ledger.md, world-ledger.md, ruleset-bible-v1.md and research/w4-bible-lookup.md/R-02 captures. Confirmed spell learning minima/costs, Light600s/10MP, potion50/+25MP, bow29/quiver100, Dirk sell9, monster documentary columns and Balork900s have source rows with exact URLs/dates. Runtime first-column XP interpretation, gold midpoint/drop odds, combat coefficients, healing magnitude, starting HP/MP/gold/kit/knowledge, AI distances/speeds and growth adapter are explicitly Prototype/Modernized; disputes are retained. Samaritan15/2500 has the retained secondary S4/S5 row, not a freshly verified Bible claim. No concrete numeric Bible/Confirmed claim lacking a retained source row was found in the inspected catalogs. No fresh web retrieval or historical gameplay authenticity claim; unlimited carry capacity and deferred wing quest are owner decisions. MP floor-band approximation remains the already documented open Rules issue.

## Execution evidence and limitations

Host uid1000; UE5.8.3 at `/home/brewerm/Downloads/unreal`. Production source remained unchanged throughout; temporary probe assertions were appended to the existing Integration fixture file, compiled, executed and then restored byte-for-byte. No test edits are included in the deliverable. Probe source is retained only in the attempt library as `temporary-probes.cpp.txt`.

| Check actually run | Observed result | Evidence in canonical attempt library |
|---|---|---|
| `UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game` | Exit0; editor Result:Succeeded,156.87s; game Result:Succeeded,140.41s | `build.log` |
| Initial W603 probe run before test module rebuild | Exit255; no tests matched; no verification claim from this run | `probes.log` |
| First probe-module build | Exit6; my scratch travel fixture incorrectly used marker fields Area/InstanceId; corrected to ALHPortal Source/Destination/PortalId; no production defect inferred | `probe-build.log` |
| Corrected editor/probe build, `UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh` | Exit0, Result:Succeeded,19.85s | `probe-build-fixed.log` |
| Headless `Lighthaven.Review.W603` | Exit0;4 completed,3 success without warnings,1 success with warning,0 failure/not-run/in-process; reported assertion duration0.180s | `probes-final.log`, `probes-index.json` |
| Headless existing `Lighthaven.Persistence` | Exit0;23 completed,23 success without warnings,0 failures/warnings/not-run/in-process; reported assertion duration0.205s | `persistence.log`, `persistence-index.json` |
| `git diff --exit-code -- Source/LighthavenTests/Integration/LHWave2Tests.cpp`; `git diff --check` | Exit0; temporary source restored; no whitespace errors | final scope check / handoff |

All four temporary probes assert **the observed defective behavior**, so their Success means the reproduction succeeded, not that a regression test or product gate passed. The one probe warning is the existing GameplayCueNotifyPaths fallback warning in InconsistentBossClaim. Persistence coverage includes SchemaRev1Migrates, SchemaRev2RoundTrip, MigrationKeepsOriginalSlot, BadChecksumFallback, BothSlotsBadVisible, BoundsBeforeAllocation, FutureVersionAndAlgorithms, MalformedAndCompatibility, InterruptedWriteFallback, CoalescingFailureRetryAndImmutability, LocalAsyncWriteAndReopen, RequestReceiptReplay, RequestEpochMismatch and digest/reward goldens. The production references callback is tested by the new probes; the generic persistence suite's synthetic compatibility fixtures do not close F2/F3.

Exact headless invocation for the probe run:

```sh
XDG_CONFIG_HOME="$PWD/Saved/BuildEnvironment/config" \
 /home/brewerm/Downloads/unreal/Engine/Binaries/Linux/UnrealEditor-Cmd \
 "$PWD/Lighthaven.uproject" \
 -ExecCmds="Automation RunTests Lighthaven.Review.W603; Quit" \
 -DDC-ForceMemoryCache -ddc=InstalledNoZenLocalFallback \
 "-LocalDataCachePath=$PWD/Saved/DerivedDataCache" \
 -nullrhi -unattended -nosound -nop4 -noshaderworker -nocrashreports \
 -stdout -FullStdOutLogOutput \
 '-ini:EditorSettings:[/Script/UnrealEd.AnalyticsPrivacySettings]:bSendUsageData=False' \
 '-ini:Engine:[ConsoleVariables]:HomeScreen.EnableHomeScreen=0' \
 -ReportExportPath="$PWD/Saved/W603ProbesFinal"
```

Persistence used the same invocation with `-ExecCmds="Automation RunTests Lighthaven.Persistence; Quit"` and `-ReportExportPath="$PWD/Saved/W603Persistence"`. Console output was redirected to the retained logs. Export durations are test durations, not editor startup wall time. Startup logs report eight unloadable baseline LFS-pointer `.umap` files; neither run loaded those maps for its assertions. No generated map, arrival review, LHValidateWorld, real map nav, cook/package, graphical launch, gamepad hardware or opposed full-route play was run. Maps require local regeneration/hydration; nav proofs require actual navigation worlds/agent routes; controller feel requires graphical/hardware checks; full death/church and earned/inventory routes require the missing G4 fixtures. The installed engine was available and both builds ran; these omissions are not attributed to a missing engine. Windows remains deferred by owner decision. No G4/G5/G6 gate is passed by this report.


## Handoff

Task ID: W6-03.
Base revision / result revision: `68bb29e28f27b886326fe2ef34261bcc2aea4935` / deliverable commit recorded by the submission receipt and attempt report.
Contract revision: 1.
Owned paths / binary assets: `docs/implementation/reviews/w6-03-adversarial.md`; no binary assets.
Behavior changed: findings documentation only.
Source-backed mechanics: existing retained Bible/secondary evidence reviewed; no new values.
Provisional tuning introduced: none.
Build/editor/cook/package/play checks actually run: see execution section; no cook/package/graphical play claim.
Results and evidence paths: this review and the canonical attempt report/library.
Checks not run and concrete missing prerequisite: real map/nav/gamepad coverage requires generated or hydrated maps, complete session/nav fixtures or graphical/hardware access; Windows deferred.
Known defects or remaining decisions: F1–F4; existing G4 earned-route, real death/church, inventory and nav-safe respawn coverage remains open.
Next task and integration notes: coordinator assigns narrow fixes to the named owners; retain source baseline references and retest each affected boundary. No source/test changes belong in this review submission. This is an evidence candidate, not a gate approval or verified task-success declaration.
