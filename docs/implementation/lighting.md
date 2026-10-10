# Lighting and native capture camera

## W5-09b runtime registration correction (2026-10-10)

The W5-09 host captures supersede its predicted means: B1 actually measured
.0062/.0008/.0006, with near-black shares .92/.9995/.9995. Its component-count
audit missed a render lifecycle defect. A diagnostic-only `UnrealEditor-Cmd
-game -nullrhi` baseline recorded **all 59 torches registered during PostLoad
before world initialization and before a scene existed**. Their physical settings,
world positions and visibility were correct. UE5.8's registration code skips
render-state creation without a scene, subsequently skips already registered
components, and cannot repair a nonexistent render state by dirtying it during
flicker. NullRHI itself never creates render states; the before-init registration
is the observed defect, and the missing rendered-light consequence follows from
the installed engine source. See the appended [W5-09b evidence](lighting-b1-w5-09.md).

`Configure` now attaches/positions the transient torch first and registers only
after `World->IsInitialized()`, including a retry for an existing unregistered
component. Normal actor registration after world initialization handles loaded
fixtures. `RF_Transient` is retained: the saved recipe reconstructs the light.
No lumen, radius, exposure, sky, player fill, geometry, post-process or shadow
setting changes. Game-world diagnostic receipts report each fixture at creation,
PostLoad and BeginPlay, including registration, render state, position, intensity,
units, attenuation, visibility, mobility, world influence and draw distance.

`Lighthaven.World.B1LoadedGameLightingAudit` loads serialized B1 into a fresh
Game world, checks all 59 lights **before** scene initialization, then initializes
the world and runs actor BeginPlay. It checks registration, flame positions and
physical settings, and checks render states when a render-capable RHI is present.
It computes nominal unoccluded direct illuminance from those registered visible
lights, using the UE lumen conversion and inverse-square/radius falloff:

`E = lm * 10000 / (4*pi) * max(0, 1 - (d²/R²)²)² / (d² + 1) * max(0, N·L)`

Distances are in cm; results are lux before temperature/tint. Receiver probes
below and beside every torch require at least 19 floor lux and 80 vertical lux.
The floor is z0 (264 cm below the flame); the vertical receiver is 100 cm sideways
and 64 cm below it. These probes test light delivery; the freestanding fixtures
do not necessarily have a physical wall at the receiver. Four room centres must
exceed 10 torch lux, and four selected dark corners remain below 6 torch lux.
The test independently reconstructs the old 12 landmark + 47 coverage lights and
compares both layouts at identical points. References are UE5.8
`PointLightComponent.cpp:145`, `DeferredLightingCommon.ush:260,309` and
`CapsuleLightIntegrate.ush:56,59`. The estimate excludes finite-source horizon
wrapping, RGB temperature/tint, materials, sky/fill, fog, exposure and tonemapping;
it is a delivery regression, not a rendered luminance measurement.

Nominal torch-only centre illuminance is 11.43–19.77 lux versus the old grid's
15.92–29.85; selected corners are 2.69–5.46 versus 4.54–10.88 lux. This is
consistent with less broad fill than the old grid (host mean approximately .3 at
EV100 2.5). The **new capture prediction is .20 ± .05**, aiming at the unchanged
.15–.25 band. This is a Prototype hypothesis with substantial uncertainty, not a
linear lux-to-sRGB conversion or a measured result. Full-frame near-black <.05
and visual readability still require fresh rendered captures; intentional void
and the reduced sky may continue to miss that criterion.

## W5-09 B1 atmosphere (2026-10-10)

B1 now has a separate **Prototype presentation** profile. `ALHVisualPiece::Build` attaches one transient warm light to every B1 Torch/Sconce recipe, including pieces loaded from existing maps. Each uses 2000 K, 1800 lm, inverse-square falloff, 700 cm attenuation radius and a 12 cm source radius. Two deterministic game-time sine waves modulate intensity by at most 6%; the phase derives from fixture position, not actor names, random state or wall-clock time. Comparing identical simulation times reproduces the intensity; waiting a fixed number of frames with variable frame durations does not imply identical game time.

