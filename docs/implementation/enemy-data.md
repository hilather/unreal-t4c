# W4-02 native enemy, encounter and item catalogs

Native value catalogs replace binary DataAssets for this wave. `LHEnemyData::Catalog/Find` returns eleven separate definitions with documentary values, built `Runtime` and `Reward` specs. `LHEncounterData::Catalog/ForSpawn` derives exactly one row from each registry slot; it stores no copied anchor, alias or minted identity. Resolve transforms and aliases from `LHWorld::Registry()` at integration. `LHItemData::Catalog/Find/StartingKit` supplies fourteen items and explicit starter quantities. No Framework/profile/hash/generator changes are made here.

## Sources and selected interpretations

All source values are transcribed from [R-03](research/w4-bible-lookup.md), [rules ledger](rules-ledger.md), [world ledger](world-ledger.md), and [Bible parameter ledger](ruleset-bible-v1.md). No new web retrieval. Classic monster numeric cells use <https://web.archive.org/web/20020202144834/http://www.t4cbible.com:80/monster.html>, retrieved 2026-10-07 (retained capture; historical patch unstated). All three XP columns remain separately available. Classic loot membership uses <https://web.archive.org/web/20011014151611/http://www.t4cbible.com:80/drops.html>, retrieved 2026-10-07. Live gold ranges and Balork900s use <https://www.t4cbible.com/monster1>, retrieved 2026-10-09, Original Content/community charts, server/version unstated. Source confirmation is documentary, never verified in play.

Rat/Bat/Slime/Undead Bat damage fields retain Disputed status in the definition. Runtime attack values explicitly select the classic side with Confirmed classic provenance and a note retaining the live conflict, because W4-01 rejects Disputed executable inputs. This is an explicit owner-selected baseline, not a resolution of the historical dispute. Goblin/Goblin Warrior/Atrocity loot membership remains Disputed. Classic membership excludes live-only Goblin Heal Pot and Atrocity Iron Key, and preserves differing rarity labels in the lookup. Dash produces an empty listed table, not proof of historical no-loot.

Prototypes for R-03 missing rows: first XP column as unconditional reward; integer uniform classic min/max damage through existing Rules; guaranteed floor-midpoint gold (3,3,7,7,10,10,14,17,43,17; Balork missing gold5); 10% overall item event, one uniformly weighted classic item, quantity1; ordinary120s; loaded/unpaused respawn clock. Rarity words do not establish drop percentages. Empty tables remain empty even though the item-event parameter is0.1.

Runtime interpretation also requires values not recoverable from Bible stats: Attack10, Dodge10, AC0, resistance0, damage bonus0, MP0; free melee with zero requirements, cooldown2s, impact1s, range200cm (Balork350cm). Reuse the shared Stage1 prototype ratio/flat armor formula; no separate rules engine. R-03's attack-rating/speed and arena/script gaps plus the ledger's Enemy runtime interpretation are the provenance for these authored adapters. Replace with evidence and G4 balance review. AI has only W4-01 allowlisted keys: aggro800cm, leash1600cm, speed180cm/s, retries3, pursuers3 (world-design1–3 proposal), decision0.2s, home50cm, safety1000cm (R-03 and W4-04). These are explicit Prototype tuning, not historical values. Capsule geometry is A-01 authored design rather than an absent historical mechanic.

## Capsule adoption review

W4-02 adopts the A-01 proposals below into runtime and maps each exact definition suffix to `Presentation.Enemy.<Suffix>`. Shared families never merge definitions. This is a source/dimension review, not a visual or swept navigation check. Decorations remain nonblocking; floor-root attaches at minus HH. Targeting/LOS for small rats and floating bat visuals still needs G4 host coverage.

