# Giant Bat — presentation spec

## Identity and evidence

Gameplay definition: `Enemy.GiantBat`. Proposed presentation ID: `Presentation.Enemy.GiantBat`. Visual family: **Bat**. Selected floor placement: **B2–B4**, inherited from the [world ledger](../../world-ledger.md#eleven-definition-floor-coverage).

Largest bat at 170 cm span. Add a broad shoulder/inner-wing mass and a continuous ochre leading-edge band; these cues supplement size. Plain Bat is narrow and unmarked, Dungeon Bat has blunt tips/dorsal V, and Undead Bat has notched pale outer panels. This is still a bat, not a miniature Balork.

Inherited Bible context: **level 3; HP 55; classic melee 4–8; live melee 4–8**, with agreement in the retained classic/live rows. Exact sources: [classic monster page](https://web.archive.org/web/20020202144834/http://www.t4cbible.com:80/monster.html) and [live Monster](https://www.t4cbible.com/Monster); retrieval **2026-10-07**, inherited R-02 capture. Historical monster-page server/patch is unspecified; live page states no game version. These are documentary statements, not verified-in-play damage outcomes or tuning installed by this spec. See [Bible roster](../../ruleset-bible-v1.md#monster-data-w4-02-not-a-w1-02-combat-coefficient) for unresolved reward semantics. Size/equipment must not alter these mechanics.

## Silhouette and reading

**Observed, 2026-10-08:** The original shared bat reference has pointed, irregularly spread membranes around a tiny brown-gray body. It does not show a giant variant. The starter concept clarifies branching wing fingers, thin leathery membranes and a furry chest. Enlarged inner-wing mass and the leading-edge band here are authored distinctions.

**Prototype production direction:** Wide two-wing silhouette with clear finger arcs and a broad furry shoulder wedge. Retain a small head relative to span. The uninterrupted leading edge must persist at distant LOD; avoid adding horns, upright humanoid torso or demon wing pattern.

## Scale and collision

All dimensions below are **Prototype**, A-01 / LH_Prototype_v1 / 2026-10-08, source URL null; inherited W0-04 values are identified. Units, root placement and [clearance method](README.md#units-root-and-clearance) apply.

| Measurement | cm |
|---|---:|
| Visible resting top above floor | 190 |
| Maximum live animated top above floor | 190 |
| Full fore–aft length, including appendages/equipment in transit | 85 |
| Full lateral span in transit | 170 |
| Root-to-front / root-to-rear transit bounds (+X / −X magnitudes) | 42.5 / 42.5 |
| Upright capsule radius R | 45 |
| Upright capsule half-height HH | 100 |
| Capsule diameter × full height | 90 × 200 |
| Conservative root-centered transit turn diameter, rounded up | 191 |

170 cm span inherits W0-04 and is a common large-route ruler. Torso center around Z=130 cm; tips remain within Z=65–190 cm. Avoid flapping above the 200 cm logical capsule envelope. Wings remain visible clearance obligations even though capsule diameter is only 90 cm.

**Arithmetic check against ordinary route minima:** max(visual width, capsule diameter) = 170 cm, leaving 70 cm total width at a 240 cm door, 150 cm in a 320 cm corridor and 130 cm on a 300 cm stair. max(animated top, capsule height) = 200 cm, leaving 100 cm under the 300 cm door/headroom requirement. The 191 cm root-centered transit turn circle is below a 320 cm landing side by 129 cm. Margins are total space, not guaranteed space per side or room for another actor.

No intended ordinary route is flagged as dimensionally too small at these bounds. Turn on the landing, not inside the door; maintain the envelope during attack/hit/death, and review actual swept limbs on stairs. This is a paper comparison only: collision, selection, slope/step contact, moving player overlap and pathfinding remain untested.

## Materials and palette

Fur dark brown `#45372D`, membrane earth brown `#775638`, leading-edge band ochre `#B29867`. Opaque leathery broad panels with restrained veins; keep shadows under the wings from hiding the target ring. All hex swatches and material responses are A-01 proposals consistent with [shared art direction](README.md#camera-and-silhouette-rules), not sampled source colors. Budget: **Bat** row in the [shared budget table](README.md#proposed-production-budgets).

## Rig and animation coverage

Reuse the bat bone layout with an authored Giant mesh proportion preset, shared clips retargeted and reviewed at full span. Do not scale the actor/capsule with the mesh. Floor `root`, hover/body, head/ears, finger/leg/tail chains; sockets `VFX_Bite`, `VFX_Hit`, `UI_Anchor`, `WingTip_L/R`. All rig anatomy beyond the sampled source view is authored reconstruction. Use the [shared timing contract](README.md#rig-and-attack-timing-contract): in-place floor root, 30 fps preview; f0 native commit, **f30 = 1.000 s impact**, visual recoil through f42. Current fixture cooldown is 3.0 s from commit, not a recovery lock. Production delay remains data-owned and must retime the contact key to `ImpactSeconds`.

| Clip/state | Required presentation |
|---|---|
| Idle | Broad controlled hover strokes that remain within the height band. |
| Locomotion | Ground-speed-driven fly/bank/start/stop; reduce wing stroke near turns visually without narrowing the declared clearance envelope. |
| Attack windup, f0–f29 | Raise the leading edges and pull the chest backward, preserving the same preview contact timing as other bats. |
| Attack impact, f30 | Single forward head/upper-body snap with membranes on the downstroke; the root does not surge forward. |
| Recoil, f31–f42 | Settle toward idle, interruptibly; one impact only. |
| Hit | Brief asymmetric wing recoil that does not strike adjacent walls outside the envelope. |
| Death + dead hold | Wings curl inward and body lowers onto the floor; settle without a ragdoll spanning a doorway. |
| Special / transitions | Hover-to-floor death required; no knockback, swoop travel or extra melee range implied by size. |

Contact is a visual key, never a damage notify. Commit/cancel animation hooks are a pending integration seam; native impact/death results drive feedback. A failed range/LOS validation may emit no `OnImpact`, so clear the cosmetic attack through its lifecycle, not by waiting indefinitely for a hit event. Death interrupts presentation; loot and corpse collision/removal belong to the lifecycle owner.

## Placeholder and replacement

Dark brown ellipsoid chest at Z=130 cm, broad inner wing cubes and tapered cone/cube tips spanning 170 cm, with thin ochre cylinders along the leading edges. Maintain readable negative space between wing halves. These are proposed engine-shape assemblies, not assets created here. Disable collision on the visual pieces; retain the reviewed capsule and stable entity/life identity. Set the floor root under the capsule as described in the README.

W4-02 maps `Enemy.GiantBat` → `Presentation.Enemy.GiantBat` → the proxy, then W5-03 replaces that binding with the approved **Bat** mesh, materials and animation set. See the [unimplemented resolver boundary](README.md#presentation-ids-and-replacement-seam). Do not edit HP, timing, reach, rewards or capsule as part of an art-only swap. Review at the 900/1200/1800 cm camera distances, in silhouette/grayscale and beside relevant family variants, before accepting the replacement.

## Provenance and rights

Actually opened: `P/assets/references/monsters/bat-family.jpg` and `P/assets/concepts/starter-characters-and-creatures-concept.png`, where [P is the read-only package root](README.md#images-actually-opened-and-rights). Original stable ID `bat-family`; [source page](https://t4cfantasy.com/Bible/Classic/MonsterImages.php), [exact image](https://t4cfantasy.com/images/skins/app20002.jpg), retrieved **2026-10-07**; Fantasy Classic hosting, original artist/client patch unspecified. Concept generated **2026-10-07**, inputs recorded in `P/assets/concepts/prompts.md`; it supplies direction, not dimensions/animation or permission.

**No commercial licence or distribution permission is established** for either source or concept. Do not cook their pixels as this creature. New anatomy, variants, materials and rig are explicit design proposals; production original/licensed assets require their own provenance and rights record. No mesh, import, animation, editor, navigation, cook, package or play result is delivered by this document.
