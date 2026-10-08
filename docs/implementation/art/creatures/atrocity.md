# Atrocity — presentation spec

## Identity and evidence

Gameplay definition: `Enemy.Atrocity`. Proposed presentation ID: `Presentation.Enemy.Atrocity`. Visual family: **Atrocity**. Selected floor placement: **B3–B4**, inherited from the [world ledger](../../world-ledger.md#eleven-definition-floor-coverage).

Unique dark, forward-hunched long-armed creature with conspicuous yellow claws. It is neither a generic zombie/skeleton nor a goblin variant. Keep a muscular quadruped-like forward weight with two supporting legs and long grasping arms; the concept is anatomical direction, not evidence of a specific species taxonomy.

Inherited Bible context: **level 5; HP 84; classic melee 5–12; live melee 5–12**, with agreement in the retained classic/live rows. Exact sources: [classic monster page](https://web.archive.org/web/20020202144834/http://www.t4cbible.com:80/monster.html) and [live Monster](https://www.t4cbible.com/Monster); retrieval **2026-10-07**, inherited R-02 capture. Historical monster-page server/patch is unspecified; live page states no game version. These are documentary statements, not verified-in-play damage outcomes or tuning installed by this spec. See [Bible roster](../../ruleset-bible-v1.md#monster-data-w4-02-not-a-w1-02-combat-coefficient) for unresolved reward semantics. Size/equipment must not alter these mechanics.

## Silhouette and reading

**Observed, 2026-10-08:** The original has a dark hunched body, long forward arms, yellow claw tips on hands/feet and angular yellowish projections at the head/back. Its rear and spike count are unclear. The deep concept emphasizes a low narrow head, massive shoulders, many yellow spines and curved elongated claws. Preserve the mass/contrast, but simplify the extra concept spines instead of treating every one as original anatomy.

**Prototype production direction:** Broad arched shoulders above a low head, long separated forearms, and two pale claw fans should read first. Sparse large dorsal projections preserve the jagged back. Avoid a straight human posture or a thicket of tiny spikes that disappears at gameplay distance.

## Scale and collision

All dimensions below are **Prototype**, A-01 / LH_Prototype_v1 / 2026-10-08, source URL null; inherited W0-04 values are identified. Units, root placement and [clearance method](README.md#units-root-and-clearance) apply.

| Measurement | cm |
|---|---:|
| Visible resting top above floor | 190 |
| Maximum animated top above floor | 210 |
| Full fore–aft length, including appendages/equipment in transit | 160 |
| Full lateral span in transit | 180 |
| Upright capsule radius R | 65 |
| Upright capsule half-height HH | 100 |
| Capsule diameter × full height | 130 × 200 |
| Conservative transit turn diameter, rounded up | 241 |

190 cm hunched top and 180 cm arm/claw span inherit W0-04. Windup shoulders may reach 210 cm. Forearms/claws stay within 180 × 160 cm transit/compact attack footprint. The conservative 241 cm turn diameter is slightly larger than an ordinary 240 cm door: cross aligned, turn only on the 320 cm pad.

**Arithmetic check against ordinary route minima:** max(visual width, capsule diameter) = 180 cm, leaving 60 cm total width at a 240 cm door, 140 cm in a 320 cm corridor and 120 cm on a 300 cm stair. max(animated top, capsule height) = 210 cm, leaving 90 cm under the 300 cm door/headroom requirement. The 241 cm transit turn circle is below a 320 cm landing side by 79 cm. Margins are total space, not guaranteed space per side or room for another actor.

No intended ordinary route is flagged as dimensionally too small at these bounds. Turn on the landing, not inside the door; maintain the envelope during attack/hit/death, and review actual swept limbs on stairs. This is a paper comparison only: collision, selection, slope/step contact, moving player overlap and pathfinding remain untested.

## Materials and palette

Skin charcoal-purple `#343038`, raised muscles muted gray-violet `#58505A`, claws/spines old horn ochre `#BFA44D`. Use broad rough skin highlights so the torso does not vanish into recesses; yellow claws are keratin-like, not emissive gold or poison. All hex swatches and material responses are A-01 proposals consistent with [shared art direction](README.md#camera-and-silhouette-rules), not sampled source colors. Budget: **Atrocity** row in the [shared budget table](README.md#proposed-production-budgets).

## Rig and animation coverage

Dedicated hunched biped skeleton with strong spine/scapula controls, long upper/lower arms and separately poseable claw fans; legs support the low forward lean. Never force a straight player retarget that erases its silhouette. Floor `root`; sockets `VFX_Claw_L/R`, `VFX_Hit` on chest, `UI_Anchor` above shoulder hump. Spike secondary motion is optional and bounded. All rig anatomy beyond the sampled source view is authored reconstruction. Use the [shared timing contract](README.md#rig-and-attack-timing-contract): in-place floor root, 30 fps preview; f0 native commit, **f30 = 1.000 s impact**, visual recoil through f42. Current fixture cooldown is 3.0 s from commit, not a recovery lock. Production delay remains data-owned and must retime the contact key to `ImpactSeconds`.

| Clip/state | Required presentation |
|---|---|
| Idle | Heavy upper-back breathing and slight finger curl, feet planted. |
| Locomotion | Hunched loping walk/run with long arm counter-swing bounded by 180 cm; blend turns/start/stop without unfolding into a human pose. |
| Attack windup, f0–f29 | Pull one long forearm back and raise the shoulder; retain hunched head below shoulder mass. |
| Attack impact, f30 | Single short forward/downward claw rake at f30, other hand braced; several fingers still represent one authoritative hit. |
| Recoil, f31–f42 | Settle toward idle, interruptibly; one impact only. |
| Hit | Asymmetric shoulder flinch, with claw tips kept in the envelope. |
| Death + dead hold | Fold knees/arms inward and settle chest-first on the floor; claws gather inward into the footprint and dead hold stays low. |
| Special / transitions | No leap, grab, disease, stun or multi-claw damage series. Optional finger/spine settle is cosmetic. |

Contact is a visual key, never a damage notify. Commit/cancel animation hooks are a pending integration seam; native impact/death results drive feedback. A failed range/LOS validation may emit no `OnImpact`, so clear the cosmetic attack through its lifecycle, not by waiting indefinitely for a hit event. Death interrupts presentation; loot and corpse collision/removal belong to the lifecycle owner.

## Placeholder and replacement

Dark overlapping shoulder/torso spheres, low head cone/sphere, two long bent cylinder arms and short supporting legs, with sparse yellow cones for claw fans and back projections. Do not start from a straight upright zombie silhouette. These are proposed engine-shape assemblies, not assets created here. Disable collision on the visual pieces; retain the reviewed capsule and stable entity/life identity. Set the floor root under the capsule as described in the README.

W4-02 maps `Enemy.Atrocity` → `Presentation.Enemy.Atrocity` → the proxy, then W5-03 replaces that binding with the approved **Atrocity** mesh, materials and animation set. See the [unimplemented resolver boundary](README.md#presentation-ids-and-replacement-seam). Do not edit HP, timing, reach, rewards or capsule as part of an art-only swap. Review at the 900/1200/1800 cm camera distances, in silhouette/grayscale and beside relevant family variants, before accepting the replacement.

## Provenance and rights

Actually opened: `P/assets/references/monsters/atrocity-family.jpg` and `P/assets/concepts/deep-dungeon-creatures-concept.png`, where [P is the read-only package root](README.md#images-actually-opened-and-rights). Original stable ID `atrocity-family`; [source page](https://t4cfantasy.com/Bible/Classic/MonsterImages.php), [exact image](https://t4cfantasy.com/images/skins/app20026.jpg), retrieved **2026-10-07**; Fantasy Classic hosting, original artist/client patch unspecified. Concept generated **2026-10-07**, inputs recorded in `P/assets/concepts/prompts.md`; it supplies direction, not dimensions/animation or permission.

**No commercial licence or distribution permission is established** for either source or concept. Do not cook their pixels as this creature. New anatomy, variants, materials and rig are explicit design proposals; production original/licensed assets require their own provenance and rights record. No mesh, import, animation, editor, navigation, cook, package or play result is delivered by this document.
