# G0-L2 Linux evidence and stopped checkpoint

Task ID: G0-L2. Attempt: `attempt-2b982fcbff457f76fee2954bac8e431adabcbb26eaa14f4943fd471bffa68ba7`.
Base revision: `20982e3ca834601cd3fe40e2867faf72c6684a90`. Build/source fix revision: `167b4be`; result revision is the submitted commit containing this report (exact SHA in submission receipt and attempt report). Contract revision: 1; schema semantic approval remains with integrator. Evidence candidate, not a declaration of verified gate success or integration.

Owned paths / binary assets: `Lighthaven.uproject`, `Source/`, `Config/`, `Content/Lighthaven/Maps/`, `build/`, this report. Changed `build/lh-env.sh`, `Source/Lighthaven/Lighthaven.Build.cs`, `build/Toolchain.md`, this report. No binary assets created or changed.

Behavior changed: unset/empty `UBA_ROOT` now defaults to project-local `Saved/UBA`, created and exported by lh-env.sh. Explicit caller UBA_ROOT remains respected. Existing project-local XDG config default retained. Added `PublicIncludePaths.Add(ModuleDirectory)` only after clang reported missing `Rules/LHRules.h` in Rules and test code. Rules semantics and assertions unchanged. UBA remained enabled; no NoUBA fallback used. Source-backed mechanics: no new values. Provisional tuning introduced: none.

## Observed G0 items

| Item | Status | Observed evidence |
| --- | --- | --- |
| Engine / SDK sanity | passed | Both Linux editor binaries executable; Build.version 5.8.3 / CL 58210709. Bundled clang 20.1.8 executed; sysroot directories present. |
| LighthavenEditor Linux Development build | passed | Retry compiled Rules, generated code and Tests; linked all three editor modules; UBT `Result: Succeeded`, 34.69 seconds. |
| Lighthaven Linux Development build | passed | Compiled and linked `Binaries/Linux/Lighthaven`; UBT `Result: Succeeded`, 79.81 seconds. Combined build script exit 0. |
| Automation execution | failed | Editor-Cmd launch aborted with exit 134: `Refusing to run with the root privileges.` No fixture ran, no index.json or editor.log created. |
| Blank map / startup-map setup | not run | Stop at first hard runtime blocker; editor cannot initialize under this worker UID. No engine empty map selected. |
| Cook / stage / pak / package / archive | not run | Same root-runtime blocker; no usable editor run established. |
| Packaged launch | not run | No archive produced. Neither real nor null rendering observed; the aborted editor invocation requested null RHI. |
| Windows package / launch | deferred | Linux-first owner decision. |

## Checks actually run

Run window: 2026-10-08 00:08–00:12 UTC (2026-10-07 Toronto). Host: brewtop, Linux 7.2.5-3-omarchy x86_64. UBT: 6 physical / 12 logical cores, 6 parallel processes.

- `ls -l /home/brewerm/mnt/ue5.8.3/Engine/Binaries/Linux/UnrealEditor /home/brewerm/mnt/ue5.8.3/Engine/Binaries/Linux/UnrealEditor-Cmd`: exit 0, executable files.
- `cat /home/brewerm/mnt/ue5.8.3/Engine/Build/Build.version`: exit 0; 5.8.3, CL 58210709, compatible CL 55116800, promoted build, `++UE5+Release-5.8`.
- `/home/brewerm/mnt/ue5.8.3/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v26_clang-20.1.8-rockylinux8/x86_64-unknown-linux-gnu/bin/clang --version`: exit 0; 20.1.8, LLVM revision `87f0227cb60147a26a1eeb4fb06e3b505e9c7261`.
- `ls /home/brewerm/mnt/ue5.8.3/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v26_clang-20.1.8-rockylinux8/x86_64-unknown-linux-gnu`: exit 0; bin, etc, include, lib, lib64, libexec, share, usr. No download needed.
- `uname -a`; `hostname`: exit 0, host above.
- `UE_ROOT=/home/brewerm/mnt/ue5.8.3 bash build/build-linux.sh --game`: first run exit 6, editor compilation failed for missing Rules header. UHT completed in 6.7052836 seconds; UBA executed C++ actions using project-local root; UBT total 79.40 seconds. Game did not run in this first invocation.
- Same build command after include-path fix: exit 0; editor 34.69 seconds, game 79.81 seconds. Full logs under `build/logs/Build-20261008T000958Z-2/{editor,game}.log`. Initial failure log: `build/logs/Build-20261008T000818Z-2/editor.log`.
- `UE_ROOT=/home/brewerm/mnt/ue5.8.3 bash build/run-tests.sh Lighthaven`: exit 134. Console evidence: `Saved/Automation.2n16q2/console.log`. No completed test evidence.
- `id`: exit 0, `uid=0(root) gid=0(root) groups=0(root),65534(nobody)`.
- `bash -n build/lh-env.sh build/build-linux.sh build/run-tests.sh build/package-linux.sh`: exit 0.
- `git diff --check`: exit 0.
- Canonical attempt-brief, attempt-input, receipts: exit 0; receipts `[]` at stopped checkpoint, no observed required updates.

