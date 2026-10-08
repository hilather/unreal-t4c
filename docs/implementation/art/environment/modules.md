# Modular dimensions and assembly schedule

Read the [kit overview](README.md). **Every dimension, count, coordinate, slope, pivot, snap and budget below is Prototype (P)**. Base grid and clearances are inherited V-01; new module engineering is A-02, LH_Prototype_v1, 2026-10-08, source URL null. Layout filenames/region IDs identify existing proposals; this document does not rewrite them.

## Snap and pivot contract

Unreal centimetres, +Z up, applied scale 1. Layout unit U=100 cm; nominal placement grid 100 cm, long wall/floor repeats 200 and 400 cm. Use **5 cm fine plan snap** for specified offsets, 20 cm shared wall strips, 25 cm boss half-widths and 50 cm stair positions. This finer snap supports the existing decimal coordinates; do not round the layout to a whole metre. Calculated slope intersections may require non-grid Z. No global source-pixel scale exists.

For walls, local +X runs along the wall; +Y points into the room. Pivot is bottom-left on the interior collision face: straight wall occupies X[0,L], Y[−20,0], Z[0,400]. The stated room bounds are clear floor inside that face. Rotate around +Z for cardinal walls. Build shared partitions once within their assigned 20 cm strip; both rooms use the opposite faces of the same wall.

For floor modules, pivot is the lower-plan corner on the finished top at Z=0; slab extends down 20 cm. For door panels, pivot is aperture center at finished-floor height, +X along opening, +Y into room; local clear box is X[−W/2,W/2], Z[0,H], through all wall thickness. For props, pivot is floor center under the full footprint; wall fixtures use backplate contact center. For ramp flights, pivot is the start center on the walking surface, +X along travel, ±Y transverse; geometry uses Z=rise×X/run, plus the approved decorative tread profile. These pivots must be included in export notes.

Wall relief, baseboards, caps, pilasters and door ornament extend outward or upward from the reserved clear prism. A bevel must not shrink an opening. Across rooms with a single shared wall, keep ornament inside the 20 cm strip or use shallow surface material detail; a repeated extra wall skin must not consume either room's clear floor. Cut slabs at stair footprints rather than cover a descending ramp with a flat collision lid.

## Structural catalog

Proposed presentation IDs below expand as `Presentation.Environment.` + suffix. Mesh name examples are `SM_LH_` + suffix with periods converted to underscores. Numeric suffixes describe this candidate's dimensions; logical IDs must be frozen before saved content depends on them. Counts/variants are P, not runtime registry entries.

| Suffix / module | Exact P dimensions, cm | Assembly / collision |
|---|---|---|
| `Shared.Floor400`, `.Floor200`, `.Floor100` | 400×400×20; 200×200×20; 100×100×20 | Flat top at Z0, collision box underneath; consistent UV scale. Share paving/floor material variants. |
| `Shared.FloorInfill` | Final footprint X[0,L]×Y[0,W], thickness20; L/W cut in 5 cm increments to the listed clear bounds | Builder-authored original rectangular trim family, not arbitrary scaling of a textured slab. Includes 20 cm wide threshold strips and 320×320 landing assembly. Cut UV density remains constant. |
| `Shared.Wall400`, `.Wall200`, `.Wall100` | L=400/200/100; T20; H400 | One simple collision box per straight. Base pivot on clear face. Church plaster/timber or basement masonry are material/face variants. |
| `Shared.WallInfill` | L=5…195 in 5 cm increments, T20,H400 | Generated/authored cut-length family for exact aperture/partition endpoints; no need to ship every length before used. Same trim UV scale and box collision. |
| `Shared.CornerInner` | Exterior L footprint is union X[−20,100]×Y[−20,0] and X[−20,0]×Y[0,100]; H400 | Pivot clear-floor vertex (0,0,0), room quadrant +X,+Y. Two box blockers sharing a face, no duplicate corner cube. Neighbour walls begin 100 cm from vertex. |
| `Shared.CornerOuter` | Solid northeast corner: union X[0,100]×Y[0,20] and X[0,20]×Y[20,100]; H400 | Pivot projecting exterior vertex (0,0,0); exposed clear faces run along +X at Y0 and +Y at X0, with walking floor south/west of them. Connection markers (100,0) and (0,100); orient adjoining straights to keep their clear faces aligned. Remaining solid quadrant is supplied by adjoining room/void boundary construction, not a walkable hole. |
| `Shared.JunctionT` | Wall-axis envelope 200×120,H400; crossbar X[−100,100],Y[−10,10], stem X[−10,10],Y[−110,−10] | Pivot wall-axis junction; three branches of T20. Intended for explicit partition intersections only. Clear faces lie 10 cm off axes: solve placement from layout faces, never drop this axis pivot on a clear-room corner. |
| `Shared.WallEnd` | 20×20×400 | End-cap finish inside existing end volume, not an extra 20 cm length. Match visible texture around exposed baffle ends. |
| `Church.Beam400`, `.Beam200` | L400/200 ×20×20 | Pivot at center of near end face; local X[0,L],Y[−10,10],Z[−10,10]. Place lower face above clear headroom. Rendering/occlusion child, no unexpected separate corridor blocker. |
| `Church.Pillar` | 60×60 footprint, H400 | Simple box; optional structural/dressing prop only in a reserved room-edge bay. Never insert at B1 hub center or a listed doorway. |
| `Basement.Pilaster` | 40 along wall ×20 depth ×400 high | Recess into wall strip or place outside clear floor. Not a license to subtract 20 cm from corridor width. |
| `Shared.Ceiling400` | 400×400 plan,20 thick; underside ≥400 above local floor | Optional render panel/cutaway child. Collision only if separately adopted by map owner; keep stair ceiling underside ≥300 above each local tread. |

