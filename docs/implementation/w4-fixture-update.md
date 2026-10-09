# W4-06a6 Stage 1 fixture updates

- `Lighthaven.Data.RuntimeSpecsValidate`: reset the fixture item definitions and populate exactly the native catalog once; the production profile already supplies these rows, so appending duplicated IDs prevented all eleven reward-validation cases from executing. Runtime validation, unresolved rejection, byte rollback and successful settlement assertions remain.
- `Lighthaven.Rewards.FullInventoryLootIntact`: set this fixture's capacity to its starter-entry count before creation and assert all slots are occupied; the Stage 1 capacity no longer supplies the old two-slot full-inventory precondition. Item rollback, gold transfer despite full slots, replay and recoverable corpse assertions remain.
- `Lighthaven.Rewards.PlayerDeathSettlesOnce`: derive HP/MP maxima through a separately initialized/imported character authority before death; full recovery must match the tested profile rather than the old synthetic 20 HP/10 MP constants. Safety rejection, duplicate rejection, byte identity and retained character-field assertions remain.
- `Lighthaven.Rewards.RenewableRatRoute`: capture initial gold and XP, then assert exactly 30 gold and 150 XP earned over fifteen kills; a populated starting wallet is separate from renewable loot earnings. Slot counts, settlement, pickup, respawn and reload assertions remain.

No production paths changed; no mechanics values or provenance introduced. Fixture capacity is test tuning only. These tests exercise domain state invariants, not playable map safety or historical balance.
