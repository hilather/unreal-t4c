# W0-01 / R-02 — Rules ledger

Reference ruleset: **T4C Bible: classic Vircom-era pages (2001–2006 snapshots, e.g. spells v1.20c, EXP v1.23), cross-checked with the live Bible**. This is documentary evidence, not verified-in-play parity. The pages span versions and servers; no single exact executable is recovered. R-03 records Matt’s 2026-10-09 decision to use the Bible classic baseline for Wave 4, adjustable later; missing runtime fields use explicitly labelled prototypes. This documentation does not claim a runtime migration was compiled or verified.

Unless explicitly marked R-03, retrieval dates below are **2026-10-07**, inherited from the verbatim [findings capture](research/t4cbible-findings.md); no fresh network fetch is claimed. Archive timestamps identify snapshot dates, not retrieval dates. The live Original Content pages were revamped in 2019 and state no game version. They add skills/spells, omit spell damage, alter several prerequisites, monster melee damage and loot lists. Classic values are preferred where conflicts occur, while live-only gold is documented for its own era.

The mirrored **Vircom 0.35-beta manual** (seven attributes, four answer options, different spell mana costs) and **Nostalgia 1.24b++** (modified 100 total/26 per stat rolling, starting HP/MP formulas and combat changes) are separate baselines. Neither supplies silent classic defaults. Classic uses five named attributes STR/END/AGI/WIS/INT. `confirmed` means a Bible page states the value; `disputed` retains conflicting claims; `missing` means null/unresolved; `prototype` is proposed tuning; `modernized` is authored presentation. Required nulls and disputed values must not be implicitly resolved by an adapter.

W1-02 currently accepts linear derived/growth and hit models. Band lookup, nonlinear encumbrance and revised policies need a later code task and integrator review. Source data below is not permission to shoehorn it into incompatible coefficients. Every zero in an adapter must be explicit; missing is never zero.

## Baseline mechanic reconciliation

The detailed values and per-parameter provenance live in [ruleset-bible-v1](ruleset-bible-v1.md). This table keeps W1-02 mechanic labels stable. Source baseline is Bible classic unless notes identify a different baseline.

