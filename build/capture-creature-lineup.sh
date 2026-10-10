#!/usr/bin/env bash
set -euo pipefail
# Run against a locally packaged Development executable on the host display.
exec python3 - "$@" <<'PY'
import os, subprocess, sys, time
from pathlib import Path
if len(sys.argv)!=2:
    sys.exit('Usage: capture-creature-lineup.sh /path/to/Development/Lighthaven')
if not (os.environ.get('DISPLAY') or os.environ.get('WAYLAND_DISPLAY')):
    sys.exit('Host display required')
binary=Path(sys.argv[1]).resolve(strict=True)
out=Path.cwd()/'Saved/CreatureCapture'/time.strftime('%Y%m%dT%H%M%S')
out.mkdir(parents=True)
kinds=('rat','bat','slime','goblin','giant_spider','balork','goblin_warrior','atrocity','dungeon_bat','giant_bat','undead_bat')
for i,kind in enumerate(kinds):
    for view in ('gameplay','close'):
        shot=out/(kind+'-'+view+'.png')
        log=out/(kind+'-'+view+'.log')
        commands=f'lh.CreatureLineup {i} '+('close' if view=='close' else '')+f',r.HighResScreenshotDelay 120,HighResShot 1280x720 filename={shot}'
        with log.open('w') as stream:
            process=subprocess.Popen([str(binary),'/Game/Lighthaven/Maps/L_TempleB1','-windowed','-ResX=1280','-ResY=720','-nosplash','-seconds=15','-ExecCmds='+commands],stdout=stream,stderr=subprocess.STDOUT)
            try: code=process.wait(timeout=55)
            except subprocess.TimeoutExpired:
                process.terminate()
                try: process.wait(timeout=5)
                except subprocess.TimeoutExpired: process.kill(); process.wait()
                code=-1
        if code or not shot.exists() or 'LH_CREATURE_LINEUP' not in log.read_text(errors='replace'):
            sys.exit(f'Capture failed: {log}')
print(out)
PY
