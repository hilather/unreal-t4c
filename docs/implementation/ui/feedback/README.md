# V-04 — In-world feedback

**First-pass visual proposal, 2026-10-08; contract revision 1.** Base `c132a859497eada23246c986d4de35358b5b62dd`. This is a specification for W1-01 targeting and W4-05 feedback, with W4 combat, interaction and lifecycle owners. It implements no widgets, components, assets or mechanics. The SVGs are original text drawings, not Unreal captures. Linux is the first runtime review platform; Windows checks are deferred. No build, play, accessibility or performance gate is passed by this package.

**Every design number is Prototype (P)**, including dimensions, thresholds, timings, caps, colour choices and budgets: author V-04, profile `LH_Prototype_v1`, date 2026-10-08, `source_url: null`. Inherited A-03/V-03/A-04/controls values remain P. Ratios and projection sizes are arithmetic on those proposals, not measured pixels in Unreal. Mock bar lengths represent geometry, not HP. Dates/revisions identify evidence. No authentic T4C HP, damage, mana, XP, loot or respawn value is introduced; no new mechanics research is needed.

| Mock sheet | Read with |
|---|---|
| [Selection and six floor samples](selection.svg) | Selection states and contrast below |
| [Nameplates, zoom and crowd limits](nameplates.svg) | Plate admission, placement and priorities |
| [Impact and lifecycle](combat-lifecycle.svg) | Event timing, text and lifecycle rules |
| [Interaction and resource cues](interaction.svg) | Prompts, generic glyphs and low resources |

Font family follows V-03 (Source Sans 3, Liberation Sans, sans-serif); local renders use Liberation Sans and do not prove cooked font coverage. Sheets use a 1920×1080 viewBox; inspect at native 1920×1080 and 1280×720, not a fit-to-window thumbnail. They are diagrammatic crops; only the explicitly labelled zoom construction compares projected world size. This README governs behavior where a sheet shows only one state.

## Inputs and deliberate reconciliations

Read against [V-02 HUD](../hud.md), [shared UI](../README.md), [V-03](../style/README.md), [A-03](../../art/placeholders/README.md), [A-04](../../art/lighting/README.md), [controls](../../controls.md), [combat foundation](../../combat-foundation.md), and [D14 presentation synchronization](../../schema-rev1-freeze.md). Existing first-pass art direction is inherited from the brief; this additional package remains reviewable tuning.

The runtime [input factory](../../../../Source/Lighthaven/Input/LHInputConfig.cpp), [controller](../../../../Source/Lighthaven/Framework/LHPlayerController.cpp) and [controller header](../../../../Source/Lighthaven/Framework/LHPlayerController.h) take precedence over stale illustrative V-02 bindings:

- **Tab / R shoulder** next target; **Q / L shoulder** previous; **F / Right-stick press** clear target. This is not a lock toggle. Left click selects; hover never commits selection. Right mouse / R trigger attacks; E / South interacts. Wheel / D-pad up/down zoom. C / North opens Character, I / Journal button opens Inventory, Escape / Menu button pauses. Use current remapped semantic bindings in actual widgets.
- Do not display Shift+Tab, D-pad context cycling, R3 zoom chords, LT quick selection or 1–6 as working controls before their owner implements them. The V-02 six-slot region remains reserved, without a false operational hint.
- Controls prose says selection range 2000 cm; **source default is 200 cm** (`SelectionRange=200.f`). Both are inherited prototype facts, not a V-04 tuning decision. Read configured targeting availability from the controller; never hard-code either number in UI or use camera distance as attack range.
- Current filtering excludes dead, obstructed and out-of-range actors and silently clears invalid selections each tick. Generic provider candidates also require explicit reachability. The enemy branch instead uses `ValidateAttack`; only None/ActiveAction/Cooldown/InsufficientMana are tolerated, and provider navigation reachability is not checked there. Enemy navigation reachability remains an integration gap. An unavailable-target drawing below does **not** authorize keeping an invalid target selected or making corpses attackable. Corpse interaction needs a separate owner-approved candidate path.
- A-03's 27/18 px primary name type is below V-03's functional minimum. V-04 proposes **30/20 px** primary names/values/actions, retaining 24/16 metadata. This explicit readability refinement needs integrator adoption. Ground amber corners and opaque plates remain A-03's direction.

## Selection geometry and states