| Parameter / mechanic | Value | Unit | status | source_url | retrieved_date | Notes |
|---|---|---|---|---|---|---|
| Creation question flow | 4 questions; pool of 8 themes, 5 answers each | count | confirmed | https://web.archive.org/web/20041011170503/http://www.t4cbible.com:80/getstart.html | 2026-10-07 | Four questions stated by 2004 guide; selection from eight is interpretation; distinctness/weights missing. |
| Creation RNG | null | distribution | missing | https://web.archive.org/web/20020602203835/http://www.t4cbible.com:80/start.html | 2026-10-07 | Same answers produce different stats; chart is not a probability law. |
| Creation maximum | 22 each | attribute points | confirmed | https://web.archive.org/web/20020602203835/http://www.t4cbible.com:80/start.html | 2026-10-07 | Not an independent uniform range. |
| Creation minima, total budget, initial pools | null | points | missing | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 | Chart totals vary 74–88; maximum chart total88 is not a conserved budget. |
| Starting level / XP / HP / MP / gold / kit / knowledge | null | mixed | missing | https://web.archive.org/web/20041011170503/http://www.t4cbible.com:80/getstart.html | 2026-10-07 | No exact classic starting grants; secondary estimate about70 HP is not exact. |
| Dream owl | WIS; Fantasy Classic chart INT | affinity | disputed | https://www.t4cbible.com/charroll | 2026-10-07 | Bible classic and live agree WIS; retain secondary disagreement. |
| Dream scarab | STR only in prose; Str category alongside STR+END in roll chart; Abomination STR+END | affinity | disputed | https://web.archive.org/web/20020602203835/http://www.t4cbible.com:80/start.html | 2026-10-07 | Do not infer numeric END bonus. |
| Nostalgia roll comparison | classic chart max total88/stat22; Nostalgia total100/stat26, 4 N/A→20 each versus classic16 | points | disputed | https://www.t4cnostalgia.com/main-ELL3M3UL.js | 2026-10-07 | Separate modified 1.24b++ baseline; no formula imported. |
| Canonical attributes | Strength, Endurance, Agility, Wisdom, Intelligence | names | confirmed | https://web.archive.org/web/20041011170503/http://www.t4cbible.com:80/getstart.html | 2026-10-07 | AGI/Dexterity maps to Agility, no sixth stat. |
| Accuracy / avoidance / STR damage / armor / resistance / speed coefficients | null | mixed | missing | https://www.t4cbible.com/skills | 2026-10-07 | Skill descriptions establish dependencies, not numeric laws. |
| Carry capacity | STR×500/(STR+100) | encumbrance | confirmed | https://web.archive.org/web/20021211172845/http://www.t4cbible.com:80/enc.html | 2026-10-07 | Nonlinear formula. Examples25→100,50→166,100→250,200→333,400→400,610→429; rounding and base/effective basis missing. |
| Initial and historical HP / MP | null | resource points | missing | https://web.archive.org/web/20021230103921/http://www.t4cbible.com:80/hp.html | 2026-10-07 | Store earned growth separately; maxima cannot be reconstructed from current attributes. |
| Retroactive growth | historical dependent growth preferred; Abomination retroactive HP/MP | policy | disputed | https://steamcommunity.com/app/523750/announcements/ | 2026-10-07 | Secondary modern relaunch disagrees; Bible bands do not establish retrospective recalculation. |
| Level entitlement | 5 attribute +15 skill points per earned level | points/level | confirmed | https://web.archive.org/web/20041011170503/http://www.t4cbible.com:80/getstart.html | 2026-10-07 | Shared spell/skill point pool; award once, debt recovery grants none. |
| Stat application timing / temporary buffs / RNG distribution | null | policy | missing | https://web.archive.org/web/20021230103921/http://www.t4cbible.com:80/hp.html | 2026-10-07 | Bands are per level; exact allocation ordering unresolved. |
| MP combination | null | formula | missing | https://web.archive.org/web/20021230103921/http://www.t4cbible.com:80/hp.html | 2026-10-07 | WIS and INT columns documented separately; addition is a proposed interpretation. |
| Initial level / source level cap | null | level | missing | https://web.archive.org/web/20021211182332/http://www.t4cbible.com:80/xp.html | 2026-10-07 | Table coverage does not establish maximum level. |
| XP curve | rounded xp.html table and PDF outputs below | cumulative XP | disputed | https://web.archive.org/web/20021211182332/http://www.t4cbible.com:80/xp.html | 2026-10-07 | Classic-era conflict; prefer rounded classic chart for reference lookup, not PDF silently. |
| XP PDF formula | display sum through x−1; C coef(level−1) sums through level−2, truncates coefficient×(level−1)^2.5 | XP | disputed | https://web.archive.org/web/20040612184134/http://www.t4cbible.com:80/exp.pdf | 2026-10-07 | P1=10; increments ti=0 at1–42,1 at43–82,4 at83–122,16 at123–142,64 at143–162,256 at163–182,1024 at183–192,4096 at193+. No formula executed here. |
| XP debt accounting | pay retained debt first; award new earned levels once | policy | modernized | docs/implementation/contracts-v1.md | 2026-10-07 | Integrator must freeze retained balance/debt normalization; loss denominator missing. |
| Requirement evaluation | null | Base/Effective policy | missing | https://www.t4cbible.com/skills | 2026-10-07 | Numeric minima do not establish stat basis. |
| Bow + equipped compatible quiver | required | boolean | confirmed | https://next.t4c.com/board/viewtopic.php?id=94 | 2026-10-07 | Secondary official v1.16 requirement retained; compatibility mapping missing. |
| Rusted Dirk | STR0 END0 AGI0 INT0 WIS0 Attack0; damage1–4; enc3; buy29/sell9 | mixed | confirmed | https://www.t4cbible.com/Weapon | 2026-10-07 | Equipable candidate, not a confirmed starter grant. |
| Ashwood Flatbow / Wooden Arrows | no attribute requirement; bow damage1–3 enc7 price29; quiver +1 damage enc3 price100 unlimited | mixed | confirmed | https://www.t4cbible.com/traders | 2026-10-07 | Inherited weapon evidence R20/R21; exact compatibility policy still needs code data. |
| Starting skills, spell grants, training cost currency/increments/scaling | null | mixed | missing | https://web.archive.org/web/20011221005225/http://www.t4cbible.com:80/skills.html | 2026-10-07 | Learn/train amounts below are documented labels; beta manual says gold+points but cannot establish classic rank transaction units. |
| Physical hit/damage | null | formula | missing | https://www.t4cbible.com/skills | 2026-10-07 | No numeric hit/dodge or STR→damage law; skill dependency alone insufficient. |
| Secondary combat claim | 2 attack:1 dodge ≈70–75%; STR/5 melee, STR/10+AGI/10 ranged | probability; damage | disputed | https://4genet.pbworks.com/w/page/968685/Hints%20and%20Tips | 2026-10-07 | Player claim, server unstated/inferred 4GE; Bible has no corroborating numeric formula; not confirmed. |
| Spell targeting / armor interaction | normal no-miss claim retained from Dialsoft; beta physical/mental distinction | policy | disputed | https://support.t4c.com/knowledgebase.php?article=24 | 2026-10-07 | Do not universally bypass armor merely because attack is a spell: beta manual explicitly distinguishes physical/mental. Classic numeric mitigation missing. |
| Spell range / cooldown / cancellation / refunds / power scaling | null | mixed | missing | https://web.archive.org/web/20020202213610/http://www.t4cbible.com:80/spells.html | 2026-10-07 | Measured output does not resolve runtime formula. |
| Weapon/effect ranges | Rusted Dirk1–4; classic measured spell output above | damage points | confirmed | https://www.t4cbible.com/Weapon | 2026-10-07 | Weapon final STR/AC composition missing. |
| Chance base/scales/min/max; armor/resistance scales; minimum damage/quantum | null | W1-02 coefficients | missing | https://www.t4cbible.com/skills | 2026-10-07 | Use separate prototype table only; no hidden authentic defaults. |
| Death losses | Realmud PvE gold10%, backpack5%, equipped0%, XP10%; denominator/item selection unresolved | percent | disputed | https://support.t4c.com/knowledgebase.php?article=29 | 2026-10-07 | Server-specific secondary baseline retained, no classic loss law. |
| No level loss / XP debt | earned level retained; regain deficit | policy | confirmed | https://support.t4c.com/knowledgebase.php?article=15 | 2026-10-07 | Secondary evidence retained; not Bible numeric loss evidence. |
| HP / mana natural regeneration | null | points/time | missing | https://www.t4cbible.com/Potions | 2026-10-07 | Potion/skill recovery does not give base rates. |
| Mana regeneration rate | 1 MP /5 seconds alive, unpaused; clamp; carry timer persisted; no offline accrual | MP; seconds | prototype | docs/plan/contracts/prototype-policy.json | 2026-10-07 | Existing development tuning retained. |
| Potion of Mana / Mana Elixir | +25 / +50 MP; mana potion enc2, price50 from Fali/Yolak | MP; enc; gold | confirmed | https://www.t4cbible.com/Items | 2026-10-07 | Potions page and classic various.html agree magnitudes. |
| Potion edge policy | full mana rejects without consumption; excess clamps | policy | prototype | docs/plan/contracts/prototype-policy.json | 2026-10-07 | Authored atomic consumption policy. |
| HP regen potion | 1/30 maxHP every2s for2min | HP; seconds | confirmed | https://web.archive.org/web/20011221005001/http://www.t4cbible.com:80/various.html | 2026-10-07 | Item effect only, never base regeneration. |

