# T4C rules recovery: t4cbible.com (live + Wayback) and related pages

Compiled 2026-10-07 (UTC). All pages were fetched with `curl`. Wayback pages use the raw `https://web.archive.org/web/<ts>id_/<url>` form. In this file the citation link is the normal `https://web.archive.org/web/<ts>/<url>` form of the same capture.

**How to read this file**
- **VERBATIM**: text copied from the page. Spelling and typos are kept. Table rows were flattened from the HTML into `a | b | c`.
- **INTERPRETATION**: my own reading. Treat it as unverified.
- Four groups of sources are kept apart and never merged:
  - **(A)** the live t4cbible.com, 2026
  - **(B)** Wayback captures of the old t4cbible.com, 2001–2007
  - **(C)** the Vircom player manual mirrored on t4cbible.com, which describes the **0.35 beta**
  - **(D)** the four other target URLs

Number formats are copied as printed. The 2002 XP page uses `.` as the thousands separator.

---

## 0. URL / fetch log

Retrieval times for live pages: 2026-10-07, about 23:18–23:33 UTC.

| # | URL tried | Method | Outcome |
|---|---|---|---|
| 1 | https://www.t4cnostalgia.com/bible/char-rolling | Wayback CDX (`t4cnostalgia.com/bible/char-rolling*`, `www.t4cnostalgia.com/bible/*`) | **No snapshot.** The domain-wide CDX listing only has 2026 asset/home captures. `web.archive.org/web/2010/<url>` returned 404 |
| 1b | same | live `curl` | **200.** The HTML is an empty Angular shell (text is only "The 4th Coming 1.24b \| T4C Nostalgia Classic Server \| PvP & PvE"). The content was recovered from the JS bundle https://www.t4cnostalgia.com/main-ELL3M3UL.js (live, 2,725,503 bytes) |
| 2 | https://archives.jeuxonline.info/fils/64035.html | Wayback CDX (exact, prefix, and `fils/*` filtered on 64035) | **No snapshot.** CDX only shows the unrelated `fils/164035.html`. `web/2010/<url>` returned 404 |
| 2b | same | live `curl` | **200**, full thread recovered (UTF-8) |
| 3 | https://4genet.pbworks.com/w/page/968685/Hints%20and%20Tips | Wayback CDX (`4genet.pbworks.com/*`, 2000-row listing grepped for hint/tip/968685) | **No snapshot** of this page (other 4genet pages exist from 2010). `web/2010/<url>` returned 404 |
| 3b | same | live `curl` | **200**, page recovered |
| 4 | https://t4cfantasy.com/Bible/Classic/Monsters.php | Wayback CDX, live | **No snapshot; live 404** |
| 4b | https://t4cfantasy.com/Bible/Classic/Monster.php | Wayback CDX | Snapshots exist: earliest 20240816040635 (`.../Bible/classic/Monster.php`), plus 2024-12 to 2025-08. Not fetched from Wayback because the live page loaded |
| 4c | same | live `curl` | **200**, monster table recovered |
| 5 | http://www.t4cbible.com/ | Wayback CDX `t4cbible.com/*` | Thousands of captures from 2001 to 2012. CDX was intermittently "Temporarily Offline" or 504; the listing was done per year 2001–2007 with `/forum` URLs filtered out |
| 5a | https://www.t4cbible.com/ (live) | live | 200. It is a vBulletin 5.7.5 forum. The bible content pages are linked from https://www.t4cbible.com/bible.html |
| 5b | live bible pages: /charroll /charclass /skills /Spells /Monster /Monster1 /traders /karma /Weapon /Armor /Items /Potions /sagainfo | live | all 200 |
| 5c | live forum topics /forum/main-forum/45-bible, /307-bible-quest-and-weapon-fix, /270-character-builder-v4-0 | live | 200 |
| 5d | live probes /bible/ /Bible/ /bible/index.php /Monster.php /robots.txt /sitemap.xml etc. | live | 404 (index.html returned 1 byte) |
| 5e | Wayback t4cbible pages fetched (all 200), listed individually below: hp.html, xp.html, roll.html, stats.html, hints.html, start.html, enc.html, monster.html, drops.html, skills.html, spells.html, exp.pdf, ver126.html, getstart.html, spellstud.html, various.html, index.php?page=hpandmp, index.php?page=charrolling, t4c_old/1/Manual/chap01–chap13.htm | Wayback `id_` | all OK. Timestamps are given at each fact. stats.html (visitor stats), ver126.html ("Version 1.26 Images") and hints.html (mostly client tech tips) have little or no rules data |
| 5e-1 | https://web.archive.org/web/20021230103921/http://www.t4cbible.com:80/hp.html | Wayback id_ | 200, HP/MP chart |
| 5e-2 | https://web.archive.org/web/20021211182332/http://www.t4cbible.com:80/xp.html | Wayback id_ | 200, XP chart |
| 5e-3 | https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html | Wayback id_ | 200, XanThor rolling chart |
| 5e-4 | https://web.archive.org/web/20040404083223/http://www.t4cbible.com:80/stats.html | Wayback id_ | 200, site visitor stats only (no rules data) |
| 5e-5 | https://web.archive.org/web/20021013040845/http://www.t4cbible.com:80/hints.html | Wayback id_ | 200, mostly client tips; one attack-training tip |
| 5e-6 | https://web.archive.org/web/20020602203835/http://www.t4cbible.com:80/start.html | Wayback id_ | 200, questions, class rolls |
| 5e-7 | https://web.archive.org/web/20021211172845/http://www.t4cbible.com:80/enc.html | Wayback id_ | 200, encumbrance |
| 5e-8 | https://web.archive.org/web/20020202144834/http://www.t4cbible.com:80/monster.html | Wayback id_ | 200, monster chart |
| 5e-9 | https://web.archive.org/web/20011014151611/http://www.t4cbible.com:80/drops.html | Wayback id_ | 200, monster drops |
| 5e-10 | https://web.archive.org/web/20011221005225/http://www.t4cbible.com:80/skills.html | Wayback id_ | 200, skills |
| 5e-11 | https://web.archive.org/web/20020202213610/http://www.t4cbible.com:80/spells.html | Wayback id_ | 200, spells with damage |
| 5e-12 | https://web.archive.org/web/20040612184134/http://www.t4cbible.com:80/exp.pdf | Wayback id_ | 200, XP formula PDF (3 pages) |
| 5e-13 | https://web.archive.org/web/20030601130857/http://www.t4cbible.com:80/ver126.html | Wayback id_ | 200, only "Version 1.26 Images" (no data) |
| 5e-14 | https://web.archive.org/web/20041011170503/http://www.t4cbible.com:80/getstart.html | Wayback id_ | 200, getting-started guide |
| 5e-15 | https://web.archive.org/web/20020123134044/http://www.t4cbible.com:80/spellstud.html | Wayback id_ | 200, Hawk spell study |
| 5e-16 | https://web.archive.org/web/20011221005001/http://www.t4cbible.com:80/various.html | Wayback id_ | 200, potions, HP/MP, encumbrance, XP |
| 5e-17 | https://web.archive.org/web/20060807200304/http://t4cbible.com:80/index.php?page=hpandmp | Wayback id_ | 200, HP/MP chart (same values) |
| 5e-18 | https://web.archive.org/web/20060807195835/http://t4cbible.com:80/index.php?page=charrolling | Wayback id_ | 200, XanThor chart again |
| 5e-19 | https://web.archive.org/web/20041210204417/http://www.t4cbible.com:80/t4c_old/1/Manual/chap01.htm … chap13.htm (timestamps: 01 20041210204417, 02 20041210205629, 03 20041210210334, 04 20041210210542, 05 20041210211332, 06 20041210212311, 07 20041210202444, 08 20041210203459, 09 20041210204005, 10 20041210204755, 11 20041210205232, 12 20041210210308, 13 20041210210451) | Wayback id_ | 200 ×13, Vircom manual (0.35 beta) |
| 5f | https://www.t4cbible.com/Forum, /articles, /forum/main-forum, /forum/main-forum/help, /forum/news, /forum/4th-saga | live | 200. Forum listings; Articles says "There are no articles in this category." |
| 5g | https://www.t4cbible.com/bible.html | live | 200, bible index (link list) |
| 1c | https://www.t4cnostalgia.com/main-ELL3M3UL.js | live | 200, Angular bundle containing the rolling page |

Pages listed in the CDX but **not** fetched (budget): `/monster2.html`, `/gmonsters.html`, `/index.php?page=playerxp|elementalweaks|getstarted|spellstud`, `/old/oldbible/Skills.html|Spells.html|Main.html`, `/old/Skills.html`, `/various.html` sections other than potions/HP, `/abbreviations.html`, `/intro.html`, `/itemeffect.html`, `/weapons.html`, `/armor.html`, quest pages. They are worth a follow-up if needed.

---

## 1. What ruleset, version or server the sources say they describe

**(A) Live t4cbible.com (retrieved 2026-10-07)**
- VERBATIM, https://www.t4cbible.com/bible.html: section headings "Original Content" (Items, Weapon, … Spells …), "miscellaneous Info" (Karma, Monster Chart, Monster Drop, Npc Chart, Chest, Traders, Skills, Roll your Character, Character Classes), then "Server Specific", "Realmud" (RM Quest, RM Weapons, RM Armor, RM Jewelry, RM Spells) and "Saga".
- VERBATIM: each content page has the tab header "The 4th Coming | Realmud".
- VERBATIM, https://www.t4cbible.com/forum/main-forum/45-bible (lucifer, Administrator, 07-05-2019): "Hello everyone, We have finally revamped the bible." A follow-up on 07-12-2019 says "Pages are complete I believe."
- VERBATIM, footer: "© 2006-2026 Dialsoft, Inc." and "Powered by vBulletin® Version 5.7.5".
- VERBATIM, Monster chart legend: "Black: Arakas Monsters / Teal : Raven Dust Monsters / Light Blue : Stoneheim Monsters / Orange : Seraph Monsters / Violet : Add-on Monsters / Red : Beta Monsters (Summon from GM only) / Italic : Mini Bosses".
- INTERPRETATION: the live pages fetched are the "Original Content" (non-Realmud) bible, revamped in 2019. No game version number is stated on them. The monster, skill and spell data are near-identical to the 2001–2002 captures (see diffs below).

**(B) Old t4cbible.com 2001–2006 (Wayback)**
- VERBATIM, spells.html 2002-02-02: "(New added spells (version 1.20c) are in light blue color)".
- VERBATIM, monster.html 2002-02-02, colosseum table: columns "EXP before 1.23 | EXP version 1.23".
- VERBATIM, xp.html and various.html: "*Most of this Info is courtesy of GM-DumMWiaM of Greek Digicon Server*".
- VERBATIM, spellstud.html: "by Hawk, player of the Digicon Server".
- VERBATIM, hints.html 2002-10-13: "But I never had this happen in ver. 1.23 either... Started with 1.24."
- VERBATIM, start.html 2002-06-02, Mage text: "(Azure is a great armor if its legal at your server)".
- INTERPRETATION: this is the Vircom-era game, versions about 1.20c to 1.24/1.26, as run on official and private servers (notably the Greek Digicon server).

**(C) Vircom manual mirrored at t4cbible.com/t4c_old/1/Manual/ (captured 2004-12-10)**
- VERBATIM, chap05: "NOTE: In the 0.35 beta version of the game, the gender is asked and stored, but since the only appearance available at this time is for male characters, female characters will appear as men."
- VERBATIM, chap05: "There are seven main attributes (also called stats) in T4C." It lists STR, END, AGI, WIS, WIL, INT and LUCK.
- **Warning:** this is a **beta-era** document. getstart.html (2004) quotes the same text but says "There are five main attributes" and drops WIL and LUCK.

**(D) Other sources**
- t4cnostalgia: VERBATIM page title "The 4th Coming 1.24b | T4C Nostalgia Classic Server | PvP & PvE"; bundle text "⭐ Classic 1.24b++ Experience".
- 4genet.pbworks: VERBATIM page title "T4C Wiki / Hints and Tips"; "( Submitted by Nerv02 NM)"; "last edited by pbworks 10 years, 6 months ago". INTERPRETATION: probably the wiki of the 4GE.net private server. This is inferred from the subdomain and the old t4cbible forum threads titled "4ge-net …"; the page does not name a server.
- jeuxonline: French forum "La 4ème Prophétie > T4C - Forum Général", thread of 13–14 Feb 2002.
- t4cfantasy: VERBATIM header "T4C Fantasy Bible", "Normal Server". Its monster values match live t4cbible exactly for every creature checked.

---

## 2. Character creation

### 2.1 Questions, answers and stat effects

**Source A (live)**: https://www.t4cbible.com/charroll, retrieved 2026-10-07. Era: unstated (page header "The 4th Coming | Realmud").

**Source B**: https://web.archive.org/web/20020602203835/http://www.t4cbible.com:80/start.html, snapshot 2002-06-02.

The answer text and effects are identical in A and B (checked by a whitespace-normalised `diff` of the two extracted blocks). VERBATIM (B):