Use an **open ground ring made of four corner brackets**, not a filled disc. Nominal centerlines follow A-03: capsule square corners `(±(R+12), ±(R+12), 2)` cm above floor, legs 20 cm long inward, nominal width 4 cm and height 2 cm. The selected actor alone retains A-03's +X facing pointer. Decorative wings, weapons and the corpse loot diamond do not change the capsule, target volume or range. Props use their A-03 footprint bounds expanded 12 cm on each side; no extra ring on decorative stair treads.

For readability, propose a projected core stroke at least **3 px at 1080p / 2 px at 720p**, with **3/2 px dark keyline on each side**, giving minimum total thickness 9/6 px. Keep centerlines and leg endpoints anchored to the nominal world corners; only cosmetic stroke thickness can grow in screen space. No hitbox growth or constant-screen-size ring radius. Short projected legs rely on the plate instead of enlarging the animal. Use butt ends so corners/gaps remain distinguishable. Out-of-range legs have one centered gap; surviving segments must each be at least 3/2 px, otherwise use solid corners plus the labelled range icon.

| A-03 actor | Capsule R / HH cm | Bracket half-extent R+12 cm | Living anchor above floor cm |
|---|---:|---:|---:|
| Brown Rat | 25 / 25 | 37 | 70 |
| Bat | 25 / 75 | 37 | 170 |
| Dungeon Bat | 30 / 80 | 42 | 180 |
| Giant Bat | 45 / 100 | 57 | 220 |
| Undead Bat | 30 / 85 | 42 | 190 |
| Green Slime | 40 / 40 | 52 | 100 |
| Giant Spider | 55 / 55 | 67 | 130 |
| Goblin | 35 / 70 | 47 | 220 |
| Goblin Warrior | 40 / 75 | 52 | 235 |
| Atrocity | 65 / 100 | 77 | 230 |
| Balork | 100 / 155 | 112 | 330 |
| Player and human NPC | 35 / 90 | 47 | 200 |

Anchors conservatively use A-03 review tops for tall carried pieces: `max(visual top, 2HH)+20`; actual rest bounds may permit a lower anchor after review. Goblin/Warrior tops include polearms. No animation-frame bounding-box jitter. Corpse ring keeps its original footprint; flattened small corpse anchor is Z40 without badge or Z98 with the 78 cm badge, raised for larger corpse bounds as needed. Props use top+20.

| State | Ground / visible-surface cue | Opaque plate and fixed HUD response |
|---|---|---|
| Hover, valid, not selected | Four `HoverEdge #C4C4C4` corners, no facing pointer, no chevron/check | Name + `Hover`; one hover actor maximum. Pointer must hit the owner-approved visible candidate. |
| Selected, usable | Four solid `FocusRing #E9BF79` corners + facing pointer; optional depth-tested outline on visible surfaces, 3/2 px core | Leading chevron + check + `Selected`; selected target frame persists while valid. Never hostile red for an NPC. |
| Selected and hovered | Keep selected recipe; no duplicate hover ring | One plate; selection wins. |
| Out of action range, still legally selected by controller | Amber split corners + opposing horizontal arrow icon | `Selected · Out of reach`; action prompt says `Move closer` only for an explicit distance diagnosis. No auto-walk or auto-retry. |
| Selection rejected/lost at range boundary | Remove selection immediately; optional split-corner residual for 1.2 s only on a still visible diagnosed candidate | `Out of reach` as unavailable feedback, never `Selected`. Without diagnosis show `Target lost` in target frame for 1.2 s. |
| Dead target / unreachable candidate | No selected ring; gray hover corners only while inspecting a visible candidate, plus slashed circle | `Dead · Cannot attack` / `Unreachable`. Dead and lootable are independent; replace with interaction focus only when that owner supplies it. |
| Obstructed / destroyed / hidden candidate | Remove ring, outline and world plate immediately | Fixed target frame: `Target lost`; specific `Blocked`/`Unavailable` only when supplied. No marker at a hidden actor's last location. |
| Gamepad cycling | Same single amber selected ring; static leading chevron, not a moving beam | On cycle show `Target n of total` for 1.2 s in target frame, supplied by targeting owner; L/R shoulder hint is fixed HUD content. No marker on every eligible target. Missing ordinal data: `Target selected`. |
| No candidates | No ring | Once per deliberate cycle: `No available target`; never fabricate n/total. |

The current enemy filter normally drops out-of-range targets, so its residual or fixed-frame response is the applicable recipe. Persistent “selected but out of range” is only for an owner that independently permits selection while rejecting an action. Hovering invalid actors and precise loss reasons require a candidate-diagnostic adapter; absent that adapter, show only information already published. Reachability is provider-supplied, never inferred from proximity or a clear line of sight.

