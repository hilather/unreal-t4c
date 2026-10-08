# W2-06: G2 Linux launch render hang

## Diagnosis and limits

Leading working hypothesis: Vulkan presentation stalls on the NVIDIA 580.178.04 / GTX 1050 Ti Max-Q / Hyprland (XWayland) path; frontend rendering workload/frame pacing may expose it. This is a mitigation candidate, not a confirmed engine/driver defect or a verified fix.

QA supplied 7 hangs in 15 launches of G2-Linux-ffdb339 versus 0 in 18 G1 launches. The reported fatal is a game-thread wait for the render thread, with RHI breadcrumb `EndDrawingViewport`; idle launches and 0/1/2 saves all reproduce. Survivors remained stable for about four minutes. `-vsync` still logged IMMEDIATE. These observations favor presentation over save-content dependence. The reported frame 375–397 at roughly nine seconds does **not** prove a very high frame rate (about 42–44 frames/s if measured from one common origin). Startup/frame numbering and actual frontend FPS need measurement. No ptrace stacks are available; no raw QA crash directories were located/read in this worker. QA observations above are supplied evidence, not reproduced here.

Reviewed `git diff 8494186 ffdb339` for config, UI, framework/session, persistence and map-generation changes. Startup switches `Dev_Combat` to `L_Frontend`, adds its cook entry and a frontend map commandlet, Slate screens/controller, session and saves. Frontend mode has no default pawn. This changes the presentation workload substantially; unrelated character/persistence additions also mean this is not a controlled driver regression experiment.

At ffdb339, `ALHFrontendController::Tick` calls `Flush` and `Refresh`, then updates input contexts only on screen transitions. `SLHFrontendWidget::Build` is called from construction, screen/modal transitions and actions; there is no widget Tick rebuilding the tree every idle frame. `FLHUIPresenter::Refresh` copies snapshot/profile views; `FLHWave2Session::Profiles` enumerates only when `bProfilesDirty`, and `Flush` returns without a queued save. No obvious infinite game-thread loop or idle repeated save enumeration was found. Copies and Slate bound attributes still cost CPU; this inspection cannot rule out all UI/render interactions. No UI change requested from W2-05 on present evidence. If profiling later shows idle rebuilds, W2-05 should gate `Build` on screen/modal/model revision changes and refresh views only when dirty, preserving focus and staged input.

## Engine source evidence (local UE 5.8.3)

Paths below are under `/home/brewerm/Downloads/unreal/Engine/Source/Runtime/`, inspected 2026-10-08.

- `RenderCore/Private/RenderingThread.cpp:1106`, `HandleRenderTaskHang`: Linux emits the fatal game-thread wait message. This identifies the wait that failed, not the original stalled call. Increasing render-fence timeouts would hide the symptom and is not proposed.
- `Engine/Private/UnrealEngine.cpp:12136–12239`: `t.MaxFPS` defaults to zero (uncapped); positive values override smoothing in `UEngine::GetMaxTickRate`. `UpdateTimeAndHandleMaxTickRate` around line 3058 uses this rate to pace real time. This does not impose a fixed simulation timestep.
- `Engine/Private/SystemSettings.cpp`: `LoadFromIni` reads system settings, applying console variables with `ECVF_SetBySystemSettingsIni` through `OnSetCVarFromIniEntry`. Thus `[SystemSettings]` in the project engine config is a supported place for the cap.
- `VulkanRHI/Private/VulkanViewport.cpp:110`: viewport construction reads `r.VSync` into `LockToVsync` before swapchain creation.
- `VulkanRHI/Private/VulkanSwapChain.cpp:434–496`: `-vulkanpresentmode=2` explicitly requests FIFO if supported (Vulkan enum FIFO=2). Without an explicit request, unlocked vsync selects IMMEDIATE when available; otherwise MAILBOX is preferred over FIFO. Consequently `r.VSync=1` is not a guarantee of FIFO, and the supplied `-vsync` observation is not evidence that a FIFO experiment was performed. No config cvar forcing FIFO was found in this selection path. `r.Vulkan.ForcePacingWithoutVSync` keeps CPU pacers enabled but does not select FIFO.
- `Launch/Private/LaunchEngineLoop.cpp:4709,5691`: non-Shipping `-seconds=N` requests normal exit after accumulated tick time exceeds N; it needs no benchmark/fixed-FPS mode. A stalled loop cannot reach that exit, so the script also enforces an external wall deadline. `LogExit: Exiting.` is emitted around line 7007 during engine shutdown.

## Mitigation

Only `[SystemSettings] t.MaxFPS=60` is added to `Config/DefaultEngine.ini`. Sixty is a provisional engineering cap, not an authentic T4C rule. It bounds frontend and gameplay render submission without changing physics to a fixed timestep, changing saved data, or forcing a platform presentation mode. It may reduce fluidity on high-refresh displays. Confirm the effective cap/FPS in a real package; user settings, device profiles or later console writes can override it. The inspected engine `Engine/Config/BaseEngine.ini:351` sets `bSmoothFrameRate=false` despite a stored 22–62 smoothing range, and no project cap or vsync assignment was found in Config/Source. Thus the default engine path permits uncapped ticks; actual old-package FPS is still unmeasured and GPU/driver limits apply.