## Creation, growth, XP, skills, spells and monster values

Creation parameters (CRE-FLOW / CRE-RNG / CRE-RANGE / CRE-START)

| Parameter / mechanic | Value | Unit | status | source_url | retrieved_date | Notes |
|---|---|---|---|---|---|---|
| Creation question flow | 4 questions; pool of 8 themes, 5 answers each | count | confirmed | https://web.archive.org/web/20041011170503/http://www.t4cbible.com:80/getstart.html | 2026-10-07 | Four questions stated by 2004 guide; selection from eight is interpretation; distinctness/weights missing. |
| Creation RNG | null | distribution | missing | https://web.archive.org/web/20020602203835/http://www.t4cbible.com:80/start.html | 2026-10-07 | Same answers produce different stats; chart is not a probability law. |
| Creation maximum | 22 each | attribute points | confirmed | https://web.archive.org/web/20020602203835/http://www.t4cbible.com:80/start.html | 2026-10-07 | Not an independent uniform range. |
| Creation minima, total budget, initial pools | null | points | missing | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 | Chart totals vary 74–88; maximum chart total88 is not a conserved budget. |
| Starting level / XP / HP / MP / gold / kit / knowledge | null | mixed | missing | https://web.archive.org/web/20041011170503/http://www.t4cbible.com:80/getstart.html | 2026-10-07 | No exact classic starting grants; secondary estimate about70 HP is not exact. |
| Dream owl | WIS; Fantasy Classic chart INT | affinity | disputed | https://www.t4cbible.com/charroll | 2026-10-07 | Bible classic and live agree WIS; retain secondary disagreement. |
| Dream scarab | STR only in prose; Str category alongside STR+END in roll chart; Abomination STR+END | affinity | disputed | https://web.archive.org/web/20020602203835/http://www.t4cbible.com:80/start.html | 2026-10-07 | Do not infer numeric END bonus. |
| Nostalgia roll comparison | classic chart max total88/stat22; Nostalgia total100/stat26, 4 N/A→20 each versus classic16 | points | disputed | https://www.t4cnostalgia.com/main-ELL3M3UL.js | 2026-10-07 | Separate modified 1.24b++ baseline; no formula imported. |

