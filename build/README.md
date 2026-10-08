# Linux build and G0 preparation

Linux first on brewtop; Windows packaging and launch are deferred by the owner.
The G0-L editor build reached successful UHT on UE 5.8.3, then stopped at a
sandbox-denied UBA cache write before C++ compilation. G0 remains blocked; see
`docs/implementation/g0-linux-report.md`. Source contracts remain drafts; native
Automation fixtures exist but have not run, and there are no authored maps.

## Setup

Extract the owner-supplied UE 5.8.3 Linux binary archive to persistent storage.
No script installs or downloads an engine, SDK or system package.

From the repository root:

```bash
cp build/local.env.example build/local.env
# Edit build/local.env: UE_ROOT="/absolute/path/to/extracted/UE_5.8.3"
bash build/lh-env.sh
```

An exported, nonempty `UE_ROOT` overrides local.env. local.env is trusted Bash
configuration, git-ignored, and sourced only when that environment value is
absent or empty. Scripts resolve project paths relative to themselves, so they
can run from another working directory. Python 3 is needed only for version and
Automation JSON parsing, not gameplay. Version mismatch prints a warning;
record and use the 5.8.3 pin for G0.

The engine must include its native Linux SDK/toolchain, Build.sh, RunUAT.sh,
UnrealEditor and executable UnrealEditor-Cmd. A working graphics device/Vulkan
driver is additionally required for graphical editor and packaged launch checks.
Host clang alone does not establish readiness. See Toolchain.md for the earlier
audit; its Windows-primary recommendation predates the Linux-first decision.

## Linux G0 checklist (not yet run)

1. Use a clean checkout with all required LFS objects. Record revision, engine
   Build.version, archive checksum, SDK/compiler and host details. Review and
   freeze the contracts only after real UHT/compiler validation.
2. Generate project files if needed by the IDE, then build:
   ```bash
   source build/lh-env.sh
   bash "$UE_ROOT/Engine/Build/BatchFiles/Linux/GenerateProjectFiles.sh" "-project=$LH_PROJECT" -game
   bash build/build-linux.sh --game
   ```
   Availability of GenerateProjectFiles.sh depends on the binary distribution;
   IDE generation is optional for Build.sh and is not a build gate by itself.
3. Open the real editor, observe the project/map and close it:
   ```bash
   source build/lh-env.sh
   "$UE_ROOT/Engine/Binaries/Linux/UnrealEditor" "$LH_PROJECT"
   ```
4. After native tests are authored, run them:
   ```bash
   bash build/run-tests.sh
   bash build/run-tests.sh Lighthaven.Rules
   ```
5. After the integrator authors a map and configures startup/cook settings,
   package it with its actual long package name:
   ```bash
   bash build/package-linux.sh /Game/Lighthaven/Maps/Dev_Blank
   ```
   This map name is illustrative; it does not exist in the current checkout.
6. Locate the game launcher in the printed archive directory, launch it from
   outside the editor on Linux, observe the intended map and a clean exit.
   Capture logs, exact executable path/hash and observed behavior. Cooking an
   explicit map does not configure the startup map. Do not assume a package
   launch succeeds merely because UAT succeeded.

Keep Windows gates recorded as deferred, not passed. Linux results alone do not
prove the original Windows delivery requirements or full gameplay acceptance.

## What the scripts establish

- `lh-env.sh`: requires an editor file and readable Build.version; prints the
  version. This does not prove the editor can execute or the SDK/GPU works.
- `build-linux.sh [--game]`: invokes Linux Development LighthavenEditor, then
  optionally Lighthaven, through the engine Build.sh with an absolute project
  path. Success establishes only UBT compilation, not tests or play.
- `run-tests.sh [filter]`: invokes the command editor with Automation RunTests,
  unattended/null RHI flags, export path, logs and the queue-empty TestExit
  condition. Requires editor exit zero and a fresh index.json containing at
  least one test with every state Success and no failed/unfinished aggregate.
  Missing/malformed reports, skipped tests, unknown states and zero tests fail
  closed. SuccessWithWarnings also fails conservatively for review. The report
  format and command completion behavior need confirmation against UE 5.8.3.
  Headless tests do not establish rendering, controller or world traversal.
- `package-linux.sh /Game/.../Map`: invokes Linux Development BuildCookRun with
  build, cook, stage, pak, package and archive. UAT success establishes its
  reported pipeline outcome; manually launching the archive remains required.

Build logs live under unique ignored build/logs directories; package logs live
under unique Saved/Logs directories. Automation uses a
fresh Saved/Automation.* directory per run, so stale reports cannot pass a new
run. Packages are archived under unique build/output/Linux-* directories.
All are ignored by Git. Pipelines preserve engine command failure status.
Scripts neither manufacture assets nor edit shared source/config/ledgers.

Installed-engine builds default an unset `XDG_CONFIG_HOME` to
`Saved/BuildEnvironment/config`, creating it before UBT starts. This prevents
.NET ApplicationData from resolving relative to a read-only engine when the
worker has no user config directory. This does not redirect UBA's separate
default `~/.epic/UnrealBuildAccelerator` cache; G0-L stopped when that path was
denied by the sandbox. No full build result is established by the config fix.