| Species | R / HH cm | Adoption |
|---|---|---|
| BrownRat |25 /25|adopted A-01|
| Bat |25 /75|adopted A-01|
| DungeonBat |30 /80|adopted A-01|
| GreenSlime |40 /40|adopted A-01|
| GiantBat |45 /100|adopted A-01|
| UndeadBat |30 /85|adopted A-01|
| GiantSpider |55 /55|adopted A-01|
| Goblin |35 /70|adopted A-01|
| GoblinWarrior |40 /75|adopted A-01|
| Atrocity |65 /100|adopted A-01|
| Balork |100 /155|adopted A-01; boss-route only|

Balork capsule200×310 fits the documented boss doors500×360 (300cm width and50cm height total spare) and550cm corridor (350cm spare). Its maximum visual440×310 envelope fits those widths/headroom;484cm turn circle fits600cm pads. These arithmetic margins do not prove corners, nav projection, door transitions or stair clearance. Balork cannot use ordinary240×300 doors with this capsule height, nor its full visual envelope. W4-06 must keep him in the boss route/arena and test C04/D05–D07, retreats and largest-agent clearance. No geometry changed and no proposal to move slots is made.

## Roster coverage matrix

C = selected documentary floor membership; P = unresolved/provisional membership; — = not assigned. C is inherited S3 except Undead Bat B2 confirmed by Bible <https://www.t4cbible.com/ArakasQuest> (2026-10-09); secondary B3 remains disputed and excluded. All *positions/counts* are Prototype regardless of membership. Definition status below concerns damage/loot conflicts and shared missing runtime fields. GUID aliases map exactly to registry GUIDs in the index below. All playable checks remain pending; no complete roster-parity or G4 claim.