Keep present mode unchanged for the first comparison so the cap is the sole changed variable. If the cap alone fails, test FIFO with `LH_SOAK_ARGS='["-vulkanpresentmode=2"]'` and confirm the selected mode in every run log. Do not claim FIFO from `-vsync` alone. Persist a platform-specific change only after evidence supports it.

## Host soak procedure

Use a normal desktop user and the same compositor, GPU/driver, save directory, window resolution and other load as QA. This worker's `id -u` is 0, so it cannot run the editor, game or native Automation.

1. Keep the original G2 archive intact. From this checkout run `bash build/launch-soak.sh /absolute/path/to/original-G2-archive 20 40`.
2. Build with `UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game`, requiring `Result: Succeeded` for both targets. Run `bash build/run-tests.sh Lighthaven` on the host.
3. Package the mitigation separately, explicitly including the frontend: `bash build/package-linux.sh /Game/Lighthaven/Maps/L_Frontend /Game/Lighthaven/Maps/Dev_Combat /Game/Lighthaven/Maps/Dev_Movement`. Merely editing this checkout does not change an existing pak/archive.
4. Run `bash build/launch-soak.sh /absolute/path/to/new-archive 20 40`. Inspect frontend map, effective pacing and `Selected VkPresentModeKHR` log entries. Repeat at 90 seconds to retain the 60-second fatal if the render loop stalls; the default 40-second run may kill a stalled run before that fatal is emitted.
5. If needed, compare the new archive with `LH_SOAK_ARGS='["-vulkanpresentmode=2"]' bash build/launch-soak.sh /absolute/path/to/new-archive 20 40`. A gameplay-map control can use `LH_SOAK_ARGS='["/Game/Lighthaven/Maps/Dev_Combat"]'`. These additional arguments are a JSON string array, never shell code. Do not inject quit/test-exit flags or change timing.

The script finds exactly one executable `Lighthaven/Binaries/Linux/Lighthaven` recursively under the archive and runs it directly, avoiding wrapper child-PID ambiguity. It requires a Development package (`-seconds` is unavailable in normal Shipping). It launches sequentially with real rendering, windowed 1280x720, `-seconds=N`, and unique absolute logs. An external deadline is N+30 seconds, allowing startup/shutdown overhead. Slow startup beyond that budget is conservatively counted as hang; inspect logs before treating it as this render defect.

Evidence is retained in `Saved/LaunchSoak/launch-soak-*/` under the invoking directory: engine logs, console logs and `summary.txt`. Each `ok` requires zero process status, at least N seconds elapsed, no render timeout and `LogExit: Exiting.`. Every other outcome counts as `hang`, with a reason (including early exit, startup failure, nonzero status or missing log). This is deliberately conservative: `hang` is not automatically the reported render defect. Exit status is 0 for all ok, 1 for any hang, 2 for invalid input/interruption. Root exits 1 before launching. Cleanup uses only the process group/session created for that launch, TERM then KILL, including its descendants; no global process-name kills or compositor-dialog kills. Ctrl-C/TERM also clean up that launch. If SIGKILL cannot reap a kernel-blocked process within five seconds, the soak stops with an error and names its PID instead of launching more instances. A forced kill is never a clean exit.

An all-ok short soak is evidence of improved launch reliability, not proof of root cause or of long-session stability. Follow with interactive frontend/create/load/play and a multi-minute survivor run; gates remain for coordinator review.

## Crash diagnostics

QA reports `CrashReportClient` missing (`CreateProc: File does not exist`). The reporter is useful for local crash-context/log collection and should be included in diagnostic Development archives if that engine distribution contains a Linux reporter binary. UAT BuildCookRun accepts `-crashreporter`; optionally add it to the invocation in `build/package-linux.sh` for a diagnostic package, then verify the staged/archive reporter is actually present and executable. Packaging is unchanged here: reporter availability has not been validated and adding the switch cannot manufacture a missing engine binary. Preserve `Saved/Crashes`, logs and QA's four crashinfo directories. Thread stack collection still needs host-approved debugger/ptrace access; no system settings were changed.

## Worker validation

Shell syntax and root refusal were checked. Synthetic executable fixtures exercise the soak controller only; they are not Unreal play evidence. `UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game` exited 0: editor `Result: Succeeded` (146.64 seconds), game `Result: Succeeded` (142.24 seconds). UBA reported that some action result store tasks did not succeed, but both targets built. Exact worker evidence is in the attempt report. Native Automation, cook, package, real launch and before/after soak remain host work because Unreal refuses this root worker. No gate is passed by this document.
