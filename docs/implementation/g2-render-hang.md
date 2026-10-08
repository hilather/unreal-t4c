# W2-07: Linux Vulkan launch stalls

## Evidence and conclusion

Presentation remains the leading investigation target, not a proven driver defect. The coordinator reports NVIDIA 580.178.04, GTX 1050 Ti Max-Q, Hyprland/Wayland, Vulkan, windowed 1280×720. Original G2 IMMEDIATE hung on run 1; the 60 FPS cap and then confirmed FIFO each survived run 1 and hung on run 2. A direct Dev_Combat launch survived six runs then stopped at frame 3 immediately after swapchain creation (IMMEDIATE, three images). QA reported 7/15 frontend hangs near frames 375–397 with `EndDrawingViewport`, versus 0/18 G1 gameplay starts. Offscreen G0/G1 launches reportedly never hung. These are supplied observations, not worker reproductions. Neither cap, FIFO, nor removing the frontend fixes the problem; map selection may change frequency. Frame numbers alone are not FPS measurements.

W2-06's idle frontend source review found no per-frame widget rebuild or unconditional repeated save enumeration. No gameplay/UI change is made here. All-thread stacks are still needed to distinguish acquire, present, fence, GPU/driver, and unrelated locks. A missing final log line is circumstantial evidence: logging is sparse/buffered and cannot prove the thread is stuck at the last logged operation.

The inherited `t.MaxFPS=60` is retained as a provisional pacing experiment, **not a hang fix**. No additional project config change is made. Keep the same package and cap throughout each comparison.

## Local engine-source evidence

Inspected 2026-10-08 in `/home/brewerm/Downloads/unreal/Engine/Source/` (installed UE 5.8.3):

- `Runtime/ApplicationCore/Private/Linux/LinuxPlatformApplicationMisc.cpp:322–433`, `InitSDL`: selection priority is `-RenderOffScreen` (dummy), `-sdlvideodriver=`, environment `SDL_VIDEO_DRIVER`/legacy `SDL_VIDEODRIVER`, `[Linux.SDL] VideoDriver`, then SDL autodetection. Line 433 logs `Using SDL video driver '...'`. The engine uses **SDL3**, not SDL2. No bare `-x11` or `-wayland` selector was found in the Linux application path; use the explicit supported option.
- `LinuxPlatformApplicationMisc.cpp:472–502` checks the actual SDL driver for Wayland/X11. `Runtime/VulkanRHI/Private/Linux/VulkanLinuxPlatform.cpp:26` accepts DISPLAY or WAYLAND_DISPLAY; their presence does not identify the selected backend. Line 373 creates the Vulkan surface through `SDL_Vulkan_CreateSurface`. Thus a Wayland desktop does not imply a native Wayland game. Read each run's SDL log: `x11` under Hyprland implies XWayland; `wayland` is native. The prior document's unqualified XWayland assertion is withdrawn pending that log. `hyprctl clients -j` can corroborate the game's `xwayland` property.
- `Runtime/VulkanRHI/Private/VulkanSwapChain.cpp:434–496`: `-vulkanpresentmode=2` selects FIFO if available; the default unlocked path prefers IMMEDIATE. A `-vsync` flag alone does not prove FIFO.
- `VulkanViewport.h:113` fixes `NumRequestedSwapchainImages=3`. `VulkanViewport.cpp:485` passes that count to swapchain creation; `VulkanSwapChain.cpp:518–628` bounds it against surface capabilities and reads the actual returned images. `r.Vulkan.SwapChainIgnoreExtraImages` (line 35, default 0, read-only) restricts use of extra images, **does not request two images**. No configurable requested image-count cvar was found in this path. Changing three to two requires an engine rebuild, outside this task.
- `VulkanDevice.cpp:84–98,547`: `r.Vulkan.DelayAcquireBackBuffer` defaults to 1, read-only, cached on device initialization. Zero acquires at frame start; one renders to an intermediate image and acquires just before presentation. `VulkanSwapChain.cpp:43,715–775`: `r.Vulkan.CpuWaitForFence` defaults to 1, read-only; acquire uses `vkAcquireNextImageKHR` with `UINT64_MAX`, then a fence wait with `UINT64_MAX`. These can block indefinitely; they are investigation points, not evidence that either caused this incident. Late runtime `-ExecCmds` changes are unsuitable for these startup/read-only cvars.
- `RenderCore/Private/RenderingThread.cpp:1106`, `HandleRenderTaskHang`, reports the game-thread wait for render completion, not the original stalled call. Do not raise the timeout to mask it.
- `Launch/Private/LaunchEngineLoop.cpp:4709,5691` handles Development `-seconds=N` after accumulated tick time. A stalled loop cannot reach that exit. `LogExit: Exiting.` appears during shutdown. Offscreen bypasses normal windowing/presentation and forces SDL dummy; successful offscreen runs strengthen the presentation hypothesis but do not validate visible input, swapchain or frontend behavior.
- `Programs/AutomationTool/AutomationUtils/ProjectParams.cs:1056,1452` recognizes `-crashreporter`. `build/package-linux.sh --diagnostic <maps...>` retains Development and adds that switch, requires the installed Linux reporter, and checks for an executable staged reporter. The installed `Engine/Binaries/Linux/CrashReportClient` was observed executable in this attempt; archive inclusion remains untested. No missing binary is downloaded or synthesized.

