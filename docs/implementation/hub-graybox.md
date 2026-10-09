# W3-01 hub graybox generator

Candidate on W3-04 base `0768422a59c34efe687c640b928ace2aca90d262`, contract revision 1.
Generates `/Game/Lighthaven/Maps/L_LighthavenTempleDistrict` using editor-only
`LHGenerateHubMap`; no binary asset is supplied by this change. World/Core are
read-only. Layout/source ordering comes from [hub specification](layout/hub-temple-district.md)
and [world ledger](world-ledger.md); this is reconstructed geometry, not original tile fidelity.

```bash
UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game
UE_ROOT=/home/brewerm/Downloads/unreal bash build/generate-hub-map.sh
cp Saved/HubLayout.tsv /tmp/hub-first.tsv
UE_ROOT=/home/brewerm/Downloads/unreal bash build/generate-hub-map.sh
cmp /tmp/hub-first.tsv Saved/HubLayout.tsv
UE_ROOT=/home/brewerm/Downloads/unreal bash build/run-tests.sh Lighthaven
```

The wrapper refuses root, extra arguments, missing command editor and read-only
LFS maps. The commandlet also checks read-only output and registry validity.
It creates a fresh world each time, overwriting only the canonical hub map.
Back up hand edits before regeneration. Only the coordinator commits reviewed
maps via LFS. Exit 0 requires SaveMap success. A sorted `Saved/HubLayout.tsv`
records generated actor classes, names, transforms, identities and tags. Layout
is deterministic; engine actor GUIDs, shader data and package bytes may differ.
Manifest checks do not prove navigation or visual correctness.

## Geometry and provenance

All generated actors carry `LH.Hub.Generated`, `LH.Geometry.Reconstructed` and
`LH.Provenance.Prototype`. Actor labels follow the region aliases; walls and
union boundary fragments have deterministic numeric suffixes used only for
presentation, never persistent entity identity. All metrics are Prototype,
`LH_Prototype_v1`, source URL null; inherited V-01/A-02/A-03/A-04 proposals,
2026-10-07/08. No new mechanical tuning, reward, offer or Bible value is added.

| Actor group | Reconstruction / source confidence | Implementation |
|---|---|---|
| Temple.Nave / DungeonWing | V church/wing motif, I connectivity, P all dimensions | 16×24 U nave; 11.8×10 U wing; exact 3.2×3 U open door cuts; walls 20 cm thick, 400 cm collision height |
| Temple.Descent.Ramp | V dungeon label, I pairing, P ramp | 400 cm run, 100 cm descent, 300 cm width; wing floor split around footprint; smooth collision at approximately 14 degrees |
| Temple.Pew / RedAisle / Altar | V aisle/benches, I altar, P fixtures | Four 300×200×100 cm pews; 400×2100 cm red aisle; 20 cm plinth and table top below 120 cm |
| Temple.* shops / mage rooms | V relative buildings/names, I internal access, P rectangles | All required room bounds and apertures; shared collinear walls built once; threshold slabs bridge the 20 cm partition strips |
| Temple.*Lane / *Spur / MageCauseway | V street/sand motifs, I passability, P route dimensions | Exact ordered centerlines; 400 cm perpendicular strips and 400² cm bend pads; no broad walkable support plane |
| District.Edge / EdgeUnder | P containment | Convex footprint union exterior clipped at intersections; 100 cm visible banks plus concealed below-floor collision to contain ramp; no barriers at route junctions |
| Scenic.Water / OuterIslandBoundary | V water/outer branches, P closure | Nonblocking, nonnav water at -50 cm; modest marker on closed outer branch; no Crypt/Cave/Stonehenge portals |
| Temple.*Roof / RoofSlope | P massing | Modest nave eave 400 cm/ridge 600 cm, wing and shop low roof proxies; no cathedral tower; runtime hard cutaway |
| NPC.* | V service region/name, P feet/yaw | Thirteen A-03 Body A primitive assemblies and distinct signature props, at the exact layout positions; nonblocking/nonnav visuals |
| Hub.Light.* | P A-04 plan | All E01–E07 and I01–I13 coordinates; movable unshadowed points; T 600 lm/600 cm, W 1000 lm/800 cm, N 1200 lm/850 cm; warm 2300 K, neutral 6500 K |
| Hub.Sun / SkyFill / FixedExposure | P A-04 baseline | Sun 3000 lux, 6500 K, pitch -45/yaw45; one sky intensity .70, no real-time capture/shadows; fixed exposure 6, bias0, bloom/motion blur0 |
| *.FadeVolume | P A-04 authoring contract | Noncolliding box bounds for rooms and individual wall segments, tagged LH.Fade.AuthoringVolume / LH.Fade.CapCm.80 |