**Occlusion is mandatory.** Ground graphics follow the actual floor plane and are depth-tested/clipped by world geometry; optional outlines show only visible actor surfaces. Screen plates require both gameplay visibility permission and camera-visible anchor/body. Do not make faded walls click-through or reveal hidden actors. A vanilla always-on-top WidgetComponent is insufficient for those rules. The integrator must supply depth/occlusion handling for projected strokes; until then retain the fixed opaque target frame and validated ground-mesh version, and record unresolved world-marker contrast. No full-screen outline postprocess or per-enemy light is required.

### Contrast against every A-04 floor

All foregrounds come from V-03. Selected/warning = amber `#E9BF79`; hover/dead = gray `#C4C4C4`; check/damage text = white `#F2F2F2`; player/loot/heal/mana = cool `#A9C4CC`; keyline/track = `#171717`; text plate = `#242424`. Enemy identity is a target-bracket icon and `Enemy`/supplied `Hostile`, NPC identity is a speech icon and supplied role, player identity is a trailing cool bar and `You`. No red/green semantic pair, rarity rainbow, hue-only miss, or red medical cross.

Recomputed using V-03's sRGB linearization and luminance weights (0.2126/0.7152/0.0722), contrast `(lighter+0.05)/(darker+0.05)`. Each cell is **flat token : floor / A-04 diffuse stress minimum**, both ratios to 1. Stress applies gains `(0.85,0.92,1.00)` and `(1.00,0.72,0.45)` to *both* floor and foreground in linear RGB, then the floor's scalar, reporting the smaller ratio. These are independent calculations, not a ratio range.

| Area / floor sRGB / stress scalar | Amber | Hover / dead gray | Primary white | Cool |
|---|---:|---:|---:|---:|
| Hub exterior / `#77766D` / 1.00 | 2.65 / 2.59 | 2.62 / 2.50 | 4.08 / 3.87 | 2.49 / 2.34 |
| Hub interior / `#77766D` / 0.80 | 2.65 / 2.51 | 2.62 / 2.41 | 4.08 / 3.69 | 2.49 / 2.25 |
| B1 / `#695640` / 0.60 | 4.06 / 3.26 | 4.01 / 3.09 | 6.24 / 4.65 | 3.81 / 2.89 |
| B2 / `#695640` / 0.45 | 4.06 / 2.93 | 4.01 / 2.78 | 6.24 / 4.12 | 3.81 / 2.62 |
| B3 / `#695640` / 0.33 | 4.06 / 2.59 | 4.01 / 2.47 | 6.24 / 3.58 | 3.81 / 2.34 |
| B4 / `#514B42` / 0.25 | 5.01 / 2.54 | 4.94 / 2.42 | 7.70 / 3.42 | 4.70 / 2.30 |

Bare diffuse amber/gray/cool fail A-04's proposed **3:1** marker goal on multiple floors. Therefore do not approve bare coloured lines. Use the opaque dark keyline and an exposure-independent overlay path for the readable core, while preserving occlusion. In token space, **amber/gray/white/cool against the keyline are 10.41 / 10.28 / 16.01 / 9.78:1**, independent of which of these floors sits outside it. Against the opaque plate they are **9.01 / 8.90 / 13.87 / 8.47:1**, exceeding V-03's **4.5:1** text goal. Dark keyline alone is not the fallback; the paired bright core plus dark surround is required. A diffuse world mesh, emissive material or post-tonemap tint is not automatically equivalent to this overlay arithmetic.

The selection sheet's six floor samples show **flat swatches**, not the stress gains or simulated lighting. Actual floor mixtures (exterior sand/water edge, red aisle, bright torch pool, pale rubble/stains), anti-aliasing, thin strokes and shortened camera booms still require rendered checks under A-04's fixed exposure. Keep text opaque/full-opacity throughout lifetime; no fade through a low-contrast intermediate. If contrast fails, simplify the adjacent floor/lighting and retain readable fixed HUD content; do not recolour creatures, brighten all emission or pierce walls.

## Nameplates and health bars

Project the A-03 anchor to screen, then place the plate's **bottom edge 12/8 px above** it; center horizontally. An opaque 3/2 px keylined leader connects to the anchor if displaced. Plate width starts at **360/240 px**, can grow to **480/320 px**; padding 12/8. Name/action line 42/28 px; state/provenance line 36/24; HP line 42/28; gap to a 12/8-high bar 12/8. A common full plate is 360×168 / 240×112. Bars have Background track, Edge boundary and neutral BarHealth fill. Resource name/value sits in its own opaque band, never on the fill. Long names wrap; full name remains in fixed target frame/Details. Do not squeeze text to fit.

