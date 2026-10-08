# V-02 — Lighthaven UI wireframes

**Design specification; written, not implemented or played.** Eleven screen layouts for W2-03 and W4-05, including W4-07 services. Contract revision 1 is still a draft. These text SVGs are original layout drawings, not Unreal screenshots, final art, runtime assets or approved mechanics. Linux is the first validation platform; Windows validation is deferred. No engine gate is passed here.

## Screen index

| Screen | Specification | Layout | Primary owner |
|---|---|---|---|
| Frontend | [Spec](frontend.md) | [SVG](frontend.svg) | W2-03 |
| Character select / continue / recovery | [Spec](character-select.md) | [SVG](character-select.svg) | W2-03 + W2-01 |
| Character creation | [Spec](character-creation.md) | [SVG](character-creation.svg) | W2-03 + W2-02 |
| Gameplay HUD / interaction / loot / travel | [Spec](hud.md) | [SVG](hud.svg) | W4-05 |
| Character sheet / allocation | [Spec](character-sheet.md) | [SVG](character-sheet.svg) | W2-03 |
| Inventory / equipment | [Spec](inventory-equipment.md) | [SVG](inventory-equipment.svg) | W2-03 |
| Spell selection / quick slots | [Spec](spell-selection.md) | [SVG](spell-selection.svg) | W4-05 |
| Trainer / vendor services | [Spec](services.md) | [SVG](services.svg) | W4-07 + W4-05 |
| NPC dialogue | [Spec](npc-dialogue.md) | [SVG](npc-dialogue.svg) | W4-05 |
| Pause / settings | [Spec](pause-settings.md) | [SVG](pause-settings.svg) | W2-03 + W4-05 |
| Death / respawn | [Spec](death-respawn.md) | [SVG](death-respawn.svg) | W4-05 + W2-01 |

Frontend → character select or creation → HUD. HUD opens a paused journal shell (Character / Inventory / Spells), contextual dialogue/services, or Pause. Death replaces gameplay input. Continue always passes through validation/recovery before enabling simulation. Each spec includes a representative SVG and defines alternate states using the same regions.

## Sources and scope

Read against [game design](../../plan/docs/01-game-design.md) §§4–6, [architecture](../../plan/docs/02-unreal-architecture.md) §§5,7,8, [validation](../../plan/docs/06-validation-and-delivery.md), [art register](../art-register.md), [rules ledger](../rules-ledger.md), [rules implementation](../rules-implementation.md), [contracts v1](../contracts-v1.md), and the exact [request/reason declarations](../../../Source/Lighthaven/Core/LHCommands.h). The rules implementation still lacks resolved runtime creation data; research-ledger proposals do not automatically enable those values. Runtime data needs the owner's explicit ruleset selection, provenance and validation.

The supplied image was actually opened at `/home/brewerm/Downloads/lighthaven-unreal-design-and-agent-waves/lighthaven-plan/assets/references/characters/character-creation-reference.png`. It shows promotional fantasy art with floating dice, portal, creature and adventurers, **no creation interface**. Its filename is not evidence of original UI layout or RNG. No reference image is copied or embedded. These layouts are authored modernization, not historical reconstruction of that image.

No mechanics research or new tuning is introduced. Mechanical fields below bind to authority data, not example numbers in drawings. Bible-first reconciliation remains with research/rules owners; no secondary-source value is promoted to verified play or selected silently over the Bible. The palette hex values, spacing, text sizes, controller map and settings percentages are V-02 interface design choices, not historical game values.

## Shared geometry and typography

All SVGs use a `1920 1080` viewBox. At 1280×720 scale uniformly by 2/3; preserve aspect ratio, with letterboxing outside the safe rectangle on other aspect ratios. Never stretch or independently compress type. Content safe rectangle is **(72,54,1776,972)** at 1080p, **(48,36,1184,648)** at 720p. Full-bleed world/art can extend beyond it. Optional guide lines in drawings do not ship.

