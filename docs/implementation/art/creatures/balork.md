# Balork — presentation spec

## Identity and evidence

Gameplay definition: `Enemy.Balork`. Proposed presentation ID: `Presentation.Enemy.Balork`. Visual family: **Demon**. Selected floor placement: **B4; final encounter, not an ordinary respawning roster slot**, inherited from the [world ledger](../../world-ledger.md#eleven-definition-floor-coverage).

Single boss definition, shared with the [final-encounter companion](balork-final-encounter.md). Distinguish from Giant Bat with a horned upright demon torso, legs, long polearm and broad red/black patterned wings. Do not create a separate encounter Enemy ID or interpret larger art as new reach, phases or summons.

Inherited Bible context: **level 15; HP 508; classic melee 13–29; live melee 13–29**, with agreement in the retained classic/live rows. Exact sources: [classic monster page](https://web.archive.org/web/20020202144834/http://www.t4cbible.com:80/monster.html) and [live Monster](https://www.t4cbible.com/Monster); retrieval **2026-10-07**, inherited R-02 capture. Historical monster-page server/patch is unspecified; live page states no game version. These are documentary statements, not verified-in-play damage outcomes or tuning installed by this spec. See [Bible roster](../../ruleset-bible-v1.md#monster-data-w4-02-not-a-w1-02-combat-coefficient) for unresolved reward semantics. Size/equipment must not alter these mechanics.

## Silhouette and reading

**Observed, 2026-10-08:** The original’s red patterned wing fills most of the left/top image, attached to a dark red body with horn-like projections and a long pale-bladed shaft extending right. Wing/weapon extremities meet or cross frame edges; it is not a complete turnaround. The deep concept supplies two broad patterned wings, a frontal horned torso, hooved-looking feet and a large double-ended poleaxe. Front/rear reconstruction, hoof form and precise blade anatomy remain interpretations.

**Prototype production direction:** Largest figure: broad scalloped wing pair with large dark/red membrane patches, horn pair above a substantial torso and a long pale-edged polearm. Keep separate negative space between wing/body/weapon. Do not replace the patterned membranes with a uniform black silhouette or rely only on glowing eyes to separate him from the dark room.

## Scale and collision

All dimensions below are **Prototype**, A-01 / LH_Prototype_v1 / 2026-10-08, source URL null; inherited W0-04 values are identified. Units, root placement and [clearance method](README.md#units-root-and-clearance) apply.

| Measurement | cm |
|---|---:|
| Visible resting top above floor | 310 |
| Maximum live animated top above floor | 310 |
| Full fore–aft length, including appendages/equipment in transit | 200 |
| Full lateral span in transit | 440 |
| Root-to-front / root-to-rear transit bounds (+X / −X magnitudes) | 100 / 100 |
| Upright capsule radius R | 100 |
| Upright capsule half-height HH | 155 |
| Capsule diameter × full height | 200 × 310 |
| Conservative root-centered transit turn diameter, rounded up | 484 |

Head 260 cm, horn tips 310 cm, spread wings 440 cm, body capsule 200 cm wide × 310 cm tall and polearm approximately 400 cm inherit W0-04. Keep full wings spread and carry the polearm across the body inside the 440 × 200 cm transit box. Grip near the shaft center and verify combined wing/weapon bounds; individual dimensions alone do not guarantee that box. No folded-wing transit is assumed.

**Arithmetic check against boss route minima:** max(visual width, capsule diameter) = 440 cm, leaving 60 cm total width at a 500 cm door, 110 cm in a 550 cm corridor and 110 cm on a 550 cm stair. max(animated top, capsule height) = 310 cm, leaving 50 cm under the 360 cm door/headroom requirement. The 484 cm root-centered transit turn circle is below a 600 cm landing side by 116 cm. Margins are total space, not guaranteed space per side or room for another actor.

**Fails ordinary routes:** wings exceed the 240 cm door by 200 cm, 320 cm corridor by 120 cm and 300 cm stair by 140 cm; 310 cm horns/capsule exceed 300 cm headroom by 10 cm. The 484 cm turn circle also exceeds ordinary 320 cm pads. Use only the [B4 authored boss extent](../../layout/b4.md#boss-navigation-and-retreat-footprint): complex rooms, C04 and the reserved Hall apron. C01–C03/return stair are not boss transit. **Optional attack exception:** a wider room-only swing may use at most a 520 cm horizontal swept diameter at ≤310 cm height; it will NOT fit a 500 cm boss doorway (20 cm excess). Use the compact jab there; the wider variant is not required and must stay disabled until presentation/AI can select it without changing timing, hit count or range. A 550 cm corridor/600 cm pad only clears that bound arithmetically. The 220 cm dev low ceiling also fails by 90 cm. Full animation, corpse and player-passing sweeps remain untested.

## Materials and palette

Body oxblood `#6B2522`, dark plates/wing pattern `#291F22`, membrane red `#A33A29`, horn aged bone `#B7A181`, polearm edge dull steel `#A9A39B`. Broad wing mottling is pigmentation with roughness variation, not lava/emission. Avoid three overlapping translucent membrane layers or screen-filling glow. All hex swatches and material responses are A-01 proposals consistent with [shared art direction](README.md#camera-and-silhouette-rules), not sampled source colors. Budget: **Demon / Balork** row in the [shared budget table](README.md#proposed-production-budgets).

## Rig and animation coverage

Dedicated demon skeleton: floor `root`, pelvis/spine/chest/head/horns, legs/feet, arms/hands, paired wing shoulder/elbow/wrist/finger chains. Root never flies. Polearm mid-grip lead hand and support-hand IK; handedness is a proposal. Sockets: `Weapon_Main`, shaft `Weapon_Support`, `VFX_WeaponTip_A/B`, `VFX_Hit` at chest, `UI_Anchor` above horns, `WingTip_L/R` for bounds inspection. Two tips never imply two impacts. All rig anatomy beyond the sampled source view is authored reconstruction. Use the [shared timing contract](README.md#rig-and-attack-timing-contract): in-place floor root, 30 fps preview; f0 native commit, **f30 = 1.000 s impact**, visual recoil through f42. Current fixture cooldown is 3.0 s from commit, not a recovery lock. Production delay remains data-owned and must retime the contact key to `ImpactSeconds`.

| Clip/state | Required presentation |
|---|---|
| Idle | Measured breathing, restrained wing tension and polearm balance inside the full-spread envelope. |
| Locomotion | Grounded walk/start/stop and deliberate turns with wings held broad; no takeoff, teleport or root-motion charge. |
| Attack windup, f0–f29 | Brace legs and draw the central polearm grip back at torso height. Avoid an overhead weapon lift beyond 310 cm or a lateral spin through a doorway. |
| Attack impact, f30 | Single compact polearm jab/downward press at f30; keep near-door attack in the transit envelope. For an optional wider room-only presentation, honor the separate sweep exception above. |
| Recoil, f31–f42 | Settle toward idle, interruptibly; one impact only. |
| Hit | Small chest/wing recoil with horns staying inside the height limit; no stun or interrupt rule inferred. |
| Death + dead hold | Knees fold and torso lowers; wings settle inward/down without a large backward fall. Lay the polearm across the spread-wing footprint, then hold an inert boss corpse; review combined corpse bounds. |
| Special / transitions | A short alert/turn-to-face pose may blend during normal authority-driven acquisition; it cannot lock camera/player, delay combat, unlock a phase or award completion. |

Contact is a visual key, never a damage notify. Commit/cancel animation hooks are a pending integration seam; native impact/death results drive feedback. A failed range/LOS validation may emit no `OnImpact`, so clear the cosmetic attack through its lifecycle, not by waiting indefinitely for a hit event. Death interrupts presentation; loot and corpse collision/removal belong to the lifecycle owner.

## Placeholder and replacement

Oxblood torso/leg primitives, two tall horn cones at 310 cm, broad red flattened cube/cone wing assemblies spanning 440 cm with dark pattern blocks, and a centered 400 cm cylinder polearm with pale cone/blade proxies. This is a scale/identity stand-in, not a textured crop of the incomplete original. These are proposed engine-shape assemblies, not assets created here. Disable collision on the visual pieces; retain the reviewed capsule and stable entity/life identity. Set the floor root under the capsule as described in the README.

W4-02 maps `Enemy.Balork` → `Presentation.Enemy.Balork` → the proxy, then W5-03 replaces that binding with the approved **Demon** mesh, materials and animation set. See the [unimplemented resolver boundary](README.md#presentation-ids-and-replacement-seam). Do not edit HP, timing, reach, rewards or capsule as part of an art-only swap. Review at the 900/1200/1800 cm camera distances, in silhouette/grayscale and beside relevant family variants, before accepting the replacement.

## Provenance and rights

Actually opened: `P/assets/references/monsters/balork-demon-family.jpg` and `P/assets/concepts/deep-dungeon-creatures-concept.png`, where [P is the read-only package root](README.md#images-actually-opened-and-rights). Original stable ID `balork-demon-family`; [source page](https://t4cfantasy.com/Bible/Classic/MonsterImages.php), [exact image](https://t4cfantasy.com/images/skins/app20013.jpg), retrieved **2026-10-07**; Fantasy Classic hosting, original artist/client patch unspecified. Concept generated **2026-10-07**, inputs recorded in `P/assets/concepts/prompts.md`; it supplies direction, not dimensions/animation or permission.

**No commercial licence or distribution permission is established** for either source or concept. Do not cook their pixels as this creature. New anatomy, variants, materials and rig are explicit design proposals; production original/licensed assets require their own provenance and rights record. No mesh, import, animation, editor, navigation, cook, package or play result is delivered by this document.
