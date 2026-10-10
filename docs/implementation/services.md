# W4-07 trainer and vendor services

Native catalog and snapshot transactions; contract revision 1. No session wiring, UI, placement, schema, or binary changes. Stock is unlimited and not persisted (Matt Q6, 2026-10-09). Persisted stock, restock, or buyback needs a later schema revision; revision 2 covers UseItem only.

## Catalog and placement contract

Offer IDs are `Offer.<NPC-name>.<subject-ID>` (for example `Offer.Sigfried.Item.AshwoodFlatbow`). NPC DefinitionId determines availability; the resolved actor entity must equal the request Trainer/Vendor and belong to the campaign RunId. Existing NPC transforms remain unchanged.

| NPC DefinitionId | Existing area / anchor | Offers |
|---|---|---|
| NPC.Kilhiam | Temple in temple district | Spell.Light |
| NPC.Moonrock | Temple in temple district | Spell.HealLight |
| NPC.Iraltok | Mage tower | Spell.FireDart |
| NPC.Uranos | Mage tower | Spell.StoneShard; no bat-wing quest |
| NPC.Shovanis | B1 healer chamber, existing B1.NPC.Shovanis | Spell.DustDevil |
| NPC.Murmuntag | Existing town training marker | Skill.Attack |
| NPC.Ortanalas | Existing town training marker | Skill.Attack, Skill.Archery |
| NPC.Kalastor | Existing town training marker | Skill.Dodge, Skill.Archery |
| NPC.Sigfried | Existing town weapon shop | Item.AshwoodFlatbow 29 gold, Item.WoodenArrows 100 gold |
| NPC.Fali | Existing town shop | Item.PotionOfMana 50 gold |
| NPC.Rolph | Existing town shop | None: stock/prices unresolved |
| NPC.BrotherKiran | Temple | No service offers; quest topics are W4-05 |
| NPC.JagarKar | Existing town marker | None: no invented offers |
| NPC.Nevanis | B1 healer chamber, existing B1.NPC.Nevanis | No Train/Learn/Buy offers; healing belongs to W4-05/06 |

W4-06 must replace the two B1 TargetPoints with ALHInteractableMarker instances at the same generator transforms: Nevanis grid (-25,6,0), Shovanis (-25,14,0), using the generator's grid-to-world conversion. DefinitionIds are NPC.Nevanis and NPC.Shovanis. Mint fresh literal instance GUIDs in the generator once (not per generation), preserving them on subsequent runs. Keep interaction pads free of arrivals and walk strips. Re-run arrival review and world validation after generator changes. Iraltok/Uranos stay at the mage tower; no NPC relocation is requested. CatalogSourcePositions tests catalog area metadata, not physical map placement.

## Authority seams

`LHServices::Execute(context, snapshot, request)` overloads train, learn, buy, and sell. Context supplies a trusted actor-resolved NPC DefinitionId/entity, measured distance, LOS, busy state, profile and ability/item lookups. W4-06 must supply fresh live health and gate pending/publishing combat actions in bBusy. The integrator must resolve NPC identity from its registered marker, never caller input. Context defaults reject when profile/LOS is absent.

Transactions mutate a copy and initialize/import a temporary character authority with the supplied profile before swapping. They do not write receipts or advance sequence. W4-06 wraps BeginRequest → Execute → CommitRequest → durable save, publishing only after acceptance. Same RequestId replays without invoking the domain; a new ID buys/trains again. Character helpers were unnecessary.

Train Points means ranks purchased; shared skill points and gold are charged per rank. Learn uses the ability lookup's acquisition requirements and costs, independent of cast mana. Base attributes are used, prior spells/skills and minima go through the existing rules requirement evaluator. Relearning rejects. Buy validates registered item data, stack limits, entry slots and Import validation; creates fresh inventory instances in the vendor area, splitting quantity by stack limit. It does not require equipment eligibility to purchase. W4-02 must expose matching item IDs and equipable/unlimited WoodenArrows data. Sell accepts only actual stocked vendors, only resolved RustedDirk sell price 9, rejects equipped instances, checks quantity and overflow, then removes the sold quantity. Other item sell prices return UnresolvedRules.

`Offers(Npc, snapshot)` is a catalog-only read model: prices, one-rank/shared-pool eligibility, spell prerequisites/relearning and life state. It cannot know live distance/LOS/busy, the profile's inventory limit, or external item lookup. UI/session must use Execute as authority and supplement capacity/spatial status through their own context; this read model is not permission to transact.

## Evidence and Prototype policy

All evidence is documentary, not verified in play. R-03 and rules/world ledgers are the supplied bounded lookup; no web research performed in this task.

- Spell costs and minima come only from LHAbilityCatalog (Light 233 gold/5 points, FireDart 532/5, HealLight 897/9, StoneShard 1328/6, DustDevil 2388/7). Source: https://www.t4cbible.com/Spells, retrieved 2026-10-09, live version unstated; retained classic 1.20c chart https://web.archive.org/web/20020202213610/http://www.t4cbible.com:80/spells.html, retrieved 2026-10-07.
- Printed Attack/Dodge 10 and Archery 15: https://www.t4cbible.com/skills, retrieved 2026-10-09; retained classic https://web.archive.org/web/20011221005225/http://www.t4cbible.com:80/skills.html, retrieved 2026-10-07. Currency/scaling and point conversion are missing: **Prototype** 10/10/15 gold per rank, 1 skill point per rank. **Prototype** rank cap 100 is a slice cap (authentic caps disputed). Replace these after source/play review.
- Bow/quiver buy prices: https://www.t4cbible.com/traders, retrieved 2026-10-09; Fali potion: https://www.t4cbible.com/Items, retrieved 2026-10-09. Rusted Dirk sell 9: https://www.t4cbible.com/Weapon, retained rules-ledger retrieval 2026-10-07. Version unstated.
- NPC spell/skill areas follow the supplied world ledger and Bible spell/skill pages. Nevanis/Shovanis B1: https://www.t4cbible.com/ArakasQuest and https://www.t4cbible.com/Spells, retrieved 2026-10-09. Placement remains authored graybox evidence.
- **Prototype** interaction distance 250 cm inclusive plus LOS, from R-03 missing NPC range. Distance uses the same centimeter convention as portal interaction. **Prototype** requirement basis: base attributes, per R-03 missing basis. Replace after source/interaction fixtures.

