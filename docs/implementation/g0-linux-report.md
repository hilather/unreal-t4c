# G0-L Linux build evidence and stopped checkpoint

Task ID: **G0-L**, attempt `attempt-3eee24a741fc49cf2ccd219f745fe839f3c633b405a4dd4b4928aa5ad5ada074`.

Base revision: `bc92bcacfeafab78aaf9ec2f8199967fcc4e84b6`. Build-config commit: `0acc8689a82b53e4d410a5d56139bc0e032574e2`; result revision is the submitted commit containing this report (receipt and exact result SHA in the attempt output report). Contract revision: **1 draft, not frozen**. Evidence candidate for coordinator review; no gate success or integration is declared.

Owned paths: `Lighthaven.uproject`, `Source/`, `Config/`, `Content/Lighthaven/Maps/`, `build/`, this report. Changed only `build/lh-env.sh`, `build/build-linux.sh`, `build/README.md`, `build/Toolchain.md`, this report. No binary assets created, no schema or rules semantics changed, no include-path change made without compiler evidence.

Behavior changed: unset `XDG_CONFIG_HOME` now defaults to an existing project-local `Saved/BuildEnvironment/config`, avoiding installed UBT's engine-relative ApplicationData writes. Explicit caller XDG configuration is respected. Build logs now reside under ignored `build/logs/`. No gameplay behavior changed; no source-backed mechanics or prototype tuning introduced.

## Observed gate items

| Item | Status | Evidence / missing prerequisite |
| --- | --- | --- |
| Engine and bundled SDK sanity | passed | Editor and command editor exist with executable modes; Build.version 5.8.3 / CL 58210709; bundled x86_64 clang executed and reports 20.1.8; sysroot directories present; UBT selected native SDK and libc++. Not an editor launch. |
| LighthavenEditor Linux Development build | failed | Retry exit 6: UBA cache parent outside allowed write roots is read-only. No completed C++ action/link established. |
| UHT substep | passed | `UHT processed LighthavenEditor in 26.4897962 seconds (12 generated files written)`. This does not freeze contract semantics or pass compilation. |
| Lighthaven Linux Development game build | not run | Stop at editor build hard blocker. |
| Native Automation tests | not run | No compiled editor/test modules; stopped at sandbox blocker. |
| Blank map / startup-map configuration | not run | Stop before editor execution; no map created and no startup map selected. |
| Cook / stage / pak / package / archive | not run | No successful native build or selected startup map; stopped at sandbox blocker. |
| Packaged Linux executable launch | not run | No packaged executable. Rendering was neither real nor null: no launch occurred. |
| Windows package and launch | deferred | Owner's Linux-first decision; no Windows machine supplied. |

## Checks actually run

Run window: 2026-10-08 approximately 00:00–00:03 UTC (2026-10-07 Toronto). Host: brewtop, Linux 7.2.5-3-omarchy x86_64. UBT detected 6 physical and 12 logical cores, selecting up to 6 processes.

- `ls -l /home/brewerm/mnt/ue5.8.3/Engine/Binaries/Linux/UnrealEditor /home/brewerm/mnt/ue5.8.3/Engine/Binaries/Linux/UnrealEditor-Cmd`: exit 0, both executable.
- `cat /home/brewerm/mnt/ue5.8.3/Engine/Build/Build.version`: exit 0; 5.8.3, CL 58210709, compatible CL 55116800, promoted installed build, branch `++UE5+Release-5.8`.
- `/home/brewerm/mnt/ue5.8.3/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v26_clang-20.1.8-rockylinux8/x86_64-unknown-linux-gnu/bin/clang --version`: exit 0; clang 20.1.8, revision `87f0227cb60147a26a1eeb4fb06e3b505e9c7261`.
- `ls /home/brewerm/mnt/ue5.8.3/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v26_clang-20.1.8-rockylinux8/x86_64-unknown-linux-gnu`: exit 0; bin, etc, include, lib, lib64, libexec, share, usr. ToolchainVersion.txt reports `v26_clang-20.1.8-rockylinux8`.
- `UE_ROOT=/home/brewerm/mnt/ue5.8.3 bash build/build-linux.sh`: exit **134**, captured using `set -o pipefail` and `tee build/logs/g0-editor-console.log`. UBT aborted while creating `/home/brewerm/mnt/ue5.8.3/Epic` on the read-only mount, before UHT.
- `mkdir -p Saved/BuildEnvironment/config`: exit 0. `XDG_CONFIG_HOME="$PWD/Saved/BuildEnvironment/config" UE_ROOT=/home/brewerm/mnt/ue5.8.3 bash build/build-linux.sh`: exit **6**, captured with pipefail and `tee build/logs/g0-editor-xdg-console.log`. UBT reported total execution time **81.33 seconds**; UHT took **26.4897962 seconds**. No game target followed.
- `git lfs version`: exit 0, git-lfs/3.8.0. No LFS asset add attempted.
- `bash -n build/lh-env.sh build/build-linux.sh build/run-tests.sh build/package-linux.sh`: exit 0.
- `git diff --check`: exit 0 (repeated after documentation edits).
- Canonical attempt brief/input/receipts read through `herdr-farm`; exit 0. Receipts returned `[]`, including at the stopped checkpoint; no required update pending in the observed responses.