Inside/outside corner definitions are local bounding footprints, not wall-centerline guesses. Provide connection markers and show each orientation in the future kit test map. If a corner would overlap an adjacent short doorway, trim the straight/infill rather than covering the aperture with the corner leg. T pieces do not create new source partitions.

## Doorways, arches and open mouths

Door panel depth20 and height400 match the wall; jamb faces end at the clear aperture boundary. For ordinary panels use a 400 cm placement bay; for boss panels use a 600 cm bay. Collision is two side boxes plus one lintel above the clear height. Each listed opening continues through the entire wall and shares a flush floor threshold. A framed opening is an architectural mesh, not an interactive door.

| Suffix | Placement bay / clear opening W×H (P cm) | Required location / exact side fill |
|---|---|---|
| `Shared.Door240` | 400 / **240×300** | B1 enclosure E01, B3 ordinary mouths, B4 C01–C03. Left/right solid side fill 80 cm each, lintel100 high. |
| `Shared.Door320` | 400 / **320×300** | Hub doorways; B1 C01–C03 and B2 corridor mouths may omit visible trim. Side fill40 each, lintel100. |
| `Shared.StairOpening300` | 400 / **300×300 minimum above local ramp** | B3/B4 stair wall cuts. Side fill50 each; vertical position/cut follows ramp as below. Do not install a lintel at Z300 relative to map origin. |
| `Basement.DoorBoss500` | 600 / **500×360** | B4 C04, D05–D07. Side fill50 each, lintel40. Full rectangular clearance mandatory. |
| `Shared.Arch240`, `.Arch320` | Same bay/depth as matching door; clear box unchanged | Decorative shallow arch underside ≥300 across full aperture, crown340; upper envelope400. Do not replace rectangular clearance with a 300 cm crown-only semicircle. |
| `Basement.ArchBoss500` | Same 600 bay,depth20; clear box500×360 | Shallow underside rise360→380 toward center; upper envelope400. Recessed/raised face treatment remains outside the clear box. |
| `Church.DoorLeafPreview` | Pair of 160 wide ×300 high ×8 deep leaves for the 320 opening | Optional future pivot at outer hinge edge; currently open-aperture layout uses no leaf collision/lock. Storage of open leaves must sit outside the route; requires map/gameplay task before use. |

Ordinary/boss panels can be assembled from side infill and a lintel instead of a monolithic mesh. Jamb ornament occupies the solid side fill, never the opening. Missing wide openings must not be solved by horizontally scaling a smaller arch.

**Unframed open seams:** B1 hub bays are 400 cm fully open, B2 HookInterior A06 is 800 cm and BayJunction A03 is 1000 cm fully open. Use flush floor/infill and no wall/door across these seams. B2 ordinary corridor mouths are 320 cm, not a universal 240 cm doorway. Ordinary corridors are 320 cm, B2 ring lanes are 700 cm before dressing, hub street strips 400 cm, and B4 C04 is 550 cm with 600² cm pads. These remain their layout-owned widths, not sizes to normalize to the module length.