## First-loop affordability

The Bible did not establish starting gold/pools. R-03 proposes **Prototype** level 1, XP 0, gold 100, skill pool 0, Attack/Dodge 10 and no spells. Bow plus unlimited quiver costs 129: earn 29 additional gold; bow alone is affordable but does not enable the ranged route without its quiver. Light requires level 2, WIS 15/INT 18, 5 points and 233 gold: earn 133 additional gold plus sufficient XP/attribute allocation. FireDart requires level 2, WIS 15/INT 21, 5 points and 532 gold: earn 432 additional gold. Level growth gives 15 skill points (retained Bible getting-started guide), so one earned level covers either spell's points; attribute feasibility depends on the real creation roll, not a promised guarantee.

For an illustrative normal Brown Rat loop, R-03 Prototype guaranteed midpoint gold is 3 (Bible range 1–5) and unconditional XP uses first printed column 45 (interpretation missing). With no other spending, 10 rats cover bow/quiver gold deficit, 45 cover Light's deficit, 144 cover FireDart's deficit; 23 rats reach classic level-2 cumulative 1000 XP. These are arithmetic under Prototype distributions, not observed economy/play results. W4-06 must test affordability with the actual W4-02 profile, inventory/carry limits, kills and allocation, and exercise zero-gold melee recovery.

## Validation boundary

Eight Lighthaven.Services acceptance tests compose the domain with Import validation and real request receipt helpers. Synthetic profiles in tests are not playable tuning. Full physical NPC/LOS, item catalog closure, unlimited quiver combat and UI/save/travel are W4-06 integration checks. No editor map generation, package, or play gate is asserted here.

## W4-07b readiness and carry policy

Ledger vocabulary distinguishes documentary mechanics from authored policy. Every service integer must be resolved and nonnegative; Missing, Disputed and null/unresolved values always refuse. Confirmed, VerifiedT4C (verified evidence) and explicitly labelled Prototype are accepted. Modernized is accepted only for authored inventory policy, not as proof of historical mechanics. No data was relabelled.

| Service input | Accepted provenance |
|---|---|
| Item StackLimit; profile InventorySlots | Confirmed, VerifiedT4C, Prototype, Modernized |
| Offer Gold/SkillPoints; ability LearningGold/LearningSkillPoints | Confirmed, VerifiedT4C, Prototype |
| Character Gold/UnspentSkillPoints | Confirmed, VerifiedT4C, Prototype |
| Train Points; Buy/Sell Quantity; held item Quantity | Confirmed, VerifiedT4C, Prototype |
| RankCap; learned TrainedValue | Confirmed, VerifiedT4C, Prototype |
| Base attributes; EarnedLevel used for learning eligibility | Confirmed, VerifiedT4C, Prototype |
| SellPrice | Confirmed, VerifiedT4C, Prototype |

Eligibility minima and policy go through the existing Rules requirement evaluator, and final snapshots go through Character Import. These evaluators retain their own readiness checks. Live health in Common must be resolved and finite; final Character validation rejects Missing/Disputed. Missing/disputed inputs are never converted to zero.

Carry weight is unlimited: **deferred by owner decision**, Matt, 2026-10-09 20:07 ET. Character Validate no longer totals weight or compares it to derived Capacity. It still validates resolved nonnegative item weight metadata, stack quantities, equipment and inventory entry limits. Unlimited weight does not remove slot limits or serialization safety bounds. The replacement point for a future reviewed weight policy is the comment following Stats(C) in Character Validate. No STR capacity or MP formula was introduced or changed.

The fixed-100 legacy derived-stat approximation is outside owned paths: Source/Lighthaven/Framework/LHWave2Profile.cpp, P.Rules.Stats final Linear(100). It is no longer an enforced weight limit. Framework/Rules owners should replace the displayed numeric Capacity with an explicit unlimited policy representation when that shared contract/UI work is scheduled; do not interpret 100 as an active carry allowance.

### W4-07b observed validation (2026-10-09 ET)

Linux UE 5.8.3 editor build succeeded. Full headless automation completed 157 tests: 146 Success, 8 Success with warnings, 3 Fail, 0 not run. ServicesThroughSession, UseItemThroughSession and Services.AuthoredPolicyReadiness passed. ProgressionRouteAffordable now passes both earned purchases but fails equipping the bow before the quiver (LHWave2Tests.cpp lines 683–686), followed by ranged activation/replay failures. Character.EquipmentQuiverAndSnapshot passes and explicitly requires backpack quiver to be insufficient. No integration test or equipment policy was changed; reconcile the ordering with the integration owner. ArrivalSafety and LightingAudit fail because the five maps are LFS pointer files in this isolated checkout.

The exact requested automation invocation was attempted, but sandbox network enforcement interrupted it at Unreal's analytics endpoint datarouter.ol.epicgames.com. The completed retry used the same invocation plus '-ini:EditorSettings:[/Script/UnrealEd.AnalyticsPrivacySettings]:bSendUsageData=False'. Exit 255 reflected automation failures. No map generation, cook, package or interactive play check was run.
