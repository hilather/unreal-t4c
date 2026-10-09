#!/usr/bin/env bash
set -euo pipefail
# Development packaged binary or UnrealEditor -game; host display required.
exec python3 - "$@" <<'PY'
import argparse, os, re, shutil, subprocess, sys, time
from pathlib import Path
from statistics import mean, median
converter = shutil.which('magick') or shutil.which('convert')
if not converter:
    sys.exit('ImageMagick is required for luminance measurements; no packages installed by this script')
parser = argparse.ArgumentParser(description='Capture arrival rooms with the native 1200cm, yaw45, pitch-55, FOV45 camera')
parser.add_argument('binary', type=Path)
parser.add_argument('--overview', action='store_true', help='Also capture a fixed room-center vantage per map')
options = parser.parse_args()
if os.getuid() == 0:
    sys.exit('Run as the normal desktop user')
binary = options.binary.resolve(strict=True)
root = Path.cwd()
out = root / 'Saved' / 'LightingCapture' / time.strftime('%Y%m%dT%H%M%S')
if any(c.isspace() for c in str(out)):
    sys.exit('Unreal console screenshot paths must not contain whitespace')
out.mkdir(parents=True, exist_ok=False)
# Parse canonical arrival positions on every invocation, rather than maintaining another registry.
registry = (root / 'Source/Lighthaven/World/LHAreaRegistry.cpp').read_text()
rows = re.findall(r'E.Id=Entrance\(TEXT\("Area\.([^" ]+)"\),TEXT\("(Temple.SafeSpawn|Entry)"\)\).*?FRotator\(([^)]+)\),FVector\(([^)]+)\)', registry)
if len(rows) != 5:
    sys.exit(f'Expected five registry arrivals, found {len(rows)}; registry format changed')
# Fixed room-center pawn locations (cm, capsule center); same native camera/boom.
# These are capture vantages only and never edit registry arrivals or map actors.
overviews = {'LighthavenTempleDistrict': (-400,500,90), 'TempleB1': (900,-1900,90),
             'TempleB2': (1000,-5600,90), 'TempleB3': (2900,300,90), 'TempleB4': (0,1000,90)}
shots = []
for area, entry, rotation, position in rows:
    x,y,z = [float(v) for v in position.split(',')]
    _,yaw,_ = [float(v) for v in rotation.split(',')]
    shots.append((area, 'arrival', (x,y,z+90), yaw))
    if options.overview:
        shots.append((area, 'overview', overviews[area], 0))
failed = False
for area, view, position, yaw in shots:
    map_name = 'L_' + area
    x,y,z = position
    label = map_name if view == 'arrival' else map_name + '-overview'
    shot = out / (label + '.png')
    log = out / (label + '.log')
    # BugItGo rotates the PAWN, not the absolute spring arm. Keep the capsule upright;
    # a nonzero registry pitch would tilt the pawn, and +180 elevates the camera target.
    # Native LHCharacter supplies boom1200, pitch-55/yaw45, collision probe and FOV45.
    commands = f'EnableCheats,BugItGo {x} {y} {z} 0 {yaw} 0,r.HighResScreenshotDelay 120,HighResShot 1280x720 filename={shot}'
    args = [str(binary)]
    if 'UnrealEditor' in binary.name:
        args += [str(root / 'Lighthaven.uproject'), '-game']
    args += ['/Game/Lighthaven/Maps/' + map_name, '-windowed', '-ResX=1280', '-ResY=720',
             '-nosplash', '-seconds=15', f'-abslog={log}', '-ExecCmds=' + commands]
    with (out / (label+'.console.log')).open('w') as console:
        process = subprocess.Popen(args, stdout=console, stderr=subprocess.STDOUT)
        try:
            code = process.wait(timeout=55)
        except subprocess.TimeoutExpired:
            process.terminate()
            try: process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                process.kill(); process.wait(timeout=5)
            code = None
    text = log.read_text(errors='replace') if log.exists() else ''
    if code != 0 or not shot.exists() or 'BugItGo to:' not in text:
        print(f'{label}: capture/placement failed exit={code}; inspect {log}')
        failed = True
        continue
    # Rec.709 luma of exported sRGB pixels, normalized 0..1 (not scene-linear illuminance).
    pixels = subprocess.check_output([converter, str(shot), '-colorspace', 'sRGB', '-depth', '8', 'rgb:-'])
    if not pixels or len(pixels) % 3:
        sys.exit(f'Invalid RGB pixel output for {shot}')
    values = [(0.2126*r + 0.7152*g + 0.0722*b)/255 for r,g,b in zip(pixels[0::3],pixels[1::3],pixels[2::3])]
    print(f'{label}: mean={mean(values):.6f} median={median(values):.6f} near_black={sum(v < .02 for v in values)/len(values):.6f} image={shot}')
print(f'Evidence: {out}')
sys.exit(1 if failed else 0)
PY
