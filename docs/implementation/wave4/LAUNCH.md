# Wave 4 launch specs. HOLD until G3 passes (Robo-Ilya report via Shepherd) or Shepherd says go, **and R-03 is merged**

**Stage 1A subset (review before G4):** W4-08 (schema rev 2, UseItem) first, then W4-01 (AI + director, B1 species), W4-02 (rows for Brown Rat/Bat/Green Slime + B1 encounter rows + starter/loot items), W4-03 (melee `Attack.Melee.Basic` only, plus mana regen), W4-04 (kill settlement, loot, respawn, player death, persistence digests), W4-05 (Samaritan errand, Nevanis heal topic, HUD, death screen, pause, live-attack controller path), then **W4-06a** (session wiring, live melee, B1 NPC markers, Stage 1A checklist). W4-07 is not needed for Stage 1A; `01-game-design.md` also lists a "first trainer" for 1A, which needs W4-07 + W4-06b. Ranged/magic, services, B2–B4, Balork and return are W4-06b / G4.

```
H="/home/brewerm/.local/bin/herdr-farm --root /home/brewerm/.herdr-projects"; R=/home/brewerm/git/unreal-t4c; S=/home/brewerm/.herdr-projects/unreal-t4c/scratch/wave4
L=Source/Lighthaven; T=Source/LighthavenTests; C=Source/LighthavenEditor/Commandlets; D=docs/implementation
```
Prompt = `cat brief-W4-0N.md _env.md > full-W4-0N.md`. Profile `codex-sol`, `--role build`, `--work-item W4-0N`. Ledger and memory writes go **before** reserving; nothing is written between reserve and launch. One launch at a time; retry once on profile-preparation or transfer-cleanup refusal, hold and log on a second. On "coordinator: current ownership …", run `runtime unreal-t4c adopt coordinator` first. A cancelled ID relaunches as `<ID>b --supersedes <ID>`.

## Order and parallel groups (write scopes are disjoint inside each group)
| Group | Tasks (parallel) | Base | Starts when |
|---|---|---|---|
| gate | R-03 (Bible lookup; launched 2026-10-09) | main | — |
| 0 | W4-08 alone (owns `Core/` + codec files that W4-04 also extends) | main with R-03 | R-03 merged and G3 passed / go-ahead |
| A | W4-01, W4-03, W4-04 | main with W4-08 | W4-08 merged (W4-04 must start from that main) |
| B | W4-02, W4-05, W4-07 | main with A merged | all of A merged (W4-02 needs W4-01 + W4-04 + W4-03 types; W4-05 needs all three; W4-07 needs W4-03 + W4-04) |
| C1 | W4-06a | main with A + W4-02 + W4-05 | W4-02 and W4-05 merged |
| C2 | W4-06b | main with W4-06a + W4-07 | W4-06a and W4-07 merged |

Group A merge order: **W4-04 → W4-03 → W4-01** (W4-03 keeps the combat API source-compatible; W4-01 compiles against it; re-check after each merge). Group B: any order.

