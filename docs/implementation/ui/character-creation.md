# Character creation

[Wireframe](character-creation.svg) · [Shared conventions](README.md)

## Purpose and commands

Stage one human character without consuming a slot. Name/appearance → four affinity questions → roll/re-roll → legal review → confirm → durable initial snapshot → church. Final submit is `FLHCreateCharacterRequest` with Request, DisplayName, AppearanceIds and accepted Creation record. W2-02 must validate an authority-issued roll/token or equivalent; the widget never invents/trusts rolled attributes. Roll/preview, form validation, slot reservation and initial-save status interfaces remain proposed integrator seams. No permanent classes, bonus appearance or direct stat editing.

## Elements and layout

Three-pane body: left (72,174,426,720), middle (522,174,726,720), right (1272,174,576,720). Header shows Create Character, ruleset label and current step. Persistent status shows summary error; footer changes hints by step. Representative SVG shows the question stage; other stages reuse the middle pane.

| Element | Contents / data / behavior |
|---|---|
| Left preview | Cosmetic human proxy at top; name summary, body/presentation, hair, skin, starter appearance IDs. Previous/Next appearance actions below proxy rotate/select cosmetically; no source-image embedding. |
| Step 1: identity | Middle: Name field + gamepad keyboard; four 72px option rows for body, hair, skin, starter appearance; explanatory “Appearance does not change attributes.” Each selector uses left/right; owner supplies compatible choices, no unbounded invented catalog. Continue moves to Q1. |
| Steps 2–5: questions | Middle: “Question n of 4,” supplied theme/prompt, five answer rows, selected check, Previous / Next. Each answer stores stable question/answer IDs. Prompt can scroll in reading mode; answer rows scroll if long. Show approved affinity explanation only when supplied, otherwise “Affinity effect unknown” in developer detail. |
| Four-answer progress | Top of middle pane: Q1–Q4 indicators, with check/current marker. This is a progress display, not a bypass around unanswered questions. Exact theme wording/order and choice labels are content data. No invented numeric affinity mapping; disputed owl mapping remains Disputed in provenance. |
| Step 6: roll/review | Middle: Roll (then Re-roll), accepted-roll summary, legal/build validation summary, starter possessions and initially learned skills/spells from preview. Details reading mode scrolls the full kit/requirements. None learned is shown only for an explicitly resolved empty set. |
| Right legal review | Five rows in canonical order: Strength, Endurance, Agility, Intelligence, Wisdom; rolled value and provenance. Below: unspent attributes, initial HP/MP, gold, skill points as expandable details; “Starter weapon: legal / unmet / unknown.” Bottom: Review/Confirm button and named error. Right pane is read-only until review step. |

Before roll, show “Not rolled” for results, not zero. If required rules remain unresolved, show “— Unknown,” “Prototype roll mode — awaiting rules data,” and keep Roll/Confirm focusable but unavailable. When approved prototype data arrives, show `Prototype` next to the roll group with generation policy/revision in Details. Do not use research maximum examples or R-01 proposal values as defaults. Required starter legality is evaluated by authority for the complete result. Confirm is available only after every answer, legal name, compatible appearance and accepted current roll validates.

## Full input flow

Initial focus Name. South opens shared in-game keyboard; Done returns to Name, down through appearance rows → preview rotate controls → Next. On each question initial focus current answer or first answer; up/down chooses focus, South selects (does not advance), then Next is explicit. Tab follows prompt Details → five answers → Previous → Next. LB/RB go previous/next **completed** creation stage; cannot skip validation. East/Escape returns previous step. At first step Back opens “Discard this draft?” with Keep Editing default / Discard; no slot is consumed.

On review: Roll/Re-roll → Starter/Rules Details → Confirm → Back. X is a labelled Re-roll shortcut only on review; every press is one new preview intent, repeated presses while pending ignored. Re-roll replaces pending roll only. Editing any answer/appearance invalidates dependent preview and disables Confirm until revalidated; preserve other fields. Returning from review retains accepted preview if inputs/rules revision did not change. Confirm opens “Create [name] with these attributes?” modal, Back default / Create; committed creation cannot be cancelled. Initial-save failure keeps the created identity associated with the same request, offers Retry Save / Back to safe recovery flow, never a second creation. On success wait for durable snapshot and validated spawn before HUD.

Mouse/keyboard: field typing, radio clicks, option arrows, explicit Roll and Next buttons; Tab/Shift+Tab, arrows, Enter/Space, Escape, Q/E stage navigation mirror controller. No mouse-only mannequin drag; rotate has buttons. Long form errors do not replace controls.

## Errors and readability

`InvalidRequest`: “Check the highlighted fields.” Preserve name, appearance, four answers and any still-valid preview; focus first invalid field when a proposed validator provides field detail. Without detail use summary “Character could not be validated” and keep focus on Confirm. No invented `InvalidName` code. `UnresolvedRules`: unknown generation/starter policy; block commit. `Ineligible` / `InvalidEquipment`: starter/result legality failed; show owner detail, preserve answers, offer Re-roll or Previous. `ReusedRequestId`, `Busy`, `SaveRequired`: shared handling, no new character on retry. Other reasons use shared fallback. Name policy/length/filter is owner-supplied; local display names do not require online uniqueness.

Body/errors/options 30/20, supporting provenance 24/16; five stats use full names, not color-only abbreviations. Each invalid field has `!` + message and optional amber edge. Minimum 72/48 target height, two-line answers 108/72 and a scrolling list; no shrinking all five choices into tiny text. Unknown stat bars are omitted, not misleadingly filled.

## Acceptance scenarios to run

V01/V02/V19: gamepad keyboard, all four questions, changing earlier answer, reroll, invalid name with retained input, missing generation data, illegal starter result, cancel at each stage, repeated Create, initial save failure and recovery. Authority/data are prerequisites; this spec does not resolve creation RNG or claim parity.
