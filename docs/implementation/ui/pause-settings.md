# Pause and settings

[Wireframe](pause-settings.svg) · [Shared conventions](README.md)

## Purpose and authority

Pause local simulation and expose save/exit and accessible input/audio options. Semantic session/settings intents: pause, resume, save at completed boundary, exit to frontend/application, preview/apply/revert settings, capture/rebind input. **No corresponding native request types or `FLHCommandResult` reason codes currently exist.** W2-01 and UI/session owners must supply them. Widgets never serialize snapshots or advance timers. Frontend Settings uses the same body with no Resume/Save/Return to Title.

## Elements and layout

Header “Paused” (or “Settings” from frontend) plus save state; subtitle “Simulation paused” only after session confirms pause. Left (72,174,576,720): Resume, Save, Settings, Return to Title, Quit Application, 72px controls. Right (672,174,1176,720): tabs Audio / Input / Controller; scrollable control list; fixed Apply / Revert / Restore Defaults row y798. Status strip reserved for errors and binding capture. Footer hints retain Confirm/Back/Tabs.

| Tab / state | Elements and data |
|---|---|
| Audio | Master, Music, Effects and Dialogue volume steppers/sliders with percentages 0–100, authored UI range, minimum 72px hit height. Value text always visible; left/right changes by 5 percentage points as interface choice. Preview through audio-settings owner; Revert restores previous applied values. Missing audio channel reports Unavailable, not a fake control. |
| Input | Device mode Automatic / Keyboard & Mouse / Controller glyphs (presentation preference only); remapping action list for move, camera, run, target, attack, interact, quick slots, journal, pause; toggle/hold preferences where the shared map supports them. Capturing displays current + proposed binding and conflicts. |
| Controller | Connected-device label and active glyph family, stick sensitivity, invert camera Y toggle, vibration toggle, quick-selector Hold / Toggle preference. Sensitivity value/bounds/step are settings-owner metadata; do not assume gameplay speed changes. Restore defaults limited to this tab. |
| Controls sheet | Focusable list of current semantic gameplay and menu bindings, scrollable with D-pad; full chords/contexts visible, including shoulder conflict resolution. |
| Save/exit | Last durable checkpoint/time from storage, dirty/pending/failed status; Save enabled only when session permits completed action boundary, explanation otherwise. |

Settings percentages are interface controls, not game mechanics. Unavailable settings show “Unavailable in this build”; unknown current sensitivity reads “— Unknown” and cannot apply. Do not expose nonfunctional resolution/graphics or account/cloud controls. Volume preference changes do not alter character/save progression.

## Focus and input

Pause initial focus Resume; frontend Settings initial focus first setting. Left nav up/down, Right enters active settings tab, Left returns nav except on an active stepper; South enters/exits slider adjustment so Left has no ambiguous meaning. LB/RB tabs, Tab follows nav → tab controls → Apply → Revert → Defaults; arrows adjust, Enter confirms, Escape backs, Q/E tabs. Mouse can drag sliders but −/+ controls and arrow keys provide equal access. Y opens Controls sheet. LT/RT pages long binding lists.

Unsaved settings draft on Back/tab or exit: Keep Editing default / Discard Changes; Apply is explicit. Capture rebind has a reachable Cancel button and East/Escape; to bind the reserved cancel button, use an explicit “Capture cancel button” subcontrol. This special mode listens only for that reserved target button; D-pad still navigates and South activates the focused Cancel control, so cancellation cannot be swallowed as a binding. Conflicts show Replace / Cancel, Cancel default. Never remove the last usable menu Confirm/Back or Pause binding; offer Restore Defaults via gamepad without capture. Duplicate gameplay inputs must be separated by context. Remapping UI values uses the same settings owner as gameplay, no direct Enhanced Input widget side effects.

Return/Quit with dirty progress opens Save and Leave / Stay / Leave Without Saving; initial focus Stay. Save and Leave remains pending until durable completion; failure shows **“Progress is not safely saved”**, last durable checkpoint and Retry / Stay / Leave Without Saving. Last action needs explicit second confirmation naming loss since checkpoint, Stay default. Back from in-flight save cannot undo a completed gameplay transaction. Do not write a competing save while one is active. If SaveRequired is received by another gameplay command, it navigates here but does not masquerade as a storage failure code.

## Errors and readability

Session statuses distinguish save busy/failed, disk unavailable, settings apply failed, binding conflict and disconnected controller; none invents an enum member. Revert/apply failure keeps previous applied settings authoritative and draft visible. Disconnect requests pause with reconnect/keyboard options; focus preserved, never auto-resume. Missing settings backend shows explanatory unavailable row.

30/20 controls, values and save errors; 24/16 device/build metadata. Percentages and toggle On/Off words complement track/check shape, no color-only state. Shared contrast and 72/48 targets. Never require precision dragging, tiny knobs or timed rebind responses.

## Acceptance scenarios to run

V11/V14/V17: real pause stopping timers; save failure and deliberate unsaved exit; conflict/cancel rebind; gamepad-only recovery of controls; volume preview/revert; disconnect/reconnect; settings return restores focus. Linux input/audio integration and real saves are required before these can pass.
