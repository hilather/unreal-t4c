# Bat — presentation spec

## Identity and evidence

Gameplay definition: `Enemy.Bat`. Proposed presentation ID: `Presentation.Enemy.Bat`. Visual family: **Bat**. Selected floor placement: **B1–B2**, inherited from the [world ledger](../../world-ledger.md#eleven-definition-floor-coverage).

Smallest of the four bat presentations. Use plain warm membranes, sharp tips and an unbroken scalloped trailing edge. Dungeon Bat has blunt tips and a pale dorsal chevron; Undead Bat has notches and pale outer panels; Giant Bat has much broader shoulders and a 170 cm span. These distinctions are authored, not variant sprites.

Inherited Bible context: **level 1; HP 27; classic melee 4–5; live melee 2–5**, with **disputed** melee damage. Exact sources: [classic monster page](https://web.archive.org/web/20020202144834/http://www.t4cbible.com:80/monster.html) and [live Monster](https://www.t4cbible.com/Monster); retrieval **2026-10-07**, inherited R-02 capture. Historical monster-page server/patch is unspecified; live page states no game version. These are documentary statements, not verified-in-play damage outcomes or tuning installed by this spec. See [Bible roster](../../ruleset-bible-v1.md#monster-data-w4-02-not-a-w1-02-combat-coefficient) for unresolved reward semantics. Size/equipment must not alter these mechanics.

## Silhouette and reading

**Observed, 2026-10-08:** The original shows a tiny brown/gray torso between unevenly extended angular membranous wings; the upper-right wing is raised and finger-like ribs interrupt the edge. It supplies one sampled pose, not a flight cycle or four variants. The starter concept adds a furry torso, very large ears, visible wing fingers and tan membrane veins. Its pose is asymmetrical and does not establish bone placement.

**Prototype production direction:** Keep the central body small against a simple wing pair. Plain, clean trailing edges are this variant’s shape cue; use a mid-value tan membrane instead of relying on tiny ears. Preserve open gaps between wings and torso during the recovery pose.

## Scale and collision

All dimensions below are **Prototype**, A-01 / LH_Prototype_v1 / 2026-10-08, source URL null; inherited W0-04 values are identified. Units, root placement and [clearance method](README.md#units-root-and-clearance) apply.

| Measurement | cm |
|---|---:|
| Visible resting top above floor | 145 |
| Maximum animated top above floor | 145 |
| Full fore–aft length, including appendages/equipment in transit | 45 |
| Full lateral span in transit | 80 |
| Upright capsule radius R | 25 |
| Upright capsule half-height HH | 75 |
| Capsule diameter × full height | 50 × 150 |
| Conservative transit turn diameter, rounded up | 92 |

The 80 cm wingspan is inherited W0-04 tuning. Torso center hovers around Z=100 cm; animate membrane tips within Z=55–145 cm. Full fore–aft length includes the small tail. Capsule remains grounded; visual hover must not authorize true flight or crossing voids.

**Arithmetic check against ordinary route minima:** max(visual width, capsule diameter) = 80 cm, leaving 160 cm total width at a 240 cm door, 240 cm in a 320 cm corridor and 220 cm on a 300 cm stair. max(animated top, capsule height) = 150 cm, leaving 150 cm under the 300 cm door/headroom requirement. The 92 cm transit turn circle is below a 320 cm landing side by 228 cm. Margins are total space, not guaranteed space per side or room for another actor.

No intended ordinary route is flagged as dimensionally too small at these bounds. Turn on the landing, not inside the door; maintain the envelope during attack/hit/death, and review actual swept limbs on stairs. This is a paper comparison only: collision, selection, slope/step contact, moving player overlap and pathfinding remain untested.

## Materials and palette

Membrane warm tan `#927451`, fur brown `#514236`, ribs dull beige `#AF9169`. Opaque leathery membrane, rough fur body, one shared bat material with upper/lower value separation. No glow. All hex swatches and material responses are A-01 proposals consistent with [shared art direction](README.md#camera-and-silhouette-rules), not sampled source colors. Budget: **Bat** row in the [shared budget table](README.md#proposed-production-budgets).

## Rig and animation coverage

Shared bat rig: floor `root`, hover/body child, head/ears, paired shoulder/elbow/wrist/finger chains, rear feet and tail. Membrane weights must preserve the scallops; check both sides. Sockets: `VFX_Bite` at mouth, `VFX_Hit` on torso, `UI_Anchor` above torso, `WingTip_L/R` for envelope inspection only. All rig anatomy beyond the sampled source view is authored reconstruction. Use the [shared timing contract](README.md#rig-and-attack-timing-contract): in-place floor root, 30 fps preview; f0 native commit, **f30 = 1.000 s impact**, visual recoil through f42. Current fixture cooldown is 3.0 s from commit, not a recovery lock. Production delay remains data-owned and must retime the contact key to `ImpactSeconds`.

| Clip/state | Required presentation |
|---|---|
| Idle | Gentle hover flap with body bob staying in the prescribed height band. |
| Locomotion | Forward hover/fly loop and turn/bank poses driven by ground movement speed; no vertical navigation or per-flap collision resize. |
| Attack windup, f0–f29 | Raise both wings and pull the head back within the band; a brief symmetric hold makes the impending snap readable. |
| Attack impact, f30 | Forward head snap with the wings at the lower stroke, centered on the body’s aim direction. |
| Recoil, f31–f42 | Settle toward idle, interruptibly; one impact only. |
| Hit | Small sideways bank without displacing the ground root. |
| Death + dead hold | Stop powered flap, tuck and lower the visual body to the floor, then hold a compact winged corpse. The actor/corpse lifecycle remains authoritative. |
| Special / transitions | Transition from hover to grounded dead hold is required; no perch or ceiling attachment gameplay. |

Contact is a visual key, never a damage notify. Commit/cancel animation hooks are a pending integration seam; native impact/death results drive feedback. A failed range/LOS validation may emit no `OnImpact`, so clear the cosmetic attack through its lifecycle, not by waiting indefinitely for a hit event. Death interrupts presentation; loot and corpse collision/removal belong to the lifecycle owner.

## Placeholder and replacement

Brown sphere torso suspended at Z=100 cm, two thin tan flattened cubes angled into a shallow wing V across 80 cm, and two small ear cones. Animate only child pieces; source JPEG is not a billboard texture. These are proposed engine-shape assemblies, not assets created here. Disable collision on the visual pieces; retain the reviewed capsule and stable entity/life identity. Set the floor root under the capsule as described in the README.

W4-02 maps `Enemy.Bat` → `Presentation.Enemy.Bat` → the proxy, then W5-03 replaces that binding with the approved **Bat** mesh, materials and animation set. See the [unimplemented resolver boundary](README.md#presentation-ids-and-replacement-seam). Do not edit HP, timing, reach, rewards or capsule as part of an art-only swap. Review at the 900/1200/1800 cm camera distances, in silhouette/grayscale and beside relevant family variants, before accepting the replacement.

## Provenance and rights

Actually opened: `P/assets/references/monsters/bat-family.jpg` and `P/assets/concepts/starter-characters-and-creatures-concept.png`, where [P is the read-only package root](README.md#images-actually-opened-and-rights). Original stable ID `bat-family`; [source page](https://t4cfantasy.com/Bible/Classic/MonsterImages.php), [exact image](https://t4cfantasy.com/images/skins/app20002.jpg), retrieved **2026-10-07**; Fantasy Classic hosting, original artist/client patch unspecified. Concept generated **2026-10-07**, inputs recorded in `P/assets/concepts/prompts.md`; it supplies direction, not dimensions/animation or permission.

**No commercial licence or distribution permission is established** for either source or concept. Do not cook their pixels as this creature. New anatomy, variants, materials and rig are explicit design proposals; production original/licensed assets require their own provenance and rights record. No mesh, import, animation, editor, navigation, cook, package or play result is delivered by this document.
