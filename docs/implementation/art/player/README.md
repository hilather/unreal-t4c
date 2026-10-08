# A-02 — Human player production specification

Handoff candidate for **W5-02**, dated 2026-10-08, against `cd6e73b139e117f779e1aafa5c4d4bdf45f6ecec`. This package specifies future assets; it creates no mesh, skeleton, animation, gameplay change or binary lease. W5-02 still requires the combat/art review dependencies in [Wave 5](../../../plan/docs/05-agent-waves.md). See [animation and retargeting](animation.md), [opened references and rights](references.md), and the companion [environment kit](../environment/README.md).

**Numerical convention:** every design quantity in this package—dimensions, variant counts, colors, material values, texture sizes, frame timings and budgets—is a **Prototype proposal (P)**, `LH_Prototype_v1`, author `A-02`, date `2026-10-08`, `source_url: null`. Values attributed to controls, A-01, W0-04 or V-01 are **inherited P**, not new historical measurements or permission to change those systems. Dates, revision strings, source filenames and observed image metadata are identifiers/evidence, not game tuning. No authentic numeric mechanic is introduced.

## Baseline and production boundary

Read [art pipeline](../../../plan/docs/04-art-pipeline.md), [game design](../../../plan/docs/01-game-design.md), [art register](../../art-register.md), [A-01 conventions](../creatures/README.md), [controls](../../controls.md), [creation UI](../../ui/character-creation.md), [rules ledger](../../rules-ledger.md), [combat foundation](../../combat-foundation.md) and the [layout package](../../layout/README.md). The code at this base takes precedence over stale implementation descriptions; differences are recorded below.

Grounded human adventurers, coarse linen, leather belts/boots, restrained dull metal. Read at the elevated camera through head/shoulder mass, hand pose, weapon outline and broad cloth value. The tiny original male/female images support different human body silhouettes and directions. The starter concept supports a modest clothed interpretation. Neither proves a face/hair/skin catalog, exact anatomy, equipment dimensions or material response. D4O promotion supports mood only; its ornate plate and purple robes are not compulsory starter gear.

W5-02 owns assigned human meshes, fitted clothing/equipment variants and animations. The integrator assigns a single writer for shared skeleton, AnimBP, presentation resolver and master materials before asset work. NPC reuse is a future compatible consumer, not permission to overwrite NPC assets. Changes to Core, UI, rules, maps or equipment legality remain outside this task and require their owning task; this spec only describes the requested seam.

## Body scale, origin and camera

| Item | Prototype specification | Basis / use |
|---|---|---|
| Standing body | Both base bodies 175 cm sole-to-crown; neutral shoulder width A 46 cm, B 42 cm; no creation height slider | W0-04 ruler; widths are A-02 sculpt starts, not sprite measurements. Outfit/hair idle top ≤180 cm. |
| Gameplay capsule | R=35 cm, HH=90 cm; diameter 70 cm, total height 180 cm; actor scale 1 | Current [LHCharacter constructor](../../../../Source/Lighthaven/Framework/LHCharacter.cpp). Identical for all cosmetics/equipment. Do not resize it to enclose a sword, bow or robe. |
| Mesh basis | Centimetres, +X forward, +Y right, +Z up; floor root under pelvis at origin; applied scale 1 | A-01 convention. Validate beside a 100 cm cube. |
| Proposed attachment | Floor-root skeletal mesh local translation `(0,0,-90)` cm on centered capsule, neutral facing +X | Not configured by the current constructor. Normalize export orientation or use a documented component rotation; never bake a compensating tilt into the animation root. |
| Locomotion | In-place at 0 / 220 / 450 cm/s idle/walk/run blend samples | Current [LHCharacter.h](../../../../Source/Lighthaven/Framework/LHCharacter.h); CharacterMovement owns translation. |
| Camera | Focus capsule center ≈90 cm above floor; boom 1200 cm, bounds 900–1800; pitch −55°, bounds −60…−50; yaw 45°, bounds 0…90; FOV 45° **horizontal** | Current controls/code and A-01; older W0-04/V-01 vertical FOV is not adopted. |
| Review | 1280×720 and 1920×1080, near/default/far zoom, orbit/pitch extremes and collision-shortened boom | P review targets. Creation preview may frame more closely but must use the same body, outfit and equipment state. |