## Launch blocks
**W4-08 schema rev 2 (UseItem)** — `--task W4-08 --write $L/Core/ --write $L/Persistence/LHSaveCodec.h --write $L/Persistence/LHSaveCodec.cpp --write $L/Persistence/LHSaveWireV1.inl --write $L/Persistence/LHSaveWireV2.inl --write $L/Persistence/LHSaveValidation.cpp --write $L/Persistence/LHSaveStore.h --write $L/Persistence/LHSaveStore.cpp --write $L/Framework/LHWave2Session.h --write $L/Character/LHCharacterAuthority.cpp --write $T/UI/LHUIPresenterTests.cpp --write $T/Core/ --write $T/Persistence/LHPersistenceTests.cpp --write $T/Persistence/LHSchemaRev2Tests.cpp --write $T/Persistence/LHSchemaRev1Fixture.inl --write $D/schema-rev2.md --output $D/schema-rev2.md`
Group 0 runs alone, so its codec files (inside W4-04's later `Persistence/` scope) and one-line edits in `LHWave2Session.h` (W4-06), `LHCharacterAuthority.cpp` (W4-07) and `LHUIPresenterTests.cpp` (W4-05) never overlap a running task; later owners start from main with W4-08 merged.

**W4-01 AI** — `--task W4-01 --write $L/AI/ --write $L/Framework/LHEnemyCharacter.h --write $L/Framework/LHEnemyCharacter.cpp --write $L/Lighthaven.Build.cs --write $T/LighthavenTests.Build.cs --write $T/AI/ --write $D/ai.md --output $D/ai.md`

**W4-03 abilities** — `--task W4-03 --write $L/Abilities/ --write $L/Rules/ --write $T/Abilities/ --write $T/Rules/ --write $D/abilities.md --output $D/abilities.md`

**W4-04 rewards/persistence** — `--task W4-04 --write $L/Rewards/ --write $L/Persistence/ --write $T/Rewards/ --write $T/Persistence/ --write $D/rewards-lifecycle.md --output $D/rewards-lifecycle.md`

**W4-02 enemy/encounter/item data** — `--task W4-02 --write $L/Data/ --write $T/Data/ --write $D/enemy-data.md --output $D/enemy-data.md`

**W4-05 quests/UI** — `--task W4-05 --write $L/Quests/ --write $L/UI/ --write $L/Input/ --write $L/Framework/LHPlayerController.h --write $L/Framework/LHPlayerController.cpp --write $T/Quests/ --write $T/UI/ --write $T/Controls/ --write $D/quests-ui.md --output $D/quests-ui.md`

**W4-07 services** — `--task W4-07 --write $L/Services/ --write $L/Character/ --write $T/Services/ --write $T/Character/ --write $D/services.md --output $D/services.md`

**W4-06a / W4-06b integration** — `--task W4-06a` (then `--task W4-06b`, `--work-item W4-06` for both) `--write $L/Framework/LHWave2Session.h --write $L/Framework/LHWave2Session.cpp --write $L/Framework/LHWave2Profile.h --write $L/Framework/LHWave2Profile.cpp --write $L/Framework/LHWave2Closure.h --write $L/Framework/LHWave2Closure.cpp --write $L/Framework/LHSessionSubsystem.h --write $L/Framework/LHSessionSubsystem.cpp --write $L/Framework/LHPlayerState.h --write $L/Framework/LHPlayerState.cpp --write $L/Framework/LHGameMode.h --write $L/Framework/LHGameMode.cpp --write $L/Framework/LHGameState.h --write $L/Framework/LHCharacter.h --write $L/Framework/LHCharacter.cpp --write $L/Framework/LHDevCombatFixture.h --write $L/Framework/LHDevCombatFixture.cpp --write $L/Framework/LHArrivalReview.h --write $L/Framework/Wave4/ --write $L/World/ --write $C/LHGenerateHubMapCommandlet.h --write $C/LHGenerateHubMapCommandlet.cpp --write $C/LHGenerateBasementAMapsCommandlet.h --write $C/LHGenerateBasementAMapsCommandlet.cpp --write $C/LHGenerateBasementBMapsCommandlet.h --write $C/LHGenerateBasementBMapsCommandlet.cpp --write Source/LighthavenEditor/Validation/ --write Config/DefaultEngine.ini --write Config/DefaultGame.ini --write Config/DefaultInput.ini --write $T/Integration/ --write $T/World/ --write $D/w4-integration.md --output $D/w4-integration.md`
Framework files are listed one by one so that no `--write` overlaps W4-01 (`LHEnemyCharacter.*`) or W4-05 (`LHPlayerController.*`). New W4-06 Framework files go under `Framework/Wave4/`. `Config/Lighthaven/ReviewedArrivals.tsv` is deliberately not writable. Matt's Q1 decision (Bible starting character + Bible XP table) is already in `brief-W4-06.md` item 2.

## Coordinator host checks after each submission
Pattern from Wave 3: clone main into `$SCRATCHPAD/int-<task>-runN`, fetch the candidate, `merge --no-ff`, then:
1. `UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game`: both `Result: Succeeded`.
2. `bash build/run-tests.sh Lighthaven`: record `N completed successful` / `Found N`; the new `Lighthaven.<Area>.*` tests from the brief are present and pass; prior tests not deleted.
3. Scope check: `git diff --name-only main...candidate` is inside the task's `--write` list; no `.umap`/`.uasset`/`ReviewedArrivals.tsv`.
4. Per task: **W4-08**: the frozen rev-1 fixture decodes and migrates to rev 2; FutureSchema is now 3; request/reward golden vectors in `LHIdentityTests.cpp` unchanged; all 83 prior tests pass; read `schema-rev2.md` and record acceptance (or amendments) in `status-ledger.md` before Group A. **W4-01** also re-run dev maps (`build/generate-dev-maps.sh`) only if Dev_Combat behaviour changed, and `Lighthaven.Integration.ControlsCombat`. **W4-02**: read the roster coverage matrix and confirm Dungeon Bat is still provisional and there's no parity claim. **W4-04**: check the new digest vectors are frozen (no later edits). **W4-05/W4-07**: spot-check in a dev session that the UI never computes prices or rewards.
5. **W4-06a / W4-06b only:**
   - Regenerate all five maps (`chmod u+w` the lockable maps first): `build/generate-hub-map.sh`, `build/generate-basement-a-maps.sh`, `build/generate-basement-b-maps.sh`.
   - Run `UE_ROOT=… bash build/review-arrivals.sh`: all nine arrivals PASS (geometry changed near B1 NPCs). Commit the refreshed `ReviewedArrivals.tsv` only from passing output, then regenerate again so the markers pick up the reviewed state.
   - `LHValidateWorld` over all five maps: **0 errors** (including new NPC/boss checks and the duplicate-ID scan).
   - Commit maps via LFS (coordinator only).
   - Full Automation again on the regenerated maps (`ArrivalSafety` included).
   - Package the six maps (explicit command in `w3-integration.md`), run packaged `-LHCheckPackagedContent` (exit 0), launch outside the editor.
   - Hand the Stage 1A checklist (06a) or the G4 checklist (06b) from `w4-integration.md` to Shepherd for Robo-Ilya. G4 needs fresh melee, ranged and magic builds through legitimate services.
6. Record each merge, test count and decision in `status-ledger.md` and project memory **before** the next reserve.