| Parameter / mechanic | Value | Unit | status | source_url | retrieved_date | Notes |
|---|---|---|---|---|---|---|
| Worthiness | amazon→STR+END; troubadour→none; king→INT; fleeing man→AGI+STR; humble man→WIS | qualitative affinity | confirmed | https://web.archive.org/web/20020602203835/http://www.t4cbible.com:80/start.html | 2026-10-07 | Numeric deltas remain missing; cross-check https://www.t4cbible.com/charroll |
| Mate | ambition→AGI+STR; dedication→WIS; candor→none; courage→STR+END; wit→INT | qualitative affinity | confirmed | https://web.archive.org/web/20020602203835/http://www.t4cbible.com:80/start.html | 2026-10-07 | Numeric deltas remain missing; cross-check https://www.t4cbible.com/charroll |
| Messenger | samite→WIS; maiden→AGI+STR; valkyrie→STR+END; gargoyle→INT; giant→none | qualitative affinity | confirmed | https://web.archive.org/web/20020602203835/http://www.t4cbible.com:80/start.html | 2026-10-07 | Numeric deltas remain missing; cross-check https://www.t4cbible.com/charroll |
| Tarot | emperor→STR+END; mage→INT; wheel→AGI+STR; fool→none; hierophant→WIS | qualitative affinity | confirmed | https://web.archive.org/web/20020602203835/http://www.t4cbible.com:80/start.html | 2026-10-07 | Numeric deltas remain missing; cross-check https://www.t4cbible.com/charroll |
| Ballad | Nimblefoot→AGI+STR; Peter→WIS; crow/swan→none; Salvieri→INT; Boeris→STR+END | qualitative affinity | confirmed | https://web.archive.org/web/20020602203835/http://www.t4cbible.com:80/start.html | 2026-10-07 | Numeric deltas remain missing; cross-check https://www.t4cbible.com/charroll |
| Prized possession | eyes/hands→INT; character strength→STR+END; frog→none; freedom→AGI+STR; family→WIS | qualitative affinity | confirmed | https://web.archive.org/web/20020602203835/http://www.t4cbible.com:80/start.html | 2026-10-07 | Numeric deltas remain missing; cross-check https://www.t4cbible.com/charroll |
| Dream | hyena→AGI+STR; raven→INT; jay→none; owl→WIS; scarab→STR (chart ambiguity above) | qualitative affinity | disputed | https://web.archive.org/web/20020602203835/http://www.t4cbible.com:80/start.html | 2026-10-07 | Numeric deltas remain missing; cross-check https://www.t4cbible.com/charroll |
| King drink | water→none; ale→STR+END; poison→AGI+STR; rejuvenation→WIS; ambrosia→INT | qualitative affinity | confirmed | https://web.archive.org/web/20020602203835/http://www.t4cbible.com:80/start.html | 2026-10-07 | Numeric deltas remain missing; cross-check https://www.t4cbible.com/charroll |

### Documented rolling chart

Representative/advice outcomes, not guaranteed results or a recovered generator. Each row is confirmed as printed, except internally inconsistent totals, which are disputed and retained verbatim. Unit: attribute points; named column order prevents WIS/INT reversal.

