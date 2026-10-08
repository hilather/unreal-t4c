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
| W0-03 native contracts (draft, not compiled) | integrated as draft | codex-sol | Lighthaven.uproject, Source/, docs/implementation/contracts-v1.md | Schema rev 1 NOT frozen: freeze after first UHT/compile on UE 5.8.3 Linux and the integrator decisions in contracts-v1.md |
| W0-04 art/reference register | integrated | codex-astra | docs/implementation/art-register.md, map-reference-notes.md, reference/, artsource/ | All 35 images opened and hashes rechecked; map-only answers for W0-01 are in map-reference-notes.md (reconcile into world-ledger) |
| R-01 rules research follow-up (ran as R-01b) | integrated | codex-sol | docs/implementation/rules-ledger.md, rules-research-notes.md | XP thresholds L1–20 confirmed (Classic guide); partial monster/spell/price evidence; 8 prototype proposals. Creation RNG, hit/damage, growth RNG still unresolved |
| B-01 Linux build/test scripts | integrated (e5f2e81) | codex-sol | .gitignore, build/*.sh, build/README.md | Ready for UE 5.8.3 Linux |
| V-01 graybox layout specs | integrated | codex-astra | docs/implementation/layout/ | Church + B1–B4 build-ready layouts for Wave 3 |
| V-02 UI wireframes | integrated (captured) | codex-astra | docs/implementation/ui/ | |
| R-02 rules ledger rebased on T4C Bible | integrated | codex-sol | rules-ledger.md, world-ledger.md, ruleset-bible-v1.md | Baseline = classic Vircom-era T4C Bible (2001–2006 snapshots). 88 ledger rows confirmed; conflicts kept disputed |
| W1-02b Bible data adapter (band/table models) | planned | codex-sol | Source/Lighthaven/Rules/ | After G0 compiles: load ruleset-bible-v1.md tables (growth bands, XP table) |
| G0 on Linux (G0-L, G0-L2, G0-L3 + coordinator host checks) | partially passed | codex-sol + host | Source/, Config/, build/ | Passed: engine/toolchain, editor+game build (sandbox and host), 7/7 Lighthaven.Rules Automation tests (host, normal user). Blocked: cook/package writes inside the read-only engine mount (Engine/Programs/AutomationTool/Saved); needs a writable engine. Not run: packaged launch. Workers run as uid 0 and Unreal refuses root, so editor/test/package steps run as coordinator host checks. Windows deferred |
| W1-02 rules code (uncompiled until engine) | integrated, NOT compiled | codex-sol | Source/Lighthaven/Rules/, Source/LighthavenTests/Rules/, rules-implementation.md | 7 Automation fixtures (Lighthaven.Rules.*). Integrator requests in rules-implementation.md (Build.cs include path, debt convention, rounding). Production data mostly Unresolved until T4C Bible rebase |
| G0 | blocked | n/a | n/a | Linux: needs UE 5.8.3 Linux install + Epic v26 clang toolchain; git-lfs installed and .gitattributes adopted (f5d2135); UE 5.8.3 Linux zip downloading (owner). Windows part deferred (owner, 2026-10-07) |

Binary asset locks: none (no .uasset/.umap exists).

Platform decision (owner, 2026-10-07): build, run and test on Linux (brewtop) for now. Windows
packaging/launch checks are deferred to a later time and recorded as deferred, not passed.
