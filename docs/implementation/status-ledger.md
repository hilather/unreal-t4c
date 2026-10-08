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
| W0-03 native contracts | rev 1 decisions accepted (S-01); header changes in S-02 | codex-sol | Lighthaven.uproject, Source/, docs/implementation/contracts-v1.md | Schema rev 1 NOT frozen: freeze after first UHT/compile on UE 5.8.3 Linux and the integrator decisions in contracts-v1.md |
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
| G0 on Linux (G0-L, G0-L2, G0-L3 + coordinator host checks) | **passed (Linux)**; Windows deferred | codex-sol + host | Source/, Config/, build/ | Host checks 2026-10-08 on brewtop, engine /home/brewerm/Downloads/unreal (UE 5.8.3 CL 58210709, clang 20.1.8): editor+game build OK; 7/7 Lighthaven.Rules tests pass; `build/package-linux.sh /Engine/Maps/Entry` BUILD SUCCESSFUL; packaged Lighthaven launched outside the editor with -RenderOffscreen on Vulkan SM5 (GTX 1050 Ti Max-Q, NVIDIA 580.178.04), exited 0 after quit. Notes: Unreal refuses root, so editor/test/package run as host checks (workers are uid 0); read-only or rename-less engine mounts break UBT's cache, so use a regular extracted engine. Schema rev 1 freeze still pending (contracts-v1.md decisions) |
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
| **G1** | **awaiting manual play check (Matt)**; automated parts passed | n/a | n/a | Checklist in docs/implementation/w1-integration.md: run, target and attack a dummy with keyboard/mouse and with gamepad in Dev_Combat; Dev_Movement ramp/door/stairs/ceiling. A skeptical review of state ownership / duplicate hit paths is also part of G1 |

Binary asset locks: Content/Lighthaven/Maps/Dev_Movement.umap and Dev_Combat.umap are generated only by build/generate-dev-maps.sh (W1-04 commandlet); never hand-edit.
