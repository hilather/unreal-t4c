# R-01b — Rules research notes

Research date: **2026-10-07**. Network research was available through the web tool. Direct shell HTTP was blocked by the sandbox domain allowlist; no raw HTML capture succeeded. Several pages could not be opened, as logged below. No failed page supplies evidence in this deliverable. The web tool sometimes supplied cached crawls; retrieval date is the access date, not a publication or measurement date.

Assumption: retain LH_Prototype_v1 as the development profile. No historical server/patch was selected, and no proposal is approved by this research. Owned changes are the rules ledger and these notes; schemas, enemy-roster.json and prototype-policy.json were read but not changed.

## Search log

| Priority | Search/open operation | Outcome and evidence consequence |
|---|---|---|
| 1 | Open [Classic Questions](https://t4cfantasy.com/Bible/Classic/Questions.php), [D4O First Steps](https://www.d4o.de/en/game-infos/first-steps/), [Dialsoft advancement](https://support.t4c.com/knowledgebase.php?article=12) | Opened. Affinity table is qualitative; maxima do not establish independent rolls or minimum stats. D4O recommendations are build advice, not starting grants. |
| 1 | Search `T4C initial hit points mana starting 20 10`; open [Abomination beginner guide](https://wiki.t4c.com/abo/doku.php?id=en:guides:guide_du_debutant) | Guide opened. No initial HP/MP or point pool recovered. Modern operator answer mapping disagrees with the Classic dream row. Unrelated game manuals excluded. |
| 1 | Open `https://www.t4cnostalgia.com/bible/char-rolling` | Internal error; guessed path supplies no evidence. |
| 2 | Open Classic Questions XP/growth/encumbrance sections | Opened. Threshold table now recorded in ledger. No XP generating formula adopted. Growth bands allow compact mathematical descriptions, but probability distribution and growth timing are unknown. |
| 2 | Search `site:archives.jeuxonline.info "T4C" "points de vie" "départ"`; open [2002 endurance thread](https://archives.jeuxonline.info/fils/64035.html) | Search surfaced a dated band table; direct open failed. Excluded as supporting evidence. Other character build posts do not establish starter rules. |
| 3 | Search `T4C hit chance armor damage formula mana regeneration Fire Dart` and `"T4C" "chance" "armor" formula` | Found Fantasy spell formulas and a community hints page. No opened source established physical-hit or AC mitigation formula. |
| 3 | Open `https://4genet.pbworks.com/w/page/968685/Hints%20and%20Tips` | Internal error. Search snippet's attack:dodge percentage and strength bonus claims deliberately not promoted. |
| 3 | Search `site:archives.jeuxonline.info "esquive" "formule" "attaque"` | Mixed-game and anecdotal results; no controlled T4C fixture or authoritative formula obtained. |
| 4 | Open [Classic Skills](https://t4cfantasy.com/Bible/Classic/Skills.php), [Classic Spells](https://t4cfantasy.com/Bible/Classic/Spells.php), [Fantasy Spells](https://www.t4cfantasy.com/Bible/Spells.php) | Opened. Cost labels, spell raw output, calculated output and timing distinguished. Modern non-Classic formulas not imported as classic constants. |
| 5 | Open [Dialsoft mana recovery](https://support.t4c.com/knowledgebase.php?article=23); search `"T4C" "regeneration" "seconds" mana` | Support opened but no natural tick interval. Regeneration-potion and item proc rates are not natural mana regeneration. |
| 5 | Open [item locations](https://t4cfantasy.com/Bible/Items.php), [traders](https://www.t4cbible.com/traders) | Opened: existing Fali50-gold mana-potion and Sigfried29/100-gold bow/quiver values freshly corroborated. No initial-gold evidence inferred from prices. |
| 6 | Open `https://t4cfantasy.com/Bible/Classic/Monsters.php` | Internal error. Navigation link leads to singular Monster.php. |
| 6 | Follow [Monster Weaknesses](https://t4cfantasy.com/Bible/Classic/MonsterWeaknesses.php) links to [Monster.php](https://t4cfantasy.com/Bible/Classic/Monster.php) and [MonsterDrops.php](https://t4cfantasy.com/Bible/Classic/MonsterDrops.php) | Opened. All eleven stat names found; Green Slime absent from drop search. Boss section supplies Balork loot/respawn, but blank gold. |
| Archive | Open `https://web.archive.org/web/20041001000000/http://www.t4cbible.com/` | Internal error. No successful Wayback snapshot; no archive version/date asserted. |

## Creation pool: operator mapping and disagreement

The following value rows inherit source_url [R23](https://wiki.t4c.com/abo/doku.php?id=en:guides:guide_du_debutant), source_baseline **current Abomination beginner wiki, patch and publication date unspecified**, retrieved_date **2026-10-07**, status **modernized**. They record abbreviated affinities, not copied question prose or numeric deltas. Column order is STR+END / AGI+STR / INT / WIS / neutral.

| Theme | value: answer cues in affinity order |
|---|---|
| Dream | beetle / hyena / raven / owl / jay |
| Ballad | Boeris / Nimblefoot / Fables / Peter / swan-crow |
| Prized possession | strength / freedom / eyes-hands / family / frog |
| King's drink | beer / poison / ambrosia / rejuvenation / water |
| Messenger | valkyrie / virgin / gargoyle / samite / giant |
| Worthiness | amazon / fleeing-man / king / humble-man / troubadour |
| Mate's trait | courage / ambition / liveliness / devotion / candor |
| Tarot | emperor / wheel / sorcerer / hierophant / fool |

Notes: R05's dream row instead labels owl INT and scarab STR, while R23 labels them WIS and STR+END; status **disputed**, both values retained in ledger. R23's translated ballad INT cue differs from Classic naming; avoid treating translated cues as stable source answer IDs. Exact question wording is available at sources but not reproduced here. Random selection weights, numeric deltas, minima and correlations remain **missing**, value null. Eight themes do not imply every combination is equally likely.

## Interpretation checks and compact source excerpts

- [Classic Skills](https://t4cfantasy.com/Bible/Classic/Skills.php) labels the column “Cost” and differentiates “learn” from “train”. Attack/Dodge/Archery trainer costs now appear in their existing ledger notes, but the page does not explicitly label currency, skill-point units or scaling. Confirming the printed number is different from confirming a transaction model.
- [Classic spell reference](https://t4cfantasy.com/Bible/Classic/Spells.php) labels one section “Calculated Base Output”. Those outputs differ from raw description ranges because they incorporate attributes. The ledger keeps both forms without choosing a rounding rule. Spell casting intervals are documented; movement cancellation, refund timing and range are not established.
- [Current Fantasy spell reference](https://www.t4cfantasy.com/Bible/Spells.php), baseline current Fantasy non-Classic guide, retrieved_date2026-10-07, status **modernized**, value: Fire Dart uses a d17 draw, an INT/23 term and a fire-power/resistance ratio; Stone Shard uses d9 and WIS/22. Notes: these explain a possible scaling structure, but do not establish Classic power defaults, rounding, AC interaction or resistance-zero behavior. Full runtime formula remains **missing**, value null for the classic candidate. R26 is a lead for later baseline-specific fixtures.
- [Monster index](https://t4cfantasy.com/Bible/Classic/Monster.php) headings include “XP+1”, “XP+50”, “XP+100”, “Dmg” and “Dmg+”. The path says Classic while its page heading says Fantasy Bible. No patch or heading interpretation was obtained. Documentary HP and column tuples are candidates, not a resolved unconditional XP reward or min/max damage contract.
- [Drop guide](https://t4cfantasy.com/Bible/Classic/MonsterDrops.php) uses qualitative rarity markers. Missing probabilities cannot be recovered by translating “rare” into a percentage. Empty cells and absent names remain null. Boss timer text does not establish whether it runs from death, despawn or server restart.
- [Dialsoft mana recovery](https://support.t4c.com/knowledgebase.php?article=23) confirms recovery options but does not document a numeric passive interval. Existing prototype mana timing and potion overflow policies remain prototypes.

## Remaining evidence needs and transition count

Only **one existing missing row became confirmed**: XP curve, restricted to the documented levels1–20 table. Existing missing→disputed/modernized/prototype counts are all **zero**. New partial rows: creation disagreement, growth coverage, carry formula, three spell outputs, eleven monster-stat tuples and ten loot candidates; missing Green Slime loot remains explicit. Eight modernized answer-theme rows live above. New proposals are not counted as source resolutions.

Required follow-up: precise server/patch; full creation generator and starter state; same-level allocation/buff growth timing and RNG; XP generating rule/cap; physical hit/AC/resistance/attack speed; trainer cost currency/units/increments/caps; exact spell mitigation/rounding/range/cancellation; natural mana ticks and potion edges; XP/damage column meaning, all drop probabilities, slime loot, Balork gold and ordinary respawn timing; death denominator, item selection and recovery. The ledger's separate prototype table offers explicit temporary choices for W1-02/W2-02/W4 owners to review, rather than filling authentic-rule nulls.
