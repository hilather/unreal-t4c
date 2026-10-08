# Linux build and G0 preparation

Linux first on brewtop; Windows packaging and launch are deferred by the owner.
Both Linux targets built in G0-L2. The coordinator reports all seven
`Lighthaven.Rules.*` tests passed on the host (`result host-check`); this worker
has not independently run them. See `docs/implementation/g0-linux-report.md`.
G1 startup is `/Game/Lighthaven/Maps/Dev_Combat`. The default explicit cook set
is Dev_Combat plus Dev_Movement; regenerate both maps before packaging.

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

## Linux G0 checklist

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
3. Run editor, tests, packaging and launch on the host as a normal user.
   Before launching the editor, fail fast if UID is zero:
   ```bash
   if [[ "$(id -u)" == 0 ]]; then
       echo "Unreal refuses to run as root; run on the host as a normal user" >&2
       exit 1
   fi
   ```
   `run-tests.sh` also checks this before invoking the editor.
   Open the real editor, observe the project/map and close it:
   ```bash
   source build/lh-env.sh
   "$UE_ROOT/Engine/Binaries/Linux/UnrealEditor" "$LH_PROJECT"
   ```
4. Run the native Automation tests:
   ```bash
   bash build/run-tests.sh
   bash build/run-tests.sh Lighthaven.Rules
   ```
5. Package both G1 maps (default), or supply an explicit replacement map list:
   ```bash
   bash build/package-linux.sh
   bash build/package-linux.sh /Game/Lighthaven/Maps/Dev_Combat /Game/Lighthaven/Maps/Dev_Movement
   bash build/package-linux.sh /Engine/Maps/Entry
   ```
   Each argument must start with `/Game/` or `/Engine/`, without `.umap`,
   whitespace or `+`. The script joins validated arguments into UAT `-map=A+B`;
   this explicit list replaces MapsToCook. Startup settings are separate.
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
- `package-linux.sh [explicit-map ...]`: invokes Linux Development BuildCookRun with
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
worker has no user config directory. UBA's separate cache defaults to project-local `Saved/UBA` via `UBA_ROOT`.
Explicit caller values for both variables remain respected. Both defaults are
set by `lh-env.sh`, also sourced by the packaging script.
