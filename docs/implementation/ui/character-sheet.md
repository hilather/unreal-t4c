# Character sheet and attribute allocation

[Wireframe](character-sheet.svg) · [Shared conventions](README.md)

## Purpose and commands

Review progression and stage permanent attribute allocation. Submit `FLHAllocateAttributePointsRequest` with named resolved nonnegative deltas, including explicit resolved zeros for unchanged attributes. UI stages deltas only; authoritative preview supplies resulting values, remaining pool and eligibility. Commit never modifies XP or grants level points from a UI callback. Read-only stats and learned skills come from character/derived view models.

## Elements and layout

Header: Character / Inventory / Spells tabs, with underline and “Selected.” Body uses (72,174,1176,720) plus (1272,174,576,720). Status/footer follow shared geometry.

| Element | Data / positioning |
|---|---|
| Character summary | Left top: display name, earned level, retained XP balance, separate XP debt, next threshold/progress supplied by owner. Unknown curve shows “Next level — Unknown.” No bar calculated from ambiguous ledger debt conventions. |
| Attribute table | Headers at y378; rows start y408: five 72px controls with 12px gaps in order Strength, Endurance, Agility, Intelligence, Wisdom. Columns: Attribute / Base / Gear / Effects / Current / Add. Each Add control has Minus, staged delta, Plus; row focus left/right adjusts, no mutation until confirm. |
| Point pools | Right top: unspent attribute points, pending spend and remaining preview; skill points in a separate labelled block. No “all points” combined currency. |
| Resource/derived details | Right middle: current/max Health and Mana, earned historical maxima, accuracy/avoidance/armor/capacity when provided; long list via Details reading mode. Unknown values use “— Unknown”; absent profile stat says “Not used.” |
| Knowledge/progress details | Focusable Details opens learned skills + ranks, learned spell names and permanent flags; display Balork mark separately from boss defeat/quest completion. No inferred reward. |
| Bottom actions | Reset draft / Confirm Allocation in right pane at y702/y798; confirm is focusable but unavailable until legal nonempty draft. |

All existing canonical values remain visible while preview pending, with “Preview pending” replacing proposed totals. Unknown growth formulas do not become “next level HP” estimates. Approved prototype derived values display `Prototype`; source details use shared provenance viewer. Column totals are authority supplied, including temporary effects; the current rules module may not yet expose all columns, so missing modifier data remains Unknown, not invented zero.

## Focus and input

Pause journal on open. Initial focus first attribute (or retained row); LB/RB Character/Inventory/Spells. Up/down through five attribute steppers → Details → Reset → Confirm; Tab same. Left/right on a stepper decreases/increases pending delta only within preview-validated pool; South opens explicit Minus / Plus controls for accessibility, East closes them. No negative allocation or undo of previously committed points. Mouse uses labelled −/+ buttons with 72px hit areas; keyboard arrows or buttons act identically.

Confirm opens summary modal listing each changed attribute and remaining pool; Cancel default / Apply Permanently. `FLHAllocateAttributePointsRequest` sent once on Apply. Reset discards local deltas only. Back or tab switch with pending deltas opens Keep Editing default / Discard Draft; after discard continue intended navigation, retaining tab focus. No silent apply when switching tabs. Successful commit refreshes snapshot, clears draft and returns focus to first changed row; rejected commit preserves draft for repair.

## Errors and readability

`InsufficientPoints`: “Attribute point pool changed. Review this draft,” show refreshed pool; do not silently cut deltas. `InvalidRequest`: illegal/missing delta; highlight row only with supplied validation. `UnresolvedRules`, `Ineligible`, `InvalidLifeState`, `Busy`, `ReusedRequestId`, `SaveRequired` use shared handling. Prior canonical state stays intact on rejection. Unknown maximum/requirements show unavailable preview, not false guaranteed build eligibility.

Use 30/20 for full stat names and numeric columns, 24/16 for headings if necessary; never shrink numbers to fit. Expanded Details repeats full column meanings at 720p. Pending additions have `+` plus “Pending,” equipment/effect values have signed numbers and named columns, so color is unnecessary. Shared opaque surfaces/contrast apply.

## Acceptance scenarios to run

V03/V04/V05/V14: insufficient pool, no points, changed pool during draft, confirmation replay, cancel/tab switch, historical growth/debt display after load, keyboard/controller continuity. Rendering is specified; progression and allocation execution remain outside V-02.