## Stair and landing library

Produce these exact profiles instead of scaling one flight. All have **300 cm clear width**, side blocking/rails outside ±150 cm and at least **300 cm overhead clearance above local walking height**. Smooth ramp collision carries the actor; decorative treads have collision disabled. P modeling starts divide each profile into 10 cm risers; that is appearance, not a new CharacterMovement step rule. Slope and tread arithmetic below is P, not observed navigation.

| Suffix / profile | Horizontal run; total rise/fall; tread proposal | Layout uses |
|---|---|---|
| `Shared.Stair400x100` | Run400; signed height ±100; 10 treads with 40 run and10 rise; grade1:4 (≈14.0°) | Hub descent along −X, B1 S02 descent along +Y. |
| `Shared.Stair600x120` | Run600; signed height ±120; 12 treads,50 run/10 rise; grade1:5 (≈11.3°) | B1 S01 and B2 entry ascent along −Y; B2 descent along +Y. |
| `Shared.Stair250x80` | Run250; signed height ±80; 8 treads,31.25 run/10 rise; grade80:250 (≈17.7°) | B3 entry ascent along +Y. |
| `Shared.Stair300x80` | Run300; signed height ±80; 8 treads,37.5 run/10 rise; grade80:300 (≈14.9°) | B3 descent along +Y; B4 return ascent along −X. |
| `Shared.Landing320` | Clear320×320; slab thickness20; fixed finished surface at layout endpoint Z | Flat approach/terminal landings below. Assemble exact footprint from floor/infill or author a dedicated piece. |
| `Shared.StairSide` | Thickness20 outside clear width; bottom extends below lowest flight point by20; top follows reviewed enclosure | Length/profile-specific side trim and box/convex blocking prevent gaps under a descending wall. No railing intrusion. |

Tread divisions internal to a flight need not snap to the world plan grid. Export the assembled run/end precisely; never round 31.25 cm tread runs or accumulate placement drift. Place the collision ramp at the intended foot-contact line, author treads to straddle it with modest relief, and review foot sliding/penetration. If visible tread contact fails, revise decorative relief/IK with controls owner rather than changing gameplay step height silently.

| Use | Endpoint / landing P schedule in layout U (100 cm) |
|---|---|
| Hub wing | Start `(−8,5,0)` → `(−12,5,−1)`; no added terminal level pad. Keep original flat arrival reserve. |
| B1 entry S01 | Y−20→−26, Z0→+1.2; approach X[2.9,6.1],Y[−20,−16.8],Z0. |
| B1 descent S02 | Y18→22,Z0→−1; approach X[43.4,46.6],Y[14.8,18],Z0. |
| B2 entry S01 | Y−58→−64,Z0→+1.2; approach X[3.4,6.6],Y[−58,−54.8],Z0. |
| B2 descent S02 | Y61→67,Z0→−1.2; approach X[51.4,54.6],Y[57.8,61],Z0. |
| B3 entry | `(32,9.5,0)`→`(32,12,0.8)`; terminal X[30.4,33.6],Y[12,15.2],Z+0.8. |
| B3 descent | `(47,66,0)`→`(47,69,−0.8)`; terminal X[45.4,48.6],Y[69,72.2],Z−0.8. |
| B4 return | `(2.5,6,0)`→`(−0.5,6,0.8)`; terminal X[−3.7,−0.5],Y[4.4,7.6],Z+0.8. |

The layout files supply the complete transverse footprints, triggers and arrivals; do not infer missing X anchors from a stair asset's pivot. Reuse via rotation and signed-profile variant, not negative actor scale. These are cosmetic portal approaches in separate maps, not physically connected storeys; do not force endpoints from paired maps to share elevation or add another travel edge.

**Mid-slope wall cuts:** B3 descent at Y68 has local ramp Z=−53⅓ cm, B4 return at X0 has Z=+66⅔ cm, B3 entry at Y12 has Z=80 cm. Continue each opening through the wall thickness and measure overhead from the highest local tread/ramp within that thickness. Use a cut-to-profile lintel/enclosure variant; a stock floor-Z0 lintel can reduce headroom. Ascending flights may require a local header above the nominal 400 cm wall if their endpoint plus 300 cm headroom exceeds it (for example 120+300=420 cm). Add that reviewed local extension/cutaway rather than lower the clearance. Side/back blockers close the terminal beyond the trigger without capping the ramp.