The B1 generator preserves all dressing inputs, then deletes the old 12 landmark and 47 grid point-light actors **after** dressing. This retains all 59 fixture placements and every gameplay transform. B1 has 59 attached torch lights, one pawn-area readability light and one skylight; **zero torch/fill shadow casters**. The point-light radius is much smaller than the old 1400 cm grid. No Lumen, ray tracing, volumetric fog or shadow atlas is introduced. Overlap cost and unshadowed light leaking through thin walls still need the GTX 1050 Ti host check. This inventory excludes the existing character `SpellLight`: it remains hidden until the Light ability is active and retains its existing shadow setting, adding one runtime point light when visible.

Neutral skylight intensity drops from .8 to .12 with a cool (.65,.75,1) linear tint. A 450 lm, 750 cm radius, cool (.55,.7,1) light follows the currently possessed pawn at +220 cm Z; it hides without a pawn and updates on repossession. It affects player/enemies/loot/nearby route together without changing gameplay or applying separate emissive materials. Manual physical exposure stays **EV100 2.5**, ISO100, f/2, reciprocal shutter `2^2.5/4`, bias0. Keeping exposure fixed makes the fixture/fill redistribution the dominant change. B2 keeps its previous lighting; hub EV10 and B3/B4 EV4 also remain unchanged.

B1 post-process adds world-space AO .65/radius120 cm, bloom .25/threshold1, saturation .95, highlight gain (1.04,1.01,.96), shadow gain (.97,1,1.04), vignette .2 and no motion blur. Nonvolumetric exponential height fog uses density .008, height falloff .25, max opacity .12, cool inscattering (.10,.14,.20). The cool fog radiance is intended to soften the otherwise black exterior void to a faint blue-grey at distance while the .12 opacity cap retains contrast; actual fog/exposure response needs a host capture. These numbers are a first visual hypothesis, source W5-09 / concept comparison, source URL null; none is a mechanics value or measured acceptance result.

The **acceptance band remains mean exported sRGB luma .15–.25, near-black share <.05** (strictly below .02). No threshold is relaxed to pass this change. Predicted room means are approximately .20/.18/.21 (uncertainty at least ±.05), inferred from reduced sky/coverage and retained exposure, **not rendered measurements**. Host captures must determine actual means and player/enemy/loot readability. Baseline room1/2 already have .1049/.1018 near-black share, mostly exterior void: therefore even unchanged framing can miss the <.05 requirement. Report full-frame metrics, and optionally report a separately identified gameplay-surface region for diagnosis; never silently crop or count a region-only pass as a full-frame pass.

`Lighthaven.World.LightingAudit` checks the new B1 component inventory, temperature/falloff/shadow policy, fog and post-process settings alongside unchanged settings for the other four maps. NullRHI cannot measure image luminance. Coordinator regeneration of B1 is required to remove legacy fill/grid and install fog/post; runtime kit/light reconstruction alone does not replace those map actors. See [W5-09 evidence and integration notes](lighting-b1-w5-09.md) for actual checks and open visual acceptance.

## W4-09e history: B3/B4 exposure

All values are **Prototype presentation tuning**, not authentic T4C mechanics. The brief reports W4-09d host mean exported sRGB luma: hub .28, B1 .27, B2 .24, B3 .41, B4 .40. Basement target is .15–.25 with near-black share <.05. B1/B2 remain unchanged at EV100 2.5; hub remains EV10. Only B3/B4 move to manual physical EV100 **4**, +1.5 stops (scene-linear exposure multiplier .353553). ISO100, f/2, reciprocal shutter4 give `log2(4 × 4 × 100 / 100)=4`; both overridden bounds are4, bias0. Landmark lights, coverage, sky.8, materials, geometry, collision and arrivals remain unchanged.

This is a tuning hypothesis requiring a new host capture, **not an observed mean in the target band**. A previous 1.5-stop change reduced B3 .68→.41, rather than scaling sRGB means linearly. Another 1.5 stops is a reasonable first iteration toward .20, but tonemapping, framing and intentional void can change that response. Do not treat the exposure formula or nullRHI audit as luminance acceptance.