| Actor | Admission and styling | Health information |
|---|---|---|
| Enemy | Selected or valid hover; explicit aggro; or damaged snapshot while visible. Name + target brackets, `Enemy`; add `Hostile` only from AI. Newly received hit keeps eligibility 3 s even if immediately healed; otherwise damaged means current HP below max from resolved data. | HP shown only if authority exposes resolved current/max; neutral shield + `Health`. Unknown maximum gives hatched/unfilled track + `Health — Unknown`, never a guessed fraction. No enemy mana by inference. |
| NPC / service | Selected/hovered/current interaction only. Speech icon + name + supplied role. No permanent town labels. | No HP bar by default; absence means not exposed, not invulnerable or full. Do not invent aggro/damage styling for civilians. |
| Player | Fixed HUD HP/MP always when known; world nameplate only on explicit self-identification request, 2 s, or an adopted accessibility preference. Cool trailing mark + name + `You`. | No duplicate world HP/MP by default. Incoming feedback and low-resource notices use fixed player frame even if world plate omitted. |

The presenter supplies resolved bar fractions or derives them from the coherent authoritative snapshot in one owner adapter; a widget only clamps a supplied draw fraction. It never estimates HP from hits, damage text, body colour or sample bar geometry. No numeric health is promised for enemy types whose owner exposes none. Runtime Prototype/Disputed/Unknown badges follow V-03; a spec-wide P statement does not replace live provenance labels.

### Zoom and crowd handling

Use the real perspective camera: horizontal FOV45°, boom **900 / 1200 / 1800 cm**, pitch−60…−50°, yaw0…90°, center near Z90, including spring-arm shortening. World radius projects naturally; screen text/plate sizes are **constant across zoom** at the current UI scale. Scale once by 2/3 at 720p; do not multiply by distance or apply DPI twice. For reference, at target-centered default pitch−55°, a 70 cm-wide span at floor-root depth `D+90 sin55°` projects to about **111.1 / 84.9 / 57.7 px at 720p** (×1.5 at1080p). This transverse ruler is not the full perspective square/body silhouette. Smaller rats can therefore have a wider screen label than body without enlarging their target.

Cap **6 world plates total**, including selected, hover, NPC and any player plate; no second batch for aggro. Lower zoom does not lift the cap. Priority: (1) selected actor, (2) explicit interaction candidate or hover, (3) AI-confirmed actor currently attacking the player, (4) other aggro, (5) damaged, (6) requested player identification. Deduplicate by stable entity+life. Within a tier sort by owner distance, then stable ID; never reorder from random draw iteration. Selected remains pinned; a new lower-tier candidate waits 0.5 s before replacing an existing same-tier plate to reduce flicker. The cap may shrink with font enlargement, never grow to preserve a count.

Place higher-priority plates first. Try anchor, then up by one plate height+12/8, then left/right by **120/80 px**; maximum displacement **240/160 px**, measured from the initial placement to the displaced plate center. Lateral slots are alternatives from the initial placement, not added on top of the upward shift. Reject slots intersecting another plate, the player body rectangle, selected body rectangle, fixed HUD regions, prompt or reserved combat text. If no slot fits, omit that world plate; selected information remains in fixed target frame. Never move a label to a different actor or obscure the feet to show a bar. The selected plate is transient actor feedback, not a permanent new panel in V-02's world clear zone `(522,246,876,444)`; long copy goes to fixed target/Details. Other unselected bodies remain visible without plates; no plate does not mean no threat. Do not draw an invented total-enemy count for culled plates. Offscreen/occluded plates disappear rather than clamping into a tracking arrow at the screen edge.

## Combat text and exact timing

Use compact **opaque Panel chips**, Primary type 30/20, one geometric icon plus readable word and signed amount. No flying naked numbers, particles, outline-only glyphs or camera shake. Default lifetime **1.0 s**, fixed position; optional rise **24/16 px** during that lifetime only when motion is enabled. Full opacity until removal. Reduced motion uses the same fixed chip with identical content/duration. Numbers in the sheet use placeholders `N` to avoid invented damage tuning.