> "Everyone believes in being worthy, but not everyone agrees on what it means in real life. Who do you feel is the most worthy ?"
> - "The fearless amazon who, cornered by a hundred warriors, nonetheless stands her ground to defend a hopeless cause. (+Str, End)"
> - "The rebellious troubadour who wanders the kingdom, pleading openly for justice and equality for all. (Nothing)"
> - "The shrewd king who plays his enemies against each other and keeps his people happy. (+Int)"
> - "The man who flees the battlefield so he can live to fight another day. (+Agi, Str)"
> - "The humble man who apologizes for his mistakes and past misdeeds. (+Wis)"
>
> "What personality trait would you look for in a life mate ?"
> - "Ambition, for without it, your life has no aim. (+Agi, Str)"
> - "Dedication, for it is the true measure of a person. (+Wis)"
> - "Candor, for innocence is priceless in a harsh world. (Nothing)"
> - "Courage, for fear lessens your worth. (+Str, End)"
> - "Wit, for its absence makes you ordinary. (+Int)"
>
> "On the night before you rite of passage, a heavenly messenger appears to you on the stroke of midnight. What does it look like ?"
> - "A figure wrapped in white samite, holding a lit taper in one hand and a silver orb in the other (+Wis)"
> - "A beautiful, alluring maiden, with hair like golden honey, skin white as milk and eyes blue as sapphires. (+Agi, Str)"
> - "A radiant valkyrie, standing proudly in glittering silver mail armor, wielding a two-handed sword and bearing a winged helmet. (+Str, End)"
> - "A winged gargoyle, its head crowned with flames and its breath smelling of sulfur and decay. (+Int)"
> - "A white-furred giant, its broad shoulders slumped, its powerful arms hanging low, its eyes gentle and inquisitive. (Nothing)"
>
> "You draw a card at random from a tarot deck. It is..."
> - "The Emperor, expressing leadership, power, decisions and strength. (+Str, End)"
> - "The Mage, expressing willpower, communication, organization and invention. (+Int)"
> - "The Wheel of Fortune, expressing destiny, abundance, originality and chance. (Agi, Str)"
> - "The Fool, expressing possibilities, choice, creative expression and trust. (Nothing)"
> - "The Hierophant, expressing morality, wisdom, teaching and spirituality. (+Wis)"
>
> "A renowned minstrel offers to sing a ballad of your choice. You ask for..."
> - "The incredible adventures of Morgan Nimblefoot in the labyrinthine Palace of a Thousand Mirrors. (+Agi, Str)"
> - "The travels and miracles of St. Peter, and the tales of his peaceful sermons and admonishments. (+Wis)"
> - "The parable of the Crow and the Swan, where the crow wages with the swan over which one is most loved by humans. (Nothing)"
> - "The arcane tales of Archmage Salvieri and his quest for eternal life and mystical illumination. (+Int)"
> - "The battles of Sir Knight Boeris against the demon king Vindigan and his fiendish horde of hellwolves. (+Str, End)"
>
> "What is it you prize the most ?"
> - "Your eyes and hands. (+Int)"
> - "Your strength of character. (+Str, End)"
> - "Jeremiah, your pet bullfrog. (Nothing)"
> - "Your freedom. (+Agi, Str)"
> - "Your friends and family (+Wis)"
>
> "A premonitory dream troubled your slumber last night, leaving you with doubts and questions. The dream was about..."
> - "A hyena feeding off your dead carcass. (+Agi, Str)"
> - "A raven perched itself on your shoulder and pecked at your temple. (+Int)"
> - "A fluttering jay bird kept asking you if you were married. (Nothing)"
> - "A white owl spoke to you of your grand destiny. (+Wis)"
> - "A scarab mysteriously imbedded itself in your chest. (+Str)"
>
> "The King is old and wise. On his sixtieth birthday, you are asked to concoct a special beverage for him. What do you choose to do ?"
> - "A glass of water, for there are no better drinks. (Nothing)"
> - "A hearty ale, to make him merry and strong spirited, as in the days of his youth as a conquering king. (+Str, End)"
> - "A poison that will make him ill and weak, so that a better and younger king may replace him. (+Agi, Str)"
> - "A potion of rejuvenation that will make him young again, so that the kingdom may benefit from his wisdom for many more years. (+Wis)"
> - "A liquor of ambrosia, for the king is a connoisseur who appreciates the finer things in life. (+Int)"

**Discrepancy, recorded and not resolved:** the Scarab answer is "(+Str)" alone in A and B, but the XanThor chart (2.3) files "Scarab" under its "Str" column alongside the "+Str, End" answers.

### 2.2 How many questions, the stat cap, and randomness