## Capture method verified in source

`Source/Lighthaven/Framework/LHCharacter.cpp` constructs `ALHCharacter::CameraBoom` with absolute rotation, relative rotation pitch−55/yaw45/roll0, arm1200 cm, collision enabled and sphere probe12 cm. `ALHCharacter::Camera` is its active attached `UCameraComponent`, FOV45. The controller possesses this character; capture does not create a separate camera actor.

Engine5.8.3 source confirms the path: `UCheatManager::BugItWorker` teleports/faces the pawn and sets controller rotation, then enables Ghost. `APlayerController::Camera(Default)` selects the camera manager's Default style; `APlayerCameraManager::UpdateViewTarget` routes that style through `UpdateViewTargetInternal` to the target actor's `CalcCamera`. `AActor::CalcCamera` chooses its first active camera component. `USpringArmComponent::GetTargetRotation` follows pawn view rotation only when `bUsePawnControlRotation` is true; otherwise the character's absolute boom rotation controls the view.

The script now keeps the gameplay camera defaults instead of enabling control rotation and repeatedly importing individual controller rotation members. This removes the W4-09d alternate rotation path. The precise cause of the prior host's floor-heavy frame is still unconfirmed: component settings alone do not prove the rendered viewpoint. A stale packaged build, a different view target or collision-shortened boom must be distinguished using the new logs and image. The script requests `Camera Default` and logs camera manager identity, possessed pawn, boom relative/absolute rotation, arm length, control-rotation flag, collision flag, camera relative location, control-rotation flag and FOV. Expected boom values: (-55,45,0), absolute True,1200, control-rotation False. CameraStyle is not a reflected engine property, so Default style is established by the console command and source path, not by getall. These getall samples occur at startup and do not report the collision-adjusted world camera pose. They are diagnostics, not a late-frame pose assertion. No diagnostic overlay contaminates luminance measurements.

Run from this checkout as the desktop user against freshly built/regenerated content:

```sh
bash build/capture-map-screenshots.sh /absolute/path/to/Development-Lighthaven-or-UnrealEditor
bash build/capture-map-screenshots.sh /absolute/path/to/Development-Lighthaven-or-UnrealEditor --overview
```

Registry arrival ground positions plus90 cm remain the pawn targets; pawn rotation remains upright. Native collision can shorten the effective1200 cm boom near walls; capture intentionally preserves the gameplay probe. If framing still fails, inspect the log for the expected pawn/camera and compare arrival with optional room-center overview before accepting. Do not move arrivals or disable collision to hide it.

The script retains independent launches,1280×720 screenshots after120 warmup frames,15 s lifetime and55 s timeout. Evidence lives under `Saved/LightingCapture/<timestamp>/`. It requires a host display, Development cheats, Python3 and ImageMagick; installs nothing. Each image reports normalized Rec.709 exported sRGB luma mean/median and near_black (fraction strictly below.02). These screen diagnostics include intentional void and are not scene-linear illuminance. Placement, launch, image and console property failures fail the script.

## Audit and integration

`Lighthaven.World.LightingAudit` expects hub EV10, B1/B2 EV2.5, B3/B4 EV4, checking the physical camera formula and overridden bounds. All other structural thresholds remain intact. The coordinator must regenerate maps, run the audit, arrival review and LHValidateWorld, package and capture on the host display. Check actual mean .15–.25, near-black <.05 and arrival room readability. No worker maps or ReviewedArrivals.tsv belong in the submission. Windows remains deferred and Linux render stalls remain a known limitation.

Worker validation details and limitations are recorded in the attempt report. No W4-09e rendered image, measured final luminance or visual acceptance is claimed.

Final worker checks: editor/game builds succeeded (exit0). LightingAudit JSON reports1 succeeded/0 failed after local regeneration (exit0). B3 editor-game nullRHI smoke exited0 and logged the native character boom−55/45, absolute True, arm1200, collision True, control-rotation False and FOV45. This is property validation, not rendered framing. Maps restored before submission.
