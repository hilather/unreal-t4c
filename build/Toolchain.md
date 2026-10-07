# W0-02 — toolchain inspection and proposed pin

Inspection date: 2026-10-07. Base: `5ab4515797f7588968661fdaa0ee34def799f5a0`. Contract revision 1. Scope: `build/` only; no binary assets owned. This is a read-only host audit and proposed configuration, not a compiled project.

**G0: BLOCKED.** No accessible Unreal editor/engine, Epic native SDK, Windows build machine/toolchain, or C++ project is available. No compile, editor, import, Automation test, cook, package, executable launch or play check ran. Schema revision 1 still requires integrator approval. The coordinator must carry this blocked status into the shared ledger; that ledger is outside this worker's ownership.

## Observed host

Exact probe commands, full output and exit statuses are in [host-inspection.txt](host-inspection.txt). Negative results describe this isolated worker's visible filesystem/device namespace, not proof about inaccessible host locations.

| Probe | Output summary |
| --- | --- |
| `hostname`; `cat /etc/os-release`; `uname -a` | brewtop; Omarchy 4.0.4, Arch-like; x86_64 Linux 7.2.5-3-omarchy |
| `lscpu` | Intel i7-8750H, **6 physical cores / 12 logical CPUs**, not 12 physical cores |
| `free -h` | 62 GiB RAM, 44 GiB available at inspection; 124 GiB swap reported |
| `df -hT / /home /opt /tmp`; `lsblk -o NAME,SIZE,FSTYPE,MOUNTPOINTS` | `/`, `/home`, `/opt` share a 952 GiB Btrfs filesystem with 217 GiB available; NVMe device 953.9 GiB. `/tmp` is a private 32 GiB tmpfs, unsuitable for engine installation |
| `lspci -nn` | Intel UHD 630 and NVIDIA GTX 1050 Ti Mobile present on PCI bus; VRAM not measured |
| `nvidia-smi`; `ls -l /dev/dri` | NVIDIA driver communication fails; no `/dev/dri` visible here |
| `vulkaninfo --summary` | Command absent: Vulkan device/API/extension capability **unverified** |
| `ldconfig -p \| rg -i "vulkan\|libGLX_nvidia"`; `ls /usr/share/vulkan/icd.d /etc/vulkan/icd.d` | Vulkan loader, Intel libraries and NVIDIA GLX library present; Intel/NVIDIA ICD JSONs present in `/usr/share`; `/etc` ICD directory absent. Libraries do not establish a working GPU |
| `clang --version`; `gcc --version` | Clang 22.1.8; GCC 16.2.1; neither establishes compatibility with Epic's pinned Linux SDK |
| `dotnet --info` | .NET runtime 10.0.11 present, **no .NET SDK installed** |
| `cmake --version`; `python3 --version` | CMake absent; Python 3.14.7 present. CMake is not itself required for a normal UBT project |
| `git --version`; `git lfs version` | Git 2.55.0; Git LFS absent |
| `command -v UnrealEditor UnrealEditor-Cmd UE4Editor wine wine64 proton steam` | None resolved on PATH |
| `ls -ld /opt /opt/* /home/brewerm/.config/Epic /home/brewerm/.local/share/Steam /home/brewerm/.steam` | No standard Epic/Steam user directories; `/opt` shows Grok Bot, containerd, packages |
| Root-wide `find` (exact expression in transcript), pruning `/proc`, `/sys`, `/dev` | No matching UnrealEditor*, UE_5*, UnrealEngine or Proton paths found in accessible locations; exit 1 with 671 permission errors. An exhaustive absence claim is not warranted |
| `getconf GNU_LIBC_VERSION` (additional command) | glibc 2.44 |

The old mobile GPU and inaccessible graphics devices make this worker unsuitable for claiming graphical readiness. Omarchy is not Epic's recommended/tested Ubuntu/Rocky baseline. CPU/RAM can support development in principle; actual editor compatibility and speed remain unmeasured.

## Engine and compiler recommendation

