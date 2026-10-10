# W5-08b — scripted Blender creature pilot candidate

Task ID: W5-08b. Contract revision: 1.
Base revision: `b1bfe7273a98bfd1a5b3992b63a4ea879771a3df`.
Result revision: the attempt's Deliverable commit carrying this report; exact candidate is in the submission receipt and worker handoff.
Owned paths: `ArtSource/Creatures/`, this document. No tracked binary assets.

## Delivered approach

Original deterministic Python builds rat, bat and green slime, bakes procedural materials, creates skin weights and five actions, exports GLB, re-imports each file, and renders hero/gameplay/action evidence. The result is a reproducible **technical pilot**, with visual and rigging gaps below. Concept-match approval and production-readiness are not claimed.

Blender 5.2.2 LTS (`d13f752e3b9c`, Linux) was present at the assigned path. Step 0 ran factory-startup/background with checkout-local XDG config and rendered a 32×32, one-sample Cycles CPU frame successfully (exit 0). Startup printed `socket(): Operation not permitted` twice, but the render saved and Blender quit normally. No installation or system change was made.

Source: [builder](../../ArtSource/Creatures/build.py), [GLB inspection](../../ArtSource/Creatures/validate_glb.py), [re-import preview](../../ArtSource/Creatures/preview_exports.py), [rebuild instructions](../../ArtSource/Creatures/README.md). Seed 508 fixes the procedural fur placement in a full run. Reproduction means source-controlled construction, not a claim that rendered PNGs are bit-identical across hardware.

Geometry comprises authored ellipsoidal animal masses, short geometric fur tufts, tapered tail/toe/whisker tubes, tessellated thickened bat membranes with ribs, and a welded continuous asymmetric slime shell. Animal anatomy is intersecting game geometry rather than a watertight sculpt. All visible pieces are joined per creature and triangulated. The slime uses a ring-generated shell with a grounded skirt and a blended peak weight. Meshes have one atlas/material per creature. No downloaded assets, reference pixels, image-to-mesh tooling or external textures are used.

The concept sheet was opened at its package path as a visual target only. It was never copied, sampled, read by the generation scripts or included in outputs. Local sources consulted: the three creature specifications and shared README, `visual-monsters.md`, and plan art-pipeline §2. These supply authored presentation targets, not authentic mechanics. Explicit uppercase task ownership takes precedence over the general lowercase directory memory for this attempt.

## Meshes, materials and export

| Creature | Actual GLB triangles | Imported joints | Textures | GLB bytes |
|---|---:|---:|---|---:|
| Brown rat | 5,668 | 8 | base/normal/ORM, each 512² | 820,900 |
| Bat | 4,498 | 5 | base/normal/ORM, each 512² | 740,812 |
| Green slime | 3,456 | 2 | base/normal/ORM, each 512² | 332,016 |

Generated binaries stay in ignored `ArtSource/Creatures/output/`. The nested `.gitignore` also excludes local configuration and bytecode; the root ignore rules are unchanged. There are no `.blend` files, Unreal imports, `Content/` or `Source/` changes. Binaries are rebuilt from Python and intentionally absent from the commit.

Base color is a Cycles diffuse-color-only bake from procedural color ramps. Tangent normals are baked from procedural bump on the production geometry, without an external/high-poly sculpt. ORM is baked emission: AO=1, material roughness, metallic=0. AO is deliberately a neutral channel, **not baked geometric occlusion**. Rat fur is rough, skin less rough; bat leather is mid-rough; slime is glossy and opaque. These are prototype authored material responses. Slime transmission and embedded bubbles are not implemented. The GLB embeds all three PNGs, including the packed metallic/roughness channels.

Units: metres in Blender/glTF, +X forward and +Z up, applied mesh transforms, floor root. Future Unreal import must calibrate metre-to-centimetre conversion with a ruler mesh. No importer settings or game collision policy are changed by this task.

## Animation and measured bounds

All three exports contain idle (60f/2s), move (24f/0.8s), attack (42f/1.4s), hit (18f/0.6s), death (60f/2s), at 30fps. Attack has explicit f0, f20, f29, **f30 contact at 1s**, and f42 neutral recovery keys. Death transitions to its corpse pose and holds through f45–f60. These clips are presentation prototypes; no key or notify grants damage. All floor-root channels remain constant. In-place movement is authored without measured stride/speed agreement or start/stop clips.

The rat has body/head, four independent leg weights and one tail bone; the bat has body/head and one bone per wing; slime has root/peak blending. Weights are normalized and present in exported meshes. These are reduced pilot rigs, **not** the complete anatomical chains specified for production. There is no independently opening jaw, articulated finger chain, multi-segment tail curl, cosmetic socket set or runtime blending contract.

The builder samples every integer frame of every source action and records exact centimetre extrema in ignored `validation.json`. Final union of sampled clips:

