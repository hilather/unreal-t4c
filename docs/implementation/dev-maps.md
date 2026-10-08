# W1-04 reproducible dev rooms

The editor commandlet `LHGenerateDevMaps` creates only
`/Game/Lighthaven/Maps/Dev_Movement` and `/Game/Lighthaven/Maps/Dev_Combat`.
It replaces those maps with fresh non-partitioned worlds; do not hand-edit them
and expect edits to survive regeneration. It saves no runtime assets, config,
or other maps. Run it in a fresh editor process with no unsaved work.
No generated map or play result is delivered by this C++ task.

All coordinates below are Unreal cm, floor top Z=0. Geometry, ranges, angles,
and lighting are **Prototype**, provenance W1-04 / LH_Prototype_v1 / 2026-10-07,
source URL null. These are authored test fixtures, not authentic T4C metrics.
The layout specification supplies U=100 cm, 20 cm walls, 400 cm wall height,
240×300 cm ordinary doors, 320 cm corridors, 300 cm stairs and 320 cm landings.
It does **not** specify step height: 20 cm is an explicit task assumption.

Both rooms have 4000×3000 cm clear floor (X ±2000, Y ±1500), a 20 cm slab,
perimeter walls outside the clear bounds, a PlayerStart at (-1600,0,100) facing
+X, sunlight, sky atmosphere and skylight. NavigationBounds is a brush-backed
NavMeshBoundsVolume centered (0,0,500), size 4000×3000×1200. Bounds alone do
not establish a baked or working navmesh; the coordinator must build navigation
and inspect reachability in the editor before saving the final LFS maps.

## Movement

- Door at X=-900, Y ±120, lintel underside Z=300. Traverse both ways.
- Corridor center Y=0, walls X[-890,90], clear Y ±160. Check camera collision
  while running through the door, corridor and mouth.
- Walkable candidate ramp begins (200,-1050,0), length 800 along slope,
  width 300, angle 30°, rise 400. Steep candidate begins (200,-550,0),
  length 600, width 300, angle 60°, rise about 519.6. Walkability labels
  assume a 45° movement slope limit and need W1-01 confirmation. Positive
  pitch rises toward +X. The top surface lower edge meets the floor.
- Five 100 cm run treads at Y=600, width 300, X centers 300 through 700,
  top Z=20,40,60,80,100. Landing center (910,600), 320×320, top Z=100.
  These are deliberately real step collisions for testing step-up behavior.
- Low ceiling center (1300,0), 600×600, underside Z=220; side walls leave
  Y ±300 clear. Exercise boom shortening, zoom and orbit without clipping.
- Boss width ruler at X=0, Y[650,1150], 500 clear width, 360 headroom.
  It tests the 440 cm full-spread Balork proxy against the layout's boss door
  minimum. This room does not certify 550 cm boss corridors/stairs or 600 cm
  boss turns; those remain W3/B4 acceptance checks.

## Combat

LOSPillar is a solid cylinder centered (-500,500,200), diameter 180, height
400. From the player floor origin, TargetLOS at (600,1000,0) lies behind it.
The three main TargetPoints share Y=0 and floor Z=0:

| Actor | X | Horizontal distance from player floor origin |
| --- | --- | --- |
| TargetMelee | -1450 | 150 cm |
| TargetMid | -1000 | 600 cm |
| TargetLong | 0 | 1600 cm |

150 cm is a provisional melee ruler, not an approved attack-range constant.
Distance measurements are center-to-center, not surface-to-surface. Confirm
range decisions against W1-02/W1-03; the generator does not implement damage.
TargetPoint icons are editor-only; small nonblocking cylinder discs named
`TargetMeleeRuler`, `TargetMidRuler`, `TargetLongRuler`, `TargetLOSRuler` remain
visible in game. TargetPoints carry `LH.Dev.DummySpawn`, `LH.Dev.Slot.Melee`
(or Mid/Long/LOS), plus `LH.Dev.DistanceCm.150/600/1600` (or `LH.Dev.LOS`).
W1-03 must consume these markers, supply the appropriate capsule-offset spawn
transform and own enemy creation. Markers alone are not enemies.

## Names, identity and integration

All explicitly spawned actor names equal labels and carry `LH.Dev.Generated`.
Names in code describe features (Floor, WallWest/East/South/North, PlayerStart,
Sun, SkyAtmosphere, SkyLight, NavigationBounds, DoorLeft/Right/Lintel,
CorridorSouth/North, RampWalkable30, RampSteep60, StepZ20/40/60/80/100,
StairLanding, LowCeiling, CeilingSupportSouth/North, BossDoorSouth/North/Lintel,
LOSPillar and target names above). No random placement or array-order IDs.
Layout determinism means identical feature names/tags/transforms, not identical
package bytes: engine-generated GUIDs/save metadata may differ on each run.

These tags are test aliases, **not FLHEntityId or FLHSpawnLifeId**. No authored
save identities are minted or inferred from labels, array order or coordinates.
An integrator adding persistent spawn slots must assign and preserve explicit
GUIDs using contracts-v1; moving a marker must preserve the assigned identity.
Only engine actors are serialized; no editor-module actor class enters the map.

The generator tries `/Script/Lighthaven.LHGameMode` via a soft class path.
If absent it logs a warning and keeps configured/engine defaults. Integrator
requests: confirm the final W1-03 class path, connect dummy marker consumption,
confirm W1-01 slope/step/capsule settings, and explicitly select/cook these maps.
No runtime module, .uproject or Config changes are made here.

## Regeneration and required host checks

```bash
UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh
UE_ROOT=/home/brewerm/Downloads/unreal bash build/generate-dev-maps.sh
```

Run generation as a normal host user; the wrapper rejects UID 0 before loading
the environment or editor. It invokes UnrealEditor-Cmd with the project,
`-run=LHGenerateDevMaps -unattended -nullrhi`. Exit 0 means both SaveMap calls
reported success. A save failure exits 1; the pair is not atomic (the first map
may already be replaced). Back up any manually changed maps beforehand.

Coordinator: generate twice, compare feature transforms/names/tags, open both
maps, build navigation, inspect collisions/ramp directions/lighting, run the
actual character on keyboard/mouse and controller, consume markers with real
dummies and test hit timing/LOS. Null RHI generation cannot prove appearance,
play, camera collision or G1. Commit resulting .umap files via LFS after review.