**Proposed exact engine pin: UE 5.8.3**, latest stable hotfix identified in Epic's public release list on the inspection date. Epic announced [5.8 availability](https://www.unrealengine.com/news/unreal-engine-5-8-is-now-available) and [5.8.3 on September 22](https://forums.unrealengine.com/t/5-8-3-hotfix-released/2833315); the [release list](https://forums.unrealengine.com/tag/released/11396) identifies no newer stable release in the evidence inspected. This is a recommendation, not an installed or compatibility-tested pin. At installation record `Engine/Build/Build.version`, distribution checksum or source tag/commit, compiler and SDK versions; never follow a moving `release` branch as the pin.

The plan's C++, GAS, Enhanced Input, native Automation/Functional Testing and Windows target need no third-party plugin. Compatibility must be proven with the minimal attack/build spike before content grows.

For Linux, use **Epic v26 clang 20.1.8 with its sysroot**, not host clang 22.1.8/GCC. Epic's [version-specific Linux requirements](https://dev.epicgames.com/documentation/en-us/unreal-engine/linux-development-requirements-for-unreal-engine) map 5.7–5.8 to v26, recommend Ubuntu 22.04/Rocky Linux 8, 32 GB RAM and 8 GB VRAM, and list NVIDIA 570+ drivers. Their quickstart still mentions clang 18.1.0; prefer the version matrix and verify against the actual 5.8.3 distribution. This machine's driver version and Vulkan feature support could not be verified.

## Installation routes and cost

All figures below are **provisional capacity/time estimates**, not measured download sizes or Epic guarantees. Obtain exact installer sizes before approval. Nothing was installed or downloaded except public documentation responses.

1. **Linux precompiled editor:** Epic's [Linux quickstart](https://dev.epicgames.com/documentation/en-us/unreal-engine/linux-development-quickstart-for-unreal-engine) provides an account-gated ZIP route. Sign in with an Epic account/EULA, obtain the exact 5.8.3 distribution if offered, extract on a persistent SSD and set up its native toolchain. Budget 30–60 GB transfer and 100–150 GB engine/toolchain/workspace/DDC initially; allow hours for download and first shader compilation. No engine compilation needed. Confirm the account-gated archive actually offers 5.8.3; not checked here. This enables Linux development/testing, not the required Windows acceptance build. Graphics access/driver and distro compatibility must first be resolved outside this worker.
2. **Linux source build:** Link Epic and GitHub accounts and accept Epic's organization invitation, as described in [source access](https://dev.epicgames.com/documentation/unreal-engine/downloading-source-code-in-unreal-engine). Fetch the exact release tag and record its commit; approved `Setup.sh` downloads dependencies/native SDK, then generate/build the editor. Budget 30–80 GB transfer and **250–350 GB free SSD** for source, dependencies, intermediates, symbols and DDC; 3–8+ hours for a first full build on this 6-core laptop, with thermal/load effects unknown. Current 217 GiB free is below this conservative budget. Source enables engine patches but adds maintenance and rebuild cost; not needed for this slice by itself.
3. **Windows machine — recommended primary path:** Supply a named Windows 11 x64 development/reference PC with working GPU, controller and SSD, install Epic Launcher UE 5.8.3 and Visual Studio. Epic's [VS matrix](https://dev.epicgames.com/documentation/en-us/unreal-engine/setting-up-visual-studio-development-environment-for-cplusplus-projects-in-unreal-engine) supports VS 2022 17.14+ or VS 2026 18.0+, recommending VS 2026 for general development, MSVC 14.50 and Windows SDK 10.0.26100+. Proposed initial compiler pin: VS 2026 18.0, MSVC 14.50, SDK 10.0.26100; verify exact installed patch and 5.8.3 UBT SDK acceptance before freezing. Include Game development with C++, Desktop development with C++, profiling tools, AddressSanitizer and relevant .NET components. Use engine-bundled .NET for UBT/UAT where provided. Estimate 40–80 GB engine transfer plus 5–20 GB VS/SDK transfer; reserve 150–200 GB SSD including project/cache/package headroom. Native Windows build and launch provide the shortest path to G0.
4. **Linux → Win64:** Epic's standard [cross-toolchain documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/linux-development-requirements-for-unreal-engine) covers **Windows → Linux**, not a native Linux → Win64 SDK. Do not treat the Linux clang/sysroot as a Windows packaging solution. Epic now publishes [WineResources](https://github.com/EpicGames/WineResources) and an [AutoSDK example](https://github.com/EpicGames/WineResources/blob/main/examples/quickstart/autosdk/README.md) for Windows engine/C++ project builds under Wine. Epic explicitly labels Wine support experimental; it involves patched Wine/containers, Windows compiler/SDK and licensing considerations. This is compatibility-layer execution, not ordinary native cross compilation. It is a possible later spike, not the proposed G0 path: no Wine/Proton here, no exact-version validation, substantial setup and downloads, and it still does not replace a real Windows launch test. No packages or container services were installed.

