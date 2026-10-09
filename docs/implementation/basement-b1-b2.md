# W3-02 — deterministic B1/B2 graybox authoring

Candidate against W3-04 `0768422a59c34efe687c640b928ace2aca90d262`. Owns the new Basement A commandlet, launcher and this document; no World/Core/config/binary changes. Maps remain editor-generated, with coordinator ownership of the LFS commit. This is an evidence candidate, not a G3 or play sign-off.

Run `UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game`, then `UE_ROOT=/home/brewerm/Downloads/unreal bash build/generate-basement-a-maps.sh` as a non-root user. The wrapper rejects arguments, root and existing read-only LFS outputs. The commandlet also checks read-only destinations. Outputs are `/Game/Lighthaven/Maps/L_TempleB1` and `/Game/Lighthaven/Maps/L_TempleB2`; it starts fresh worlds, rather than appending to existing maps.

All metrics are **Prototype**: inherited V-01 / LH_Prototype_v1 / 2026-10-07 construction coordinates, A-02 modules and A-01 creature envelopes, with A-04 / LH_Prototype_v1 / 2026-10-08 lighting. No historical measurements, mechanics, HP/XP/loot or new enemy profiles are introduced. Source backing is the existing [layout conventions](layout/README.md), [B1](layout/b1.md), [B2](layout/b2.md), [map observations](map-reference-notes.md), [world ledger](world-ledger.md), [world actors/registry](world-systems.md) and A-01/A-02/A-03/A-04 art guides. No external research or visual-source reinspection was performed.

## Construction and route cross-check

The exact union of clear room/corridor rectangles is partitioned at their boundary coordinates. Each resulting floor cell is placed once and walls are emitted only on exposed boundaries. Rectangle intersections remove internal seams; adjacent rooms only connect at their listed shared open seam. No floor covers the B2 void or any of the four ramp footprints. Ramps are smooth rotated 20 cm slabs with nonblocking tread stripes, side masonry and terminal blockers extending down to Z=-150 cm. Floors end flush at slope start; no overhead slab is added.

B1 retains Entry → Hub, independent Hub → Healers and Hub → Descent arms, both closed bays and the descent enclosure with its 240 cm low-Y opening and 300 cm lintel underside. The enclosure's outside perimeter remains available. B2 retains Entry → ring → LowerBay → Junction → +Y/-X/+Y dogleg → LongHall → Descent, both ring lanes, the separate BranchJunction/dead-end/UpperHook branches, the full 10 U Bay seam and 8 U Hook seam. The 2 U Hook–Hall gap, ring void and dogleg interior have no walkable floor. No chest legend becomes loot or an encounter.

Before save, collision capsule sweeps check the flat main routes, both ring lanes, B1 enclosure perimeter and optional branches. B1 uses a conservative combined R40/HH75 cm envelope (Slime radius, Bat height); B2 R55/HH100 cm (Spider radius, Giant Bat height). The written 320 cm corridors, 300 cm stairs and 240 cm B1 enclosure door exceed these envelopes. A-01's largest visible widths are B1 Slime 90 cm and B2 Spider/Giant Bat 170 cm. There is at least 300 cm doorway headroom; ramps have no ceiling. Sweeps do not certify limbs, simultaneous player passing, slope traversal or cooked navigation.

Each floor has one NavMeshBoundsVolume covering its floor union. Navigation rebuild/cook and actual AI routes remain separate checks. The generator rejects an unexpected registry roster/count, an anchor outside the floor union or an invalid registry; expected counts are 17 B1 and 25 B2. No enemies are activated by this authoring operation. Nevanis (-25,6) and Shovanis (-25,14) have separate ground-anchor TargetPoints and nonblocking human rulers; actual service bindings remain W4-07.

## Travel and identity

The registry supplies all marker transforms, spawn GUIDs and portal GUIDs. No identity is derived from coordinates, labels or list indices. Actor names and geometry order are fixed; semantic determinism means equivalent authored transforms/components/IDs, not byte-identical Unreal package metadata.

