# Death and respawn

[Wireframe](death-respawn.svg) · [Shared conventions](README.md)

## Purpose and authority

Explain the already-authoritative death outcome and permit safe respawn after durability. Death transaction settles once, records consequence and safe checkpoint, commits, then respawns through session/gameplay owner. **There is no `FLHRespawnRequest` or native respawn reason code.** “Return to church” is a proposed session intent; never use `FLHRequestTravelRequest` with a fabricated portal or apply XP penalty from the screen.

## Elements and layout

Backdrop is opaque ink with optional dim world silhouette, no flashing blood texture. Header (72,54,1776,96): “You have fallen.” Main card (372,174,1176,720): name/area and authoritative cause if known; consequence summary at (396,318,1128,216); safe destination/durability status at (396,558,1128,96); Return to Church / Save Details / Return to Title controls at y690/y786 (primary alone then two secondary buttons). Status/footer use shared regions.

Consequence fields: earned level retained, separate XP debt/XP effect, gold/item loss, inventory preserved/changed according to the committed transaction, checkpoint destination, restored resource preview only if the owner supplies it. Unknown loss is “Consequence — Unknown”; **never assume all items lost, level decreased, or zero loss from absent data**. If the selected development profile explicitly provides no-penalty policy, show **“Prototype death policy: no penalty”** and the confirmed consequence; do not claim Classic parity. Prototype respawn values and destination marked when applicable. Do not recalculate maxima from current attributes.

## State sequence and focus

1. Settling: title and “Recording death…”; input context blocks combat; primary visible as unavailable, focus on Save Details. UI does not settle again on open/reload.
2. Saving checkpoint: “Saving return point…”; primary unavailable. Busy/durable status comes from session owner, not mere command acceptance.
3. Ready: “Checkpoint saved”; initial focus Return to Church (display actual validated safe destination name). South requests respawn once. Show “Returning…” and disable duplicate confirmation until safe placement/restore completed; then HUD after input release.
4. Failure: “Progress is not safely saved. Your last durable checkpoint is [summary].” Replace primary with Retry Save; provide Details / Return to Title. Return to Title opens explicit Leave Without Saving confirmation, Stay default, explaining old durable checkpoint may be loaded. No automatic respawn with uncommitted death state.

Up/down moves primary → Details → Return to Title; right/left between secondary buttons. South/Enter/click acts. East/Escape opens Return-to-Title confirmation (Stay default), never revives or resumes combat. Shoulders do nothing; Y Details. Details is scrollable by D-pad/stick or Page keys, East returns original button. Tab/Shift+Tab follows same order. Respawn cannot be cancelled after owner commits it; retain one request/session operation identity through retry. Reload reconstructs death state from authoritative snapshot, not animation completion.

## Errors and readability

Safe destination missing: “Safe return point unavailable,” primary disabled, Details and Return to Title retained. This is a respawn/session validation status; do not invent an `InvalidDestination` response on a nonexistent respawn request. If a previous gameplay request arrives with `InvalidLifeState`, route here without replaying death. `Busy`/`SaveRequired` on such a command use shared messages, distinct from storage result. Incompatible save recovery uses [character select](character-select.md), never silently resets the character. Unknown death policy disables gameplay settlement in authority, and UI shows “Awaiting valid death policy”; it does not approve a no-penalty fallback.

Body, consequences and errors 30/20; metadata 24/16. Death, saving, saved and failure each use heading plus icon/shape. Opaque card meets shared contrast, dim scene cannot obscure loss summary. No red-only cues, screen shake or timed choice. Long item-loss details scroll while primary/exit remain fixed.

## Acceptance scenarios to run

V10/V11/V12: death transaction once, repeated Return, save failure then retry, leave unsaved acknowledgment, missing safe checkpoint, reload while dead/travelling, preserved inventory and boss/quest flags, no repeated level entitlement while debt recovers. No respawn/build/play evidence is claimed.