| Answers | Total | Str | End | Agi | Wis | Int | status | source_url | retrieved_date |
|---|---|---|---|---|---|---|---|---|---|
| 2 Str, 2 Agi | 88 | 22 | 20 | 20 | 12 | 14 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 4 Agi | 88 | 22 | 16 | 22 | 12 | 16 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Agi, 2 Wis | 88 | 20 | 16 | 18 | 18 | 16 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Agi, 2 N/A | 86 | 20 | 16 | 20 | 14 | 16 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Agi, 2 Int | 86 | 18 | 14 | 20 | 14 | 20 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Str, 2 Wis | 86 | 20 | 20 | 14 | 18 | 14 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 3 Agi, Str | 85 | 22 | 17 | 21 | 11 | 14 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 3 Agi, Wis | 85 | 21 | 15 | 20 | 14 | 15 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 4 Str | 84 | 22 | 22 | 16 | 12 | 12 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Agi, Str, Wis | 84 | 21 | 17 | 18 | 14 | 14 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 3 Agi, Int | 84 | 20 | 14 | 21 | 12 | 17 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Str, 2 Int | 84 | 18 | 18 | 16 | 14 | 18 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Str, 2 N/A | 84 | 20 | 20 | 16 | 14 | 14 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 3 Agi, N/A | 84 | 21 | 15 | 21 | 12 | 15 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Agi, Str, N/A | 83 | 21 | 17 | 19 | 12 | 14 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Agi, Str, Int | 83 | 20 | 16 | 19 | 12 | 16 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 3 Str, Agi | 83 | 22 | 21 | 17 | 11 | 12 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Str, Agi, Wis | 83 | 21 | 19 | 16 | 14 | 13 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 4 Wis | 82 | 16 | 16 | 12 | 22 | 16 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Wis, 2 Int | 82 | 14 | 14 | 14 | 20 | 20 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Wis, 2 N/A | 82 | 16 | 16 | 20 | 14 | 16 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Wis, Str, Agi | 82 | 19 | 17 | 15 | 17 | 14 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 3 Str, Wis | 82 | 21 | 21 | 14 | 14 | 12 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Agi, Wis, Int | 82 | 18 | 14 | 18 | 15 | 17 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Str, Agi, Int | 82 | 20 | 18 | 17 | 12 | 15 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Agi, Wis, N/A | 82 | 19 | 15 | 18 | 15 | 15 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 3 Str, Int | 81 | 20 | 20 | 15 | 12 | 14 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Str, Agi, N/A | 81 | 21 | 18 | 17 | 12 | 13 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 3 Wis, Agi | 81 | 17 | 15 | 14 | 20 | 15 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Agi, Int, N/A | 81 | 18 | 14 | 19 | 13 | 17 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 3 Str, N/A | 81 | 21 | 21 | 15 | 12 | 12 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Str, Int, Wis | 80 | 18 | 18 | 14 | 15 | 15 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Str, Wis, N/A | 80 | 19 | 19 | 14 | 15 | 13 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 N/A, Str, Agi | 80 | 19 | 17 | 17 | 13 | 14 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 3 Wis, Str | 80 | 17 | 17 | 12 | 20 | 14 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 N/A, Agi, Wis | 80 | 17 | 15 | 16 | 16 | 15 | disputed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Wis, Agi, Int | 80 | 16 | 14 | 15 | 18 | 17 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Wis, Agi, N/A | 80 | 17 | 15 | 15 | 18 | 15 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Int, Str, Agi | 80 | 17 | 15 | 17 | 13 | 18 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Int, 2 N/A | 80 | 14 | 14 | 16 | 16 | 20 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 4 N/A | 80 | 16 | 16 | 16 | 16 | 16 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Wis, Str, Int | 79 | 16 | 16 | 13 | 18 | 16 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Wis, Str, N/A | 79 | 17 | 17 | 13 | 18 | 14 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Str, Int, N/A | 79 | 18 | 18 | 15 | 13 | 15 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Int, Agi, Wis | 79 | 15 | 13 | 16 | 16 | 19 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Int, Agi, N/A | 78 | 15 | 13 | 17 | 14 | 19 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 3 Wis, Int | 78 | 14 | 14 | 12 | 21 | 17 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 N/A, Str, Wis | 78 | 17 | 17 | 14 | 16 | 14 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 N/A, Agi, Int | 78 | 16 | 14 | 17 | 14 | 17 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 3 N/A, Agi | 78 | 17 | 15 | 17 | 14 | 15 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Int, Str, Wis | 78 | 15 | 15 | 14 | 16 | 18 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 3 Int, Agi | 78 | 14 | 12 | 17 | 14 | 21 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 4 Int | 78 | 12 | 12 | 16 | 16 | 22 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 3 Wis, N/A | 78 | 15 | 15 | 12 | 21 | 15 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 3 N/A, Str | 77 | 17 | 17 | 15 | 14 | 14 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 3 Int, Str | 77 | 14 | 14 | 15 | 14 | 20 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Wis, Int, N/A | 77 | 14 | 14 | 13 | 19 | 17 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 N/A, Str, Int | 77 | 16 | 16 | 15 | 15 | 16 | disputed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Int, Str, N/A | 77 | 15 | 15 | 15 | 14 | 18 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 3 Int, Wis | 76 | 12 | 12 | 14 | 17 | 21 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 Int, Wis, N/A | 76 | 13 | 13 | 14 | 17 | 19 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 3 N/A, Wis | 76 | 15 | 15 | 14 | 17 | 15 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 2 N/A, Wis, Int | 76 | 14 | 14 | 14 | 17 | 17 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 3 Int, N/A | 75 | 12 | 12 | 15 | 15 | 21 | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |
| 3 N/A, Int | 74 | 14 | 14 | 15 | 15 | 17 | disputed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 |


The [parameter document](ruleset-bible-v1.md) contains all45 growth-band cells, all200 XP rows (classic and PDF), 16 classic skills plus six separately labelled live additions, all66 classic spells with era conflicts, and the eleven-definition monster stats and loot. Source statements are confirmed as documentary evidence; conflicts remain disputed. Starting gold/kit/HP/mana and other unknowns stay missing even where prototypes are proposed.

## Authored presentation and prototype policies retained

| Mechanic | source_url | source_baseline | status | value | notes |
|---|---|---|---|---|---|
| Local creation flow | null | LH_Prototype_v1 design | modernized | Profile/name → cosmetic human appearance → questions → roll/re-roll → review → confirm → church | Design §4; commit once, cancel without consuming slot; persist answer IDs, accepted named stats and generation version. Appearance has no bonus. |
| AGI / Dexterity mapping | null | LH_Prototype_v1 native contract | modernized | AGI and interface Dexterity → Agility | contracts/README.md; never add a sixth stat. Native order uses named fields, not source array order. |
| Atomic level-up / debt recovery | R10; local architecture §4 | Dialsoft no-level-loss; LH consistency policy | modernized | Each newly earned level awarded once; recovering XP debt grants no repeat points | Preserve earned level, allocations, training and growth through death/load; multi-threshold awards include every new level once. |
| Target controls and movement | null | LH design §§5, architecture §5 | modernized | Mouse/controller targets, elevated 3D camera, smooth walk/run | Authored presentation; no stamina, parkour or invulnerable dodge in slice. |
| Movement/commitment tuning | null | LH_Prototype_v1 | prototype | Walk220 cm/s; run450 cm/s; stop moving during attack/cast commitment | Existing design/prototype-policy, not historical speed. Replace after W1-01/W4-03 play review; interruption/refund details unresolved. |
| Starter kit | null | LH_Prototype_v1 | prototype | Item.RustedDirk candidate; clothing/torch/gold quantities null | Existing design/prototype-policy. W2-02 validates equipability for every allowed roll and declares remaining kit before runtime use. No new numeric defaults here. |
| Initial death profile | null | LH_Prototype_v1 | prototype | No death penalty during initial development | prototype-policy.json; replace in W2-02/W4-05 after selected baseline's denominator/item-loss/respawn research; do not present as authentic. |
| Mana regeneration rate | null | LH_Prototype_v1 | prototype | 1 MP / 5 seconds alive, unpaused simulation; clamp to max; no offline accrual | Existing policy/design; persist fractional timer to avoid reload ticks. W4-03 replaces rate with verified timing or reviewed tuning. |
| Potion edge policy | null | LH_Prototype_v1 | prototype | At full mana reject without consumption; excess restore clamps | Existing policy; W4-03 verifies source semantics or retains declared tuning. Validate/consume once. |

