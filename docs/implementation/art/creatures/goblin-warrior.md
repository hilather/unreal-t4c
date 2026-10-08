# Goblin Warrior — presentation spec

## Identity and evidence

Gameplay definition: `Enemy.GoblinWarrior`. Proposed presentation ID: `Presentation.Enemy.GoblinWarrior`. Visual family: **Goblin**. Selected floor placement: **B3**, inherited from the [world ledger](../../world-ledger.md#eleven-definition-floor-coverage).

Maintain its own data/presentation ID and visibly heavier threat read beside Goblin. Prototype distinctions: same compact red biped/head height, broad squared leather shoulder yoke, a single pale band across the back, and a wider forked blade on the upright polearm. All are proposed equipment differences; the family picture does not establish Warrior armour or the exact Goblin Blade item model.

Inherited Bible context: **level 12; HP 199; classic melee 10–23; live melee 10–23**, with agreement in the retained classic/live rows. Exact sources: [classic monster page](https://web.archive.org/web/20020202144834/http://www.t4cbible.com:80/monster.html) and [live Monster](https://www.t4cbible.com/Monster); retrieval **2026-10-07**, inherited R-02 capture. Historical monster-page server/patch is unspecified; live page states no game version. These are documentary statements, not verified-in-play damage outcomes or tuning installed by this spec. See [Bible roster](../../ruleset-bible-v1.md#monster-data-w4-02-not-a-w1-02-combat-coefficient) for unresolved reward semantics. Size/equipment must not alter these mechanics.

## Silhouette and reading

**Observed, 2026-10-08:** The shared original presents one small red humanoid, pointed head/ears and tall pale bladed shaft. It does not label a Warrior-specific costume. In the deep concept the red figure has layered straps, ornaments and a bladed staff with a second small blade; use that material direction, while treating this spec’s broad yoke/back band/wider blade as new design.

**Prototype production direction:** A square shoulder/back mass under the triangular ears and wider pale weapon head survive the elevated view. Preserve enough bare red arms/legs to keep the family link. The back band must be large and continuous at far zoom; colour and a tiny helmet badge alone would not separate it from Goblin.

## Scale and collision

All dimensions below are **Prototype**, A-01 / LH_Prototype_v1 / 2026-10-08, source URL null; inherited W0-04 values are identified. Units, root placement and [clearance method](README.md#units-root-and-clearance) apply.

| Measurement | cm |
|---|---:|
| Visible resting top above floor | 210 |
| Maximum live animated top above floor | 215 |
| Full fore–aft length, including appendages/equipment in transit | 120 |
| Full lateral span in transit | 105 |
| Root-to-front / root-to-rear transit bounds (+X / −X magnitudes) | 70 / 50 |
| Upright capsule radius R | 40 |
| Upright capsule half-height HH | 75 |
| Capsule diameter × full height | 80 × 150 |
| Conservative root-centered transit turn diameter, rounded up | 175 |

Head stays at the inherited 125 cm goblin ruler. Heavier yoke widens the shoulders without increasing head height; approximately 190 cm polearm raises the total carried height to 210 cm. Cap all windup tips at 215 cm. Never enlarge runtime actor scale to make Warrior seem stronger.

**Arithmetic check against ordinary route minima:** max(visual width, capsule diameter) = 105 cm, leaving 135 cm total width at a 240 cm door, 215 cm in a 320 cm corridor and 195 cm on a 300 cm stair. max(animated top, capsule height) = 215 cm, leaving 85 cm under the 300 cm door/headroom requirement. The 175 cm root-centered transit turn circle is below a 320 cm landing side by 145 cm. Margins are total space, not guaranteed space per side or room for another actor.

Transit/compact attack fit ordinary routes arithmetically, including the 215 cm tip beneath the dev room’s 220 cm low ceiling with only 5 cm visual margin. **Death exception:** rigid 190 cm weapon requires a root-centered 210 × 110 cm visual death box (238 cm diagonal), still below a 320 cm landing. Align it along the corridor and review jamb/corner clearance; a cross-door fall is not certified by that arithmetic. Corpse visuals remain nonblocking. Low ceiling and death transitions need real animation review.

## Materials and palette

Skin deep iron red `#793127`, broad leather yoke `#4E4031`, pale back band dusty linen `#C0AC86`, wider weapon edge worn iron `#B8B1A3`. No polished plate suit or luminous rank marking; rough leather and one broad light panel supply top-view contrast. All hex swatches and material responses are A-01 proposals consistent with [shared art direction](README.md#camera-and-silhouette-rules), not sampled source colors. Budget: **Goblin** row in the [shared budget table](README.md#proposed-production-budgets).

## Rig and animation coverage

Reuse the goblin skeleton and lead/support grip arrangement. Separate equipment mesh or authored silhouette variant for yoke/weapon; no new collision primitives on armour. Required floor `root`, pelvis/spine/head/ears, limbs/hands; `Weapon_Main`, shaft `Weapon_Support`, `VFX_WeaponTip`, `VFX_Hit`, `UI_Anchor`. Verify the wider blade does not break the shared jab pose. All rig anatomy beyond the sampled source view is authored reconstruction. Use the [shared timing contract](README.md#rig-and-attack-timing-contract): in-place floor root, 30 fps preview; f0 native commit, **f30 = 1.000 s impact**, visual recoil through f42. Current fixture cooldown is 3.0 s from commit, not a recovery lock. Production delay remains data-owned and must retime the contact key to `ImpactSeconds`.

| Clip/state | Required presentation |
|---|---|
| Idle | Braced bent-knee stance showing the broad shoulder yoke; restrained breathing. |
| Locomotion | Short measured walk/run and start/stop turns with weapon upright. Match actual movement speed without inventing slower Warrior mechanics. |
| Attack windup, f0–f29 | Bring the wider blade back at chest level with a broad shoulder turn; visual weight comes from pose, not an extra gameplay delay. |
| Attack impact, f30 | One compact forward/downward polearm jab at f30, shoulder band still visible from above; no second blade hit. |
| Recoil, f31–f42 | Settle toward idle, interruptibly; one impact only. |
| Hit | Brief upper-body recoil while feet remain grounded; no unapproved stagger or stun. |
| Death + dead hold | Drop to knees then settle sideways with weapon laid along the corridor into the separate death envelope above. |
| Special / transitions | No shield, charge, shout buff or new combo. Retain weapon with corpse; loot rules are independent of visible equipment. |

Contact is a visual key, never a damage notify. Commit/cancel animation hooks are a pending integration seam; native impact/death results drive feedback. A failed range/LOS validation may emit no `OnImpact`, so clear the cosmetic attack through its lifecycle, not by waiting indefinitely for a hit event. Death interrupts presentation; loot and corpse collision/removal belong to the lifecycle owner.

## Placeholder and replacement

Red torso/head primitives at the Goblin ruler, two broad brown shoulder cubes, one pale back-band cube and a taller pale cylinder polearm with a wider two-cone head. These shape differences must be visible beside the ordinary proxy. These are proposed engine-shape assemblies, not assets created here. Disable collision on the visual pieces; retain the reviewed capsule and stable entity/life identity. Set the floor root under the capsule as described in the README.

W4-02 maps `Enemy.GoblinWarrior` → `Presentation.Enemy.GoblinWarrior` → the proxy, then W5-03 replaces that binding with the approved **Goblin** mesh, materials and animation set. See the [unimplemented resolver boundary](README.md#presentation-ids-and-replacement-seam). Do not edit HP, timing, reach, rewards or capsule as part of an art-only swap. Review at the 900/1200/1800 cm camera distances, in silhouette/grayscale and beside relevant family variants, before accepting the replacement.

## Provenance and rights

Actually opened: `P/assets/references/monsters/goblin-family.jpg` and `P/assets/concepts/deep-dungeon-creatures-concept.png`, where [P is the read-only package root](README.md#images-actually-opened-and-rights). Original stable ID `goblin-family`; [source page](https://t4cfantasy.com/Bible/Classic/MonsterImages.php), [exact image](https://t4cfantasy.com/images/skins/app20001.jpg), retrieved **2026-10-07**; Fantasy Classic hosting, original artist/client patch unspecified. Concept generated **2026-10-07**, inputs recorded in `P/assets/concepts/prompts.md`; it supplies direction, not dimensions/animation or permission.

**No commercial licence or distribution permission is established** for either source or concept. Do not cook their pixels as this creature. New anatomy, variants, materials and rig are explicit design proposals; production original/licensed assets require their own provenance and rights record. No mesh, import, animation, editor, navigation, cook, package or play result is delivered by this document.
