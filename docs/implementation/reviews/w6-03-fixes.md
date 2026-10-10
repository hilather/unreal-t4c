# W6-03fixd fixes and regression evidence

Base: `93a186bec8145732418418bfacdd6749027bf03f`. Linux first, Windows deferred. This is an evidence candidate, not gate approval.

## Changes

- F1: player entity origin is always `Area.LighthavenTempleDistrict` (D12). Capture and StartEncounters therefore use one cooldown owner across floors. Enemy owners retain their area. Light keeps its existing explicit effect-area remapping policy. No schema change or offline clock advancement.
- F2: the production compatibility callback checks learned skills against enabled training offers, spells/cooldowns against the ability catalog, bosses against boss enemy definitions, quests against the two enabled quest implementations, and inventory/object/corpse items against the item catalog. Comparisons check canonical spelling as well as FName equality. Cooldown owners must belong to this run and a player origin or authored enemy slot. Encounter definitions already resolve through the area's spawn catalog in ValidateWorld. No enabled object-definition catalog exists; object syntax/ownership validation remains, with contained-item closure added.
- F3: load requires boss defeat/unique claim/deterministic BossUnique ID/retained claim agreement in both directions. Balork's quest facts must match defeat; Samaritan's rewarded turn-in must match its deterministic claim and completed facts. Retained unique claims must be accounted for by those records. Load performs no reconstruction or reward grant.
- F4: every travel return retains Request; acceptance reports Reason=None. Sequence and async durability behavior are unchanged.
- F5: FindByEntity is the combat spawn-slot lookup and now selects only a living actor. FindByLife continues to resolve retained corpses for TakeLoot. The frozen command shape contains no generation; this fixes selection without inventing a schema field.

Permanent tests live in `Source/LighthavenTests/Review/LHW603Tests.cpp`. Fixtures use production session creation and compatibility, real Encode/Decode, deterministic reward settlement and a quest observer, travel coordinator/save adapter, and corpse-first registry ordering. The registry regression deliberately installs two runtime lives via reflected registry storage to isolate lookup from navigation; earned-route tests remain the navigation/session integration evidence.

## Validation

Host uid1000, UE5.8.3 at `/home/brewerm/Downloads/unreal`. No engine/SDK downloads, Core changes, Integration test changes, or binary assets submitted.

| Executed check | Observed result |
|---|---|
| Initial `UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game` | Exit0; editor Succeeded 166.67s, game Succeeded 208.59s |
| First new-test compile | Exit6; fixture omitted Decode's Character argument; corrected |
| Corrected editor rebuild | Exit0; Succeeded 324.96s including mutex wait |
| Final `build/build-linux.sh --game` | Exit0; editor Succeeded 40.95s, game Succeeded 58.09s |
| `Lighthaven.Review.W603` | Exit0; 3 completed: 1 success without warnings, 2 with warnings, 0 failures/not-run/in-process; assertion duration 0.348141s |
| Full `Lighthaven`, baseline pointer maps | Exit255; 196 completed: 180 success without warnings, 5 with warnings, 11 failures, 0 not-run/in-process; assertion duration 48.415524s |
| Local hub/BasementA/BasementB generation | Each exit1 due baseline unloadable pointer packages; all five playable maps saved (SavePackage log evidence) |
| `Lighthaven.Integration.G4.` with generated maps | Exit255; 8 completed: 3 success without warnings, 1 with warnings, 4 failures, 0 not-run/in-process; assertion duration 27.244108s |
| Full `Lighthaven` with generated maps | Exit255; 196 completed: 186 success without warnings, 5 with warnings, 5 failures, 0 not-run/in-process; assertion duration 95.323898s |
| `git diff --check` | Exit0 |

The baseline pointer-map failures (all other tests passed):

- `Lighthaven.Integration.G4.DeathAndChurchRespawn`
- `Lighthaven.Integration.G4.DoorClearance`
- `Lighthaven.Integration.G4.EarnedMagicRoute`
- `Lighthaven.Integration.G4.EarnedMeleeRoute`
- `Lighthaven.Integration.G4.EarnedRangedRoute`
- `Lighthaven.Integration.G4.SessionFloorRouteAndReload`
- `Lighthaven.Integration.G4.SessionInventoryRollback`
- `Lighthaven.Integration.Wave3.ArrivalSafety`
- `Lighthaven.Integration.Wave4.B1SpawnOnContinue`
- `Lighthaven.Integration.Wave4.B1SpawnOnTravelArrival`
- `Lighthaven.World.LightingAudit`