## Driver flags and explicit sync

NVIDIA's [575.51.02 release notes](https://www.nvidia.com/download/driverResults.aspx/243334/en-us/1000/) explicitly extend `__NV_DISABLE_EXPLICIT_SYNC` to Vulkan and describe a fixed Wayland `VK_KHR_present_wait` hang (retrieved 2026-10-08). This supports a **single-variable explicit-sync experiment**, not a diagnosis of 580.178.04 or proof that this package uses present-wait. The 580.178.04 README URL was inaccessible during this worker's retrieval; check the installed driver's documentation and record whether it honors this variable. Do not install/downgrade a driver in this attempt.

NVIDIA documents `__GL_SYNC_TO_VBLANK` and `__GL_YIELD` as [OpenGL environment settings](https://download.nvidia.com/XFree86/Linux-x86_64/304.48/README/openglenvvariables.html) (retrieved 2026-10-08; older documentation). They are not supported here as Vulkan presentation fixes. Do not bundle `__GL_*`, GLX vendor selection, GBM backend, and sync flags into one launch. `DXVK_*` flags target DXVK; this is a native UE Vulkan executable. Vulkan loader `VK_DRIVER_FILES`/`VK_ICD_FILENAMES` and `VK_INSTANCE_LAYERS` change driver/layer selection rather than inherently fixing synchronization; capture existing values and selected GPU/driver, and leave them constant unless a loader/layer issue is evidenced.

Record compositor/XWayland versions, `wayland-info` syncobj protocol availability (if already installed), `nvidia-smi`, `uname -a`, and kernel journal/Xid messages. Protocol advertisement is not proof this client negotiated explicit sync. Disabling it is diagnostic and may introduce tearing or corruption; preserve a baseline. No system/compositor changes were performed by this worker.

## Ranked one-variable A/B experiments

Use a normal desktop user, the same diagnostic archive, saves, map, cap, resolution, compositor and other load. First capture a default baseline of 20 launches × 40 seconds; repeat promising comparisons at 90 seconds and in reversed order. A short all-ok soak does not prove the cause or pass G2. Set `archive=/absolute/path/to/archive` in the host shell. Start each row from its stated baseline rather than accumulating flags.

| Rank | Single changed variable | Command / evidence |
| --- | --- | --- |
| 1 | SDL backend | `LH_SOAK_ENV='SDL_VIDEODRIVER=x11' bash build/launch-soak.sh "$archive" 20 40`, then separately `LH_SOAK_ENV='SDL_VIDEODRIVER=wayland' bash build/launch-soak.sh "$archive" 20 40`. Check `Using SDL video driver` each time. Clear competing `SDL_VIDEO_DRIVER` in the host environment first for both arms. Alternatively compare `LH_SOAK_ARGS='["-sdlvideodriver=x11"]'` against `...wayland`; never combine both mechanisms. Unsupported backend/startup failure is not this render stall. |
| 2 | Explicit sync disabled | Relative to the confirmed backend baseline: `LH_SOAK_ENV='__NV_DISABLE_EXPLICIT_SYNC=1' bash build/launch-soak.sh "$archive" 20 40`. If forcing SDL in both arms, repeat the same SDL assignment in both env strings. Restore the baseline after testing. |
| 3 | Visible presentation removed | `LH_SOAK_ARGS='["-RenderOffscreen"]' bash build/launch-soak.sh "$archive" 20 40`. Confirm dummy SDL. This control includes the consequences of offscreen rendering and is not a visual/input gate. |
| 4 | Acquire timing | Build a separate diagnostic archive with only `[SystemSettings] r.Vulkan.DelayAcquireBackBuffer=0` added in a coordinator-owned experiment; compare to default 1. Startup/read-only cvar requires packaging the config; confirm effective value with diagnostic logs/debugger. Same soak command. Do not persist without evidence. |
| 5 | CPU acquire fence mode | Separate archive with only `r.Vulkan.CpuWaitForFence=0` versus default 1. This changes synchronization to semaphore use; engine-source/driver stacks should guide this lower-priority experiment. Do not combine with row 4. |
| 6 | Desktop presentation stack | Same confirmed X11 backend/package on an already available Xorg desktop versus XWayland. Record desktop/session versions and incidental differences; this is less controlled. No installs/system changes authorized here. |

FIFO and frame cap already failed; keep them as labeled controls, not top-ranked fixes. A gameplay-only control uses `LH_SOAK_ARGS='["/Game/Lighthaven/Maps/Dev_Combat"]'` against the same frontend package. Ensure both maps were cooked. Requesting different swapchain counts is an engine-integrator request, not an invented `r.Vulkan.*` knob.

## Soak behavior and limitations

`LH_SOAK_ENV` accepts space-separated `NAME=value` assignments with shell-style quoting, parsed with `shlex` and passed as child environment overrides; nothing is shell-evaluated. `LH_SOAK_ARGS` remains a JSON array. Invalid values fail before launch. Explicit overrides and arguments are retained in summary.txt. The script prints each direct game PID for stack capture and records last logged frame and time since engine/console log growth. It retains the N+30-second external deadline; missing clean exit plus a logged frame and at least ten seconds without log growth is classified `hang reason=no-log-growth` even without the render-timeout fatal. This conservative diagnostic threshold is provisional; quiet healthy logs alone are not a hang. A zero exit plus clean exit and minimum duration is still required for ok.

After TERM/KILL, reap waits **up to 60 seconds**. An unreaped launch counts once as `hang (unreaped)` and subsequent runs continue; the final summary includes `ok=X hang=Y` and surviving unreaped PIDs. Do not call such runs independent: a blocked process may retain GPU resources and contaminate later results. Preserve PID/kernel evidence and let the owner recover the host if needed. Ctrl-C/TERM clean up only the created process group. No global process kills. Summary reasons remain conservative: nonzero/early exit and missing logs also count in hang totals; inspect logs to separate startup failures from presentation stalls.

## Diagnostic package and live stacks

Build: `UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game`.
Package on the normal-user host:

```sh
UE_ROOT=/home/brewerm/Downloads/unreal bash build/package-linux.sh --diagnostic /Game/Lighthaven/Maps/L_Frontend /Game/Lighthaven/Maps/Dev_Combat /Game/Lighthaven/Maps/Dev_Movement
```

The reporter catches fatal/crash paths, not necessarily a silent stall. Retain the archive reporter, symbols, `Saved/Crashes`, engine/console logs and kernel journal. The script checks reporter presence; cook/stage has not been run here.

Before the soak kills a stalled PID, capture from another host terminal:

```sh
cat /proc/sys/kernel/yama/ptrace_scope
gdb -q -batch -ex 'set pagination off' -ex 'info threads' -ex 'thread apply all bt full' -ex 'detach' -p GAME_PID > stall-stacks.txt 2>&1
```

Use the printed game PID, same UID, installed gdb and matching unstripped symbols. With Yama scope 1 a sibling debugger may be denied; the owner can instead launch the game under gdb as its parent (then interrupt and run `thread apply all bt full`), or explicitly enable temporary ptrace access using their approved host administration procedure, restoring the prior setting afterward. Scope 2 needs administrative debugger privileges; scope 3 cannot be relaxed until reboot. Container/seccomp restrictions can also deny attach. No sysctl or privileges changed here. Capture `/proc/GAME_PID/task/*/wchan` and process state if gdb is unavailable; those are not full user stacks. An uninterruptible kernel task may resist debugger/SIGKILL.

For a separate profiling run use `LH_SOAK_ARGS='["-stdout","-FullStdOutLogOutput","-trace=cpu,frame","-tracefile=/absolute/writable/path/launch.utrace"]'` with **one run**, then open the trace in matching Unreal Insights. Use a unique trace path for each launch; trace overhead is a changed diagnostic variable. UE's TraceLog source parses trace/tracefile options (`Runtime/Core/Private/ProfilingDebugging/TraceAuxiliary.cpp:1955–2000`); `-LogTrace` alone is not established here as an all-thread stack mechanism. An unfinished trace may lack the final stall events and GPU-driver internal stacks. Reproduce without tracing too.

## Worker validation

This attempt runs as uid 0. Native Unreal Automation/editor/game/cook/package/real desktop A/B soaks were not run because Unreal refuses root and this worker has no normal-user desktop session. The expected host test is `Lighthaven.Integration.Wave2.GameplayInputHandoff`; the viewport now has GEngine as its required Within=Engine outer, retaining UI-only ignore-input, gameplay-context and possession assertions. No gate is declared passed. See the attempt report for actual build and controller-fixture checks; W2-06's earlier build timings are not this attempt's results.

Actual build command above exited 0: editor `Result: Succeeded` (102.18 seconds), game `Result: Succeeded` (113.76 seconds). UBA reported some action-result storage tasks did not succeed; both targets nevertheless reported success. Shell syntax, embedded Python syntax, diff whitespace and mocked soak/package-controller scenarios passed. Mock fixtures do not verify Unreal runtime or driver behavior.
