# NPC dialogue

[Wireframe](npc-dialogue.svg) · [Shared conventions](README.md)

## Purpose and commands

Discover selectable topics, quests, healing and services. Submit `FLHInteractRequest` with stable NPC Target and authored Topic ID. Selecting an acceptance/turn-in topic requests the authority-owned transaction; the widget never increments kill counters, sets quest flags, heals or awards XP/gold. Existing dialogue text and availability come from a dialogue read model; “Interact” alone does not establish all predicates or reward amounts.

## Elements and layout

Header: NPC display name, role, source-positioned area if supplied. Left (72,174,576,720): original portrait/proxy, role, context/quest summary, optional service shortcut. Right (672,174,1176,720): transcript panel (696,258,1128,240), followed by scrollable topic list (696,522,1128,360) with 72px controls and 24px gaps. Footer contains Select / Back / Read transcript. Shared status strip keeps rejection visible.

Transcript shows speaker label and authored response text; optional historical keyword shown as supporting metadata for each topic, never free typing required. Topics include available quest/service choices from data, with selected chevron and textual state: New / In progress / Ready to turn in / Completed / Unavailable. Locked topics remain inspectable with supplied reason; don't disclose future branch spoilers from arbitrary hidden data. Last row is Leave. More topics scroll without moving transcript/footer.

Quest preview binds acceptance state, eligible progress, target objective, exact reward preview if resolved, and already-claimed state. Kill counts and corpse collection are different objectives, never merged by wording. Balork defeat, dialogue, mark, completion and claim are separately represented by the owner's allowed summary; do not say “Original quest complete” for authored slice victory. Unknown reward shows “Reward — Unknown”; prototype group marked Prototype. Unknown required predicates disable submission rather than showing a zero reward or silently opening a branch. A confirmed reward receipt is a published result, never a new interaction from the widget.

## Focus and input

Pause on open. Initial focus first available topic or remembered stable Topic ID. Up/down moves topic rows, South selects; ordinary informational topics request response, service topics open [services](services.md) after accepted interaction. Quest acceptance/turn-in opens review modal containing objective/reward/cost if any, Cancel default / Accept or Turn In. Y enters transcript reading mode; up/down scrolls transcript, East returns to selected topic. LT/RT pages topic list when list focused. Tab: Read Transcript → topics → service shortcut → Leave; mouse clicks explicit choices and scroll controls, keyboard arrows/Enter/Escape equivalent. Shoulders do nothing (service subdialog has its own tabs).

Back closes child confirmation first, then dialogue to HUD; restoring input waits for held buttons to release. Refresh response/topic list after accepted result, keeping stable topic focus or nearest surviving row. Pending interaction prevents duplicate Confirm and branch switching. No keyword-only path, timed dialogue wheel or mouse-only scrolling.

## Errors and readability

`NotFound`: NPC/topic no longer present; “This conversation is no longer available,” Close. `OutOfRange`, `Obstructed`: close-to-world option, no automatic movement or healing. `Ineligible`: unmet quest/service condition, preserved transcript; already-claimed state displays Completed based on view model, never a fabricated duplicate-reward error enum. `InvalidLifeState` routes death; `Busy`, `ActiveAction`, `SaveRequired`, `UnresolvedRules`, `InvalidRequest`, `ReusedRequestId` shared handling. Optional `InsufficientGold` or `InsufficientPoints` only where a real interaction service returns it; a dialogue line cannot deduct directly. Save failure after turn-in marks Not saved without offering a second reward intent.

Dialogue/choices 30/20, metadata keywords 24/16. Speaker names bold; full transcript scrollable, no auto-advance timeout. Quest states use words and distinct markers, not colored bullets alone. Portrait cannot reduce contrast; text stays on opaque surface. Full reward prose lives in confirmation/Details when it exceeds one row.

## Acceptance scenarios to run

V08/V12/V14: no accepted quest, progress after acceptance, eligible turn-in, repeated turn-in and reload, missing topic, scroll long response, service round-trip and gamepad focus, Balork dialogue/defeat state differences. Quest predicates/content must be supplied by W4-05.
