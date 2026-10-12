# W5-25 world prop binding — evidence candidate

Task ID: W5-25. Contract revision: 1.
Base revision: `dffac4648f13fabbd00bc2bc88913c250f72150b`.
Result revision: the submission receipt records the exact candidate OID.
Owned paths changed: C++ under `Visual/Player/`, `Visual/Props/`, new
`Visual/Effects/`, presentation hooks in `Framework/`, tests under
`Source/LighthavenTests/Visual/Props/`, and this document. No binary assets.

## Behavior changed

`ULHPlayerEquipmentVisual` resolves the bound inventory instance to its exact
item definition ID and uses the props catalogue. Dirk/goblin blade attach to
`Socket.Weapon.R`, flatbow to `Socket.Bow.L`, quiver to
`Socket.Quiver.Back`, and the drawn arrow to `Socket.Arrow`. Imported player
sockets are used when available; procedural anchors remain the fallback.
The original boxes remain hidden while their imported replacement resolves,
and become visible again when it does not. The bow mesh includes its string;
the arrow hides when the existing bow action ends. Four reusable static mesh
components carry equipment; all disable collision, overlap and navigation.

An active authoritative Light duration enables the imported torch in the left
hand, except while that hand holds the flatbow, and an orb above the caster.
There is no independent active-torch state in the current runtime; inventory
ownership does not imply an active torch. This assumes the existing Light
state permits a cosmetic torch. The spell's existing illumination remains;
its point light now disables shadows. **Zero additional effect lights** are
created: the light cap for this binding is the one existing player spell light.
Other world lighting is outside this cap.

The player import signature is unchanged: `LHPlayerVisual.h` and the full bodies
of `ConfigureImportedMesh` and `CompactImportedMesh` remain byte-identical to
the base. No character package reimport is required. New equipment code is in
separate files; runtime changes to `LHPlayerVisual.cpp` occur outside those hooks.

A corpse badge requires a matching spawn life, a finalized and unclaimed corpse
record, and remaining items or resolved positive gold. Live enemies, missing
records and emptied corpses show none. Presentation reads a snapshot copy once
per controller tick and never writes loot state. No ground-bundle runtime state
was found, so the sack is unbound. `FLHObjectRecord` remains schema-only for
chests; no playable chest object is bound.

`ULHMeshEffects` is a per-actor pool capped at **16 transient mesh slots** plus
one persistent light orb. Projectile visuals span the published
`FLHAttackEvent::ImpactSeconds` from commit to authority completion. They never
schedule, delay or resolve an impact. Finished/cancelled events remove their
matching flight identity; hits present fire impact, stone shatter, melee spark
or arrow puff, and melee completion presents a slash arc. Heal and dust effects
present at successful completion. Misses omit hit effects. Enemy combat uses
the same read-only presentation delegate seam. The target position is a cosmetic
sample of the matching enemy actor or current player avatar.

The pool translates, scales and spins mesh components; translucent kit materials
now expose `PresentationFade`, set through dynamic instances. Opaque fragments
shrink out. Fire stretches along travel, stone rolls, dust spins, heal rises,
and the slash sweeps. Dust anchors at the target capsule foot, so creature
size does not bury the effect. The orb follows the caster while Light remains active and
fades/shrinks for 0.2 seconds after that state ends. No Niagara, gameplay timer,
collision, RNG, command, save schema, combat formula or authority clock changes.

Source-backed mechanics: no new mechanics. Provisional presentation tuning:
0.22-second slash, 0.35-second hit effects, 0.65-second heal/dust, 0.2-second orb
fade; 16 slots, badge scale 0.45/offset 110cm, orb offset 100cm,
heal rise 35cm and spin/scale curves. These are Prototype art tuning derived from
W5-23's lifecycle suggestions, not authentic T4C values. Host visual review may
replace them without affecting combat timings.

## Validation

Observed uid 1000; installed Unreal 5.8.3 Linux and Blender 5.2.2 were used.
No engine, SDK or large asset download/install occurred. Both targets used the
non-adaptive unity farm recipe, local XML configuration copies, and these flags:
`-WaitMutex -DisableAdaptiveUnity -UBASharedMemoryTempFile=true -NoUBA`.
`XDG_CONFIG_HOME=$PWD/Saved/BuildEnvironment/config` and
`UBA_ROOT=$PWD/Saved/UBA` were scoped to build commands.