| Region / token | 1920×1080 | 1280×720 |
|---|---:|---:|
| Base spacing / small inset / panel inset | 12 / 12 / 24 px | 8 / 8 / 16 px |
| Grid | 12 columns of 126 px; 24 px gutters | 12 columns of 84 px; 16 px gutters |
| Header | x72 y54 w1776 h96 | x48 y36 w1184 h64 |
| Body | x72 y174 w1776 h720 | x48 y116 w1184 h480 |
| Persistent status / errors | x72 y900 w1776 h48 | x48 y600 w1184 h32 |
| Footer / input hints | x72 y954 w1776 h72 | x48 y636 w1184 h48 |
| Title / section heading | 48 / 36 px | 32 / 24 px |
| Body, controls, numbers | 30 px, 42 px line height | 20 px, 28 px line height |
| Supporting metadata / footer minimum | 24 px, 36 px line height | 16 px, 24 px line height |
| Minimum focusable target | 72 px high, 72 px wide | 48 px high, 48 px wide |
| Focus outline | 3 px, 6 px outside target | 2 px, 4 px outside target |

Use a plain sans-serif (wireframes: Liberation Sans fallback sans-serif), medium weight for body, bold for headings/actions, tabular figures for resource/cost columns. No ornamental font for functional text. The smallest allowed text is 24/16 and is reserved for supporting metadata; errors, requirements and selectable text use 30/20. Two-line buttons grow to 108/72. No shrink-to-fit. Wrap descriptions; clamp list titles to two lines and expose the complete title in the detail pane. Lists scroll, with focused rows kept wholly visible and a visible “row n of total” indicator. Long text never pushes footer or primary actions outside the safe rectangle. Detail reading mode: focus the Details control, South/Enter enters its scroll area, D-pad/stick or Page Up/Down scrolls, East/Escape returns to that control. No tooltip-only requirements.

Default two-pane layout: left (72,174,576,720), right (672,174,1176,720). Three-pane layout: (72,174,426,720), (522,174,726,720), (1272,174,576,720). Individual specs pin deviations. Modal: centered (372,282,1176,516), opaque surface, 24px inset; title/body/error above bottom action row, default focus Cancel/Stay. Scroll modal body if needed. Parent focus is retained and restored on close.

## Palette, states and readability

Art direction follows rough dark timber, off-white plaster/linen, restrained amber torchlight and cooler fill. Wireframes deliberately omit textures. Proposed UI tokens:

| Token | Hex | Use |
|---|---|---|
| `ink` | `#171717` | Opaque root / modal backdrop |
| `panel` | `#242424` | Opaque reading surface |
| `raised` | `#333333` | Rows, selected fill, buttons |
| `text` | `#F2F2F2` | Primary copy and resource values |
| `muted` | `#C4C4C4` | Supporting copy, never hidden disabled labels |
| `edge` | `#909090` | Control boundaries, empty bars |
| `amber` | `#E9BF79` | Focus / development provenance / warning accent |
| `cool` | `#A9C4CC` | Mana / informational accent |

Text pairs must retain at least 4.5:1 contrast; control/focus boundaries at least 3:1 against adjacent surface. UI uses opaque surfaces behind text even over bright world geometry. Accent text is on `panel`/`ink`, not white. Do not use muted-on-amber. Focus = amber outline plus leading chevron; selection = raised fill plus check and “Selected”; equipped = labelled slot badge; unavailable = lock/slash icon plus explanation, still inspectable and focusable. Pressed state adds inset outline; pending adds spinner **and** “Working…” text, reduced-motion replaces rotation with static hourglass. Status never depends on hue: `!` with “Unavailable/Error,” check with “Saved/Equipped,” labelled HP/MP bars and numeric values. Do not use red/green as a pass/fail pair. No pulsing/flashing health warning.

Every screen inherits the 30/20 body and 24/16 metadata minimum, contrast rules and opaque error regions. Readability lines in each spec identify its special risks. Optional text enlargement reflows scrollable body content; it must not scale only the texture or hide navigation. It is an integration verification requirement, not implemented functionality.

## Input and focus contract (authored default)

Use semantic actions and platform glyphs. In these drawings `[A]` means South/Confirm, `[B]` East/Back, `[X]` West/secondary, `[Y]` North/details, `[LB/RB]` shoulders, `[LT/RT]` triggers, `[View]` journal, `[Menu]` pause. Xbox lettering is illustrative; platform/remapping changes glyphs, never the labels. Glyph comes immediately before its action, in a 36px/24px box with 12px/8px gap. Footer groups align left-to-right with 36px/24px separation. Keep hints inside the safe footer. More than four hint groups go into a focusable Controls sheet; always retain Confirm, Back and context tabs.

