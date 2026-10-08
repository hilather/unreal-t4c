# Character select, continue and save recovery

[Wireframe](character-select.svg) · [Shared conventions](README.md)

## Purpose and authority

Choose one stable local CharacterId and load a validated coherent save. Session intents: list/inspect profiles, continue selected character, acknowledge recovery, retry load, delete selected profile. These interfaces and their statuses are **not in `LHCommands.h`** and require W2-01/02 integration. Never submit `FLHCreateCharacterRequest` to load a character or interpret `SaveRequired` as a corrupt-save code.

## Elements and layout

| Region | Contents and data |
|---|---|
| Header | Characters; back breadcrumb; profile count from local storage, no invented slot cap. |
| Left (72,174,576,720) | Scrollable 108px profile cards: name, earned level, area, compatibility badge; durable timestamp if known. Stable ID binds selection; name is not identity. Last valid selection gets initial focus. New Character at bottom y798. |
| Right (672,174,1176,720) | Name/appearance thumbnail at top; earned level, active area, ruleset/revision, durable save time and generation summary. Recovery banner (696,390,1128,180); summary (696,594,1128,108); Details control at y702; Continue / Delete row at y798. |
| Status / footer | Load state, selected row/total, Confirm / Back / Details. |

Normal banner: “Ready to continue.” Recovery banner: **“The newest save could not be read. An earlier valid save is available. Progress after this save may be lost.”** Show recovered timestamp/sequence only if the save owner provides them; do not promise a minute count. Continue label becomes “Continue recovered save.” Acknowledge in modal: “Load this earlier save?” with Back default / Load recovered save. Loading does not erase the failed generation automatically.

All-invalid: “No valid save found. Your save files have been kept.” Show Retry / Back / Details, disable Continue with explanation. Future schema/content mismatch: “This character needs compatible game data” with required/current versions when known; no silent reset/migration/new character replacement. Explicit owner-approved migration would require a separate reviewed confirmation, not a guessed path here. Unknown attributes/level use “— Unknown”; do not reveal numeric fixture defaults.

## Focus and input

Up/down through profile cards then New; Right moves to Continue, Down to Delete, Up to Details; Left restores selected card. Tab order is cards → New → Details → Continue → Delete → Back. Y opens Details reading mode. LT/RT page profile list. South selects card, a second deliberate Continue activates load; selecting never loads immediately. East/Escape returns frontend unless a child modal is open. Shoulders do nothing. Mouse single-click selects and explicit buttons act; keyboard arrows/Tab/Enter match.

Delete opens centered modal naming the profile, “Delete this character and its local progress?”; Keep Character default → Delete Character. No timed hold, typed-name or mouse requirement. Pending delete disables both repeated delete and switching profiles until outcome; success selects nearest remaining card, or New. Failure preserves list and focus. Back during load stays at a cancelable stage only if owner reports cancellation possible; otherwise show “Finishing load…” and wait. Never enable world input before snapshot validation.

## Errors and readability

Session failures are separate from `FLHCommandResult`: read failure, incompatible schema/content, write/delete failure, busy storage. State exact outcome; do not claim files were deleted if result is uncertain. Retry references the same profile, never an array index. On changed profile list refresh, anchor focus to stable ID or nearest surviving row. Recovery `!` icon + heading + explanation, 30/20 body, not amber alone; metadata 24/16. Recovery text wraps inside its reserved banner, with Details for long diagnostics. Shared contrast and minimum target sizes apply.

## Acceptance scenarios to run

Two saves with identical display names; newest generation truncated and older valid; both invalid; future schema; missing optional portrait; failed delete; rapid Confirm; controller-only recovery acknowledgment and safe deletion cancellation. No load/recovery behavior is implemented by this drawing.
