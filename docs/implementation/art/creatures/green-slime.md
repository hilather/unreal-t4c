# Green Slime — presentation spec

## Identity and evidence

Gameplay definition: `Enemy.GreenSlime`. Proposed presentation ID: `Presentation.Enemy.GreenSlime`. Visual family: **Slime**. Selected floor placement: **B1–B4**, inherited from the [world ledger](../../world-ledger.md#eleven-definition-floor-coverage).

Only slime definition in this slice. Broad pooled green form distinguishes it from low Rat; it has no tail, rigid limbs or mandatory face. B1–B4 share one presentation record, with lighting variation supplied by the scene rather than new enemy identities.

Inherited Bible context: **level 2; HP 41; classic melee 4–7; live melee 3–7**, with **disputed** melee damage. Exact sources: [classic monster page](https://web.archive.org/web/20020202144834/http://www.t4cbible.com:80/monster.html) and [live Monster](https://www.t4cbible.com/Monster); retrieval **2026-10-07**, inherited R-02 capture. Historical monster-page server/patch is unspecified; live page states no game version. These are documentary statements, not verified-in-play damage outcomes or tuning installed by this spec. See [Bible roster](../../ruleset-bible-v1.md#monster-data-w4-02-not-a-w1-02-combat-coefficient) for unresolved reward semantics. Size/equipment must not alter these mechanics.

## Silhouette and reading

**Observed, 2026-10-08:** The original is a low irregular green pool with raised lobes, bright green highlights and very dark hollows. It has no clearly established eyes or mouth. The starter concept makes it much taller, glossy and partly translucent, with embedded bubbles; those are optional material/shape interpretations, not evidence of a creature face or measured volume.

**Prototype production direction:** A broad soft skirt and off-center mound must survive in silhouette. Keep an uneven but simple perimeter and one large highlight zone; tiny bubbles must not replace the large mass. Avoid a tall cylindrical jelly body that loses the original pooled read.

## Scale and collision

All dimensions below are **Prototype**, A-01 / LH_Prototype_v1 / 2026-10-08, source URL null; inherited W0-04 values are identified. Units, root placement and [clearance method](README.md#units-root-and-clearance) apply.

| Measurement | cm |
|---|---:|
| Visible resting top above floor | 40 |
| Maximum live animated top above floor | 60 |
| Full fore–aft length, including appendages/equipment in transit | 90 |
| Full lateral span in transit | 90 |
| Root-to-front / root-to-rear transit bounds (+X / −X magnitudes) | 45 / 45 |
| Upright capsule radius R | 40 |
| Upright capsule half-height HH | 40 |
| Capsule diameter × full height | 80 × 80 |
| Conservative root-centered transit turn diameter, rounded up | 128 |

90 cm footprint/40 cm resting peak inherits W0-04. Windup/death squash stays within the same footprint, with the peak allowed up to 60 cm. The capsule is 80 cm tall, above the visible peak; validate low-wall targeting and aim height before adoption.

**Arithmetic check against ordinary route minima:** max(visual width, capsule diameter) = 90 cm, leaving 150 cm total width at a 240 cm door, 230 cm in a 320 cm corridor and 210 cm on a 300 cm stair. max(animated top, capsule height) = 80 cm, leaving 220 cm under the 300 cm door/headroom requirement. The 128 cm root-centered transit turn circle is below a 320 cm landing side by 192 cm. Margins are total space, not guaranteed space per side or room for another actor.

No intended ordinary route is flagged as dimensionally too small at these bounds. Turn on the landing, not inside the door; maintain the envelope during attack/hit/death, and review actual swept limbs on stairs. This is a paper comparison only: collision, selection, slope/step contact, moving player overlap and pathfinding remain untested.

## Materials and palette

Base moss green `#3F722E`, raised lobes leaf green `#709B40`, hollows deep olive `#263E23`. Use an opaque material with roughness variation and restrained specular; test a subsurface approximation only after baseline profiling. No refractive shell, emissive acid or damage puddle implied. All hex swatches and material responses are A-01 proposals consistent with [shared art direction](README.md#camera-and-silhouette-rules), not sampled source colors. Budget: **Slime** row in the [shared budget table](README.md#proposed-production-budgets).

## Rig and animation coverage

Floor `root`, body center and a few perimeter/peak deform bones or controlled morphs. Preserve skirt contact without animating the root or collision. Sockets: `VFX_Strike` on the forward lobe, `VFX_Hit` near center mass, `UI_Anchor` above peak. No jaw/eye rig needed. All rig anatomy beyond the sampled source view is authored reconstruction. Use the [shared timing contract](README.md#rig-and-attack-timing-contract): in-place floor root, 30 fps preview; f0 native commit, **f30 = 1.000 s impact**, visual recoil through f42. Current fixture cooldown is 3.0 s from commit, not a recovery lock. Production delay remains data-owned and must retime the contact key to `ImpactSeconds`.

| Clip/state | Required presentation |
|---|---|
| Idle | Slow asymmetric mound breathing with skirt grounded and footprint stable. |
| Locomotion | Forward rolling/squash illusion driven by ground speed; no actual rolling capsule and no trail that claims damage. |
| Attack windup, f0–f29 | Pull front lobe back while raising the rear peak, exposing a clear forward direction. |
| Attack impact, f30 | One forward lobe snap reaches its widest forward pose at f30 within the 90 cm footprint. |
| Recoil, f31–f42 | Settle toward idle, interruptibly; one impact only. |
| Hit | One damped squash ripple; do not spawn extra lobes or change collision. |
| Death + dead hold | Collapse the peak into a low still puddle, same footprint; dead hold is inert and nonhazardous. Dissolve only when lifecycle authorizes removal. |
| Special / transitions | Deformation transitions required; splitting, acid spray and persistent hazards are outside this spec. |

Contact is a visual key, never a damage notify. Commit/cancel animation hooks are a pending integration seam; native impact/death results drive feedback. A failed range/LOS validation may emit no `OnImpact`, so clear the cosmetic attack through its lifecycle, not by waiting indefinitely for a hit event. Death interrupts presentation; loot and corpse collision/removal belong to the lifecycle owner.

## Placeholder and replacement

Three overlapping flattened green spheres with uneven heights under a 90 cm footprint; one larger off-center mound. Scale only visual child primitives for squash, keeping floor contact and collision unchanged. These are proposed engine-shape assemblies, not assets created here. Disable collision on the visual pieces; retain the reviewed capsule and stable entity/life identity. Set the floor root under the capsule as described in the README.

W4-02 maps `Enemy.GreenSlime` → `Presentation.Enemy.GreenSlime` → the proxy, then W5-03 replaces that binding with the approved **Slime** mesh, materials and animation set. See the [unimplemented resolver boundary](README.md#presentation-ids-and-replacement-seam). Do not edit HP, timing, reach, rewards or capsule as part of an art-only swap. Review at the 900/1200/1800 cm camera distances, in silhouette/grayscale and beside relevant family variants, before accepting the replacement.

## Provenance and rights

Actually opened: `P/assets/references/monsters/green-slime-family.jpg` and `P/assets/concepts/starter-characters-and-creatures-concept.png`, where [P is the read-only package root](README.md#images-actually-opened-and-rights). Original stable ID `green-slime-family`; [source page](https://t4cfantasy.com/Bible/Classic/MonsterImages.php), [exact image](https://t4cfantasy.com/images/skins/app20005.jpg), retrieved **2026-10-07**; Fantasy Classic hosting, original artist/client patch unspecified. Concept generated **2026-10-07**, inputs recorded in `P/assets/concepts/prompts.md`; it supplies direction, not dimensions/animation or permission.

**No commercial licence or distribution permission is established** for either source or concept. Do not cook their pixels as this creature. New anatomy, variants, materials and rig are explicit design proposals; production original/licensed assets require their own provenance and rights record. No mesh, import, animation, editor, navigation, cook, package or play result is delivered by this document.