- Menus: D-pad or left stick moves focus; South confirms, East backs out one layer. Up/down do not wrap; left/right change a stepper only while it is focused. Lists retain selection separately from focus. Shoulder tabs switch named tabs, remembering each tab's focus. Tab/Shift+Tab traverse the listed order, arrows adjust/list-navigate, Enter/Space confirm, Escape back, Q/E switch tabs. Mouse clicks the same controls; hovering never changes a committed selection. No drag-only, double-click-only, wheel-only or hover-only interaction.
- Page-sized lists use LT/RT or Page Up/Down. All scroll areas have a focusable reading mode. No hidden actions require analog precision or a timed hold.
- Gameplay: left stick/WASD move, right stick/middle-drag orbits, L3/Shift toggles run, R3/F acquires/releases target lock, LB/RB or Shift+Tab/Tab cycle previous/next target, RT/right mouse attacks, South/E interacts, View/I opens journal (C opens Character), Menu/Escape pauses. Hold LT for quick-slot selection; while held LB/RB stop cycling targets, D-pad/left stick selects six slots, South submits selected ability and release/East cancels. With the Toggle preference, LT opens, a second LT or East closes without casting, and South confirms; release alone has no effect. The full spell screen is another hold-free alternative. Hotkeys 1–6 submit those same slot abilities. R3 + D-pad up/down zoom is an authored contextual mapping; defer R3 lock-toggle until release and suppress it if the zoom chord was used. No combat mapping leaks into a menu.
- Opening journal, dialogue, services, pause or death requests a local simulation pause through the session owner. Clear held movement/attack, change context, then enable menu focus. Close restores context only after all buttons are released. Full spell menu is paused; quick selector is also paused while open (single-player adaptation). Ability chosen there executes only after closing the pause context and authority revalidation.
- Opening a modal confines focus; Back cancels unless a committed operation is already in progress. Pending mutation: suppress duplicate Confirm, retain inputs and request ID, allow read-only details; Back cannot cancel an already committed action. On device switch retain logical focus and show current glyphs. On gamepad disconnect request pause and show “Controller disconnected”; reconnect or keyboard dismissal restores focus. Never resume automatically.
- Gamepad text entry is required on Linux too: name field opens an in-game keyboard, not a platform keyboard assumption. Alphabet grid of ten columns, D-pad movement without wrapping, South enters, X deletes, Y toggles case; bottom row Space / Symbols / Done / Cancel. Down from a short row clamps to nearest column. East cancels keyboard only and restores the previous field value; Done accepts the staged text. Physical typing/paste/IME feed the same field. Validation uses the owner's Unicode/name policy, not ASCII-only rules invented here.

## Authority, development data and command results

Screens submit intents and read published snapshots. They never set HP, XP, gold, learned knowledge, inventory or canonical stats, and never calculate combat/growth, prices, eligibility or death penalties. UI may stage text, appearance IDs, answer IDs, allocation deltas, quantity and quick-slot selection. Authoritative preview validates staged input; successful commands publish coherent new state. While an outcome is unknown, keep the same request ID and payload for receipt recovery/retry. After a definitive rejection (including `Busy`) and refresh, a deliberate retry is a new intent with a fresh ID; otherwise a cached rejection could repeat forever. Edited payloads are new intents only after the prior outcome is known. Never mint a fresh ID for an uncertain or accepted operation. `bReplay` success dismisses pending UI without replaying rewards/toasts. Bind delayed results to the originating request/character; do not apply them to a different selection. The current command handler is synchronous; pending states describe owner orchestration, previews and storage, not a new network transport.

Display resolved values with their unit; display unresolved as **“— Unknown”**, never zero, empty, free, full or a fabricated percent. `Prototype` appears beside every provisional numeric field (or an explicitly labelled containing group); `Disputed` appears when sources conflict. A developer Details page exposes field path, ruleset/revision, provenance status, source URL/baseline/retrieval date and notes from `FLHFieldProvenance`. This is presentation of supplied metadata, not a new source claim. Unknown bars have hatched/empty outlines and “Unknown,” no progress fill. Required unresolved data disables commit but keeps the action focusable with an explanation. Optional absent values use “Not used by this profile,” distinct from unknown. A known zero displays `0` only when resolved. Do not show proposed ledger defaults as live numbers before runtime approval.

`FLHCommandResult` currently provides only Request, Disposition, Reason, CommittedSequence and bReplay. Detailed field errors, eligibility comparisons, service quotes and source metadata require presenter/read-model data; do not pretend they are in that result. Generic failures remain useful without optional details. All rejected gameplay commands leave canonical state unchanged. UI retains staged edits and refreshes the authoritative view before retry.