| Event / words | Shape + token | Publication and placement |
|---|---|---|
| `Damage −N HP` | Shield with diagonal notch + Primary | Resolved `OnImpact` with `bHit`; anchored near recipient, no number at button press, commit, anticipation or animation contact. Use owner-labelled resolved damage; see clamp distinction below. |
| `Miss` | Two separated strokes crossing an empty circle + Secondary | Resolved `OnImpact` with `bHit=false`; no victim hit flash, no `0 damage` substitution. |
| `Dodge` | Two stepped chevrons + Secondary | Only a future explicit authoritative dodge outcome. Current `FCombatResult` has bHit only, so current miss is always **Miss**, never inferred Dodge. |
| `Heal +N HP` | Open palms / upward ray + Cool | Healing owner's settled event, after its explicit impact/resolution; current melee component has no heal event. Never infer from regen/load or positive attribute delta. |
| `Mana −N MP` | Diamond + Cool | A labelled spend receipt in player resource frame **at confirmed commit**, because mana is charged there; never a pre-impact damage/heal number. Default no floating mana chip. Cost preview remains labelled Cost and is not a spend. |
| `Can't attack: [reason]` | Error octagon + Amber outline, Primary words | Fixed resource/action frame immediately after known rejection; no damage text. Summary persists until another outcome or deliberate dismissal; full reason in paused Details. Coalesce held/repeated identical failures. |

**Impact time is authoritative.** `ImpactSeconds` comes from the committed activation, never a UI constant or animation notify. Zero delay can resolve synchronously inside `RequestBasicAttack`; observers and identity context must already be installed. Do not attach listeners only after the call returns. The foundation currently publishes only `OnImpact(Id, Result)` and `OnDeath(Id)`: no target ID in OnImpact, no cost/commit/cancel/failed-impact delegate, no reason for an invalidated late impact. D14/W4 must supply identity and start/finish seams before anticipation, cost chips or reason-specific late failure can be reliable. A timeout can clear a cosmetic windup, never assert Hit/Miss/Refund. Missing hooks mean omit that cue, not simulate it.

The current result's `Damage` is the resolved combat amount **before HP clamps to zero**. Display it as `Damage −N HP`, not “N HP lost.” Actual effective loss/heal requires an owner-published delta; do not subtract pre/post widget samples, which can include simultaneous events. Current damage events cannot be relabelled “critical,” “blocked” or “dodge.” Basic melee only is implemented; do not copy its timing to future projectiles/spells.

Mana/cooldown begin at commit. If cancelled or invalidated after that, preserve the charged mana and remaining cooldown; no refund toast and no fabricated miss. A failed impact may emit no OnImpact at all. Pause freezes gameplay timers and feedback lifetimes; restore/travel clears transients instead of letting them finish against a replacement actor.

Death can arrive **before the attacker OnImpact**: latch dead presentation immediately, suppress later victim flash/windup, but permit exactly one resolved lethal damage chip with that hit identity if still visible. Never resurrect the plate's HP/living state to display it. Deduplicate by entity+life+ActivationId+ImpactIndex and feedback kind; command receipt replay (`bReplay`) hydrates state without repeating chips/rewards. Loot/XP comes from lifecycle/reward owners, not a death animation or OnImpact listener.

### Stacking and rejection copy

Reserve **2 chips per recipient, 8 world chips globally**. Chip baseline separation **54/36 px**, margin **12/8**; start above the plate, then use the nearest clear side slot. Chips cannot overlap names, player/selected body, HUD or prompts. Player incoming results have first priority, selected target second, other visible recipients third; preserve event order within a recipient. At pressure evict the oldest lower-priority cosmetic chip; if no space remains, omit the world chip and keep the latest player/target feedback in its fixed frame. No summing hits, merging identities, accumulated damage number or delayed chip queue that could imply a later impact. Snapshot bars still update for every accepted change. A bounded **32-entry session feedback history** in paused Details can retain supplied outcome text; it is neither a save ledger nor the command receipt store.

Use `Can't attack:` as a fixed heading and one short body summary in V-02's 426×96 region; this fits two 42 px line boxes. Long explanation stays in Details. No error timer forces the user to read while fighting. Refresh snapshots before deliberate retry; never mint a second intent for an uncertain receipt.

