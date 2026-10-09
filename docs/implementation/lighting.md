# W4-09c basement lighting and capture framing

The W4-09b brief reports basement arrival captures at mean sRGB luma .023–.050, with near-black walkable areas. This revision adds floor coverage and increases interior exposure; the requested B1–B4 mean .12–.25 and hub .2–.35 remain **host visual acceptance targets**, not observed results here.

All values below are **Prototype presentation tuning**, based on the brief's capture evidence and the existing A-04 graybox lighting, not authentic T4C mechanics. The Bible lookup and rules/world ledgers were checked: they provide gameplay numbers, not Unreal lighting calibration. No gameplay numbers change. Replace these presentation values after a real-RHI capture of regenerated maps; preserve dungeon mood and warm landmark contrast.

| Setting | B1–B4 | Rationale |
| --- | --- | --- |
| Manual physical EV100 | 1 (previously 2), ISO100, f/2, reciprocal shutter .5, bias0, equal overridden bounds1 | One stop more exposure; `log2(2² × .5 × 100 / 100)=1` |
| Neutral / warm / torch locals | 1800 / 1500 / 900 lm | Broader readable floor fill with existing warm landmark profiles |
| B3 cell local / B4 boss spot | 600 / 4000 lm | Cell detail and boss landmark; existing spot cone50/65 degrees retained |
| Local attenuation | 1400 cm, boss spot2000 cm | Overlapping corridor and room coverage |
| Neutral sky | .8, specified `/Engine/EngineResources/GrayLightTextureCube.GrayLightTextureCube`, no realtime capture or shadows, lit lower hemisphere | Consistent replaceable ambient source across the four floors |
| Coverage locals | Neutral1800 lm, 6500 K, z250 cm, no shadows, Movable | Sample authored floor rectangles/strips at spacing ≤800 cm per axis; B1/B2 skip absent floor/pits, B3/B4 suppress candidates within400 cm of a prior fill |

Coverage is generated from existing floor descriptions and does not move geometry, arrivals, portals, encounters, NPCs or their identities. B1's GUID-bearing Nevanis/Shovanis interactable placement remains intact. Added point lights have no blocking primitive components. All existing lights remain Movable. The hub generator and project rendering settings retain the W4-09 settings: EV10, sun3000 lux, sky.7, entirely dynamic lights, no Lumen GI/reflections. No renderer stall fix is claimed.

B1/B2 coverage names start at `A04_T1000`; B3/B4 use `Coverage_###`. B3/B4 collect positions before spawning, avoiding actor-array mutation during iteration. Floor strip order is authored and deterministic. Broad, shadowless fill deliberately prioritizes graybox readability; light overlap, wall leakage and GPU cost need host review on the GTX1050Ti. No emissive materials or fake lightmaps are introduced.

## Capture

Run from this checkout as the desktop user:

```sh
bash build/capture-map-screenshots.sh /absolute/path/to/Development-Lighthaven-or-UnrealEditor
bash build/capture-map-screenshots.sh /absolute/path/to/Development-Lighthaven-or-UnrealEditor --overview
```

The arrival shot uses the current registry ground pivot plus90 cm (capsule center), upright pawn rotation (pitch/roll0), and arrival yaw. **BugItGo teleports/rotates the pawn**, as confirmed in installed UE5.8.3 `CheatManager.cpp::BugItWorker`; it does not set the absolute spring arm. The prior command applied registry pitch and a180 cm offset to the pawn. The current registry pitch is already0; lowering the target by90 cm and explicitly keeping it upright uses native `LHCharacter.cpp` defaults: boom1200 cm, absolute yaw45/pitch-55, horizontal FOV45, collision probe12 cm. These are the selected defaults among `hub-graybox.md`'s review values. Boom collision may shorten the view exactly as in gameplay. No camera property or arrival registry is edited.

