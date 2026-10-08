# Frontend

[Wireframe](frontend.svg) · [Shared conventions](README.md)

## Purpose and authority

Entry to local play. New Character opens creation with an unused local slot selected through the profile owner; Continue validates the last selected profile and opens [character select](character-select.md) for confirmation/recovery. Characters opens that same screen without precommitting a load. Settings opens [pause/settings](pause-settings.md) in frontend mode. Quit requests application exit. These are session/navigation intents with **no existing `FLHCommandResult` request**; interfaces belong to W2-01/03. This screen neither creates a record nor awards progress.

## Elements and layout

| Region at 1080p | Contents and data |
|---|---|
| Header (72,54,1776,96) | Lighthaven title; “Single-player”; optional build/profile label in developer metadata. |
| Left (72,174,576,720) | Five 72px controls at y270/366/462/558/654: New Character, Continue, Characters, Settings, Quit. Continue subtitle shows last profile display name or “No saved character.” |
| Right (672,174,1176,720) | Reserved original world/character composition as a blank wireframe field; lower inset card (696,606,1128,240) shows last profile name, earned level, area, durable save time if available. No downloaded promotional image as runtime background. |
| Status / footer | Profile scan / no saves / unavailable storage; shared input legend. |

Continue stays focusable with “No valid save available” when empty; South then explains and offers New Character. Do not select a corrupt profile merely because its timestamp is latest. A scan-pending card says “Checking local saves…” and does not display stale stats. Missing optional portrait uses a labelled silhouette, not missing-progress assumptions. Unknown level reads “Level — Unknown”; pending profile metadata is not a zero-level character. Developer `Prototype` profile badge is separate from the product title.

## Focus and input

Initial focus Continue when last profile is validated and compatible, otherwise New Character. Up/down follows New → Continue → Characters → Settings → Quit; no wrapping. South/Enter/click invokes focused action. East/Escape at root opens Quit confirmation; shoulders have no action. Quit modal presents Stay then Quit; default Stay. Back returns to the launching control after closing settings/characters. Background/card is informational; Y opens read-only profile details if available. No mouse hover is needed.

## States and errors

Profile scan failure: “Local saves could not be read” with Retry / Characters / Settings; no destructive repair. Storage permission or disk errors are **session errors, not command reason codes**. Continue load/recovery/incompatibility follows character-select spec. Quit during profile scan may close safely because no mutation is running; quit during a creation/save operation routes to the shared pending/save-failure flow, never silently discards. Generic unexpected session error offers Retry or Back and preserves profile identity.

Readability: 30/20 controls and 24/16 supporting text; opaque card over any future art. Disabled Continue keeps normal legible text plus a lock and explanation, never opacity alone. Focus uses chevron plus amber outline.

## Acceptance scenarios to run

Cold start with no profiles; valid Continue; damaged latest profile; all saves unreadable; enter settings and return using only gamepad; root Back and safe Quit cancellation. No engine checks have run for this design.
