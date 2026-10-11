#!/usr/bin/env bash
set -euo pipefail
# Run against a locally packaged Development executable on the host display.
export LH_CAPTURE_HELPER_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
exec python3 - "$@" <<'PY'
import os, sys, time, shutil, subprocess, argparse, json
from pathlib import Path
sys.dont_write_bytecode = True
sys.path.insert(0, os.environ['LH_CAPTURE_HELPER_DIR'])
from lh_capture_common import launch, retry_view, self_test
def frame_times(count):
    return [frame/(count-1) for frame in range(count)]
if sys.argv[1:] == ['--self-test']:
    self_test()
    for count in (8,4,2,32):
        times=frame_times(count)
        assert len(times)==count and times[0]==0 and times[-1]==1
        assert all(abs(b-a-1/(count-1)) < 1e-12 for a,b in zip(times,times[1:]))
    print('sequence sampling self-test passed')
    sys.exit(0)
parser=argparse.ArgumentParser(description='Still and deterministic creature clip captures')
parser.add_argument('binary')
parser.add_argument('--attack-sequence', action='store_true')
parser.add_argument('--attack-frames', type=int, default=8)
parser.add_argument('--idle-frames', type=int, default=4)
options=parser.parse_args()
if not 2 <= options.attack_frames <= 32 or not 2 <= options.idle_frames <= 32:
    parser.error('frame counts must be between 2 and 32')
if not (os.environ.get('DISPLAY') or os.environ.get('WAYLAND_DISPLAY')):
    sys.exit('Host display required')
binary=Path(options.binary).resolve(strict=True)
out=Path.cwd()/'Saved/CreatureCapture'/time.strftime('%Y%m%dT%H%M%S')
if any(c.isspace() for c in str(out)):
    sys.exit('Unreal console screenshot paths must not contain whitespace')
out.mkdir(parents=True, exist_ok=False)
(out/'sampling.json').write_text(json.dumps({
    'attack_sequence': options.attack_sequence,
    'attack_times': frame_times(options.attack_frames),
    'idle_times': frame_times(options.idle_frames),
    'frame_order': 'attack then idle; inclusive clip endpoints',
}, indent=2)+'\n')
failed = False
kinds=('rat','bat','slime','goblin','giant_spider','balork','goblin_warrior','atrocity','dungeon_bat','giant_bat','undead_bat')
for i,kind in enumerate(kinds):
    jobs=[('gameplay',None,None),('close',None,None)]
    if options.attack_sequence:
        jobs += [(f'{action}-{frame:02d}',action,normalized)
                 for action,count in (('attack',options.attack_frames),('idle',options.idle_frames))
                 for frame,normalized in enumerate(frame_times(count))]
    for view,action,normalized in jobs:
        shot=out/(kind+'-'+view+'.png')
        log=out/(kind+'-'+view+'.log')
        pose=f'close {action} {normalized:.9f}' if action else ('close' if view=='close' else '')
        commands=f'lh.CreatureLineup {i} {pose},r.HighResScreenshotDelay 120,HighResShot 1280x720 filename={shot}'
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
                      and 'LH_CREATURE_LINEUP focus=' in text
                      and 'pawnHidden=1' in text
                      and ('rigLights=3' if view=='close' or action else 'rigLights=0') in text)
            if action:
                accepted &= f'LH_CREATURE_SAMPLE action={action} normalized={normalized:.6f}' in text
            return accepted,result
        captured,_=retry_view(kind+'-'+view,capture_attempt,out/'attempts.jsonl')
        failed |= not captured
montage=shutil.which('montage')
if montage and not failed:
    fonts=subprocess.run([montage,'-list','font'],capture_output=True,text=True)
    available=[line.strip()[6:] for line in fonts.stdout.splitlines() if line.strip().startswith('Font: ')]
    font_args=['-font',available[0]] if available else []
    shots=[]
    for kind in kinds:
        shots += ['-label',kind,str(out/(kind+'-close.png'))]
    result=subprocess.run([montage,*font_args,'-fill','#eeeeee',*shots,'-thumbnail','640x360','-tile','2x6',
                           '-geometry','+8+24','-background','#202020',str(out/'close-contact-sheet.png')])
    failed |= result.returncode != 0
    if options.attack_sequence:
        strips=[]
        for kind in kinds:
            frames=[]
            for action,count in (('attack',options.attack_frames),('idle',options.idle_frames)):
                for frame,normalized in enumerate(frame_times(count)):
                    frames += ['-label',f'{action} {normalized:.3f}',str(out/f'{kind}-{action}-{frame:02d}.png')]
            strip=out/f'{kind}-motion-strip.png'
            result=subprocess.run([montage,*font_args,'-fill','#eeeeee','-background','#202020',*frames,'-thumbnail','320x180','-tile',f'{options.attack_frames+options.idle_frames}x1',
                                   '-geometry','+2+18',str(strip)])
            failed |= result.returncode != 0
            strips += ['-label',kind,str(strip)]
        if not failed:
            result=subprocess.run([montage,*font_args,*strips,'-tile','1x11','-geometry','+2+24',
                                   '-background','#202020','-fill','#eeeeee',str(out/'motion-contact-sheet.png')])
            failed |= result.returncode != 0
elif not montage:
    print('montage unavailable; contact sheet skipped (no install)')
print(out)
sys.exit(1 if failed else 0)
PY
