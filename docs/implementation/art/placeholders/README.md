# A-03 — Primitive presentation catalogue

First-pass specification, 2026-10-08; base `fe01c733e82412edf28de7ac620cedfd40680d79`, contract revision 1. **Proposed visual assemblies, not implemented actors, assets or a gameplay-readability pass.** Approved A-01/A-02 direction is inherited from the brief. Every design number here (cm, seconds, scale, colour, projection and material setting) is **Prototype**, profile `LH_Prototype_v1`, author A-03, date 2026-10-08, `source_url: null`; A-01/A-02/V-03/controls dimensions and tokens remain inherited Prototype. No authentic mechanic, price, HP, XP, loot weight or respawn interval is introduced.

The deliverable is this catalogue and original text SVG sheets: [creatures](lineup-creatures.svg), [appearance and services](lineup-services.svg), [interactables](lineup-interactables.svg), and [far-view comparisons](lineup-far.svg). SVGs contain geometry and text only; they are analytic camera studies, not Unreal screenshots. No external reference pixels, fonts, binaries, new code or content assets are included.

## Sources and decisions

Read together with [A-01 and its eleven creature specs](../creatures/README.md), [Balork encounter companion](../creatures/balork-final-encounter.md), [A-02 player](../player/README.md), [human motion](../player/animation.md), [environment kit](../environment/README.md), [exact modules](../environment/modules.md), [V-03 tokens](../../ui/style/README.md), [layout and travel graph](../../layout/README.md), [world ledger](../../world-ledger.md), [dev maps](../../dev-maps.md), [combat foundation](../../combat-foundation.md), and [D13/D14](../../schema-rev1-freeze.md#d13--presentation-registry-and-appearance-selections). Local evidence is sufficient: this task invents no new historical mechanics and performs no new Bible retrieval. Upstream reference observations/rights records are inherited; no source image was opened or copied in A-03.

At this base, [LHDefinitions.h](../../../../Source/Lighthaven/Core/LHDefinitions.h) already contains `PresentationId` and `ULHPresentationDefinition`, superseding A-01/A-02's older statement that both are absent. The typed record has a generic soft `Visual`; these specs do not implement its resolver or populate a catalogue. [LHCombatComponent.h](../../../../Source/Lighthaven/Abilities/LHCombatComponent.h) still exposes only `OnImpact`/`OnDeath`, so D14 commit/cancel/finish synchronization remains a prerequisite. [LHCharacter.cpp](../../../../Source/Lighthaven/Framework/LHCharacter.cpp) supplies R35/HH90 and a 45° **horizontal** FOV; use this over layout's stale vertical-FOV wording. The current enemy is a combat actor with logged death and a separate cylinder ruler from the dev generator, not an existing creature model. Do not relabel that ruler as an A-03 assembly.

Coverage: ten composable player appearance IDs (twelve body/hair/skin combinations), eleven enemy IDs **including** Balork, Balork's final-encounter state on the same ID, thirteen named hub NPCs plus two B1 NPCs, and conditional interactable bindings. NPC names/roles follow the [hub anchors](../../layout/hub-temple-district.md#named-service-anchor-candidates) and [B1](../../layout/b1.md). Nevanis/Shovanis remain in B1; Iraltok/Uranos remain at the mage building. Jagar Kar has no assigned offer; Brother Kiran is a priest, not an invented healing vendor. Lothan is optional scenery and has no required service binding here.

## Assembly convention

All measurements are final **cm**, not engine mesh scale factors. Local floor-root `(0,0,0)`, +X forward, +Y right, +Z up. On a centered Character capsule attach the visual floor root at `(0,0,-HH)`; keep actor scale 1. `C` = cube dimensions X×Y×Z; `S` = sphere scaled to those full diameters; `Y` = cylinder with full diameter and length; `N` = cone with base diameter and base-to-tip length. Tables below give centers for C/S and exact base/end points for Y/N. The engine cylinder/cone's native Z axis rotates onto that segment. Endpoint length and midpoint define its transform; round only for display. A flat sphere is an ellipsoid, not a half sphere. Cube dimensions are local unless a yaw is explicitly given. No torus, custom mesh, imported icon, translucent card or texture is needed.

Recipe tables are normative; the sheet is a faceted projection of these same pieces. Material abbreviations in those tables resolve to exact sRGB hex below each recipe. Convert sRGB to linear once when setting an engine material parameter; do not treat hex channels as linear values. All pieces are opaque, roughness 0.8, metallic 0 except named exposed iron/steel pieces (metallic 1, roughness 0.65). Base emissive is 0. Marks are separate shallow cubes/spheres/cones with no coplanar surfaces. No permanent luminous creature eyes. The SVG’s thin charcoal construction strokes separate overlapping primitives only; they do not prescribe a permanent runtime outline. An optional outline is a **UI selection aid**, not a material baked onto every edge.

Every decorative child has collision, overlap generation, navigation influence and Visibility blocking disabled. W4/integrator must adopt each A-01 capsule explicitly; this document does not change the current dummy capsule. Capsules below enclose the blocking body, not wing/tail/weapon span. No extra target/hit volume follows these primitives. Interaction/door/ramp collision stays with the gameplay/map owner. Mesh swaps keep gameplay, saves, spawn/life GUIDs, capsule and mechanical hashes unchanged.

The catalogue gives both actual rest assemblies and inherited **maximum review envelopes**. A proxy may be smaller than the maximum; it must never exceed it. Mark geometry counts in the bounds. Keep cosmetic scale pulses inward (0.98→1.00); do not scale capsules, extend wings or grow props outside clearance. Ordinary minimum door 240×300, corridor320, stair300, turn pad320²; boss door500×360, corridor550, pad600², inherited A-01/V-01. Centered arithmetic is not a collision/nav/play test. Never turn Balork into an ordinary-route resident by scaling him down.

## Common facing, label and selection treatment

Each creature/NPC recipe names a physical front cue. Add a nonblocking ground pointer only when that actor is selected: a pale `N`, base diameter 12, base `(R+8,0,3)`, tip `(R+26,0,3)`, flattened vertically to 4 cm after orienting to +X. This is presentation geometry outside the body, not reach. If a floor/wing hides it, rely on the screen-space label/chevron rather than disabling depth tests through walls. Player uses the same pointer with R35. Stairs use their explicit travel arrows; doors face their approach side. Symmetric slimes/corpses retain the pointer and label.

Selected world target: `Color.FocusRing #E9BF79` plus four ground corner brackets (each leg 20 cm, width 4, height 2) around the capsule's bounding square at `±(R+12)`, Z2. A selected prop uses its stated footprint half-extents +12 instead. A corpse retains its owner’s body/selection footprint; the loot diamond never becomes the selectable volume. Draw no filled disc under the whole creature. Optional visible-surface outline is the same amber, proposed 2 physical px at 720p /3 at1080p, depth-tested. Hover uses `Color.HoverEdge #C4C4C4`, never a second selection. Screen label includes `Selected` and a chevron, so colour is supplementary. Friendly interaction focus uses this same amber; an NPC must not acquire a hostile red ring. Player self-identification uses `Color.Info #A9C4CC`, a small trailing bar and `You`, not a hostile target label. Diagnostic capsule rulers are excluded from normal play.

World nameplate anchor = floor-root `(0,0,max(rest top,2HH)+20)` for living actors; props use `(0,0,top+20)`; small corpses anchor at Z40 without badge or above the badge at Z98 when present. Text billboards on opaque `#242424` with primary `#F2F2F2`; secondary service/state `#C4C4C4`. Use proposed 18 px primary /16 px secondary at720p (27/24 at1080p). Wrap roles onto a second line; don't truncate the entity name. Show selected/hovered actor only plus interaction prompts supplied by the interaction owner, avoiding fifteen permanent town plates. Player shows `[DisplayName] · You` on request/selection; creation preview shows `Body A/B · Cropped/Tied · [skin name]` as text. Never expose a presentation ID as the normal player name. All entries inherit this exact target/focus treatment unless explicitly stated.

## State cues and event ownership

Cue timing uses **active simulation time**, pausing with gameplay. Effects never resolve hits or award loot. Timers are keyed by entity + life generation + activation/impact identity; cancellation, destruction, travel, reload and binding replacement discard old effects. Hydrate current state without replaying a death, reward, saved toast or aggro acquisition. Keep identity marks visible during all cues. NPCs do not receive hostile states; interactables use the object rows only.

| State / trigger | Exact proposed primitive-only cue | Authority boundary |
|---|---|---|
| Living idle | Base palette; body group optional inward scale 0.98→1.00→0.98 over2.0 s; floor pivot fixed. Wings, polearms and service badges remain still. Static at1.00 is the reduced-motion/default fallback. | Cosmetic, never a breathing health indicator; no false aggro. |
| Aggro acquired | On AI owner's explicit acquire event, front mark adds amber emission 0.25 for0.20 s once, then returns0; selected label adds `Hostile`. | Do not infer aggro from proximity, target selection or player button press. Missing AI event = no aggro pulse. |
| Attack committed / anticipation | After canonical commit, accent on existing front mark ramps emissive0→0.35 over delay D; selected state says `Attacking`. No flashing, whole-body tint or shape enlargement. | D=`ImpactSeconds` from that activation. D=0 skips anticipation. Missing commit hook = idle until authoritative result, never fake windup. |
| Contact | At commit+D stop the anticipation accent; one contact beat may set front mark emissive 0.5 for0.06 s **only on resolved feedback**, not a speculative hit. | Fixture D1.0 s gives sample f30 at30fps. Native impact timer owns resolution; frame rate and cue end cannot cause damage. A miss uses `Miss` text with no victim flash. |
| Nonlethal accepted hit | Victim mark/torso blends 35% toward `#F2F2F2`, emission0.35, immediate peak then linear decay to base in0.12 s. No opacity reduction, knockback, extra root motion or hit-stun. | Requires accepted hit plus correct target/life context retained at commit; attacker OnImpact alone has no target ID. One beat per impact identity. Further hits restart decay without additive brightness. |
| Attack finished/recovery | Clear `Attacking`; any accent decays to base within0.40 s; movement/new action interrupts it. | Fixture preview settles by f42=1.40 s. Cooldown3.0 s is inherited dev tuning from commit, **not** a recovery/movement lock. Production D/cooldown remain data-owned. |
| Cancelled/invalidated | Clear windup/attack text immediately; return mark to base over0.08 s; no victim/contact flash. | Range/LOS failure may publish no OnImpact; requires finish/cancel seam so the effect cannot hang. Precommit rejection never starts it. |
| Authoritative dead | Death overrides hit/aggro/attack immediately. Over0.30 s blend living colours65% toward `#333333`; hold at70% original RGB saturation, zero emission. Use the family dead transforms below, completed within0.40 s. Label `Dead` until loot owner says otherwise. | Target OnDeath can precede attacker OnImpact; ignore that later hit flash on a dead target. Visual collapse never disables a blocker or finalizes loot. Player uses `Defeated` and waits for lifecycle, no automatic enemy-loot badge. |
| Lootable | Dead assembly plus the corpse loot badge below; mark `#A9C4CC`, emission0.15 constant, label `[name] · Loot`. Chest uses its latch badge. | Only when authority reports remaining available contents; death is not proof of loot. Empty containers use `Empty` and no badge. Failed pickup/full inventory keeps the badge. No rarity colours. |
| Respawning | Before committed new life: render nothing, no countdown/beacon. After owner's valid new-life spawn, newly alive body starts emission0.20 in its own mark colour and decays to0 in0.35 s; optional inward scale 0.98→1.00 over0.35 s. | The cue does not set a respawn delay, permit in-view spawning or tick unloaded areas. Never replay on loading an already alive life. No Balork respawn in the selected campaign. |
| NPC interaction | On accepted interaction, badge emission0.15 in `#A9C4CC` for0.20 s; keep identity prop and name. | Does not signal purchase, healing, training or quest completion before their owner confirms. Presence-only NPCs show no unavailable invented offer. |
| Object pending / accepted / failed | Pending keeps a static `Working…` label; accepted travel leaves through owner, accepted loot updates badge, error shows owner's words without flashing. Doors change pose only after owner's open state. | No visual timer closes doors, travels, saves, locks an arena or rerolls contents. |

Disable the optional pulses and use static marks/text for reduced motion; no strobe or full-screen flash. The equations above set material multipliers, not light intensity in lumens. No new dynamic lights, postprocess stack or emissive gameplay fog is required. Foundation currently executes melee only; do not infer bow projectile impact or spell travel time from the melee contact beat.

Dead transforms (visual-only, capsule/lifecycle owner independent): rat and slime scale Z to0.35 about floor; spider scales all body and leg geometry in Z to0.30 about the floor, preserving XY and floor contact; bats lower the complete assembly until its lowest point is Z0, then scale Z to0.15 about floor (wings retain variant outline). Goblins scale all non-weapon pieces together in Z to0.30 about the floor and put the rigid weapon on the floor aligned +X: ordinary weapon assembly axis Z7: shaft from(−90,0,7) to(70,0,7), blade base(60,0,7) to tip(80,0,7); Warrior weapon assembly axis Z8: shaft from(−95,0,8) to(95,0,8), blade bases(68,±9,8) to tips(95,±9,8). Rotate every piece together from upright +Z into +X; ordinary blade radius7 and Warrior radius8 just touch the floor. Keep the complete ordinary/Warrior assemblies inside X±95,Y±50 / X±105,Y±55 respectively. Reserve A-01's190×100 /210×110 death boxes and do not swing a rigid shaft through a wall en route: cross-fade the old/new **opaque** poses by discrete visibility at death midpoint, no blending through geometry. Atrocity compresses the whole assembly Z to0.25. Balork compresses all non-weapon pieces (torso, limbs, wings, head, muzzle, horns and marks) together in Z to0.20 about the floor and lays the complete shaft/blade assembly across Y at X40,Z18 (shaft endpoints Y±200; blade baseY165/tipY210, diameter36), inside 440×200; blade bottom now touches Z0; keep full wing footprint clear of the return lane. Human/player floor-root scaleZ0.25 is a temporary defeated pose, not an animated fall; A-02's later200×120 death review envelope still needs testing. Corpses keep identifying marks, never pulse as living bodies.

### Balork encounter state, same presentation ID

`Enemy.Balork → Presentation.Enemy.Balork` always uses recipe E11. No twelfth Enemy, phase, second boss record or new encounter-presentation ID. Dormant/alive uses `Balork`; explicit acquisition adds `Balork · Hostile`; native commit adds `Balork · Attacking`; defeat uses `Balork · Defeated` and only authority-provided remaining loot. Persisted already-defeated state shows the owner's corpse/absence without replaying acquisition, flash, mark award or victory. The selected campaign does not respawn him. The final objective remains defeat → return to church → save, owned by quest/session systems; neither a trophy beacon nor a magic save altar is introduced. Keep spread wings 440, horns 310 and carried shaft 400 inside the200×440 footprint; only the existing B4 boss complex/C04/Hall apron are intended routes. The optional520 cm sweep from A-01 is not part of this proxy.

## Readability study and unresolved checks

Open SVGs at **100% physical pixel size**, scrolling the sheets; fitting an entire tall sheet to a window invalidates the comparison. Each cell is a translated crop of the same 1280×720 perspective camera at a target-centered test stand, not a collage with individually enlarged miniatures. Default D1200 cm, focusZ90, pitch−55°, yaw45°, horizontal FOV45°; far sheet D1800. Focal length =1280/(2 tan22.5°)=1545.0967 px; for camera-forward depth z, projection uses x'=f·right/z, y'=−f·up/z. Each crop is recentered at its floor origin only. Curved engine shapes are represented with low-sided projected hulls, flat sRGB fill and depth ordering. This tests mass/mark scale; it is not a renderer, exposure simulation, occlusion test or screenshot. 1080p equivalents multiply screen dimensions by1.5; no text is embedded into world meshes.

Default sheets show all eleven creatures, a dressed human ruler, all fifteen NPCs and each appearance change that can matter at distance. The far sheet repeats the six bat pairs, goblins, small ground shapes, selected services and broad palettes in colour and luminance grayscale. Service names are captions outside the geometry. Source Sans 3 is not installed by this task; the SVG uses a system sans-serif fallback solely for specification captions.

| Easily-confused pair(s), exhaustive within the listed families | Shape/size difference | Broad value/colour **and non-colour mark** |
|---|---|---|
| Bat / Dungeon Bat | Span80 /100; pointed tapered tips / squared tips | Warm tan / slate; plain wings / raised pale dorsal V |
| Bat / Giant Bat | Span80 /170; narrow / deep broad inner wings | Tan / dark earth; no band / continuous thick ochre leading bars |
| Bat / Undead Bat | Span80 /110; unbroken point / notched trailing outer panels | Tan / ash outer thirds; no mark / pale broken outer blocks |
| Dungeon Bat / Giant Bat | Span100 /170; squared ends / pointed deep panels | Slate / brown; center V / edge bars |
| Dungeon Bat / Undead Bat | Span100 /110 (size alone insufficient); squared / notched | Center light V / two separate pale outer panels; keep gaps geometric |
| Giant Bat / Undead Bat | Span170 /110; heavy broad / cut-out outer corners | Leading bands / pale outer blocks; high contrast position differs |
| Goblin / Goblin Warrior | Same head125; width envelopes85 /105, carried top185 /210; one blade / split blade | Bare red back / dark shoulder yoke with one broad linen back bar |
| Brown Rat / Green Slime | Rat narrow28 with long tail; slime round90 with no legs/tail | Umber back stripe / leaf-green off-center mound |
| Brown Rat / Giant Spider | Rat90×28, spider150×170 with eight separate bent legs | Umber tail / gray body with rust front; radial vs single trailing appendage |
| Green Slime / Giant Spider | Round pooled lobes / raised two-part body and radial legs | Green asymmetrical mound / gray ridge and rust front |
| Giant Bat / Giant Spider | Same170 span; bat body atZ130 with solid wings / low spider65 with leg gaps | Ochre edge bars / central gray ridge; wing mass vs eight thin legs |
| Undead Bat / Giant Spider | Hovering110 wing span / grounded170 legs | Paired pale wing blocks / central pale ridge; notches vs radial limbs |
| Goblin or Warrior / player or human NPC | Enemy head125 and tall off-center polearm / human175 with compact clothes | Red uncovered head/ears / skin-and-linen head, rounded hair; human never uses goblin ear cones |
| Atrocity / Goblin Warrior | Broad hunched180 arms / upright105 with tall narrow weapon | Charcoal with paired yellow claw fans / red with single pale yoke bar |
| Atrocity / Balork |180 vs440 width; hunched arms / symmetric spread wings plus horns | Ochre claw fans / red patterned wings and horizontal400 shaft |
| Body A / Body B | Shoulders46 /42 only; both175 and same capsule | Same outfit/skin palette deliberately; body choice labels carry identity at far view. No claim4 cm width is reliably recognizable at1800. |
| Cropped / Tied hair | Compact crown / rear18 cm knot plus short descending tail | Same dark brown; rear projection gives non-colour cue, subtle at far view |
| Any of the three skin pairs; Face A / Face B | Cosmetic tones/faces share scale and visible shape | Intentionally no artificial rank/identity symbol. Creation label and selected swatch name identify the option; not an in-world gameplay distinction. Face is bundled with body. |
| Every pair of fifteen human NPCs (105 pairs) | Shared175 ruler; unique badge signatures listed below, plus different clothing/prop masses | Each unordered pair has distinct geometry. Role labels remain mandatory, especially at far zoom; no skin tone encodes service. |
| Brother Kiran / Moonrock / Nevanis (three pairs) | Kiran broad book, Moonrock paired blocks, Nevanis long apron+plus | Linen / slate / pale; book spine / two tablets / plus, never all generic healers |
| Kilhiam / Iraltok / Uranos / Shovanis (six pairs) | Sun sphere with rays / flame cone / stepped cairn / three offset air bars | Pale amber / red / slate / tan; distinct outer contour and count, not hue-only spell symbols |
| Ortanalas / Kalastor / Murmuntag (three pairs) | Two upright training bars / stepped three-bar dodge zigzag / broad round striking plate | Linen bars / cool zigzag / ochre round plate; trainer names clarify shared Archery |
| Sigfried / Fali / Rolph (three pairs) | Tall angular bow+back quiver / round bottle+neck / square broad armour bib | Brown / cool blue / iron; bow void / bottle neck / full chest slab |
| Samaritan / Jagar Kar | Rounded side satchel+rolled top / two short separated shoulder tabs | Tan bag / slate mantle; unique bag silhouette / empty-handed tabs; no invented Jagar service |
| Selected NPC / selected player | NPC signature+role name / current appearance and `You` | Same amber interaction focus; player self bar cool, but words/props remain primary |
| Up stairs / down stairs | Same floor arrow;1 transverse bar /2 bars and literal destination label | Cool / amber; bar count plus `Up`/`Down` prevents colour-only direction |
| Chest / corpse loot | Raised box with lid seam / flattened body plus small upright diamond | Timber/iron / corpse's retained family colours+cool badge; both label contents status |
| Chest / scenery crate | Chest split lid+latch; scenery A-02 plain cube has no latch badge | No active badge on decor; `Chest · Loot/Empty` only from authority |
| Door aperture / portal / safe arrival | Portal has floor arrow and destination; door has upright split leaves only if enabled; safe arrival unmarked | No save/portal glow on SafeSpawn; words and geometry identify actual interaction |
| Dead Undead Bat / alive Undead Bat; dead slime / living slime | Grounded compressed wings / hovering body; flattened mound / upright mound | Dead desaturation/static hold plus `Dead/Loot`; species palette alone never means death |

Static acceptance is limited to source coverage, exact capsule matching, primitive bounds, SVG/XML/local-link checks and visual inspection of these drawings. Small rat details, tied hair,4 cm body width, and complex service marks may need selection labels at far zoom. No outline/label system currently guarantees that fallback. Required integration review: 720p/1080p,900/1200/1800 boom, yaw0/45/90, pitch−60/−55/−50, dark/warm floors, grayscale, crowded room, selected/unselected targets and shortened boom. Check depth occlusion and no target-through-wall cue, full return paths, corpse clutter, every bat/goblin pair, dead-vs-live shapes, late events, zero-delay hit, death-before-impact, pause, reload, full inventory and binding failure. These are future Linux checks; Windows remains deferred. No gate is passed here.

## Catalogue and geometry

The following table is the proposed import mapping. Each row is one case-canonical logical ID, one exact recipe and one **future** `/Game/` presentation record path; none of these assets exists by virtue of this document. A-01/D13 enemy/player IDs are retained. New NPC/interactable IDs and reuse of A-02 environment IDs require integrator adoption. Record paths name `ULHPresentationDefinition` assets, whose Visual can initially refer to a code-built supported assembly and later to the family mesh/bundle. Family production meshes remain `/Game/Lighthaven/Art/Creatures/<Family>/SK_<Family>` and humans `/Game/Lighthaven/Art/Player/SK_Human_A` or `_B`; changing payloads does not change IDs. Missing binding shows the same labeled family/human proxy with bounded diagnostic; unknown mandatory gameplay identity rejects. No fallback substitution to another species and no hard reference from a save to a mesh.

<!-- generated-catalogue -->
| Presentation ID | Recipe | Proposed future record (package path) |
|---|---|---|
| `Presentation.Player.Body.A` | [P-BodyA](#p-bodya) | `/Game/Lighthaven/Art/Player/DA_Presentation_Body_A` |
| `Presentation.Player.Body.B` | [P-BodyB](#p-bodyb) | `/Game/Lighthaven/Art/Player/DA_Presentation_Body_B` |
| `Presentation.Player.Face.A` | [P-FaceA](#p-facea) | `/Game/Lighthaven/Art/Player/DA_Presentation_Face_A` |
| `Presentation.Player.Face.B` | [P-FaceB](#p-faceb) | `/Game/Lighthaven/Art/Player/DA_Presentation_Face_B` |
| `Presentation.Player.Hair.Cropped` | [P-HairCropped](#p-haircropped) | `/Game/Lighthaven/Art/Player/DA_Presentation_Hair_Cropped` |
| `Presentation.Player.Hair.Tied` | [P-HairTied](#p-hairtied) | `/Game/Lighthaven/Art/Player/DA_Presentation_Hair_Tied` |
| `Presentation.Player.Skin.LightWarm` | [P-SkinLightWarm](#p-skinlightwarm) | `/Game/Lighthaven/Art/Player/DA_Presentation_Skin_LightWarm` |
| `Presentation.Player.Skin.MediumWarm` | [P-SkinMediumWarm](#p-skinmediumwarm) | `/Game/Lighthaven/Art/Player/DA_Presentation_Skin_MediumWarm` |
| `Presentation.Player.Skin.DeepWarm` | [P-SkinDeepWarm](#p-skindeepwarm) | `/Game/Lighthaven/Art/Player/DA_Presentation_Skin_DeepWarm` |
| `Presentation.Player.Outfit.StarterLinen` | [P-Outfit](#p-outfit) | `/Game/Lighthaven/Art/Player/DA_Presentation_Outfit_StarterLinen` |
| `Presentation.Enemy.BrownRat` | [E01](#e01) | `/Game/Lighthaven/Art/Creatures/Rat/DA_Presentation_BrownRat` |
| `Presentation.Enemy.Bat` | [E02](#e02) | `/Game/Lighthaven/Art/Creatures/Bat/DA_Presentation_Bat` |
| `Presentation.Enemy.DungeonBat` | [E03](#e03) | `/Game/Lighthaven/Art/Creatures/Bat/DA_Presentation_DungeonBat` |
| `Presentation.Enemy.GreenSlime` | [E04](#e04) | `/Game/Lighthaven/Art/Creatures/Slime/DA_Presentation_GreenSlime` |
| `Presentation.Enemy.GiantBat` | [E05](#e05) | `/Game/Lighthaven/Art/Creatures/Bat/DA_Presentation_GiantBat` |
| `Presentation.Enemy.UndeadBat` | [E06](#e06) | `/Game/Lighthaven/Art/Creatures/Bat/DA_Presentation_UndeadBat` |
| `Presentation.Enemy.GiantSpider` | [E07](#e07) | `/Game/Lighthaven/Art/Creatures/Spider/DA_Presentation_GiantSpider` |
| `Presentation.Enemy.Goblin` | [E08](#e08) | `/Game/Lighthaven/Art/Creatures/Goblin/DA_Presentation_Goblin` |
| `Presentation.Enemy.GoblinWarrior` | [E09](#e09) | `/Game/Lighthaven/Art/Creatures/Goblin/DA_Presentation_GoblinWarrior` |
| `Presentation.Enemy.Atrocity` | [E10](#e10) | `/Game/Lighthaven/Art/Creatures/Atrocity/DA_Presentation_Atrocity` |
| `Presentation.Enemy.Balork` | [E11](#e11) | `/Game/Lighthaven/Art/Creatures/Demon/DA_Presentation_Balork` |
| `Presentation.NPC.BrotherKiran` | [N01](#n01) | `/Game/Lighthaven/Art/NPCs/DA_Presentation_BrotherKiran` |
| `Presentation.NPC.Kilhiam` | [N02](#n02) | `/Game/Lighthaven/Art/NPCs/DA_Presentation_Kilhiam` |
| `Presentation.NPC.Moonrock` | [N03](#n03) | `/Game/Lighthaven/Art/NPCs/DA_Presentation_Moonrock` |
| `Presentation.NPC.Samaritan` | [N04](#n04) | `/Game/Lighthaven/Art/NPCs/DA_Presentation_Samaritan` |
| `Presentation.NPC.Sigfried` | [N05](#n05) | `/Game/Lighthaven/Art/NPCs/DA_Presentation_Sigfried` |
| `Presentation.NPC.Fali` | [N06](#n06) | `/Game/Lighthaven/Art/NPCs/DA_Presentation_Fali` |
| `Presentation.NPC.Rolph` | [N07](#n07) | `/Game/Lighthaven/Art/NPCs/DA_Presentation_Rolph` |
| `Presentation.NPC.Ortanalas` | [N08](#n08) | `/Game/Lighthaven/Art/NPCs/DA_Presentation_Ortanalas` |
| `Presentation.NPC.JagarKar` | [N09](#n09) | `/Game/Lighthaven/Art/NPCs/DA_Presentation_JagarKar` |
| `Presentation.NPC.Kalastor` | [N10](#n10) | `/Game/Lighthaven/Art/NPCs/DA_Presentation_Kalastor` |
| `Presentation.NPC.Murmuntag` | [N11](#n11) | `/Game/Lighthaven/Art/NPCs/DA_Presentation_Murmuntag` |
| `Presentation.NPC.Uranos` | [N12](#n12) | `/Game/Lighthaven/Art/NPCs/DA_Presentation_Uranos` |
| `Presentation.NPC.Iraltok` | [N13](#n13) | `/Game/Lighthaven/Art/NPCs/DA_Presentation_Iraltok` |
| `Presentation.NPC.Nevanis` | [N14](#n14) | `/Game/Lighthaven/Art/NPCs/DA_Presentation_Nevanis` |
| `Presentation.NPC.Shovanis` | [N15](#n15) | `/Game/Lighthaven/Art/NPCs/DA_Presentation_Shovanis` |
| `Presentation.Interactable.Stairs.Up` | [I01](#i01) | `/Game/Lighthaven/Art/Interactables/DA_Presentation_Stairs_Up` |
| `Presentation.Interactable.Stairs.Down` | [I02](#i02) | `/Game/Lighthaven/Art/Interactables/DA_Presentation_Stairs_Down` |
| `Presentation.Environment.Shared.Stair400x100` | [S01](#s01) | `/Game/Lighthaven/Art/Environment/Shared/DA_Presentation_Stair400x100` |
| `Presentation.Environment.Shared.Stair600x120` | [S02](#s02) | `/Game/Lighthaven/Art/Environment/Shared/DA_Presentation_Stair600x120` |
| `Presentation.Environment.Shared.Stair250x80` | [S03](#s03) | `/Game/Lighthaven/Art/Environment/Shared/DA_Presentation_Stair250x80` |
| `Presentation.Environment.Shared.Stair300x80` | [S04](#s04) | `/Game/Lighthaven/Art/Environment/Shared/DA_Presentation_Stair300x80` |
| `Presentation.Environment.Church.DoorLeafPreview` | [I03](#i03) | `/Game/Lighthaven/Art/Environment/Church/DA_Presentation_DoorLeafPreview` |
| `Presentation.Interactable.Chest` | [I04](#i04) | `/Game/Lighthaven/Art/Interactables/DA_Presentation_Chest` |
| `Presentation.Interactable.Corpse.Loot` | [I05](#i05) | `/Game/Lighthaven/Art/Interactables/DA_Presentation_Corpse_Loot` |

Every package path above resolves its same-named object (for example `.../DA_Presentation_Bat.DA_Presentation_Bat`). No registry key is minted for a save point: Save is a session/menu command and `Temple.SafeSpawn` is creation/recovery only. Equipment review IDs in A-02 are not creation classes; this task does not grant a dirk, bow, spell, robe or quiver. Sigfried’s display bow/quiver is an NPC signature only.

### Player component bindings

All ten IDs inherit the shared player nameplate, cool self bar and optional amber selection. Only Body has a capsule; Face/Hair/Skin/Outfit are composed under that same root and never spawn an independent actor. The P-Body tables show the complete modest assembly for ease of construction. Body owns proportions/anchors, Face replaces head/nose, Hair replaces crown/knot, Skin sets all exposed face/hand colours, Outfit owns torso/hem/legs/boots/belt/sleeves. No doubled torso or duplicated head on composition. Skin selection applies before rendering.

<a id="p-facea"></a>
#### P-FaceA — Face A

Use P-BodyA face S28×26×28 centered(0,0,161) and nose N diameter8 from(13,0,160) to(20,0,160), selected Skin colour; no distinct eye geometry at gameplay distance. Compatible only Body.A.

<a id="p-faceb"></a>
#### P-FaceB — Face B

Same primitive head/nose dimensions and chosen Skin as Face A; compatible only Body.B. Different future face asset, deliberately identical distant placeholder. Do not invent a fifth selector.

<a id="p-haircropped"></a>
#### P-HairCropped — Cropped hair

S30×28×12 centered(−1,0,170), #30251D. Fits both bodies; top176. No rear knot.

<a id="p-hairtied"></a>
#### P-HairTied — Tied hair

Cropped cap plus S18×18×18 at(−16,0,168) and Y diameter12 from(−22,0,163) to(−24,0,146), all #30251D. Fits both bodies; top177. Rear knot/short tail is the distinguishing cue.

<a id="p-skinlightwarm"></a>
#### P-SkinLightWarm — Light Warm

Replace face/nose/hands with #BD8E72; geometry unchanged. Creation label Light Warm.

<a id="p-skinmediumwarm"></a>
#### P-SkinMediumWarm — Medium Warm

Replace face/nose/hands with #8B5A40; geometry unchanged. Creation label Medium Warm.

<a id="p-skindeepwarm"></a>
#### P-SkinDeepWarm — Deep Warm

Replace face/nose/hands with #51362C; geometry unchanged. Creation label Deep Warm. Test under actual cool fill; never brighten only this material to imply a stat.

<a id="p-outfit"></a>
#### P-Outfit — Starter Linen

Use the exact torso/hem/pelvis/legs/sleeves in selected P-Body recipe, #B5A58A; boots and belt #55402C. Modest fallback clothing, no item/stat grant. Outfit never occludes the whole face/hair. Label Starter Linen.

All twelve combinations are Body A/B × Cropped/Tied × Light/Medium/Deep Warm, with matching bundled Face and StarterLinen. No body or skin option needs a gameplay identity mark. Small differences may disappear at far view; creation names/preview and persistent IDs preserve the choice.

### Primitive recipes

Tables list every rest-pose child. No number is historical. For Y/N, size is full diameter; the two points determine length. For C/S, size is full X×Y×Z and point is center. Unlisted yaw is zero. Material names are local to each recipe. Rest geometry is inside the stated maximum review box; state transforms above are additional bounded poses. NPCs repeat the human recipe in the sheet, but only their differences are listed below.

<a id="p-bodya"></a>
#### P-BodyA — Body A

Dressed assembly ruler; rounded face/nose points +X. Body dimensions below include the shared starter outfit; face/hair/skin bindings replace only their named pieces.

Capsule R35 / HH90; review box X[-50,50], Y[-50,50], Z[0,180]. Label: **Body A** (player uses actual DisplayName in play).

Palette: `skin` `#8B5A40`, `cloth` `#B5A58A`, `leather` `#55402C`, `hair` `#30251D`, `nose` `#BD8E72`.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| boot-1 | C (24,16,18) | (5,-12,9) | leather |
| leg-1 | Y 14 | (0,-12,18) → (0,-12,76) | cloth |
| sleeve-1 | Y 16 | (0,-29,103) → (0,-29,141) | cloth |
| hand-1 | S (14,12,18) | (2,-29,95) | skin |
| boot1 | C (24,16,18) | (5,12,9) | leather |
| leg1 | Y 14 | (0,12,18) → (0,12,76) | cloth |
| sleeve1 | Y 16 | (0,29,103) → (0,29,141) | cloth |
| hand1 | S (14,12,18) | (2,29,95) | skin |
| pelvis | S (30,34,28) | (0,0,81) | cloth |
| tunic hem | N 50 | (0,0,76) → (0,0,111) | cloth |
| torso | C (28,46,58) | (0,0,118) | cloth |
| belt | C (30,48,8) | (0,0,92) | leather |
| face | S (28,26,28) | (0,0,161) | skin |
| nose | N 8 | (13,0,160) → (20,0,160) | skin |
| hair cap | S (30,28,12) | (-1,0,170) | hair |

<a id="p-bodyb"></a>
#### P-BodyB — Body B

Dressed assembly ruler; rounded face/nose points +X. Body dimensions below include the shared starter outfit; face/hair/skin bindings replace only their named pieces.

Capsule R35 / HH90; review box X[-50,50], Y[-50,50], Z[0,180]. Label: **Body B** (player uses actual DisplayName in play).

Palette: `skin` `#8B5A40`, `cloth` `#B5A58A`, `leather` `#55402C`, `hair` `#30251D`, `nose` `#BD8E72`.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| boot-1 | C (24,16,18) | (5,-12,9) | leather |
| leg-1 | Y 14 | (0,-12,18) → (0,-12,76) | cloth |
| sleeve-1 | Y 16 | (0,-27,103) → (0,-27,141) | cloth |
| hand-1 | S (14,12,18) | (2,-27,95) | skin |
| boot1 | C (24,16,18) | (5,12,9) | leather |
| leg1 | Y 14 | (0,12,18) → (0,12,76) | cloth |
| sleeve1 | Y 16 | (0,27,103) → (0,27,141) | cloth |
| hand1 | S (14,12,18) | (2,27,95) | skin |
| pelvis | S (30,34,28) | (0,0,81) | cloth |
| tunic hem | N 50 | (0,0,76) → (0,0,111) | cloth |
| torso | C (28,42,58) | (0,0,118) | cloth |
| belt | C (30,44,8) | (0,0,92) | leather |
| face | S (28,26,28) | (0,0,161) | skin |
| nose | N 8 | (13,0,160) → (20,0,160) | skin |
| hair cap | S (30,28,12) | (-1,0,170) | hair |

<a id="e01"></a>
#### E01 — Brown Rat

Rest top25, maximum35; narrow back stripe and continuous tail, snout faces +X.

Capsule R25 / HH25; review box X[-67.5,22.5], Y[-14,14], Z[0,35]. Label: **Brown Rat**. Shared amber target highlight.

Palette: `body` `#594537`, `mark` `#806850`, `tail` `#9A7865`.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| body | S (35,24,23) | (-4,0,12.5) | body |
| head | S (16,18,16) | (10,0,12) | body |
| snout | N 10 | (16,0,12) → (22.5,0,12) | tail |
| back stripe | C (24,14,2) | (-5,0,23) | mark |
| tail | Y 5 | (-20,0,8) → (-65,0,4) | tail |
| ear-1 | S (7,7,7) | (4,-10.5,21) | tail |
| foot(-1, -12) | Y 5 | (-12,-9,0) → (-12,-9,6) | tail |
| foot(-1, 10) | Y 5 | (10,-9,0) → (10,-9,6) | tail |
| ear1 | S (7,7,7) | (4,10.5,21) | tail |
| foot(1, -12) | Y 5 | (-12,9,0) → (-12,9,6) | tail |
| foot(1, 10) | Y 5 | (10,9,0) → (10,9,6) | tail |

<a id="e02"></a>
#### E02 — Bat

Suspended body; short +X muzzle and paired ears. Wing marks stay on top surfaces. Ground capsule remains grounded.

Capsule R25 / HH75; review box X[-22.5,22.5], Y[-40,40], Z[0,145]. Label: **Bat**. Shared amber target highlight.

Palette: `body` `#514236`, `wing` `#927451`, `mark` `#AF9169`.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| body | S (24.75,20,36) | (0,0,100) | body |
| muzzle | S (12,14,12) | (16.5,0,100) | mark |
| ear-1 | N 10 | (2,-7,115) → (2,-7,135) | body |
| inner wing-1 | C (34,21,5) | (0,-20.5,100) | wing |
| point-1 | N 24 | (0,-25,100) → (0,-40,100) | wing |
| ear1 | N 10 | (2,7,115) → (2,7,135) | body |
| inner wing1 | C (34,21,5) | (0,20.5,100) | wing |
| point1 | N 24 | (0,25,100) → (0,40,100) | wing |

<a id="e03"></a>
#### E03 — Dungeon Bat

Suspended body; short +X muzzle and paired ears. Wing marks stay on top surfaces. Ground capsule remains grounded.

Capsule R30 / HH80; review box X[-27.5,27.5], Y[-50,50], Z[0,155]. Label: **Dungeon Bat**. Shared amber target highlight.

Palette: `body` `#494B48`, `wing` `#666453`, `mark` `#B7A987`.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| body | S (30.25,20,36) | (0,0,110) | body |
| muzzle | S (12,14,12) | (21.5,0,110) | mark |
| ear-1 | N 10 | (2,-7,125) → (2,-7,145) | body |
| square wing-1 | C (44,39,6) | (-2,-30.5,110) | wing |
| joined V-1 | Y 10 | (15,0,130) → (-12,-30,117) | mark |
| ear1 | N 10 | (2,7,125) → (2,7,145) | body |
| square wing1 | C (44,39,6) | (-2,30.5,110) | wing |
| joined V1 | Y 10 | (15,0,130) → (-12,30,117) | mark |

<a id="e05"></a>
#### E05 — Giant Bat

Suspended body; short +X muzzle and paired ears. Wing marks stay on top surfaces. Ground capsule remains grounded.

Capsule R45 / HH100; review box X[-42.5,42.5], Y[-85,85], Z[0,190]. Label: **Giant Bat**. Shared amber target highlight.

Palette: `body` `#45372D`, `wing` `#775638`, `mark` `#B29867`.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| body | S (46.75,32,52) | (0,0,130) | body |
| muzzle | S (12,14,12) | (36.5,0,130) | mark |
| ear-1 | N 10 | (2,-7,145) → (2,-7,165) | body |
| deep inner wing-1 | C (64,41,8) | (-3,-36.5,130) | wing |
| point-1 | N 44 | (-3,-51,130) → (-3,-85,130) | wing |
| leading band-1 | C (12,45,3) | (23,-38.5,136) | mark |
| ear1 | N 10 | (2,7,145) → (2,7,165) | body |
| deep inner wing1 | C (64,41,8) | (-3,36.5,130) | wing |
| point1 | N 44 | (-3,51,130) → (-3,85,130) | wing |
| leading band1 | C (12,45,3) | (23,38.5,136) | mark |

<a id="e06"></a>
#### E06 — Undead Bat

Suspended body; short +X muzzle and paired ears. Wing marks stay on top surfaces. Ground capsule remains grounded.

Capsule R30 / HH85; review box X[-30,30], Y[-55,55], Z[0,165]. Label: **Undead Bat**. Shared amber target highlight.

Palette: `body` `#414544`, `wing` `#64685D`, `mark` `#B6B49F`.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| body | S (33,20,36) | (0,0,115) | body |
| muzzle | S (12,14,12) | (24,0,115) | mark |
| ear-1 | N 10 | (2,-7,130) → (2,-7,150) | body |
| inner wing-1 | C (44,20,6) | (0,-21,115) | wing |
| outer leading block-1 | C (24,24,6) | (10,-43,115) | mark |
| outer rear block-1 | C (20,12,6) | (-12,-37,115) | mark |
| ear1 | N 10 | (2,7,130) → (2,7,150) | body |
| inner wing1 | C (44,20,6) | (0,21,115) | wing |
| outer leading block1 | C (24,24,6) | (10,43,115) | mark |
| outer rear block1 | C (20,12,6) | (-12,37,115) | mark |

<a id="e04"></a>
#### E04 — Green Slime

Rest top40, maximum60. Off-center raised mound at +X marks facing; selected pointer supplies precise heading.

Capsule R40 / HH40; review box X[-45,45], Y[-45,45], Z[0,60]. Label: **Green Slime**. Shared amber target highlight.

Palette: `body` `#3F722E`, `mark` `#709B40`, `dark` `#263E23`.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| pooled base | S (90,90,16) | (0,0,8) | dark |
| left lobe | S (56,54,26) | (-10,-12,13) | body |
| high front lobe | S (62,58,40) | (12,8,20) | mark |

<a id="e07"></a>
#### E07 — Giant Spider

Rest top65, maximum90; eight bent leg gaps and rust-red front ball at +X.

Capsule R55 / HH55; review box X[-65,85], Y[-85,85], Z[0,90]. Label: **Giant Spider**. Shared amber target highlight.

Palette: `body` `#66645E`, `joint` `#3D3D39`, `front` `#8F493B`, `ridge` `#959083`.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| abdomen | S (66,62,50) | (-25,0,40) | body |
| front body | S (42,46,32) | (28,0,37) | body |
| face | S (18,26,18) | (51,0,32) | front |
| ridge | C (44,16,3) | (-25,0,63) | ridge |
| upper leg(-1, 0) | Y 8 | (-40,-20,36) → (-52,-62,53) | body |
| lower leg(-1, 0) | Y 8 | (-52,-62,53) → (-59,-80,5) | joint |
| upper leg(-1, 1) | Y 8 | (-15,-20,36) → (-24,-62,53) | body |
| lower leg(-1, 1) | Y 8 | (-24,-62,53) → (-25,-80,5) | joint |
| upper leg(-1, 2) | Y 8 | (15,-20,36) → (25,-62,53) | body |
| lower leg(-1, 2) | Y 8 | (25,-62,53) → (34,-80,5) | joint |
| upper leg(-1, 3) | Y 8 | (34,-20,36) → (58,-62,53) | body |
| lower leg(-1, 3) | Y 8 | (58,-62,53) → (79,-80,5) | joint |
| upper leg(1, 0) | Y 8 | (-40,20,36) → (-52,62,53) | body |
| lower leg(1, 0) | Y 8 | (-52,62,53) → (-59,80,5) | joint |
| upper leg(1, 1) | Y 8 | (-15,20,36) → (-24,62,53) | body |
| lower leg(1, 1) | Y 8 | (-24,62,53) → (-25,80,5) | joint |
| upper leg(1, 2) | Y 8 | (15,20,36) → (25,62,53) | body |
| lower leg(1, 2) | Y 8 | (25,62,53) → (34,80,5) | joint |
| upper leg(1, 3) | Y 8 | (34,20,36) → (58,62,53) | body |
| lower leg(1, 3) | Y 8 | (58,62,53) → (79,80,5) | joint |

<a id="e08"></a>
#### E08 — Goblin

Head125; ordinary carried top185, Warrior210. Nose and front-carried polearm indicate +X. Yoke/bar are Warrior-only.

Capsule R35 / HH70; review box X[-45,65], Y[-42.5,42.5], Z[0,200]. Label: **Goblin**. Shared amber target highlight.

Palette: `skin` `#8B3227`, `cloth` `#48392C`, `steel` `#B3ABA0`, `mark` `#512D27`.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| leg-1 | Y 13 | (0,-12,6) → (0,-12,58) | skin |
| foot-1 | S (24,16,12) | (6,-12,6) | cloth |
| arm-1 | Y 12 | (0,-23,87) → (14,-27,57) | skin |
| ear-1 | N 13 | (0,-12,111) → (0,-30,119) | skin |
| leg1 | Y 13 | (0,12,6) → (0,12,58) | skin |
| foot1 | S (24,16,12) | (6,12,6) | cloth |
| arm1 | Y 12 | (0,23,87) → (14,27,57) | skin |
| ear1 | N 13 | (0,12,111) → (0,30,119) | skin |
| torso | S (34,40,52) | (0,0,79) | skin |
| waist | C (36,44,15) | (0,0,56) | cloth |
| head | S (28,26,28) | (4,0,111) | skin |
| nose | N 10 | (16,0,111) → (28,0,111) | skin |
| shaft | Y 6 | (24,30,15) → (24,30,175) | cloth |
| single blade | N 14 | (24,30,165) → (24,30,185) | steel |

<a id="e09"></a>
#### E09 — Goblin Warrior

Head125; ordinary carried top185, Warrior210. Nose and front-carried polearm indicate +X. Yoke/bar are Warrior-only.

Capsule R40 / HH75; review box X[-50,70], Y[-52.5,52.5], Z[0,215]. Label: **Goblin Warrior**. Shared amber target highlight.

Palette: `skin` `#793127`, `cloth` `#4E4031`, `steel` `#B8B1A3`, `mark` `#C0AC86`.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| leg-1 | Y 13 | (0,-12,6) → (0,-12,58) | skin |
| foot-1 | S (24,16,12) | (6,-12,6) | cloth |
| arm-1 | Y 12 | (0,-23,87) → (14,-27,57) | skin |
| ear-1 | N 13 | (0,-12,111) → (0,-30,119) | skin |
| leg1 | Y 13 | (0,12,6) → (0,12,58) | skin |
| foot1 | S (24,16,12) | (6,12,6) | cloth |
| arm1 | Y 12 | (0,23,87) → (14,27,57) | skin |
| ear1 | N 13 | (0,12,111) → (0,30,119) | skin |
| torso | S (34,40,52) | (0,0,79) | skin |
| waist | C (36,44,15) | (0,0,56) | cloth |
| head | S (28,26,28) | (4,0,111) | skin |
| nose | N 10 | (16,0,111) → (28,0,111) | skin |
| shoulder-1 | C (32,28,20) | (-3,-29,100) | cloth |
| shoulder1 | C (32,28,20) | (-3,29,100) | cloth |
| back band | C (18,68,4) | (-11,0,111) | mark |
| shaft | Y 8 | (28,34,20) → (28,34,210) | cloth |
| split blade-1 | N 16 | (28,25,183) → (28,25,210) | steel |
| split blade1 | N 16 | (28,43,183) → (28,43,210) | steel |

<a id="e10"></a>
#### E10 — Atrocity

Rest top190, maximum210; hunched shoulders and forward ochre claw fans. No upright zombie pose.

Capsule R65 / HH100; review box X[-65,95], Y[-90,90], Z[0,210]. Label: **Atrocity**. Shared amber target highlight.

Palette: `skin` `#343038`, `muscle` `#58505A`, `claw` `#BFA44D`.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| hunch | S (110,110,100) | (-10,0,137) | skin |
| head | S (42,46,42) | (53,0,114) | muscle |
| snout | N 20 | (70,0,110) → (92,0,103) | skin |
| shoulder-1 | S (70,60,60) | (-12,-42,151) | muscle |
| leg-1 | Y 32 | (-20,-32,20) → (-20,-32,94) | skin |
| foot-1 | S (52,40,24) | (0,-32,12) | skin |
| upper arm-1 | Y 30 | (-7,-56,141) → (21,-72,98) | muscle |
| forearm-1 | Y 24 | (21,-72,98) → (59,-72,69) | skin |
| claw(-1, -1) | N 10 | (63,-62,70) → (92,-62,54) | claw |
| claw(-1, 0) | N 10 | (63,-69,70) → (92,-69,54) | claw |
| claw(-1, 1) | N 10 | (63,-76,70) → (92,-76,54) | claw |
| shoulder1 | S (70,60,60) | (-12,42,151) | muscle |
| leg1 | Y 32 | (-20,32,20) → (-20,32,94) | skin |
| foot1 | S (52,40,24) | (0,32,12) | skin |
| upper arm1 | Y 30 | (-7,56,141) → (21,72,98) | muscle |
| forearm1 | Y 24 | (21,72,98) → (59,72,69) | skin |
| claw(1, -1) | N 10 | (63,62,70) → (92,62,54) | claw |
| claw(1, 0) | N 10 | (63,69,70) → (92,69,54) | claw |
| claw(1, 1) | N 10 | (63,76,70) → (92,76,54) | claw |
| back spine | N 20 | (-25,0,170) → (-45,0,190) | claw |

<a id="e11"></a>
#### E11 — Balork

Head260, horn310; wings440, horizontal shaft400. Red patterned wing blocks plus horns separate boss from every ordinary family.

Capsule R100 / HH155; review box X[-100,100], Y[-220,220], Z[0,310]. Label: **Balork**. Shared amber target highlight.

Palette: `body` `#6B2522`, `dark` `#291F22`, `wing` `#A33A29`, `horn` `#B7A181`, `steel` `#A9A39B`.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| torso | S (100,120,140) | (0,0,165) | body |
| head | S (58,64,56) | (15,0,232) | body |
| muzzle | N 24 | (40,0,229) → (65,0,226) | dark |
| leg-1 | Y 38 | (0,-35,20) → (0,-35,115) | body |
| foot-1 | S (70,45,30) | (18,-35,15) | dark |
| horn-1 | N 24 | (12,-23,252) → (2,-36,310) | horn |
| inner wing-1 | C (100,105,12) | (-35,-102.5,208) | wing |
| wing tip-1 | N 80 | (-35,-145,208) → (-35,-220,208) | wing |
| wing pattern-1 | C (55,64,4) | (-26,-108,217) | dark |
| arm-1 | Y 30 | (0,-55,200) → (40,-64,137) | body |
| leg1 | Y 38 | (0,35,20) → (0,35,115) | body |
| foot1 | S (70,45,30) | (18,35,15) | dark |
| horn1 | N 24 | (12,23,252) → (2,36,310) | horn |
| inner wing1 | C (100,105,12) | (-35,102.5,208) | wing |
| wing tip1 | N 80 | (-35,145,208) → (-35,220,208) | wing |
| wing pattern1 | C (55,64,4) | (-26,108,217) | dark |
| arm1 | Y 30 | (0,55,200) → (40,64,137) | body |
| polearm shaft | Y 10 | (40,-200,130) → (40,200,130) | horn |
| blade | N 36 | (40,165,130) → (40,210,130) | steel |

<a id="n01"></a>
#### N01 — Brother Kiran · Priest

Book: broad open slab and raised central spine. Human front remains +X. Common human capsule; all props fit100×100×180.

Capsule R35 / HH90; review box X[-50,50], Y[-50,50], Z[0,180]. Label: **Brother Kiran · Priest**. Shared amber target highlight.

Palette: `skin` `#8B5A40`, `cloth` `#B5A58A`, `leather` `#55402C`, `hair` `#30251D`, `nose` `#BD8E72`, `sign` `#A9C4CC`, `pale` `#C0AC86`, `red` `#A33A29`, `stone` `#959083`.

Start from all P-BodyA pieces with this palette (same face/hair/boots; all NPCs use Medium Warm here as an arbitrary shared placeholder). Add only these signature pieces; they identify services independently of skin. NPC role/presence text above is the complete label, with service text wrapped.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| book | C (35,46,8) | (26,0,132) | pale |
| spine | C (37,5,10) | (26,0,134) | leather |

<a id="n02"></a>
#### N02 — Kilhiam · Light teacher

Sun: sphere and four thick square rays. Human front remains +X. Common human capsule; all props fit100×100×180.

Capsule R35 / HH90; review box X[-50,50], Y[-50,50], Z[0,180]. Label: **Kilhiam · Light teacher**. Shared amber target highlight.

Palette: `skin` `#8B5A40`, `cloth` `#C0AC86`, `leather` `#55402C`, `hair` `#30251D`, `nose` `#BD8E72`, `sign` `#A9C4CC`, `pale` `#C0AC86`, `red` `#A33A29`, `stone` `#959083`.

Start from all P-BodyA pieces with this palette (same face/hair/boots; all NPCs use Medium Warm here as an arbitrary shared placeholder). Add only these signature pieces; they identify services independently of skin. NPC role/presence text above is the complete label, with service text wrapped.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| sun | S (26,26,26) | (25,0,143) | pale |
| sun ray Y-1 | C (9,14,6) | (25,-20,143) | pale |
| sun ray X-1 | C (14,9,6) | (8,0,143) | pale |
| sun ray Y1 | C (9,14,6) | (25,20,143) | pale |
| sun ray X1 | C (14,9,6) | (42,0,143) | pale |

<a id="n03"></a>
#### N03 — Moonrock · Heal Light teacher

Two separated pale tablets. Human front remains +X. Common human capsule; all props fit100×100×180.

Capsule R35 / HH90; review box X[-50,50], Y[-50,50], Z[0,180]. Label: **Moonrock · Heal Light teacher**. Shared amber target highlight.

Palette: `skin` `#8B5A40`, `cloth` `#5E6973`, `leather` `#55402C`, `hair` `#30251D`, `nose` `#BD8E72`, `sign` `#A9C4CC`, `pale` `#C0AC86`, `red` `#A33A29`, `stone` `#959083`.

Start from all P-BodyA pieces with this palette (same face/hair/boots; all NPCs use Medium Warm here as an arbitrary shared placeholder). Add only these signature pieces; they identify services independently of skin. NPC role/presence text above is the complete label, with service text wrapped.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| tablet-1 | C (32,14,12) | (25,-12,139) | sign |
| tablet1 | C (32,14,12) | (25,12,139) | sign |

<a id="n04"></a>
#### N04 — Samaritan · Rat errand

Side satchel: round bag and horizontal rolled top. Human front remains +X. Common human capsule; all props fit100×100×180.

Capsule R35 / HH90; review box X[-50,50], Y[-50,50], Z[0,180]. Label: **Samaritan · Rat errand**. Shared amber target highlight.

Palette: `skin` `#8B5A40`, `cloth` `#927451`, `leather` `#55402C`, `hair` `#30251D`, `nose` `#BD8E72`, `sign` `#A9C4CC`, `pale` `#C0AC86`, `red` `#A33A29`, `stone` `#959083`.

Start from all P-BodyA pieces with this palette (same face/hair/boots; all NPCs use Medium Warm here as an arbitrary shared placeholder). Add only these signature pieces; they identify services independently of skin. NPC role/presence text above is the complete label, with service text wrapped.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| satchel | S (30,28,36) | (-6,34,82) | pale |
| rolled bag top | Y 12 | (-17,34,99) → (7,34,99) | leather |

<a id="n05"></a>
#### N05 — Sigfried · Bow / quiver

Angular tall bow with open center and separate back quiver. Human front remains +X. Common human capsule; all props fit100×100×180.

Capsule R35 / HH90; review box X[-50,50], Y[-50,50], Z[0,180]. Label: **Sigfried · Bow / quiver**. Shared amber target highlight.

Palette: `skin` `#8B5A40`, `cloth` `#62634A`, `leather` `#55402C`, `hair` `#30251D`, `nose` `#BD8E72`, `sign` `#A9C4CC`, `pale` `#C0AC86`, `red` `#A33A29`, `stone` `#959083`.

Start from all P-BodyA pieces with this palette (same face/hair/boots; all NPCs use Medium Warm here as an arbitrary shared placeholder). Add only these signature pieces; they identify services independently of skin. NPC role/presence text above is the complete label, with service text wrapped.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| bow lower | Y 8 | (0,-42,35) → (22,-42,105) | leather |
| bow upper | Y 8 | (22,-42,105) → (0,-42,175) | leather |
| quiver | Y 18 | (-25,22,88) → (-25,22,143) | leather |
| arrow17 | Y 4 | (-25,17,95) → (-25,17,160) | pale |
| arrow27 | Y 4 | (-25,27,95) → (-25,27,160) | pale |

<a id="n06"></a>
#### N06 — Fali · Mana potion seller

Round bottle with narrow neck; no potion price implied. Human front remains +X. Common human capsule; all props fit100×100×180.

Capsule R35 / HH90; review box X[-50,50], Y[-50,50], Z[0,180]. Label: **Fali · Mana potion seller**. Shared amber target highlight.

Palette: `skin` `#8B5A40`, `cloth` `#5E6973`, `leather` `#55402C`, `hair` `#30251D`, `nose` `#BD8E72`, `sign` `#A9C4CC`, `pale` `#C0AC86`, `red` `#A33A29`, `stone` `#959083`.

Start from all P-BodyA pieces with this palette (same face/hair/boots; all NPCs use Medium Warm here as an arbitrary shared placeholder). Add only these signature pieces; they identify services independently of skin. NPC role/presence text above is the complete label, with service text wrapped.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| bottle | S (32,32,36) | (27,0,125) | sign |
| bottle neck | Y 12 | (27,0,139) → (27,0,156) | pale |

<a id="n07"></a>
#### N07 — Rolph · Armour merchant

Wide armour bib with square raised rim; stock unresolved. Human front remains +X. Common human capsule; all props fit100×100×180.

Capsule R35 / HH90; review box X[-50,50], Y[-50,50], Z[0,180]. Label: **Rolph · Armour merchant**. Shared amber target highlight.

Palette: `skin` `#8B5A40`, `cloth` `#535451`, `leather` `#55402C`, `hair` `#30251D`, `nose` `#BD8E72`, `sign` `#A9C4CC`, `pale` `#C0AC86`, `red` `#A33A29`, `stone` `#959083`.

Start from all P-BodyA pieces with this palette (same face/hair/boots; all NPCs use Medium Warm here as an arbitrary shared placeholder). Add only these signature pieces; they identify services independently of skin. NPC role/presence text above is the complete label, with service text wrapped.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| bib | C (8,58,55) | (19,0,119) | stone |
| raised rim | C (12,62,10) | (19,0,149) | pale |

<a id="n08"></a>
#### N08 — Ortanalas · Attack / Archery

Two long upright training bars, clear center gap. Human front remains +X. Common human capsule; all props fit100×100×180.

Capsule R35 / HH90; review box X[-50,50], Y[-50,50], Z[0,180]. Label: **Ortanalas · Attack / Archery**. Shared amber target highlight.

Palette: `skin` `#8B5A40`, `cloth` `#B5A58A`, `leather` `#55402C`, `hair` `#30251D`, `nose` `#BD8E72`, `sign` `#A9C4CC`, `pale` `#C0AC86`, `red` `#A33A29`, `stone` `#959083`.

Start from all P-BodyA pieces with this palette (same face/hair/boots; all NPCs use Medium Warm here as an arbitrary shared placeholder). Add only these signature pieces; they identify services independently of skin. NPC role/presence text above is the complete label, with service text wrapped.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| training bar-1 | Y 12 | (27,-17,97) → (27,-17,165) | pale |
| training bar1 | Y 12 | (27,17,97) → (27,17,165) | pale |

<a id="n09"></a>
#### N09 — Jagar Kar · Town resident

Two short separated shoulder tabs; no assigned offer. Human front remains +X. Common human capsule; all props fit100×100×180.

Capsule R35 / HH90; review box X[-50,50], Y[-50,50], Z[0,180]. Label: **Jagar Kar · Town resident**. Shared amber target highlight.

Palette: `skin` `#8B5A40`, `cloth` `#5E6973`, `leather` `#55402C`, `hair` `#30251D`, `nose` `#BD8E72`, `sign` `#A9C4CC`, `pale` `#C0AC86`, `red` `#A33A29`, `stone` `#959083`.

Start from all P-BodyA pieces with this palette (same face/hair/boots; all NPCs use Medium Warm here as an arbitrary shared placeholder). Add only these signature pieces; they identify services independently of skin. NPC role/presence text above is the complete label, with service text wrapped.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| shoulder tab-1 | C (25,14,8) | (0,-24,150) | pale |
| shoulder tab1 | C (25,14,8) | (0,24,150) | pale |

<a id="n10"></a>
#### N10 — Kalastor · Dodge / Archery

Three offset flat bars, stepped zigzag. Human front remains +X. Common human capsule; all props fit100×100×180.

Capsule R35 / HH90; review box X[-50,50], Y[-50,50], Z[0,180]. Label: **Kalastor · Dodge / Archery**. Shared amber target highlight.

Palette: `skin` `#8B5A40`, `cloth` `#62634A`, `leather` `#55402C`, `hair` `#30251D`, `nose` `#BD8E72`, `sign` `#A9C4CC`, `pale` `#C0AC86`, `red` `#A33A29`, `stone` `#959083`.

Start from all P-BodyA pieces with this palette (same face/hair/boots; all NPCs use Medium Warm here as an arbitrary shared placeholder). Add only these signature pieces; they identify services independently of skin. NPC role/presence text above is the complete label, with service text wrapped.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| zigzag bar0 | C (16,32,6) | (12,-12,141) | sign |
| zigzag bar1 | C (16,32,6) | (22,0,146) | sign |
| zigzag bar2 | C (16,32,6) | (32,12,151) | sign |

<a id="n11"></a>
#### N11 — Murmuntag · Attack trainer

Round plate with a thick square boss. Human front remains +X. Common human capsule; all props fit100×100×180.

Capsule R35 / HH90; review box X[-50,50], Y[-50,50], Z[0,180]. Label: **Murmuntag · Attack trainer**. Shared amber target highlight.

Palette: `skin` `#8B5A40`, `cloth` `#806850`, `leather` `#55402C`, `hair` `#30251D`, `nose` `#BD8E72`, `sign` `#A9C4CC`, `pale` `#C0AC86`, `red` `#A33A29`, `stone` `#959083`.

Start from all P-BodyA pieces with this palette (same face/hair/boots; all NPCs use Medium Warm here as an arbitrary shared placeholder). Add only these signature pieces; they identify services independently of skin. NPC role/presence text above is the complete label, with service text wrapped.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| round plate | Y 44 | (22,0,126) → (22,0,135) | pale |
| boss | C (18,18,10) | (22,0,140) | leather |

<a id="n12"></a>
#### N12 — Uranos · Stone Shard / wings

Three stepped stone cubes, largest at bottom. Human front remains +X. Common human capsule; all props fit100×100×180.

Capsule R35 / HH90; review box X[-50,50], Y[-50,50], Z[0,180]. Label: **Uranos · Stone Shard / wings**. Shared amber target highlight.

Palette: `skin` `#8B5A40`, `cloth` `#666453`, `leather` `#55402C`, `hair` `#30251D`, `nose` `#BD8E72`, `sign` `#A9C4CC`, `pale` `#C0AC86`, `red` `#A33A29`, `stone` `#959083`.

Start from all P-BodyA pieces with this palette (same face/hair/boots; all NPCs use Medium Warm here as an arbitrary shared placeholder). Add only these signature pieces; they identify services independently of skin. NPC role/presence text above is the complete label, with service text wrapped.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| cairn0 | C (38,38,13) | (25,0,122) | stone |
| cairn1 | C (28,28,13) | (25,0,135) | stone |
| cairn2 | C (18,18,13) | (25,0,148) | stone |

<a id="n13"></a>
#### N13 — Iraltok · Fire Dart teacher

Tall flame cone on square base. Human front remains +X. Common human capsule; all props fit100×100×180.

Capsule R35 / HH90; review box X[-50,50], Y[-50,50], Z[0,180]. Label: **Iraltok · Fire Dart teacher**. Shared amber target highlight.

Palette: `skin` `#8B5A40`, `cloth` `#733D35`, `leather` `#55402C`, `hair` `#30251D`, `nose` `#BD8E72`, `sign` `#A9C4CC`, `pale` `#C0AC86`, `red` `#A33A29`, `stone` `#959083`.

Start from all P-BodyA pieces with this palette (same face/hair/boots; all NPCs use Medium Warm here as an arbitrary shared placeholder). Add only these signature pieces; they identify services independently of skin. NPC role/presence text above is the complete label, with service text wrapped.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| flame base | C (30,30,6) | (25,0,120) | pale |
| flame | N 30 | (25,0,123) → (25,0,172) | red |

<a id="n14"></a>
#### N14 — Nevanis · Healer (B1)

Long pale apron plus large cool cross. Human front remains +X. Common human capsule; all props fit100×100×180.

Capsule R35 / HH90; review box X[-50,50], Y[-50,50], Z[0,180]. Label: **Nevanis · Healer (B1)**. Shared amber target highlight.

Palette: `skin` `#8B5A40`, `cloth` `#B5A58A`, `leather` `#55402C`, `hair` `#30251D`, `nose` `#BD8E72`, `sign` `#A9C4CC`, `pale` `#C0AC86`, `red` `#A33A29`, `stone` `#959083`.

Start from all P-BodyA pieces with this palette (same face/hair/boots; all NPCs use Medium Warm here as an arbitrary shared placeholder). Add only these signature pieces; they identify services independently of skin. NPC role/presence text above is the complete label, with service text wrapped.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| apron | C (4,36,72) | (17,0,83) | pale |
| plus vertical | C (34,10,5) | (28,0,140) | sign |
| plus crossbar | C (10,34,5) | (28,0,141) | sign |

<a id="n15"></a>
#### N15 — Shovanis · Dust Devil (B1)

Three short offset cylinders forming an air curl. Human front remains +X. Common human capsule; all props fit100×100×180.

Capsule R35 / HH90; review box X[-50,50], Y[-50,50], Z[0,180]. Label: **Shovanis · Dust Devil (B1)**. Shared amber target highlight.

Palette: `skin` `#8B5A40`, `cloth` `#927451`, `leather` `#55402C`, `hair` `#30251D`, `nose` `#BD8E72`, `sign` `#A9C4CC`, `pale` `#C0AC86`, `red` `#A33A29`, `stone` `#959083`.

Start from all P-BodyA pieces with this palette (same face/hair/boots; all NPCs use Medium Warm here as an arbitrary shared placeholder). Add only these signature pieces; they identify services independently of skin. NPC role/presence text above is the complete label, with service text wrapped.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| air curl0 | Y 10 | (12,-19,132) → (12,19,132) | pale |
| air curl1 | Y 10 | (21,-14,142) → (21,24,142) | pale |
| air curl2 | Y 10 | (30,-9,152) → (30,29,152) | pale |

<a id="i01"></a>
#### I01 — Stairs Up

Travel points +X. Single transverse bar=Up, double=Down; destination label is mandatory. Nonblocking floor cue; not the travel trigger.

No character capsule. Review box X[-80,80], Y[-60,60], Z[0,6]. Shared amber selection brackets; facing/label as stated above.

Palette: `base` `#A9C4CC`.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| arrow stem | C (60,14,2) | (-15,0,2) | base |
| arrow head | N 36; flatten world-Z around center to0.12 | (15,0,3) → (65,0,3) | base |
| direction bar0 | C (8,72,2) | (-65,0,2) | base |

<a id="i02"></a>
#### I02 — Stairs Down

Travel points +X. Single transverse bar=Up, double=Down; destination label is mandatory. Nonblocking floor cue; not the travel trigger.

No character capsule. Review box X[-80,80], Y[-60,60], Z[0,6]. Shared amber selection brackets; facing/label as stated above.

Palette: `base` `#E9BF79`.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| arrow stem | C (60,14,2) | (-15,0,2) | base |
| arrow head | N 36; flatten world-Z around center to0.12 | (15,0,3) → (65,0,3) | base |
| direction bar0 | C (8,72,2) | (-65,0,2) | base |
| direction bar1 | C (8,72,2) | (-50,0,2) | base |

<a id="s01"></a>
#### S01 — Stair 400×100

Shown rising +X; signed descending variant built from exact endpoint elevations, never negative actor scale. A-02 width300. Treads are visual children over map-owned smooth collision ramp.

No character capsule. Review box X[0,400], Y[-150,150], Z[-20,100]. Decorative stair: no separate selection or nameplate; select the associated I01/I02 travel cue.

Palette: `stone` `#77766D`.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| tread1 | C (40,300,30) | (20,0,-5) | stone |
| tread2 | C (40,300,40) | (60,0,0) | stone |
| tread3 | C (40,300,50) | (100,0,5) | stone |
| tread4 | C (40,300,60) | (140,0,10) | stone |
| tread5 | C (40,300,70) | (180,0,15) | stone |
| tread6 | C (40,300,80) | (220,0,20) | stone |
| tread7 | C (40,300,90) | (260,0,25) | stone |
| tread8 | C (40,300,100) | (300,0,30) | stone |
| tread9 | C (40,300,110) | (340,0,35) | stone |
| tread10 | C (40,300,120) | (380,0,40) | stone |

Descending counterpart with run L, rise magnitude H and n=H/10: for i=1…n use C(L/n,300,H+20−10i), centered((i−0.5)L/n,0,(−10i−H−20)/2). Tops are −10i and every base is Z=−H−20, below the walking surface. Do not mirror the complete ascending solid or use negative scale. The authored run/rise endpoints and approach remain those in A-02. Map-owned smooth ramp gives collision; these cubes are only a coarse tread illustration, not a new step-up rule. Use the Up/Down overlay on the flat approach, not an enlarged trigger.

<a id="s02"></a>
#### S02 — Stair 600×120

Shown rising +X; signed descending variant built from exact endpoint elevations, never negative actor scale. A-02 width300. Treads are visual children over map-owned smooth collision ramp.

No character capsule. Review box X[0,600], Y[-150,150], Z[-20,120]. Decorative stair: no separate selection or nameplate; select the associated I01/I02 travel cue.

Palette: `stone` `#77766D`.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| tread1 | C (50,300,30) | (25,0,-5) | stone |
| tread2 | C (50,300,40) | (75,0,0) | stone |
| tread3 | C (50,300,50) | (125,0,5) | stone |
| tread4 | C (50,300,60) | (175,0,10) | stone |
| tread5 | C (50,300,70) | (225,0,15) | stone |
| tread6 | C (50,300,80) | (275,0,20) | stone |
| tread7 | C (50,300,90) | (325,0,25) | stone |
| tread8 | C (50,300,100) | (375,0,30) | stone |
| tread9 | C (50,300,110) | (425,0,35) | stone |
| tread10 | C (50,300,120) | (475,0,40) | stone |
| tread11 | C (50,300,130) | (525,0,45) | stone |
| tread12 | C (50,300,140) | (575,0,50) | stone |

Descending counterpart with run L, rise magnitude H and n=H/10: for i=1…n use C(L/n,300,H+20−10i), centered((i−0.5)L/n,0,(−10i−H−20)/2). Tops are −10i and every base is Z=−H−20, below the walking surface. Do not mirror the complete ascending solid or use negative scale. The authored run/rise endpoints and approach remain those in A-02. Map-owned smooth ramp gives collision; these cubes are only a coarse tread illustration, not a new step-up rule. Use the Up/Down overlay on the flat approach, not an enlarged trigger.

<a id="s03"></a>
#### S03 — Stair 250×80

Shown rising +X; signed descending variant built from exact endpoint elevations, never negative actor scale. A-02 width300. Treads are visual children over map-owned smooth collision ramp.

No character capsule. Review box X[0,250], Y[-150,150], Z[-20,80]. Decorative stair: no separate selection or nameplate; select the associated I01/I02 travel cue.

Palette: `stone` `#77766D`.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| tread1 | C (31.25,300,30) | (15.625,0,-5) | stone |
| tread2 | C (31.25,300,40) | (46.875,0,0) | stone |
| tread3 | C (31.25,300,50) | (78.125,0,5) | stone |
| tread4 | C (31.25,300,60) | (109.375,0,10) | stone |
| tread5 | C (31.25,300,70) | (140.625,0,15) | stone |
| tread6 | C (31.25,300,80) | (171.875,0,20) | stone |
| tread7 | C (31.25,300,90) | (203.125,0,25) | stone |
| tread8 | C (31.25,300,100) | (234.375,0,30) | stone |

Descending counterpart with run L, rise magnitude H and n=H/10: for i=1…n use C(L/n,300,H+20−10i), centered((i−0.5)L/n,0,(−10i−H−20)/2). Tops are −10i and every base is Z=−H−20, below the walking surface. Do not mirror the complete ascending solid or use negative scale. The authored run/rise endpoints and approach remain those in A-02. Map-owned smooth ramp gives collision; these cubes are only a coarse tread illustration, not a new step-up rule. Use the Up/Down overlay on the flat approach, not an enlarged trigger.

<a id="s04"></a>
#### S04 — Stair 300×80

Shown rising +X; signed descending variant built from exact endpoint elevations, never negative actor scale. A-02 width300. Treads are visual children over map-owned smooth collision ramp.

No character capsule. Review box X[0,300], Y[-150,150], Z[-20,80]. Decorative stair: no separate selection or nameplate; select the associated I01/I02 travel cue.

Palette: `stone` `#77766D`.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| tread1 | C (37.5,300,30) | (18.75,0,-5) | stone |
| tread2 | C (37.5,300,40) | (56.25,0,0) | stone |
| tread3 | C (37.5,300,50) | (93.75,0,5) | stone |
| tread4 | C (37.5,300,60) | (131.25,0,10) | stone |
| tread5 | C (37.5,300,70) | (168.75,0,15) | stone |
| tread6 | C (37.5,300,80) | (206.25,0,20) | stone |
| tread7 | C (37.5,300,90) | (243.75,0,25) | stone |
| tread8 | C (37.5,300,100) | (281.25,0,30) | stone |

Descending counterpart with run L, rise magnitude H and n=H/10: for i=1…n use C(L/n,300,H+20−10i), centered((i−0.5)L/n,0,(−10i−H−20)/2). Tops are −10i and every base is Z=−H−20, below the walking surface. Do not mirror the complete ascending solid or use negative scale. The authored run/rise endpoints and approach remain those in A-02. Map-owned smooth ramp gives collision; these cubes are only a coarse tread illustration, not a new step-up rule. Use the Up/Down overlay on the flat approach, not an enlarged trigger.

<a id="i03"></a>
#### I03 — Door leaves (conditional)

A-02 door basis: +X across opening, +Y into room; approach from −Y. Closed preview only; open apertures remain default. Leaves160×300×8, full pair320. Gameplay must own hinge/collision/open state.

No character capsule. Review box X[-160,160], Y[-8,4], Z[0,300]. Shared amber selection brackets; facing/label as stated above.

Palette: `wood` `#43352A`, `iron` `#535451`, `mark` `#C0AC86`.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| leaf-1 | C (160,8,300) | (-80,0,150) | wood |
| strap(-1, 70) | C (140,2,12) | (-80,-7,70) | iron |
| strap(-1, 230) | C (140,2,12) | (-80,-7,230) | iron |
| handle-1 | C (10,2,24) | (-15,-7,145) | mark |
| leaf1 | C (160,8,300) | (80,0,150) | wood |
| strap(1, 70) | C (140,2,12) | (80,-7,70) | iron |
| strap(1, 230) | C (140,2,12) | (80,-7,230) | iron |
| handle1 | C (10,2,24) | (15,-7,145) | mark |

<a id="i04"></a>
#### I04 — Chest (conditional)

Chest front +X with pale latch; optional until a persistent object/contents are authored. Label Chest · Loot or Chest · Empty from authority; no new map placement.

No character capsule. Review box X[-50,54], Y[-35,35], Z[0,68]. Shared amber selection brackets; facing/label as stated above.

Palette: `wood` `#43352A`, `iron` `#535451`, `mark` `#A9C4CC`.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| box | C (100,70,48) | (0,0,24) | wood |
| lid | C (100,70,16) | (0,0,59) | wood |
| strap-25 | C (100,8,3) | (0,-25,66) | iron |
| strap25 | C (100,8,3) | (0,25,66) | iron |
| latch | C (4,18,24) | (52,0,46) | mark |

<a id="i05"></a>
#### I05 — Corpse loot badge

Overlay beside retained family corpse. Two opposed cones form upright diamond; place on corpse front edge+24 cm only if clear, otherwise use label. Unknown corpse fallback: flattened neutral sphere90×60×12 atZ6 plus badge; do not substitute a live slime.

No character capsule. Review box X[-18,18], Y[-18,18], Z[0,78]. Shared amber selection brackets; facing/label as stated above.

Palette: `mark` `#A9C4CC`.

| Piece | Primitive / full size cm | Center or base → end cm | Material |
|---|---|---|---|
| diamond top | N 28 | (0,0,42) → (0,0,78) | mark |
| diamond bottom | N 28 | (0,0,42) → (0,0,6) | mark |

### Interactable placement and live states

| Binding | Placement/label policy | State geometry |
|---|---|---|
| I01 / I02 | Shared by eight directed ends of the four existing temple↔B1↔B2↔B3↔B4 pairs. Floor arrow on flat approach; +X points toward authored departure. Up label `Up to [destination]`, Down `Down to [destination]`. This says level direction, not world +Z. Never add a B4 descent. | Statically available only if owner permits travel. Pending/failed words from owner; no portal bloom or countdown. |
| S01–S04 | Use the exact A-02 run/rise/width for each map. No selection or separate label on decorative treads; associated I01/I02 is the selected travel identity. | Permanent geometry, no aggro/hit/dead/respawn cue. |
| I03 | Existing320 church aperture only; do not scale into240 or500 openings. Closed preview has clear pair seam and handles toward−Y. Label `Door · Open/Closed` only if an interactive door is adopted. | Each160 leaf hinges at X±160,Z0, rotates90° outward into reserved storage after authoritative open state. No invented lock, auto-close or enemy gate; validate swing/storage before enabling. |
| I04 | Optional room-edge chest with separately authored instance/content. Floor pivot, latch points+X; no source map chest symbol creates one automatically. | Lootable latch cool0.15 emission; empty latch iron #535451, label Empty. Open lid remains atrest until a reviewed hinge pose exists; status text carries the state. |
| I05 | Badge for owner-provided corpse contents, added to species corpse and preserving its name. Place at front extent+24cm only if visible clear floor; otherwise suppress geometry and retain label. | Loot available=diamond and Loot; empty/no loot=no badge and Dead/Empty. Dead body remains static, no autonomous cleanup or timer. |
| Save / SafeSpawn | No world presentation ID, beacon, shrine or save point. Keep reserved600×600 arrival boxes empty. Save UI owns Working/Saved/Error; neither corpse nor altar implies save. | No scene pulse for durable save until the session owner confirms; no save success animation specified here. |

### Integration handoff

Integrator adopts this table as data, including the new NPC/interactable IDs, without editing shared schemas in this task. W3-01/02 and W4-02/07 construct the primitives and bind actual selection/interaction/lifecycle signals; W4-04 owns respawn/loot. Retain known floor disputes (Dungeon Bat B2 provisional; Undead Bat uses selected B2), existing services and actor identities. Do not let the generic NPC body create an offer or let a decorative corpse create loot. Then run the Linux camera/collision/event checks listed above. W5 owners replace the Visual payload after the same capsule/identity/timing checks and clean-cook asset inclusion. All future asset paths, missing start/cancel delegates, geometry performance, world-label occlusion and distant service readability remain integration decisions.