| Exact `ELHCommandReason` | Short summary after `Can't attack:` |
|---|---|
| `InvalidRequest` | Action not valid |
| `ReusedRequestId` | Refresh and try again |
| `UnresolvedRules` | Rules data unavailable |
| `InvalidLifeState` | Current state prevents it |
| `NotFound` | Target unavailable |
| `OutOfRange` | Out of reach or blocked |
| `Obstructed` | Path blocked |
| `Ineligible` | Requirements not met |
| `InsufficientMana` | Not enough mana |
| `InvalidEquipment` | Check equipment |
| `Cooldown` | Not ready yet |
| `ActiveAction` | Finish current action |
| `Busy` | Action still finishing |

`OutOfRange` deliberately covers both distance and LOS: native `InRangeAndSight` collapses them. Only separately supplied diagnostics permit “Move closer” versus “Path blocked.” `None` on an accepted request gives no error; rejected/None or future codes show `Action unavailable`, with diagnostic in Details. Shared noncombat reasons `InsufficientPoints`, `InsufficientGold`, `InventoryFull`, `InvalidDestination`, `SaveRequired` retain V-02's contextual action copy (“Not enough points/gold,” “Inventory full. Nothing transferred,” “Destination unavailable,” “Save progress first”); do not show an attack heading for shopping/loot/travel. A phase-labelled postcommit failure must say `Attack interrupted: [reason]`, retaining cost, rather than implying a precommit rejection left resources unchanged.

## Interaction prompts

One prompt in V-02's **(522,714,876,108)** region at1080p, uniformly scaled at720p; opaque Panel, Edge boundary, 12/8 inset, 30/20 functional text. Two 42/28 line boxes: first action/name with input glyph, second availability/destination. If full generic binding + long name cannot fit, first line keeps binding+verb; full name and destination move to a reflowed destination/dialogue panel or Details before commit. Never shrink type or hide the actual destination behind ellipsis. No prompt over every nearby corpse. Selected interaction candidate wins; fallback comes from interaction owner's ordered candidates, not widget nearest-distance logic.

| Owner-provided candidate | Available KBM / generic gamepad examples | Unavailable and pending |
|---|---|---|
| NPC / service | Keycap E + `Talk · Kilhiam`; South position diagram + `South · Talk · Kilhiam`; second line supplied `Light teacher` | `Move closer to talk` only on explicit range status; missing offer does not invent Buy/Heal/Train. Presence-only NPC can show name/role with no action glyph. |
| Stairs / portal | E / South + `Inspect travel`; second line `Down to B2` or `Up to Temple`, supplied destination | `Move closer to travel`; first activation opens Stay/Travel confirmation, never immediate relocation. Stay default. Save/load fence says `Travel in progress`; no destination guess, new B4 descent or glowing teleport. |
| Loot / corpse | E / South + `Inspect loot · [name]`; second line `Loot available` | `Move closer to loot`; full inventory shows exact failure and leaves badge; finalized empty shows `Empty` without active pickup glyph. No Take all or gold sentinel introduced. |
| Door, only if gameplay adopts it | E / South + supplied `Open door` or `Close door` | `Move closer`; supplied blocked/locked reason only. Frame cannot choose hinge animation, unlock, auto-close or create a door at an open archway. |

Unavailable action keeps full-opacity label, slashed-circle and explanation; replace active glyph with that icon so it does not promise a submit. The selection owner may already have dropped the candidate: then remove its prompt and show fixed-frame loss feedback, not a stale action. Pending retains name + static hourglass + `Working…`, suppresses duplicate activation and changes only on owner result. Actual inventory/loot/service mutations use the V-02 modal and existing authority commands; no mutation in the prompt component.

Glyphs follow V-03: 36/24 square geometry, 3/2 stroke, 12/8 label gap, no icon fonts/logos/branded letters. A face glyph is four small circles with the South position filled, labelled **South**. Shoulder glyph is neutral rounded rectangle plus **L shoulder / R shoulder** words. Stick press is crossed circle with center dot plus **Right-stick press**. Keyboard uses current remapped keycap; mouse uses two-button outline with active half + `Right mouse`. Display only the active device's binding, retain logical target on device switch, pause on disconnect through session owner. In the sheet, alternate device examples are side by side for comparison, not two simultaneous prompts.

## Lifecycle and low resources