| Definition | Floor | Floor status | Slots | Definition status | Placement aliases | Playable validation |
|---|---|---|---|---|---|---|
|Enemy.BrownRat|B1|C|12|disputed damage; Prototype runtime|B1.Spawn.BrownRat.01, B1.Spawn.BrownRat.02, B1.Spawn.BrownRat.03, B1.Spawn.BrownRat.04, B1.Spawn.BrownRat.05, B1.Spawn.BrownRat.06, B1.Spawn.BrownRat.07, B1.Spawn.BrownRat.08, B1.Spawn.BrownRat.09, B1.Spawn.BrownRat.10, B1.Spawn.BrownRat.11, B1.Spawn.BrownRat.12|pending G4 host walk|
|Enemy.BrownRat|B2|C|6|disputed damage; Prototype runtime|B2.Spawn.BrownRat.01, B2.Spawn.BrownRat.02, B2.Spawn.BrownRat.03, B2.Spawn.BrownRat.04, B2.Spawn.BrownRat.05, B2.Spawn.BrownRat.06|pending G4 host walk|
|Enemy.BrownRat|B3|C|4|disputed damage; Prototype runtime|B3.Rat.01, B3.Rat.02, B3.Rat.03, B3.Rat.04|pending G4 host walk|
|Enemy.BrownRat|B4|C|3|disputed damage; Prototype runtime|B4.Rat.01, B4.Rat.02, B4.Rat.03|pending G4 host walk|
|Enemy.Bat|B1|C|3|disputed damage; Prototype runtime|B1.Spawn.Bat.01, B1.Spawn.Bat.02, B1.Spawn.Bat.03|pending G4 host walk|
|Enemy.Bat|B2|C|3|disputed damage; Prototype runtime|B2.Spawn.Bat.01, B2.Spawn.Bat.02, B2.Spawn.Bat.03|pending G4 host walk|
|Enemy.Bat|B3|—|0|disputed damage; Prototype runtime|—|pending G4 host walk|
|Enemy.Bat|B4|—|0|disputed damage; Prototype runtime|—|pending G4 host walk|
|Enemy.DungeonBat|B1|—|0|confirmed cells; Prototype runtime|—|pending G4 host walk|
|Enemy.DungeonBat|B2|P|2|confirmed cells; Prototype runtime|B2.Spawn.DungeonBat.01, B2.Spawn.DungeonBat.02|pending G4 host walk|
|Enemy.DungeonBat|B3|—|0|confirmed cells; Prototype runtime|—|pending G4 host walk|
|Enemy.DungeonBat|B4|—|0|confirmed cells; Prototype runtime|—|pending G4 host walk|
|Enemy.GreenSlime|B1|C|2|disputed damage; Prototype runtime|B1.Spawn.GreenSlime.01, B1.Spawn.GreenSlime.02|pending G4 host walk|
|Enemy.GreenSlime|B2|C|3|disputed damage; Prototype runtime|B2.Spawn.GreenSlime.01, B2.Spawn.GreenSlime.02, B2.Spawn.GreenSlime.03|pending G4 host walk|
|Enemy.GreenSlime|B3|C|3|disputed damage; Prototype runtime|B3.Slime.01, B3.Slime.02, B3.Slime.03|pending G4 host walk|
|Enemy.GreenSlime|B4|C|2|disputed damage; Prototype runtime|B4.Slime.01, B4.Slime.02|pending G4 host walk|
|Enemy.GiantBat|B1|—|0|confirmed cells; Prototype runtime|—|pending G4 host walk|
|Enemy.GiantBat|B2|C|4|confirmed cells; Prototype runtime|B2.Spawn.GiantBat.01, B2.Spawn.GiantBat.02, B2.Spawn.GiantBat.03, B2.Spawn.GiantBat.04|pending G4 host walk|
|Enemy.GiantBat|B3|C|4|confirmed cells; Prototype runtime|B3.GiantBat.01, B3.GiantBat.02, B3.GiantBat.03, B3.GiantBat.04|pending G4 host walk|
|Enemy.GiantBat|B4|C|4|confirmed cells; Prototype runtime|B4.GiantBat.01, B4.GiantBat.02, B4.GiantBat.03, B4.GiantBat.04|pending G4 host walk|
|Enemy.UndeadBat|B1|—|0|disputed damage; Prototype runtime|—|pending G4 host walk|
|Enemy.UndeadBat|B2|C|4|disputed damage; Prototype runtime|B2.Spawn.UndeadBat.01, B2.Spawn.UndeadBat.02, B2.Spawn.UndeadBat.03, B2.Spawn.UndeadBat.04|pending G4 host walk|
|Enemy.UndeadBat|B3|—|0|disputed damage; Prototype runtime|—|pending G4 host walk|
|Enemy.UndeadBat|B4|—|0|disputed damage; Prototype runtime|—|pending G4 host walk|
|Enemy.GiantSpider|B1|—|0|confirmed cells; Prototype runtime|—|pending G4 host walk|
|Enemy.GiantSpider|B2|C|3|confirmed cells; Prototype runtime|B2.Spawn.GiantSpider.01, B2.Spawn.GiantSpider.02, B2.Spawn.GiantSpider.03|pending G4 host walk|
|Enemy.GiantSpider|B3|—|0|confirmed cells; Prototype runtime|—|pending G4 host walk|
|Enemy.GiantSpider|B4|—|0|confirmed cells; Prototype runtime|—|pending G4 host walk|
|Enemy.Goblin|B1|—|0|disputed loot; Prototype runtime|—|pending G4 host walk|
|Enemy.Goblin|B2|—|0|disputed loot; Prototype runtime|—|pending G4 host walk|
|Enemy.Goblin|B3|C|6|disputed loot; Prototype runtime|B3.Goblin.01, B3.Goblin.02, B3.Goblin.03, B3.Goblin.04, B3.Goblin.05, B3.Goblin.06|pending G4 host walk|
|Enemy.Goblin|B4|—|0|disputed loot; Prototype runtime|—|pending G4 host walk|
|Enemy.GoblinWarrior|B1|—|0|disputed loot; Prototype runtime|—|pending G4 host walk|
|Enemy.GoblinWarrior|B2|—|0|disputed loot; Prototype runtime|—|pending G4 host walk|
|Enemy.GoblinWarrior|B3|C|3|disputed loot; Prototype runtime|B3.GoblinWarrior.01, B3.GoblinWarrior.02, B3.GoblinWarrior.03|pending G4 host walk|
|Enemy.GoblinWarrior|B4|—|0|disputed loot; Prototype runtime|—|pending G4 host walk|
|Enemy.Atrocity|B1|—|0|disputed loot; Prototype runtime|—|pending G4 host walk|
|Enemy.Atrocity|B2|—|0|disputed loot; Prototype runtime|—|pending G4 host walk|
|Enemy.Atrocity|B3|C|2|disputed loot; Prototype runtime|B3.Atrocity.01, B3.Atrocity.02|pending G4 host walk|
|Enemy.Atrocity|B4|C|3|disputed loot; Prototype runtime|B4.Atrocity.01, B4.Atrocity.02, B4.Atrocity.03|pending G4 host walk|
|Enemy.Balork|B1|—|0|confirmed cells; Prototype runtime|—|pending G4 host walk|
|Enemy.Balork|B2|—|0|confirmed cells; Prototype runtime|—|pending G4 host walk|
|Enemy.Balork|B3|—|0|confirmed cells; Prototype runtime|—|pending G4 host walk|
|Enemy.Balork|B4|C|1|confirmed cells; Prototype runtime|B4.Balork.01|pending G4 host walk|

