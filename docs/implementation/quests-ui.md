# W4-05 quests and gameplay UI

Implementation candidate for schema revision 2, base `9ca6a5c0de9b1c8e0454648f0b906fa2991c7414`. No Core shape changes or binary assets. Native transactions validate a snapshot copy; session integration, persistence, actor resolution and publication belong to W4-06.

| Quest/topic | NPC | Behavior and evidence |
|---|---|---|
| Quest.SamaritanRats / Topic.AcceptRats | NPC.Samaritan | Accept before eligible kills. Count settled player Brown Rat kills in TempleB1–B4; cap at 15. World ledger S4/S5, retrieved 2026-10-07, Classic guide baseline. |
| Topic.TurnInRats | NPC.Samaritan | Require 15, grant 2500 XP through character authority (including growth/debt), completed and rewarded independently persisted. Canonical QuestTurnInRewardId(Run, Quest, TurnIn); repeat is ineligible. Source: https://t4cfantasy.com/Bible/Classic/QuestAR.php (ledger S5, retrieved 2026-10-07, secondary Classic evidence). No fresh web retrieval; no Bible numeric rat row in R-03. |
| Quest.BalorkReturn / Topic.BalorkReturn | NPC.BrotherKiran | Settled player Balork defeat sets ReturnToChurch; explicit church NPC topic changes to Complete. Arrival does nothing. No numeric completion reward; completed survives subsequent boss lives. Authored Stage 1 objective, not historical quest chain. |
| Topic.Church / Topic.Descent | NPC.BrotherKiran | New tutorial text; no offers or rewards. |
| Topic.Heal | NPC.Nevanis | Healing service documented at https://www.t4cbible.com/ArakasQuest and https://www.t4cbible.com/npc, R-03 retrieved 2026-10-09, live version unstated, documentary only. Amount, fee and restrictions missing. **Prototype:** living injured players get full derived HP, zero gold, no MP, no cooldown/level/karma cap; full HP returns NoEffect. Replace after source/balance review. |
| Topic.Services | Sigfried, Fali, Rolph, Ortanalas, JagarKar, Kalastor, Murmuntag, Iraltok, Kilhiam, Moonrock, Uranos, Shovanis, BrotherKiran | Opens owner-supplied offers after accepted interaction; empty list means none available. Native identity roster; UI never invents stock or prices. W4-07/W4-06 must filter availability to actual offers. |

R-03 and world ledger mark numeric NPC/loot distances missing. **Prototype:** NPC inclusive 250 cm plus LOS; corpse 200 cm plus LOS; portal retains existing 250 cm. These are interaction geometry, not T4C parity; replace after reachability review. Actor selection uses stable entity identity, never path-derived command identity. Brown Rat kill count belongs to the quest record and is independent of corpse lifetime. Kill observers are called only within successful SettleKill transactions; do not invoke again for cleanup or replay.

## Integration interfaces

`FLHInteractContext` carries resolved NPC definition/entity, distance, LOS and a pointer to the authoritative character profile. W4-06 resolves actors from the request target and must reject a wrong-area/unavailable NPC before calling ExecuteInteract. Never accept caller-supplied context. Use BeginRequest → ExecuteInteract → authority Import/codec validation → CommitRequest; publish only after settlement. On rejection keep original snapshot. Supply `FLHQuestKillObserver` to the reward settlement observer list. Rat XP uses GrantExperience so growth IDs use the profile's canonical growth helper; the outer wrapper owns transaction sequence and receipts.

Defaulted UI read virtuals:

- `HudState()`: resolved HP/MP and maxima, stable player ID, target name/health, objective, save durability, completion notice, six owner-ordered abilities with eligibility/mana/cooldown feedback and self-target policy.
- `DialogueTopics(Entity)`: IDs, labels, text, eligibility/feedback; adapt LHQuests::Topics and authoritative eligibility, no UI rewards.
- `ServiceOffers(Entity)`: Train/Learn/Buy/Sell, skill/spell/offer ID or owned item ID, authoritative quantity and formatted price/eligibility. No UI includes of AI, Services or Rewards.
- `CorpseContents(Container)`: explicit Gold or Item rows, remaining quantities, item identity unset for Gold.

Defaulted session virtuals `RequestRespawn()` and `SetGameplayPaused(bool)` are intentionally unavailable/no-op until W4-06 implements them. Pause entry also calls the controller's real `SetPause(true)`; close restores `SetPause(false)` and existing context release gating. W4-06 must stop runtime regen, AI, loot and respawn clocks under engine pause, and ensure dead-awaiting-respawn/travel/missing-content are the only IsBlocked reasons. Never freeze movement due to a save in flight. Death UI uses owner bDead, not a widget mutation. Respawn requests no XP or health directly.

Controller attack routes through the presenter's command-handler seam with fresh epoch/request IDs and selected stable enemy entity. RetryCommand captures the original request, including ability or owned consumable and target; a fresh intent creates a new ID. W4-06 supplies per-ability target policy and stable player ID for self actions. No live direct combat call; dev map path retained. Live movement gates only on IsBlocked; existing dev-map action gating remains.

Six slots select by 1–6, or held LT plus South/East/West/North/D-pad left/right. LT is the shoulder trigger selector; release never casts. Attack activates selected ability. 7 / LT+D-pad down uses assigned owned item. K / West opens the focusable six-slot ability panel. `AbilityCatalog()` defaults to HudState().Abilities; override it with every inspectable ability. Spell rows inspect owner feedback, six slot buttons choose the destination, AssignAbility binds a learned/available row locally (replacement is an explicit Assign action). Ability and item assignment are session-local via inventory AssignItem; not persisted (no schema proposal needed). Empty/unlearned slots remain inspectable and authority rejects invalid intent. UI sends UseItem, never a consumable ability prefix. Inventory Use and hotbar item retries preserve original IDs.

Slate screens reuse existing V-03 style and keyboard/gamepad focus controls, with dynamic dialogue/service/loot rows, error copy, inventory Use/AssignItem, death respawn and pause resume. HUD has opaque resource/target/save frames, health/mana bars and six labelled ability slots; unresolved pools render an empty bar with unknown text, never a fabricated percentage. Save state and boss notice are owner-authored and never inferred from command acceptance. This is a source implementation, with visual/runtime readability pending host review.

## Deferred and limits

Bat-wing quest and Gustave are absent. Balork completion is a nonnumeric single completion flag, separate from ordinary boss life rewards/900-second respawn owned by W4-04. Settings backend, persistent custom hotbar bindings, quantity steppers and the elaborate spell details wireframe are not implemented here; loot controls request the owner-supplied whole row quantity. The adapter can supply a chosen bounded quantity, but UI does not calculate price/eligibility. Service catalog is external and was not present at this baseline. Full live behavior requires W4-06 wiring and W4-07 offers. Physical controller, maps, cook/package and play verification belong to the coordinator's host gate.

Acceptance tests cover domain settlement/receipts/reload and presenter contracts. LiveAttackSubmitsUseAbility exercises the shared presenter command seam with a session double, not physical controller input. PauseStopsSimulation uses a real transient world with the owner setting PauserPlayerState; controller/session integration still needs the host test.

Selection range in a live session expands to the maximum resolved range in the six owner-supplied ability rows (W4-03 catalog); target selection never uses melee attack validation for ranged/spell targets. Activation still validates the actual selected ability. Dev SelectionRange remains its existing labelled Prototype fallback.
