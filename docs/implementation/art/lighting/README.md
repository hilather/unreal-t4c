# A-04 — Graybox lighting and elevated-camera readability

First-pass builder guide, **2026-10-08**, contract revision 1, base `7e08383c3f8d66955bb5b91f6dcddf9788841e95`. This delivers proposed settings and six original text SVG plans, **not implemented lighting, Unreal screenshots or a performance pass**. W3-01 owns the hub, W3-02 B1/B2, W3-03 B3/B4; the integrator owns project renderer settings and presentation implementation. W5 map owners can retain the same cues when replacing grayboxes.

**Every design number is Prototype (P)**: positions, colours, Kelvin, lux/lumens, radii, ambient/exposure values, counts, milliseconds, contrast goals and fade timings. New proposals have provenance `A-04 / LH_Prototype_v1 / 2026-10-08 / source_url: null`. Existing A-02/A-03/V-01/V-03/controls numbers remain inherited P. Calculations are checks on proposals, not observed in-game results. Dates, revision IDs and documented engine limits are evidence metadata, not historical T4C tuning. No mechanics or enemy roster is changed.

## Approved direction and governing sources

Use [layout conventions](../../layout/README.md), [hub](../../layout/hub-temple-district.md), [B1](../../layout/b1.md), [B2](../../layout/b2.md), [B3](../../layout/b3.md) and [B4](../../layout/b4.md) for floor unions, doors, stairs and protected zones. These SVGs locate light cues; their schematic room outlines **do not replace aperture/collision plans**. An apparent touching rectangle is not a new doorway. The floor tables and existing paired travel records remain authoritative proposals for construction.

The [A-02 kit](../environment/README.md), [modules](../environment/modules.md) and [reference record](../environment/references.md) supply modest masonry, gray paving, red aisle, dark timber, amber fixtures and cool fill. Preserve B1 dry stone, B2 damp lower bands and ring coping, B3 pale patches/capped partitions, B4 dark gray-brown stone and stronger amber rhythm. Darkness comes from quieter background surfaces and isolated pools; it must not erase the walkable floor or an enemy. Do not add cathedral scale, coloured magical fog, new portals or a red boss-room wash.

[A-03](../placeholders/README.md) supplies unchanged creature/player palettes, state marks, labels and up/down arrow shapes. [V-03](../../ui/style/README.md) owns opaque charcoal UI, amber focus and cool information. [Controls](../../controls.md) supplies the actual **45° horizontal FOV**, overriding V-01's unadopted vertical-FOV wording. [Dev maps](../../dev-maps.md) supply the movement/LOS fixtures. [Architecture §6](../../../plan/docs/02-unreal-architecture.md#6-church-basement-areas-and-enemy-lifecycle) and [§8](../../../plan/docs/02-unreal-architecture.md#8-ui-graphics-and-validation-targets) require cutaways, scalable lighting and measured performance.

The recorded toolchain is UE 5.8.3; [Toolchain.md](../../../../build/Toolchain.md) includes later engine/build evidence after its historical engine-absence audit. At this base `Config/DefaultEngine.ini` does not explicitly pin renderer or exposure settings. The settings below require adoption and actual renderer inspection; no project setting is changed here. Linux/brewtop with the brief's GTX 1050 Ti is the first review target. Windows validation remains deferred.

## Lighting approach and budget

### First graybox pass

Use conventional raster lighting, non-Nanite primitive meshes, **Shadow Maps**, no Lumen GI/reflections, no hardware ray tracing, no virtual shadow maps and no real-time sky capture. Disable volumetric fog/clouds, bloom, lens flare, motion blur, depth of field, chromatic aberration and local exposure for the readability baseline. Use shared opaque rough materials. These are cost-control proposals, not a measured statement that a particular feature cannot run on the GPU. Do not change the project's entire rendering path or install a plugin for this task.

While walls move, use **Movable** local point lights so a missing bake is not mistaken for final lighting. Most are unshadowed. Start with one fixed Movable directional light in the hub, conventional dynamic shadows, 6500 K, **2000–4000 lux**, starting at 3000 lux; proposal rotation yaw45°, pitch−45°. This is a reduced-intensity graybox daylight rig, not a measured outdoor sky. Use no directional light in basement maps. Fixed sun direction avoids introducing time-of-day work. A stationary exterior directional light becomes an option only after a tested static-lighting workflow exists.

Each loaded map gets **one neutral SkyLight**, nonshadowing for the temporary fill baseline, with a fixed neutral captured/specified cubemap and **Real Time Capture off**. Its unitless intensity scale is in the area tables. Record the actual capture/cubemap and Lower Hemisphere settings; a different source invalidates comparing the multipliers. Use an achromatic calibration source, with the 6500 K neutral intent from A-02; this is not a literal SkyLight temperature control. Author that source during map work, not an external HDRI download. The hub interior shares the exterior SkyLight and exposure: do not stack an interior SkyLight. Dungeon fill is an intentional visibility floor, not simulated underground daylight. Do not call it baked indirect light.

Lock exposure before comparing areas: one unbound Post Process Volume per map, **Min EV100 = Max EV100 = 6**, Exposure Compensation 0, no adaptation/local exposure, no second camera override. This initial lock is a calibration proposal; adjust the shared lock once using a neutral gray card if needed, then record it and use it across all six areas. Equal min/max disables adaptation; do not also add a compensating per-floor exposure change. Keep the same postprocess white balance and neutral colour grading. Different floor brightness must survive a fixed exposure. If the owner uses Manual metering instead, match the reference card first and record Apply Physical Camera Exposure and camera settings explicitly. [Epic exposure documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/auto-exposure-in-unreal-engine), retrieved 2026-10-08.

A neutral fill cannot substitute for structural shadows everywhere. Unshadowed local lights can leak through partitions: inspect adjacent rooms, baffles and the B2 gap. Reduce radius, place the emitter on the lit side or replace the point with a downward/inward spot before adding shadow casters. Never solve a leak by deleting collision or making a wall fully emissive. If a marker remains unreadable, raise local neutral coverage or simplify its background before lowering the whole floor's light level.

### After layout freeze, optional cheaper steady-state path

