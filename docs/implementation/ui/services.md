# Trainer and vendor service dialogs

[Wireframe](services.svg) · [Shared conventions](README.md)

## Purpose and commands

An interaction at a real nearby NPC opens only that NPC's supplied offers. Training submits `FLHTrainSkillRequest` (Trainer, Skill, requested Points); spell learning `FLHLearnSpellRequest` (Trainer, Spell); buy `FLHBuyItemRequest` (Vendor, Offer, Quantity); sell `FLHSellItemRequest` (Vendor, owned Item instance, Quantity). Requests carry no client-picked price or acting character. Actual proximity, offer freshness, skill/gold pools, eligibility, inventory and equipment are validated together; no partial charge. Closing a service cannot give a reward.

## Elements and layout

Header shows NPC name/location and appropriate tabs: Skills / Spells for trainer, Buy / Sell for vendor; a combined NPC can show all supplied capabilities, never invented stock. Balances **Gold** and **Skill Points** are always separately visible. Left (72,174,576,720) holds offers with name, owned/learned state, availability, short cost. Right (672,174,1176,720) holds detail and requirement comparison, quantity/points stepper, before → cost → after quote, confirmation button. Persistent status at y900 carries the last reason; footer has tabs/confirm/back/details.

| Mode | Detail fields / primary label |
|---|---|
| Skills | Current trained rank, requested training points, predicted rank, gold cost and skill-point cost from quote, applicable cap and prerequisites / Train |
| Spells | Learned status, level/attribute/skill/spell prerequisites, learning gold + skill points, **separate cast MP** / Learn |
| Buy | Vendor offer ID, item name/quantity, buy unit price, stock if tracked, quantity stepper, total gold, resulting owned count and capacity eligibility / Buy |
| Sell | Owned item ID/quantity, equipped/quest/sellable state, sell unit price distinct from buy price, quantity stepper, total proceeds, remaining quantity / Sell |

The quote is authoritative read-only presentation with freshness; UI does not multiply unresolved prices or assume “one point = one rank.” Step increments/bounds come from owner policy. Unknown price/rank conversion/prerequisites read “— Unknown” and disable primary commit. Resolved zero price reads Free only with explicit zero; unknown never free. Ledger prototype proposals do not approve stock/prices. Each numeric group labelled Prototype when applicable; Details exposes supplied sources. Teacher names/locations come from data; no global remote training or relocated mage-tower teacher.

## Focus and input

Pause on open, initial focus first/last offer. LB/RB switches capability tabs, returning to remembered offer. Up/down browses offers; Right moves to quantity/points (if applicable), then Details, then primary button; Left returns offer. South on row selects, on stepper enters adjustment (left/right change); East leaves adjustment. LT/RT pages list; Y Details reading mode. Tab order tabs → offer rows → amount → Details → primary → Close. Mouse/keyboard explicit controls mirror this, with Q/E tabs and Escape back.

Primary opens modal containing item/skill/spell, quantity/points, gold debit/credit and skill-point debit, expected outcome. Cancel default / Confirm. Refresh quote on modal entry; changed price/offer requires a new review, never automatic acceptance. Disable duplicate Confirm while owner completes command. Accepted result refreshes all balances/stock/knowledge together, with one receipt message (replayed result does not replay message). Continue browsing same offer; sold-out/removed offer picks nearest neighbor. East closes modal then service back to NPC dialogue, then HUD. No drag, hover or double-click requirement.

## Errors and rejection mapping

| Reason | Contextual feedback and retained state |
|---|---|
| `InsufficientGold` | “Not enough gold for this purchase/training.” Retain offer/amount, refresh quote and balance. |
| `InsufficientPoints` | “Not enough skill points.” Attribute pool is never offered as payment. |
| `Ineligible` | Requirements unmet/already learned/unsellable per owner detail; do not invent `AlreadyLearned` reason. |
| `InvalidEquipment` | Selling equipped item disallowed or slot policy failed; offer Return to Inventory, never auto-unequip. |
| `InventoryFull` | Purchase unchanged; offer Inventory/Back with vendor context retained; recheck proximity on return. |
| `OutOfRange`, `Obstructed`, `NotFound` | NPC/offer no longer usable. Close to world or refresh, never silently substitute vendor/offer. |
| `UnresolvedRules`, `InvalidRequest` | Unknown cost/amount/policy or invalid request; keep entered quantity and show generic summary if no field detail. |
| `InvalidLifeState`, `ActiveAction`, `Busy`, `SaveRequired`, `ReusedRequestId` | Shared lifecycle/durability/receipt handling. |

Rejected transaction applies no gold/point/item/knowledge change. Session pause is not proof of eligibility; authority always rechecks. Pending result cannot be cancelled by switching tabs. A failed subsequent save shows Not saved, distinct from a rejected purchase.

Readability: costs and both currencies 30/20 with unit labels; metadata 24/16. Requirement comparisons use “Met / Unmet / Unknown” and shapes, not red/green. Negative gold debit and positive sell proceeds include words. Opaque quote and 72/48 controls follow shared contrast.

## Acceptance scenarios to run

V05/V18: skill cost scaling unresolved, insufficient gold/points independently, stale quote, already learned spell, full inventory, equipped-item sale rejected, missing quiver purchase/equip route, service range loss, repeated Confirm, save failure after accepted transaction. No real transaction has run here.