| Creature | X min/max (cm) | Y min/max (cm) | Z min/max (cm) | Target envelope assessment |
|---|---|---|---|---|
| Rat | −67.44 / 22.44 | −13.39 / 13.59 | −0.12 / 27.66 | Fits 90×28cm horizontal box and 35cm live top; remaining death floor penetration of 0.12cm |
| Bat | −20.41 / 16.01 | −39.77 / 39.73 | 3.26 / 124.93 | Fits 45×80cm horizontal box and 145cm live top; corpse remains 3.26cm above floor |
| Slime | −42.64 / 43.06 | −40.98 / 44.00 | 0 / 45.61 | Fits 90×90cm footprint and 60cm live top; neutral peak approximately 40.72cm versus 40cm target |

Bounds are sampled geometry evidence, not continuous swept-clearance certification. Bodies do not fill every target envelope dimension: bat neutral top is about 123cm, not 145cm, and torso hovers near 100cm. No capsule sizes, navigation, collision, LOS, hit reach, rewards or server rules are changed. No new source-backed mechanics or gameplay tuning is introduced.

## Renders and feedback

Nine final 1280×720 Cycles CPU PNGs, 12 samples with denoise, were visually inspected. Names are `rat_hero.png`, `rat_gameplay.png`, `rat_actions.png`, `bat_hero.png`, `bat_gameplay.png`, `bat_actions.png`, `slime_hero.png`, `slime_gameplay.png`, `slime_actions.png`. They are copied to the attempt's worker-output `library/`; textures and GLBs are excluded there. Each contact sheet shows idle, move, attack f30, hit and terminal death samples. It shows representative poses, not temporal playback proof.

Hero views use orthographic three-quarter cameras. Gameplay views use a 12m boom, 55° down pitch, 45° yaw, 45° horizontal FOV and 90cm floor focus. The final bat/slime gameplay renders were made from re-imported GLBs via `preview_exports.py`; the rat gameplay render was refreshed by its final source rebuild at the same camera. The studio floor, broad warm key and cool fill are an art inspection setup, not dungeon lighting.

Feedback iterations shortened/tapered the rat face, exposed eyes and added whiskers/fur geometry; adjusted bat membrane edges/ribs and torso proportions; replaced a dome-like slime with a broad skirt and uneven raised mound; corrected an initial 24fps rat export; and repaired static sine-loop keys by adding quarter-cycle samples. Action sheets explicitly freeze evaluated geometry so later frame changes cannot reset earlier columns. Rat motion was refined to retain foot-floor contact during idle/move; bounds still reveal the tiny death-transition floor defect above.

Visual assessment, **not acceptance**:

- Rat reads as a low brown quadruped with a continuous bare tail at gameplay scale. It remains round and toy-like relative to the concept; fur tufts read coarsely rather than as dense directional fur. Snout, toes and limb anatomy need a stronger sculpt pass.
- Bat reads as a small brown flying creature with warm leathery wing planes. The foreshortened wing silhouette is too angular, ears/face too soft, and ribs less intricate than the concept. Reduced wing articulation cannot reproduce a natural flap/fold.
- Slime has the best broad material/silhouette correspondence: glossy green, asymmetric mound, skirt and collapsed puddle. It is too smooth and lacks the concept's smaller lobes, embedded bubbles, deep green cavities and partial translucency. Broad studio reflections overstate gloss relative to likely dungeon light.

At gameplay distance large masses and tail/wing/skirt cues survive; fine facial and fur detail does not. Review across dark dungeon backgrounds, grayscale, zoom/orbit extremes and actual selection-ring overlays remains open. No claim is made that the requested beautiful concept-match bar has been achieved.

## Checks actually run and limitations

Observed checks: Step 0 startup/render exit 0; multiple complete scripted bake/export/render/re-import runs, plus final rat refinement rebuild; re-import armature/skin/action assertions for all three; independent GLB validation for container integrity, 3–8k triangle budget, exactly five named animations, expected durations, normalized weights, non-static clips, constant root channels and three embedded 512² PNGs; Python syntax compilation; `git diff --check`; visual inspection of all nine render types and the concept target. The exporter emitted a shared texture-sampler warning; all authored textures use the same default sampler. No sampling failure was observed. Blender also warns that `Material.use_nodes` is deprecated for Blender 6; 5.2.2 is the tested version.

Checks not run: Unreal import/editor/cook/package/play are outside this Blender-only contract. Runtime evidence requires the future import task, unit/material calibration, animation binding and actual game camera/lighting. Anatomical skin review, continuous collision sweeps, terrain contact, LODs, crowded-room performance, grayscale/selection testing and full animation playback review remain unperformed. There is no required-update receipt pending for this attempt.

Known defects/remaining decisions: visual quality below concept target; reduced anatomy rigs and missing sockets/start-stop clips; rat death floor penetration; hovering bat corpse; neutral slime height slightly above 40cm; no geometric AO or translucency; no production LODs or gait metadata. Treat these as follow-up work rather than silently approving the pilot as final art.

Next task/integration notes: coordinator reviews the PNG evidence and chooses a creature-refinement pass before asset import. Preserve gameplay definitions, collisions and timing when integrating; bind contact to authority `ImpactSeconds`, not a damage animation notify. Rebuild rather than committing these generated outputs. This handoff is an evidence candidate and passes no project gate.
