# W4-09e B3/B4 exposure and native capture camera

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