Use the common rig and equal leg length for both bodies; alter torso/hip/face surface shape without altering movement speed or collision. Do not interpret A/B as mechanical sex, stats or classes. The original filenames remain male/female in the reference record; display labels can be “Body A / Body B.” Preserve ordinary layout apertures of 240×300 cm, corridors 320 cm, stairs 300 cm and turns 320×320 cm after dressing. Idle/travel gear target envelope is 100 cm wide ×100 cm fore–aft ×180 cm high, centered on root; larger attack/death poses need separate clearance checks. This is a visual review envelope, not a new target volume.

## Minimum appearance catalog

**Evidence minimum:** the original set shows the same male body in eight directions and the same female body in eight directions. It does not supply eight variants per body or recoverable facial detail. A strictly source-evidenced stand-in therefore has two body presets, each with a fixed interpreted head, hair and skin. No independent customization count is authenticated.

**Small production proposal:** two body shapes, a bundled face for each, two hair shapes, three skin palettes and one modest starter outfit. Hair/skin alternatives are explicitly authored extensions needed to populate the planned selectors. This is twelve body/hair/skin combinations, not twelve supplied reference characters. All combinations below must be fitted; defer extra faces, dyes, body morphs and sliders.

| Category / proposed presentation ID | Artist deliverable | Compatibility / evidence |
|---|---|---|
| `Presentation.Player.Body.A` | Broader torso/shoulder human surface, same rig height | Interprets `human-male-1`…`8`; exact build is P. Bundles Face.A by default. |
| `Presentation.Player.Body.B` | Narrower shoulder/waist human surface, same movement ruler | Interprets `human-female-1`…`8`; exact build is P. Bundles Face.B. |
| `Presentation.Player.Face.A` / `Presentation.Player.Face.B` | One neutral, original face per corresponding body; ordinary eyes/nose/mouth, no facial animation dependency | P reconstruction. Fixed by body in minimum UI; no separate face selector is claimed. |
| `Presentation.Player.Hair.Cropped` | Compact dark-brown short hair mesh with broad clumps | Original silhouettes suggest short head hair; sculpt/detail is P. Fits both heads. |
| `Presentation.Player.Hair.Tied` | Compact tied-back style, no long trailing cards | Starter concept suggests tied hair. P extension, fits both heads; not an original-client option. |
| `Presentation.Player.Skin.LightWarm` | Starting base-color swatch `#BD8E72` | P unlit sRGB starting color, not sampled skin evidence. Fits both bodies/faces and all exposed regions. |
| `Presentation.Player.Skin.MediumWarm` | `#8B5A40` | P original palette extension with same shader and texture detail. |
| `Presentation.Player.Skin.DeepWarm` | `#51362C` | P original palette extension; review facial/hand separation under cool fill and amber light. |
| `Presentation.Player.Outfit.StarterLinen` | Modest tunic, trousers, boot/wrap silhouette and belt, fitted to both bodies | Starter concept informs linen/leather masses; pattern/cut/back view reconstructed. Always retains a modest base underlayer. |

Body selection atomically chooses its matching face and fitted outfit pieces; both hairs and all skin palettes remain available. Hair initially uses `#30251D` (P); hair color is not another selector. One starter-appearance option is valid: show its label without cycling nonexistent content. Body/face/hair/skin are cosmetic only. Outfit.StarterLinen is the fallback clothing appearance, not an armor item or stat grant; visibly replace its relevant layer only when authority reports actual equipped gear.

The creation UI already defines body, hair, skin and starter-appearance rows. A body preset expands its bundled face in the proposed validation adapter; do not add a fifth face row in this task. Example logical selection: Body.B + Face.B + Hair.Cropped + Skin.DeepWarm + Outfit.StarterLinen. Require one compatible ID per category, reject duplicate/conflicting categories, and preserve the unsubmitted form on validation failure. No compatibility resolver is currently implemented.

