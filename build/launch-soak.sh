#!/usr/bin/env bash
set -euo pipefail
if [[ "$(id -u)" == 0 ]]; then
    echo "Unreal refuses to run as root; run on the host as a normal user" >&2
    exit 1
fi
if [[ $# -lt 1 || $# -gt 3 ]]; then
    echo "Usage: $0 <archive-dir> [runs=20] [seconds=40]" >&2
    exit 2
fi
# No editor/UE_ROOT dependency: this runs an existing Development archive.
exec python3 - "$@" <<'PY'
import os
import json
import shlex
import shutil
from pathlib import Path
import re
import signal
import subprocess
import sys
import tempfile
import time

try:
    archive = Path(sys.argv[1]).resolve(strict=True)
    runs = int(sys.argv[2]) if len(sys.argv) > 2 else 20
    seconds = int(sys.argv[3]) if len(sys.argv) > 3 else 40
    if not archive.is_dir() or not 1 <= runs <= 1000 or not 1 <= seconds <= 3600:
        raise ValueError('archive must be a directory; runs 1..1000, seconds 1..3600')
    child_env = os.environ.copy()
    overrides = {}
    for assignment in shlex.split(os.environ.get('LH_SOAK_ENV', '')):
        name, separator, value = assignment.partition('=')
        if not separator or not re.fullmatch(r'[A-Za-z_][A-Za-z0-9_]*', name):
            raise ValueError('LH_SOAK_ENV must contain NAME=value assignments (shell quoting allowed, no evaluation)')
        overrides[name] = value
    child_env.update(overrides)
    extra = json.loads(os.environ.get('LH_SOAK_ARGS', '[]'))
    if not isinstance(extra, list) or any(not isinstance(a, str) for a in extra):
        raise ValueError('LH_SOAK_ARGS must be a JSON array of strings')
    if any(a.lower().startswith(('-seconds', '-abslog', '-benchmark', '-fps', '-nullrhi')) for a in extra):
        raise ValueError('LH_SOAK_ARGS cannot override timing, logs or real rendering')
    # Launch the binary directly, not the wrapper whose child PID is ambiguous.
    binaries = [p for p in archive.rglob('Lighthaven/Binaries/Linux/Lighthaven')
                if p.is_file() and os.access(p, os.X_OK)]
    if len(binaries) != 1:
        raise ValueError(f'expected one Development Lighthaven binary, found {len(binaries)}')
except (OSError, ValueError) as error:
    print(f'Invalid soak arguments/archive: {error}', file=sys.stderr)
    sys.exit(2)

binary = binaries[0]
evidence_root = Path.cwd() / 'Saved' / 'LaunchSoak'
evidence_root.mkdir(parents=True, exist_ok=True)
evidence = Path(tempfile.mkdtemp(prefix='launch-soak-', dir=evidence_root))
print(f'Evidence: {evidence}', flush=True)
active = None
unreaped = []

def cleanup():
    global active
    if active is None:
        return True
    if active.poll() is not None:
        active = None
        return True
    # The new session contains only this launch and its descendants; never pkill.
    try:
        os.killpg(active.pid, signal.SIGTERM)
    except ProcessLookupError:
        pass
    time.sleep(1)
    try:
        os.killpg(active.pid, signal.SIGKILL)
    except ProcessLookupError:
        pass
    process = active
    active = None
    try:
        process.wait(timeout=60)
    except subprocess.TimeoutExpired:
        unreaped.append(process)
        print(f'Unreaped launch PID {process.pid}; continuing soak (resources may remain occupied)', flush=True)
        return False
    return True

def interrupted(signum, frame):
    raise KeyboardInterrupt

signal.signal(signal.SIGTERM, interrupted)
ok = hang = 0
try:
    with (evidence / 'summary.txt').open('w') as summary:
        summary.write(f'env_overrides={overrides!r} args={extra!r}\n')
        for index in range(1, runs + 1):
            log = evidence / f'run-{index:03d}.log'
            console = evidence / f'run-{index:03d}.console.log'
            command = [str(binary), '-windowed', '-ResX=1280', '-ResY=720',
                       f'-seconds={seconds}', f'-abslog={log}', '-log', '-nosplash']
            if os.environ.get('LH_SOAK_IN_PLAY', '0') == '1':
                command += ['/Game/Lighthaven/Maps/L_LighthavenTempleDistrict']
            command += extra
            start = time.monotonic()
            timed_out = False
            code = None
            frame_samples = []
            counter_samples = []
            next_probe = None
            with console.open('w') as output:
                active = subprocess.Popen(command, cwd=binary.parent, stdout=output,
                                          stderr=subprocess.STDOUT, start_new_session=True, env=child_env)
                print(f'run={index} pid={active.pid}', flush=True)
                last_growth = start
                signature = None
                try:
                    while True:
                        now = time.monotonic()
                        current = tuple((p.stat().st_size, p.stat().st_mtime_ns) if p.exists() else None
                                        for p in (log, console))
                        if current != signature:
                            signature = current
                            last_growth = now
                        if next_probe is None: next_probe = start + seconds + 18
                        if now >= next_probe and shutil.which('gdb'):
                            # GFrameCounter is independent of normal log verbosity. Attach may be denied.
                            try:
                                probe = subprocess.run(['gdb', '-q', '-batch', '-p', str(active.pid),
                                    '-ex', 'p GFrameCounter', '-ex', 'detach'], capture_output=True, text=True, timeout=3)
                                with (evidence / f'run-{index:03d}.frames.txt').open('a') as probes:
                                    probes.write(f't={now-start:.1f}\n{probe.stdout}{probe.stderr}\n')
                                match = re.search(r'\$\d+ = (\d+)', probe.stdout)
                                if match: counter_samples.append(int(match.group(1)))
                            except subprocess.TimeoutExpired:
                                pass
                            next_probe = now + 5
                        sample_text = log.read_text(errors='replace') if log.exists() else ''
                        sample_frames = re.findall(r'\[\s*(\d+)\]', sample_text)
                        frame_samples.append((now, int(sample_frames[-1]) if sample_frames else None))
                        code = active.poll()
                        if code is not None:
                            break
                        if now - start >= seconds + 30:
                            timed_out = True
                            break
                        time.sleep(1)
                finally:
                    elapsed = time.monotonic() - start
                    stalled_for = time.monotonic() - last_growth
                    # Capture before cleanup. Attach failures/timeouts remain evidence, not empty success.
                    live = active is not None and active.poll() is None
                    if live and timed_out:
                        stack_path = evidence / f'run-{index:03d}.stacks.txt'
                        with stack_path.open('w') as stacks:
                            debugger = shutil.which('gdb') or shutil.which('eu-stack')
                            if debugger:
                                args = ([debugger, '-q', '-batch', '-p', str(active.pid),
                                         '-ex', 'set pagination off', '-ex', 'thread apply all bt', '-ex', 'detach']
                                        if Path(debugger).name == 'gdb' else [debugger, '-p', str(active.pid)])
                                try:
                                    result = subprocess.run(args, stdout=stacks, stderr=subprocess.STDOUT, timeout=15)
                                    stacks.write(f'\ndebugger_exit={result.returncode}\n')
                                except subprocess.TimeoutExpired:
                                    stacks.write('Debugger timed out after 15 seconds\n')
                            else:
                                stacks.write('gdb/eu-stack absent; full user stacks unavailable\n')
                            for task in sorted(Path(f'/proc/{active.pid}/task').glob('*')):
                                for field in ('status', 'wchan', 'stack'):
                                    try: stacks.write(f'\n{task.name}/{field}: { (task / field).read_text() }\n')
                                    except OSError as error: stacks.write(f'{task.name}/{field}: {error}\n')
                    reaped = cleanup()
            text = log.read_text(errors='replace') if log.exists() else ''
            timeout_error = bool(re.search(r'GameThread timed out waiting for (?:RenderThread|.*Render)', text))
            clean = bool(re.search(r'LogExit:.*Exiting\.', text))
            good = code == 0 and clean and not timeout_error and not timed_out and elapsed >= seconds
            frames = re.findall(r'\[\s*(\d+)\]', text)
            last_frame = frames[-1] if frames else 'unknown'
            recent = [f for t, f in frame_samples if t >= start + elapsed - 10 and f is not None]
            advancing = len(set(counter_samples)) > 1 or len(set(recent)) > 1
            stagnant = timed_out and not advancing and bool(frames)
            observed_frame = counter_samples[-1] if counter_samples else int(last_frame) if frames else None
            classification = ('log-quiet-false-positive' if timed_out and advancing else
                              'startup-stall' if stagnant and observed_frame <= 3 else
                              'late-hang' if stagnant and len(counter_samples) >= 2 else
                              'late-hang-unconfirmed' if stagnant else 'unclassified-exit')
            reason = ('unreaped' if not reaped else 'deadline-no-frame-progress-evidence' if stagnant else 'clean-exit' if good else 'render-timeout' if timeout_error else
                      'wall-timeout' if timed_out else 'missing-clean-exit/early-exit/nonzero-exit')
            ok += int(good)
            hang += int(not good)
            line = f'run={index} {"ok" if good else "hang (unreaped)" if not reaped else "hang"} reason={reason} class={classification} advancing={advancing} exit={code} elapsed={elapsed:.1f}s last_frame={last_frame} counter_samples={counter_samples} stagnant={stalled_for:.1f}s log={log} console={console}'
            print(line, flush=True)
            summary.write(line + '\n')
            summary.flush()
        line = f'ok={ok} hang={hang} unreaped_pids={[p.pid for p in unreaped if p.poll() is None]}'
        print(line, flush=True)
        summary.write(line + '\n')
except (KeyboardInterrupt, OSError, ValueError, RuntimeError) as error:
    print(f'Soak interrupted/failed: {error}; partial evidence: {evidence}', file=sys.stderr)
    sys.exit(2)
finally:
    cleanup()
sys.exit(1 if hang else 0)
PY
