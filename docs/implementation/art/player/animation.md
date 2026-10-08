# Human animation and retargeting contract

Companion to [player spec](README.md). Every count, frame, time, distance and budget is **Prototype (P)** under its provenance convention. This is a production proposal, not installed animations. No motion footage is supplied by the still reference set.

## Common rig and import

Author a floor-root humanoid hierarchy: root → pelvis → spine/chest → neck/head; paired clavicle/upper-arm/forearm/hand/fingers and thigh/calf/foot/toe chains. Preserve one hierarchy, bone names, neutral A-pose and socket contract for both body meshes. Mesh fit may differ; limb-chain lengths and neutral root remain shared in the minimum. The actual skeleton asset/name map is frozen by its assigned owner, not independently recreated per outfit.

The provisional export target is a tested Blender→Unreal skeletal FBX workflow using the project's installed engine version, with centimetre units and +X mesh forward as in A-01. Record Blender/exporter version, unit/axis conversion, rest pose, applied transforms, skeleton name map, normals/tangents and animation sampling in each production asset's import note. Do not assert universal FBX switches before a test export. A 100 cm cube and a 175 cm clothed human beside the R35/HH90 capsule are the import test; verify root at sole level, no duplicate armature root, no unit factor or yaw drift.

If the temporary mannequin's skeleton differs, define explicit source and target retarget rigs: pelvis retarget root, spine, head, arms, legs, fingers and foot/hand IK goals. Match reference poses first; test foot contacts and hand grips before batch retargeting. Do not delete bones from a licensed mannequin to meet the proposed deform-bone budget. Use a reviewed custom deform rig or record a budget exception. Accessories must use the approved human sockets, never a mannequin-specific accidental bone name. Bow/string is a separate small prop rig with its own nock and grip anchors.

Locomotion is in-place; root stays floor-centered and unanimated in translation. CharacterMovement controls movement and currently orients the actor to motion. Authored turns/start-stop clips are optional blends, not a new root-motion controller. An attack-facing adapter remains integrator work: face/aim the presentation toward the committed target without claiming the current input controller already does so. At stairs use bounded visual foot adjustment; no foot IK moves the capsule, teleports across treads or changes step eligibility.

## Required motion coverage

Preview authoring rate is **30 fps**, zero-based sample f0; durations below are P starting clips. Pose names and motion phases survive runtime retiming. Deliver clips for both body fits on the same hierarchy; weapon overlays may share lower-body motion.

| Clip / future name suffix | Preview length / keys (P) | Required read and integration boundary |
|---|---|---|
| `Idle` | 60 frames / 2.0 s loop | Quiet breathing, feet planted, readable empty-hand, dirk/shield, bow and cast-ready overlays. No attack cue while merely cooling down. |
| `Walk` / `Run` | Walk 30 frames / 1.0 s; run 24 / 0.8 s loops | Alternate contacts, no sliding at review speeds 220 / 450 cm/s. Equivalent stride displacement 220 / 360 cm per cycle informs gait; retarget/play rate is reviewed in-engine. |
| `Melee` | f0 commit; f1–29 anticipation; f30 contact; f42 settled | Draw dirk/right forearm back with modest torso rotation; at f30 extend the blade toward approved target. No translation lunge. Shield/open left hand clears torso and camera. |
| `Bow` | f0 commit; f1–15 nock/draw; f16–29 hold; f30 release; f42 settle | Left hand supports bow, right hand draws to cheek; nock/string/arrow aligned; release fingers at f30. Release timing is a future ability proposal, not proof of ranged hit timing. |
| `Cast` | f0 commit; f1–29 palm/forearm preparation; f30 release pose; f42 settle | Open-palm silhouette and restrained hand effect. Empty-hand version mandatory; staff overlay optional when equipped. Different spell effects do not invent new combat coefficients. |
| `Hit` | 9 frames / 0.3 s, peak f3 | Small torso/head recoil blended over locomotion or attack. It must not postpone a committed impact or create an unauthorized stun. Death overrides it. |
| `Death` / `DeadHold` | Fall 36 frames / 1.2 s, then held end pose | Controlled collapse within a 200×120 cm floor review box; no autonomous ragdoll blocker or loot creation. Authority controls life/collision and cleanup. Recheck corridor clearance separately from standing envelope. |
| `Interact` | 24 frames / 0.8 s, reach/apex f12 | Neutral reach/nod suitable for service/portal interaction. Cosmetic acknowledgment after accepted interaction; montage completion never commits purchase, travel or save. |