| Authoritative state | Presentation | Removal / restrictions |
|---|---|---|
| Alive → dead | A-03 flattened family pose and dead palette, plate `Dead`; player says `Defeated`. Remove enemy HP bar, aggro and attack indicators. | Death once per life; no loot promise, kill-count/XP award or auto-respawn from animation. Fixed target loss and optional last hit follow rules above. |
| Dead, loot finalized and available | A-03 cool diamond beside corpse + `Loot`; reuse corpse footprint. No extra pillar, beam or light. | Only actual remaining contents enable inspection. Suppress diamond if its placement lacks clear floor; use visible plate/prompt. Lootable state is not current combat selection. |
| Contents just exhausted | Remove diamond immediately; show check + `Looted` for 1.2 s only for confirmed local successful collection, then `Empty` when inspecting. | Other causes of empty state/load show `Empty` directly, never a fake local pickup. Partial collection keeps Loot; InventoryFull retains all state. Corpse cleanup belongs to lifecycle owner. |
| Waiting to respawn | No world marker, ghost body, timer or placeholder target. | Do not reveal a future spawn/location. If death UI has an explicit owner pending state, use static hourglass + `Preparing return`; no guessed countdown. |
| New life committed | A-03 optional 0.35 s mark settle/inward scale; static body in reduced motion. If selected afterward use normal live plate. | New life key flushes old effects. No replay when loading an already live actor. Selected campaign Balork does not respawn. |

Use **HUD-only low-resource cues**, no screen-edge vignette, full-screen tint, flashing health bar or pulse. In the existing player frame, shield + `Low health` or diamond + `Low mana` becomes a static amber warning beside the separate labelled numeric band; retain neutral/cool bar colours. First-pass presentation thresholds: enter at **≤25% HP**, clear **≥30% HP**; enter **≤20% MP**, clear **≥25% MP**. These are UI attention proposals, not mechanics. The owner presenter evaluates resolved snapshots and hysteresis; widgets consume LowHealth/LowMana flags. Unknown/absent maxima suppress threshold logic and display Unknown; never divide by zero. HP0 follows authoritative life state, not a UI-authored death. Known MP0 says `Mana empty`; do not suggest a cast is affordable from the low-mana flag. Ability affordability remains authority-owned.

If both warnings are active, use two separate labelled lines within the player frame, expanding that frame downward inside its left HUD column if required by text enlargement; never over the player/quick slots. Warning remains while true. Rejection `Not enough mana` has higher priority in the action-message frame and can point to owner-supplied recovery Details; no new offline/paused recovery promise. Do not append warning badges to all enemy plates.

## C++ presentation handoff, no code or schema changes

Names in this table are **semantic adapter responsibilities**, not declarations claimed to exist. Shared headers, enums, schemas and binary assets stay with their owners. Use one local-player feedback presenter and one pooled screen overlay; actor components expose anchors/lifetime only. Keep world markers noncolliding, non-navigational and nonblocking on Visibility. No widget writes attributes, applies damage, grants abilities, rolls RNG, determines aggro, finalizes loot, spawns or saves.

| Owner data / identity | Update trigger | Presenter use / current gap |
|---|---|---|
| Controller target, candidate order, binding/context | Select/cycle/clear; validity/context/device change | Current `GetSelectedTarget` can be sampled once per frame by one adapter. No target-change/loss-reason event currently exists. Owner adds structured diagnostics/ordinal; UI must not rescan actors or call attack validation per plate. |
| Stable entity ID, runtime actor binding, life/restore generation, floor/visual bounds | Spawn, possession, restore, destroy, presentation swap, travel | Cache anchors; erase pooled state when generation changes. Actor path/index is not save identity. Camera projection does not alter gameplay. |
| HP/MP snapshots, maxima, exposed fractions, provenance, low flags | GAS attribute/owner snapshot publication; load hydrate | Coherent data only; unknown handling. Attribute changes can update bars but do not imply heal/hit/cost events. |
| Committed action/target context, cost receipt, impact identity, outcome, terminal status | Commit, impact, finish/cancel | Bind before activation (zero delay). Current OnImpact/OnDeath are insufficient alone for all cues. Authority adapter retains target/life/epoch and handles reentrancy; never ask current selection who was hit. |
| AI aggro/current attack target | Acquire/release and target changes | Admit Hostile/attacker plates; no distance-based aggro inference. |
| Loot state, interaction eligibility/reason, destination, door state | Inventory/loot transaction, reachability update, selection, travel/save phase | Prompt and lifecycle badge update atomically from owner state, no eligibility arithmetic or loot generation. |
| Session pause, resume, load/travel, receipt replay | Session state transition | Freeze cosmetic timers, clear world effects on teardown, hydrate without replay. Input context reset/held-button release remains control owner work. |

### Pooling and GTX 1050 Ti budget proposals

