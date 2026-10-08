# Goblin — presentation spec

## Identity and evidence

Gameplay definition: `Enemy.Goblin`. Proposed presentation ID: `Presentation.Enemy.Goblin`. Visual family: **Goblin**. Selected floor placement: **B3**, inherited from the [world ledger](../../world-ledger.md#eleven-definition-floor-coverage).

Ordinary compact red biped. Keep bare/slender shoulder silhouette, plain dark waist wrap and a narrow pale bladed polearm. Goblin Warrior shares the 125 cm head height but gains a broad leather shoulder yoke, pale back band and wider weapon head; do not rely only on tint to communicate the more dangerous definition. Carried art does not determine loot or weapon statistics.

Inherited Bible context: **level 5; HP 84; classic melee 5–12; live melee 5–12**, with agreement in the retained classic/live rows. Exact sources: [classic monster page](https://web.archive.org/web/20020202144834/http://www.t4cbible.com:80/monster.html) and [live Monster](https://www.t4cbible.com/Monster); retrieval **2026-10-07**, inherited R-02 capture. Historical monster-page server/patch is unspecified; live page states no game version. These are documentary statements, not verified-in-play damage outcomes or tuning installed by this spec. See [Bible roster](../../ruleset-bible-v1.md#monster-data-w4-02-not-a-w1-02-combat-coefficient) for unresolved reward semantics. Size/equipment must not alter these mechanics.

## Silhouette and reading

**Observed, 2026-10-08:** The original is a small crouched red humanoid with pointed head/ear projections and a thin upright pale shaft/blade at image-right. It does not resolve detailed armour or handedness. The deep concept adds long pointed ears, sharp teeth, straps, ornaments, a loincloth and a tall double-edged spear/polearm. Keep the red biped and shaft silhouette, but treat clothing, rear anatomy and blade construction as proposals.

**Prototype production direction:** Three large cues: triangular ear/head silhouette, low bent-knee red torso and one narrow vertical weapon. Use the bare shoulder line to separate this variant from Warrior. Do not substitute a green cartoon goblin or human-height knight.

## Scale and collision

All dimensions below are **Prototype**, A-01 / LH_Prototype_v1 / 2026-10-08, source URL null; inherited W0-04 values are identified. Units, root placement and [clearance method](README.md#units-root-and-clearance) apply.

| Measurement | cm |
|---|---:|
| Visible resting top above floor | 185 |
| Maximum live animated top above floor | 200 |
| Full fore–aft length, including appendages/equipment in transit | 110 |
| Full lateral span in transit | 85 |
| Root-to-front / root-to-rear transit bounds (+X / −X magnitudes) | 65 / 45 |
| Upright capsule radius R | 35 |
| Upright capsule half-height HH | 70 |
| Capsule diameter × full height | 70 × 140 |
| Conservative root-centered transit turn diameter, rounded up | 156 |

Standing head height 125 cm inherits W0-04; table top includes the carried approximately 170 cm polearm in a tilted stance. Maximum 200 cm animated height constrains weapon lifts. Length includes forward weapon angle. The capsule is 70 cm wide × 140 cm tall; the shaft is nonblocking cosmetic geometry.

**Arithmetic check against ordinary route minima:** max(visual width, capsule diameter) = 85 cm, leaving 155 cm total width at a 240 cm door, 235 cm in a 320 cm corridor and 215 cm on a 300 cm stair. max(animated top, capsule height) = 200 cm, leaving 100 cm under the 300 cm door/headroom requirement. The 156 cm root-centered transit turn circle is below a 320 cm landing side by 164 cm. Margins are total space, not guaranteed space per side or room for another actor.

Transit fits ordinary routes arithmetically. **Death exception:** a rigid 170 cm polearm laid flat will not fit the 110 × 85 cm transit box. Reserve a root-centered 190 × 100 cm visual death box (215 cm turn diagonal), still below 240 cm door width when aligned and 320 cm landings; collapse shaft along the corridor, not across its jambs. Do not scale or delete the polearm to hide clipping. Runtime corpse placement and obstacle avoidance need review; the death pose must not add blocking collision.

## Materials and palette

Skin muted iron red `#8B3227`, shaded red-brown `#512D27`, cloth/leather `#48392C`, blade dull pale iron `#B3ABA0`. Keep the ordinary back largely red and bare. Metal highlight is restrained; fine straps are secondary to body/weapon silhouette. All hex swatches and material responses are A-01 proposals consistent with [shared art direction](README.md#camera-and-silhouette-rules), not sampled source colors. Budget: **Goblin** row in the [shared budget table](README.md#proposed-production-budgets).

## Rig and animation coverage

Shared goblin biped: floor `root`, pelvis/spine/head, ears, arms/hands and legs/feet, with polearm grip support. Propose right-hand lead grip and left-hand support as reconstruction, not source handedness. Sockets: `Weapon_Main` on right hand, `Weapon_Support` left-hand IK target on shaft, `VFX_WeaponTip`, `VFX_Hit` at torso and `UI_Anchor` over head. Mesh/equipment use the same agreed skeleton between variants. All rig anatomy beyond the sampled source view is authored reconstruction. Use the [shared timing contract](README.md#rig-and-attack-timing-contract): in-place floor root, 30 fps preview; f0 native commit, **f30 = 1.000 s impact**, visual recoil through f42. Current fixture cooldown is 3.0 s from commit, not a recovery lock. Production delay remains data-owned and must retime the contact key to `ImpactSeconds`.

| Clip/state | Required presentation |
|---|---|
| Idle | Low bent-knee stance, short breath and controlled spear balance. |
| Locomotion | Compact walk/run/start/stop and turning with weapon held upright inside envelope; avoid shaft swing through corners. |
| Attack windup, f0–f29 | Draw the shaft back along the body with an obvious shoulder turn, staying below 200 cm. |
| Attack impact, f30 | Single forward short polearm jab, blade at its maximum authored extension at f30; weapon length does not change authority range. |
| Recoil, f31–f42 | Settle toward idle, interruptibly; one impact only. |
| Hit | Short chest/shoulder recoil, retaining grip alignment. |
| Death + dead hold | Knees buckle and body folds to one side; lay the rigid polearm along the corridor inside the root-centered 190 × 100 cm death envelope specified above. |
| Special / transitions | No ranged throw, shield bash or independent weapon damage socket. Weapon stays attached to corpse until lifecycle cleanup; it is not a free pickup. |

Contact is a visual key, never a damage notify. Commit/cancel animation hooks are a pending integration seam; native impact/death results drive feedback. A failed range/LOS validation may emit no `OnImpact`, so clear the cosmetic attack through its lifecycle, not by waiting indefinitely for a hit event. Death interrupts presentation; loot and corpse collision/removal belong to the lifecycle owner.

## Placeholder and replacement

Two red spheres/capsule-like cylinders for head/torso, thin limb cylinders, two ear cones, brown waist cube and a pale narrow cylinder shaft with one cone blade. Pose to 125 cm head/185 cm total carried top. These are proposed engine-shape assemblies, not assets created here. Disable collision on the visual pieces; retain the reviewed capsule and stable entity/life identity. Set the floor root under the capsule as described in the README.

W4-02 maps `Enemy.Goblin` → `Presentation.Enemy.Goblin` → the proxy, then W5-03 replaces that binding with the approved **Goblin** mesh, materials and animation set. See the [unimplemented resolver boundary](README.md#presentation-ids-and-replacement-seam). Do not edit HP, timing, reach, rewards or capsule as part of an art-only swap. Review at the 900/1200/1800 cm camera distances, in silhouette/grayscale and beside relevant family variants, before accepting the replacement.

## Provenance and rights

Actually opened: `P/assets/references/monsters/goblin-family.jpg` and `P/assets/concepts/deep-dungeon-creatures-concept.png`, where [P is the read-only package root](README.md#images-actually-opened-and-rights). Original stable ID `goblin-family`; [source page](https://t4cfantasy.com/Bible/Classic/MonsterImages.php), [exact image](https://t4cfantasy.com/images/skins/app20001.jpg), retrieved **2026-10-07**; Fantasy Classic hosting, original artist/client patch unspecified. Concept generated **2026-10-07**, inputs recorded in `P/assets/concepts/prompts.md`; it supplies direction, not dimensions/animation or permission.

**No commercial licence or distribution permission is established** for either source or concept. Do not cook their pixels as this creature. New anatomy, variants, materials and rig are explicit design proposals; production original/licensed assets require their own provenance and rights record. No mesh, import, animation, editor, navigation, cook, package or play result is delivered by this document.
