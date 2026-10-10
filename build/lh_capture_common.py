"""Bounded capture launch/reap and retry helpers; no display dependency."""
import json
from pathlib import Path
import subprocess
import sys
import tempfile


def wait_bounded(process, timeout):
    try:
        return process.wait(timeout=timeout)
    except subprocess.TimeoutExpired:
        return None


def record_survivor(pid, destination):
    """Best-effort Linux diagnostics, including permission/disappearance errors."""
    parts = [f'PID: {pid}\n']
    for name in ('status', 'wchan', 'stack'):
        try:
            value = (Path('/proc') / str(pid) / name).read_text()
            if name == 'stack':
                value = '\n'.join(value.splitlines()[:20])
        except OSError as error:
            value = f'unavailable: {error}'
        parts.append(f'\n{name}:\n{value}\n')
    destination.write_text(''.join(parts))


def launch(args, console, diagnostics, *, timeout=55, term_timeout=5,
           kill_timeout=60, process_factory=subprocess.Popen):
    """Timeout stays a failure even if termination exits cleanly."""
    try:
        process = process_factory(args, stdout=console, stderr=subprocess.STDOUT)
    except OSError as error:
        return {'exit': None, 'timed_out': False, 'survived_kill': False,
                'error': str(error)}
    code = wait_bounded(process, timeout)
    timed_out = code is None
    survived = False
    if timed_out:
        try:
            process.terminate()
        except ProcessLookupError:
            pass
        if wait_bounded(process, term_timeout) is None:
            try:
                process.kill()
            except ProcessLookupError:
                pass
            survived = wait_bounded(process, kill_timeout) is None
            if survived:
                record_survivor(process.pid, diagnostics)
    return {'pid': process.pid, 'exit': code, 'timed_out': timed_out,
            'survived_kill': survived}


def retry_view(label, attempt, evidence, *, max_attempts=3):
    """attempt(number) returns (accepted, details); persist each outcome immediately."""
    for number in range(1, max_attempts + 1):
        accepted, details = attempt(number)
        record = dict(details, view=label, attempt=number, captured=accepted)
        with evidence.open('a') as stream:
            stream.write(json.dumps(record) + '\n')
        if accepted:
            break
    print(f'{label}: {"captured" if accepted else "failed"}, attempts={number}', flush=True)
    return accepted, number


def self_test():
    with tempfile.TemporaryDirectory() as folder:
        out = Path(folder)
        # Readiness handshake ensures SIGTERM is ignored before the timeout starts.
        fake = out / 'fake.py'
        fake.write_text('import signal, sys, time\n'
                        'if sys.argv[1] == "exit": sys.exit(0)\n'
                        'if sys.argv[1] == "ignore": signal.signal(signal.SIGTERM, signal.SIG_IGN)\n'
                        'print("ready", flush=True)\ntime.sleep(60)\n')
        def ready_process(args, **kwargs):
            process = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            assert process.stdout.readline() == b'ready\n'
            process.stdout.close()
            return process
        with (out / 'console').open('w') as console:
            result = launch([sys.executable, str(fake), 'exit'], console, out / 'diag', timeout=2)
            assert result['exit'] == 0 and not result['timed_out']
            result = launch([sys.executable, '-c', 'raise SystemExit(7)'], console, out / 'diag', timeout=2)
            assert result['exit'] == 7 and not result['timed_out']
            result = launch([str(out / 'missing')], console, out / 'diag')
            assert 'error' in result
            for mode in ('hang', 'ignore'):
                result = launch([sys.executable, str(fake), mode], console, out / 'diag',
                                timeout=.05, term_timeout=.05, kill_timeout=2,
                                process_factory=ready_process)
                assert result['timed_out'] and not result['survived_kill']
            class Survivor:
                pid = 999999999  # Simulated driver-stuck child, never signal a real PID.
                def wait(self, timeout):
                    raise subprocess.TimeoutExpired('simulated', timeout)
                def terminate(self): pass
                def kill(self): pass
            result = launch([], console, out / 'diag', process_factory=lambda *a, **k: Survivor())
            assert result['survived_kill'] and 'PID: 999999999' in (out / 'diag').read_text()
            record_survivor(__import__('os').getpid(), out / 'live-diag')
            assert 'State:' in (out / 'live-diag').read_text()
        evidence = out / 'attempts.jsonl'
        assert retry_view('retry-success', lambda n: (n == 3, {}), evidence) == (True, 3)
        assert retry_view('exhausted', lambda n: (False, {}), evidence) == (False, 3)
        assert retry_view('next-view', lambda n: (True, {}), evidence) == (True, 1)
        assert len(evidence.read_text().splitlines()) == 7
    print('capture helper self-test passed')


if __name__ == '__main__':
    if sys.argv[1:] != ['--self-test']:
        sys.exit('Usage: lh_capture_common.py --self-test')
    self_test()