Keep fingers and face simple; named states matter more than flourish. Bake every required state and compatible equipment overlay before calling the set complete. Side/back views in the source are not motion frames; do not mirror an asymmetric equipped pose into a fabricated animation set.

## Explicit impact alignment

[Combat foundation](../../combat-foundation.md#pipeline), [LHBasicAttackAbility.cpp](../../../../Source/Lighthaven/Abilities/LHBasicAttackAbility.cpp) and [LHCombatComponent](../../../../Source/Lighthaven/Abilities/LHCombatComponent.h) establish an explicit timer after accepted native commit. `ImpactSeconds` is configuration; there is no production recovery-duration field. The [dev fixture](../../../../Source/Lighthaven/Framework/LHDevCombatFixture.cpp) supplies **1.0 s impact and 3.0 s cooldown**, inherited P, not original T4C timing. Reuse A-01's **f30 = 1.000 s** contact and **f42 = 1.400 s** cosmetic settle for this fixture only.

For runtime delay D, place the authored contact key at simulation commit time + D by adapting only anticipation. If D is zero, skip anticipation and present the immediate result. Do not change combat timing to fit the asset or time-stretch an entire loop so contact drifts. A long approved windup may hold a readable ready pose before contact; no repeated swings or repeated damage implications. Late visual binding seeks to elapsed action time, not a fresh windup. Sampling rate and montage notifies never own damage, RNG, mana, inventory or rewards.

Current `LHPlayerController::PlayerTick` suppresses player movement while `IsActionPending()`, in addition to the ability stopping velocity on commit. This is implemented at this base, despite older documents leaving it to the controls owner. The ability ends at impact and clears pending state: the remaining cooldown does **not** lock movement. Recoil blends out if movement resumes, another approved action starts, or death/travel/cancel occurs. There is no new recovery lock in this art proposal.

| Authority signal / missing seam | Required presentation response |
|---|---|
| Accepted commit: **no public start delegate exists yet** | Integrator supplies activation identity, target context, commit time and D. Start windup only here, never on raw attack button press. |
| `OnImpact(FLHHitIdentity, Result)` on attacker | Display resolved hit/miss feedback once per ActivationId + ImpactIndex. Result event is too late to start windup. It does not include a target ID; adapter retains committed target/life context for hit reaction. |
| Validation failure at impact | May emit no OnImpact; clear the old pose through end/cancel synchronization, with no invented hit flash or delayed callback. |
| `OnDeath` on victim | Cancel prior presentation, play death/hold once for that life. Other non-combat health changes require the lifecycle owner's signal. |
| Cancel, avatar replacement, travel, load | Integrator supplies end/cancel lifecycle notification; clear queued cosmetic cues, invalidate old activation/life context. A stale effect must not play on a new actor life. |
| Pause | Follow active simulation time, including held pose; no wall-clock release during pause. |

**Bow/cast limitation:** this foundation executes melee only. W4 must define commit/release/impact/cancel events for ranged and spells. For an explicitly instant-hit ranged prototype, f30 release may coincide with the approved authoritative impact. For a projectile, release and damage impact differ: f30 is the release clip key and target hit feedback waits for the actual authority event. Do not manufacture projectile travel, consume an arrow, assume every spell hits, or reuse melee's timer as an unreviewed spell rule. Cast release is likewise an effect-specific future contract. These coverage clips can be previewed in DCC while live integration remains outstanding.

## Review evidence W5-02 must supply

An import record, original/cleared source provenance, shared skeleton and retarget mappings; a neutral pose/front-side-back review; locomotion at actual speeds; equipment socket closeups; required animation clips and transitions; Linux gameplay captures at the controls camera and a frame/time trace showing commit → contact → resolved feedback. Run cancellation, target departure/LOS failure, miss, death, zero delay, pause, input resumption after impact and LOD changes during attack. Logs must distinguish a cosmetic key from the authoritative timer. No such runtime or DCC playback is claimed by A-02.
