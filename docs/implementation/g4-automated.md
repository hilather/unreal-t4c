# W4-15 automated evidence candidate

Base: `aa03c765c4b231a62b36eb0a3434a428992b5f5c`, contract revision 1.
This is a partial deliverable with a failing clearance regression, not G4 approval.
Production code and mechanics are unchanged. No grants, cooldown resets, snapshot
edits or direct reward settlement occur in the new G4 tests.

## Mapping and limits

| Requested checklist | Test / evidence | Remaining scope |
| --- | --- | --- |
| Fresh Bible creation | G4.LegalPurchaseReplayAndRetrySave asserts HP30, MP10, gold100 through creation presenter | Build-specific creation/earned allocation routes not implemented |
| Save failure → RetrySave; repeated requests | G4.LegalPurchaseReplayAndRetrySave: legal mana-potion purchase, failed storage, exact replay, widget RetrySave, independent reload | Failure at combat/death/travel boundaries remains separate |
| Hub → B1 → B2 → B3 → B4 → return; reload every floor | G4.SessionFloorRouteAndReload uses actual session RequestTravel, authored portals, generated geometry, travel/save adapter and encounter population | Travel-only route; no melee/ranged/magic progression or Balork defeat; no locomotion/pathfinding walkthrough |
| Arrival grants nothing / replay | Same route asserts unchanged XP/gold, exact independently decoded saves and no write on duplicate load callbacks | Repeated user portal interaction debounce remains existing coverage |
| Occupied arrival | Same route ends with an actual blocking player capsule, collision refusal reason and no arrival write; durable source remains hub | Test adapter collision callback mirrors placement query; does not invoke private production Place. Restore callback acknowledges recovery without reloading source geometry; real GameInstance recovery remains unverified |
| Largest enemy / doors | G4.DoorClearance sweeps player and maximum catalog capsule envelope through B3/B4 lintel centers; B1/B2 authored flat route segments; positive floor-trace controls | Stair/ramp sweeps and complete B1/B2 aperture enumeration still required |
| B4 C04 and D05–D07 | C04Door0/1Lintel, ArenaPartitionLow/HighLintel, ReliquaryPartitionLintel; asserted five-opening count; both capsule sizes | Sweeps establish collision clearance, not CharacterMovement walking or nav-agent reachability |
| Death/church, pending action floor change, inventory rollback, simultaneous lethal hits | Existing Wave4 / Rewards fixtures only; no new G4 tests for these in this candidate | Required real-session edge tests remain unfinished |
| Balork 15:00 and single claim across reload/travel | Existing Wave4.BalorkSingleClaim uses synthetic floor installs and direct respawn advancement | Required clock/session-only replacement remains unfinished |
| Training/vendors/first damage spell funded by real kills | Existing Wave4.ServicesThroughSession and ProgressionRouteAffordable use synthetic floors/cooldown resets | Three complete earned build routes remain unfinished; existing tests must not be represented as satisfying this requirement |

The route fixture loads distinct copies of real generated worlds. Positioning the
player at each authored portal isolates command/travel validation; it is not evidence
that a player can walk the connecting route. Normal arrival installation uses the
adapter hook required by travel; checkpoint data is not edited by the test.

## Collision defect candidate

Repro: regenerate B3/B4, run `Lighthaven.Integration.G4.DoorClearance`.
The catalog maximum is Balork (radius100, half-height155 cm; authored Prototype
capsule in LHEnemyCatalog). Sweep at center Z157 through standard B3/B4 doors.
Expected under W4-15's largest-enemy requirement: no blocking collision.
Observed: standard door lintels block the enemy sweep while player35/90 clears.
Standard lintel underside is Z300; capsule top is Z312, explaining the collision.
B4 C04 and the three interior partition apertures clear both capsules.
This is a geometry/catalog requirement mismatch, not a claim that Balork currently
tries to traverse every floor. Coordinator should decide whether the global-largest
requirement means changing door height or restricting the clearance envelope by
floor. Production is deliberately unmodified.

## Manual-only and unfinished automation

Visual lighting/readability, Light appearance, animation/effects/audio, controller
feel, packaged Linux launch and in-play render-stall behavior remain unobserved.
Windows packaging/launch is deferred by owner decision. Those cannot be passed
with null RHI. The unfinished automated items in the table are additional work,
not inherently manual-only and not an engine-install blocker. No full earned
build route was attempted in this candidate; its feasibility is unestablished.
MP floor-band policy remains outside this test-only scope; carry capacity remains
unlimited by the retained owner decision.

## Commands and evidence

Engine `/home/brewerm/Downloads/unreal`, UE5.8.3 Linux, uid1000.
`UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game`:
final editor/game both `Result: Succeeded` (last build log retained in worker library).
Generated maps using `generate-hub-map.sh`, `generate-basement-a-maps.sh`
(with LH_NO_ZEN=1), `generate-basement-b-maps.sh`. Scripts exit1 with pointer-package
startup errors after saving five playable maps. Hub also regenerated directly with
`-run=LHGenerateHubMap -DDC-ForceMemoryCache`. No map or review TSV is submitted.

Headless form:

```sh
XDG_CONFIG_HOME="$PWD/Saved/BuildEnvironment/config" \
 /home/brewerm/Downloads/unreal/Engine/Binaries/Linux/UnrealEditor-Cmd \
 "$PWD/Lighthaven.uproject" \
 -ExecCmds="Automation RunTests Lighthaven.Integration.G4; Quit" \
 -DDC-ForceMemoryCache -nullrhi -unattended -nosound -nop4 \
 '-ini:Engine:[ConsoleVariables]:HomeScreen.EnableHomeScreen=0' \
 -ReportExportPath="$PWD/Saved/G4LastReport" -log=G4Last.log
```

Initial three-test run: two pass, DoorClearance fails. Its two B1/B2 missing-lintel
fixture assertions were corrected to use authored polygon-route probes; the B3/B4
collision failures are retained. Final run counts and broader regression evidence
are recorded below after execution. `git diff --check` passed. No cook/package,
graphical editor or hands-on play performed. Evidence resides in the canonical
attempt's `library/`, not in tracked binary assets.

Final revision execution: G4LastReport contains 3 completed tests: **2 success,
1 failure** (DoorClearance, 29 errors); process exit255. Player sweeps clear; largest
enemy also blocks B1 segment (4500,500,157) → (4500,1400,157), Geometry_0286.
B3 has22 blocked lintels, B4 has6. Five special B4 openings clear both shapes.
Occupied-arrival assertions passed in the final SessionFloorRouteAndReload test.
Final build: editor succeeded (about12 seconds), game succeeded (2.71 seconds).
The broader Integration invocation logged31 completions (30 success,1 failure)
before a sandbox-blocked request to datarouter.ol.epicgames.com terminated the
process/tool; no G4FinalReport was exported. This is incomplete regression evidence,
not a completed suite count. Final targeted invocation duration about65 seconds.