`AppearanceIds` is a generic array of content IDs in [creation commands](../../../../Source/Lighthaven/Core/LHCommands.h) and [save snapshots](../../../../Source/Lighthaven/Core/LHSaveSnapshot.h). The proposed dotted `Presentation.Player.*` namespace reuses A-01's convention; it is not an approved enum or asset path. The integrator must freeze catalog/validation/versioning. Missing loaded visuals use a labeled modest proxy while retaining canonical saved IDs; do not silently rewrite a save to the default appearance. Invalid new choices fail validation before creation commits.

## Equipment layers and socket contract

Canonical slots in [LHDefinitions.h](../../../../Source/Lighthaven/Core/LHDefinitions.h) are `MainHand`, `OffHand`, **`Quiver`**, `Head`, `Torso`, `Legs`, `Feet`, `Accessory` (`Unspecified` is a sentinel). Physical hands and these logical slots are independent. No glove, cape, belt or shoulder gameplay slot is added: those are fitted sublayers of an existing item's presentation.

Sockets below are proposed skeleton/mesh anchors. Author them by posed grip landmarks, not universal world coordinates. In reference pose place the socket origin on its contact surface. Held-prop convention: +X from grip toward working tip, +Z toward back of hand/shield top, +Y completes the basis. Store a fitted relative transform per prop; export transforms applied, never stretch fingers to compensate for a bad pivot. Resolve bone names against the adopted skeleton once and keep socket names stable on both bodies/LODs.

| Logical slot / layer | Proposed socket or binding | Required behavior |
|---|---|---|
| MainHand, melee | `Socket.Weapon.R` on right hand palm grip; prop `Socket.Tip` on dirk tip | Short one-handed dirk silhouette. Tip is cosmetic only, never a hitbox or trace-based damage source. |
| MainHand, bow | `Socket.Bow.L` on left palm; bow asset `Socket.Nock` at string center, `Socket.Grip.R` at draw anchor | A bow still occupies MainHand while visually held left. Right-hand draw IK follows bow/nock; no phantom second weapon. |
| OffHand, shield | `Socket.Shield.L` on left forearm shield grip; optional `Socket.OffHand.L` on palm for another approved item | Shield follows forearm, clears elbow. Torch/offhand variants are alternatives, not simultaneous attachments. |
| Quiver | `Socket.Quiver.Back` on upper spine, behind right shoulder; quiver asset `Socket.ArrowPickup` at mouth | Separate visible quiver plus strap overlay, independent of bow mesh. Keep hair and neckline clear. Default top below head crown in travel pose. |
| MainHand, optional staff | `Socket.Weapon.R`, support hand IK authored per staff | Asset coverage for future equipped staff only; magic does not require or grant one. Empty-hand cast is mandatory. |
| Head | `Socket.Head` for rigid headgear; skinned hood/hat at matching head bones | Headgear declares hair hide-mask. Leave hair root/skin closed beneath rim; no bald-hole exposure when LOD changes. |
| Torso | Skinned tunic/leather/robe over shared humanoid rig | Torso mesh includes optional sleeves, shoulder pieces and gloves; owns corresponding body hide-mask. Belts/capes are sublayers, no independent authority slot. |
| Legs | Fitted skinned trousers, or leg layer under torso robe | Use common waist seam; torso long-hem mask hides overlapping trouser surface without deleting canonical leg equipment. |
| Feet | Skinned boots/wraps, consistent ankle seam | Feet stay at floor; no mesh-scaled high heel variation. |
| Accessory | `Socket.Accessory` at chest for a pendant, or no visible mesh | Keep a neutral optional anchor. A ring need not grow into a readable world prop; do not invent additional ring slots. |
| Cosmetic feedback | `Socket.Cast.R` / `.L` at palms; `Socket.Hit` at sternum; `Socket.Nameplate` above head | Cosmetic origin only. Source/target authority decides results; overhead UI should remain readable independently of pose. |
| Optional stow | `Socket.Stow.HipL` for dirk; `Socket.Stow.BackL` for bow | Only show if integration supplies a stowed state. Quiver stays on Back; no duplicate drawn/stowed meshes. |

