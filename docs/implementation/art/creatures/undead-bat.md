# Undead Bat — presentation spec

## Identity and evidence

Gameplay definition: `Enemy.UndeadBat`. Proposed presentation ID: `Presentation.Enemy.UndeadBat`. Visual family: **Bat**. Selected floor placement: **B2 selected; secondary B3 claim remains disputed**, inherited from the [world ledger](../../world-ledger.md#eleven-definition-floor-coverage).

Separate from the other three bat definitions despite shared source art. Prototype cues: 110 cm span, a shallow notch in each trailing outer wing and pale outer membrane panels against a dark center. Keep intact membrane webs; “Undead” does not prove exposed bones. Dungeon Bat has a central chevron, Giant Bat a continuous leading-edge band, plain Bat no marking.

Inherited Bible context: **level 3; HP 55; classic melee 4–7; live melee 4–8**, with **disputed** melee damage. Exact sources: [classic monster page](https://web.archive.org/web/20020202144834/http://www.t4cbible.com:80/monster.html) and [live Monster](https://www.t4cbible.com/Monster); retrieval **2026-10-07**, inherited R-02 capture. Historical monster-page server/patch is unspecified; live page states no game version. These are documentary statements, not verified-in-play damage outcomes or tuning installed by this spec. See [Bible roster](../../ruleset-bible-v1.md#monster-data-w4-02-not-a-w1-02-combat-coefficient) for unresolved reward semantics. Size/equipment must not alter these mechanics.

## Silhouette and reading

**Observed, 2026-10-08:** The original shows brown-gray membranes and a tiny body, with unequal wing extension. It shows no bone exposure or undead-specific damage. The starter sheet gives the common bat detailed fur, ears and veining; notches and desaturated outer panels below are invented differentiation. The Bible’s Decaying Bat Wings loot name is not an anatomy diagram.

**Prototype production direction:** Paired broad pale outer patches and notched trailing silhouette must distinguish this bat even at the same distance as Dungeon Bat. Keep notch shapes large and simple, not a lacework of tiny holes that disappears or flickers at LOD2.

## Scale and collision

All dimensions below are **Prototype**, A-01 / LH_Prototype_v1 / 2026-10-08, source URL null; inherited W0-04 values are identified. Units, root placement and [clearance method](README.md#units-root-and-clearance) apply.

| Measurement | cm |
|---|---:|
| Visible resting top above floor | 165 |
| Maximum live animated top above floor | 165 |
| Full fore–aft length, including appendages/equipment in transit | 60 |
| Full lateral span in transit | 110 |
| Root-to-front / root-to-rear transit bounds (+X / −X magnitudes) | 30 / 30 |
| Upright capsule radius R | 30 |
| Upright capsule half-height HH | 85 |
| Capsule diameter × full height | 60 × 170 |
| Conservative root-centered transit turn diameter, rounded up | 126 |

110 cm wingspan inherits W0-04. Torso center around Z=115 cm; wing motion stays Z=60–165 cm. Ground-root navigation stays identical in principle to other bats, with its own reviewed capsule, and does not add ceiling or void traversal.

**Arithmetic check against ordinary route minima:** max(visual width, capsule diameter) = 110 cm, leaving 130 cm total width at a 240 cm door, 210 cm in a 320 cm corridor and 190 cm on a 300 cm stair. max(animated top, capsule height) = 170 cm, leaving 130 cm under the 300 cm door/headroom requirement. The 126 cm root-centered transit turn circle is below a 320 cm landing side by 194 cm. Margins are total space, not guaranteed space per side or room for another actor.

No intended ordinary route is flagged as dimensionally too small at these bounds. Turn on the landing, not inside the door; maintain the envelope during attack/hit/death, and review actual swept limbs on stairs. This is a paper comparison only: collision, selection, slope/step contact, moving player overlap and pathfinding remain untested.

## Materials and palette

Torso cool charcoal `#414544`, inner membrane gray brown `#64685D`, outer panels pale ash `#B6B49F`. Opaque dry leathery surface; no bone-white skeleton, exposed wound, poison glow or spectral transparency. All hex swatches and material responses are A-01 proposals consistent with [shared art direction](README.md#camera-and-silhouette-rules), not sampled source colors. Budget: **Bat** row in the [shared budget table](README.md#proposed-production-budgets).

## Rig and animation coverage

Shared bat bones and sockets: floor `root`, hover/body, head/ears, two wing finger chains, feet/tail; `VFX_Bite`, `VFX_Hit`, `UI_Anchor`, `WingTip_L/R`. Notches belong to authored silhouette topology at all LODs, not an expensive translucent cutout. All rig anatomy beyond the sampled source view is authored reconstruction. Use the [shared timing contract](README.md#rig-and-attack-timing-contract): in-place floor root, 30 fps preview; f0 native commit, **f30 = 1.000 s impact**, visual recoil through f42. Current fixture cooldown is 3.0 s from commit, not a recovery lock. Production delay remains data-owned and must retime the contact key to `ImpactSeconds`.

| Clip/state | Required presentation |
|---|---|
| Idle | Uneven-looking wing pose with stable body hover; animation style may feel brittle but cannot imply intermittent collision. |
| Locomotion | Ground-driven fly/bank with the pale panels visible from above; action duration does not change combat cooldown. |
| Attack windup, f0–f29 | Raise the notched edges, briefly hold, and retract the head; no magical charge. |
| Attack impact, f30 | One forward bite pose on the downstroke, matching the same f30 sample. |
| Recoil, f31–f42 | Settle toward idle, interruptibly; one impact only. |
| Hit | Short torso/wing recoil blended without delaying an accepted attack. |
| Death + dead hold | Lower and fold the body, leaving a pale notched wing profile in the dead hold; no automatic dissolving or loot-wing emission. |
| Special / transitions | Hover-to-floor death transition only. Wing loot appears through the reward owner, not a severing animation. |

Contact is a visual key, never a damage notify. Commit/cancel animation hooks are a pending integration seam; native impact/death results drive feedback. A failed range/LOS validation may emit no `OnImpact`, so clear the cosmetic attack through its lifecycle, not by waiting indefinitely for a hit event. Death interrupts presentation; loot and corpse collision/removal belong to the lifecycle owner.

## Placeholder and replacement

Charcoal torso sphere at Z=115 cm and segmented gray wing cubes across 110 cm; outer pale cubes stop short of one segment to show a single notch per trailing edge. Do not use translucency. These are proposed engine-shape assemblies, not assets created here. Disable collision on the visual pieces; retain the reviewed capsule and stable entity/life identity. Set the floor root under the capsule as described in the README.

W4-02 maps `Enemy.UndeadBat` → `Presentation.Enemy.UndeadBat` → the proxy, then W5-03 replaces that binding with the approved **Bat** mesh, materials and animation set. See the [unimplemented resolver boundary](README.md#presentation-ids-and-replacement-seam). Do not edit HP, timing, reach, rewards or capsule as part of an art-only swap. Review at the 900/1200/1800 cm camera distances, in silhouette/grayscale and beside relevant family variants, before accepting the replacement.

## Provenance and rights

Actually opened: `P/assets/references/monsters/bat-family.jpg` and `P/assets/concepts/starter-characters-and-creatures-concept.png`, where [P is the read-only package root](README.md#images-actually-opened-and-rights). Original stable ID `bat-family`; [source page](https://t4cfantasy.com/Bible/Classic/MonsterImages.php), [exact image](https://t4cfantasy.com/images/skins/app20002.jpg), retrieved **2026-10-07**; Fantasy Classic hosting, original artist/client patch unspecified. Concept generated **2026-10-07**, inputs recorded in `P/assets/concepts/prompts.md`; it supplies direction, not dimensions/animation or permission.

**No commercial licence or distribution permission is established** for either source or concept. Do not cook their pixels as this creature. New anatomy, variants, materials and rig are explicit design proposals; production original/licensed assets require their own provenance and rights record. No mesh, import, animation, editor, navigation, cook, package or play result is delivered by this document.