| Exact `ELHCommandReason` | Default player copy / response |
|---|---|
| `None` | Accepted only: refresh from committed snapshot; never treat rejected/None as success. |
| `InvalidRequest` | “This action could not be validated.” Keep inputs; show field errors only if supplied. |
| `ReusedRequestId` | “This action could not be matched. Refresh and try again.” Resolve pending receipt; diagnostic details behind Details. |
| `UnresolvedRules` | “Unavailable: required rules data is unknown.” Show provenance detail; no zero fallback. |
| `InvalidLifeState` | “Unavailable in your current state.” Route to death screen when dead. |
| `NotFound` | “This target or offer is no longer available.” Refresh list; preserve nearest surviving focus. |
| `OutOfRange` | “Move closer and try again.” Close/service return can restore HUD; never auto-move. |
| `Obstructed` | “The path is blocked.” Retain target; no repeated auto-cast. |
| `Ineligible` | “Requirements not met.” Show authoritative comparisons if available. |
| `InsufficientPoints` | “Not enough points.” Name attribute/skill pool in context. |
| `InsufficientGold` | “Not enough gold.” Refresh balance/quote; no deduction on rejection. |
| `InsufficientMana` | “Not enough mana.” Show current/required if known, recovery hint. |
| `InventoryFull` | “Inventory full. Nothing transferred.” Offer Inventory / Back. |
| `InvalidEquipment` | “Equipment does not support this action.” Show slot/quiver detail if supplied. |
| `Cooldown` | “Not ready yet.” Show remaining time only when provided. |
| `ActiveAction` | “Finish the current action first.” Do not silently queue a purchase/cast. |
| `InvalidDestination` | “This destination is unavailable.” Remain at safe source checkpoint. |
| `SaveRequired` | “Progress must be saved first.” Retry save / Cancel via session owner. |
| `Busy` | “Another action is finishing.” Disable duplicate submission until resolved. |

Unexpected or future reasons use “Action unavailable. Your changes were not applied” and a readable Details diagnostic. No invented `InvalidName`, `SaveFailed`, `CorruptSave`, `AlreadyLearned`, `Respawn` or `FullMana` reason codes. Save/load/device/settings errors are separate proposed session statuses, never falsely mapped into `FLHCommandResult`.

## Interfaces needing integrator decisions

These are **semantic intents, not new C++ request names or schema edits**. Labels below are sufficient to build presentation, but execution must await owner interfaces. Keep their unavailable state visible in development.

| Missing interface / data | Required behavior | Owner |
|---|---|---|
| Profile list, create-slot selection, continue/load, recovery acknowledgment, delete, save/exit | Stable character identity, validated generation summaries, pending/failed/durable states; never direct widget file IO | W2-01/02 |
| Creation preview / reroll / legal review / name validation | Authoritative generation token or equivalent, four question/answer definitions, retained edits, atomically accepted result; UI cannot supply trusted rolled stats | W2-02 + integrator |
| Allocation preview; inventory/service eligibility and quote | Read-only snapshot of proposed outcome with freshness; authority recomputes on commit | W2-02 / W4-07 |
| Quick-slot bindings and targeting adapter | Bind known abilities to six presentation slots; persist preference through approved owner if desired; canonical spell knowledge unchanged | W4-03/05 |
| Consumable use from an item instance | `FLHUseAbilityRequest` lacks item/quantity; no ambiguous consumption adapter invented | W4-03 + integrator |
| Gold-only loot pickup | Existing request has Item; sentinel semantics unresolved, so gold transfer disabled until frozen | W4-04 |
| Pause, resume, input rebinding, volumes, exit and death/respawn | Session/settings commands, explicit results, durability fence before safe resume | W2-01 + W4-05 |
| Detailed validation/provenance view models | Field errors and authority eligibility beyond the single reason enum | Integrator / UI owners |

## Verification handoff

Run V01/V02/V14 on Linux at both resolutions with a gamepad from cold frontend through creation/name entry, allocation, equipment/quiver, learning, casting, dialogue, death and recovery. Run V05/V11/V13/V17/V18/V19 for rejection, corrupted saves, unavailable content, zero resources and starter legality. Test expanded text, long names, multiline questions, rapid Confirm, modal Back, stale quotes, device switch and disconnect. Check every focused row is visible and every disabled action explains why. These are acceptance scenarios **to run after implementation**, not observed results. See the attempt evidence report for static layout checks actually performed.