## Balork separately

B4 membership confirmed; one existing `B4.Balork.01` anchor pending M4 arena review. Level15, HP508, classic melee13–29, XP columns2025/1553/1452; unconditional2025 XP is Prototype interpretation. Gold5 is missing-field Prototype. Flowing Black Robe/Light Heal are classic listed drops; each has5% effective chance under the authored10% single-item event, unrelated to historical demi-rare frequency. Respawn900s is the Bible15:00 with loaded/unpaused after-death clock Prototype. Every encounter including Balork uses OrdinaryRepeat; no PermanentDefeat. Per-life kill rewards are separate from the completion reward, which W4-04/05 must enforce single-claim. No phases/summons/script authenticity asserted.

## Items and integration

Dirk damage1–4, enc3, buy29/sell9 uses <https://www.t4cbible.com/Weapon>, retained2026-10-07. Bow1–3, enc7, buy29 and quiver+1, enc3, buy100 use <https://www.t4cbible.com/traders>, retained2026-10-07; compatibility is the explicit single bow/quiver pairing, Unlimited consumption. Potion+25MP/enc2 uses <https://www.t4cbible.com/Potions>,2026-10-09; buy50 <https://www.t4cbible.com/Items>,2026-10-09. Cloth requirements0/AC0/dodge0/enc3 each use <https://www.t4cbible.com/Armor>,2026-10-09. Unknown buy/sell/requirements/modifiers/encumbrance remain Unresolved; notably `Item.LightHeal` is an item with no invented UseItem effect, never a spell grant. Potion uses W4-03's real UseItem effect.

Explicit integration assumption: inventory instances are individually held (`StackLimit=1`, Modernized policy, not authentic stack counts). `StartingKit()` grants Dirk1, vest1, pants1, torches3 (R-03 missing kit, Prototype); allocate three separate Torch instances. Unknown loot uses a Modernized `UnresearchedLootExemptFromCapacity` carry policy: historical `Encumbrance` stays Unresolved but the `Character.Weight` adapter is an explicit capacity exclusion0. This permits carry/loot transfer without inventing source stats. Such rows have Unspecified slot, unresolved eligibility/modifiers and sell prices, so cannot be equipped or sold. This deliberate temporary exemption requires coordinator review; research weight before claiming economy fidelity. Known rows copy source encumbrance into Character.Weight. Known equipable items have explicit zero unregistered modifiers as a slice policy; cloth AC/dodge retain Bible provenance. No saved-shape change.

