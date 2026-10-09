# Task W4-09b: make the basements readable; fix capture framing

Base: main 0f3880f (W4-08 + W4-09 merged). W4-09 fixed the hub (no longer white) but the basements are still too dark. Coordinator host capture of the packaged build, `build/capture-map-screenshots.sh`, 1280×720, arrival camera:

| Map | mean luminance | median |
|---|---:|---:|
| Hub `L_LighthavenTempleDistrict` | 0.282 | 0.309 |
| B1 `L_TempleB1` | 0.031 | 0.016 |
| B2 `L_TempleB2` | 0.023 | 0.011 |
| B3 `L_TempleB3` | 0.050 | 0.009 |
| B4 `L_TempleB4` | 0.026 | 0.005 |

Visually, B1 is near-black except a faint floor grid in one corner. The hub frame shows only road and floor: the capture camera pitches too steeply at the ground, so it can't judge a room.

## Work
1. **Basements readable at a glance** (dungeon mood, not daylight). Target, on the fixed capture, B1–B4 mean luminance roughly **0.12–0.25** with no large pure-black areas in walkable space, the hub staying ~0.2–0.35. Tune through the generators and `Config/DefaultEngine.ini` rendering sections: interior EV100, local light intensity, attenuation radius and placement density along corridors and rooms, skylight/ambient fill (the basements have no sun), and the indoor post-process. Keep everything Movable/dynamic as W4-09 decided. Document each value as **Prototype presentation tuning** in `lighting.md`.
2. **Capture framing.** Make `build/capture-map-screenshots.sh` frame the arrival room like the game camera (the game's default boom/yaw/pitch from `hub-graybox.md`'s camera review values), not a steep floor view. Add an optional second shot per map from a fixed overview point. Keep the luminance report, and add the share of near-black pixels (< 0.02).
3. **Audit.** Extend `Lighthaven.World.LightingAudit` with your new expectations, e.g. minimum light count per room or corridor segment and the interior fill presence.
4. Don't move arrivals and don't add collision near them; lights must not block. The coordinator re-runs `build/review-arrivals.sh`, `LHValidateWorld` and the capture on the host and judges the result.

## Own only
`Source/LighthavenEditor/Commandlets/LHGenerateHubMapCommandlet.cpp`, `LHGenerateBasementAMapsCommandlet.cpp`, `LHGenerateBasementBMapsCommandlet.cpp` (lighting/post-process code only), `Config/DefaultEngine.ini` (rendering sections only), `Source/LighthavenTests/World/LHLightingAuditTests.cpp`, `build/capture-map-screenshots.sh`, `docs/implementation/lighting.md`.