Headless invocations used `library/w603-run.sh` (retained exact script). Arguments were `Lighthaven.Review.W603 W603Review`, `Lighthaven W603FullPointers`, `Lighthaven.Integration.G4. W603G4Generated`, and `Lighthaven W603FullGenerated`. It invokes UnrealEditor-Cmd with Automation RunTests, memory DDC/InstalledNoZenLocalFallback, local cache/config, nullrhi, unattended, nosound, nop4, noshaderworker and JSON report export. All output logs and index.json exports are retained in the canonical attempt library. Durations above are build-reported or assertion durations, not inferred editor startup wall times. Warnings in the new tests are the existing GameplayCue path fallback and deliberately absent B1 spawn markers in the travel seam fixture.

## Further findings and route limits

**F6 (new, unpatched): earned-route B1 combat rejects with OutOfRange (6).** In the focused G4 run, magic resolves requested `LHEnemyCharacter_22` generation1 to the same living actor; melee resolves `_21` generation1; ranged resolves `_35` generation2. All show playerHP30 and reason6, replacing the baseline corpse resolution/InvalidLifeState4. Each completed legal creation, generated hub→B1 travel, and repeated real B1 fights/respawns, but stopped inside initial Farm (60 kills for melee/ranged, 200 for magic), before returning to hub for earned training/purchases/spell learning. No completed Farm summary was emitted, so no exact completed-kill count is claimed. No route reached its class-specific four-floor combat or Balork completion. Full-suite repetition shows magic `_34` generation2, melee `_22` generation1 and ranged `_31` generation2, again exact living resolution and reason6. SessionInventoryRollback also stops on the same rejection (focused generation1; full generation4).

Repro: run the retained G4 fixtures with locally generated maps. `Integration/LHWave2Tests.cpp:1480–1497` places the player 150cm from the enemy, advances 0.5s, then sends UseAbility. `Abilities/LHCombatComponent.cpp:51–62,83` combines distance and line-of-sight failure into OutOfRange. Logs do not measure final separation or identify a blocking hit, so the specific distance versus sight cause remains unresolved. Retained corpses still block Visibility (LHEnemyCharacter.cpp:64–66), and enemies move during that half-second; those are investigation leads, not established causes. No range relaxation, corpse visibility change, fixture edit, free resource grant or teleport workaround was added.

**F7 (new observation, unpatched): DeathAndChurchRespawn depends on suite context.** Focused G4 passes the opposed death/church bridge, but full generated-map execution fails at `real populated enemy` (no alive enemy found after arrival). This needs isolation of spawn/world state under the full suite. The focused passing result does not establish full-suite stability.

Thus the requested baseline full suite has only the listed pointer-map failures, but generated-map validation is not green: F6 and F7 remain. No G4 gate or verified task-success claim is made. Generated maps were restored to their original pointer files after testing; no ReviewedArrivals or Integration changes remain.

Assumption/compatibility limit: enabled catalogs are authoritative; unknown retained unique claims and legacy player cooldown owners outside the hub origin are invalid rather than silently reconstructed/remapped. Earlier invalid generations can trigger A/B fallback; if neither is valid the existing visible save recovery applies. No saved-shape migration was introduced. Object definitions and flags without an enabled catalog retain syntax checks. Graphical play, cook/package and hardware/gamepad tests were not run; graphical/hardware session and packaging work remain outside this task. Windows is deferred by owner decision.


## Handoff

Task ID: W6-03fixd.
Base revision / result revision: `93a186bec8145732418418bfacdd6749027bf03f` / submission candidate.
Contract revision: 1.
Owned paths / binary assets: Framework, AI, Review tests, this document; no binary assets.
Behavior changed: F1–F5 as above.
Source-backed mechanics: no numeric mechanics changed.
Provisional tuning introduced: none.
Build/editor/cook/package/play checks actually run: final validation section.
Results and evidence paths: this document and canonical attempt report/library.
Checks not run and concrete missing prerequisite: Windows deferred; graphical/gamepad checks require graphical/hardware session.
Known defects or remaining decisions: F6 OutOfRange earned-route blocker; F7 full-suite death/spawn context failure; object catalog absence and legacy owner rejection noted above.
Next task and integration notes: coordinator review of source and evidence, then isolate F6 and F7 with the G4 test owner. No merge/push performed.