W4-06 appends `Character` definitions to the new profile, expands starter grants into individual instances, uses runtime/reward/encounter values and canonicalizes all data/provenance/policies into CatalogHash. W4-07 resolves offered item IDs through this catalog and must reject Unresolved sell values; bow/quiver/potion sales are intentionally unresolved. Native rows contain no UObject lifetime and pointers remain stable. Regenerating encounters never re-mints registry GUIDs/anchors. Any future movement proposal changes CatalogHash, requires map regeneration and, if arrivals/nearby geometry move, renewed arrival review.

## Open rows and validation boundaries

- Dungeon Bat B2 floor remains missing/provisional (two slots).
- Undead Bat B3 secondary disagreement remains open; B2 only is selected.
- Four classic/live damage conflicts and three loot/rarity conflicts remain disputed.
- Every enemy has missing attack cadence/accuracy, distribution, drop odds/quantity, and authored AI/combat adapters; ten ordinary respawn timers are missing.
- Balork gold, arena/script, timer origin and M4 arena anchor review remain open.
- Unknown item mechanics/prices/encumbrance remain unresolved; carry-capacity exemption and individual-instance policy need integration review.
- Largest-agent doors/stairs/nav, blocked-spawn outcome, in-play economy, legal melee/ranged/magic routes and all44 definition×floor playable rows remain pending G4 host walk. Deferred bat-wing quest is not implemented.

Acceptance tests are `Lighthaven.Data.RosterContractParity`, `RegistrySpawnsResolve`, `ProvenanceAndUnresolved`, `RuntimeSpecsValidate`, `ItemCatalogClosure`. RuntimeSpecsValidate calls the actual W4-01 validator and real W4-04 SettleKill transaction for each catalog reward using a synthetic character profile (not a legal-play claim). Build/test observations are in the attempt evidence report; no map/package/play result is inferred from data tests.

## Registry GUID alias index (reference transcription only)

Runtime consults registry; this documentation index is not an authoring copy of anchors.

