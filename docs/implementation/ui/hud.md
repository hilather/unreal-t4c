# Gameplay HUD, interaction, loot and travel

[Wireframe](hud.svg) · [Shared conventions](README.md)

## Purpose and commands

Keep the world legible while exposing resources, target, actions and save durability. Attack/cast submits `FLHUseAbilityRequest` (Ability, stable Target). Talk/use submits `FLHInteractRequest` (Target, Topic). Loot pane submits `FLHTakeLootRequest` (Container, Item, resolved Quantity); portal confirmation submits `FLHRequestTravelRequest` (Portal, expected Destination). Target selection and quick-slot bindings are presentation/session intents, never damage or XP. Do not invent sentinel targeting or gold-only loot conventions; adapters must be frozen first.

## Elements and layout

| Region at 1080p | Display / data |
|---|---|
| Player frame (72,54,426,216) | Display name, current/max HP labelled Health, current/max MP labelled Mana; numeric values over separate neutral/blue-accent bars. Status effects use icon + name/countdown in Details. |
| Target frame (672,54,576,156) | Target display name/variant, life state, current/max HP if exposed; locked chevron. Hidden when no target, “Target lost” briefly on invalidation. Do not expose unknown enemy HP as a fabricated percentage. |
| Save frame (1422,54,426,156) | Saving… / Saved / Not saved; timestamp or last durable checkpoint if supplied. Spinner/check/! plus words. Accepted command is not evidence of disk persistence. |
| World clear zone (522,246,876,444) | No persistent panels; ground target brackets use shape plus name, legible against bright/dark geometry. Optional area/objective line sits below save frame; counts supplied by quest owner only. |
| Context prompt (522,714,876,108) | South/E + action verb + selected NPC, corpse or stair name. With competing targets, show “n of total” and D-pad left/right context cycle; no nearest-enemy ambiguity. |
| Quick slots (522,846,876,84) | Six 126px slots, 24px gaps; slot number, short ability name/icon, MP cost and cooldown when resolved, lock/empty/cannot-cast marker. Full name/requirements live in selection view. No tiny icon-only cues. |
| Resource/action message (72,846,426,96) | Last rejection, up to two body lines; longer explanation opens Details in pause/journal. Never obscure slot labels. |
| Footer (72,954,1776,72) | Attack / Interact / Quick slots / Journal-Pause legend, adapting to device and action. |

Full-width shared status strip is not used while slots occupy that band; save failures remain persistent in the save frame. Unknown HP/MP show “— Unknown” and outline-only bars; prototype recovery/cost detail carries its badge. No stamina bar. Earned level/XP/debt are in Character, avoiding a false promise of a sourced XP curve on the HUD. Boss victory notice is a published event, not another reward trigger.

## Full input flow

Shared gameplay map applies: stick move/camera, R3 lock, LB/RB target cycle, RT attack, South interact, LT quick selector, View journal, Menu pause. Context cycling uses D-pad left/right only outside LT selector. Mouse selects target by click, right-click attack, E interact; Tab/Shift+Tab cycles next/previous target, F toggles lock; 1–6 submit quick slots. Escape pauses. View menu tabs give gamepad access to character, inventory and spells without keyboard shortcuts.

LT opens six-slot horizontal overlay in the slot region, pauses via session owner and highlights last slot. D-pad left/right selects, South closes pause then requests ability; in Hold mode release/East cancels, never casts. In Toggle mode second LT or East cancels, release does nothing, South confirms. Unlearned/empty slots are focusable with explanation. Full spell-selection menu is another hold-free alternative. Context switch consumes held inputs. Selected ability requires authoritative target-policy adapter (self/target/ground if supported), not a guessed empty Target.

Loot interaction opens modal in shared modal rectangle, pauses, lists finalized remaining items with quantity and an explicit Take control; focus rows → quantity stepper → Take → Close. South selects row, left/right changes quantity within authority bounds, LT/RT pages, East closes. No “Take all” batch seam is invented. `InventoryFull` offers Open Inventory/Back and leaves contents untouched; restore corpse selection on return. Gold row reads “Collection unavailable” until gold-only request convention is frozen; never invent an Item sentinel. Already looted/removed corpse uses `NotFound` and closes safely.

Stair interaction opens destination panel with Stay default / Travel. On Travel use `FLHRequestTravelRequest`; freeze further interaction while source save/load/arrival save complete. Pre-save failure offers Retry Save / Cancel at source. Invalid destination/load failure shows return-to-source state; do not show “Arrived / Saved” before both validation and durability. No travel reward. Shared session lifecycle owner provides progress; HUD does not move the character.

## Errors and readability

`InsufficientMana`, `Cooldown`, `InvalidEquipment` (including equipped-quiver requirement), `Ineligible`, `InvalidLifeState`, `ActiveAction`, `OutOfRange`, `Obstructed`, `NotFound`, `UnresolvedRules` map to shared copy. Failed attacks retain target when valid; no automatic repeated cast/charge. Zero mana hint: “Mana recovers during active play; check recovery details.” Never promise recovery while paused/offline. `SaveRequired`, `Busy`, `InvalidDestination` keep travel fence and expose Retry/Cancel only when safe. Save-write failure itself is a session status, not a new command enum. Y in paused Details exposes full reason when short HUD message is insufficient.

Resources/action text 30/20; slot supporting numbers and footer 24/16. Full names and detailed costs are in a focusable overlay, not hover tooltips. Health uses HP/Health + shape; mana uses MP/Mana + shape; low-resource warning adds text without flashing. Opaque frames preserve shared contrast over torchlight. No guessed fraction for unknown maxima.

## Acceptance scenarios to run

V06/V09/V11/V14/V17/V18: target lost, wall/range boundary, empty mana, missing quiver, unlimited quiver, full inventory, stale corpse, failed pre-travel save and load, rapid interact, controller switch and releasing held attack on menu entry. Actual in-engine camera/readability/input checks remain unrun.