Optional overview shots use fixed room-center pawn positions and the same native camera (not a free-flight aerial camera): hub(-400,500,90), B1(900,-1900,90), B2(1000,-5600,90), B3(2900,300,90), B4(0,1000,90), in cm, upright/yaw0. These capture-only prototype vantages are recorded in the script and have no persistence/authoring effect.

Independent launches retain logs and images under `Saved/LightingCapture/<timestamp>/`. HighResShot requests1280×720 after120 warmup frames; each launch has a55 s timeout and a15 s engine lifetime. Failed placement, launch or missing image fails the script. Screenshot paths containing whitespace are rejected before launch. Requires desktop/display, Development cheat commands, Python3 and ImageMagick; installs nothing. Shipping is unsupported.

Each shot reports mean, median, and **near_black** = fraction of normalized Rec.709 sRGB luma pixels strictly below .02. Statistics cover the whole screenshot, including intentional void; inspect walkable space separately. They are screen diagnostics, not scene-linear illuminance or proof of room readability. Optional overviews are additional evidence, not substitutes for arrival acceptance.

## Audit and integration

`Lighthaven.World.LightingAudit` checks all five registry maps: original landmark inventories20/12/17/17/13 remain after subtracting coverage; ≥10 coverage lights per interior; neutral fill within1400 cm in XY of every encounter and every entrance; no blocking light primitives; interior local radius≥1400, lumens600..4000; Movable lights; one shadowless specified non-null sky at.7/.8 with lit interior lower hemisphere; one manual physical unbound PP at EV10/1 with equal bounds and bias0; hub-only sun3000 lux. This is structural coverage, not a screenshot test; nullRHI cannot certify actual brightness or cubemap rendering.

Coordinator integration sequence: build editor/game, regenerate all five maps using the existing scripts, run the lighting audit, `build/review-arrivals.sh` and `LHValidateWorld`, then package/capture both arrival and overview views on the host. Do not integrate generated binaries from this worker. Judge luma targets plus near-black walkable coverage and dungeon mood; tune if needed. Windows remains deferred. The render-stall limitation remains unchanged.

## Worker evidence (2026-10-09)

UID1000, UE5.8.3 Linux. `UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game` finished with exit0 and both targets `Result: Succeeded` (editor6.22 s, game18.25 s in final incremental run). Initial complete editor/game builds reported130.87/139.09 s. Sandbox build configuration under ignored `Saved/` disables UBA detouring; UE still uses its local UBA executor, and the final link noted unsuccessful action-cache stores without failing the build.

Regenerated all five playable maps with the three native commandlets, `-nullrhi`, and `-ddc=InstalledNoZenLocalFallback -LocalDataCachePath=<checkout>/DerivedDataCache`. Each saved its maps and returned exit1 due to remaining LFS-pointer startup packages (8/6/4 reported errors for A/B/hub runs). Generated B3/B4 manifests contain91/74 coverage lights respectively. No binary is submitted.

Full `Automation RunTests Lighthaven; Quit` with filesystem DDC fallback returned exit0: **151 completed, 146 clean successes +5 successes with warnings, 0 failed/not-run/in-process**. `Lighthaven.World.LightingAudit` passed; suite test duration5.841235 s excludes editor startup. Warning-bearing tests: BowRequiresQuiver, AI.StateMachine, Wave3.ArrivalSafety, Wave4.Stage1A.ErrandLoop, Wave4.LiveMeleeCommand. Initial memory-cache-only launch stalled at Zen's read-only user config path and was interrupted; filesystem fallback was the successful run.

Shell syntax, embedded Python compilation, five-arrival registry parsing, CLI help and `git diff --check` passed. Real-RHI captures, measured luma targets, overview composition, package/play checks and GPU cost remain unobserved: this worker has no DISPLAY/WAYLAND_DISPLAY. The coordinator must perform host visual review. Complete logs and JSON automation evidence accompany the attempt report outside the repository. Generated map files were restored to their baseline pointers after the tests.
