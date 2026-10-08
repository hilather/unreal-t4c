# A-01 — Basement creature presentation specifications

Production handoff candidate, 2026-10-08, based on `efdec2d60b598bddd6cda14b9820393a4e245bd8`. These are written specifications for W4-02 and W5-03, not imported assets or observed gameplay. No runtime code, shared contract, source image or binary asset is changed.

## Coverage and evidence

The brief says “all eleven, plus Balork”; the [roster](../../../plan/contracts/enemy-roster.json) and [world matrix](../../world-ledger.md#eleven-definition-floor-coverage) actually contain **eleven IDs including Balork: ten ordinary definitions and one boss**. This package covers every exact ID once, plus a separate [Balork final-encounter companion](balork-final-encounter.md). It does not invent a twelfth gameplay definition. The companion is staging guidance for the same boss.

| Gameplay definition | Proposed presentation ID | Family | Selected floors | Spec |
|---|---|---|---|---|
| `Enemy.BrownRat` | `Presentation.Enemy.BrownRat` | Rat | B1–B4 | [Brown Rat](brown-rat.md) |
| `Enemy.Bat` | `Presentation.Enemy.Bat` | Bat | B1–B2 | [Bat](bat.md) |
| `Enemy.DungeonBat` | `Presentation.Enemy.DungeonBat` | Bat | B2 provisional; source floor unknown | [Dungeon Bat](dungeon-bat.md) |
| `Enemy.GreenSlime` | `Presentation.Enemy.GreenSlime` | Slime | B1–B4 | [Green Slime](green-slime.md) |
| `Enemy.GiantBat` | `Presentation.Enemy.GiantBat` | Bat | B2–B4 | [Giant Bat](giant-bat.md) |
| `Enemy.UndeadBat` | `Presentation.Enemy.UndeadBat` | Bat | B2 selected; B3 disputed | [Undead Bat](undead-bat.md) |
| `Enemy.GiantSpider` | `Presentation.Enemy.GiantSpider` | Spider | B2 | [Giant Spider](giant-spider.md) |
| `Enemy.Goblin` | `Presentation.Enemy.Goblin` | Goblin | B3 | [Goblin](goblin.md) |
| `Enemy.GoblinWarrior` | `Presentation.Enemy.GoblinWarrior` | Goblin | B3 | [Goblin Warrior](goblin-warrior.md) |
| `Enemy.Atrocity` | `Presentation.Enemy.Atrocity` | Atrocity | B3–B4 | [Atrocity](atrocity.md) |
| `Enemy.Balork` | `Presentation.Enemy.Balork` | Demon | B4 final encounter | [Balork](balork.md) |

Floor statuses are inherited from the world ledger, with secondary-source disagreements retained. Visual resemblance does not settle floor placement. Bible level/HP/melee values in the specs are documentary context from [ruleset-bible-v1](../../ruleset-bible-v1.md#monster-data-w4-02-not-a-w1-02-combat-coefficient) and [rules-ledger](../../rules-ledger.md), retrieved **2026-10-07 by R-02**, not fetched again by A-01. Monster-page historical patch/server is unspecified; a 2002 snapshot is not a version number. Live/classic conflicts remain disputed. No XP, loot, attack-speed or damage interpretation is newly authorized here.

Evidence labels used throughout:

- **Observed:** visible in an image opened by A-01 on 2026-10-08; the image does not establish unseen anatomy or dimensions.
- **Inherited:** values/status from the named local source, not a new network retrieval or play result.
- **Prototype:** authored proposal, `source_url: null`, baseline `LH_Prototype_v1`, author `A-01`, date `2026-10-08`. This applies to every new size, palette swatch, capsule, socket position, animation pose/frame, LOD threshold and budget below. W0-04/V-01 inherited proposals remain proposals.

## Units, root and clearance

Use Unreal centimetres, mesh forward **+X**, right **+Y**, up **+Z**, applied scale 1. Model root is at the floor under the body center. A Character actor origin is at capsule center: attach the floor-root mesh at local Z = **−capsule half-height**, then add bat visual hover through child bones. Validate import with a 100 cm cube. No image pixel-to-centimetre conversion is implied.

Each spec lists visible height above floor, full fore–aft length (including tail/equipment), lateral width, and upright capsule **radius R / half-height HH**. Capsule diameter is 2R; full height is 2HH, with HH ≥ R. Capsules enclose the blocking body, not all wings, legs or tails; some low creatures need a taller logical capsule than their visible body. This must be reviewed by W4-01/02 for navigation, selection and origin-to-origin LOS/range. Mesh sockets and limb lengths are not hitboxes or damage reach. Do not scale the whole actor to make a mesh fit.

The [layout specification](../../layout/README.md#grid-coordinates-and-construction) supplies these **unobstructed** minima, after frames, props and rails:

| Route | Door W × H | Corridor W | Stair W | Turn/landing square |
|---|---:|---:|---:|---:|
| Ordinary residents | 240 × 300 cm | 320 cm | 300 cm | 320 × 320 cm |
| Required full-spread boss routes | 500 × 360 cm | 550 cm | 550 cm | 600 × 600 cm |

Per-spec arithmetic compares visible width and 2R with route width, max(visible height, 2HH) with headroom, and a conservative horizontal bounding-box diagonal with the turning pad. It proves only that the proposed bounds fit the written dimensions when centered/aligned. It does **not** prove swept corners, slopes, contacts, simultaneous player passing or implemented navigation. Stair headroom is measured above local ramp/tread height, not the map's Z=0. Keep death and hit poses within the stated review envelope or flag a larger one. Tail/wing/weapon tips must clear walls even if the capsule passes.

W4 must review and adopt collision proposals before creating the art proxy. Once adopted, hold that capsule/nav-agent/selection policy constant across proxy → mesh → LOD swaps. A-01 does not resize the existing dev dummy. Small-target assistance belongs to controls/UI; enlarging a mesh or Visibility blocker silently would affect targeting.

## Camera and silhouette rules

Use the newer [controls specification](../../controls.md): focus at the player capsule center, approximately 90 cm above floor; boom **1200 cm**, bounded **900–1800 cm**; pitch **−55°**, bounded **−60…−50°**; yaw **45°**, bounded **0…90°**; **45° horizontal FOV** under current engine defaults. The older W0-04/V-01 **vertical** FOV proposal has not been adopted. These are inherited prototype controls, not observed art validation. Inspect at 720p and 1080p, both zoom and orbit extremes, and with boom collision shortening.

At that distance, retain large masses and negative spaces: rat back/snout/tail, bat membrane silhouette, spider radial stance, slime pooled edge, goblin polearm and pointed head, Atrocity hunched shoulders/yellow claws, Balork wing pattern/horns/polearm. Fine fur, teeth, individual bubbles, finger counts and tiny eyes must not carry identity. Show distinct variants together, in grayscale as well as torchlight; confirm selected-target name/ring and body remain readable. Variants need at least one shape/equipment cue and one broad value/marking cue, not only hue. Do not hide the target ring beneath opaque slime skirts, dark wings or corpses.

Retain the [art register](../../art-register.md#palette-and-materials)'s earthy, grounded stylization: matte fur/leather, rough skin/chitin, dull metal, restrained highlights, amber light with cooler fill. Swatches in specs are suggested base-color sRGB hex values, not measured source colors. Metallic = 0 for organic surfaces; only exposed metal uses a metallic response. Start opaque, including slime; two-sided bat/demon membranes may be needed, but avoid stacked translucent cards and emissive eyes as the sole readability solution. Rear anatomy and variant markings are authored reconstruction.

## Presentation IDs and replacement seam

`Presentation.Enemy.<ExactRosterSuffix>` is a **proposed case-canonical content namespace**, independent of asset path and revision. Never replace `Enemy.*` IDs, spawn/life GUIDs or reward identities with these strings. Shared bat/goblin meshes still have separate presentation records. Propose folders `/Game/Lighthaven/Art/Creatures/<Family>/`, meshes `SK_<Family>`, shared skeletons `SKEL_<Family>`, animations `AN_<Family>_<Action>`, material instances `MI_<Suffix>` and bindings `DA_Presentation_<Suffix>`; these names describe future work, not assets present here.

Current [`ULHDefinition`](../../../../Source/Lighthaven/Core/LHDefinitions.h) contains `Id` and generic `TSoftObjectPtr<UObject> Visual`; it has **no dedicated PresentationId field or typed presentation registry**. W4-02/integrator must approve the proposed mapping and implement a resolver/asset contract outside this task. Until then, this table is the handoff mapping, not loadable data. The approved binding should first resolve a project-authored primitive assembly, then a mesh/material/animation set, retaining the same logical presentation ID. Missing assets should retain the labeled family proxy and report the unresolved binding, not substitute another enemy. Clean-cook verification must prove soft references are included.

W5-03 owns the family assets after assignment. Shared skeleton/master material/animation Blueprint contracts remain integrator-controlled, with one writer per binary asset. No assets or leases are created here.

## Rig and attack timing contract

Use in-place locomotion and an unanimated floor root; CharacterMovement owns translation. Each rig needs idle, locomotion with start/stop blending, one basic attack, nonlethal hit, death and a dead hold. Bat hover/flight and slime deformation remain visual children of a ground-navigation root. Do not add true flight, poison, web attacks, summons, phase changes or extra hit callbacks from a reference pose.

The [combat foundation](../../combat-foundation.md#pipeline) resolves a **single explicit `ImpactSeconds` timer after native commit**, not an animation notify. The current [`LHDevCombatFixture`](../../../../Source/Lighthaven/Framework/LHDevCombatFixture.cpp) sets **ImpactSeconds = 1.0 s**, **CooldownSeconds = 3.0 s** (Prototype, W1-INT, 2026-10-08); combat-foundation's default synthetic fixture uses the same timing (some regression cases override it). There is no fixed production creature timing or recovery-duration field. The separate rules-ledger “one attack per 2000 ms” proposal is not this fixture's cooldown and is not adopted by these specs.

Author a **30 fps preview** with zero-based sample **f0 at commit**, readable anticipation through **f29**, and first contact/maximum extension at **f30 = 1.000 s**. Proposed visual recoil settles by **f42 = 1.400 s**, then returns to idle. This is a preview preset, not a frame-rate-driven timer. The native ability ends at impact; the remainder of the 3 s cooldown is not an animation or movement lock. Every species names its actual f30 pose below. No held-contact pose or multi-hit animation may imply repeated damage.

For approved runtime delay D, align the contact key to commit time + D using a windup section of duration D; do not change combat to match the artist's clip. For D=0, skip anticipation and present the immediate result. Recovery is cosmetic and interruptible. Do not time-stretch an entire loop so that its contact drifts. Pause follows active simulation. Death, cancellation, replacement activation and travel interrupt the old pose/effects; stale callbacks cannot replay feedback on the new life.

`OnImpact(FLHHitIdentity, Result)` (identity contains ActivationId and ImpactIndex) supplies resolved hit/miss feedback; it is not emitted for every failed validation. `OnDeath` supplies the lethal event. Neither should mint damage, loot or XP. There is currently **no public attack-start presentation delegate** in [`ULHCombatComponent`](../../../../Source/Lighthaven/Abilities/LHCombatComponent.h); the integrator must supply commit/cancel synchronization to animate windup reliably. Do not start on raw button press or use the damage event to start the windup. Movement suppression ownership also needs coordination: velocity stop alone does not prove input inhibition. Hit reactions must blend without postponing another already committed f30; death may cancel through existing authority. Sockets are cosmetic anchors only.

## Current dummy and proposed placeholders

[dev-maps.md](../../dev-maps.md) describes TargetPoints/discs awaiting consumers, but the current [generator](../../../../Source/LighthavenEditor/Commandlets/LHGenerateDevMapsCommandlet.cpp) already spawns `ALHEnemyCharacter` at each marker plus a separate capsule-sized nonblocking **cylinder ruler**. The enemy has a combat component and logged death; it has no creature mesh/animation/death presentation. Thus neither markers alone nor a cylinder ruler are an implemented species. Source inspection is the evidence here; no generated map was opened.

Each spec proposes an engine sphere/cube/cone/cylinder assembly using its own flat colors. Primitives are visual-only children, with collision disabled; the reviewed character capsule remains the sole blocker. These are future stand-ins, not created assets. No downloaded reference card is necessary; the original JPEGs are opaque and their black backgrounds are pixels. Replace only through the agreed presentation binding, preserving identity and authority.

## Proposed production budgets

These are A-01 starting targets within the plan's provisional ranges, not performance measurements. Triangles count the full visible creature including equipment; texture sizes apply to one family set of base color, normal and packed masks. Variants share the set and use parameters/masks where possible. Upper limits require profiling before multiplying residents.

| Family | LOD0 / LOD1 / LOD2 triangles | Texture set | Material slots | Deforming bone target |
|---|---:|---:|---:|---:|
| Rat | 5k / 2k / 700 | 1024² | 1 | ≤32 |
| Bat (four variants) | 6k / 2.5k / 800 | 1024²; Giant optional 2048² after review | 1 | ≤40 |
| Slime | 5k / 2k / 600 | 1024² | 1 | ≤12 or equivalent controlled morphs |
| Spider | 10k / 4k / 1.5k | 2048² | 1 | ≤48 |
| Goblin (two variants) | 14k / 6k / 2k | 2048² | ≤2, including polearm | ≤64 |
| Atrocity | 18k / 8k / 2.5k | 2048² | ≤2 | ≤64 |
| Demon / Balork | 40k / 18k / 6k | 2048² body + 2048² wings/weapon | ≤3 | ≤80 |

LOD0 is the import/close inspection mesh. Prototype switch trials: LOD0 → LOD1 below a **200 px** projected bounding-box major axis; LOD1 → LOD2 below **80 px**, with a **10%** hysteresis band. These are review measurements, not ready-made Unreal ScreenSize values; the importer must calibrate the actual projection/FOV. Small species may normally use LOD2. Preserve wing outline, polearm and broad variant marks at every level; do not cull a living attackable creature based solely on these thresholds. Collision, target cues, sockets and authoritative impact delivery must survive LOD changes; animation throttling cannot own the timer.

## Images actually opened and rights

Read-only root **P** = `/home/brewerm/Downloads/lighthaven-unreal-design-and-agent-waves/lighthaven-plan`. A-01 opened all nine files below individually at original detail on 2026-10-08. No crops, copies, generated images or image edits were made. Hash/provenance authority is the retained [manifest](../../../plan/assets/asset-manifest.json); original visual observations are in each spec.

| ID | Path beneath P | Source / concept relation |
|---|---|---|
| rat-family | `assets/references/monsters/rat-family.jpg` | Fantasy Classic `app20003.jpg`; starter concept input |
| bat-family | `assets/references/monsters/bat-family.jpg` | Fantasy Classic `app20002.jpg`; starter concept input for all four bat definitions |
| goblin-family | `assets/references/monsters/goblin-family.jpg` | Fantasy Classic `app20001.jpg`; deep concept input for both goblins |
| giant-spider-family | `assets/references/monsters/giant-spider-family.jpg` | Fantasy Classic `app20007.jpg`; deep concept input |
| green-slime-family | `assets/references/monsters/green-slime-family.jpg` | Fantasy Classic `app20005.jpg`; starter concept input |
| atrocity-family | `assets/references/monsters/atrocity-family.jpg` | Fantasy Classic `app20026.jpg`; deep concept input |
| balork-demon-family | `assets/references/monsters/balork-demon-family.jpg` | Fantasy Classic `app20013.jpg`; deep concept input |
| starter-characters-and-creatures-concept | `assets/concepts/starter-characters-and-creatures-concept.png` | Generated 2026-10-07; rat/bat/slime and two humans |
| deep-dungeon-creatures-concept | `assets/concepts/deep-dungeon-creatures-concept.png` | Generated 2026-10-07; spider/goblin/Atrocity/Balork |

All seven originals: [source page](https://t4cfantasy.com/Bible/Classic/MonsterImages.php), direct image prefix `https://t4cfantasy.com/images/skins/`, retrieval **2026-10-07**, host baseline **Fantasy Classic; original artist/client patch unspecified**. Each spec also gives its exact direct URL. Concepts are generated proposals per `P/assets/concepts/prompts.md`; tool model/version and underlying rights are not established. **No commercial licence or distribution permission is established for any reference or concept.** AI-derived details are not automatically cleared. Use these images for internal study; final original/licensed assets need a recorded rights review before inclusion in a distributable cook.

## Handoff checks to run after assets exist

1. W4-02/integrator approves IDs, binding type, capsules and timing profiles; resolves commit/cancel presentation events. Maintain distinct definitions and floor uncertainty.
2. W5-03 imports one family at a time, checks units/root/sockets/LOD/material costs, all motion coverage and the f30 preview contact. Keep shared assets under a single assigned writer.
3. W3/W4 walk each intended doorway and stair both ways, turn at pads, test player passing, LOS and hit poses. Balork additionally needs his full B4 extent, weapon sweep and retreat boundaries checked; he fails ordinary routes by design.
4. Review all variants together at actual controls camera settings, dark/warm backgrounds, grayscale, zoom/orbit extremes and low ceiling boom shortening. Capture actual Linux editor/game evidence; Windows checks remain deferred.
5. Fixed gameplay fixture across art swaps: same capsule, timing, damage/reward outcome and identity; missing-reference fallback, cancellation/death and clean cooked asset audit. Profile a crowded room before adopting budgets.

A-01's checks are textual coverage, local image inspection, provenance/hash comparison and dimension arithmetic. Import, animation playback, collision/nav/selection, performance, editor/cook/package/play and rights clearance remain unperformed; this document passes no project gate.
