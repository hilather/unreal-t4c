# W4-09d basement exposure and capture framing

All values here are **Prototype presentation tuning**, based on the coordinator's W4-09c packaged Linux captures and the existing A-04 graybox lights. They are not authentic T4C mechanics. No gameplay numbers change. Replace these values after real-RHI capture of regenerated maps.

## Exposure

B1–B4 now use manual physical EV100 **2.5**, up from 1 (+1.5 stops, scene-linear exposure multiplier `2^-1.5 = .353553`). ISO100 and f/2 stay fixed; reciprocal shutter is `2^2.5 / 4 = 1.414214`, so `log2(fstop² × shutter × 100 / ISO)=2.5`. Both overridden exposure bounds are 2.5 and bias is zero. All lighting stays dynamic.

| Map | W4-09c host mean sRGB luma | W4-09c near-black share | Expected W4-09d mean (provisional) | Acceptance |
| --- | --- | --- | --- | --- |
| Hub | .28 | .0002 | .28 | mean .20–.35 |
| B1 | .53 | not supplied | approximately .19 | mean .15–.25, near-black <.05 |
| B2 | .50 | not supplied | approximately .18 | mean .15–.25, near-black <.05 |
| B3 | .68 | .019 | approximately .24 | mean .15–.25, near-black <.05 |
| B4 | .68 | not supplied | approximately .24 | mean .15–.25, near-black <.05 |

Expected means are coarse tuning hypotheses using the brief's suggested 1.5-stop reduction, **not a physical prediction or observed result**: exported sRGB luma, tonemapping and the corrected framing do not scale linearly with EV. Expected near-black share is below5% on each basement; it cannot be calculated from a prior mean. Coordinator capture must establish the actual values and readability before acceptance. If the result misses the band, tune EV/floor fill from the measured capture rather than removing landmark lights.

Local profiles remain neutral1800 / warm1500 / torch900 lm, B3 cell600 lm and B4 boss4000 lm. Radius remains1400 cm (boss2000), interior sky remains.8 with specified GrayLightTextureCube, lit lower hemisphere and no shadows. Existing coverage placement, shadowless neutral fill and all landmark lights remain intact, preserving warm/neutral contrast. Coverage is authored at ≤800 cm spacing, z250 cm; B1/B2 names start `A04_T1000`, B3/B4 use `Coverage_###`. No geometry, collision, arrivals, portals, encounters or NPC marker code changes. Hub retains EV10, sky.7 and sun3000 lux. No renderer-stall fix is claimed.

## Capture

Run from this checkout as the desktop user, against a freshly regenerated and packaged Development build (or editor):

```sh
bash build/capture-map-screenshots.sh /absolute/path/to/Development-Lighthaven-or-UnrealEditor
bash build/capture-map-screenshots.sh /absolute/path/to/Development-Lighthaven-or-UnrealEditor --overview
```

Arrival targets remain the registry ground position plus90 cm (capsule center), with upright pawn rotation. `BugItGo` only sets pawn/control rotation; it does not set the absolute spring arm. The script now explicitly selects `Camera Default`, sets the native spring arm length1200 cm, enables its pawn-control rotation and sets controller pitch−55/yaw45/roll0, plus camera FOV45. These match `hub-graybox.md`'s selected camera review values and `LHCharacter` defaults. Native camera/12 cm sphere probe and wall shortening remain active; this is not the engine's generic ThirdPerson/free camera. The pawn stays upright while the spring arm obtains the requested rotation each tick. Settings affect only the capture process, with no source/map/arrival edits.

`setnopec` avoids editor component reconstruction. Controller struct members are set separately because Unreal's ExecCmds parser splits at commas; omitted members retain their values. `getall` records controller rotation, boom length, control-rotation flag and FOV in each launch log. Inspect those values with the screenshot when checking framing. Target/boom/pitch/yaw/FOV also print before each launch. Composition and actual camera pose still need host visual review.

Optional overview targets remain hub(-400,500,90), B1(900,-1900,90), B2(1000,-5600,90), B3(2900,300,90), B4(0,1000,90), in cm, using the same camera. Independent launches retain images/logs under `Saved/LightingCapture/<timestamp>/`; HighResShot requests1280×720 after120 warmup frames, with55 s launch timeout and15 s engine lifetime. Placement/launch/image failure fails the script. Requires desktop display, Development cheats, Python3 and ImageMagick. Shipping is unsupported; installs nothing.

Each shot retains mean, median and **near_black** (fraction of normalized Rec.709 exported sRGB luma pixels strictly below.02). Whole-image statistics include intentional void; inspect walkable areas separately. They are screen diagnostics, not scene-linear illuminance. Overview shots supplement arrival acceptance.

## Audit and integration

`Lighthaven.World.LightingAudit` now expects basement EV2.5 and hub EV10, verifying the physical camera formula and matching overridden bounds. Other thresholds stay intact: landmark inventories20/12/17/17/13, ≥10 coverage lights per interior, fill within1400 cm XY of every encounter/entrance, no blocking light primitives, local radius≥1400, lumens600..4000, Movable lights, one specified non-null shadowless sky (.7/.8), manual physical unbound exposure, zero bias and hub-only sun3000 lux. This structural audit cannot measure rendered luma under nullRHI.

Coordinator: build editor/game, regenerate all five maps, run LightingAudit, arrival review and LHValidateWorld, package and run arrival/overview captures on the host display. Judge the measured bands, near-black share and dungeon mood. Do not integrate worker-generated maps or ReviewedArrivals.tsv. Windows remains deferred; known Linux render stalls remain unchanged.

## Worker checks

UE5.8.3 Linux, uid1000: final editor/game build returned exit0 with both targets `Result: Succeeded`. A nullRHI lighting audit against all five locally regenerated maps passed (1 test, 0 failures). A B1 editor-game nullRHI smoke launch returned exit0 and logged controller pitch−55/yaw45/roll0, boom1200, control-rotation True and FOV45. Generated maps were restored to baseline pointers before submission. Earlier build/test retries and exact commands/results are recorded in the attempt report. No W4-09d rendered screenshots, measured after-luminance or visual acceptance are claimed in this document.