- Final `LighthavenEditor Linux Development` build: exit 0, `Result: Succeeded`,
  139.96 seconds. Final `Lighthaven Linux Development`: exit 0,
  `Result: Succeeded`, 137.27 seconds. Initial builds caught and corrected six
  compilation errors (two in the first build, four in the second). Intermediate
  editor builds also succeeded. UBA reports failed action-result-store tasks;
  those are warnings alongside the observed successful target results.
- `UE_ROOT=/home/brewerm/Downloads/unreal
  BLENDER_ROOT=/home/brewerm/Downloads/blender-5.2.2-linux-x64
  bash build/build-props-art.sh` ran the real Blender export/icon render and
  validators. The first Unreal step exited 1 because the project module had
  not built yet. After build, the same stage with `LH_PROPS_SKIP_EXPORT=1`
  exited 0 and printed `PROPS_IMPORT_VERIFIED`.
- Imported 63 packages, **1,675,979 bytes**, with 29 bounds checks. Ordinary
  props and effects retain the authored mesh pivots; fade adds 7,446 material
  bytes relative to W5-24's receipt. Existing content was hydrated from 442
  SHA-256-verified local LFS cache objects (69,213,024 bytes) for checks only.
- Final focused `Automation RunTests Lighthaven.Visual.Props`: exit 0,
  **3/3 passed with warnings**, zero failed/unrun; test duration 0.096563 seconds.
  Checks cover exact item resolution and dangling bindings, actual equipped
  meshes, an asset disappearing and box fallback returning, item/gold/empty/
  claimed/live/corpse badge cases, every effect mesh, attack/ability callbacks,
  cancellation, cap overflow and idle return, light orb state and unchanged
  duration, target capsule-foot dust placement, unchanged health and RNG.
- A completed full suite before the final dust correction: exit 255,
  **246 tests: 245 passed with warnings, 1 failed, 0 unrun/in-process**,
  test duration 308.420563 seconds. Arrivals **9/9 PASS**. Failure:
  `Lighthaven.Visual.Monsters.CreatureImportedAssets`, line 90, eleven missing
  generated creature-validation JSON fixtures. The earlier first full run was
  interrupted by a sandbox-denied Epic telemetry request before exporting its
  report. The completed retry used the documented privacy flags plus
  `-NoAnalytics`; no fatal/assertion was found in the interrupted log.

Final full suite after the dust correction: exit **255**, **246 tests: 245 passed
with warnings, 1 failed, 0 unrun/in-process**, test duration
**292.054779 seconds**. Sole failure is the same
`Lighthaven.Visual.Monsters.CreatureImportedAssets` missing-fixture test.
Arrival safety: **9/9 PASS**. The new prop tests, existing replay/determinism,
combat, route, save/load and travel checks passed in this final run. Common
null-RHI/SDL Wayland warnings remain; this is not a warning-free suite.

Evidence: this attempt's worker-output `library/` contains final and prior full
JSON results, focused JSON results, build logs, trimmed failed/full/import logs,
the 63-package import receipt, arrival TSV and player signature-input comparison.
Python syntax and Git whitespace checks exited 0. Generated-file cleanup and
final scope checks are recorded in the cleanup evidence before submission.
Content pointers were restored and all generated props/export/build/Saved
artifacts removed at 2026-10-12 06:07:31 UTC, well over 20 minutes before the
wall-budget deadline. Final Git scope/whitespace checks passed; evidence is
1.1 MiB and no generated binary packages remain.

## Checks not run, remaining limitations and integration

No rendered in-engine visual review, GPU/translucency performance test, cook,
packaged launch or hands-on play was run. Those require a host display/package
validation pass; Windows packaging remains deferred by owner decision. The
imported bow string is static geometry and does not deform during draw. Socket
fit on both bodies across all actions, torch/bow hand choice and visual tuning
remain host review items. No visual gate or verified task success is claimed.

The missing `artsource/creatures/output/*-validation.json` fixtures are rat,
bat, slime, goblin, giant_spider, balork, goblin_warrior, atrocity, dungeon_bat,
giant_bat and undead_bat. They are outside this task's owned paths. No fixture
was fabricated and no failing test was disabled.

Next task: coordinator reviews this source candidate, imports the updated props
materials through `build/build-props-art.sh`, supplies the real creature
validation fixtures, and performs host visual/cook/package checks. The player
packages need no reimport. Binary destinations and LFS integration remain with
the coordinator. No push, merge, SQLite change or worker-capacity action occurred.