No reference image, imported texture or external HDRI is used. Flat opaque
materials are embedded in the generated map, with A-02/A-03 sRGB colors
converted once to linear, roughness .85. Engine cubes/spheres/cylinders/cones
are scaled using final centimeter dimensions. Decorative children have no
collision, overlap or navigation influence.

## Identity and safety

`Temple.SafeSpawn` and `Temple.Descent` are ALHEntranceMarker actors populated
from the W3-04 registry, including absolute ground-contact transforms and facing.
Both retain `bSafetyReviewed=false`; generation does not approve travel safety.
PlayerStart uses the actual default ALHCharacter capsule half-height over
(800,500,0), yaw90. Nave X[5,11]/Y[2,8] and wing X[-7,-1]/Y[2,8]
remain empty. No enemies, loot or encounter anchors are placed.

`Temple.Descent.Departure` is ALHPortal at (-1000,500,-50), with registry
GUID `b1019c84a8c44227b9d5609e785c0afc`, source hub Temple.Descent,
destination B1 Entry, direction Descent. Explicit interaction is required by
W3-04; no overlap travel is introduced. The B1 return edge already exists in
the registry and must be placed/checked by W3-02. The marker is distinct from
the arrival at (-400,500,0), yaw0. A double-bar amber arrow is on the flat
approach; it is decorative, not an automatic travel trigger.

NPC ALHInteractableMarker actors use authored `NPC.*` DefinitionIds and separate
literal GUIDs minted once in this commandlet. The source constants are the
identity authority for this candidate: preserve them when moving or replacing
proxies. No GUID is derived from labels, coordinates or array positions. These
DefinitionIds are authoring aliases awaiting W4 catalog binding, not asserted
existing offer definitions. Iraltok/Uranos remain in the mage building; Nevanis
and Shovanis belong to B1. Brother Kiran/Jagar Kar receive no invented offers.
NPC services provide no free/debug teaching, items, healing or quest credit.


| NPC alias | Authored instance GUID |
|---|---|
| NPC.BrotherKiran | `cad1834f92084c3484f48b9c4c077a7b` |
| NPC.Kilhiam | `6dfd770dbf874ac09dd2efba40bcd81e` |
| NPC.Moonrock | `5ef65f0ad7cf450ca6554e17c4115287` |
| NPC.Samaritan | `7aed0467432c4f868c81e65ede50f1b2` |
| NPC.Sigfried | `36351285404c489f8accd6b6e3a9d2ab` |
| NPC.Fali | `0d1ff670d056496ca68fac87dec830c5` |
| NPC.Rolph | `461bc4749f07451aa01ef2bec941aff0` |
| NPC.Ortanalas | `16e51f72d20d4efc8bc725cc47c2cdb7` |
| NPC.JagarKar | `4522396925a0480fbafbde5975c3cb30` |
| NPC.Kalastor | `8cb18c8e2d2c47a48a59a0a99375c1cf` |
| NPC.Murmuntag | `6ef21e05308e465cb6add6ce34e0d150` |
| NPC.Uranos | `ea0144a11a54417487447d0c02d9706e` |
| NPC.Iraltok | `448469187fbd4606b5767ff3b8b87218` |

## Presentation and runtime seams

Upper structural walls retain full BlockAll collision and Visibility blocking;
only their render mesh is hidden. Separate 80 cm caps show the footprint.
Roof render is hidden/noncolliding and casts no shadow. This is A-04's allowed
stable temporary cutaway, not a runtime fade implementation. Fade volumes are
metadata for presentation integration; they do not tick, change targeting or
allow click/attack through walls. Roof pieces can be made visible in the editor
for exterior massing review. Runtime occupancy transitions, silhouette margins,
0.15 s fade / 0.20 s restoration delay / 0.25 s restore remain unimplemented.

SkyFill uses an embedded 16² six-face constant achromatic sRGB128 cubemap,
with the lower hemisphere overridden to the same neutral color. It requires
actual gray-card calibration before the .70 multiplier can support cross-map
comparison. Renderer-wide Shadow Maps/Lumen/AA policy and local
exposure remain integrator-owned; no Config change is made here. Fixed exposure
6 assumes the project's EV100 range setting; host must check the actual project
exposure mode. The 20 authored local lights need overlap/visible-view profiling;
no A-04 budget pass is claimed.

