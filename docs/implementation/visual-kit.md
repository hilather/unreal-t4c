# W5-01 native procedural environment kit

Owner decision 2026-10-10: reproducible stylized geometry, no downloads or binary content. All dimensions, swatches and budgets are **Prototype presentation tuning**, W5-01 / LH_Prototype_v1 / 2026-10-10 / source_url null, inherited from [A-02 environment](art/environment/README.md), [module schedule](art/environment/modules.md), [A-04](art/lighting/README.md), [V-01](layout/README.md), [art pipeline](../plan/docs/04-art-pipeline.md) and the hub/basement implementation briefs. These are not historical mechanics. No HP, XP, loot or encounter changes.

## Integration API

Include `Visual/LHVisualKit.h`. `LHVisual::PieceIds()` is the case-canonical catalog. All suffixes below expand to `Presentation.Environment.`. `SpawnProp(World, Id, Transform, Style, bDescending=false)` constructs an `ALHVisualPiece` with merged procedural box geometry and dynamic engine materials. Unknown IDs, invalid/non-unit transforms, invalid styles and descent requests for non-stairs return null. No tick, random seed, external files, baked assets, global catalog mutation, map placement or light actors are involved. The runtime module and tests depend on the enabled engine `ProceduralMeshComponent` plugin.

`MakeRecipe(Id, Style, Out, bDescending=false)` produces geometry, collision boxes, colors, roughness and per-piece budgets without a world; failure leaves Out intact. `ALHVisualPiece::Build(Recipe)` supports editor generators as well as runtime creation and rebuild, retaining an existing assembly on invalid input. Recipe copying/customization is a presentation seam, not a gameplay definition. The recipe is a serialized USTRUCT property. OnConstruction and BeginPlay rebuild the assembly from that recipe, including transient dynamic materials. Map serialization/cook must still be exercised by W5-05; W5-01 does not save a map. W5-05 should validate before saving and after rebuilding.

`SpawnWallRun(World, Start, End, Style)` builds horizontal runs in <=400 cm segments, including exact-length infill. Start/End are bottom corners on the clear wall face. No slope, rounding or source-tile conversion. Length range 5..100000 cm bounds allocation. Invalid input creates nothing; a build failure destroys the entire partial run. Return array ownership belongs to the caller. Rotations orient local +X along the run; local +Y faces the room. Unit actor scale is required.

`ValidatePlacedSet(Pieces)` returns piece, actual procedural-index triangle, nonempty section and box counts plus errors for null actors, exceeded budgets, geometry count mismatch and collision profile violations. Draw totals are conservative **base-pass section estimates**, not measured GPU draw calls: shadows, extra passes and renderer batching change cost. No level-wide automatic placement or performance gate is implied. `Fingerprint()` compares explicit recipe fields, without pointers, padding, actor IDs or global RNG. It is a regression checksum, not persistent identity or a cryptographic asset digest.

## Catalog, pivots and budgets

Centimetres; +Z up. Walls: bottom-left on clear face, X[0,L], Y[-20,0], Z[0,400]. Floors: plan lower corner on top, X/Y[0,L], Z[-20,0]. Frames: aperture center at ground, +X across opening, Y[-20,0]. Pillar/props: floor center. Fixture origin: backplate contact center, +Y projects into room. Door leaf: hinge at local X0, extends X[0,160], Z[0,300]. Stairs: walking surface start center, +X travel, ±Y150; bDescending selects negative rise without negative actor scale.

| Suffixes | Shape / clearance | Actual triangles per assembly | Ceiling triangles / base draws |
|---|---|---:|---:|
| Shared.Wall100 / Wall200 / Wall400 | 20 thick, 400 high; eight 50 cm staggered courses | 156 / 252 / 444 | 2000 / 2 |
| Shared.Floor100 / Floor200 / Floor400 | 20 deep slab with 100 cm inset paving tiles | 24 / 60 / 204 | 2000 / 2 |
| Shared.Door240 / Door320; Basement.DoorBoss500 | 400 / 400 / 600 bay; clear 240×300 / 320×300 / 500×360 | 36 | 4000 / 2 |
| Shared.Arch240 / Arch320; Basement.ArchBoss500 | Same rectangular clearance; six shallow stepped voussoirs above it | 108 | 4000 / 2 |
| Shared.Stair400x100 / Stair600x120 / Stair250x80 / Stair300x80 | 300 wide; signed rise ±100 / ±120 / ±80 / ±80 | 132 / 156 / 108 / 108 | 6000 / 2 |
| Church.Pillar | 60 square, 400 high, broad base/capital | 36 | 4000 / 2 |
| Church.DoorLeafPreview | One 160×300×8 leaf, five boards, two straps; closed blocking preview | 96 | 2000 / 2 |
| Shared.Torch / Sconce | 25×40 backplate, projecting support and warm flame proxy | 36 | 2000 / 2 |
| Shared.Barrel / Crate | Barrel 62×62×100; crate 82×82×80 incl. bands | 72 / 36 | 2000 / 2 |
| Shared.Table / Bench | 160×90×80 / 160×45×45; top and four legs | 60 | 4000 / 2 |
| Shared.Altar | Three tiers, 200×160 footprint, top120 | 36 | 4000 / 2 |
| Shared.Debris | Five fixed rotated 16×24×12 fragments | 60 | 2000 / 2 |