P asset rulers: dirk 45 cm total with 12 cm grip; round shield 55 cm diameter, 8 cm depth; bow 140 cm tip-to-tip in rest pose; quiver 55 cm long ×18 cm diameter; display arrows 65 cm; optional staff 160 cm. These sizes establish authoring starts, not named item's historical shape or attack range. Quiver arrows are a constant small decorative group: the inherited [rules-ledger quiver candidate](../../rules-ledger.md) is reusable, so a release must not progressively empty it or consume inventory. No extra accuracy/damage/encumbrance data is inferred.

Use a proposed grip/fit record for each equipment presentation, keyed by content ID rather than filename. Suggested IDs are `Presentation.Equipment.RustedDirk`, `.AshwoodFlatbow`, `.WoodenArrows`, `.Shield.Plain`, `.Armor.Cloth`, `.Armor.Leather`, `.Staff.Plain`. Named item identity for the first three is inherited; exact geometry is P. Plain shield/leather/staff are art review samples pending data approval, not new item definitions or guaranteed starter grants. `ULHDefinition::Visual` is currently only a generic soft object reference; a typed equipment/presentation binding is still integrator work.

Only successful canonical equip/unequip changes the shown loadout. On an authority-approved bow loadout, the offhand presentation follows the authoritative slot/conflict result: never hide an equipped shield to simulate a valid bow build or decide legality in the AnimBP. W4 defines legal combinations; reject/flag unsupported visual combinations until reviewed. Item-instance IDs and quiver slot identity remain intact across art replacement.

## Starter route readability

These are **review assemblies from supplied equipment state**, not creation classes or alternative free kits. The same human can change route through normal gear/training.

| Assembly / proposed review ID | Silhouette and broad material cue | Required variants |
|---|---|---|
| `Presentation.Player.Review.Melee` | Short right-hand dirk, open left hand by default; optional authority-equipped shield gives a separate circular mass. Linen `#B5A58A`, leather `#55402C`, dull steel `#747571` (P). | Dirk alone first; shield overlay second; cloth and leather fitted variants. Do not use the promo's ornate plate as starter equipment. |
| `Presentation.Player.Review.Ranged` | Tall bowed outline at left side, right draw elbow, quiver/fletching above right shoulder. Restrained olive cloth accent `#62634A` (P). | Bow with separate quiver visible from front-quarter and rear; invalid/missing quiver reported by UI/authority, no conjured arrows. |
| `Presentation.Player.Review.Magic` | Open palm cast and quiet short-lived hand light; optional fitted robe hem gives a wider cloth base. Desaturated slate cloth accent `#5E6973` (P). | Empty hands first, then legal melee/offhand/staff overlays if approved. No staff, robe or learned spell is granted by choosing an appearance. |

Keep the outfit palette subdued so the church's red aisle and enemy cues remain distinct. Shape/pose must distinguish assemblies in grayscale; hue alone is insufficient. Eye glow, ornate embroidery and large capes are outside the minimum. A robe must clear legs at full run and keep the feet/target ring readable. Default to skinned cloth with authored folds; cloth simulation is optional after profiling and must not determine collision.

## Rig, materials and budget

Use the [animation contract](animation.md) for impact timing and common skeleton/retargeting. Deliver one shared humanoid hierarchy with two fitted body meshes, shared pose-compatible equipment, and an in-place animation set. Proposed asset names: `SK_Human_A`, `SK_Human_B`, `SKEL_Human`, `ABP_Human`, `AN_Human_<Action>`, `MI_Human_<Material>`; folder `/Game/Lighthaven/Art/Player/`. These are future assets. W5-02 must agree the skeleton with the integrator before any NPC or equipment writer depends on it.