## Separate prototype proposals

All proposals are prototype, not historical claims; see nearest Bible evidence and adapter limitations in ruleset-bible-v1.

| Parameter group | Proposed value | Reason / replacement | Unit | status | source_url | retrieved_date |
|---|---|---|---|---|---|---|
| Starting resources | Level1, XP0, max/current HP30 and MP10, unspent attributes0, skill points0, gold100 | HP buffer tolerates learning combat; gold buys two mana potions but requires earnings for ranged kit. Replace W1-02/W2-02 after starter fixtures/economy review. | mixed (units in value) | prototype | https://web.archive.org/web/20041011170503/http://www.t4cbible.com:80/getstart.html (nearest evidence; choice authored) | 2026-10-07 |
| Starting knowledge and kit | Attack10, Dodge10, Archery0; no learned spells; Rusted Dirk1, cloth vest1, cloth pants1, torches3 | Keeps free melee entry and trainer-based routes. R-03 live Armor confirms all cloth requirements0; starting quantities remain unsourced. Replace W2-02/W4-07. | mixed (units in value) | prototype | https://www.t4cbible.com/Weapon (nearest evidence; choice authored) | 2026-10-07 |
| Growth award | HP=7+floor(base END/20); MP=4+floor(base INT/30)+floor(base WIS/60); snapshot before allocation; no retroactive changes or temporary buffs | Deterministic midpoint choice supports reproducible saves; source ranges motivate tuning, not parity. Replace W1-02 after timing/RNG fixtures. | mixed (units in value) | prototype | https://web.archive.org/web/20021230103921/http://www.t4cbible.com:80/hp.html (nearest evidence; choice authored) | 2026-10-07 |
| Physical resolution | Hit chance clamp(Attack/(Attack+Dodge),0.10,0.95); both zero→0.50; Archery substitutes Attack for bows; damage=max(1,uniform integer weapon base range+floor(STR/5)−AC); bow bonus floor((STR+AGI)/10)+quiver bonus; interval1500ms | Explicit simple tuning, not recovered formulas; avoids divide by zero. Replace W1-02/W4-03 after combat/equipment fixtures and balance review. | mixed (units in value) | prototype | https://www.t4cbible.com/skills (nearest evidence; choice authored) | 2026-10-07 |
| Requirement basis / training | Base stats only for requirements; 1 skill point→1 skill rank; Attack/Dodge10 gold per rank, Archery15; cap rank100 for slice | Source Cost labels motivate gold choices but do not prove currency/unit/cap. Replace W1-02/W4-07 after trainer observations; learned spells keep sourced acquisition costs. | mixed (units in value) | prototype | https://web.archive.org/web/20011221005225/http://www.t4cbible.com:80/skills.html (nearest evidence; choice authored) | 2026-10-07 |
| Spell magnitude arithmetic | For prototype starter effects only, uniform integer within classic measured output range at minimum requirements; neutral power/resistance100; prototype cast interval1500ms | Authored simplification avoids treating measured output as raw damage or double-counting attribute bonuses; actual mitigation and scaling remain missing. Replace W1-02/W4-03 with power/resistance/AC fixtures. | mixed (units in value) | prototype | https://web.archive.org/web/20020202213610/http://www.t4cbible.com:80/spells.html (nearest evidence; choice authored) | 2026-10-07 |
| Enemy runtime interpretation | Use Bible HP and classic melee bounds/loot membership (preserve live disputes); Prototype first XP column as reward, uniform integer melee draw, Attack10, attack every2000ms, guaranteed midpoint live gold,10% overall item event choosing one classic listed item uniformly; missing Balork gold→Prototype5; dash→Prototype empty. Balork respawn900s is Bible-backed, not a permanent-defeat exception | All XP/damage/drop interpretations and fallback gold are authored proposals. Renewable gold supports trainer/potion access; Balork loot rarity not reproduced. W4-02/04 must replace/review each definition and test economy; never mark native roster resolved merely from this table. | mixed (units in value) | prototype | https://www.t4cbible.com/Monster; https://www.t4cbible.com/Monster1 (nearest evidence; choice authored) | 2026-10-07 |