26 IDs, five styles, four signed stair profiles. Detailed blocks/tiles are merged into at most two sections per actor, rather than one component per stone. Simple planar faces have explicit normals, outward winding and 1 metre UV repeats. Recessed mortar cores avoid coplanar detail. No texture allocation, Nanite, Lumen, LOD or imported asset dependency; this low-fidelity kit substitutes for A-02's authored texture/LOD production proposal. No corners, roof kit or arbitrary floor infill API is promised in this task; layout owners can compose rectangular pieces without filling intentional voids.

## Area palette and camera treatment

Colors are unlit sRGB starts converted to linear. Opaque engine `/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial` exposes Color and Roughness to dynamic instances. Roughness .85 masonry / .90 accents, .80 timber / .65 dull fixture bands. No unsupported metallic, emissive or texture parameter is claimed; torch flame is a warm lit solid proxy, not an actual emitter. Actual lights/exposure remain with the map owners and [lighting.md](lighting.md).

| Style | Masonry / accent | Floor identity |
|---|---|---|
| Church | #78634B / #B2A58A | #77766D gray paving; upper wall courses use plaster tint |
| B1Cellar | #78634B / #9B9484 | #695640 worn tan slabs, sparse pale tiles |
| B2Damp | #78634B / #433E32 | Brown floor with dark accents; bottom two wall courses form damp band |
| B3Crypt | #78634B / #9B9484 | Pale slab accents; contiguous mid-wall mineral repair patches |
| B4Ritual | #514B42 / #733D35 | Brown floor with dark red accent tiles; dark masonry and altar focal mass |

Timber/doors/containers #43352A with dull #535451 bands; fixtures #535451 and #E8AA53 warm proxy flame. Crypt names a presentation treatment for B3, not an added cemetery or skeleton encounter. B4 ritual adds no hazard or boss phase.

Broad 50 cm courses, large paving, restrained prop silhouettes and palette separation target boom1200 cm / pitch−55° / yaw45° / horizontal FOV45. Review also boom900..1800, pitch−60..−50, yaw0..90, shortened camera, 720p/1080p and grayscale. No real-RHI capture or readable-at-camera acceptance is claimed by source construction or nullRHI tests. A-03/V-03/V-04 interaction/UI colors and silhouettes remain owned by their presenters; kit props carry no selection rings, identity, loot or interaction logic.

## Collision and placement responsibilities

Render mesh is always NoCollision, no navigation influence, no complex collision. Every structural piece has explicit BlockAll UBoxComponents with Pawn/Visibility blocking and navigation influence. Wall/floor blockers match the solid nominal volume independently of inset render detail. Frame jambs/lintel preserve the entire rectangular aperture. Stairs have one smooth rotated 20 cm slab whose top matches signed run/rise; tread bands do not collide. Tables/benches block through top and four legs; barrels use one conservative 60 cm square footprint, with outer bands cosmetic. Torch/sconce/debris have no blockers and no nav influence. Closed door leaf is a blocking preview; W5-05 must not put it across currently open routes without a gameplay door owner.

Props belong outside protected arrival boxes, turn pads, combat lanes, NPC approach pads and baffle apertures. Preserve existing 240×300 ordinary / 500×360 boss doors, 320/550 corridors and 300 cm stairs. No kit actor automatically fades or cuts away: full-height walls need map-owner render cutaway/fading with blockers retained. Nothing here moves entrances or supplies approval for arrival/nav safety. W5-05 must regenerate, rerun arrival review and LHValidateWorld, then collision, nav and actual-camera walkthroughs.

## Validation

`Lighthaven.Visual.CatalogBuildCollisionBudgets` covers every ID in every style (130 assemblies), collision profiles, material parameters and measured section/index budgets. `DeterminismAndRebuild` compares exact generated vertex positions/normals/UVs/indices and recipe fingerprints, and checks no blocker accumulation on rebuild. `WallRunClearanceAndInvalidInputs` checks rotated/infill runs, aperture clearance, invalid input and atomic rejected rebuilds. `SignedStairsPhysicsAndRecipeSerialization` tests all four profiles in both directions with real box physics queries and serialized recipe round-trips. Execution results and remaining checks are recorded in the attempt report. NullRHI can establish native construction, not rendered lighting or GPU performance. Cook/package and placement acceptance belong to W5-05 integration; Windows remains deferred.

Observed in this worker checkout (UE5.8.3 Linux, UID1000): final editor/game build exit0, both `Result: Succeeded` (6.83 / 2.98 seconds; game up to date after the fixture-only change). Final headless `Lighthaven.Visual` report: 4 expected / 4 succeeded, 0 warnings in tests, 0 failures, 0 not run; process exit0. Catalog: 130 pieces, 13020 triangles, 235 base sections, 225 collision boxes. Exact commands, exploratory corrections and logs are in the attempt report/library. Baseline LFS-pointer map and read-only settings/DDC startup diagnostics remain; no map, rendering or packaging result follows from these native tests.