## Windows build/package procedure (NOT RUN)

After Matt supplies the chosen machine and authorizes installation, the integrator creates a minimal C++ project/blank map and module definitions outside this worker's scope. Use the same engine pin on every machine. A clean checkout must fetch all LFS objects before generating files/building.

Proposed Windows commands (paths illustrative; not executed):

```bat
"C:\Epic\UE_5.8\Engine\Build\BatchFiles\Build.bat" LighthavenEditor Win64 Development -Project="C:\src\unreal-t4c\Lighthaven.uproject" -WaitMutex
"C:\Epic\UE_5.8\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun -project="C:\src\unreal-t4c\Lighthaven.uproject" -noP4 -platform=Win64 -clientconfig=Development -build -cook -stage -pak -package -archive -archivedirectory="C:\artifacts\Lighthaven-G0" -map=/Game/Lighthaven/Maps/Dev_Blank -prereqs -utf8output
```

Explicitly configure default/startup map and cook list; build game and editor targets; run native Automation/Functional checks and validators in the editor target. Use [Epic's packaging workflow](https://dev.epicgames.com/documentation/en-us/unreal-engine/packaging-your-project). Capture compiler/UAT logs and resulting executable/hash; launch the archived executable outside the editor from a clean destination on Windows and observe the blank map/exit. A later Shipping package must be checked separately. Editor-only validation/test modules must not ship. G0 also requires integrator schema freeze; successful source generation alone passes nothing.

No concrete Windows reference machine has been supplied. Proposed performance setup is 1920×1080, Medium preset, 60 FPS target (plan target, not measured); coordinator must name CPU/GPU/VRAM/RAM/OS/driver and approve preset before binding performance acceptance. brewtop is not a Windows reference PC.

## Minimal project and plugin proposal (not created)

```text
Source/Lighthaven/             runtime module; Core, Rules, Framework, Character,
                              Abilities, AI, World, Persistence, UI
Source/LighthavenEditor/       Editor module; Validation (UnrealEd/DataValidation)
Source/LighthavenTests/        editor-only native Automation test module
Source/Lighthaven.Target.cs    Win64 game target, runtime module only
Source/LighthavenEditor.Target.cs  editor target, all three modules
Content/Lighthaven/            Maps, Data, Characters, Environment, Input, UI, Audio, Tests
Config/
art-source/                   approved editable art
reference/                    provenance/reference only, excluded from cooking
build/                        toolchain/release documentation
```

Enable `GameplayAbilities`, `EnhancedInput`, and editor-only `DataValidation`; use `FunctionalTesting` for actual functional test maps when needed. Runtime module dependencies: Core, CoreUObject, Engine, InputCore, EnhancedInput, GameplayAbilities, GameplayTags, GameplayTasks, UMG, Slate/SlateCore as UI needs, AIModule and NavigationSystem. GameplayTags/GameplayTasks are module dependencies, not separate third-party installs. Unreal Insights/native Automation are engine tools. No CommonUI, Lyra, online subsystem, experimental movement or marketplace plugin is required. Verify module descriptor/loading types against installed UE before implementation. C++ rules own formulas and save/reward state; Blueprints own presentation. Shared headers/config/tags remain integrator-owned.

## Git LFS proposal

[Draft attributes](gitattributes.proposed) are deliberately inactive under `build/`. Coordinator should adopt at root only after Git LFS installation on all writers/build machines and confirmation of remote LFS storage/quota and lock API. All `.uasset`/`.umap` are LFS + lockable; selected binary source-art formats under `art-source/` follow the same policy. JSON, C++, INI, Markdown and manifests stay ordinary Git. Generated Binaries/Intermediate/Saved/DDC and packaged output should be ignored by integrator-owned configuration.

Git attributes cannot select by file size ([Git LFS FAQ](https://github.com/git-lfs/git-lfs/blob/main/docs/man/git-lfs-faq.adoc)). Draft extension rules intentionally include small files in those source-art families; add explicit paths for other approved large binary sources. Do not migrate history or import plan images in this task. The reference/concept PNGs remain at `/home/brewerm/Downloads/lighthaven-unreal-design-and-agent-waves/lighthaven-plan/assets/` until coordinator approves the art/LFS route; references are not runtime assets. One writer per binary remains mandatory even if remote locking is unavailable. After authorized setup, verify fresh-clone LFS fetch/checkout/fsck and lock behavior; none ran here.

## What Matt must provide or approve

- Select/approve UE 5.8.3 and native Windows primary route (assumed recommendation, no approval inferred). Provide the Windows machine/reference specs and access; engine + VS/SDK downloads above are multi-GB.
- Provide Epic account/EULA acceptance; for source builds, GitHub account linking and invitation acceptance. No credentials requested or inspected. Confirm exact distribution/tag availability and checksum/commit.
- Approve persistent installation location/capacity: Windows 150–200 GB; Linux binary 100–150 GB; source 250–350 GB estimates. Shared 217 GiB free must not be counted independently for `/home` and `/opt`.
- If Linux editor use is desired, approve required native v26 SDK (estimate 1–3 GB transfer / 3–8 GB installed), GPU device access, Vulkan diagnostic utility and any driver changes. Small utility/package changes still require authorization; no system changes here. Confirm working Vulkan/driver and actual VRAM. Host clang/runtime presence is insufficient.
- Approve Git LFS installation (estimate tens of MB), root attributes, remote storage/quota and binary lease process; all writers must have LFS before adding binary assets. Coordinator adopts root proposal, not this worker.
- Approve actual project/module/plugin creation by W0-03 integrator and schedule real compile/test/cook/package/Windows-launch evidence. No extra third-party plugins currently proposed.

## Evidence and handoff

Task ID: W0-02.
Base revision / result revision: `5ab4515797f7588968661fdaa0ee34def799f5a0` / see attempt report and submission receipt (avoids self-referential commit hash).
Contract revision: 1.
Owned paths / binary assets: `build/`; none.
Behavior changed: documentation/transcript and inactive LFS proposal only.
Source-backed mechanics: none changed; toolchain sources linked inline, consulted 2026-10-07.
Provisional tuning introduced: no gameplay tuning; capacity/build-time estimates and Medium performance preset are proposals.
Build/editor/cook/package/play checks actually run: none. Host commands in `host-inspection.txt`, plus `getconf GNU_LIBC_VERSION`; documentation read and Epic public web lookup. Git scope/whitespace checks are recorded in attempt report.
Results and evidence paths: `build/Toolchain.md`, `build/host-inspection.txt`, `build/gitattributes.proposed`; attempt output `report.md`.
Checks not run and concrete missing prerequisite: C++ build/native Automation requires UE/SDK/project; editor/import/functional/graphics checks require UE/project and working accessible GPU/Vulkan; Win64 cook/package requires Windows engine/MSVC/SDK/project; Windows executable launch/play/controller/performance requires packaged executable and named Windows machine/controller; LFS fetch/fsck/lock requires LFS client and configured remote. G0 remains blocked.
Known defects or remaining decisions: no installed version, partial filesystem visibility, no working GPU proof, missing .NET SDK/CMake/LFS/Wine, Windows reference machine unspecified, Linux archive availability unconfirmed, installation sizes/time estimated. No implementation defects can be assessed without a project.
Next task and integration notes: coordinator records G0 blocked and approves route; W0-03 may draft uncompiled schemas while tooling is unavailable. After approved provisioning, integrator creates minimal project and re-runs full W0-02/G0 acceptance. Root attributes/module/project/ledger changes belong to coordinator.