Full logs kept under ignored `build/logs/`: `g0-editor-console.log`, `g0-editor-xdg-console.log`, `g0-ubt.log`, `g0-trace.uba`. Supporting copies are in this attempt's output `library/`. Generated UHT files remain ignored under `Intermediate/`; native UBT log/trace remain under `Saved/BuildEnvironment/config/Epic/UnrealBuildTool/`. No generated output is committed.

Key retry log lines:

```text
UHT processed LighthavenEditor in 26.4897962 seconds (12 generated files written)
Using Clang compiler 20.1.8 (.../v26_clang-20.1.8-rockylinux8/x86_64-unknown-linux-gnu/bin/clang++)
Using bundled libc++ standard C++ library.
Using Unreal Build Accelerator local executor to run 15 action(s)
Unhandled exception: IOException: Read-only file system : '/home/brewerm/.herdr-farm-homes/codex-sol/.epic'
... UnrealBuildTool.UBAExecutor.Init ... UBAExecutor.cs:line 310
Result: Failed (OtherCompilationError)
Total execution time: 81.33 seconds
```

## Tests per fixture

All are **not run**, with the same prerequisite: completed native editor/test-module build. No assertions altered and no rules defect inferred from an unexecuted fixture. Prefix is `Lighthaven.Rules.`:

- `Prototype.Synthetic.CreationRNGAndPointConservation`
- `Prototype.Synthetic.DerivedStatsEquipUnequipSymmetry`
- `Prototype.Synthetic.LevelEntitlementGrowthAndDebt`
- `Prototype.Synthetic.EquipmentSpellAndQuiverRequirements`
- `Prototype.Synthetic.HitDamageBoundariesAndRounding`
- `Prototype.Ledger.ManaFractionalCarry`
- `Prototype.Ledger.UnresolvedParametersReject`

## Blocker, decisions and next task

UBA's default root is `/home/brewerm/.herdr-farm-homes/codex-sol/.epic/UnrealBuildAccelerator`; creation fails at its `.epic` parent. This is outside the worker's allowed write roots. Following the explicit sandbox stop instruction, no further build or alternate UBA cache configuration was attempted, HOME was not changed, and no permissions were expanded. No downloads, installs, sudo, engine changes, or hand-authored binary assets occurred.

Read-only engine-source inspection shows `UBAExecutor.cs` lines 257–282 select configured RootDir, then `UBA_ROOT` / `BOX_ROOT`, then the user-profile default. This is evidence for the next authorized task's cache policy decision, not a tested remedy. Supply a writable approved UBA cache configuration or execution environment, then rerun the editor build and fix genuine compiler errors before the game build, Automation, map, package and launch. The new XDG default was supported by the explicit-env retry; a subsequent build through the committed default has not been run after this hard blocker.

Integrator decisions in contracts-v1.md and rules-implementation.md remain pending, including semantic freeze, XP convention, RNG encoding, adapters and policy provenance. UHT success alone does not resolve them. Coordinator should reconcile its status-ledger's obsolete extraction/engine-absence note with this observed engine availability and new cache blocker; that ledger is outside this worker's scope. Clean-checkout reproducibility, graphics, editor runtime compatibility, package dependencies and archive checksum remain unmeasured.