- VERBATIM, start.html 2002-06-02 (link above): "if you give the exact same answers to the questions, you will never get the same starting stats (and the results differ to a great degree)".
- VERBATIM, same page: "(Tip: max number you can get on a stat is 22)".
- VERBATIM, same page: "It seems that the system Vircom uses is a completely weird one that we have yet to figure."
- VERBATIM, manual chap05 (https://web.archive.org/web/20041210211332/http://www.t4cbible.com:80/t4c_old/1/Manual/chap05.htm, snapshot 2004-12-10, **0.35 beta era**): "T4C will then ask you four questions which will be used to determine what kind of character you are going to play. Based on your answers, the stats for your various attributes will have specific ranges". Also: "Once you've answered the four questions, you will see your stats appear. If they are not to your satisfaction, you can press Reroll."
- VERBATIM, manual chap03 (https://web.archive.org/web/20041210210334/http://www.t4cbible.com:80/t4c_old/1/Manual/chap03.htm): "Next, you will be asked four questions. Answer with 1 , 2 , 3 or 4 for each question". Note: this says 4 answer options, while the bible lists 5 answers per question.
- VERBATIM, manual chap05: "You can create up to three characters in The 4th Coming."
- VERBATIM, getstart.html (https://web.archive.org/web/20041011170503/http://www.t4cbible.com:80/getstart.html, snapshot 2004-10-11): the same four-questions text, plus "There are five main attributes (also called stats) in T4C." (STR, END, AGI, WIS, INT).
- INTERPRETATION: the game draws 4 questions from a pool of 8. The bible lists 8 questions with 5 answers each.

### 2.3 XanThor rolling chart (classic)

https://web.archive.org/web/20040411201032/http://www.t4cbible.com:80/roll.html, snapshot 2004-04-11. The same chart is reproduced at https://web.archive.org/web/20060807195835/http://t4cbible.com:80/index.php?page=charrolling (2006-08-07).

VERBATIM: "Courtesy of Xan Boy's Web Page" and "Use tables to determine which answers to use for the roll you want. (Do not answer the 'Dream' question---just start over again)".

Answer-category table (VERBATIM):

| Str | Agi | Int | Wis | N/A |
|---|---|---|---|---|
| Strength | Freedom | Eyes and Hands | Friends | Pet bullfrog |
| Amazon | Man who flees | Shrewd King | Humble man | Rebellious |
| Courage | Ambition | Wit | Dedication | Candor |
| Valkyrie | Beautiful Maiden | Gargoyle | Figure in White Samite | Giant |
| The Emperor | The Wheel | The Mage | The Heirophant | The Fool |
| Ale | Poison | Ambrosia | Rejuvination | Water |
| Scarab | Hyena | Raven | White Owl | Jay Bird |
| Sir Knight | Nimblefoot | Archmage | St. Peter | Crow and Swan |

"Character Rolling Chart" (VERBATIM, all 65 rows as on the page). The columns are the four answer categories chosen, then the total and the five stats:

| Answers | Total | Str | End | Agi | Wis | Int |
|---|---|---|---|---|---|---|
| 2 Str, 2 Agi | 88 | 22 | 20 | 20 | 12 | 14 |
| 4 Agi | 88 | 22 | 16 | 22 | 12 | 16 |
| 2 Agi, 2 Wis | 88 | 20 | 16 | 18 | 18 | 16 |
| 2 Agi, 2 N/A | 86 | 20 | 16 | 20 | 14 | 16 |
| 2 Agi, 2 Int | 86 | 18 | 14 | 20 | 14 | 20 |
| 2 Str, 2 Wis | 86 | 20 | 20 | 14 | 18 | 14 |
| 3 Agi, Str | 85 | 22 | 17 | 21 | 11 | 14 |
| 3 Agi, Wis | 85 | 21 | 15 | 20 | 14 | 15 |
| 4 Str | 84 | 22 | 22 | 16 | 12 | 12 |
| 2 Agi, Str, Wis | 84 | 21 | 17 | 18 | 14 | 14 |
| 3 Agi, Int | 84 | 20 | 14 | 21 | 12 | 17 |
| 2 Str, 2 Int | 84 | 18 | 18 | 16 | 14 | 18 |
| 2 Str, 2 N/A | 84 | 20 | 20 | 16 | 14 | 14 |
| 3 Agi, N/A | 84 | 21 | 15 | 21 | 12 | 15 |
| 2 Agi, Str, N/A | 83 | 21 | 17 | 19 | 12 | 14 |
| 2 Agi, Str, Int | 83 | 20 | 16 | 19 | 12 | 16 |
| 3 Str, Agi | 83 | 22 | 21 | 17 | 11 | 12 |
| 2 Str, Agi, Wis | 83 | 21 | 19 | 16 | 14 | 13 |
| 4 Wis | 82 | 16 | 16 | 12 | 22 | 16 |
| 2 Wis, 2 Int | 82 | 14 | 14 | 14 | 20 | 20 |
| 2 Wis, 2 N/A | 82 | 16 | 16 | 20 | 14 | 16 |
| 2 Wis, Str, Agi | 82 | 19 | 17 | 15 | 17 | 14 |
| 3 Str, Wis | 82 | 21 | 21 | 14 | 14 | 12 |
| 2 Agi, Wis, Int | 82 | 18 | 14 | 18 | 15 | 17 |
| 2 Str, Agi, Int | 82 | 20 | 18 | 17 | 12 | 15 |
| 2 Agi, Wis, N/A | 82 | 19 | 15 | 18 | 15 | 15 |
| 3 Str, Int | 81 | 20 | 20 | 15 | 12 | 14 |
| 2 Str, Agi, N/A | 81 | 21 | 18 | 17 | 12 | 13 |
| 3 Wis, Agi | 81 | 17 | 15 | 14 | 20 | 15 |
| 2 Agi, Int, N/A | 81 | 18 | 14 | 19 | 13 | 17 |
| 3 Str, N/A | 81 | 21 | 21 | 15 | 12 | 12 |
| 2 Str, Int, Wis | 80 | 18 | 18 | 14 | 15 | 15 |
| 2 Str, Wis, N/A | 80 | 19 | 19 | 14 | 15 | 13 |
| 2 N/A, Str, Agi | 80 | 19 | 17 | 17 | 13 | 14 |
| 3 Wis, Str | 80 | 17 | 17 | 12 | 20 | 14 |
| 2 N/A, Agi, Wis | 80 | 17 | 15 | 16 | 16 | 15 |
| 2 Wis, Agi, Int | 80 | 16 | 14 | 15 | 18 | 17 |
| 2 Wis, Agi, N/A | 80 | 17 | 15 | 15 | 18 | 15 |
| 2 Int, Str, Agi | 80 | 17 | 15 | 17 | 13 | 18 |
| 2 Int, 2 N/A | 80 | 14 | 14 | 16 | 16 | 20 |
| 4 N/A | 80 | 16 | 16 | 16 | 16 | 16 |
| 2 Wis, Str, Int | 79 | 16 | 16 | 13 | 18 | 16 |
| 2 Wis, Str, N/A | 79 | 17 | 17 | 13 | 18 | 14 |
| 2 Str, Int, N/A | 79 | 18 | 18 | 15 | 13 | 15 |
| 2 Int, Agi, Wis | 79 | 15 | 13 | 16 | 16 | 19 |
| 2 Int, Agi, N/A | 78 | 15 | 13 | 17 | 14 | 19 |
| 3 Wis, Int | 78 | 14 | 14 | 12 | 21 | 17 |
| 2 N/A, Str, Wis | 78 | 17 | 17 | 14 | 16 | 14 |
| 2 N/A, Agi, Int | 78 | 16 | 14 | 17 | 14 | 17 |
| 3 N/A, Agi | 78 | 17 | 15 | 17 | 14 | 15 |
| 2 Int, Str, Wis | 78 | 15 | 15 | 14 | 16 | 18 |
| 3 Int, Agi | 78 | 14 | 12 | 17 | 14 | 21 |
| 4 Int | 78 | 12 | 12 | 16 | 16 | 22 |
| 3 Wis, N/A | 78 | 15 | 15 | 12 | 21 | 15 |
| 3 N/A, Str | 77 | 17 | 17 | 15 | 14 | 14 |
| 3 Int, Str | 77 | 14 | 14 | 15 | 14 | 20 |
| 2 Wis, Int, N/A | 77 | 14 | 14 | 13 | 19 | 17 |
| 2 N/A, Str, Int | 77 | 16 | 16 | 15 | 15 | 16 |
| 2 Int, Str, N/A | 77 | 15 | 15 | 15 | 14 | 18 |
| 3 Int, Wis | 76 | 12 | 12 | 14 | 17 | 21 |
| 2 Int, Wis, N/A | 76 | 13 | 13 | 14 | 17 | 19 |
| 3 N/A, Wis | 76 | 15 | 15 | 14 | 17 | 15 |
| 2 N/A, Wis, Int | 76 | 14 | 14 | 14 | 17 | 17 |
| 3 Int, N/A | 75 | 12 | 12 | 15 | 15 | 21 |
| 3 N/A, Int | 74 | 14 | 14 | 15 | 15 | 17 |

INTERPRETATION, not stated on the page: the chart appears to list one representative or best roll per answer combination. Its maximum total is 88. "4 N/A" gives 16 in every stat.

### 2.4 Character-class roll advice (B 2002 and A live, same numbers)

VERBATIM from start.html 2002-06-02, repeated on live https://www.t4cbible.com/charclass:

- Warrior: "A good roll is: 22 Str, 22 End, 15+ Agi"
- Mage: "A good roll is: 20+ Wis, 20+ Int, 14+ End"
- Paladin: "A good roll is: 20+ Str, 18+ End, 18+ Wis"
- Battlemage: "A good roll is: 14+ Str, 14+ End, 20+ Int"
- Archer: "A good roll is: 18+ Str, 20+ End, 20+ Agi"
- Cleric: "A good roll is: 22 Wis, 16+ End"
- Thief: "A good roll is: 20+ Agi, 14+ Wis, 14+ Int"
- The live page adds Healer: "A good roll is: 20+ Wis, 15+ Str, 15+ End".

### 2.5 T4C Nostalgia (1.24b server). Server-specific and **modified**

Source: the live JS bundle https://www.t4cnostalgia.com/main-ELL3M3UL.js (component `app-rolling-character`, route `game/rolling-character`), retrieved 2026-10-07. The HTML page itself renders empty without JS.

VERBATIM, string literals, interpolated values filled in from the same component (`classicMaxTotal=88`, `maxTotal=100`, `maxStat=kT=26`, `rollBase=6`, `diceMax=4`, `rollAnswers=4`):
- "T4C Nostalgia change: the original roll system caps the stat total at [88]. On Nostalgia the maximum total was raised to [100] points — reaching it is very hard. The Character Rolling Chart below is generated from the live Nostalgia server formula (max total [100]); a single stat can reach up to [26]."
- "You answer [4] questions during character creation. Each answer is sorted into one of five categories (Str, Agi, Int, Wis or N/A). Every category that receives an answer grants bonus points to your stats, then each stat is rolled as:" `stat = (1–[4]) + [6] + category bonus`
- "The four answers add roughly 50 bonus points total, spread across the stats."
- "**HP** = 2d5 + 48 + END · **Mana** = 10 + INT×2/3 + WIS/3 + (0–5)"
- "Each stat rolls 1–[4], so the real result can be up to 3 lower per stat (total range: Total − 15 up to Total)."
- Release notes in the same bundle: "Classic roll questions (increased from 88 to 100 max points)".
- Code, VERBATIM. Per-answer multipliers; the bonus is `Math.floor(mult*count)` and the chart value is `10 + bonus`:
  `{key:"str",mult:{str:4,agi:2.5,end:4,int:1,wis:1}}, {key:"agi",mult:{str:4,agi:4,end:2,int:1.5,wis:1}}, {key:"int",mult:{str:1,agi:2,end:2,int:4,wis:3.5}}, {key:"wis",mult:{str:2,agi:1.5,end:2.5,int:2.5,wis:4}}, {key:"na",mult:{str:2.5,agi:2.5,end:2.5,int:2.5,wis:2.5}}`
- INTERPRETATION: these formulas describe the **Nostalgia server**, which says it changed the roll system. They are not confirmed as Vircom classic. The classic cap of 88 that Nostalgia quotes matches the top total (88) in XanThor's 2004 chart. By this formula, 4 × N/A gives 20 per stat (10 + floor(2.5 × 4)), while the classic XanThor chart says 16.

### 2.6 Starting HP, mana, gold and kit

- Starting HP: the only number found is a player estimate. VERBATIM, jeuxonline thread (2.6 / 3.1 source), Taylor Asylum, 13/2/2002 18:22:19: "Il ne faut pas oublier qu'à la création du perso on commence à environ 70 pv." [Translation: Don't forget that at character creation you start at about 70 HP.]
- Nostalgia formula (server-specific): "HP = 2d5 + 48 + END", "Mana = 10 + INT×2/3 + WIS/3 + (0–5)" (section 2.5).
- Starting kit. VERBATIM, manual chap13 (https://web.archive.org/web/20041210210451/http://www.t4cbible.com:80/t4c_old/1/Manual/chap13.htm, beta era): "Most T4C servers will provide you with a bit of starting equipment, including a torch or two." VERBATIM, chap03: "When you enter the game, you are wearing only minimal clothing (underwears)."
- **Not found:** starting gold, an exact starting kit list, or exact starting HP/mana for the classic game.

---

## 3. Level-up growth

### 3.1 HP and mana gained per level, by attribute

Primary source: https://web.archive.org/web/20021230103921/http://www.t4cbible.com:80/hp.html, snapshot 2002-12-30, title "T4C Bible - Hit & Mana Points Chart". The values are identical in https://web.archive.org/web/20011221005001/http://www.t4cbible.com:80/various.html (2001-12-21, section "D. HP & MP when leveling", with "*Most of this Info is courtesy of GM-DumMWiaM of Greek Digicon Server*") and in https://web.archive.org/web/20060807200304/http://t4cbible.com:80/index.php?page=hpandmp (2006-08-07). All 45 rows were checked programmatically.

VERBATIM: "In this chart we list the amount of hit and mana points you gain on level up, according to your stats."

Note: the separator character on the page is `&Oslash;` in a `font-family:Wingdings` span, which renders as an arrow. It is written as → below.

| End / HP | Wis / MP | Int / MP |
|---|---|---|
| 1-19 End → 6-8 HP | 1-59 Wis → 0 MP | 1-29 Int → 3-5 MP |
| 20-39 End → 7-9 HP | 60-119 Wis → 1 MP | 30-59 Int → 4-6 MP |
| 40-59 End → 8-10 HP | 120-179 Wis → 2 MP | 60-89 Int → 5-7 MP |
| 60-79 End → 9-11 HP | 180-239 Wis → 3 MP | 90-119 Int → 6-8 MP |
| 80-99 End → 10-12 HP | 240-299 Wis → 4 MP | 120-149 Int → 7-9 MP |
| 100-119 End → 11-13 HP | 300-359 Wis → 5 MP | 150-179 Int → 8-10 MP |
| 120-139 End → 12-14 HP | 360-419 Wis → 6 MP | 180-209 Int → 9-11 MP |
| 140-159 End → 13-15 HP | 420-479 Wis → 7 MP | 210-239 Int → 10-12 MP |
| 160-179 End → 14-16 HP | 480-539 Wis → 8 MP | 240-269 Int → 11-13 MP |
| 180-199 End → 15-17 HP | | 270-299 Int → 12-14 MP |
| 200-219 End → 16-18 HP | | 300-329 Int → 13-15 MP |
| 220-239 End → 17-19 HP | | 330-359 Int → 14-16 MP |
| 240-259 End → 18-20 HP | | 360-389 Int → 15-17 MP |
| 260-279 End → 19-21 HP | | 390-419 Int → 16-18 MP |
| 280-299 End → 20-22 HP | | 420-449 Int → 17-19 MP |
| 300-319 End → 21-23 HP | | 450-479 Int → 18-20 MP |
| | | 480-509 Int → 19-21 MP |
| | | 510-539 Int → 20-22 MP |
| | | 540-569 Int → 21-23 MP |
| | | 570-599 Int → 22-24 MP |

Corroboration (jeuxonline, live https://archives.jeuxonline.info/fils/64035.html, retrieved 2026-10-07; thread "palier d'end et pv das le debut", T4C - Forum Général, posts dated 13–14/2/2002). VERBATIM:
- Taylor Asylum, 13/2/2002 15:39:12: "50 Endu = 8-10 pv par lvl" [50 End = 8-10 HP per level]
- dub, 13/2/2002 15:46:23: "Moi j'ai un perso avec 30 endurance et j'ai 1000 pv au niveau 100. … parfois je chope 8 pv de plus par niveau parfois 9, etc" [I have a char with 30 endurance and 1000 HP at level 100 … sometimes I get 8 more HP per level, sometimes 9, etc.]
- Orion Ystralia, 15:47:55: "Il y a un pallier au lvl 60..." [There is a threshold at lvl 60...]. INTERPRETATION: probably meant End 60, not level.
- pyrotess, 17:04:10: "voila le tableau donnée par le site T4C, et qui a été testé et vérifié !! Nombre de points de vie gagnés à chaque niveau. Rapport End / PV 1-19 End -- 6-8 PV 20-39 End -- 7-9 PV 40-59 End -- 8-10 PV … 300-319 End -- 21-23 PV" [here is the table given by the T4C site, which has been tested and verified!! Number of HP gained at each level. End/HP ratio …]. All 16 bands match the bible table above.
- Gnub, 14/2/2002 12:14:24: "que tu sois à 40, 50 et jusqu'a 59 tu es dans le même palier, donc en moyenne le même nombre de pv par lvl" [whether you are at 40, 50 or up to 59 you are in the same band, so on average the same HP per level]
- ex Thor AkumaGW, 20:10:10: "300end c mieux :D bah vi lvl 126: 2400pv pur..." [300 end is better, lvl 126: 2400 pure HP]

Nostalgia (server-specific, bundle text, VERBATIM): "HP & MP Progression | Original 1.24 progression (no modifications) | HP gained per level is based on Endurance | MP gained per level is based on Wisdom and Intelligence | Follows classic T4C scaling tables".

INTERPRETATION: the table is per level gained, using the stat value at the time of levelling (the jeuxonline posts imply raising End early gives more total HP: "taura plus de pv si tu met l'endu au debut qu'a la fin" [you'll have more HP if you put End in at the start rather than at the end]). The table does not say whether Wis MP and Int MP add together. The Nostalgia text says MP is "based on Wisdom and Intelligence".

### 3.2 Skill and stat points per level

- VERBATIM, manual chap05 (2004-12-10 snapshot, beta-era manual) and getstart.html (2004-10-11): "When you have achieved a level, you will gain 15 skill points that you are free to spend or to keep for later uses." … "whenever you gain a level, you will also gain 5 attribute points that you may distribute as you choose."
- VERBATIM, manual chap09: "These points are necessary for you to learn not just skills, but spells as well".

### 3.3 XP table

Source 1: https://web.archive.org/web/20021211182332/http://www.t4cbible.com:80/xp.html, snapshot 2002-12-11, "T4C Bible - Player XP Chart", "*Most of this Info is courtesy of GM-DumMWiaM of Greek Digicon Server*".

Source 2: https://web.archive.org/web/20040612184134/http://www.t4cbible.com:80/exp.pdf, snapshot 2004-06-12. "THE SECRET OF EXPERIENCE POINT IN T4C … Written by Aequirius". Era: unstated.

VERBATIM formula from exp.pdf (rendered from the PDF text layer):
```
Xp(x) = ( 1000 + Σ_{i=1}^{x-1} P_i ) * (x - 1)^2.5        x = Level
Pi = Pi-1 + ti ,  P1 = 10
1 <= i <= 42 => ti = 0 ; 43 <= i <= 82 => ti = 1 ; 83 <= i <= 122 => ti = 4 ;
123 <= i <= 142 => ti = 16 ; 143 <= i <= 162 => ti = 64 ; 163 <= i <= 182 => ti = 256 ;
183 <= i <= 192 => ti = 1024 ; 193 <= i => ti = 4096
```
The PDF's own C code: `return _int64(coef(level - 1)*pow(level - 1, 2.5));` with `coef(x)` = `1000 + sum_{i=1}^{x-1} coefp(i)`.

INTERPRETATION / caution: the PDF's C code calls `coef(level-1)`, which sums only up to level−2. That differs from the displayed Σ upper bound. The PDF's printed outputs (e.g. "Level 3 = 5713") are what its code produced. The 2002 xp.html values are rounded and differ slightly: level 3 is "5.700" vs "5713", level 20 is "1.857.000" vs "1856803", level 200 is "248.292.513.235" vs "248112513235". The xp.html "XPs to next Level" column is not always consistent with its own "Required XPs" column (e.g. L21 2.128.800 + 297.300 ≠ L22 2.425.300). Both are reproduced as printed. **Verified:** I re-ran the PDF's C logic (`coef(level-1)*(level-1)^2.5`, truncated) and it reproduces the PDF's printed values exactly at levels 3, 20, 100 and 200 (5713, 1856803, 388514575, 248112513235). So the printed table follows the code (sum to level−2), not the Σ-to-x−1 display.

| Level | xp.html 2002 "Required XPs" (verbatim, "." = thousands sep.) | xp.html 2002 "XPs to next Level" | exp.pdf (Aequirius) "Level N =" |
|---|---|---|---|
| 1 | 0 | 1.000 | 0 |
| 2 | 1.000 | 4.700 | 1000 |
| 3 | 5.700 | 10.200 | 5713 |
| 4 | 15.900 | 17.100 | 15900 |
| 5 | 33.000 | 25.100 | 32960 |
| 6 | 58.100 | 34.500 | 58137 |
| 7 | 92.600 | 44.800 | 92590 |
| 8 | 137.400 | 56.300 | 137420 |
| 9 | 193.700 | 68.750 | 193690 |
| 10 | 262.450 | 82.250 | 262440 |
| 11 | 344.700 | 96.742 | 344688 |
| 12 | 441.442 | 112.258 | 441442 |
| 13 | 553.700 | 128.800 | 553702 |
| 14 | 682.500 | 146.200 | 682458 |
| 15 | 828.700 | 164.780 | 828702 |
| 16 | 993.480 | 184.220 | 993420 |
| 17 | 1.177.700 | 204.700 | 1177600 |
| 18 | 1.382.400 | 225.900 | 1382229 |
| 19 | 1.608.300 | 248.700 | 1608300 |
| 20 | 1.857.000 | 271.000 | 1856803 |
| 21 | 2.128.800 | 297.300 | 2128736 |
| 22 | 2.425.300 | 321.600 | 2425099 |
| 23 | 2.746.900 | 348.100 | 2746895 |
| 24 | 3.095.000 | 375.800 | 3095133 |
| 25 | 3.470.800 | 404.200 | 3470828 |
| 26 | 3.875.000 | 433.700 | 3875000 |
| 27 | 4.308.700 | 464.200 | 4308671 |
| 28 | 4.772.900 | 495.800 | 4772873 |
| 29 | 5.268.700 | 528.400 | 5268643 |
| 30 | 5.797.100 | 561.900 | 5797022 |
| 31 | 6.359.000 | 597.000 | 6359058 |
| 32 | 6.956.000 | 633.000 | 6955808 |
| 33 | 7.589.000 | 668.800 | 7588330 |
| 34 | 8.257.800 | 707.200 | 8257693 |
| 35 | 8.965.000 | 746.600 | 8964971 |
| 36 | 9.711.600 | 786.400 | 9711244 |
| 37 | 10.498.000 | 827.300 | 10497600 |
| 38 | 11.325.300 | 869.900 | 11325130 |
| 39 | 12.195.200 | 912.800 | 12194936 |
| 40 | 13.108.000 | 958.000 | 13108125 |
| 41 | 14.066.000 | 1.003.000 | 14065811 |
| 42 | 15.069.000 | 1.050.300 | 15069112 |
| 43 | 16.119.300 | 1.098.200 | 16119157 |
| 44 | 17.217.500 | 1.159.500 | 17217079 |
| 45 | 18.377.000 | 1.225.000 | 18376860 |
| 46 | 19.602.000 | 1.294.000 | 19601875 |
| 47 | 20.896.000 | 1.366.000 | 20895653 |
| 48 | 22.262.000 | 1.442.000 | 22261887 |
| 49 | 23.704.000 | 1.523.500 | 23704431 |
| 50 | 25.227.500 | 1.607.500 | 25227307 |
| 51 | 26.835.000 | 1.696.000 | 26834702 |
| 52 | 28.531.000 | 1.789.000 | 28530977 |
| 53 | 30.320.000 | 1.888.000 | 30320667 |
| 54 | 32.208.000 | 1.991.000 | 32208480 |
| 55 | 34.199.000 | 2.099.000 | 34199305 |
| 56 | 36.298.000 | 2.220.000 | 36298212 |
| 57 | 38.518.000 | 2.326.000 | 38510454 |
| 58 | 40.844.000 | 2.453.000 | 40841471 |
| 59 | 43.297.000 | 2.585.500 | 43296888 |
| 60 | 45.882.500 | 2.722.000 | 45882525 |
| 61 | 48.604.500 | 2.866.500 | 48604391 |
| 62 | 51.471.000 | 3.012.000 | 51468694 |
| 63 | 54.483.000 | 3.167.500 | 54481835 |
| 64 | 57.650.500 | 3.330.500 | 57650418 |
| 65 | 60.981.000 | 3.502.000 | 60981248 |
| 66 | 64.483.000 | 3.676.000 | 64481332 |
| 67 | 68.159.000 | 3.861.000 | 68157887 |
| 68 | 72.020.000 | 4.054.000 | 72018335 |
| 69 | 76.074.000 | 4.246.000 | 76070309 |
| 70 | 80.320.000 | 4.468.000 | 80321655 |
| 71 | 84.788.000 | 4.671.000 | 84780433 |
| 72 | 89.459.000 | 4.901.000 | 89454921 |
| 73 | 94.360.000 | 5.132.000 | 94353613 |
| 74 | 99.492.000 | 5.369.000 | 99485226 |
| 75 | 104.861.000 | 5.628.000 | 104858697 |
| 76 | 110.489.000 | 5.900.000 | 110483190 |
| 77 | 116.389.000 | 6.136.000 | 116368095 |
| 78 | 122.525.000 | 6.432.800 | 122523028 |
| 79 | 128.957.800 | 6.725.200 | 128957839 |
| 80 | 135.683.000 | 7.031.000 | 135682607 |
| 81 | 142.714.000 | 7.334.000 | 142707647 |
| 82 | 150.048.000 | 7.652.000 | 150043509 |
| 83 | 157.700.000 | 7.992.000 | 157700981 |
| 84 | 165.692.000 | 8.527.000 | 165691091 |
| 85 | 174.219.000 | 9.097.000 | 174219116 |
| 86 | 183.316.000 | 9.690.000 | 183314046 |
| 87 | 193.006.000 | 10.321.000 | 193005738 |
| 88 | 203.327.000 | 10.977.000 | 203324924 |
| 89 | 214.304.000 | 11.670.000 | 214303219 |
| 90 | 225.974.000 | 12.395.000 | 225973131 |
| 91 | 238.369.000 | 13.154.000 | 238368062 |
| 92 | 251.523.000 | 13.949.000 | 251522325 |
| 93 | 265.472.000 | 14.781.000 | 265471143 |
| 94 | 280.253.000 | 15.647.000 | 280250663 |
| 95 | 295.900.000 | 16.552.000 | 295897957 |
| 96 | 312.452.000 | 17.498.000 | 312451036 |
| 97 | 329.950.000 | 18.485.000 | 329948855 |
| 98 | 348.435.000 | 19.505.200 | 348431315 |
| 99 | 367.940.200 | 20.574.800 | 367939280 |
| 100 | 388.515.000 | 21.685.000 | 388514575 |
| 101 | 410.200.000 | 22.839.000 | 410200000 |
| 102 | 433.039.000 | 24.039.000 | 433039331 |
| 103 | 457.078.000 | 25.282.000 | 457077334 |
| 104 | 482.360.000 | 26.573.000 | 482359765 |
| 105 | 508.933.000 | 27.914.000 | 508933382 |
| 106 | 536.847.000 | 29.299.241 | 536845948 |
| 107 | 566.146.241 | 30.737.818 | 566146241 |
| 108 | 596.884.059 | 32.226.169 | 596884059 |
| 109 | 629.110.228 | 33.767.772 | 629110228 |
| 110 | 662.878.000 | 35.358.000 | 662876609 |
| 111 | 698.236.000 | 37.004.000 | 698236100 |
| 112 | 735.240.000 | 38.714.000 | 735242649 |
| 113 | 773.954.000 | 40.464.000 | 773951259 |
| 114 | 814.418.000 | 42.282.000 | 814417991 |
| 115 | 856.700.000 | 44.156.000 | 856699973 |
| 116 | 900.856.000 | 46.088.000 | 900855408 |
| 117 | 946.944.000 | 48.081.000 | 946943578 |
| 118 | 995.025.000 | 50.139.000 | 995024850 |
| 119 | 1.045.164.000 | 52.250.000 | 1045160686 |
| 120 | 1.097.414.000 | 54.425.000 | 1097413646 |
| 121 | 1.151.839.000 | 56.684.000 | 1151847393 |
| 122 | 1.208.532.000 | 58.988.000 | 1208526704 |
| 123 | 1.267.520.000 | 61.371.000 | 1267517472 |
| 124 | 1.328.891.000 | 65.875.000 | 1328886716 |
| 125 | 1.394.766.000 | 70.587.000 | 1394757221 |
| 126 | 1.465.353.000 | 75.422.000 | 1465323296 |
| 127 | 1.540.775.000 | 80.568.000 | 1540783411 |
| 128 | 1.621.343.000 | 85.859.000 | 1621340222 |
| 129 | 1.707.202.000 | 91.402.000 | 1707200598 |
| 130 | 1.798.604.000 | 97.085.000 | 1798575648 |
| 131 | 1.895.689.000 | 103.011.000 | 1895680745 |
| 132 | 1.998.700.000 | 109.264.000 | 1998735552 |
| 133 | 2.107.964.000 | 115.630.000 | 2107964046 |
| 134 | 2.223.594.548 | 122.265.192 | 2223594547 |
| 135 | 2.345.859.740 | 129.136.960 | 2345859739 |
| 136 | 2.474.996.700 | 136.250.219 | 2474996699 |
| 137 | 2.611.246.919 | 143.609.410 | 2611246918 |
| 138 | 2.754.856.329 | 151.218.997 | 2754856328 |
| 139 | 2.906.075.326 | 159.083.472 | 2906075325 |
| 140 | 3.065.158.798 | 167.207.347 | 3065158797 |
| 141 | 3.232.366.145 | 175.595.161 | 3232366144 |
| 142 | 3.407.961.306 | 184.251.474 | 3407961305 |
| 143 | 3.592.212.780 | 193.180.877 | 3592212779 |
| 144 | 3.785.393.657 | 214.331.912 | 3785393656 |
| 145 | 3.999.725.569 | 236.390.601 | 3999725568 |
| 146 | 4.236.116.170 | 259.374.608 | 4236116169 |
| 147 | 4.495.490.778 | 283.301.694 | 4495490777 |
| 148 | 4.778.792.472 | 308.189.716 | 4778792471 |
| 149 | 5.086.982.188 | 334.056.628 | 5086982187 |
| 150 | 5.421.038.816 | 360.920.485 | 5421038815 |
| 151 | 5.781.959.301 | 388.799.434 | 5781959300 |
| 152 | 6.170.758.735 | 417.711.719 | 6170758734 |
| 153 | 6.588.470.454 | 447.675.679 | 6588470453 |
| 154 | 7.036.146.133 | 478.709.748 | 7036146132 |
| 155 | 7.514.855.881 | 510.832.456 | 7514855880 |
| 156 | 8.025.688.337 | 544.062.425 | 8025688336 |
| 157 | 8.569.750.762 | 578.418.369 | 8569750761 |
| 158 | 9.148.169.131 | 613.919.099 | 9148169130 |
| 159 | 9.762.088.230 | 650.583.517 | 9762088229 |
| 160 | 10.412.671.747 | 688.430.615 | 10412671746 |
| 161 | 11.101.102.362 | 727.479.480 | 11101102361 |
| 162 | 11.828.581.842 | 767.749.288 | 11828581841 |
| 163 | 12.596.331.130 | 809.260.678 | 12596331129 |
| 164 | 13.405.591.808 | 918.159.410 | 13405590439 |
| 165 | 14.323.751.218 | 1.031.379.434 | 14323751217 |
| 166 | 15.355.130.652 | 1.149.182.309 | 15355130651 |
| 167 | 16.504.312.961 | 1.270.876.801 | 16504120960 |
| 168 | 17.775.189.762 | 1.397.690.676 | 17775189761 |
| 169 | 19.172.880.438 | 1.528.932.071 | 19172880437 |
| 170 | 20.701.812.509 | 1.664.869.478 | 20701812508 |
| 171 | 22.366.681.987 | 1.805.579.761 | 22366681986 |
| 172 | 24.172.261.748 | 1.951.140.140 | 24172261747 |
| 173 | 26.123.401.888 | 2.101.628.196 | 26123401887 |
| 174 | 28.225.030.084 | 2.257.121.870 | 28225030083 |
| 175 | 30.482.151.954 | 2.417.699.456 | 30482151953 |
| 176 | 32.899.851.410 | 2.583.439.606 | 32899851409 |
| 177 | 35.483.291.016 | 2.754.421.324 | 35483291015 |
| 178 | 38.237.712.340 | 2.930.723.969 | 38237712339 |
| 179 | 41.168.436.309 | 3.112.427.249 | 41168436308 |
| 180 | 44.280.863.558 | 3.299.611.218 | 44280863557 |
| 181 | 47.580.474.776 | 3.492.356.286 | 47580474775 |
| 182 | 51.072.831.062 | 3.690.743.201 | 51072831061 |
| 183 | 54.763.574.263 | 3.894.853.063 | 54763574262 |
| 184 | 58.658.427.326 | 4.457.467.571 | 58658427325 |
| 185 | 63.115.894.897 | 5.000.000.000 | 63115894896 |
| 186 | 68.115.894.897 | 5.684.380.591 | 68156298166 |
| 187 | 73.800.275.488 | 6.268.508.276 | 73800275487 |
| 188 | 80.068.783.764 | 6.914.316.086 | 80068783763 |
| 189 | 86.983.099.850 | 7.581.722.080 | 86983099849 |
| 190 | 94.564.821.930 | 8.271.048.973 | 94564821929 |
| 191 | 102.835.870.903 | 8.903.784.220 | 102835870903 |
| 192 | 111.739.655.123 | 9.795.599.824 | 111818491760 |
| 193 | 121.535.254.947 | 10.473.802.789 | 121535254947 |
| 194 | 132.009.057.736 | 12.864.437.343 | 132009057736 |
| 195 | 144.873.495.079 | 15.341.124.333 | 144873495079 |
| 196 | 160.214.619.412 | 17.905.159.852 | 160214619412 |
| 197 | 178.119.779.264 | 20.557.845.457 | 178119779264 |
| 198 | 198.677.624.721 | 23.300.488.152 | 198677624721 |
| 199 | 221.978.112.873 | 26.134.400.362 | 221978112873 |
| 200 | 248.292.513.235 |  | 248112513235 |

---

## 4. Combat: hit, dodge, damage and armour

**No numeric hit, dodge or damage formula was found on any t4cbible page.** What exists:

- VERBATIM, manual chap08 (https://web.archive.org/web/20041210203459/http://www.t4cbible.com:80/t4c_old/1/Manual/chap08.htm, snapshot 2004-12-10, beta-era manual):
  - "Your chances to hit are determined by your Attack Skill and the defender's Dodge Skill (more on this later). The higher your Attack Skill, the higher the odds of successfully hitting someone (or something)."
  - "The amount of damage varies based on your weapon's base damage, your strength attribute and, in the case of magical weapons, a variety of other factors (we're not telling)."
  - "All armors have an Armor Class (AC) value which is subtracted from the damage you receive when you're hit. (Note that some spells are not affected by the armor you wear.)"
  - Death: "You carry with you a Gem of Destiny that teleports you to a safe haven … At the beginning of the game, it teleports you back to the starting point (the temple in Lighthaven)."
  - PvP: "if the minimum level has been set to 5, then players from level 1 to 5 cannot be attacked by other players"; "PvP Range is a plus/minus value…"
- VERBATIM, manual chap05: "AGI : Agility; … Agility is very important in combat since it modifies your Armor Class (AC, a value which makes you more or less hard to hit)." Also: "STR : Strength; this affects how much damage you do with your weapons and how much weight you can carry".
- VERBATIM, manual chap09 (https://web.archive.org/web/20041210204005/http://www.t4cbible.com:80/t4c_old/1/Manual/chap09.htm): "Parry … If the parry attempt succeeds, the damage received from a physical strike is reduced to zero (0)." "Armor Penetration … If it is successful, it reduces the opponent's AC. The higher the skill, the more the defender's AC is reduced (up to a point)." "Aside from the Attack and Dodge skills, no skill can be trained above a rating of 100."
- VERBATIM, live skills page (https://www.t4cbible.com/skills): "Note: All skills max out at 100 except attack, dodge and archery".
- VERBATIM, hints.html (https://web.archive.org/web/20021013040845/http://www.t4cbible.com:80/hints.html, 2002-10-13): "If you train your attack skill (and you have equip a plus weapon) remove your weapon and re-equip it in order to gain the extra bonus from your new train."
- **4GE.net wiki** (live https://4genet.pbworks.com/w/page/968685/Hints%20and%20Tips, retrieved 2026-10-07). Player-submitted, server-specific. VERBATIM, "( Submitted by Nerv02 NM)":
  - "Powerful blow adds 33% more damage to a physical blow."
  - "Armor penetration reduces opponents magical AC (green colored number) to zero and base AC (white colored number) by 1/2."
  - "Stunblow - Opponent is stunned in place, unable to move or attack using weapons (bows/swords etc) but does not stop spell casting."
  - "Attack/Archery - Higher your attack/archery, the higher your chance to hit targets with dodge. For every 2 attack/archery : 1 dodge, chance to hit is 70-75%"
  - "Strength - Every 5 stat points added to strength adds an additional 1 damage to melee attacks. Every 10 strength adds 1 damage to ranged weapons."
  - "Agility - Every 10 stat points invested in agility adds 1 damage to ranged weapons."
  - "Mental spells ignores targets Armor."
- Nostalgia (server-specific, bundle): "Equipment Refinement | Weapons to +5 (+10% dmg and accuracy each level)"; "Combat Adjustment: | Weapons / Bows / Maces reduce | Dodge | effectiveness (25%)".
- Jeuxonline (Gnub, 13/2/2002 18:12:48): "les 6 de c.a du cuir clouté" [the 6 AC of studded leather].

### 4.1 Encumbrance

https://web.archive.org/web/20021211172845/http://www.t4cbible.com:80/enc.html, snapshot 2002-12-11. VERBATIM: "Calculate your Encumbrance: Enc = (Str * 500) / (Str + 100)". Sample rows: "25 Str → 100 Enc", "50 Str → 166 Enc", "100 Str → 250 Enc", "200 Str → 333 Enc", "400 Str → 400 Enc", "610 Str → 429 Enc". The page has the same Wingdings arrow glyph.

---

## 5. Mana regeneration

- VERBATIM, manual chap09: "Meditate … Meditation is always successful and simply increases the recovery rate of lost mana."
- VERBATIM, manual chap10 (https://web.archive.org/web/20041210204755/http://www.t4cbible.com:80/t4c_old/1/Manual/chap10.htm): "Some potions (such as the Potion of Mana ) and skills (such as Meditate ) allow you to restore your mana." The mana-cost paragraph is truncated in the capture: "When you have no more magic points (MPs), you need to wait before you".
- VERBATIM, manual chap10, exhaustion: "There are three types of exhaustion: mental, physical and movement. … Exhaustion lasts for a variable period of time depending on the spell and the exhaustion type."
- VERBATIM, various.html 2001-12-21, potions: "Potion of Mana | +25 Mana Points", "Mana Elixir | +50 Mana Points", "Manastone | +Half your normal Mana Points", "Mana Prism | Regenerates mana very fast for 2 min", "Potion of Regeneration | Regenerate 1/30 of you max Hit Points every 2 sec for 2 min", "Light Healing Potion | +25 Hit Points", "Healing Potion | +50 Hit Points", "Serious Healing Potion | +100 Hit Points", "Critical Healing Potion | +250 Hit Points", "Potion of Fury | +10 Strength for 5 min".
- Live https://www.t4cbible.com/Potions agrees: "Potion of Light Healing | +25 Hit Points | 2" (the last column is Enc), "Potion of Mana | +25 Mana Points | 2". Live Items: "Potion of Mana | Arakas, buy from Fali & Yolak (50 GP)"; "Potion of Healing | Arakas, buy from Fali & Yolak (125 GP)"; "Torch | Buy from all Pot shops (12 GP)".
- Nostalgia (server-specific): "Meditation - Increased recovery speed.", "Rapid Healing - Increased recovery speed."
- **Not found:** any numeric base HP or mana regeneration rate.

Game time (manual chap13, beta-era), VERBATIM: "1 minute | 3 seconds | 20 minutes | 1 minute | 1 hour | 3 minutes | 6 hours | 18 minutes | 24 hours | 1 hour 12 minutes." (game time | real time pairs). Also: "Torches last approximately 10 minutes of real time".

---

## 6. Skills: requirements, costs and trainers

Live https://www.t4cbible.com/skills (retrieved 2026-10-07). The 16 classic skills are **identical in requirements and cost** to https://web.archive.org/web/20011221005225/http://www.t4cbible.com:80/skills.html (snapshot 2001-12-21), checked programmatically. Small trainer-name differences exist; e.g. 2001 lists "Steelblade Delnar" for Attack/Dodge/Archery in SC, and "SC - Elandor (train)" for Meditate where live has "SC - Eldantor (teach & train)". Critical Strike, Primal Scream, Immobilization, Power Conjuring, Resurrect and Loot appear **only on live**.

The table below is VERBATIM from live (header "Skill Name | Requirments | Teacher/Trainer | Cost"):

| Skill | Requirements | Teacher/Trainer | Cost |
|---|---|---|---|
| Attack | None | LH - Murmuntag, Ortanalas / WH - Karl, Garnir / SS - Derran Ironstrife, Doremas / SC - Eldrig | 10 train |
| Dodge | None | LH - Kalastor / WH - The Lurker / SS - Doremas, Baldric Silverknife / RD Woods - Morindin Arrowmist / SC - Mirymwen Featherfoot | 10 train |
| Archery | None | LH - Kalastor, Ortanalas / WH - Karl / SS - Baldric Silverknife / RD Woods - Morindin Arrowmist / SC - Mirymwen Featherfoot | 15 train |
| Stun Blow | Lvl 3, 20 Agi & 25 Str | LH - Jagar Kar (teach), Ortanalas (train) / WH - Arganor Iargh (teach), Garnir (train) / SS - Derran Ironstrife (train) | 150 learn / 20 train |
| Peek | Lvl 5, 30 Agi | LH - A Dark Figure (teach), Kalastor (train) / WH - The Lurker (train) / Arakas Thieves Town - Lyria (train) / SS - Baldric Silverknife (teach & train) | 500 learn / 25 train |
| Parry | Lvl 10, 30 Agi & 20 Int | WH - Arganor Iargh (teach), Garnir (train) / SS - Doremas (train) / RD Woods - Morindin Arrowmist (teach & train) | 75 train / 900 learn |
| Picklock | Lvl 12, 40 Agi | Arakas Thieves Town - Lyria (teach & train) / WH - The Lurker (train) | 1600 learn / 100 train |
| First Aid | Lvl 12, 20 Int, 20 Wis | RD Woods - Morindin Arrowmist (teach & train) / SH - Greenleaf Roen (teach & train) | 1000 learn / 30 train |
| Hide | Lvl 13, 34 Agi, 20 Int | SC Thieve's Hideout - Meltar Winterstorm (teach & train) / (You must first say "Hide" to Chryseida Yolangda) | 1325 learn / 25 train |
| Powerful Blow | Lvl 15, 30 Agi & 50 Str | LH - Jagar Kar (teach), Ortanalas (train) / WH - Arganor Iargh (teach), Garnir (train) / SS - Derran Ironstrife (train), Adriana (teach) / | 2500 learn / 50 train |
| Meditate | Lvl 16, 30 Int & 30 Wis | Arakas Druids - Rainmist Lantalir (teach & train) / SC - Eldantor (teach & train) | 3000 learn / 100 train |
| Rob * | Lvl 17, 50 Agi & 25 Peek skill | SS - Baldric Silverknife (teach & train) / SC - Lightfoot Daran (teach & train) | 5000 learn / 250 train |
| Search | Lvl 20, 25 Int | SC - Chryseida Yolangda (teach & train) | 2500 learn / 50 train |
| Sneak | Lvl 24, 75 Agi | SC Thieves' Hideout - Dantalir the Bard (teach & train) | 2500 learn / 100 train |
| Armour Penetration | Lvl 25, 40 Agi, 75 Str & 30 Int | SS - Adriana (teach & train) / SC Thieve's Hideout - Rablek Swiftblade (teach & train) | 7500 learn / 300 train |
| Rapid Healing | Lvl 30, 80 End | Arakas - Mhorgwloth the Troll (teach & train) / SH Green Skraug - Worgwloth Trugg (teach & train) | 5000 learn / 200 train |
| Critical Strike | Lvl 35, Str 70,Int 30 | Arakas - Thragor the Brave in Windhowl (teach & train) [teacher <td> is unclosed in the page HTML; restored here] | 250 learn / 250 train |
| Primal Scream | Lvl 12, Str 38, | Krapath Geanileana in Brigand Camp near Lighthaven(teach & train) [teacher <td> is unclosed in the page HTML; restored here] | 5000 learn / 200 train |
| Immobilization | Lvl 51, Str 56, Agi 47 | Mikla Eksilu at Zahkar’s Tower (teach & train) [teacher <td> is unclosed in the page HTML; restored here] | 8000 learn / 300 train |
| Power Conjuring | Lvl 35, Int 150, Wis 75 | Krivythas Eastul in Windhowl’s Mage Tower(teach & train) [teacher <td> is unclosed in the page HTML; restored here] | 8000 learn / 300 train |
| Resurrect | Lvl 17, Wis 50 | Jhilsara Farven in Arakas Druid Camp(teach & train) [teacher <td> is unclosed in the page HTML; restored here] | 8000 learn / 300 train |
| Loot | Lvl 17, Agi 50 | Nalryn in Arakas Brigand’s Hideout (teach & train) [teacher <td> is unclosed in the page HTML; restored here] | 6000 learn / 200 train |

VERBATIM footnote (live): "* Minimum level requirements for skill "Rob" is level 17 but… Baldric Silverknife, skill teacher, resides at Silversky, so you cannot learn it until you can access Raven Dust (which is at 25th level of your character)…"

VERBATIM descriptions (live), selected: "Attack (Passive) : Determines whether a physical attack hits or misses." "Dodge (Passive) : Determines whether a melee or ranged attack is avoided or not." "Powerful Blow (Passive) : Increases the amount of damage done during physical combat." "Rapid Healing (Passive) : Increases a character's ability to regenerate lost hit points." "First Aid : … first aid cannot be performed on others."

Manual (beta) on training, VERBATIM: "Once you have learned a skill, you can train it. This costs you both skill points and gold pieces."

---

## 7. Spells: requirements, costs and trainers

### 7.1 2002 spell chart, with measured damage

https://web.archive.org/web/20020202213610/http://www.t4cbible.com:80/spells.html, snapshot 2002-02-02. VERBATIM notes:
- "(New added spells (version 1.20c) are in light blue color)"
- On the Damage column: "we decided to measure the raw dmg using the exact requirements for every spell in wis and int, base caster's power of 100, base target's resistances of 100 and no items on either the caster nor the target that could affect the measure. … as the character grows up adding wis and int, spell dmg is increased accordingly."
- "**** Mana cost for the Gateway spell is the full mana points of your character."

Section A of the page ("sorted by Level Requirement"), one row per spell, VERBATIM. The row count (66) was checked against a separate raw-HTML cell parse:

| Spell | Lvl | Wis | Int | MP | Reqs | Pts | Gold | Damage | Trainer |
|---|---|---|---|---|---|---|---|---|---|
| Fire Dart | 2 | 15 | 21 | 1 | - | 5 | 532 | 8-23 | LH Mage Tower-Iraltok |
| Light | 2 | 15 | 18 | 10 | - | 5 | 233 | - | LH Temple-Kilhaim |
| Heal Light | 3 | 19 | 15 | 2 | - | 9 | 897 | - | LH Temple-Moonrock |
| Stone Shard | 4 | 20 | 17 | 2 | - | 6 | 1328 | 13-21 | LH Mage Tower-Uranos |
| Cure Poison | 5 | 17 | 26 | 2 | - | 9 | 1825 | - | LH Temple-Moonrock |
| Dust Devil | 6 | 21 | 21 | 2 | - | 7 | 2388 | 8-18 | LH Dungeon-Shovanis |
| Poison | 7 | 18 | 30 | 2 | - | 7 | 3017 | 1-2 | LH Mage Tower-Lothan |
| Protection | 8 | 25 | 20 | 9 | - | 7 | 3712 | - | LH Temple-Moonrock |
| Ice Shard | 9 | 19 | 34 | 3 | - | 8 | 4473 | 15-36 | LH Mage Tower-Lothan |
| Flaming Arrow | 10 | 15 | 44 | 3 | Fire Dart | 8 | 5300 | 19-46 | LH Mage Tower-Iraltok |
| Lesser Drain | 12 | 15 | 50 | 3 | - | 13 | 7152 | 12-24 | LH Temple-Araknor * |
| Heal Serious | 13 | 34 | 16 | 4 | Heal Light | 14 | 8177 | - | LH Temple-Moonrock |
| Lightning Bolt | 15 | 30 | 30 | 4 | Dust Devil | 10 | 10425 | 14-39 | AR Druids-Hornwind Dunikus |
| Poison Arrow | 17 | 23 | 51 | 5 | Poison | 10 | 12937 | 18-43 | LH Mage Tower-Lothan |
| Shatter | 18 | 37 | 26 | 5 | Stone Shard | 11 | 14292 | 29-33 | LH Mage Tower-Uranos |
| Turn Undead | 19 | 43 | 16 | 5 | - | 17 | 15713 | 22-27 | RD Temple-Brother Thorkas |
| Curse | 20 | 16 | 74 | 5 | - | 17 | 17200 | - | LH Dungeon-Shovanis |
| Freeze | 20 | 24 | 57 | 5 | Ice Shard | 11 | 17200 | 25-44 | WH Mage Tower-Marsac Cred |
| Barrier | 21 | 24 | 59 | 10 | Protection | 12 | 18753 | - | RD Druids-Riverbane Kederic |
| Word of Recall | 21 | 36 | 36 | 100 | - | 12 | 18753 | - | LH Dungeon-Shovanis |
| Fire Bolt | 22 | 16 | 80 | 6 | Flaming Arrow | 12 | 20372 | 37-81 | WH Mage Tower-Clar Liurn |
| Drain Life | 23 | 16 | 83 | 6 | Lesser Drain | 19 | 22057 | 21-38 | RD-Blind Man ** |
| Earthen Strength | 23 | 43 | 29 | 30 | - | 12 | 22057 | - | RD Temple-Brother Giamas |
| Minor Combat Sense | 24 | 31 | 55 | 6 | - | 13 | 23808 | - | SSK-Blackrose Elysana |
| Ice Bolt | 25 | 26 | 68 | 6 | Ice Shard | 13 | 25625 | 36-73 | WH Mage Tower-Marsac Cred |
| Call Lightning | 26 | 41 | 42 | 7 | Lightning Bolt | 13 | 27058 | 25-55 | AR Druids-Hornwind Dunikus |
| Fireball | 27 | 16 | 94 | 7 | Fire Bolt | 14 | 29457 | 45-96 | WH Mage Tower-Clar Liurn |
| Heal Critical | 27 | 55 | 17 | 7 | Heal Serious | 21 | 29457 | - | AR Druids-Earthsong Yrian |
| Clear Thought | 28 | 28 | 74 | 21 | - | 14 | 31472 | - | RD Druids-Riverbane Kederic |
| Mana Shield | 29 | 28 | 76 | 14 | Barrier | 14 | 33553 | - | RD Woods-Etheanan |
| Chain Lightning | 30 | 45 | 46 | 7 | Call Lightning | 15 | 35700 | 31-66 | RD Tower Of Sorcery-Zhakar |
| Entangle | 31 | 52 | 34 | 10 | - | 15 | 37319 | - | SH Kaph Leth Village-Shaman Weethgwotha |
| Mana Burst | 32 | 39 | 63 | 8 | - | 15 | 40192 | 38-62 | SSK-Blackrose Elysana |
| Plague Spell | 33 | 16 | 112 | 40 | - | 24 | 42537 |  | RD-Blind Man ** |
| Detect Hidden | 35 | 15 | 118 | 90 | - | 16 | 47425 | - | RD Castle-Starbolt Aloysious |
| Stone Skin | 37 | 59 | 37 | 45 | Protection | 17 | 52577 | - | RD Temple-Brother Giamas |
| Tranquility | 38 | 71 | 17 | 45 | Heal Critical | 26 | 55252 | - | RD Temple-Brother Thorkas |
| Flame Wave | 40 | 16 | 133 | 10 | Fireball | 18 | 60800 | 52-111 | RD Castle-Starbolt Aloysious |
| Vortex of Air | 40 | 55 | 56 | 10 | Chain Lightning | 18 | 60800 | 35-86 | RD Tower Of Sorcery-Zhakar |
| Greater Drain | 41 | 16 | 136 | 10 | Drain Life | 28 | 63673 | 36-64 | RD-Blind Man ** |
| Resist Fire | 43 | 34 | 105 | 10 | Mana Shield | 19 | 69617 | - | RD Druids-Waterbreeze Celestina |
| Nimbleness | 44 | 59 | 60 | 11 | - | 19 | 72688 | - | RD Tower Of Sorcery-Zhakar |
| Ice Ball | 45 | 35 | 110 | 11 | Ice Bolt | 20 | 75825 | 66-123 | RD Druids-Waterbreeze Celestina |
| Resist Ice | 46 | 15 | 150 | 11 | Mana Shield | 20 | 79028 | - | RD Castle-Starbolt Aloysious |
| Earthquake | 47 | 71 | 43 | 11 | Shatter | 20 | 82297 | 72-100 | RD Temple-Brother Giamas |
| Fire Shield | 48 | 16 | 156 | 22 | Flame Wave | 22 | 85632 | 9-15 | RD Woods-Etheanan |
| Electric Shield | 49 | 64 | 65 | 24 | Chain Lightning | 21 | 89033 | 6-11 | RD Woods-Etheanan |
| Detect Invisibility | 50 | 16 | 162 | 60 | - | 21 | 92500 | - | RD Castle-Starbolt Aloysious |
| Sanctuary | 52 | 91 | 18 | 120 | Mana Shield & Stone Skin | 33 | 99632 | - | RD Castle-Bishop Crowbanner |
| Rain of Fire | 53 | 17 | 171 | 12 | Flame Wave | 22 | 103297 | 94-170 | RD Woods-Etheanan |
| Mana Surge | 54 | 17 | 174 | 39 | - | 23 | 107028 | - | RD Woods-Etheanan |
| Glacier | 55 | 40 | 130 | 13 | Ice Ball | 23 | 110825 | 78-146 | RD Woods-Etheanan |
| Healing | 55 | 96 | 18 | 13 | Heal Critical | 35 | 110825 | - | RD Castle-Bishop Crowbanner |
| Dispel | 56 | 57 | 99 | 13 | - | 28 | 170913 | - | SH-Invisible Man ***** |
| Invisibility | 58 | 72 | 74 | 70 | - | 24 | 122612 | - | SH-Invisible Man ***** |
| Bless | 59 | 102 | 19 | 140 | - | 37 | 126673 | - | SC Temple-Nissus Haloseeker |
| Flare | 60 | 17 | 191 | 14 | Rain of Fire | 25 | 130300 | 87-144 | SH-Invisible Man ***** |
| Mass Healing | 63 | 108 | 19 | 15 | Healing | 39 | 143557 | - | SC Temple-Nissus Haloseeker |
| Ice Storm | 65 | 44 | 152 | 15 | Glacier & Freeze | 26 | 152425 | 100-174 | SC-Mithanna Snowraven |
| Major Combat Sense | 66 | 64 | 114 | 15 | Minor Combat Sense | 27 | 156948 | - | SH-Invisible Man ***** |
| Boulders | 67 | 95 | 55 | 16 | Earthquake | 27 | 161537 | 112-147 | SH Kaph Leth Village-Shaman Weethgwotha |
| Soulsteal | 68 | 17 | 215 | 16 | Greater Drain | 41 | 166192 | 63-104 | SH Mordenthal's Castle-Sir Mordenthal |
| True Sight | 69 | 98 | 56 | 80 | - | 28 | 170913 | - | SH Kaph Leth Village-Shaman Weethgwotha |
| Gateway *** | 70 | 84 | 86 | **** | Word of Recall | 5 | 175700 | - | SH Sunken Woods-Healthy Looking Old Man |
| Blizzard | 74 | 48 | 170 | 17 | Glacier | 29 | 195508 | 95-143 | SC-Mithanna Snowraven |
| Healing Mist | 75 | 125 | 20 | 17 | Mass Healing | 45 | 200625 | - | SC Temple-Nissus Haloseeker |

### 7.2 Live spell chart

https://www.t4cbible.com/Spells, retrieved 2026-10-07. No Damage column; an Element column was added. VERBATIM. The row count (75) was checked against a raw-HTML cell parse:

| Spell | LvL | Wis | Int | Mp | Prerequisites | Points | Gold | Teacher | Element |
|---|---|---|---|---|---|---|---|---|---|
| Fire Dart | 2 | 15 | 21 | 1 | - | 5 | 532 | LH Mage Tower - Iraltok | Fire |
| Light | 2 | 15 | 18 | 10 | - | 5 | 233 | LH Temple-Kilhiam | Light |
| Heal Light | 3 | 19 | 15 | 2 | - | 9 | 897 | LH Temple-Moonrock | Light |
| Stone Shard | 4 | 20 | 17 | 2 | - | 6 | 1328 | LH Mage Tower-Uranos | Earth |
| Cure Poison | 5 | 17 | 26 | 2 | - | 9 | 1825 | LH Temple-Moonrock | Water |
| Dust Devil | 6 | 21 | 21 | 2 | - | 7 | 2388 | LH Dungeon-Shovanis | Air |
| Poison | 7 | 18 | 30 | 2 | - | 7 | 3017 | LH Mage Tower-Lothan | Water |
| Protection | 8 | 25 | 20 | 9 | - | 7 | 3712 | LH Temple-Moonrock | Earth |
| Ice Shard | 9 | 19 | 34 | 3 | - | 8 | 4473 | LH Mage Tower-Lothan | Water |
| Flaming Arrow | 10 | 15 | 44 | 3 | Fire Dart | 8 | 5300 | LH Mage Tower-Iraltok | Fire |
| Lesser Drain | 12 | 15 | 50 | 3 | - | 13 | 7152 | LH Temple-Araknor * | Dark |
| Heal Serious | 13 | 34 | 16 | 4 | Heal Light | 14 | 8177 | LH Temple-Moonrock | Light |
| Lightning Bolt | 15 | 30 | 30 | 4 | Dust Devil | 10 | 10425 | AR Druids-Hornwind Dunikus | Air |
| Poison Arrow | 17 | 23 | 51 | 5 | Poison | 10 | 12937 | LH Mage Tower-Lothan | Water |
| Shatter | 18 | 37 | 26 | 5 | Stone Shard | 11 | 14292 | LH Mage Tower-Uranos | Earth |
| Turn Undead | 19 | 43 | 16 | 5 | - | 17 | 15713 | RD Temple-Brother Thorkas | Light |
| Curse | 20 | 16 | 74 | 5 | - | 17 | 17200 | LH Dungeon-Shovanis | Dark |
| Freeze | 20 | 24 | 57 | 5 | Ice Shard | 11 | 17200 | WH Mage Tower-Marsac Cred | Water |
| Barrier | 21 | 24 | 59 | 10 | - | 12 | 18753 | RD Druids-Riverbane Kederic | Water |
| Word of Recall | 21 | 36 | 36 | 100 | - | 12 | 18753 | LH Dungeon-Shovanis | Air |
| Fire Bolt | 22 | 16 | 80 | 6 | Flaming Arrow | 12 | 20372 | WH Mage Tower-Clar Liurn | Fire |
| Drain Life | 23 | 16 | 83 | 6 | Lesser Drain | 19 | 22057 | RD-Blind Man | Dark |
| Earthen Strength | 23 | 43 | 29 | 30 | - | 12 | 22057 | RD Temple-Brother Giamas | Earth |
| Minor Combat Sense | 24 | 31 | 55 | 6 | - | 13 | 23808 | SSK-Blackrose Elysana | None |
| Ice Bolt | 25 | 26 | 68 | 6 | Ice Shard | 13 | 25625 | WH Mage Tower-Marsac Cred | Water |
| Call Lightning | 26 | 41 | 42 | 7 | Bolt | 13 | 27058 | AR Druids-Hornwind Dunikus | Air |
| Fireball | 27 | 16 | 94 | 7 | Fire Bolt | 14 | 29457 | WH Mage Tower-Clar Liurn | Fire |
| Heal Critical | 27 | 55 | 17 | 7 | Heal Serious | 21 | 29457 | AR Druids-Earthsong Yrian | Light |
| Clear Thought | 28 | 28 | 74 | 21 | - | 14 | 31472 | RD Druids-Riverbane Kederic | Water |
| Mana Shield | 29 | 28 | 76 | 14 | Protection & Barrier | 14 | 33553 | RD Woods-Etheanan | Water |
| Chain Lightning | 30 | 45 | 46 | 7 | Call Lightning | 15 | 35700 | RD Tower Of Sorcery-Zhakar | Air |
| Entangle | 31 | 52 | 34 | 10 | - | 15 | 37913 | SH Kahp Leth Village-Shaman Weethgwotha | Earth |
| Mana Burst | 32 | 39 | 63 | 8 | - | 15 | 40192 | SSK-Blackrose Elysana | None |
| Plague Spell | 33 | 16 | 112 | 40 | - | 24 | 42537 | RD-Blind Man | Dark |
| Detect Hidden | 35 | 15 | 118 | 90 | - | 16 | 47425 | RD Castle-Starbolt Aloysious | Fire |
| Stone Skin | 37 | 59 | 37 | 45 | Protection | 17 | 52577 | RD Temple-Brother Giamas | Earth |
| Tranquility | 38 | 71 | 17 | 45 | Heal Critical | 26 | 55252 | RD Temple-Brother Thorkas | Light |
| Flame Wave | 40 | 16 | 133 | 10 | Fireball | 18 | 60800 | RD Castle-Starbolt Aloysious | Fire |
| Vortex of Air | 40 | 55 | 56 | 10 | Chain Lightning | 18 | 60800 | RD Tower Of Sorcery-Zhakar | Air |
| Greater Drain | 41 | 16 | 136 | 10 | Drain Life | 28 | 63673 | RD-Blind Man | Dark |
| Resist Fire | 43 | 34 | 105 | 10 | Mana Shield | 19 | 69617 | RD Druids-Waterbreeze Celestina | Fire |
| Nimbleness | 44 | 59 | 60 | 11 | - | 19 | 72688 | RD Tower Of Sorcery-Zhakar | Air |
| Ice Ball | 45 | 35 | 110 | 11 | Ice Bolt | 20 | 75825 | RD Druids-Waterbreeze Celestina | Water |
| Resist Ice | 46 | 15 | 150 | 11 | Mana Shield | 20 | 79028 | RD Castle-Starbolt Aloysious | Water |
| Earthquake | 47 | 71 | 43 | 11 | Shatter | 20 | 82297 | RD Temple-Brother Giamas | Earth |
| Fire Shield | 48 | 16 | 156 | 22 | Flame Wave | 22 | 85632 | RD Woods-Etheanan | Fire |
| Electric Shield | 49 | 64 | 65 | 24 | Chain Lightning | 21 | 89033 | RD Woods-Etheanan | Air |
| Detect Invisibility | 50 | 16 | 162 | 60 | - | 21 | 92500 | RD Castle-Starbolt Aloysious | Fire |
| Sanctuary | 52 | 91 | 18 | 120 | Mana Shield & Stone Skin | 33 | 99632 | RD Castle-Bishop Crowbanner | Light |
| Rain of Fire | 53 | 17 | 171 | 12 | Flame Wave | 22 | 103297 | RD Woods-Etheanan | Fire |
| Mana Surge | 54 | 17 | 174 | 39 | - | 23 | 107028 | RD Woods-Etheanan | None |
| Glacier | 55 | 40 | 130 | 13 | Ice Ball | 23 | 110825 | RD Woods-Etheanan | Water |
| Healing | 55 | 96 | 18 | 13 | Heal Critical | 35 | 110825 | RD Castle-Bishop Crowbanner | Light |
| Dispel | 56 | 57 | 99 | 13 | - | 28 | 170913 | SH-Invisible Man | None |
| Invisibility | 58 | 72 | 74 | 70 | - | 24 | 122612 | SH-Invisible Man | Air |
| Bless | 59 | 102 | 19 | 140 | - | 37 | 126673 | SC Temple-Nissus Haloseeker | Light |
| Flare | 60 | 17 | 191 | 14 | Rain of Fire | 25 | 130300 | SH-Invisible Man | Fire |
| Mass Healing | 63 | 108 | 19 | 15 | Healing | 39 | 143557 | SC Temple-Nissus Haloseeker | Light |
| Ice Storm | 65 | 44 | 152 | 15 | Glacier & Freeze | 26 | 152425 | SC-Mithanna Snowraven | Water |
| Major Combat Sense | 66 | 64 | 114 | 15 | Minor Combat Sense | 27 | 156948 | SH-Invisible Man | None |
| Boulders | 67 | 95 | 55 | 16 | Earthquake | 27 | 161537 | SH Kahp Leth Village-Shaman Weethgwotha | Earth |
| Soulsteal | 68 | 17 | 215 | 16 | Greater Drain | 41 | 166192 | SH Mordenthal's Castle-Sir Mordenthal | Dark |
| True Sight | 69 | 98 | 56 | 80 | - | 28 | 170913 | SH Kahp Leth Village-Shaman Weethgwotha | Earth |
| Gateway | 70 | 84 | 86 | Full | Word of Recall | 5 | 175700 | SH Sunken Woods-Healthy Looking Old Man | Air |
| Blizzard | 74 | 48 | 170 | 17 | Ice Storm | 29 | 195508 | SC-Mithanna Snowraven | Water |
| Healing Mist | 75 | 125 | 20 | 17 | Mass Healing | 45 | 200625 | SC Temple-Nissus Haloseeker | Light |
| Firestorm | 76 | 17 | 238 | 18 | Flare | 30 | 205808 | RD Library-Filandrius | Fire |
| Tornado | 79 | 93 | 96 | 18 | Vortex of Air | 31 | 221753 | RD Library-Filandrius | Air |
| Avalanche | 82 | 52 | 187 | 19 | Blizzard | 32 | 238292 | RD Library-Filandrius | Water |
| Inferno | 83 | 17 | 259 | 19 | Flare | 32 | 243937 | RD Library-Filandrius | Fire |
| Ice Shield | 84 | 53 | 191 | 100 | - | 33 | 249648 | RD Library-Filandrius | Water |
| Hurricane | 88 | 102 | 105 | 20 | Tornado | 34 | 273152 | RD Library-Filandrius | Air |
| Portal | 92 | 106 | 109 | Full | Relevant Gateway | 10 | 297712 | RD Library-Filandrius | None |
| Tsunami | 96 | 58 | 217 | 22 | Avalanche | 37 | 323328 | RD Library-Filandrius | Water |
| Meteor | 100 | 18 | 309 | 23 | Firestorm & Inferno | 38 | 350000 | RD Library-Filandrius | Fire |

INTERPRETATION (programmatic comparison of spells that appear in both tables): Lvl, Wis, Int, MP, Points and Gold are identical except for these differences:
- Entangle Gold: 37319 (2002) vs 37913 (live)
- Prerequisites:
  - Barrier: "Protection" (2002) vs "-" (live)
  - Call Lightning: "Lightning Bolt" vs "Bolt"
  - Mana Shield: "Barrier" vs "Protection & Barrier"
  - Blizzard: "Glacier" vs "Ice Storm"

These spells are in the live table only (the names do not occur anywhere in the 2002 page): Firestorm, Tornado, Avalanche, Inferno, Ice Shield, Hurricane, Portal, Tsunami, Meteor. Gateway's 2002 row was not parsed because of the "***" footnote marker.

### 7.3 Manual (beta) sample spells: different mana values

VERBATIM, manual chap10 (2004-12-10 snapshot, 0.35-beta manual): "Light | Mana: 1 …", "Fire Dart | Mana: 2 | Attack: Physical | Target: Living | Duration: Instant | Element: Fire", "Poison | Mana: 6 | Attack: Mental", "Ice Shard | Mana: 4 | Attack: Physical", "Dust Devil | Mana: 3 | Attack: Mental", "Heal Light | Mana: 3 | Attack: Mental | Target: PC … Modified by high Wisdom.", "Stone Shard | Mana: 3 | Attack: Physical".

These mana costs differ from both bible charts (e.g. Fire Dart MP 1, Light MP 10, Heal Light MP 2 in the bible). Also VERBATIM: "Attacks can be physical or mental. If they are physical, then physical protection (such as armor) applies against the damage taken. If they are mental, then physical protection doesn't apply."

### 7.4 Spell-damage study (player research)

https://web.archive.org/web/20020123134044/http://www.t4cbible.com:80/spellstud.html, snapshot 2002-01-23, "by Hawk, player of the Digicon Server". VERBATIM:
- "Most of the following have not been certified by GMs and are just my personal conclusions so be careful"
- "(dmg of the spell in the specific int) * power (percentage) / resistance (percentage) = dmg"
- "remember that having 100 power means having +100 from the original 100, thus 200"
- "my speculations are that for every 100 Intelligence above the spell's base requirement, the base spell damage goes up by 8-12% (not sure of course)"
- "Mental Spells (Shatter, freeze, mana burst, vortex of air, flare, dark spells, inferno, tsunami etc) are not AT ALL affected by AC. On the other side, physical spells (boulders, meteor) are, besides resistance, affected in a great degree by your opponent's AC."
- "The speed of spells maxes out at 1 hit/sec, except mana burst that maxes out at 2 hits/sec."

---

## 8. Monsters: the Lighthaven temple basement set

**Location data actually found.** VERBATIM, boss tables, 2001 drops and live Monster1: "Balork | Flowing Black Robe (dr), Light Heal | AR LH Temple Dungeon Level 4". Live adds "| 15:00" in the "Respawn" column, and the live gold cell is blank. The t4cnostalgia bundle also has the string "Lighthaven, Temple Dungeon lvl #4", and in its karma list "Positive karma for having Balork's Brand removed."

None of the fetched pages say which other monsters live in the LH temple basement. The rows below are the creatures named in the task; their basement location is **not** confirmed by these sources.

### 8.1 Stats

Sources:
- t4cbible 2002 = https://web.archive.org/web/20020202144834/http://www.t4cbible.com:80/monster.html (snapshot 2002-02-02)
- t4cbible live = https://www.t4cbible.com/Monster (2026-10-07)
- t4cfantasy = https://t4cfantasy.com/Bible/Classic/Monster.php (live 2026-10-07)

Column headers are VERBATIM "Monster Name | Lvl | XP +1 | XP +50 | XP +100 | Spells/Abilities | Min Dmg | Max Dmg | HP". t4cfantasy labels the damage columns "Dmg | Dmg+". **No page defines "XP +1 / +50 / +100".** Live note, VERBATIM: "Note: The monster damage that we list, is the melee monster damage. Monster spells does different damages."

Row markup is the HTML font colour or italic on the name cell. Per the legend: no colour = Black = Arakas; #FF0000 = "Red : Beta Monsters (Summon from GM only)"; italic = "Mini Bosses". Balork's #008000 applies only to the "[+karma]" tag.

| Monster | Source | Lvl | XP +1 | XP +50 | XP +100 | Spells/Abilities | Min Dmg | Max Dmg | HP | row markup (font colour / italic) |
|---|---|---|---|---|---|---|---|---|---|---|
| Brown Rat | t4cbible 2002-02-02 | 1 | 45 | 42 | 42 | - | 4 | 5 | 27 |  |
| Brown Rat | t4cbible live 2026 | 1 | 45 | 42 | 42 | - | 2 | 5 | 27 |  |
| Brown Rat | t4cfantasy live 2026 | 1 | 45 | 42 | 42 | - | 2 | 5 | 27 | n/a |
| Large Rat | t4cbible 2002-02-02 | 2 | 74 | 69 | 68 | - | 3 | 7 | 41 | #FF0000 |
| Large Rat | t4cbible live 2026 | 2 | 74 | 69 | 68 | - | 3 | 7 | 41 | #FF0000 |
| Large Rat | t4cfantasy live 2026 | 2 | 74 | 69 | 68 | - | 3 | 7 | 41 | n/a |
| Bat | t4cbible 2002-02-02 | 1 | 42 | 37 | 32 | - | 4 | 5 | 27 |  |
| Bat | t4cbible live 2026 | 1 | 42 | 37 | 32 | - | 2 | 5 | 27 |  |
| Bat | t4cfantasy live 2026 | 1 | 42 | 37 | 32 | - | 2 | 5 | 27 | n/a |
| Dungeon Bat | t4cbible 2002-02-02 | 2 | 75 | 69 | 68 | - | 3 | 7 | 41 |  |
| Dungeon Bat | t4cbible live 2026 | 2 | 75 | 69 | 68 | - | 3 | 7 | 41 |  |
| Dungeon Bat | t4cfantasy live 2026 | 2 | 75 | 69 | 68 | - | 3 | 7 | 41 | n/a |
| Giant Bat | t4cbible 2002-02-02 | 3 | 107 | 94 | 93 | - | 4 | 8 | 55 |  |
| Giant Bat | t4cbible live 2026 | 3 | 107 | 94 | 93 | - | 4 | 8 | 55 |  |
| Giant Bat | t4cfantasy live 2026 | 3 | 107 | 94 | 93 | - | 4 | 8 | 55 | n/a |
| Undead Bat | t4cbible 2002-02-02 | 3 | 94 | 93 | 93 | - | 4 | 7 | 55 |  |
| Undead Bat | t4cbible live 2026 | 3 | 94 | 93 | 93 | - | 4 | 8 | 55 |  |
| Undead Bat | t4cfantasy live 2026 | 3 | 94 | 93 | 93 | - | 4 | 8 | 55 | n/a |
| Giant Spider | t4cbible 2002-02-02 | 4 | 146 | 124 | 122 | - | 4 | 10 | 69 |  |
| Giant Spider | t4cbible live 2026 | 4 | 146 | 124 | 122 | - | 4 | 10 | 69 |  |
| Giant Spider | t4cfantasy live 2026 | 4 | 146 | 124 | 122 | - | 4 | 10 | 69 | n/a |
| Green Slime | t4cbible 2002-02-02 | 2 | 74 | 69 | 68 | - | 4 | 7 | 41 |  |
| Green Slime | t4cbible live 2026 | 2 | 74 | 69 | 68 | - | 3 | 7 | 41 |  |
| Green Slime | t4cfantasy live 2026 | 2 | 74 | 69 | 68 | - | 3 | 7 | 41 | n/a |
| Goblin | t4cbible 2002-02-02 | 5 | 192 | 161 | 157 | - | 5 | 12 | 84 |  |
| Goblin | t4cbible live 2026 | 5 | 192 | 161 | 157 | - | 5 | 12 | 84 |  |
| Goblin | t4cfantasy live 2026 | 5 | 192 | 161 | 157 | - | 5 | 12 | 84 | n/a |
| Goblin Scout | t4cbible 2002-02-02 | 8 | 379 | 292 | 281 | - | 8 | 18 | 132 |  |
| Goblin Scout | t4cbible live 2026 | 8 | 379 | 292 | 281 | - | 7 | 17 | 132 |  |
| Goblin Scout | t4cfantasy live 2026 | 8 | 379 | 292 | 281 | - | 7 | 17 | 132 | n/a |
| Goblin Warrior | t4cbible 2002-02-02 | 12 | 706 | 527 | 499 | - | 10 | 23 | 199 |  |
| Goblin Warrior | t4cbible live 2026 | 12 | 706 | 527 | 499 | - | 10 | 23 | 199 |  |
| Goblin Warrior | t4cfantasy live 2026 | 12 | 706 | 527 | 499 | - | 10 | 23 | 199 | n/a |
| Goblin Subchief | t4cbible 2002-02-02 | 15 | 1012 | 776 | 725 | - | 14 | 29 | 254 |  |
| Goblin Subchief | t4cbible live 2026 | 15 | 1012 | 776 | 725 | - | 13 | 29 | 254 |  |
| Goblin Subchief | t4cfantasy live 2026 | 15 | 1012 | 776 | 725 | - | 13 | 29 | 254 | n/a |
| Goblin Bomberman | t4cbible 2002-02-02 | 15 | 1012 | 776 | 725 | Kamikaze Bomb | 13 | 29 | 254 | #FF0000 |
| Goblin Bomberman | t4cbible live 2026 | 15 | 1012 | 776 | 725 | Kamikaze Bomb | 13 | 29 | 254 | #FF0000 |
| Goblin Bomberman | t4cfantasy live 2026 | 15 | 1012 | 776 | 725 | Kamikaze Bomb | 13 | 29 | 254 | n/a |
| Goblin Chieftain | t4cbible 2002-02-02 | 18 | 1360 | 1070 | 988 | - | 15 | 35 | 313 |  |
| Goblin Chieftain | t4cbible live 2026 | 18 | 1360 | 1070 | 988 | - | 15 | 35 | 313 |  |
| Goblin Chieftain | t4cfantasy live 2026 | 18 | 1360 | 1070 | 988 | - | 15 | 35 | 313 | n/a |
| Goblin Warchief | t4cbible 2002-02-02 | 19 | 1484 | 1176 | 1082 | - | 16 | 34 | 334 |  |
| Goblin Warchief | t4cbible live 2026 | 19 | 1484 | 1176 | 1082 | - | 16 | 37 | 334 |  |
| Goblin Warchief | t4cfantasy live 2026 | 19 | 1484 | 1176 | 1082 | - | 16 | 37 | 334 | n/a |
| Goblin Warlord | t4cbible 2002-02-02 | 20 | 1630 | 1304 | 1195 | - | 17 | 36 | 353 |  |
| Goblin Warlord | t4cbible live 2026 | 20 | 1630 | 1304 | 1195 | - | 17 | 39 | 353 |  |
| Goblin Warlord | t4cfantasy live 2026 | 20 | 1630 | 1304 | 1195 | - | 17 | 39 | 353 | n/a |
| Atrocity | t4cbible 2002-02-02 | 5 | 231 | 161 | 157 | - | 5 | 12 | 84 |  |
| Atrocity | t4cbible live 2026 | 5 | 231 | 161 | 157 | - | 5 | 12 | 84 |  |
| Atrocity | t4cfantasy live 2026 | 5 | 231 | 161 | 157 | - | 5 | 12 | 84 | n/a |
| Balork [+karma] | t4cbible 2002-02-02 | 15 | 2025 | 1553 | 1452 | - | 13 | 29 | 508 | #008000;ITALIC |
| Balork [+karma] | t4cbible live 2026 | 15 | 2025 | 1553 | 1452 | - | 13 | 29 | 508 | #008000;ITALIC |
| Balork [+karma] | t4cfantasy live 2026 | 15 | 2025 | 1553 | 1452 | - | 13 | 29 | 508 | n/a |
| Skeleton | t4cbible 2002-02-02 | 4 | 139 | 124 | 122 | - | 4 | 10 | 69 |  |
| Skeleton | t4cbible live 2026 | 4 | 139 | 124 | 122 | - | 4 | 10 | 69 |  |
| Skeleton | t4cfantasy live 2026 | 4 | 139 | 124 | 122 | - | 4 | 10 | 69 | n/a |

Differences between the 2002 and live t4cbible rows (INTERPRETATION, from the table above): Brown Rat and Bat Min Dmg 4 vs 2; Green Slime Min Dmg 4 vs 3; Undead Bat Max Dmg 7 vs 8; Goblin Scout 8-18 vs 7-17; Goblin Subchief Min 14 vs 13; Goblin Warchief Max 34 vs 37; Goblin Warlord Max 36 vs 39. Lvl, XP and HP are unchanged for this set. t4cfantasy matches live t4cbible.

### 8.2 Loot and gold

Sources:
- 2001 = https://web.archive.org/web/20011014151611/http://www.t4cbible.com:80/drops.html (snapshot 2001-10-14). It has no gold column.
- Live = https://www.t4cbible.com/Monster1 (2026-10-07). Legend, VERBATIM: "(dr): Demi rare drop (r): Rare drop (vr): Very rare drop".

| Monster | 2001-10-14 snapshot: Item(s) Dropped [| boss location] | live 2026: Item(s) Dropped | live: Gold | live boss table: Location | Respawn |
|---|---|---|---|---|---|
| Brown Rat | Torch | Torch | 1-5 | | |
| Bat | Torch, Light Heal | Torch, Light Heal | 1-5 | | |
| Dungeon Bat | - | - | 3-11 | | |
| Giant Bat | - | - | 5-16 | | |
| Undead Bat | Decaying Bat Wings | Decaying Bat Wings | 5-16 | | |
| Giant Spider | Torch, Light Heal | Torch, Light Heal | 7-22 | | |
| Green Slime | - | - | 3-11 | | |
| Goblin | Goblin Leather Armor, Light Heal, Iron Ring | Goblin Leather Armor (dr), Light Heal, Iron Ring, Heal Pot | 8-27 | | |
| Goblin Scout | Light Heal, Leather Belt, Goblin Leather Armor | Light Heal, Leather Belt, Goblin Leather Armor (dr), Ring of Light, Heal Pot | 14-44 | | |
| Goblin Warrior | Goblin Blade (dr), Iron Key, Light Heal | Goblin Blade (r), Iron Key (dr), Light Heal | 21-66 | | |
| Goblin Subchief | Ringmail, Light Heal, Mana Pot, Goblin Blood (dr), Leather Belt | Ringmail Armor (dr), Light Heal, Mana Pot, Goblin Blood (r), Leather Belt | 26-82 | | |
| Goblin Chieftain | Feather, Goblin Blood (dr) | Feather, Goblin Blood (dr), Iron Key, Ringmail Armor/Leggings/Helmet (r) | 32-99 | | |
| Goblin Warchief | Feather, Iron Key, Bracelet of Power (vr), Golden Ring (r), Ringmail Leggings | Feather, Iron Key, Bracelet of Power (vr), Golden Ring (r), Ringmail Armor/Helmet/Leggings (dr), Goblin Blood (r) | 36-104 | | |
| Goblin Warlord | Sword of Fury (r), Goblin Blood (dr), Ringmail Leggings/Armor | Sword of Fury, Goblin Blood (r), Ringmail Leggings/Armor/Helmet (r) | 35-110 | | |
| Atrocity | Light Heal | Light Heal, Iron Key | 8-27 | | |
| Skeleton | Heal Pot, Torch, Leather Gloves/Belt | Heal Pot, Light Heal, Ring of Confidence (dr), Leather Gloves, Skeleton Bone (vr) | 7-22 | | |
| Balork | Flowing Black Robe (dr), Light Heal; boss-table location: AR LH Temple Dungeon Level 4 | Flowing Black Robe (dr), Light Heal | (blank) | AR LH Temple Dungeon Level 4 | 15:00 |

### 8.3 Stats of the basement-relevant drop items (live Items / Weapon / Armor pages, 2026-10-07)

VERBATIM rows:
- Weapon header "Item Name | Str | End | Agi | int | Wis | Att | Bonuses | Damage | Enc | Buy | Sell":
  - "Rusted Long Sword | 24 | 0 | 0 | 0 | 0 | 0 | - | 6-11 | 7 | 605 | 201"
  - "Goblin Blade | 53 | 0 | 0 | 0 | 0 | 0 | - | 22-31 | 6 | Drop | 1755"
  - "Rusted Dirk | 0 | 0 | 0 | 0 | 0 | 0 | - | 1-4 | 3 | 29 | 9"
- Armor header "Item Name | End | Str | Agi | Int | Wis | Ac | Dodge | Buy | Sell | Enc | Special Bonuses | Full Set":
  - "Leather Belt | 25 | 0 | 0 | 0 | 0 | 0,3 | 0 | 212 | 70 | 2 | -"
  - "Leather Gloves | 25 | 0 | 0 | 0 | 0 | 0,405 | -1 | 248 | 83 | 3 | -"
  - "Leather Armor | 25 | 0 | 0 | 0 | 0 | 1,45 | -4 | 607 | 202 | 8 | - | +3,4 AC -8 Dodge 24 Enc"
- Items (where to find):
  - "Flowing Black Robe | Arakas, dropped by Balork"
  - "Goblin Leather Armor | Arakas, dropped by Goblin, Goblin Scout & Ruk the Miner"
  - "Iron Key | Arakas, dropped by Atrocity, Brigand, Orc Warrior, Raider, Goblin Chieftain/Warchief/Warrior, Skeleton Guardian & Tomb Raider"
  - "Decaying Bat Wings | Arakas, dropped by Undead Bat"
  - "Torch | Buy from all Pot shops (12 GP), dropped by various monsters and found in chests in all islands"

---

## 9. Not found or unavailable

- **Exact pages unavailable on Wayback:**
  - t4cnostalgia /bible/char-rolling (never archived; recovered live from the JS bundle)
  - jeuxonline fils/64035 (never archived; recovered live)
  - 4genet pbworks page 968685 (never archived; recovered live)
  - t4cfantasy `Monsters.php` (never existed in CDX; live 404). The `Monster.php` variant is fine.
- **Not found anywhere fetched:**
  - starting gold
  - itemised starting kit
  - exact classic starting HP or mana (only the jeuxonline "environ 70 pv" estimate and the Nostalgia formula)
  - a numeric hit/dodge formula (only the pbworks "2 attack : 1 dodge → 70-75%" claim)
  - a classic strength→damage formula (only the pbworks claim)
  - base HP/MP regeneration rates
  - the meaning of the monster "XP +1/+50/+100" columns
  - a confirmed list of which monsters spawn in the LH temple basement (only Balork's "LH Temple Dungeon Level 4" is stated)
- **Not fetched (budget), but present in CDX:** `/index.php?page=playerxp` (2006), `/index.php?page=elementalweaks`, `/gmonsters.html`, `/monster2.html`, `/old/oldbible/Skills.html` and `Spells.html` (2006 captures; possibly the oldest bible), `/itemeffect.html`, `/weapons.html`, `/armor.html`.

## 10. Local copies (scratchpad, untrusted downloads)

All raw HTML and PDF files and their text conversions are under `/tmp/claude-1000/-home-brewerm--herdr-projects-unreal-t4c/d9ef0a69-79c3-440e-bda3-557c9c0ac858/scratchpad/wb/`:
- `live/`: live t4cbible pages
- `wb/`: Wayback captures, including `exp.pdf` / `exp.txt` and `chapNN.htm`
- `other/`: nostalgia HTML and `nostalgia_main.js`, `jol.html`, `pb.html`, `fantasy_monster.html`
- `cdx_bible2.txt`: CDX listing


## 11. R-03 append — Wave 4 lookup, 2026-10-09

Fresh successful live web reads: https://www.t4cbible.com/monster (lowercase), https://www.t4cbible.com/monster1 (lowercase), https://www.t4cbible.com/Spells, /skills, /traders, /Potions, /spelldesciprt, /Items, /Weapon, /Armor, /ArakasQuest and /npc. These are cached web-tool responses accessed2026-10-09; version unstated. Complete value/status/source/Prototype coverage is in [w4-bible-lookup.md](w4-bible-lookup.md). No Source files edited and no runtime tests performed.

Key new facts: Armor cloth pants/vest all requirements0; Light illuminates caster vicinity600s/10 MP; Heal Light restores self/friendly target2 MP but amount not specified. ArakasQuest explicitly puts Undead Bats on B2, confirms Uranos Skull Dagger+2500 XP, and describes Nevanis healing without numeric amount/cost/restrictions. Neither wing count nor odds is stated. Dark Fang's level<6 healing is a separate service. Numeric grants for starting HP/MP/gold/skills/kit remain unrecovered. Monster HP/XP/melee/live gold/loot match §8; classic conflicts are retained. Attack/Dodge/Archery lack the live100 cap. Balork respawn15:00 remains confirmed; owner-selected recurring respawn supersedes old campaign permanent defeat, completion single-claim stays separate.

Classic C1/C2/C3/C5/C6/C7/C8/B1/B2/various.html and id_ retry for C1/C2/C3 could not be accessed via web. Direct shell HTTP to Bible was rejected by sandbox domain allowlist. Thus §§2–8 classic quotations remain inherited2026-10-07 evidence, not new retrievals. No PDF/binary/image downloaded. Missing denotes search/access limitations, not an exhaustive absence claim. Domain-scoped queries for starter grants, death pools/penalties, training units, Dungeon Bat floors, wing count/chance, interaction distance and cleanup did not resolve these classic gaps.

A scoped search surfaced https://www.t4cbible.com/forum/abomination/247-abo-info (post2019-10-20, accessed2026-10-09): Abomination ground cleanup15min. Separate server-specific evidence only; not a classic corpse duration. Saga Palid Bat and Realmud wing ingredients were excluded as separate content. Proposed missing-field values and their reasoning are labelled Prototype in the lookup; none are verified in play or silently promoted to Bible facts.
