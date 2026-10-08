# Brown Rat — presentation spec

## Identity and evidence

Gameplay definition: `Enemy.BrownRat`. Proposed presentation ID: `Presentation.Enemy.BrownRat`. Visual family: **Rat**. Selected floor placement: **B1–B4**, inherited from the [world ledger](../../world-ledger.md#eleven-definition-floor-coverage).

The only rat definition in this slice. Preserve a small quadruped and long bare tail; do not replace it with an upright vermin fighter or enlarge it toward human scale to solve selection.

Inherited Bible context: **level 1; HP 27; classic melee 4–5; live melee 2–5**, with **disputed** melee damage. Exact sources: [classic monster page](https://web.archive.org/web/20020202144834/http://www.t4cbible.com:80/monster.html) and [live Monster](https://www.t4cbible.com/Monster); retrieval **2026-10-07**, inherited R-02 capture. Historical monster-page server/patch is unspecified; live page states no game version. These are documentary statements, not verified-in-play damage outcomes or tuning installed by this spec. See [Bible roster](../../ruleset-bible-v1.md#monster-data-w4-02-not-a-w1-02-combat-coefficient) for unresolved reward semantics. Size/equipment must not alter these mechanics.

## Silhouette and reading

**Observed, 2026-10-08:** The original is a tiny brown-gray animal on black: a curved back, low head toward image-right, short legs and a very thin tail extending left. Its face is too small to recover reliable eye/ear anatomy. The starter concept visibly adds coarse brown fur, large rounded ears, pale toes and a long curved pink-brown tail. Those details and the unseen rear are interpretations, not recovered anatomy.

**Prototype production direction:** Read as a low bean-shaped back with a pointed snout and continuous thin tail. Keep belly clearance and four foot contacts legible in motion. At maximum zoom the back and tail matter more than whiskers; the rat name/ring must remain available when the body is only a small screen shape.

## Scale and collision

All dimensions below are **Prototype**, A-01 / LH_Prototype_v1 / 2026-10-08, source URL null; inherited W0-04 values are identified. Units, root placement and [clearance method](README.md#units-root-and-clearance) apply.

| Measurement | cm |
|---|---:|
| Visible resting top above floor | 25 |
| Maximum animated top above floor | 35 |
| Full fore–aft length, including appendages/equipment in transit | 90 |
| Full lateral span in transit | 28 |
| Upright capsule radius R | 25 |
| Upright capsule half-height HH | 25 |
| Capsule diameter × full height | 50 × 50 |
| Conservative transit turn diameter, rounded up | 95 |

Body length 45 cm excluding the approximately 45 cm tail, inherited from W0-04; full 90 cm length includes it. Keep the tail curled inside the stated box during turns/death. The 50 cm-tall logical capsule exceeds the visible back: selection and low-obstacle LOS need explicit W4 review.

**Arithmetic check against ordinary route minima:** max(visual width, capsule diameter) = 50 cm, leaving 190 cm total width at a 240 cm door, 270 cm in a 320 cm corridor and 250 cm on a 300 cm stair. max(animated top, capsule height) = 50 cm, leaving 250 cm under the 300 cm door/headroom requirement. The 95 cm transit turn circle is below a 320 cm landing side by 225 cm. Margins are total space, not guaranteed space per side or room for another actor.

No intended ordinary route is flagged as dimensionally too small at these bounds. Turn on the landing, not inside the door; maintain the envelope during attack/hit/death, and review actual swept limbs on stairs. This is a paper comparison only: collision, selection, slope/step contact, moving player overlap and pathfinding remain untested.

## Materials and palette

Fur umber `#594537`, back highlight `#806850`, bare tail/ears muted rose-brown `#9A7865`. Opaque rough fur volume with normal detail; no fur-card cloud or luminous eyes. Keep the back lighter than the deepest floor shadows. All hex swatches and material responses are A-01 proposals consistent with [shared art direction](README.md#camera-and-silhouette-rules), not sampled source colors. Budget: **Rat** row in the [shared budget table](README.md#proposed-production-budgets).

## Rig and animation coverage

Dedicated quadruped: floor `root`, pelvis/spine/chest, neck/head/jaw, four leg chains and a short tail chain. Ear bones are optional. Required sockets: `VFX_Bite` at mouth, `VFX_Hit` at chest, `UI_Anchor` above back. All positions scale with the mesh, not the actor capsule. All rig anatomy beyond the sampled source view is authored reconstruction. Use the [shared timing contract](README.md#rig-and-attack-timing-contract): in-place floor root, 30 fps preview; f0 native commit, **f30 = 1.000 s impact**, visual recoil through f42. Current fixture cooldown is 3.0 s from commit, not a recovery lock. Production delay remains data-owned and must retime the contact key to `ImpactSeconds`.

| Clip/state | Required presentation |
|---|---|
| Idle | Small breathing/sniff and tail settle; no idle root drift. |
| Locomotion | Scurry loop with grounded feet, plus slower walk/start/stop blends; author measured stride metadata for runtime speed matching. |
| Attack windup, f0–f29 | Lower snout, pull chest back and briefly bunch the hindquarters; keep the root fixed. |
| Attack impact, f30 | Short forward head snap with jaw open at the bite pose; no body-root lunge. |
| Recoil, f31–f42 | Settle toward idle, interruptibly; one impact only. |
| Hit | Brief shoulder recoil layered over movement; do not postpone a committed bite. |
| Death + dead hold | Roll onto one side, legs settle and tail curls into the 90 × 28 cm box; dead hold has no further breath or autonomous motion. |
| Special / transitions | No extra ability. A sniff/look variation is cosmetic and optional. |

Contact is a visual key, never a damage notify. Commit/cancel animation hooks are a pending integration seam; native impact/death results drive feedback. A failed range/LOS validation may emit no `OnImpact`, so clear the cosmetic attack through its lifecycle, not by waiting indefinitely for a hit event. Death interrupts presentation; loot and corpse collision/removal belong to the lifecycle owner.

## Placeholder and replacement

Flattened umber sphere for body (45 × 28 × 25 cm), small snout cone, four short cylinders and a thin curved chain of cylinders for the 45 cm tail; muted rose-brown tail. Keep total assembly in the stated envelope. These are proposed engine-shape assemblies, not assets created here. Disable collision on the visual pieces; retain the reviewed capsule and stable entity/life identity. Set the floor root under the capsule as described in the README.

W4-02 maps `Enemy.BrownRat` → `Presentation.Enemy.BrownRat` → the proxy, then W5-03 replaces that binding with the approved **Rat** mesh, materials and animation set. See the [unimplemented resolver boundary](README.md#presentation-ids-and-replacement-seam). Do not edit HP, timing, reach, rewards or capsule as part of an art-only swap. Review at the 900/1200/1800 cm camera distances, in silhouette/grayscale and beside relevant family variants, before accepting the replacement.

## Provenance and rights

Actually opened: `P/assets/references/monsters/rat-family.jpg` and `P/assets/concepts/starter-characters-and-creatures-concept.png`, where [P is the read-only package root](README.md#images-actually-opened-and-rights). Original stable ID `rat-family`; [source page](https://t4cfantasy.com/Bible/Classic/MonsterImages.php), [exact image](https://t4cfantasy.com/images/skins/app20003.jpg), retrieved **2026-10-07**; Fantasy Classic hosting, original artist/client patch unspecified. Concept generated **2026-10-07**, inputs recorded in `P/assets/concepts/prompts.md`; it supplies direction, not dimensions/animation or permission.

**No commercial licence or distribution permission is established** for either source or concept. Do not cook their pixels as this creature. New anatomy, variants, materials and rig are explicit design proposals; production original/licensed assets require their own provenance and rights record. No mesh, import, animation, editor, navigation, cook, package or play result is delivered by this document.
