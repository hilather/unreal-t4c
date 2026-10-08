# Spell selection and quick-slot assignment

[Wireframe](spell-selection.svg) · [Shared conventions](README.md)

## Purpose and commands

Inspect learned spells, select an action and assign six quick slots. Casting submits `FLHUseAbilityRequest` (Ability, stable Target resolved by ability policy). Binding a slot is a proposed presentation/settings intent, **not** spell learning and not a new gameplay request. Persistent bindings require integrator schema/settings approval. Unlearned spell acquisition is through a positioned trainer and `FLHLearnSpellRequest` in services, never a Learn button here that grants knowledge.

## Elements and layout

Header: journal tabs, authoritative current/max MP. Left (72,174,576,720): Known / All filter, 108px spell rows with name, Learned/Not learned, castability and short provenance badge. Right (672,174,1176,720): selected name, targeting type, cast MP, range, cooldown, effect description, prerequisites, and teacher/location if provided; Details reading mode for overflow. Bottom right (696,678,1128,84): six numbered quick-slot cells (168px with 24px gutters); below at y798: Assign / Cast / Details actions.

Rows may include Light, Fire Dart, Heal Light, Stone Shard and Dust Devil **only when definitions exist**; names here are slice candidates, not a guarantee of initially learned spells. Assign and Cast are unavailable for unlearned or unknown-knowledge rows, with “Learn this spell at its trainer first”; teacher details remain inspectable. Learning gold/skill cost is separately labelled “At trainer”; casting MP must never be presented as learning price. Unknown costs/range/timing/effects show “— Unknown,” required unknowns disable Cast. No fake countdown for unresolved cooldown. Resolved prototype values marked `Prototype`; missing empty slot labelled Empty, not Unknown.

## Focus and input

Initial focus last selected learned spell, otherwise first row, otherwise Back. LB/RB journal tabs; X cycles Known/All, also reachable filter. Up/down list; Right opens detail actions; Left returns list. South on row selects only. Assign enters six-cell binding row, left/right chooses destination, South confirms cosmetic binding, East cancels. Replacing a binding shows displaced spell name and Replace/Cancel (default Cancel); it does not unlearn either spell. X on focused binding exposes Clear, confirmed explicitly. Every shortcut has a visible button. Tab order filter → spell rows → Details → Assign → Cast → binding cells → Back.

Cast opens targeting confirmation showing target/self from the adapter; if a target is required and absent, Close to Target is offered instead of sending an empty Target. Cancel default / Cast. On Cast close menu pause, clear held inputs and submit through authority with fresh validation; failure returns brief HUD feedback and retained spell selection. No cast executes while menu pause is still held. East closes a subpanel first, then journal. Keyboard arrows/Tab/Enter, Q/E journal tabs, Escape back; mouse uses explicit Assign/Cast/slot controls. Six hotkeys in gameplay match these assignments.

Quick access from HUD uses LT plus D-pad selection/South confirm, with Hold/Toggle behavior from the shared input contract; full menu above is another hold-free alternative. No analog radial precision is required, and releasing LT alone never casts.

## Errors and readability

`Ineligible`: not learned or prerequisites unmet; `InsufficientMana`: show current/needed from read model and normal-play recovery hint; `Cooldown`, `OutOfRange`, `Obstructed`, `NotFound`, `InvalidEquipment`, `InvalidLifeState`, `ActiveAction`, `Busy`, `UnresolvedRules` use shared mappings. `InvalidRequest` can indicate missing target adapter/policy; no widget may decide a target sentinel. Assignment errors are settings/presentation statuses, not `FLHCommandResult` reasons; retain previous binding if persistence fails.

30/20 spell names, requirements and action text; 24/16 slot numbers/metadata. Known check, unknown question mark plus word, locked icon plus “Not learned,” cooldown clock plus text. Full title remains in detail pane when slot label is abbreviated. Shared opacity/contrast; no blue-only learned state.

## Acceptance scenarios to run

Controller with zero known spells; preview unknown spell; assign/replace/clear all six slots; zero MP and zero gold recovery; correct self versus target policy; stale target after closing menu; input-release gating; inspect learned state after reload. W4-03/05 must supply adapters before execution.
