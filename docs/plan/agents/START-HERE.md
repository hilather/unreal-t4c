# Coordinator launch brief

You are implementing an Unreal single-player Lighthaven vertical slice, not designing an MMO backend. Read `README.md`, all six numbered docs, the evidence ledgers and contract examples before assigning work. The requested finish is a playable Windows build, not just source code.

## First session

1. Inspect the repository and its actual instructions, installed Unreal/toolchain, current code and user work. Do not assume this package supplies a project; it supplies plans, references and concepts.
2. Create a small status ledger with task IDs from `docs/05-agent-waves.md`. Record planned/active/blocked/reviewed/integrated status and asset ownership.
3. Perform Wave 0: pin engine, verify blank build/package, reconcile the reference baseline, define native contracts and establish Git/LFS.
4. Assign independent tasks to isolated branches/worktrees. Hold shared header/schema changes with the integrator. Lease each binary asset to one writer.
5. Require each agent to return the handoff below. Integrate and play the milestone before opening dependent waves.

## Priorities

Get create → run → fight → save/load → church/B1 working, then complete the four-floor descent and legitimate melee/ranged/magic routes. Keep art prototypes replaceable through presentation IDs. Research missing numbers with explicit provenance. If a formula cannot be verified, use an explicitly declared prototype value rather than silently asserting parity.

Do not introduce Go services, PostgreSQL, accounts, matchmaking, browser streaming or multiplayer transports. Do not duplicate C++ rules in Python. Do not build a huge automated inventory/report system instead of a playable loop. Use native tests for substantive rule and state risks.

## Independent review prompt

Review the integrated revision against the task acceptance criteria and source evidence. Look for concrete failures: unreachable trainers, missing mana recovery, finite-resource softlocks, unequipable starter gear, XP debt awarding points again, source floor confusion, missing enemy variants, double rewards, invalid travel, save corruption, controller traps and binary-asset conflicts.

Report severity, exact reproduction or evidence, affected owner and smallest sufficient fix. Distinguish verified defect from design suggestion. Do not demand unrelated architecture rewrites or new frameworks. Retest the affected gate after fixes.

## Handoff format

```text
Task ID:
Base revision / result revision:
Contract revision:
Owned paths / binary assets:
Behavior changed:
Source-backed mechanics:
Provisional tuning introduced:
Build/editor/cook/package/play checks actually run:
Results and evidence paths:
Checks not run and concrete missing prerequisite:
Known defects or remaining decisions:
Next task and integration notes:
```

If the runtime cannot execute Unreal, complete useful text/code work but do not pass editor/package/play gates. Return precise missing prerequisites rather than fabricated screenshots, `.uasset` placeholders or claimed gameplay results.