## Secondary source register (compatibility aliases)

These aliases are retained for existing world/contract links; they do not override the Bible baseline. Retrieval date2026-10-07.



Every URL/baseline below is inherited from the package's R register; retrieved_date 2026-10-07. R01–R22 retain inherited provenance unless a fresh check is explicitly stated; R23–R26 were freshly opened on 2026-10-07.

| Key | source_url | source_baseline |
|---|---|---|
| R01 | https://www.t4cnostalgia.com/game/how-to-play | Nostalgia operator guidance |
| R02 | https://support.t4c.com/knowledgebase.php?article=12 | Dialsoft character advancement |
| R03 | https://steamcommunity.com/app/523750/announcements/ | Official Abomination relaunch, 2024-08-16; feed, not pinned post |
| R04 | https://www.d4o.de/en/game-infos/first-steps/ | D4O operator onboarding |
| R05 | https://t4cfantasy.com/Bible/Classic/Questions.php | Classic creation/growth tables; patch unspecified |
| R06 | https://support.t4c.com/knowledgebase.php?article=25 | Dialsoft spell learning |
| R07 | https://support.t4c.com/knowledgebase.php?article=24 | Dialsoft spell system |
| R08 | https://support.t4c.com/knowledgebase.php?article=14 | Dialsoft experience loss |
| R09 | https://support.t4c.com/knowledgebase.php?article=29 | Realmud server settings |
| R10 | https://support.t4c.com/knowledgebase.php?article=15 | Dialsoft no level loss |
| R11 | https://t4cfantasy.com/Bible/Classic/Skills.php | Classic skills; patch unspecified |
| R12 | https://www.t4cbible.com/Weapon | Community weapon reference; version unspecified |
| R13 | https://t4cfantasy.com/Bible/Classic/Spells.php | Classic spells; patch unspecified |
| R16 | https://support.t4c.com/knowledgebase.php?article=23 | Dialsoft mana recovery |
| R17 | https://t4cfantasy.com/Bible/Classic/Misc.php | Classic consumables |
| R18 | https://t4cfantasy.com/Bible/Items.php | Fantasy item locations |
| R19 | https://t4cfantasy.com/Bible/Classic/NPCs.php | Classic NPC locations |
| R20 | https://t4cfantasy.com/Bible/Classic/Weapons.php | Classic weapons/quivers |
| R21 | https://www.t4cbible.com/traders | Community weapon traders |
| R22 | https://next.t4c.com/board/viewtopic.php?id=94 | Official archived version 1.16 notes |
| R23 | https://wiki.t4c.com/abo/doku.php?id=en:guides:guide_du_debutant | Current Abomination beginner wiki; patch/date unspecified; modern evidence |
| R24 | https://t4cfantasy.com/Bible/Classic/Monster.php | Fantasy monster guide under Classic path; patch unspecified |
| R25 | https://t4cfantasy.com/Bible/Classic/MonsterDrops.php | Fantasy Classic drop guide; patch unspecified |
| R26 | https://www.t4cfantasy.com/Bible/Spells.php | Current Fantasy non-Classic spell reference; patch unspecified; modern evidence |


## Additional stable ledger aliases and unresolved roster parameters

| Parameter / mechanic | Value | Unit | status | source_url | retrieved_date | Notes |
|---|---|---|---|---|---|---|
| HP growth dependence | END bands1–319; HP6–23 in documented ranges | HP/level | confirmed | https://web.archive.org/web/20021230103921/http://www.t4cbible.com:80/hp.html | 2026-10-07 | No extrapolation beyond coverage. |
| MP growth dependence | INT bands1–599; WIS bands1–539 | MP/level | confirmed | https://web.archive.org/web/20021230103921/http://www.t4cbible.com:80/hp.html | 2026-10-07 | Combining components and random law unresolved. |
| Growth table samples | END1–19→6–8; END20–39→7–9; INT1–29→3–5; INT30–59→4–6; WIS1–59→0 | points/level | confirmed | https://web.archive.org/web/20021230103921/http://www.t4cbible.com:80/hp.html | 2026-10-07 | Full45 band entries in ruleset table. |
| Growth chart coverage | END through319; INT through599; WIS through539 | attribute points | disputed | https://web.archive.org/web/20021230103921/http://www.t4cbible.com:80/hp.html | 2026-10-07 | Prior Fantasy coverage END439/INT659; Bible captured coverage preferred; no inferred extended bands. |
| Reachable maximum examples | 4STR22/22/16/12/12; 4AGI22/16/22/12/16; 2WIS2INT14/14/14/20/20 | STR/END/AGI/WIS/INT | confirmed | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | 2026-10-07 | Chart examples, not guaranteed rolls. |
| Monster HP/XP/loot and economy | HP/melee/level and printed XP/listed loot known; reward semantics/drop odds null | mixed | missing | https://www.t4cbible.com/Monster | 2026-10-07 | Partial confirmed data in roster tables; aggregate remains unresolved. |
| Basement spawn locations other than Balork | null | floor/anchor | missing | https://www.t4cbible.com/Monster1 | 2026-10-07 | Secondary floor placements retained separately in world ledger. |
| Balork gold / loot probabilities | null | gold; probability | missing | https://www.t4cbible.com/Monster1 | 2026-10-07 | Blank gold is not zero. |
| Skill maximums | 100 except Attack/Dodge/Archery; those caps null | rank | confirmed | https://www.t4cbible.com/skills | 2026-10-07 | Live Bible claim; prototype slice cap100 remains separate. |
| Starter identity | Rusted Dirk1–4 requires0 stats; secondary Rusty Dagger identity not established | item | disputed | https://www.t4cbible.com/Weapon | 2026-10-07 | Starting grant missing; Abomination kit cannot define classic starter. |