|Alias|GUID|Enemy|
|---|---|---|
|B1.Spawn.BrownRat.01|bf36087d7eb24f6ea2e7c07103873227|Enemy.BrownRat|
|B1.Spawn.BrownRat.02|88644d7a3ae645af974dd7511596167a|Enemy.BrownRat|
|B1.Spawn.BrownRat.03|a617647a10b64634999dc71df2bbf36c|Enemy.BrownRat|
|B1.Spawn.BrownRat.04|547c0e1111e247048dd80a5e413dd6a2|Enemy.BrownRat|
|B1.Spawn.BrownRat.05|1b3a7b8385c846d796a985398cc44012|Enemy.BrownRat|
|B1.Spawn.BrownRat.06|7923806dc5d04f61972176663d2cabc4|Enemy.BrownRat|
|B1.Spawn.BrownRat.07|381587dd28a641e297175e1bfa57f081|Enemy.BrownRat|
|B1.Spawn.BrownRat.08|c31db6d3e1484dccb8e787a805838d47|Enemy.BrownRat|
|B1.Spawn.BrownRat.09|09a280c69426461ca75880a5753d08a5|Enemy.BrownRat|
|B1.Spawn.BrownRat.10|5fda812916144bf193c7e431431c843d|Enemy.BrownRat|
|B1.Spawn.BrownRat.11|e8ff0750a4ba4684bd95c61e49cb5d20|Enemy.BrownRat|
|B1.Spawn.BrownRat.12|64462184aec1481caa2275b644109639|Enemy.BrownRat|
|B1.Spawn.Bat.01|8c95497840044140bc1ddaac935bcc96|Enemy.Bat|
|B1.Spawn.Bat.02|2cefccd5562c42c0a2ef1750ad337d83|Enemy.Bat|
|B1.Spawn.Bat.03|6be9983f4c4b4a6b9eec217e9a157d99|Enemy.Bat|
|B1.Spawn.GreenSlime.01|c8ab0a145a494ba19167de5e0dc0ac08|Enemy.GreenSlime|
|B1.Spawn.GreenSlime.02|04da4b221a6a45cd970feec6dd56925c|Enemy.GreenSlime|
|B2.Spawn.BrownRat.01|8fccefae310d497ab6835868ddab59fe|Enemy.BrownRat|
|B2.Spawn.BrownRat.02|dc32dec46f4e47b3956ba0eaaeaaaed5|Enemy.BrownRat|
|B2.Spawn.BrownRat.03|780c95e4a9e04062981410323b06c569|Enemy.BrownRat|
|B2.Spawn.BrownRat.04|7427f49b5c6d460db1b2d34bbd806fa7|Enemy.BrownRat|
|B2.Spawn.BrownRat.05|7cafb7e2e46a42eb94057523f3071764|Enemy.BrownRat|
|B2.Spawn.BrownRat.06|b0fde1a58f1c4cc29a7bf5e47b34a06f|Enemy.BrownRat|
|B2.Spawn.Bat.01|62e451557a55420a807312bd2822f137|Enemy.Bat|
|B2.Spawn.Bat.02|c0fff622fdc74acebae524e74468db1f|Enemy.Bat|
|B2.Spawn.Bat.03|bd7c1f9c7e054bd9a60c4016a5828952|Enemy.Bat|
|B2.Spawn.GreenSlime.01|3bcdce577e6b4c4ca1c730e049d78902|Enemy.GreenSlime|
|B2.Spawn.GreenSlime.02|a5bf72cced8a45a9840bbb17511f9e10|Enemy.GreenSlime|
|B2.Spawn.GreenSlime.03|38f4d26127fa443888f6ee802f40d53d|Enemy.GreenSlime|
|B2.Spawn.GiantBat.01|e8bb9d7772d646748418036e61ad75b3|Enemy.GiantBat|
|B2.Spawn.GiantBat.02|47eb779f8caf4f0baa7b6ed065523e9e|Enemy.GiantBat|
|B2.Spawn.GiantBat.03|f62d45d285c9459480b521c922fbc677|Enemy.GiantBat|
|B2.Spawn.GiantBat.04|54e69b68096d41d68b5b453bef535737|Enemy.GiantBat|
|B2.Spawn.UndeadBat.01|b5b7640c1c784a7c8fa0180afbc6a501|Enemy.UndeadBat|
|B2.Spawn.UndeadBat.02|a7418fd006094ce09dd427ae0fa5ecbe|Enemy.UndeadBat|
|B2.Spawn.UndeadBat.03|38712490d2f847c6b12166a9fdbb420a|Enemy.UndeadBat|
|B2.Spawn.UndeadBat.04|f71969ee6e904ba8aeed9a301ec60432|Enemy.UndeadBat|
|B2.Spawn.GiantSpider.01|2190358c5b7943389836ab0ed25c0430|Enemy.GiantSpider|
|B2.Spawn.GiantSpider.02|09855e71862c4a76bfc3ddf705e9717a|Enemy.GiantSpider|
|B2.Spawn.GiantSpider.03|09ea0a1bf15b42dba4a8289d88530a12|Enemy.GiantSpider|
|B2.Spawn.DungeonBat.01|d85b1bdbbae144da9c4e1ee2d0f9b3fe|Enemy.DungeonBat|
|B2.Spawn.DungeonBat.02|c6e036fff619409380a1e82f685ea6e9|Enemy.DungeonBat|
|B3.Rat.01|54a31c7602124540b025b755c08a77aa|Enemy.BrownRat|
|B3.Rat.02|0dedc33dd30c4ade9ce75ba6634fdfa9|Enemy.BrownRat|
|B3.Rat.03|021f5d18a69d41d28d8afd268a1021dc|Enemy.BrownRat|
|B3.Rat.04|3d8814563e6740e9a019ed6918535bda|Enemy.BrownRat|
|B3.Slime.01|8be7c543236d43cb8bc303f7792e7476|Enemy.GreenSlime|
|B3.Slime.02|b14e1fe5a2d5481b85b63e576bd28136|Enemy.GreenSlime|
|B3.Slime.03|699b1293d51d4ed9ad01e4cc963f3db8|Enemy.GreenSlime|
|B3.GiantBat.01|24e196e104014f229642545fd0d87425|Enemy.GiantBat|
|B3.GiantBat.02|49747db56bd6499f8f05ffc72c5a16ed|Enemy.GiantBat|
|B3.GiantBat.03|17223428f081481d88184dd54bee4728|Enemy.GiantBat|
|B3.GiantBat.04|9489c162999a4169b3d6797408586245|Enemy.GiantBat|
|B3.Goblin.01|7f7b3de0395b4066974cc9147725ce9e|Enemy.Goblin|
|B3.Goblin.02|8fec7d37439a49a4894116b6329b0e64|Enemy.Goblin|
|B3.Goblin.03|59fb1b644dd24b1386bba14002c4b39e|Enemy.Goblin|
|B3.Goblin.04|b2b308c1474d4bd0b2fc3e6c74015972|Enemy.Goblin|
|B3.Goblin.05|87dce5536f3b434393177010b9e33a7d|Enemy.Goblin|
|B3.Goblin.06|43a64c60c9ff44db986cb1217c777d39|Enemy.Goblin|
|B3.GoblinWarrior.01|4af96671aaf341099816b0be719c5235|Enemy.GoblinWarrior|
|B3.GoblinWarrior.02|f8aaabcd3b71411a8e8332048d1f5042|Enemy.GoblinWarrior|
|B3.GoblinWarrior.03|c129733c4c6a49b693c7fb2933399050|Enemy.GoblinWarrior|
|B3.Atrocity.01|f9bea13ecb9545018333d02c12cf97c1|Enemy.Atrocity|
|B3.Atrocity.02|a17f538064804509a4c3e2ac6433c4c3|Enemy.Atrocity|
|B4.Rat.01|e87bcae47524436599aeedda21e227b6|Enemy.BrownRat|
|B4.Rat.02|816ebf68462f47d29c423f89560e44ba|Enemy.BrownRat|
|B4.Rat.03|3970e648b6c94e1dbcf0593d12666d9a|Enemy.BrownRat|
|B4.Slime.01|4298282572b5480786ca87fbb372f71d|Enemy.GreenSlime|
|B4.Slime.02|fe33cf57f68344da8a0aa58ea68e1e9b|Enemy.GreenSlime|
|B4.GiantBat.01|eb513875896a403395200d9ab85a281e|Enemy.GiantBat|
|B4.GiantBat.02|a3d68bab72d443d28a352603268352f2|Enemy.GiantBat|
|B4.GiantBat.03|470c8615d01d4c77a0f2759a092bb526|Enemy.GiantBat|
|B4.GiantBat.04|98ff591eb49645989ae2f664a19c7a3d|Enemy.GiantBat|
|B4.Atrocity.01|1b57ddae8d4d4f1e82679888899ceef2|Enemy.Atrocity|
|B4.Atrocity.02|b30be138283c4991b4a4ee35793d1604|Enemy.Atrocity|
|B4.Atrocity.03|56d2c71d54284bbdb45c42d28d0a37d4|Enemy.Atrocity|
|B4.Balork.01|77146f171dc04cd289f7357ba15ce45c|Enemy.Balork|

`CarryPolicy` is native catalog metadata consumed through `Character.Weight`, not an added generic Core/Eligibility policy token; the frozen D09 eligibility extension array remains empty. The carry exemption records a development assumption, not a source value or approved historical economy rule. W4-06 should hash this metadata along with the source fields.