W3-04 actors are scene-root markers only. They do not implement ILHControlTarget,
interaction traces, proximity, prompts or transactions; the permitted files
cannot change those runtime classes. A-03 selection-only opaque screen nameplates
and `Down to Temple B1` prompt are presentation/interaction-owner work. Consequently
this generator preserves door/service approach clearances, but **does not make
NPC offers or portal interaction playable**. W4-07/integrator must attach target
providers and legitimate offers to these persistent IDs and validate all thirteen
3×3 U pads. No permanent billboard labels are substituted for the planned UI.

Hub.NavigationBounds covers the full route and rooms (8000×17000×1400 cm).
Bounds are brush-backed; they are not evidence of a baked navmesh. The host must
build navigation and inspect the exact floor union, diagonal sand turns, door
cuts and ramp terminus before saving the final map.

## Validation and next review

Build/run observations are recorded in the attempt report and appended here
once available. No G3 or fidelity approval follows a successful build/generation.
Required host review: open the generated map; build nav; walk SafeSpawn to all
thirteen pads and back, then hub→B1→hub. Check keyboard/controller targeting,
legitimate offers, quit/reload and invalid destination recovery after runtime
binding and safety review. Review 900/1200/1800 cm boom, yaw0/45/90,
pitch-60/-55/-50, horizontal FOV45 at720p/1080p, including wall-shortened
camera. Inspect water-edge containment, all sand bends, cutaway collision/LOS,
service badge distinction, light overlap and neutral sky calibration. Windows
package checks remain deferred; no editor viewport, cook, package or play result
is inferred from NullRHI generation.

The generation wrapper selects the filesystem-only `-DDC=(Local)` graph and
puts its cache under `Saved/DerivedDataCache`. This avoids launching Zen or
writing engine/user cache directories in an isolated worker. It does not change
project renderer, runtime data or the user's global cache configuration.

In this worker the original Dev_Combat, Dev_Movement and L_Frontend maps are
unresolved LFS pointer files. `git lfs checkout` reports that LFS is not installed.
They are left unchanged. Host map loading/cook requires real baseline map bytes.

Observed on 2026-10-08/09 UTC, UID1000, UE5.8.3 Linux: the final
`UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game`
returned 0 in 230 seconds, with both editor and game targets reporting
`Result: Succeeded` (90.54 s and 138.33 s engine timings). Recovery required
worktree-local disabling of UBA detouring and a direct clean rebuild of the
editor PCH; no tracked build configuration was changed. A diagnostic NoPCH
build exposed baseline missing UWorld/UGameInstance includes outside this scope
and failed; it is not the final build configuration.

Two filesystem-cache generator runs each logged successful SaveMap and the
same counts: 13 NPCs, 2 unreviewed entrances, 1 registry portal, 20 local lights,
187 union boundary segments, 61 wall segments. They exited **1**, in 18 s and 19 s,
because the asset registry reported the three unresolved baseline LFS maps as
unloadable. These are failed editor process checks, even though map saving was
observed. Their 931-row sorted layout manifests compared equal (`cmp` exit 0),
SHA256 `44e7b0747e28ce5b890fae36b9c2bc476f218aef6beb8499087e08275ddf80f6`.
The temporary editor-generated hub map was removed after comparison and is not
part of this source deliverable. Coordinator: regenerate with hydrated baseline
maps, require a clean process exit, inspect/review, then commit with LFS.

`bash -n build/generate-hub-map.sh` passed. The wrapper rejected an extra
argument with exit 2 and mocked UID0 with exit 1 before editor startup. The mock
only tested shell behavior; actual generation used UID1000. Read-only LFS-map
rejection was inspected in source, not exercised against an owned locked map.

Automation: the normal `bash build/run-tests.sh Lighthaven` attempts failed
before tests (Linux Home Screen/X11 settings-directory crash, then unavailable
Zen cache). A direct runner with `-DDC=(Local)`, worktree LocalDataCachePath,
`-NoCrashDialog` and saved `HomeScreen.EnableHomeScreen=0` completed 76 tests
in 105 s: **74 Success, 2 Fail, 0 unfinished** (seven successes carried warnings).
Exit 255; report `Saved/Automation.HubDirect/index.json`. The two failures are
`Lighthaven.World.AreaHydrationAndHighWater` (subsystem created with invalid
Package outer instead of GameInstance) and `Lighthaven.World.RegistryIntegrity`
(the “Wrong casing fails” expectation). These match the W3-04b fixes anticipated
in the brief; no World/Tests changes are made here. The suite is not passed.