## Open questions / research still required



Starting gold and itemised kit; exact starting HP/mana/level/XP/point pools/knowledge; creation numeric answer deltas, minima, RNG/weights; numeric hit/dodge and Strength→damage formulas; natural HP/mana regeneration; monster XP-column semantics; basement spawn locations other than Balork; drop probabilities and Balork gold; ordinary respawn timing; growth RNG/stat basis/allocation timing/WIS+INT combination; full spell mitigation, range/timing/cancellation/refunds; requirements basis and skill cost currency/increments/scaling; death denominator/item selection. Null values must reject required reference content, not turn into zero. No runtime/editor/build/play validation was performed by R-02.


## R-03 Wave 4 reconciliation — 2026-10-09

The field-by-field [Wave 4 lookup](research/w4-bible-lookup.md) supplies statuses, exact Bible URLs/dates, short source excerpts and Prototype recommendations for **all eight requested groups**. It supplements the inherited tables above. Classic archive pages could not be reopened in this attempt; classic facts retain R-02's2026-10-07 retrieval, while successful live checks are2026-10-09. `missing` denotes a bounded search gap, not proven absence across the site. No gameplay parity was tested.

Fresh live confirmations: cloth vest/pants require0 in every attribute (Armor), so their starter equipability is resolved, while granting them remains **missing**. Spell descriptions confirm Heal Light costs2 MP and heals self/one friendly target; **magnitude missing**. Light costs10 MP, illuminates caster vicinity for600s; **radius missing**. Acquisition minima/points/gold for the five Stage1 spells match the retained classic table. Sigfried bow29/quiver100 and Fali mana potion50/+25 MP are confirmed; training10/10/15 is confirmed as printed, **currency/scaling/points-per-rank missing**. Attack/Dodge/Archery are exempt from the live100 skill cap; slice100 is Prototype. Nevanis heals on request; **amount/cost/restrictions missing**, not Dark Fang's level<6 rule.

Starting HP30/MP10/gold100, Attack10/Dodge10 and kit quantities stay **missing as Bible grants**; retained values remain Prototype. Existing growth midpoint formulas are Prototype distributions/timing/MP addition, not replacements for the confirmed Bible bands. Use the classic cumulative XP table, retaining its disagreement with the PDF; never substitute next-level increments where internally inconsistent.

Enemy HP and melee bounds are documented; classic/live conflicts remain **disputed**. Use classic bounds/loot membership per selected era and live-only gold ranges where classic has none. First XP column as flat reward, uniform damage draws, midpoint guaranteed gold,2000ms attack interval, Attack10, item-event10%/one uniform listed item are **Prototype interpretations**. Numeric attack rating, XP semantics, distributions, drop odds and ordinary respawn remain **missing**. Balork's gold is blank, not0; fallback5 remains Prototype. Dash loot is a missing list, not evidence of guaranteed empty loot. Robe rarity is qualitative, never a recovered5% probability.

Balork respawn **confirmed900s** (live15:00); clock/origin Prototype. Completion reward single-claim is owner policy, distinct from recurring kills. Undead Bat B2 is now Bible-confirmed; B3 remains an inherited secondary disagreement, not a union. Dungeon Bat floor **missing**, PrototypeB2. Wings count/chance **missing**; deferred quest reward Skull Dagger+2500 XP confirmed. Exact arena/phases/summons remain missing.

New missing-field recommendations (all **Prototype**, research date2026-10-09): Nevanis full HP/free for alive injured players; Heal Light+10 HP clamped; Light600cm radius; death full HP/MP at reviewed church anchor with retained no-development-penalty policy; respawn suppression within1000cm and view/relevancy; ordinary corpse loot300s loaded/unpaused with unique/quest items retained; NPC250cm/loot200cm with LOS. These address recovery, safety and clutter; no balance validation is claimed. Future wings quest proposal1 wing/10% is record-only. Beta Gem of Destiny church destination is separate-era evidence. Abomination2019 ground auto-vape15min is server-specific, not classic corpse duration. Secondary Realmud death-loss percentages stay disputed, never classic defaults.