Preallocate **6 plate widgets, 8 combat chips, 2 ground-marker instances** (selected plus distinct hover/residual), **1 optional selected outline**, **1 interaction prompt**, **1 fixed action message**, plus existing player/target frames. Residual invalid feedback shares the hover slot; it never adds a third ground marker. At most **8 visible corpse loot badges**, independent of six plates: prioritize selected interaction then nearest visible, stable-ID tie. Under pressure omit distant badges, not available loot itself; interaction still resolves through its owner. A-03 corpse meshes and encounter actor counts remain separate owner budgets.

Reuse the same brushes, icon geometry, font and materials; no widget-per-world-actor allocation, per-hit spawn/destroy, dynamic light, particle emitter, per-plate render target or full-screen outline effect. Release/rebind pool handles with entity+life+event generation so an old timer cannot hide a new plate. Dedup cache and history are bounded to **32 recent events per observed life, 64 retained lives**; lifecycle teardown/restore clears the relevant caches. Late/replayed effects still need the authority's epoch/receipt gate: this bounded cosmetic cache is not persistent idempotency protection.

Snapshot/event changes invalidate text/brush layout; camera movement projects only admitted anchors once each rendered frame. No per-widget polling tick. Reconsider lower-priority admission/collision slots at **10 Hz** and on target/life events; selected-target loss must clear immediately through the adapter. Visibility work shares owner results; if extra camera occlusion traces are needed, budget **4 per frame**, prioritize selected/interaction, retain hidden for unknown results and expire unselected visible results after **0.1 s**. Revalidate the selected/interaction camera-visibility result every frame; invalidate cached visibility immediately on camera/actor movement or door/wall occlusion changes. Unknown results hide the affected world cue until refreshed. Do not use stale actor transform/visibility to draw through a newly closed door. These scheduling values do not slow gameplay range/LOS validation, which stays authoritative.

Allocate a first-pass **≤0.5 ms game-thread / ≤0.5 ms GPU** feedback share in the inherited **16.67 ms** whole-frame target, measured at native720p and1080p on the actual GTX 1050 Ti. These are separate thread budgets, not guaranteed additive savings. Inspect Slate/UMG invalidation, draw calls, overdraw and allocations while cycling, fighting and looting at maximum pool occupancy. No measured performance is claimed. If over budget, remove optional selected outline and animated chip movement, reduce lower-priority world plates/badges, then profile again; retain fixed readable HUD, event correctness and selected feedback.

## Integration checks still required

- W1-01/integrator reconciles configured selection distance and exposes target/candidate diagnostics, input labels and no-through-wall feedback. Dead-corpse interaction stays separate from living combat targeting. V-02 binding prose is not modified by this scoped task.
- W4-01/03/05 supplies D14 commit/target/cost/cancel/finish events, including zero-delay and reentrant replacement, precise miss versus dodge, and optional effective deltas. Rejections returned/logged by `RequestSelectedAttack` need a UI bridge; accepted-only `OnAttackRequested` is not that bridge. W4-02 supplies AI signals; W4-04 supplies loot/life/respawn; W4-07 supplies service prompts.
- On Linux, inspect all six A-04 areas at900/1200/1800 boom, pitch−60/−55/−50, yaw0/45/90, shortened boom and both resolutions. Sample actual core/keyline/text pixels on darkest floor, brightest pool, red aisle, pale rubble, sand/water edge. Check grayscale shape identity and enlarged text; retain screenshots as actual renderer evidence, not these SVGs.
- Exercise hover versus selection, forward/back cycle and empty set, provider-unreachable, distance/LOS loss, death before OnImpact, zero delay, duplicate/replay event, cancel after cost, restore into new life, pause/resume, inventory full, partial/final loot, unknown maxima and both low warnings. Verify no number precedes impact except the explicitly labelled committed mana spend; no extra damage/loot/save path exists.
- Use a crowded permitted encounter with more than six eligible plates/eight chip events; check deterministic priority, body/feet visibility and no delayed damage queue. Test device switch, controller disconnect and actual bindings. Existing G1 gamepad coverage remains UNTESTED; this task does not close it. Capture CPU/GPU traces and pool counts; Windows remains deferred.

Assumptions requiring later review: adopt readable 30/20 primary plates over A-03's smaller text; keep overlay/keyline stroke outside world exposure while correctly occluded; use HUD-only resource warnings; use current runtime bindings/range until owners change them. The required event/presenter seams and visual rendering do not exist merely because they are specified here. Static arithmetic, SVG parsing/render inspection and source checks are recorded in the attempt report; runtime readability/performance remain unobserved.
