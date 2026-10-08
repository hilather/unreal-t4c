# Giant Spider — presentation spec

## Identity and evidence

Gameplay definition: `Enemy.GiantSpider`. Proposed presentation ID: `Presentation.Enemy.GiantSpider`. Visual family: **Spider**. Selected floor placement: **B2**, inherited from the [world ledger](../../world-ledger.md#eleven-definition-floor-coverage).

The only spider definition in this roster. Preserve Giant Spider identity; do not create a tarantula/Gustave variant or poison/web mechanics from the dungeon theme. Eight articulated legs and a low separate abdomen are the family cues.

Inherited Bible context: **level 4; HP 69; classic melee 4–10; live melee 4–10**, with agreement in the retained classic/live rows. Exact sources: [classic monster page](https://web.archive.org/web/20020202144834/http://www.t4cbible.com:80/monster.html) and [live Monster](https://www.t4cbible.com/Monster); retrieval **2026-10-07**, inherited R-02 capture. Historical monster-page server/patch is unspecified; live page states no game version. These are documentary statements, not verified-in-play damage outcomes or tuning installed by this spec. See [Bible roster](../../ruleset-bible-v1.md#monster-data-w4-02-not-a-w1-02-combat-coefficient) for unresolved reward semantics. Size/equipment must not alter these mechanics.

## Silhouette and reading

**Observed, 2026-10-08:** The original shows a low gray abdomen/body with thin angular legs radiating outward and a small red detail at the front. Several legs overlap; exact joints and anatomy are not all recoverable. The deep concept visibly adds heavy hairy joints, ridged carapace, multiple red eyes and large fangs. Those eye count/hair/fang details are interpretation. Model and audit eight complete legs explicitly rather than tracing ambiguous overlaps.

**Prototype production direction:** Keep a clear gap between the raised abdomen and front body, with eight distinct leg arcs in the top-oblique view. Long thin feet must survive LOD simplification. Give the face a small restrained rust-red accent; glowing eyes are not required to identify the species.

## Scale and collision

All dimensions below are **Prototype**, A-01 / LH_Prototype_v1 / 2026-10-08, source URL null; inherited W0-04 values are identified. Units, root placement and [clearance method](README.md#units-root-and-clearance) apply.

| Measurement | cm |
|---|---:|
| Visible resting top above floor | 65 |
| Maximum animated top above floor | 90 |
| Full fore–aft length, including appendages/equipment in transit | 150 |
| Full lateral span in transit | 170 |
| Upright capsule radius R | 55 |
| Upright capsule half-height HH | 55 |
| Capsule diameter × full height | 110 × 110 |
| Conservative transit turn diameter, rounded up | 227 |

170 cm leg-tip span and 65 cm resting top inherit W0-04. The forward fang pose may raise the front to 90 cm but cannot spread legs beyond 170 × 150 cm. Capsule diameter 110 cm excludes outer legs, and capsule top 110 cm exceeds visible body; review targeting and low lintels accordingly.

**Arithmetic check against ordinary route minima:** max(visual width, capsule diameter) = 170 cm, leaving 70 cm total width at a 240 cm door, 150 cm in a 320 cm corridor and 130 cm on a 300 cm stair. max(animated top, capsule height) = 110 cm, leaving 190 cm under the 300 cm door/headroom requirement. The 227 cm transit turn circle is below a 320 cm landing side by 93 cm. Margins are total space, not guaranteed space per side or room for another actor.

No intended ordinary route is flagged as dimensionally too small at these bounds. Turn on the landing, not inside the door; maintain the envelope during attack/hit/death, and review actual swept limbs on stairs. This is a paper comparison only: collision, selection, slope/step contact, moving player overlap and pathfinding remain untested.

## Materials and palette

Carapace gray-brown `#66645E`, leg joints charcoal `#3D3D39`, sparse face rust `#8F493B`, worn ridges `#959083`. Mostly rough opaque chitin with a few broad worn highlights; silhouette geometry carries legs instead of hair-card density. All hex swatches and material responses are A-01 proposals consistent with [shared art direction](README.md#camera-and-silhouette-rules), not sampled source colors. Budget: **Spider** row in the [shared budget table](README.md#proposed-production-budgets).

## Rig and animation coverage

Dedicated arthropod rig: floor `root`, body/abdomen, head/fang controls and four paired three-segment leg chains, with terminal contact controls as needed within budget. Confirm eight legs at bind pose and every LOD. Sockets: `VFX_Bite` between fangs, `VFX_Hit` on carapace, `UI_Anchor` above abdomen; leg-end debug sockets help clearance inspection. All rig anatomy beyond the sampled source view is authored reconstruction. Use the [shared timing contract](README.md#rig-and-attack-timing-contract): in-place floor root, 30 fps preview; f0 native commit, **f30 = 1.000 s impact**, visual recoil through f42. Current fixture cooldown is 3.0 s from commit, not a recovery lock. Production delay remains data-owned and must retime the contact key to `ImpactSeconds`.

| Clip/state | Required presentation |
|---|---|
| Idle | Small body rise/fall with planted leg tips; avoid a constant busy eight-leg shuffle. |
| Locomotion | Alternating leg-group walk/skitter with grounded contacts, plus turns/start/stop; foot IK is cosmetic and cannot pull the capsule through a wall. |
| Attack windup, f0–f29 | Plant rear legs, lift front body and retract fangs; keep outer foot contacts inside the footprint. |
| Attack impact, f30 | Single forward fang jab at f30; front legs frame the mouth rather than becoming additional attacks. |
| Recoil, f31–f42 | Settle toward idle, interruptibly; one impact only. |
| Hit | Brief abdomen recoil and leg compression without rolling the root. |
| Death + dead hold | Lower body and curl legs inward into the footprint; dead hold remains identifiable as a spider without physics limbs blocking a stair. |
| Special / transitions | No web shot, wall climb, ceiling walk, egg spawn or poison effect. Ground contact adjustment on ramps is presentation work only. |

Contact is a visual key, never a damage notify. Commit/cancel animation hooks are a pending integration seam; native impact/death results drive feedback. A failed range/LOS validation may emit no `OnImpact`, so clear the cosmetic attack through its lifecycle, not by waiting indefinitely for a hit event. Death interrupts presentation; loot and corpse collision/removal belong to the lifecycle owner.

## Placeholder and replacement

Two gray flattened spheres for abdomen/front body and eight bent pairs of thin cylinders in a 170 × 150 cm footprint, with a small rust-red front sphere. Preserve separate leg silhouettes even in the proxy. These are proposed engine-shape assemblies, not assets created here. Disable collision on the visual pieces; retain the reviewed capsule and stable entity/life identity. Set the floor root under the capsule as described in the README.

W4-02 maps `Enemy.GiantSpider` → `Presentation.Enemy.GiantSpider` → the proxy, then W5-03 replaces that binding with the approved **Spider** mesh, materials and animation set. See the [unimplemented resolver boundary](README.md#presentation-ids-and-replacement-seam). Do not edit HP, timing, reach, rewards or capsule as part of an art-only swap. Review at the 900/1200/1800 cm camera distances, in silhouette/grayscale and beside relevant family variants, before accepting the replacement.

## Provenance and rights

Actually opened: `P/assets/references/monsters/giant-spider-family.jpg` and `P/assets/concepts/deep-dungeon-creatures-concept.png`, where [P is the read-only package root](README.md#images-actually-opened-and-rights). Original stable ID `giant-spider-family`; [source page](https://t4cfantasy.com/Bible/Classic/MonsterImages.php), [exact image](https://t4cfantasy.com/images/skins/app20007.jpg), retrieved **2026-10-07**; Fantasy Classic hosting, original artist/client patch unspecified. Concept generated **2026-10-07**, inputs recorded in `P/assets/concepts/prompts.md`; it supplies direction, not dimensions/animation or permission.

**No commercial licence or distribution permission is established** for either source or concept. Do not cook their pixels as this creature. New anatomy, variants, materials and rig are explicit design proposals; production original/licensed assets require their own provenance and rights record. No mesh, import, animation, editor, navigation, cook, package or play result is delivered by this document.
