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
| W0-01 rules + world ledgers | active | codex-sol | docs/implementation/rules-ledger.md, world-ledger.md | |
| W0-02 toolchain check | active | codex-sol | build/ | Check and report only; no installs |
| W0-03 native contracts (draft, not compiled) | planned | codex-sol | Source/Lighthaven/Core/ drafts | Waits on W0-01 + W0-02 |
| W0-04 art/reference register | active | codex-astra | docs/implementation/art-register.md, reference/, artsource/ | Images stay at package path until LFS is decided |
| G0 | blocked | n/a | n/a | No Unreal Engine, no git-lfs, no Wine/Windows on brewtop |

Binary asset locks: none (no .uasset/.umap exists).