| Budget (all P) | Target / ceiling |
|---|---|
| Complete equipped player, LOD0 / LOD1 / LOD2 | ≤40k / 20k / 8k triangles, including hair, clothing and visible weapons/quiver. No separate uncapped equipment budget. |
| LOD0 planning split | 18k visible body/head, 5k hair, 10k outfit, 7k all held/stowed gear; redistribute inside total when covered body regions are hidden. |
| Texture sets | Body/head shared 2048²; clothing shared 2048²; shared starter-equipment atlas 2048²; hair 1024². Each set has base color, normal and packed masks; skins/body variants share these. No 4K default. |
| Material sections | ≤6 visible draw sections total across body/hair/outfit/gear; shared opaque master families. Treat slot count as a profiling target, not a draw-call guarantee. |
| Rig | ≤80 deforming bones, ≤4 weights per vertex as an authoring target; preserve required adopted skeleton/helper bones separately. No physics-driven hair or fingers required. |
| LOD trial | Same A-01 review thresholds: bounding-box major axis below 200 px → LOD1, below 80 px → LOD2, 10% hysteresis. Calibrate actual Unreal screen-size settings; these are not import values. |
| Materials | Organic/cloth/leather metallic 0, bare steel metallic 1; roughness cloth 0.8, leather 0.65, skin 0.6, dull steel 0.55. P starting values; skin tones must not change roughness to fake brightness. |

The six-section target assumes body/head share one section, hair one, the supported fitted outfit combination one, and up to three gear sections. Author composite outfit meshes for the minimum supported cloth/leather/robe combinations, or have the integrator approve a merge strategy before adding independent torso/leg/boot components. Sharing a material across separate components does not make them one section. Quiver and its fixed decorative arrows share a prop mesh/section; a briefly drawn arrow still counts in the complete loadout budget. If a required legal combination exceeds this target, record and profile the exception rather than hiding equipped items.

Prefer opaque modeled hair clumps; masked wisps only after camera/overdraw review. Use the shared packed-mask convention R=ambient occlusion, G=roughness, B=metallic for skin, hair, cloth, leather and metal; document per-channel import settings. Source pictures are not base-color textures and generated lighting is not a normal/roughness map. Keep equipment silhouette, sockets, masks and body seams consistent across LOD swaps. Occluded skin can be masked by outfit region; remove neither head/hand attachment bones nor canonical equipment state.

## Placeholder and replacement path

The current native player selects no mesh, AnimBP or skeleton. There is no already-present animated human proxy to claim as delivered. Next-wave proposal: use an already available, rights-recorded engine/template mannequin with its compatible locomotion clips, normalized as a mesh child to this capsule/height. Do not download a new asset pack for this task. If unavailable, use a project-authored primitive torso/head/limb assembly and explicitly list missing motion states. All visual components have collision disabled; retain the actual Character capsule.

Flat linen/olive/slate accents plus geometric dirk, bow arc and quiver cylinder make the proposed review assemblies distinguishable. Label the review mode “Prototype”; do not imply the colored mannequin implements finished body/hair/skin choices. No source sprite needs to be cooked. Invalid or missing presentation assets retain the modest proxy and a diagnostic while preserving IDs.

Replacement sequence: integrator freezes catalog and resolver → W5-02 imports shared skeleton and base clothed human → validates bodies/attachments/retargeted locomotion → binds the explicit combat timing adapter → fits each equipment combination → replaces resolver soft references without changing AppearanceIds, item-instance IDs, capsule, speed or rules. Registry/asset-manager cook inclusion must be deliberate; filename renames do not rename the logical IDs. Proxy and final mesh must run the same authoritative fixture and produce the same results.

## Review still required

- Inspect both bodies with every hair/skin choice, headgear hide-mask and each review assembly in creation and gameplay views; test save/reload, missing binding and invalid combinations through the eventual validator.
- Walk/run, turn and stop through the environment's ordinary door/stair test kit; inspect hands, feet, belt/robe/boot seams, bowstring, quiver/hair and shield intersections at every LOD.
- Observe committed melee contact at the supplied impact delay, cancellation, zero delay, hit/miss, death, pause, travel and interruption. Bow/cast require their own ability contracts before live playback can be accepted.
- Review on gray/brown floors under daylight, cool fill and amber torches, in grayscale and at both output resolutions; profile complete equipped costs and crowded-room shadows.
- Perform Linux import/editor/cook/package/play checks after assets and presentation adapters exist; Windows checks remain deferred. No such checks were run by A-02 and no gate is passed by these documents.
