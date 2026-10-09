# W4-09: explicit graybox lighting

Source inspection on 2026-10-09 found three different exposure contracts. This is a code correction with pending host visual calibration, not an observed visual pass.

| Map | Evidence / likely contribution | Changed exposure |
| --- | --- | --- |
| Hub | Equal bounds 6, no metering override; project did not pin extended luminance. A 3000 lux sun with this low exposure target can clip; disabling adaptation does not establish the correct exposure. | Manual physical EV100 10, ISO100, f/2, shutter256 |
| B1/B2 | Manual physical EV1006 (f/2, shutter16) despite 600–1200 lm point lights and .5/.4 neutral fill: much less exposure than B3/B4. No automatic adjustment can compensate. | Manual physical EV1002, ISO100, f/2, shutter1 (16× the previous exposure) |
| B3/B4 | Equal automatic bounds2, inherited metering and unpinned extended-range interpretation. B3 reported dim; B4 not observed. | Manual physical EV1002, same interior camera contract |

These are **Prototype presentation tuning**, derived from A-04 intent and UE's installed renderer formula, not historical mechanics. Replace through host neutral-card and gameplay-view calibration. UE 5.8.3 `PostProcessEyeAdaptation.cpp:526` computes `log2(fstop² * shutter * 100 / ISO)`. Manual physical exposure removes dependence on automatic-bound interpretation; explicit equal bounds are retained for audit and future metering changes. Bias remains0; hub/B1/B2 bloom and motion blur remain0. Project extended luminance is explicitly enabled for consistent EV bound semantics.

All five generators already use **Movable** local, sun and sky lights. Static/stationary lightmaps are therefore not the identified cause. `r.AllowStaticLighting=False` now deliberately selects an entirely dynamic graybox pipeline. Static geometry remains static. Generation with `-nullrhi` produces no baked lighting or render capture; specified cubemap references and movable direct lights are the intended inputs. Both `GrayLightTextureCube` and `GrayTextureCube` exist in the installed engine. B1/B2 retain the former; B3/B4 retain the latter; hub retains its authored embedded neutral cube. Cubemap filtering/cook and actual sky contribution still need real-RHI host review.

No renderer settings previously pinned GI/reflections. The new rendering section explicitly disables dynamic GI and reflections: A-04 neutral-fill/direct-light grayboxes do not require Lumen or a lighting build. No Vulkan, synchronization, shadow algorithm or engine changes are made. Existing per-light shadows remain authored. Arrival transforms and geometry are unchanged.

## Audit and capture

`Lighthaven.World.LightingAudit` loads all five registry map packages headlessly and checks local inventories20/12/17/17/13, one sky per map, exactly one sun in the hub, all lights movable, point/spot units lumens and intensities275..3200, sun3000 lux, sky.26...70, specified non-null sky cube, one unbound manual physical-camera volume, overridden physical parameters giving EV10/2, equal overridden bounds and zero overridden bias. Thresholds are the existing authored Prototype inventory, not visual acceptance thresholds. Regenerate maps before running; LFS pointers are not valid input maps.

Host steps after build:

1. Regenerate using the three existing generation scripts, preserving reviewed arrival hashes; coordinator owns generated binaries.
2. Run the headless audit with `Automation RunTests Lighthaven.World.LightingAudit; Quit` (normal-user `UnrealEditor-Cmd`, `-nullrhi`, existing environment recipe).
3. Package all five maps in a Development archive or use the built editor on the desktop. Run `bash build/capture-map-screenshots.sh /absolute/path/to/Lighthaven` (or `UnrealEditor`) from the checkout. Requires Python3 and ImageMagick already installed; does not install it.
4. Inspect `Saved/LightingCapture/<timestamp>/`. Each independent launch uses the current registry first arrival/Temple.SafeSpawn and BugItGo with a180cm eye-height offset; this does not edit arrivals. `EnableCheats`/BugItGo must log successful placement or the capture is rejected. HighResShot requests120 delay frames for rendering warmup. Script prints mean/median normalized Rec.709 **sRGB luma**, excluding UI via HighResShot. These are diagnostic screen statistics, not scene luminance or a visual gate. Confirm actual third-person camera framing in the images; controller camera offsets can affect the view.

No sandbox captures or visual conclusions are claimed. A failed launch/camera command/missing image fails the script and retains logs. Screenshot directory paths should not contain whitespace (Unreal console filename parsing). An editor or Development executable with cheat commands is required; Shipping is unsupported.
