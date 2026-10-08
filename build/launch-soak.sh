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

def cleanup():
    global active
    if active is None:
        return
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
        process.wait(timeout=5)
    except subprocess.TimeoutExpired:
        raise RuntimeError(f'launch PID {process.pid} did not reap after SIGKILL; stopping soak')

def interrupted(signum, frame):
    raise KeyboardInterrupt

signal.signal(signal.SIGTERM, interrupted)
ok = hang = 0
try:
    with (evidence / 'summary.txt').open('w') as summary:
        for index in range(1, runs + 1):
            log = evidence / f'run-{index:03d}.log'
            console = evidence / f'run-{index:03d}.console.log'
            command = [str(binary), '-windowed', '-ResX=1280', '-ResY=720',
                       f'-seconds={seconds}', f'-abslog={log}', '-log', '-nosplash']
            # Optional comparison arguments are a JSON array, never shell-evaluated.
            import json
            extra = json.loads(os.environ.get('LH_SOAK_ARGS', '[]'))
            if not isinstance(extra, list) or any(not isinstance(a, str) for a in extra):
                raise ValueError('LH_SOAK_ARGS must be a JSON array of strings')
            # Preserve duration/log identity and normal real-time rendering.
            if any(a.lower().startswith(('-seconds', '-abslog', '-benchmark', '-fps', '-nullrhi')) for a in extra):
                raise ValueError('LH_SOAK_ARGS cannot override timing, logs or real rendering')
            command += extra
            start = time.monotonic()
            timed_out = False
            with console.open('w') as output:
                active = subprocess.Popen(command, cwd=binary.parent, stdout=output,
                                          stderr=subprocess.STDOUT, start_new_session=True)
                try:
                    code = active.wait(timeout=seconds + 30)
                except subprocess.TimeoutExpired:
                    timed_out = True
                    code = None
                finally:
                    elapsed = time.monotonic() - start
                    cleanup()
            text = log.read_text(errors='replace') if log.exists() else ''
            timeout_error = bool(re.search(r'GameThread timed out waiting for (?:RenderThread|.*Render)', text))
            clean = bool(re.search(r'LogExit:.*Exiting\.', text))
            good = code == 0 and clean and not timeout_error and not timed_out and elapsed >= seconds
            reason = ('clean-exit' if good else 'render-timeout' if timeout_error else
                      'wall-timeout' if timed_out else 'missing-clean-exit/early-exit/nonzero-exit')
            ok += int(good)
            hang += int(not good)
            line = f'run={index} {"ok" if good else "hang"} reason={reason} exit={code} elapsed={elapsed:.1f}s log={log} console={console}'
            print(line, flush=True)
            summary.write(line + '\n')
            summary.flush()
        line = f'ok={ok} hang={hang}'
        print(line, flush=True)
        summary.write(line + '\n')
except (KeyboardInterrupt, OSError, ValueError, RuntimeError) as error:
    print(f'Soak interrupted/failed: {error}; partial evidence: {evidence}', file=sys.stderr)
    sys.exit(2)
finally:
    cleanup()
sys.exit(1 if hang else 0)
PY