## Church roof, furnishing and fixtures

| Suffix | Exact P geometry / envelope, cm | Placement restriction |
|---|---|---|
| `Church.RoofNaveSlope` | Repeat 400 along ridge; slope half-span800 horizontal, rise200; surface length≈824.62, thickness20 | Two slopes cover the inherited 1600 cm nave width with eaveZ400/ridgeZ600; six 400 cm segments along 2400 cm length. Separate gable end caps, no invented central roof hole. No default overhang into routes. |
| `Church.RoofWingSlope` | Repeat200 along ridge; half-span500, rise150; surface length≈522.02, thickness20 | Lower wing roof eaveZ400/ridgeZ550 (A-02 P), ridge follows wing X length1180 with cut end infill. Eave matches the wall top; keep render roof cut away over occupied wing and ramp. |
| `Church.GableNave` / `.GableWing` | Triangular face base1600,height200 / base1000,height150; thickness20 | Fit above respective eaves, no gameplay blocker unless adopted by map owner. Timber face/rim shares trim atlas. |
| `Church.RoofEdgeTrim` | Repeat400 or cut infill along edge; horizontal width20, thickness20 | Cover the outward20 cm wall-top strip beyond the nominal clear-room roof span; terminate flush with exterior wall face. Same treatment closes ridge-end/gable wall strips. No added projection into routes. |
| `Church.Pew` | 300 wide ×80 deep ×100 max high, seat45 high | Fits within a V-01 pew block300×200; can place paired pews with a 40 cm gap inside that block, not into cross-aisles. Simple single box collision if adopted. |
| `Church.Altar` | Plinth400×100×20; table/mass≤400×100 footprint, total top≤120 above floor | Inside nave X[6,10],Y[22,23] U. Keep front approach clear and avoid a new interaction/relic requirement. |
| `Basement.BalorkFixture` | Entire solid/visible footprint≤400×400; total top120 | B4 X[14,18],Y[70,74] U. Stepped mass with restrained low central relief; no tall extension beyond this envelope. |
| `Shared.Barrel` | Diameter60,height90 | Optional alcove decoration; box/convex blocker only outside routes and pads. |
| `Shared.Crate` | 60×60×60 | Same; do not turn into loot or navigation cover without gameplay review. |
| `Shared.RubbleEdge` | 100 along wall ×30 deep ×15 high | Room-edge recess only; corridor version is a flush decal/material mark with no projecting heap. |
| `Shared.Sconce` | Backplate20×10×40; full bracket/flame envelope30 wide ×30 deep ×80 high | Mount contact centerZ250 as P starting rule. Keep projection outside clear routes; use a recess/light-only cue where no recess exists. Fixture/light are separate. |
| `Shared.LampRecess` | Wall opening30 wide×10 deep×60 high, retained rear wall thickness10 | Fits within20 cm wall thickness; glow/fixture contained, no new traversable/Visibility gap. Use in narrow shared walls. |
| `Church.CandleStand` | Diameter40,height100 including flame | Altar/room edge only; no corridor floor placement. Nonblocking flame; light actor budget shared with other fixtures. |

Roof segments are deliberately large custom assemblies following the modest inherited envelope, not a requirement for cathedral rafters. The low wing roof is presentation only; its cutaway and lack of new overhead blocker are essential for the proposed elevated view. If map art review changes an elevation, the map owner records the reconstruction change.

Hub street/causeway edges need cut/spline trim: its diagonal vectors include (−8,+6), (−16,+8) and (−3,+6) U. Preserve 400 cm **perpendicular** width and 400×400 cm clear bend pads; cardinal/45° corner pieces alone cannot trace it. Do not round the path, add a bridge or cook a terrain-wide nav surface just to avoid a trim piece.

## Assembly checks before map dressing

Use a measured ruler assembly for 240/300/320/500 cm openings and each flight, plus common/boss turn pads. Confirm door side fills sum to placement bay, stairs meet flat pads with no lip, thresholds span shared20 cm walls, inner/outer/T pieces have no overlapping collision seam, and LODs preserve the clear box. Probe B2 ring void/gap, B3 narrow baffles and B4's blocked southern EastChamber gap; none may become a new shortcut. Scope kit changes separately from map-owned circulation fixes.

All checks above are requirements for future assets. A-02 performed document arithmetic and source/reference inspection only; no asset importer, navmesh bake, collision sweep, rendered screenshot or gameplay result is delivered here.
