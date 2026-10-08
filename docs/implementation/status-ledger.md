# Lighthaven status ledger

Coordinator-owned. Task IDs from `docs/plan/docs/05-agent-waves.md`.
Statuses: planned / active / blocked / reviewed / integrated.

Directory convention (owner decision 2026-10-07): project directories are lowercase
(`docs/implementation/`, `build/`, `reference/`, `artsource/`). Directories Unreal itself
requires keep engine casing (`Source/`, `Content/`, `Config/`, `Plugins/`, `.uproject`).
Where the plan says `Docs/Implementation/`, `Build/`, `Reference/` or `ArtSource/`, use
the lowercase form.

| Task | Status | Profile | Owned paths / binary assets | Notes |
|---|---|---|---|---|
| W0-01 rules + world ledgers | integrated (e0262f7) | codex-sol | docs/implementation/rules-ledger.md, world-ledger.md | Map-only facts marked needs-visual-check for W0-04 |
| W0-02 toolchain check | integrated (d5f10f2) | codex-sol | build/ | Proposes UE 5.8.3; Linux route chosen (see below) |
| W0-03 native contracts | **schema rev 1 frozen** (S-01 decisions, S-02 headers) | codex-sol | Lighthaven.uproject, Source/, docs/implementation/contracts-v1.md | Schema rev 1 NOT frozen: freeze after first UHT/compile on UE 5.8.3 Linux and the integrator decisions in contracts-v1.md |
| W0-04 art/reference register | integrated | codex-astra | docs/implementation/art-register.md, map-reference-notes.md, reference/, artsource/ | All 35 images opened and hashes rechecked; map-only answers for W0-01 are in map-reference-notes.md (reconcile into world-ledger) |
| R-01 rules research follow-up (ran as R-01b) | integrated | codex-sol | docs/implementation/rules-ledger.md, rules-research-notes.md | XP thresholds L1–20 confirmed (Classic guide); partial monster/spell/price evidence; 8 prototype proposals. Creation RNG, hit/damage, growth RNG still unresolved |
| B-01 Linux build/test scripts | integrated (e5f2e81) | codex-sol | .gitignore, build/*.sh, build/README.md | Ready for UE 5.8.3 Linux |
| V-01 graybox layout specs | integrated | codex-astra | docs/implementation/layout/ | Church + B1–B4 build-ready layouts for Wave 3 |
| A-01 creature presentation specs | integrated | codex-astra | docs/implementation/art/creatures/ | 11 roster definitions + Balork encounter companion. Open: no typed PresentationId registry yet (W4-02/integrator); combat lacks commit/cancel animation hooks (only OnImpact) and a recovery duration (W4/W5) |
| A-02 player + environment kit specs | integrated | codex-astra | docs/implementation/art/player/, docs/implementation/art/environment/ | Prep for W5-01/W5-02 |
| V-02 UI wireframes | integrated (captured) | codex-astra | docs/implementation/ui/ | |
| R-02 rules ledger rebased on T4C Bible | integrated | codex-sol | rules-ledger.md, world-ledger.md, ruleset-bible-v1.md | Baseline = classic Vircom-era T4C Bible (2001–2006 snapshots). 88 ledger rows confirmed; conflicts kept disputed |
| W1-02b/W1-02c Bible table models and ruleset | integrated, host-tested | codex-sol | Source/Lighthaven/Rules/, Source/LighthavenTests/Rules/ | 13/13 Lighthaven.Rules tests pass on host (UE 5.8.3) |
| W1-03 GAS combat foundation (W1-03/b/c) | integrated, host-tested | codex-sol | Source/Lighthaven/Abilities/, Framework GameMode/GameState/PlayerState/EnemyCharacter | 19/19 Lighthaven tests pass on host (6 Abilities, 13 Rules). Test worlds: unique name, single init, destroyed per test |
| W1-01 controls and camera (C++) | integrated, host-tested | codex-sol | Framework/LHPlayerController*, LHCharacter*, Source/Lighthaven/Input/, LighthavenTests/Controls/ | Needs InputCore in Lighthaven.Build.cs (integrator), GameMode/Config wiring and combat seam connection: W1-INT |
| W1-04 dev test rooms (map-generator commandlet) | integrated | codex-sol | LighthavenEditor/Commandlets/, LighthavenEditor.Build.cs, build/generate-dev-maps.sh | Dev_Movement/Dev_Combat generated on host (0 errors) and committed via LFS. LighthavenEditor loading phase set to Default so commandlets resolve. Regenerate after W1-INT so maps use LHGameMode |
| G0 on Linux (G0-L, G0-L2, G0-L3 + coordinator host checks) | **passed (Linux), schema rev 1 frozen 2026-10-08**; Windows deferred | codex-sol + host | Source/, Config/, build/ | Host checks 2026-10-08 on brewtop, engine /home/brewerm/Downloads/unreal (UE 5.8.3 CL 58210709, clang 20.1.8): editor+game build OK; 7/7 Lighthaven.Rules tests pass; `build/package-linux.sh /Engine/Maps/Entry` BUILD SUCCESSFUL; packaged Lighthaven launched outside the editor with -RenderOffscreen on Vulkan SM5 (GTX 1050 Ti Max-Q, NVIDIA 580.178.04), exited 0 after quit. Notes: Unreal refuses root, so editor/test/package run as host checks (workers are uid 0); read-only or rename-less engine mounts break UBT's cache, so use a regular extracted engine. Schema rev 1 frozen via S-01/S-02: host 30/30 tests |
| W1-02 rules code (uncompiled until engine) | integrated, NOT compiled | codex-sol | Source/Lighthaven/Rules/, Source/LighthavenTests/Rules/, rules-implementation.md | 7 Automation fixtures (Lighthaven.Rules.*). Integrator requests in rules-implementation.md (Build.cs include path, debt convention, rounding). Production data mostly Unresolved until T4C Bible rebase |
| G0 | blocked | n/a | n/a | Linux: needs UE 5.8.3 Linux install + Epic v26 clang toolchain; git-lfs installed and .gitattributes adopted (f5d2135); UE 5.8.3 Linux zip downloading (owner). Windows part deferred (owner, 2026-10-07) |

Binary asset locks: none (no .uasset/.umap exists). Engine for builds: `UE_ROOT=/home/brewerm/Downloads/unreal`.

Platform decision (owner, 2026-10-07): build, run and test on Linux (brewtop) for now. Windows
packaging/launch checks are deferred to a later time and recorded as deferred, not passed.

Host test status (2026-10-08, main incl. W1-01): editor+game build OK; 22/22 Lighthaven Automation tests pass (13 Rules, 6 Abilities, 3 Controls).

| Task | Status | Profile | Owned paths / binary assets | Notes |
|---|---|---|---|---|
| W1-INT Wave 1 wiring | integrated, host-tested | codex-sol | Framework/, Input/, Lighthaven.Build.cs, Config/, Commandlets, LighthavenTests/Integration/ | GameMode defaults, Enhanced Input config, controls→combat attack path, Dev_Combat dummies + startup map. Host: build OK; 24/24 tests (13 Rules, 6 Abilities, 3 Controls, 2 Integration); dev maps regenerated with LHGameMode (LFS); package Dev_Combat BUILD SUCCESSFUL; packaged game loads Dev_Combat with LHGameMode and exits cleanly |
| G1-SKEPTIC review | integrated | codex-sol | docs/reviews/G1-skeptic.md | 2 major verified-by-inspection defects: dead player can still move; stale impact callback tears down a newer activation |
| G1-FIX + G1-FIX2 | integrated, host-tested | codex-sol | Abilities/, Framework/, LighthavenTests/Abilities+Integration | Both G1 findings fixed (movement gated on live matching avatar; impact cleanup bound to its own activation serial). 4 regressions driven by the real timer. Host: 28/28 Lighthaven tests pass |
| G1 QA (Robo-Ilya for Matt, 2026-10-08) | **G1 not passed** | n/a | /home/brewerm/robo-ilya/qa-t4c/G1-report-2026-10-08.md | 30/30 tests; most KBM/movement pass. FAIL: package cooks one map; held movement resumes after attack without release; input dead after Wayland workspace switch; LOS pillar untestable; minors (no-target log, SecurityToken in DefaultEngine.ini) |
| G1-FIX3 + G1-FIX3b | integrated, host-tested | codex-sol | build/package-linux.sh, Framework/, Input/, Commandlets/, Config/, tests | Host: 32/32 tests; maps regenerated (0 errors, LFS 2e32e23); two-map package OK and Dev_Movement loads; Config unchanged after editor runs. Report: docs/implementation/g1-fix3.md |
| **G1** | **automated fixes verified; awaiting Matt's interactive re-check** of: bug 2 (hold D through attack: no movement until release + re-press), bug 3 (Wayland workspace switch: input returns without a click; compare X11 per g1-fix3.md), bug 4 (LOS pillar positions in Dev_Combat), and the GUI nav build | n/a | n/a | **Matt decision 2026-10-08 06:02 ET:** gamepad checklist items are marked UNTESTED and are **not a G1 blocker**. G1 passes once the G1-FIX3 bugs are fixed and verified. Gamepad coverage carried forward as an open item | Checklist in docs/implementation/w1-integration.md: run, target and attack a dummy with keyboard/mouse and with gamepad in Dev_Combat; Dev_Movement ramp/door/stairs/ceiling. A skeptical review of state ownership / duplicate hit paths is also part of G1 |

Binary asset locks: Content/Lighthaven/Maps/Dev_Movement.umap and Dev_Combat.umap are generated only by build/generate-dev-maps.sh (W1-04 commandlet); never hand-edit.

Host test status (2026-10-08, main incl. S-02): 30/30 Lighthaven Automation tests pass (13 Rules, 8 Abilities, 3 Controls, 4 Integration, 2 Core.Schema).

| Task | Status | Profile | Owned paths / binary assets | Notes |
|---|---|---|---|---|
| W2-01 persistence | integrated, host-tested (W2-01b + W2-01c fix) | 51/51 host tests | codex-sol | Source/Lighthaven/Persistence/, LighthavenTests/Persistence/ | Brief ready (coordinator scratch/wave2). Launch after Matt's G1 play check |
| W2-02 character | integrated, host-tested (W2-02b + W2-02d + W2-02e) | 56/56 host tests | codex-sol | Source/Lighthaven/Character/, LighthavenTests/Character/ | Brief ready |
| W2-03 frontend/character UI | W2-03b + W2-03c integrated: 5 Slate screens on presenters, V-03 style, L_Frontend generated on host (LFS). Host 38/38 tests. Live save/character adapters pending W2-04 | codex-sol | Source/Lighthaven/UI/, LighthavenTests/UI/, frontend map commandlet | Brief ready; L_Frontend generated on host |
| W2-04 Wave 2 integration (W2-04, W2-04b, W2-04c) | integrated, host-tested | codex-sol | Framework/, Config/, Integration tests, UI/Character/Persistence adapters | 65/65 host tests; Linux package (L_Frontend+Dev_Movement+Dev_Combat) OK; packaged game starts in L_Frontend |

Open (Windows phase): Unreal writes project `Build/` (e.g. Build/Linux/FileOpenOrder) while our scripts live in lowercase `build/`; on case-insensitive Windows these are one folder. Decide before Windows work (rename scripts dir, or keep and gitignore engine subpaths).

**Matt decisions 2026-10-08 07:25 ET (relayed by Shepherd):**
1. Push approved. Matt pushed `main` (ee9f9fb plus the LFS maps) to github.com/hilather/unreal-t4c; `origin/main` tracks it. **The repository is public**: never commit secrets, credentials or unlicensed binaries; reference images stay out of the repo (no commercial licence established).
2. Wave 2 starts now, in parallel with Matt's remaining G1 hand checks (bugs 2–4 re-check, GUI nav build). W2-01..W2-03 launch one at a time; W2-04 after W2-01..03 are merged and host-tested. Any G1 hand-check failure comes back as a Wave 1 fix.
3. Art direction for A-01/A-02/V-02 is **not yet approved** (Matt reviewing). No work may treat those specs as approved direction until he signs off.

**Matt decision 2026-10-08 07:28 ET (relayed by Shepherd):** A-01 (creatures), A-02 (player + environment kit) and V-02 (UI wireframes) art direction is **approved as a first pass** ("looks fine as a first start"), open to revision later. The astra visuals lane resumes: one visuals worker at a time, staggered with Wave 2 launches.

| Task | Status | Profile | Owned paths / binary assets | Notes |
|---|---|---|---|---|
| V-03 UI visual style guide | integrated (first pass) | codex-astra | docs/implementation/ui/style/ | Tokens, typography, iconography, controller glyph policy, widget states from approved V-02/A-01/A-02; consumable by W2-03 C++ widgets |

**Incident 2026-10-08 (W2 launch):** W2-01..03 were reserved, then the coordinator wrote project memory (the art-direction record) before their briefs were prepared. The frozen knowledge snapshot went stale and the ticker looped on prepare-brief ("memory changed after knowledge selection"), so the workers never received briefs (usage not_bound). No work was lost. Recovery: cancel-attempt, stop, and relaunch each task in order. **Rule: finish all memory and ledger writes before reserving launches; never write memory between reserve and launch** (farm fix BRIEF-SNAPSHOT-STALE-1). W2-03's relaunch brief includes the V-03 style guide.

| Task | Status | Profile | Owned paths / binary assets | Notes |
|---|---|---|---|---|
| W2-02d character on current main + unity-safe helpers | submitted; host build OK; host tests crash | codex-sol | Character/, 8 .cpp namespace edits | Crash in Lighthaven.Character.UnresolvedAndImportValidation: TArray self-aliasing at LHCharacterTests.cpp:398 (test only). 13 tests passed before the crash |
| W2-02e character test self-aliasing fix | integrated (launch held, then relaunched after the orchestrator reverted the run-as-owner-uid setting; coordinator re-adopted) | codex-sol | LighthavenTests/Character/ | Refusal at profile probe: "native probe did not establish prompt readiness (process_observation_failed) … pidfd observation unavailable". Reported to Shepherd |

| A-03 placeholder presentation specs | integrated (first pass) |  codex-astra | docs/implementation/art/placeholders/ | Code-drawable stand-ins (engine shapes, V-03/A-01 palette, A-01 dimensions) and a presentation-ID table for W3/W4, from approved first-pass direction |

Host test status (2026-10-08, main incl. W2-01/02/03): 56/56 Lighthaven Automation tests pass. Conventions: file-local helpers in file-unique namespaces (unity safety); never pass a TArray element to the same array's Add/Insert.
| W2-01d..f canonical command digest (D04) + reward IDs (D06) | integrated, host-tested | codex-sol | Persistence/ | LHSave::RequestDigest / LHSave::GrowthId with frozen vectors; 59/59 host tests. Lesson: UE 5.8 FString::AppendChar(0) is a no-op; build NUL test input via GetCharArray().Insert |
| A-04 graybox lighting and readability guide | integrated (first pass) | codex-astra | docs/implementation/art/lighting/ | Wave 3 prep: light levels, torch placement, roof/wall occlusion for the elevated camera, landmark lighting per floor |
| V-04 in-world feedback visuals | integrated (first pass) | codex-astra | docs/implementation/ui/feedback/ | Target ring/outline, nameplates and health bars, damage numbers, interaction prompts, loot/death markers, resource warnings; consistent with V-02/V-03/A-03/A-04 |

| **G2** | **FAILED** (Robo-Ilya 2026-10-08, report /home/brewerm/robo-ilya/qa-t4c/G2-report-2026-10-08.md): BLOCKER no gameplay input after frontend travel; launch render hang 7/15; confirmed Quit swallowed during save failure; Continue not disabled when both generations unreadable; minors. Fixes: W2-05 (input/UI), W2-06 (render hang). Re-run items 4, 5, 6, 8 after | n/a | n/a | Checklist: docs/implementation/w2-integration.md "G2 packaged Linux checklist". Automated parts pass (65/65 tests, package, launch). Windows deferred |

**2026-10-08 ~16:20 ET:** main pushed to origin (ee9f9fb..999f424, LFS objects included; repo public). Wave 3 briefs drafted (coordinator scratch/wave3: W3-04 first, then W3-01/02/03 map generators in parallel, then W3-05), **held until G2 passes and Matt gives the go-ahead**. Astra lane idle until G2 screenshots exist for a visual review.
| W2-05 G2 gameplay-input and UI fixes | integrated (68/68 host tests) | codex-sol | UI/, Framework/, Persistence/ (wording), tests | Bug 1 game-only input on gameplay BeginPlay/OnPossess; item 8 Submit guard; item 7 Continue disabled; appearance picker, stale errors, first-save wording |
| W2-06 G2 launch render hang | integrated (68/68 host tests) | codex-sol | Config/, build/launch-soak.sh, docs | GameThread waited 60 s on RenderThread ~9 s after frontend in 7/15 launches (EndDrawingViewport, Vulkan IMMEDIATE, GTX 1050 Ti); G1 18/18 OK. Diff since G1, mitigate, soak on host |
| W2-07 G2 hang investigation + test/soak fixes | integrated (68/68 host tests) | codex-sol | tests, build/launch-soak.sh, Config, docs | Fix GameplayInputHandoff test; soak reap tolerance; investigate environment (SDL video driver, XWayland vs Wayland, NVIDIA Vulkan env, crash reporter) |
| W2-07b GameplayInputHandoff LocalPlayer outer | integrated | codex-sol | LighthavenTests/ | 68/68 host tests |

**Matt decision 2026-10-08 (in chat):** the launch render stall (GameThread waits on RenderThread; ~1 in 2 frontend launches and ~1 in 7 gameplay-map launches on brewtop: GTX 1050 Ti Max-Q, NVIDIA 580.178.04, Hyprland/XWayland, Vulkan) is recorded as a **known environment limitation** and does **not** block G2. Workaround: relaunch. Evidence and A/B plan: docs/implementation/g2-render-hang.md. The coordinator may use the display for soak experiments. Wave 3 proceeds; W3-04 starts while the G2 hand re-check (items 4, 5, 6 in-game compare and 8, build G2c-Linux-88f9265) is pending.