| Map entrance | Departure ground center, cm / yaw | Destination | Arrival ground center, cm / yaw |
|---|---|---|---|
| B1 Entry | (450,-2500,100) / -90 | Hub Temple.Descent | (450,-1650,0) / +90 |
| B1 Descent | (4500,2100,-75) / +90 | B2 Entry | (4500,1400,0) / -90 |
| B2 Entry | (500,-6300,100) / -90 | B1 Descent | (500,-5450,0) / +90 |
| B2 Descent | (5300,6600,-100) / +90 | B3 Entry | (5300,5750,0) / -90 |

Arrivals are separate from terminal departure positions. ALHPortal uses explicit interaction rather than automatic overlap; runtime interaction/travel integration belongs to W3-04/W4. The registry already holds reverse directed edges for Hub and B3; this generator writes only B1/B2 actors. ALHEntranceMarker `bSafetyReviewed` stays false. Bounds-only TriggerBoxes tagged `LH.Safety.NoCombat.Required` record Entry/C01 and descent enclosure/approaches, plus B1 Healers/C02; collision/overlap events are disabled. These are authoring exclusion requirements, **not implemented damage, pursuit or corpse protection**.

ALHSpawnMarker uses every registry alias→GUID mapping for B1/B2, with facing +Y and ground-contact Z. B1: Brown Rat12/Bat3/Slime2. B2: Brown Rat6/Bat3/Slime3/Giant Bat4/Undead Bat4/Giant Spider3/Dungeon Bat2. Dungeon Bat markers carry `LH.Placement.Provisional`; Undead Bat markers carry `LH.Provenance.Disputed`, preserving the selected B2 profile and unresolved B3 source disagreement. No generic undead, goblins, Atrocities or Balork are placed.

## Presentation, lighting and deviations

A-04's exact twelve B1 and seventeen B2 point-light coordinates are authored, including inward dogleg offsets. Lights are Movable, inverse square, unshadowed: T2200K/600lm/600cm, W2200K/1000lm/800cm, N6500K/1200lm/850cm. No basement directional light, particle flames or flicker. One nonshadowing SkyLight per floor, intensity B1 .50/B2 .40, real-time capture disabled, specified `/Engine/EngineResources/GrayLightTextureCube.GrayLightTextureCube`, white lower hemisphere. That built-in source has not been neutral-card calibrated; ambient comparisons require host review. No external HDRI or new binary is authored.

One unbound PostProcessVolume uses Manual physical-camera exposure at EV1006: ISO100, f/2, shutter1/16 second, compensation0, bloom0 and motion blur0. Equal automatic exposure bounds6 are also retained, but Manual metering avoids depending on the project extended-luminance setting. Neutral-card calibration and effective local exposure/renderer configuration still require host review; project-wide renderer settings are integrator-owned and unchanged.

Deliberate graybox deviations: fixed 120 cm visual wall cutaway with invisible full-height blockers (Z -150..400), rather than runtime camera-wall fading. Internal doorway lintel remains full visible geometry; A-04 fixtures are light-only to preserve clearance. Gray engine cubes stand in for A-02 masonry/paving; no final material palette, damp-band texture or licensed reference art. Flush B1 hub motif and B2 Bay threshold, healer cross and branch pier are original nonblocking primitive cues; stairs use stripe silhouettes rather than final A-03 arrow assets. B2 coping is inside the void edge, with full-height invisible collision. Largest-creature *visual* review is deferred to the visuals lane; no rendered screenshot, approved art or camera judgement is claimed.

## Validation and handoff

See the attempt report for exact observed build/generation/test commands, timings, exit codes and evidence. The generator logs a sorted MD5 fingerprint of generated actor names/classes/transforms/tags and portal/spawn identities; it excludes package metadata and does not certify all component properties. Checks still required on the integrated host: generate twice and compare authored IDs/transforms; rebuild/cook navigation; controller walk both floor routes and branches; traverse every paired portal after loading hub/B3; validate facing, quit/reload and invalid-destination recovery; test protected-zone attack/leash rules; inspect lighting, small-target readability and limbs at camera extremes. No Windows checks are claimed (deferred). W3-04b's known base-test fixes are outside these owned paths.
