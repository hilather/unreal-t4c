#!/usr/bin/env bash
set -euo pipefail
# Run against a locally packaged Development executable on the host display.
export LH_CAPTURE_HELPER_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
exec python3 - "$@" <<'PY'
import os, sys, time
from pathlib import Path
sys.dont_write_bytecode = True
sys.path.insert(0, os.environ['LH_CAPTURE_HELPER_DIR'])
from lh_capture_common import launch, retry_view, self_test
if sys.argv[1:] == ['--self-test']:
    self_test()
    sys.exit(0)
if len(sys.argv)!=2:
    sys.exit('Usage: capture-creature-lineup.sh /path/to/Development/Lighthaven')
if not (os.environ.get('DISPLAY') or os.environ.get('WAYLAND_DISPLAY')):
    sys.exit('Host display required')
binary=Path(sys.argv[1]).resolve(strict=True)
out=Path.cwd()/'Saved/CreatureCapture'/time.strftime('%Y%m%dT%H%M%S')
if any(c.isspace() for c in str(out)):
    sys.exit('Unreal console screenshot paths must not contain whitespace')
out.mkdir(parents=True, exist_ok=False)
failed = False
kinds=('rat','bat','slime','goblin','giant_spider','balork','goblin_warrior','atrocity','dungeon_bat','giant_bat','undead_bat')
for i,kind in enumerate(kinds):
    for view in ('gameplay','close'):
        shot=out/(kind+'-'+view+'.png')
        log=out/(kind+'-'+view+'.log')
        commands=f'lh.CreatureLineup {i} '+('close' if view=='close' else '')+f',r.HighResScreenshotDelay 120,HighResShot 1280x720 filename={shot}'
        args=[str(binary),'/Game/Lighthaven/Maps/L_TempleB1','-windowed','-ResX=1280','-ResY=720','-nosplash','-seconds=15',f'-abslog={log}','-ExecCmds='+commands]
        console=out/(kind+'-'+view+'.console.log')
        def capture_attempt(number):
            for path in (shot,log,console):
                if path.exists():
                    path.rename(path.with_name(path.stem+f'.attempt{number-1}'+path.suffix))
            with console.open('w') as stream:
                result=launch(args,stream,out/(kind+'-'+view+f'.attempt{number}.process.txt'))
            text=log.read_text(errors='replace') if log.exists() else ''
            accepted=(result['exit']==0 and shot.exists() and shot.stat().st_size>0
                      and 'LH_CREATURE_LINEUP' in text)
            return accepted,result
        captured,_=retry_view(kind+'-'+view,capture_attempt,out/'attempts.jsonl')
        failed |= not captured
print(out)
sys.exit(1 if failed else 0)
PY