Key evidence:

```text
fatal error: 'Rules/LHRules.h' file not found
[11/14] Link libUnrealEditor-Lighthaven.so
[13/14] Link libUnrealEditor-LighthavenTests.so
Result: Succeeded
Total execution time: 34.69 seconds
[6/7] Link Lighthaven
Result: Succeeded
Total execution time: 79.81 seconds
Refusing to run with the root privileges.
libc++abi: __cxa_guard_acquire detected recursive initialization
```

Both successful builds also emitted `Some action result store tasks did not succeed`. This did not prevent compile/link success; cache-store reliability has not been established. Full logs and final UBT trace remain ignored; copies accompany attempt output `library/`. No generated files committed.

## Tests per fixture

Each is **not run** because engine startup aborted before Automation initialization. Test invocation overall is failed, not a rules failure. Prefix `Lighthaven.Rules.`:

- `Prototype.Synthetic.CreationRNGAndPointConservation`
- `Prototype.Synthetic.DerivedStatsEquipUnequipSymmetry`
- `Prototype.Synthetic.LevelEntitlementGrowthAndDebt`
- `Prototype.Synthetic.EquipmentSpellAndQuiverRequirements`
- `Prototype.Synthetic.HitDamageBoundariesAndRounding`
- `Prototype.Ledger.ManaFractionalCarry`
- `Prototype.Ledger.UnresolvedParametersReject`

## Checks not run, defects and next task

Concrete blocker: worker executes as UID 0; supplied UnrealEditor-Cmd refuses root privileges before runtime logs or Automation results. No further runtime, map, packaging or launch attempts made after the first hard blocker. No requested write outside worktree arose after the approved UBA configuration. DDC, shader-worker and crash-report cache behavior remain unmeasured because startup did not proceed. HOME and engine mount unchanged; no sudo, permission expansion, downloads, installs, assertion weakening or root-check bypass attempted.

Next task: coordinator supplies a non-root Unreal runtime execution environment with writable project-local generated directories. Re-run Automation, then editor-create blank map (or select engine empty map), startup-map configuration, Development packaging and timed archived launch. Retain full logs and per-fixture outcomes. Coordinator reviews this evidence and updates its owned status ledger; no merge/push performed. Clean-checkout reproducibility and rendering remain untested. Semantic contract decisions from prior report remain pending despite successful compilation.

## How G0 runs on Linux

G0-L3 supplies configuration and host-run scripts. Builds can run in the sandbox
using project-local `UBA_ROOT` and `XDG_CONFIG_HOME` from `build/lh-env.sh`.
The worker sandbox runs as UID 0; Unreal refuses to initialize as root.
The coordinator runs editor, Automation, cook/package and archived launch on
brewtop as a normal user and records that evidence as `result host-check`.
`build/run-tests.sh` now rejects root before launching the editor.

The G0-L3 task brief reports the G0-L2 host game/editor builds succeeded and all
seven `Lighthaven.Rules.*` tests passed. These are coordinator-reported host
results, not new observations by this worker; the historical sandbox failure
and fixture list above remain the G0-L2 worker record.

The committed temporary game/editor startup map is `/Engine/Maps/Entry`, also
listed in packaging MapsToCook. The project name is Lighthaven, with a fixed
project GUID; the packaging script selects Linux with `-platform=Linux`. No gameplay settings or
binary maps are added. Package on the host with:

```bash
bash build/package-linux.sh /Engine/Maps/Entry
```

The script still requires one explicit map, now allowing `/Game/...` and
`/Engine/Maps/...`; it retains both writable environment defaults. Engine-map
cooking and packaged startup have not been observed in G0-L3. The optional
blank-map creation script is omitted: Entry fulfills the temporary map contract
without enabling a plugin or relying on an untested map-creation commandlet.
The coordinator must retain cook/archive/launch logs and observe startup before
reviewing the Linux gate. Windows checks remain deferred.