Bake static torch/fill lighting with Lightmass only after the integrator enables Allow Static Lighting, disables Lumen, permits precomputed lighting in each map and supplies valid lightmap UVs. Build and inspect lightmaps plus the volumetric lightmap used by moving actors. Keep a limited dynamic key for actor grounding if necessary. A static fixture can keep a tiny emissive flame, but a baked light cannot flicker dynamically. Static lighting is a conditional follow-up, not a prerequisite for the changing graybox. [Epic static lighting](https://dev.epicgames.com/documentation/en-us/unreal-engine/static-light-mobility-in-unreal-engine) and [Lumen/precomputed lighting](https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-global-illumination-and-reflections-in-unreal-engine), retrieved 2026-10-08.

Do not make every torch Stationary. Stationary lights have at most four overlapping shadow channels, sometimes fewer because of allocation topology; a Stationary directional light consumes a channel across the map. Inspect Stationary Light Overlap and avoid dynamic-shadow fallback if that path is adopted. Our lower local-light budget below applies even when channel allocation is valid. Conventional stationary shadow techniques also constrain Nanite use, supporting conventional graybox meshes here. [Epic stationary lighting](https://dev.epicgames.com/documentation/en-us/unreal-engine/stationary-light-mobility-in-unreal-engine) and [Virtual Shadow Maps](https://dev.epicgames.com/documentation/en-us/unreal-engine/virtual-shadow-maps-in-unreal-engine), retrieved 2026-10-08.

### Proposed runtime ceilings

Count **influences on visible geometry**, including through-wall attenuation and adjacent-room lights, not only lights whose origins are in the current room. The map tables list authored placements, not simultaneous activation requirements. Static baked emitters have a different runtime cost but still need lightmap memory/cook review.

| Room / view | Local dynamic-light allowance | Shadow allowance | Proposed lighting + shadow GPU share |
|---|---|---|---|
| Hub exterior at far zoom | Up to 4 visible service-door locals; no lamp on every causeway bend | Directional only; 0 local casters | ≤2.5 ms for full visible lighting/shadows |
| Hub service interior / ordinary basement room | Up to 3 locals; 4 for Nave, B1.Descent including enclosure, B2.LongHall edge overlap or B4 arena | Start 0; at most 1 local caster per room, only if an occluder/actor requires it | ≤1.0 ms single-room contribution; ≤2.5 ms complete view |
| Connector / landing | Up to 2 locals influencing its floor; use shared spill where sufficient | 0 additional casters | Included in room/view budget |
| Balork complex and Hall approach | Up to 4 locals on any visible surface, including F01; up to 8 visible across complex | F01 may be the sole arena caster after review; at most 2 local casters in the complete view | ≤3.0 ms complete lighting/shadow share |
| Any worst far-zoom view | ≤12 visible dynamic local lights; ≤4 overlapping at any point | ≤2 local casters, plus hub sun only where present | ≤3.0 ms ceiling, included within whole-frame target |

A-02's inherited ceiling is ≤4 overlapping locals and ≤2 local shadow casters on the player; A-04 starts more conservatively. The **whole frame** target is 60 fps / 16.67 ms, not 16.67 ms for lighting. A 30 fps / 33.33 ms fallback is evaluated later without calling it a 60 fps pass. These budgets are allocations to test on the actual GPU, not observed timings or additive per-room guarantees.

Low-cost comparison: keep exposure, ambient, route/arrival illumination and state cues fixed; disable all local shadows first, reduce the sun's shadow distance/quality, then remove redundant decorative locals. Retain F01's neutral fill and the doorway/stair profiles. Flames can remain tiny static emissive meshes with no extra point light. Start with no particle flames, no per-ember lights and no brightness flicker. If still over budget, adopt the reviewed bake path or revisit geometry/material cost; do not hide enemies in darkness to save time. No automatic room-light toggling, visible pop-in or unimplemented lighting manager is assumed.

## Fixture profiles and coordinates

Positions use each layout's **local XYZ in U = 100 cm**, not source-image pixels. The plans deliberately use top-down +X right / +Y up, unlike the reference's diagonal projection. Room keys Rxx and fixture keys Exx/Ixx/Txx/Fxx are **document aliases scoped to that SVG**, never entity IDs. Every row inherits the profile's Kelvin, intensity and attenuation ranges, its area's ambient and the budget above. Use the midpoint as an initial value and record actual settings after calibration.

| Profile | Emitter / A-02 palette | Intensity proposal | Attenuation radius proposal | Mount / shadow start |
|---|---|---|---|---|
| T — ordinary torch/door lamp | Point, inverse-square falloff; 2200–2400 K (start 2200), A-02 amber | 400–800 lm, start 600 | 500–700 cm, start 600 | Z250 cm above local flat floor; no shadow |
| W — landmark torch/fixture | Same amber; 2200–2400 K | 800–1200 lm, start 1000 | 700–900 cm, start 800 | As table; no shadow; larger pool, no animated beacon |
| N — neutral route/arrival | Point, inverse-square; 6500 K neutral from A-02 | 900–1500 lm, start 1200 | 750–950 cm, start 850 | Z250 cm; no shadow; fixture housing may be warm stone/iron |
| F — arena overhead fill | Down-facing Spot, inverse-square; 6500 K | 2400–4000 lm, start 3200 | 1500–1900 cm, start 1700 | Z600 cm; inner/outer half-angle 50°/65°; shadow off initially, optional single caster |

The F cone at floor distance 600 cm reaches approximately 12.9 U at its outer edge; ambient and nearby T/W pools cover remaining corners. Cone coverage and falloff are **not** a certified boss-silhouette result. F01 is a light-only authoring device with no new roof hole, hanging prop or gameplay effect. The B3.C3Cell row explicitly overrides T intensity/radius to avoid filling the entire surrounding room.

Use A-02 `Shared.Sconce` / `Shared.LampRecess`, dull iron `#535451`, warm stone `#78634B`. Fixtures are distinct from light actors. At an aperture or narrow shared wall use a recess or light-only cue: no opaque bracket occupies the clear route. Light centers may sit slightly inside clear floor because they have no collision; emitter must be on the intended illuminated side of masonry. Keep backplate/mesh outside the route prism. B2 dogleg and B4 approach rows explicitly move emitter centers inward from V-01's fixture cue coordinates while retaining those cues. Do not move portals, NPC pads or walls to accommodate lights.

Light-source temperature affects illumination; it does **not** recolour material tokens. Never apply `#E9BF79` or `#A9C4CC` as an additional saturated colour filter on top of the Kelvin control. Those are UI/state colours. All mesh-base hex colours are unlit sRGB; convert once to linear material parameters. No GI is expected from A-03's small emissive marks in this baseline.

## Area placement schedules

The hub exterior plan covers the full connected service district, including the winding mage route. All street/causeway strips without a local fixture use the shared daylight and neutral fill; readable pale sand and low edge geometry are the route cues. Never add a glowing bridge/portal or put fixture props on bend pads. The interior plan uses the same map frame and includes all required service interiors; shared exterior E fixtures count toward their budgets. Unlisted B1 bays, B2 optional connectors, B3 recesses and B4 corners retain the ambient visibility floor; no enemy corner is deliberately black.

### Hub exterior

Plan: [hub-exterior.svg](hub-exterior.svg). **H: SkyLight 0.70 (0.55–0.85); same map as interior**. Daylit service route · warm thresholds · pale winding sand.

| Plan ID | Layout region | Light center (X,Y,Z), U | Profile | Placement / landmark |
|---|---|---|---|---|
| `E01` | `Temple.Forecourt` | `(10,-0.3,2.5)` | T | Nave door jamb; read Samaritan and church entry |
| `E02` | `Temple.SigfriedShop` | `(-20,7.7,2.5)` | T | Exterior jamb, clear of WestLane door |
| `E03` | `Temple.FaliRolphShop` | `(-12,-11.7,2.5)` | T | Exterior jamb; shared service door |
| `E04` | `Temple.TrainingHouse` | `(14,-21.7,2.5)` | T | Exterior jamb; training threshold |
| `E05` | `Temple.KalastorHouse` | `(14,-51.7,2.5)` | T | Exterior jamb; retain separate south route |
| `E06` | `Temple.MurmuntagSquare` | `(-21,-62,2.5)` | T | Light-only edge cue; no invented trainer building |
| `E07` | `Temple.MageVestibule` | `(-31,59.7,2.5)` | T | Exterior jamb; terminates winding sand route |

### Hub interiors

Plan: [hub-interior.svg](hub-interior.svg). **H: shared SkyLight 0.70; interior daylight controlled by roof policy**. Gray paving · red aisle · low roofs cut away on entry.

| Plan ID | Layout region | Light center (X,Y,Z), U | Profile | Placement / landmark |
|---|---|---|---|---|
| `I01` | `Temple.Nave` | `(8,22.5,3.5)` | W | Altar-side pool; light above fixture, not a save beacon |
| `I02` | `Temple.Nave` | `(0.4,14,2.5)` | T | Side-wall cue for Kilhiam; preserve cross-aisle |
| `I03` | `Temple.Nave` | `(15.6,20,2.5)` | T | Side-wall cue for Moonrock; preserve approach |
| `I04` | `Temple.DungeonWing` | `(-0.5,5,2.5)` | T | Inherited lintel cue; light-only anchor, recess mesh clear of doorway |
| `I05` | `Temple.DungeonWing` | `(-8,2.2,2.5)` | N | Covers ramp mouth and return arrival without occupying either |
| `I06` | `Temple.SigfriedShop` | `(-29.6,18,2.5)` | T | Side wall near Sigfried, clear service pad |
| `I07` | `Temple.FaliRolphShop` | `(-14,-23.6,2.5)` | T | Rear wall between vendors; keep both names distinct |
| `I08` | `Temple.TrainingHouse` | `(12,-33.6,2.5)` | T | Rear wall; Jagar Kar and Ortanalas remain distinct |
| `I09` | `Temple.KalastorHouse` | `(12,-63.6,2.5)` | T | Rear wall behind approach |
| `I10` | `Temple.MageVestibule` | `(-29,65.6,2.5)` | T | Clear hall mouth to two teachers |
| `I11` | `Temple.MageHall` | `(-35.6,81,2.5)` | N | Far hall/Iraltok turn; no false exit at hall end |
| `I12` | `Temple.UranosRoom` | `(-47.6,72,2.5)` | T | Rear wall and stone service badge |
| `I13` | `Temple.IraltokRoom` | `(-16.4,77,2.5)` | T | Rear wall; do not wash red flame badge red |

### B1 · three-arm hub

Plan: [b1.svg](b1.svg). **B1: SkyLight 0.50 (0.40–0.60)**. Dry warm stone · bright return · separately readable healer detour.

| Plan ID | Layout region | Light center (X,Y,Z), U | Profile | Placement / landmark |
|---|---|---|---|---|
| `T01` | `B1.Entry` | `(2,-20,2.5)` | N | Offset beside inherited (4.5,-20) mouth; whole Entry safe |
| `T02` | `B1.C01.EntryHub` | `(7.7,-5,2.5)` | T | Recess; Entry return connector |
| `T03` | `B1.Hub` | `(9,0.4,2.5)` | T | Entry arm; retain flush center motif |
| `T04` | `B1.Hub` | `(0.4,12,2.5)` | T | Healer arm; include NegXBay in neutral floor visibility |
| `T05` | `B1.Hub` | `(17.6,12,2.5)` | T | Onward arm; include PosXBay in neutral floor visibility |
| `T06` | `B1.Healers` | `(-12,12,2.5)` | W | Inherited healer landmark; light-only anchor if no wall mount |
| `T07` | `B1.Healers` | `(-31.6,10,2.5)` | N | Both NPC bodies/badges; no moving or emissive service beacon |
| `T08` | `B1.C03.HubDescent` | `(24,10.3,2.5)` | T | Recess; ordinary connector stays clear |
| `T09` | `B1.Descent` | `(40,10,2.5)` | W | Inherited enclosure-corner landmark; no mount across aperture |
| `T10` | `B1.Descent` | `(53.6,7,2.5)` | T | Outer perimeter/encounter read |
| `T11` | `B1.Descent` | `(33,25.6,2.5)` | T | Far perimeter rats must be visible |
| `T12` | `B1.Descent.Enclosure` | `(48.5,18,2.5)` | N | Protected landing and downward tread cue |

### B2 · ring and dogleg

Plan: [b2.svg](b2.svg). **B2: SkyLight 0.40 (0.32–0.48)**. Damp lower bands · visible void coping · two equal ring routes.

| Plan ID | Layout region | Light center (X,Y,Z), U | Profile | Placement / landmark |
|---|---|---|---|---|
| `T01` | `B2.Entry` | `(2,-58,2.5)` | N | Return stair and whole Entry/C01 safety |
| `T02` | `B2.C01.EntryRing` | `(8.7,-40,2.5)` | T | Return connector, recessed |
| `T03` | `B2.Ring` | `(2,-30,2.5)` | T | Inherited first ring cue; no prop in corner |
| `T04` | `B2.Ring` | `(20,-16,2.5)` | T | Inherited opposite ring cue; both lanes remain lit by fill |
| `T05` | `B2.LowerBay` | `(14,-0.5,2.5)` | N | Offset from flush return marker (11,0); no wall across seam |
| `T06` | `B2.Junction` | `(21.6,12,2.5)` | T | Central fighting floor and next mouth |
| `T07` | `B2.BranchJunction` | `(-8,14,2.5)` | T | Inherited branch pier cue; light-only if no recess |
| `T08` | `B2.BranchJunction` | `(-17,19.6,2.5)` | T | Upper branch mouth, not portal glow |
| `T09` | `B2.LeftDeadEnd` | `(-29.6,-24,2.5)` | T | Closed end with readable rat/slime corners |
| `T10` | `B2.UpperHook.Stem` | `(-8.4,35,2.5)` | T | Hook bend; retain blocked gap to LongHall |
| `T11` | `B2.UpperHook.Terminus` | `(-39.6,45,2.5)` | T | Quiet dead end; no stair icon |
| `T12` | `B2.Dogleg` | `(16.3,38,2.5)` | T | Emitter inside elbow; inherited fixture anchor (17,38) outside turn |
| `T13` | `B2.Dogleg` | `(-2.3,38,2.5)` | T | Emitter inside elbow; inherited fixture anchor (-3,38); no shortcut |
| `T14` | `B2.LongHall` | `(7,63.6,2.5)` | T | First hall pool; neutral ambient retains bat marks |
| `T15` | `B2.LongHall` | `(25,48.4,2.5)` | T | Second hall pool; avoid center route clutter |
| `T16` | `B2.LongHall` | `(36,59,2.5)` | W | Inherited onward-wing cue |
| `T17` | `B2.Descent` | `(55,61,2.5)` | N | Inherited stair cue; whole Descent/C08 protected |

### B3 · partitioned circuit

Plan: [b3.svg](b3.svg). **B3: SkyLight 0.32 (0.26–0.38)**. Cooler broad fill · pale baffle caps · warm Level 4 tip.

| Plan ID | Layout region | Light center (X,Y,Z), U | Profile | Placement / landmark |
|---|---|---|---|---|
| `T01` | `B3.Entry` | `(29,10,2.5)` | N | Inherited return cue; warm housing, neutral stair light |
| `T02` | `B3.LowerWest` | `(23.6,-6,2.5)` | T | Pale patch at (20,-5) and outer circulation |
| `T03` | `B3.C3Cell` | `(18.6,-3.5,2.5)` | T | Small recess: use 200–350 lm, radius 250–350 cm |
| `T04` | `B3.West` | `(-9.6,6,2.5)` | T | Wall stubs remain legible; no hidden slime at foot |
| `T05` | `B3.WestBay` | `(17.6,17,2.5)` | T | Closed bay, no new stair |
| `T06` | `B3.C03` | `(1.3,17,2.5)` | T | Recess return link |
| `T07` | `B3.UpperCentral` | `(1,37.6,2.5)` | T | Distinguish branch from circuit |
| `T08` | `B3.BranchNear` | `(-18,35.6,2.5)` | T | Short upper-left branch |
| `T09` | `B3.BranchEnd` | `(-43.6,33,2.5)` | T | Warm end wall by restrained dark-red patch; not portal |
| `T10` | `B3.C04` | `(14.7,26,2.5)` | N | Beside inherited (16,26) bend/void lip |
| `T11` | `B3.C04` | `(27.3,30,2.5)` | N | Beside inherited (26,30) bend; no cross-gap beam |
| `T12` | `B3.East` | `(29,45.6,2.5)` | T | C2 baffle cap; keep recessed interior readable |
| `T13` | `B3.LowerRight` | `(57.6,34,2.5)` | T | Divider ends and ordinary encounter floor |
| `T14` | `B3.C06` | `(51.3,6,2.5)` | T | Return elbow; no marker implying a third portal |
| `T15` | `B3.C07` | `(48.3,48,2.5)` | T | Recess along Level 4 arm |
| `T16` | `B3.Descent` | `(50,66,2.5)` | W | Inherited warm tip; light fixture outside 3 U run |
| `T17` | `B3.Descent` | `(42.4,60,2.5)` | N | Arrival floor separate from terminal trigger |

### B4 · Balork complex

Plan: [b4.svg](b4.svg). **B4: SkyLight 0.26 (0.22–0.32); arena F01 retained**. Dark gray-brown walls · amber rhythm · cool arena separation.

| Plan ID | Layout region | Light center (X,Y,Z), U | Profile | Placement / landmark |
|---|---|---|---|---|
| `T01` | `B4.Entry` | `(1,10,2.5)` | N | Inherited entry fixture; entire reserved pad and ramp remain readable |
| `T02` | `B4.C01` | `(3,17.3,2.5)` | T | Return dogleg elbow; recess/light-only |
| `T03` | `B4.Hall` | `(0.4,27,2.5)` | T | Entry mouth and rat near dark nonhazardous patch |
| `T04` | `B4.Hall` | `(21.6,38,2.5)` | T | Side branches and Hall apron |
| `T05` | `B4.WestChamber` | `(-27,34,2.5)` | T | Inherited side pool; no false onward cue |
| `T06` | `B4.EastChamber` | `(55,35,2.5)` | T | Inherited side pool beyond baffle |
| `T07` | `B4.EastChamber` | `(39,43.6,2.5)` | N | Northern baffle bypass; southern plinth stays closed |
| `T08` | `B4.C04` | `(8.55,52,2.5)` | T | Light-only inward offset from fixture (7,52); mesh stays outside clear corridor |
| `T09` | `B4.C04` | `(13.45,52,2.5)` | T | Light-only inward offset from fixture (15,52); both nonshadowing initially |
| `T10` | `B4.BalorkArena` | `(16,81,2.5)` | W | Inherited warm rear light silhouettes low fixture |
| `F01` | `B4.BalorkArena` | `(12,66,6)` | F | Overhead down-facing neutral soft fill; no visible ceiling prop |
| `T11` | `B4.Reliquary` | `(41.8,61,2.5)` | T | D05/D07 circulation; no treasure promise |
| `T12` | `B4.Ossuary` | `(45.8,79,2.5)` | T | Far pale forms read as decoration; D06/D07 stay clear |


### Landmark priorities and floor separation

| Area | Reading to preserve at default and far camera | Background treatment / exceptions |
|---|---|---|
| Hub exterior | Nave threshold, Samaritan on forecourt, each service front, continuous sand to mage apron | Daylight service hub; roofs remain opaque outside until they obstruct the approach. No dusk requirement. |
| Hub interior | Red aisle to low altar; I04 wing cue distinct from exterior exit; separate Uranos/Iraltok rooms | Gray paving, pale plaster, quiet timber; altar glow never implies interaction/save. All service badges need neutral fill. |
| B1 | Entry's neutral up-stair, three distinct Hub arms, warmer healer branch, enclosure mouth and down-stair | Dry broad warm floor. Keep flush Hub motif; do not install a center pillar. T06/T07 must reveal both NPCs without one merged plate. |
| B2 | Two ring lanes around dark void with readable coping; LowerBay return mark; both dogleg elbows; T16 onward wing | Restrained damp bands lower wall values, not wet reflective floors. UpperHook stays a quiet closed branch separated from LongHall. No beam crossing their blocked gap. |
| B3 | LowerWest cell/baffles, both circuit return routes, cool C04 bends, warm T16 Level 4 tip | Broader pale repair patches on walls; keep floor behind goblin skin/Atrocity uncluttered. BranchEnd/WestBay receive no travel arrow. |
| B4 | Safe return, Hall branch mouths, paired approach lamps, Balork silhouette against dark stone, low rear fixture, D05–D07 loop | Dark `#514B42` stone, selective amber, neutral F01 on wings/torso/weapon. No red wash, bloom or exposure drop on acquisition/death. Keep the return view readable after defeat. |

In B4 keep all light fixtures out of C04's 550 cm clear corridor, 500×360 cm door apertures and 600×600 cm turn pads. Preserve the Hall apron, all D05–D07 approaches and altar bypasses. The boss reference anchor is `(12,66,0)`; F01 sits above it but must light the **moving** 440 cm wing envelope and retreat route, not only that one pose. T10 sits behind the low fixture, leaving its top/side readable without a red field behind the wings. Dim or remove red stain decals behind combat silhouettes. No phase, arena lock, new altar interaction, defeat beacon or B4 descent is introduced.

## Elevated-camera occlusion and target visibility

Review the inherited controls at boom **900/1200/1800 cm**, yaw **0/45/90°**, pitch **−60/−55/−50°**, focus about **90 cm** above floor and **45° horizontal FOV** at 16:9. Boom length is camera-to-focus distance, not camera height. Its nominal height is `90 + boom × sin(abs(pitch))`: about **662–1649 cm** across the stated extremes and **1073 cm** at default. These are arithmetic projections before spring-arm collision. Test the collision-shortened camera at low ceilings/near walls too; do not silently enlarge FOV or move the player to make a screenshot pass.

The current controls implement boom collision, **not a roof/wall fade system**. W3/presentation owners must implement or explicitly use a temporary cutaway until that system exists. The following is a proposed behaviour contract, not code supplied by A-04.

| Surface | Fade/cutaway trigger | Proposed rendering behaviour | Invariant / recovery |
|---|---|---|---|
| Nave/wing/shop/mage roofs, ceilings and their beams/gables | Player occupies the room; also pre-cut the approached room when the camera-to-player/doorway view would be covered by its roof | Fade visual roof assembly to hidden over 0.15 s; group dependent beams/roof trim so no floating opaque strip remains | Structural collision and existing spring-arm query response stay unchanged. Do not introduce a roof collider where the graybox has none. |
| Camera-facing exterior/interior wall above low cap | Camera-to-player head/torso/feet or a currently eligible selected target intersects the wall's presentation proxy; include projected silhouette with 20 px at 720p /30 px at 1080p margin | Mask/cut upper wall above a proposed 80 cm cap, with the cut region enlarged around eligible silhouettes when needed. Door jamb/lintel trim shares that state | Keep lower footprint/edge legible; do not hide a whole distant room or reveal a through-wall enemy. A selected target must first pass normal gameplay LOS. |
| Partition ends, B1 enclosure, B3 baffles, B4 EastChamber divider | Same camera occlusion test, on individual segments; both sides of a threshold reviewed | Fade/cut upper obstructing segment, retain capped footprint and opening edges; no broad whole-floor fade | Geometry, nav, targeting/cursor Visibility and attack LOS blockers stay intact. Cutaway does not make a wall traversable or selectable through. |
| Pillars / solid LOS props / B4 altar | Only the camera-obstructing upper visual section after a valid player/target view test | Keep base footprint and a readable solid edge; locally cut top only if necessary. Never make entire pillar invisible | The dev LOS pillar remains a blocker. No outline or nameplate on an ineligible target through it. Altar remains within its existing footprint and is not interactive. |
| Void curbs, low boundary caps, blocked plinths, ramps, floor/stair arrows | Do not fade with nearby upper walls | Opaque stable silhouette and floor edge | B2 void and B4 southern EastChamber plinth stay visibly closed; no apparent route across black floor gaps. |

Use an **opaque/masked cutaway** with an optional dithered transition; no large translucent roof layers, glass walls, depth-test disable or whole-scene opacity. Evaluate dither under the actual antialiasing method at 720p; if it shimmers/ghosts, use a stable hard cut or short discrete switch as the graybox fallback. This guide does not require adding TSR. Preserve roughness/base colour on the retained wall; no bright emissive cut edge. Turn off shadows on purely removable roof/ceiling/upper decorative pieces at authoring time for this baseline so an invisible roof does not black out the room. Keep structural wall/pillar shadow policy deliberate and check its retained shadow with ambient fill.

Require **0.20 s continuously clear** before restoration, then restore over **0.25 s**; shared walls stay cut while either occupied/approached room still needs them. At travel/load, establish the occupied-room cutaway before the first visible frame; clear old-room fade state. Camera jitter at a threshold must not flash roofs over the character. Pause freezes optional fades or retains the current cut; it never restarts state cues.

Baked shadows cannot update with a runtime cutaway. Exclude removable visual roof/upper pieces from the bake where appropriate, or supply a deliberately reviewed permanent cutaway bake and actor fill. Do not claim material fading removed a baked shadow. Low-quality mode must preserve the same visibility logic even if dynamic shadows are absent. Visual occlusion traces must not alter the gameplay's Visibility-channel blockers or target eligibility.

## Gameplay readability rules

- **Stairs and paired portals:** light flat approach, tread direction and terminal trigger surroundings, not only a flame. Retain A-03's single-bar cool Up arrow / double-bar amber Down arrow plus `Up/Down to [destination]`. Arrows live on flat approaches, not floating over an unseen pit. Doorway lamps get no travel arrows. B4 has only the B3 return. Test every pair in both directions and from its arrival-facing camera.
- **Safe arrivals:** SVG blue dashed boxes are authoring annotations only. In game show empty, readable floor, never a force field, save shrine or glowing safe-zone border. Preserve complete protection specified by layout, including B1 Entry+C01, Healers+C02 and enclosure, B2 Entry+C01 and Descent+C08, and B3/B4 reserves and stairs. Lighting does not enforce no-spawn/no-patrol/no-attack rules. Map/AI/travel owners must test actual damage/leash/load behaviour; use inert proxies if those systems are absent.
- **Player and targets:** preserve A-03 opaque body/mark colours; keep a broad quieter floor patch under combat feet, without decorative high-frequency seams or red stains under red enemies. Cool player self bar and `You` remain distinct from amber selected-target corners. Selection has no filled ground disc. Use the existing optional depth-tested visible-surface outline and selected label; colour alone never substitutes for family silhouettes, facing marks or words. Selection cannot be the only way to notice an unselected hostile.
- **Interactables and services:** light NPC faces/large badges and both service approach pads independently. No light implies a new offer. Chests are conditional existing gameplay objects, not permissions to instantiate source chest symbols; loot badge/Empty text follows authority. Emission on a loot badge does not add a point light. A-03 screen labels retain opaque `#242424` plates and off-white text outside world exposure/grading; gameplay eligibility/occlusion still decides whether to show them.
- **LOS blockers:** expose the pillar/baffle base, cap, both free ends and floor path around it. A shadow may reinforce its footprint but never create a fake floor gap. With local shadows off its geometry must still read. Repeat `Dev_Combat`'s LOSBlockedPosition/LOSClearPosition selection and impact-time rejection after cutaway adoption. The faded pillar must not become click-through or attack-through.
- **Dark corners:** all authored encounter anchors and legal pursuit paths retain enough fill to see body mass before combat. No A-04 hidden-enemy/ambush exception is authorized. If a future owner deliberately adds one, record the encounter change and provide a visible silhouette/motion cue at the corner before exposure to attack; sound may supplement but not replace a visual cue. Until that review, raise fill or clear the background. Darkness is not a new stealth mechanic.
- **State cues:** retain A-03 front-mark windup, brief accepted-hit white blend, compressed/static dead shapes, cool loot diamond and non-replayed respawn mark. No global tint, flash, additional dynamic light or lighting change on hit/death/aggro. Reduced-motion checks use static marks/text. Do not infer aggro, accepted damage, loot, respawn or save success from lighting.

## Placeholder contrast check — analytic warning, runtime result pending

The table below is a **flat-colour stress calculation**, not a Kelvin simulation, Lightmass bake, screenshot or proof of readability. It deliberately tests weak pairs before asking map owners to capture them. It includes all eleven creature families even where absent from a floor; off-roster columns are palette stress tests, not new spawn assignments. Human rows cover the player and shared NPC base, while the state tokens cover service/loot/up/down marks. NPC signature props and every variant pair still require the A-03 visual review.

Method: linearize each sRGB hex as in V-03, multiply both object and floor by a neutral/cool test gain `(0.85,0.92,1.00)` and separately an amber-biased gain `(1.00,0.72,0.45)`, then by the area's test scalar. Use relative luminance `0.2126R + 0.7152G + 0.0722B`, and ratio `(max(Y)+0.05)/(min(Y)+0.05)`. Report the **lower** ratio of the two light-gain tests. Scalars are **Hub out 1.00, Hub in 0.80, B1 0.60, B2 0.45, B3 0.33, B4 0.25**, deliberately authored stress levels; they are **not** SkyLight controls or predicted screen luminance. Kelvin cannot be converted to lit pixels by these arbitrary gains. No tonemapping, normals, shadows, texture, bloom, emission, size or eye adaptation is modeled.

Floor swatches: hub paving `#77766D`, B1–B3 floor `#695640`, B4 dark-stone instance `#514B42`. Exterior sand/water, red aisle, pale rubble and local stains require additional screenshot samples; paving alone cannot certify the hub. A slash separates two independently calculated materials, not a range. All numbers are ratios `:1`, rounded to two decimals.

| A-03 diffuse material(s) | Hub out | Hub in | B1 | B2 | B3 | B4 |
|---|---:|---:|---:|---:|---:|---:|
| Starter linen `#B5A58A` | 1.87 | 1.81 | 2.35 | 2.16 | 1.95 | 1.96 |
| Light Warm skin `#BD8E72` | 1.55 | 1.52 | 2.06 | 1.91 | 1.75 | 1.77 |
| Medium Warm skin `#8B5A40` | 1.18 | 1.16 | 1.16 | 1.13 | 1.11 | 1.20 |
| Deep Warm skin `#51362C` | 2.15 | 2.00 | 1.35 | 1.29 | 1.22 | 1.08 |
| Hair / deep timber risk `#30251D` | 2.83 | 2.54 | 1.64 | 1.50 | 1.38 | 1.18 |
| Rat body / back stripe `#594537` / `#806850` | 1.82 / 1.10 | 1.73 / 1.10 | 1.19 / 1.25 | 1.16 / 1.21 | 1.13 / 1.17 | 1.01 / 1.25 |
| Bat wing / mark `#927451` / `#AF9169` | 1.04 / 1.52 | 1.04 / 1.49 | 1.45 / 1.99 | 1.39 / 1.85 | 1.32 / 1.70 | 1.38 / 1.73 |
| Dungeon Bat wing / V `#666453` / `#B7A987` | 1.28 / 1.94 | 1.26 / 1.88 | 1.10 / 2.43 | 1.09 / 2.22 | 1.07 / 2.01 | 1.16 / 2.01 |
| Giant Bat wing / edge bar `#775638` / `#B29867` | 1.35 / 1.62 | 1.33 / 1.59 | 1.04 / 2.11 | 1.03 / 1.95 | 1.03 / 1.78 | 1.12 / 1.80 |
| Undead Bat wing / pale block `#64685D` / `#B6B49F` | 1.23 / 2.11 | 1.22 / 2.04 | 1.14 / 2.63 | 1.12 / 2.40 | 1.10 / 2.15 | 1.18 / 2.14 |
| Slime body / mark `#3F722E` / `#709B40` | 1.24 / 1.36 | 1.23 / 1.34 | 1.11 / 1.77 | 1.10 / 1.66 | 1.08 / 1.54 | 1.17 / 1.58 |
| Spider body / ridge `#66645E` / `#959083` | 1.27 / 1.41 | 1.25 / 1.39 | 1.11 / 1.83 | 1.09 / 1.71 | 1.08 / 1.59 | 1.16 / 1.62 |
| Goblin skin / mark `#8B3227` / `#512D27` | 1.54 / 2.30 | 1.49 / 2.12 | 1.05 / 1.42 | 1.04 / 1.34 | 1.03 / 1.26 | 1.02 / 1.10 |
| Warrior skin / back bar `#793127` / `#C0AC86` | 1.75 / 2.03 | 1.67 / 1.97 | 1.16 / 2.55 | 1.13 / 2.32 | 1.11 / 2.09 | 1.01 / 2.08 |
| Atrocity skin / muscle `#343038` / `#58505A` | 2.54 / 1.62 | 2.32 / 1.56 | 1.52 / 1.08 | 1.41 / 1.07 | 1.32 / 1.06 | 1.14 / 1.04 |
| Balork wing / horn `#A33A29` / `#B7A181` | 1.25 / 1.81 | 1.23 / 1.76 | 1.03 / 2.30 | 1.03 / 2.11 | 1.02 / 1.92 | 1.12 / 1.92 |
| Focus / down arrow `#E9BF79` | 2.59 | 2.51 | 3.26 | 2.93 | 2.59 | 2.54 |
| Info / loot / up arrow `#A9C4CC` | 2.34 | 2.25 | 2.89 | 2.62 | 2.34 | 2.30 |
| Hit white / hover `#F2F2F2` / `#C4C4C4` | 3.87 / 2.50 | 3.69 / 2.41 | 4.65 / 3.09 | 4.12 / 2.78 | 3.58 / 2.47 | 3.42 / 2.42 |


**Finding:** several bodies approach 1:1 and even the diffuse cool/amber cues do not reliably reach 3:1. Greater darkness reduces the useful contrast of small marks. This is a warning against approving the floor from a colour swatch or increasing torch saturation; it does not establish that the actual renderer fails these values. Linen and pale bat/warrior marks help, but no row alone guarantees recognition.

| Area | Mandatory difficult comparison in the actual scene | First lighting/background remedy if it fails |
|---|---|---|
| Hub exterior / interior | All skin tones/hair, Samaritan satchel, bow/quiver vs bottle vs armour bib, Iraltok red badge on red aisle, amber focus over paving | Keep neutral face/body illumination and opaque role plates; reduce nearby floor/aisle pattern contrast. Do not recolour a skin option or assign it a different light. |
| B1 | Small rat vs brown floor, bat vs rat/slime, selected amber against torch pool, both healer plates | Increase neutral floor/side fill at rats, keep tail/body edge visible; reduce bright torch hotspot under selection corners. |
| B2 | All six bat pairs, spider vs Giant Bat, rat/Slime at void edge; cool Up/loot marks | Preserve notch/V/edge-bar geometry and neutral upper surfaces; keep coping brighter than void but quieter than marks. Check both ring lanes and darkest optional branch. |
| B3 | Red Goblin vs Warrior back bar, Atrocity skin/muscle, C3Cell rat, partition-end silhouettes | Cool broad fill and plain brown floor; reserve pale repairs for wall faces away from bodies. Remove stains competing with goblin skin. |
| B4 | Rat/Slime in side chambers, Atrocity, Balork red wings and dark torso on dark stone, horn/polearm, dead body and loot | Keep F01 neutral and check all boss routes, including Hall apron and D05–D07; reduce red/pale floor clutter. Raise fill locally before weakening identity colours. |

A-03's dead transform blends living colours toward `#333333`, reduces saturation and removes emission; it will often merge with B4 stone. Treat flattened shape + authority-owned `Dead/Loot` plate/badge as necessary, and test a dead rat and dead Balork beside their live proxies. Do not brighten the corpse into a living pulse. The hit cue's white-blend/emission and attack mark's low emission are not represented by the diffuse ratios; test their actual timed material outputs with postprocess locked, with reduced motion, and against the brightest torch pool. No gameplay event hook is created by this guide.

Proposed **screen-space acceptance checks** after rendering: large unselected body edges aim for ≥1.5:1 against adjacent floor; small rat/tail/critical identity marks aim for ≥2:1 on their useful silhouette segments. Selected brackets/outline/arrow/badge aim for ≥3:1 against their immediate background. These are A-04 visual review goals, not WCAG rules for 3D creatures. Also require recognition at native size in grayscale; a ratio is insufficient if the mark is a subpixel smear or only the horn is visible. Use an opaque backing for nameplate text and keep V-03's ≥4.5:1 text target; inherited token arithmetic is primary text `#F2F2F2` on `#242424` **13.87:1**, secondary **8.90:1**, amber **9.01:1**, cool **8.47:1**. These are token calculations, not measured Unreal text output.

If a scene fails: simplify the floor behind the actor, move/re-aim local illumination within the same room, increase neutral fill within budget, then use A-03's existing selected visible-surface outline/label. Do not use an always-visible through-wall outline, enlarge creatures/capsules, strengthen material emission globally, add per-enemy lights or change skin colours. If unselected enemies still cannot be seen at far zoom, return the material/camera issue to its owner with a screenshot; selection labels do not close that defect.

## W3 validation checklist and evidence handoff

### Establish a comparable scene

- [ ] Adopt renderer settings deliberately with integrator; record engine version/CL, effective GI/reflection/shadow/AA settings, static-lighting state, exact SkyLight source/intensity/Lower Hemisphere setting, exposure lock, sun/local-light settings, tone mapping and colour grade. Capture console/settings evidence. A changed exposure/cubemap invalidates comparing floor brightness.
- [ ] Build the actual layout-owned maps and A-03 proxies/labels/state bindings. Dev maps are useful calibration fixtures; their generator overwrites them, so do not hand-dress those maps expecting changes to survive. Assign any persistent lighting fixture/map separately. No A-04 binary lease is implied.
- [ ] Use the same gray card, linen human, rat, slime, pale mark and focus/loot cues as a movable review lineup in each area; use off-roster creatures only in an explicitly labelled QA setup. Record positions and light settings. Then remove QA props and inspect actual permitted encounters.
- [ ] Start screenshots at **1920×1080** and **1280×720**, native **100% screen percentage**, same 16:9 crop, UI scale and exposure. Save colour and luminance-grayscale views; do not resize a 1080p capture and call it a 720p render. Record whether an image is editor, standalone or packaged. No beauty camera or enlarged proxies.

### Screenshot sheet per area

For each station below capture default camera first; then **all eight min/max boom × yaw × pitch combinations**, with close views at shortest collision-shortened boom where relevant. Repeat at both resolutions and in the local-shadows-off comparison. Capture each problematic unselected and selected case with HUD/nameplate visible, plus a matching uncluttered inspection image. Label every image with map, XYZ/facing, camera settings, resolution, quality and fixture/exposure preset. These are required future screenshots, not images supplied by the SVG plans.

| Map owner / area | Stations and comparisons to capture |
|---|---|
| W3-01 hub exterior | Nave exit→Samaritan/Forecourt, each service threshold, every mage-causeway bend plus blocked scenic forks; reverse view toward church. Show open walking strips after fixtures. |
| W3-01 hub interiors | SafeSpawn looking up red aisle; wing doorway/ramp and return arrival; each service approach; MageHall turns to Uranos/Iraltok. Capture roof cut/restore across doorways and camera movement. |
| W3-02 B1 | Entry return mouth/arrival, Hub center with all three arms, both bays, both healer pads, enclosure doorway/inside/perimeter. Selected and unselected rat, bat and slime at worst corner. |
| W3-02 B2 | Entry/Ring mouth with both lanes and void curb; LowerBay return threshold; both optional ends; both dogleg elbows; LongHall toward Descent and reverse. Compare all bat variants and spider in the same light. |
| W3-03 B3 | C3Cell doorway with rat; West stubs/WestBay; UpperCentral branch choice; BranchEnd; both C04 void bends; East baffle; LowerRight divider/return; Level 4 tip/arrival. Compare Goblin/Warrior/Atrocity behind caps. |
| W3-03 B4 | Entry ramp/arrival, Hall patch/three branches, both side chambers and northern baffle bypass; paired C04 lights; boss anchor, every D05–D07 door/turn and altar bypass; Hall retreat facing return. Alive/attack/dead/loot Balork and small enemies at far zoom. |

- [ ] Sample adjacent rendered body/floor and cue/background patches using the luminance formula, excluding specular/fire pixels; retain sample locations and ratios. Include darkest playable corner, brightest pool, red aisle/stain and pale rubble. Review silhouette/recognition at 100% as well as ratios. Do not call this document's swatch arithmetic a rendered contrast measurement.
- [ ] Record short clips for fade trigger/restoration, threshold jitter, camera collision, accepted hit/windup/death/loot, pause and travel/load. A still cannot validate flicker, anticipation timing, cue cancellation or roof flashes. Confirm no replayed death/respawn cue on loading a settled life.
- [ ] Repeat pillar LOS blocked/clear and impact-time rejection from dev-maps.md after fade adoption; selection must clear/exclude obstructed actors. Inspect B1 enclosure and B3/B4 baffles with the same retained-blocker rule. No target label through walls.
- [ ] Walk both directions of every stair pair and every return route, inspect spawn/arrival-facing camera, empty reserves, no instant bounce-back and no enemy damage/parking on protected floor. Test B4 return before/after defeat without a new gate. Visual safety and gameplay safety require separate observations.
- [ ] Inspect Light Complexity/Stationary Light Overlap where applicable, attenuation volumes through walls and material/shader complexity. Count visible/overlapping local lights at far zoom, not just actor counts in the room. Verify low-cost mode retains floor, void edge, player, unselected enemies, stairs and state cues.

### Unreal Insights measurements, later on the host

Capture a warmed standalone or Linux packaged route, with editor performance recorded separately. Record CPU model, actual GPU selected, VRAM, RAM, driver, display mode, resolution, scalability, AA, screen percentage and exposure preset. A GTX model name alone is not a complete reference-machine record. Keep profiling runs uncapped with VSync off to measure cost, then test the intended presentation cap separately. Do not use a NullRHI run as graphical performance evidence.

Collect **CPU/game-thread, render-thread/RHI and GPU tracks** in Unreal Insights, with bookmarks for arrival, roof fade, worst visible encounters, Balork pursuit/turns and each travel/save. Use the GPU profiler alongside Insights to separate direct-lighting, shadow depth/projection, base-pass/material, translucency and postprocess costs. Capture the same view with local shadows on/off to identify their cost; those two captures alone do not isolate all lighting work. Missing GPU tracks mean GPU cost is unmeasured, not zero. [Epic Timing Insights](https://dev.epicgames.com/documentation/en-us/unreal-engine/timing-insights-in-unreal-engine), retrieved 2026-10-08.

Proposed sampling: **30 s** per stationary worst-view replay and **3** repeat route runs after warmup. Report median, 95th/99th percentile and maximum CPU/GPU frame times, frame counts above 16.67 ms, steady lighting/shadow cost against the table, draw calls/visible lights, texture residency/VRAM and system memory. Capture cold load and shader/PSO compilation separately from warm traversal; record hitch duration, travel/load/save duration and which thread stalled. Include the busiest permitted encounter with A-03 effects, loot/corpse clutter and a moving/cutting camera; not all floor slots need simultaneous aggro. Record thermal/throttling state if run-to-run results drift. Averages alone do not pass the target.

W3 owners return fixture/exposure settings, native screenshots/clips, count/contrast notes and trace paths, plus explicit failures and deferred systems. Baseline fallback retains gameplay visibility; any required expansion of light count, renderer features, palette or camera goes back to its owner with evidence. G3 travel/play and later art/performance gates remain with the coordinator.

## Delivery limits and integration notes

A-04 changes only this guide and six SVGs under `docs/implementation/art/lighting/`. Plans contain original text/vector geometry from local layout coordinates, no source pixels, binary assets, executable code or imported font. All shade regions in the plans are diagram notation, **not a preview of the proposed exposure or attenuation**.

Documentation/coordinate/colour arithmetic and SVG structural/render checks are recorded in the attempt report. No Unreal editor, bake, navmesh, gameplay, cook/package, screenshot, cutaway material or Insights session was run for A-04. Concrete runtime prerequisites are W3 maps with this lighting adopted, working A-03/selection/cutaway/event bindings, and the host's graphics session/profiling access. The recorded engine installation does not provide those results automatically. Remaining decisions: exact calibrated exposure/cubemap, actual renderer settings, which single room lights need shadows, bake route after layout freeze, and measured small-creature/skin/state visibility. Next: W3 owners apply this guide within their map ownership and return the validation evidence above; integrator reviews renderer and fade implementation separately.
