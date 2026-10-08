# Inventory and equipment

[Wireframe](inventory-equipment.svg) · [Shared conventions](README.md)

## Purpose and commands

Inspect owned items and equip/unequip through `FLHEquipItemRequest` (stable Item instance, explicit Slot, bUnequip). A swap is one authoritative equip intent; never clear the old item in UI first. Only the owner applies modifiers and slot compatibility. Item use requires W4-03 to resolve an instance-aware consumption adapter: `FLHUseAbilityRequest` alone does not identify which potion to consume. Display unavailable Use with explanation until that seam exists; do not invent `FLHUseItemRequest`. Sale happens only at a real vendor through [services](services.md).

## Elements and layout

Shared journal tabs; three body panes (426/726/576 widths). Left is equipped slots, middle inventory, right selected details/actions. Header shows authoritative gold and inventory count; encumbrance displayed only when profile includes and resolves it.

| Element | Data and behavior |
|---|---|
| Equipment list | Eight 72px rows: Main Hand, Off Hand, **Quiver**, Head, Torso, Legs, Feet, Accessory. Maps exactly to `ELHEquipmentSlot` excluding Unspecified. Each shows equipped item name or Empty; long names use detail pane. No cosmetic extra finger slots. |
| Inventory list | Filter row All / Equipment / Consumables / Quest is local presentation; item rows show name, owned quantity, equipped badge and stable selection. Sort by type then localized name then stable ID; no gameplay ordering implied. Scroll count visible. |
| Detail pane | Full item name, icon placeholder, quantity, slot, modifiers, eligibility minima/current comparison and requirement basis (Base / Effective / Unknown). Full requirements are a focusable scrolling region, never hover-only. |
| Quiver detail | “Wooden Arrows — Unlimited” only for that resolved definition; check equipped slot, not mere backpack ownership. Missing compatibility data shows Unknown. Do not display a decrementing arrow count or guess other quivers are unlimited. |
| Action row | Equip / Unequip according to selected item; Use for supported consumable; Details. Slot chooser opens when multiple **validated** slots exist; otherwise shows explicit target slot. |
| Status/footer | `! Requirements not met: …`, focus hints and list count; keep selected item visible after reject. |

Unknown price, weight, damage or requirements reads “— Unknown.” Research item candidates are not free starting grants. Prototype values carry a group/field badge. Unresolved requirement policy disables equip with “Requirement basis unknown,” not a false green check. Known unmet minima show “Requires [attribute/value]; you have [value]” from eligibility read model. Raw weapon range is labelled base weapon damage, never final hit damage.

## Focus and input

Initial focus last inventory row, or first equipped slot when inventory empty. D-pad up/down moves rows; left/right switches Equipment ↔ Inventory ↔ Details/action pane, restoring each selection. LB/RB journal tabs; X cycles list filter (explicit filter control also in Tab order). LT/RT pages. South on inventory row focuses primary action; second South requests equip after optional slot choice. South on equipment row focuses Unequip. Equipment swapping shows pending preview and requires explicit confirmation if another item is replaced; Cancel default. Back closes detail/slot modal before leaving journal. Mouse single click selects; buttons Equip/Unequip, not drag-drop required. Tab: filter → inventory rows → equipment rows → Details → Equip/Unequip → Use → Back; arrows preserve pane navigation.

After acceptance reselect the same stable item, refresh both inventory and equipment together. If item disappears, use nearest surviving row. Back during pending command follows shared wait/receipt rules. No Drop/Delete item action is introduced by this slice spec.

## Errors and readability

`Ineligible`: requirement detail; `InvalidEquipment`: slot/conflict/quiver explanation; `NotFound`: selected item no longer owned, refresh without choosing another for the same command; `InventoryFull`: unequip/swap rejected intact if applicable; `InvalidRequest`, `UnresolvedRules`, `InvalidLifeState`, `Busy`, `SaveRequired` shared handling. Use errors (`InsufficientMana`, `Cooldown`, etc.) apply only once a valid ability adapter exists; full-mana potion rejection has no dedicated enum, so do not invent `FullMana`.

30/20 row names and requirements, 24/16 metadata. Slots have both full labels and simple glyph placeholders; equipped check + “Equipped” separate from amber focus. Requirement unmet uses `!` and prose, not red. Unlimited quiver uses the word, not infinity symbol alone. Shared contrast; long requirement lists scroll and retain actions.

## Acceptance scenarios to run

V05/V18/V19: full inventory, failed equip preserving old gear, explicit quiver slot, backpack-only quiver fails shot, unlimited equipped quiver stays unchanged, stale item, all allowed creation outputs, controller filter/slot chooser. No item validation or play claim is made here.
