#!/usr/bin/env bash
set -euo pipefail
# Development packaged binary or UnrealEditor -game; host display required.
exec python3 - "$@" <<'PY'
import os, re, shutil, subprocess, sys, time
from pathlib import Path
from statistics import mean, median
converter = shutil.which('magick') or shutil.which('convert')
if not converter:
    sys.exit('ImageMagick is required for luminance measurements; no packages installed by this script')
if len(sys.argv) != 2 or os.getuid() == 0:
    sys.exit('Usage (normal desktop user): capture-map-screenshots.sh /path/to/Development-Lighthaven-or-UnrealEditor')
binary = Path(sys.argv[1]).resolve(strict=True)
root = Path.cwd()
out = root / 'Saved' / 'LightingCapture' / time.strftime('%Y%m%dT%H%M%S')
out.mkdir(parents=True, exist_ok=False)
# Parse canonical arrival positions on every invocation, rather than maintaining another registry.
registry = (root / 'Source/Lighthaven/World/LHAreaRegistry.cpp').read_text()
rows = re.findall(r'E.Id=Entrance\(TEXT\("Area\.([^" ]+)"\),TEXT\("(Temple.SafeSpawn|Entry)"\)\).*?FRotator\(([^)]+)\),FVector\(([^)]+)\)', registry)
if len(rows) != 5:
    sys.exit(f'Expected five registry arrivals, found {len(rows)}; registry format changed')
failed = False
for area, entry, rotation, position in rows:
    map_name = 'L_' + area
    x,y,z = [float(v) for v in position.split(',')]
    pitch,yaw,roll = [float(v) for v in rotation.split(',')]
    shot = out / (map_name + '.png')
    log = out / (map_name + '.log')
    commands = f'EnableCheats,BugItGo {x} {y} {z+180} {pitch} {yaw} {roll},r.HighResScreenshotDelay 120,HighResShot 1280x720 filename={shot}'
    args = [str(binary)]
    if 'UnrealEditor' in binary.name:
        args += [str(root / 'Lighthaven.uproject'), '-game']
    args += ['/Game/Lighthaven/Maps/' + map_name, '-windowed', '-ResX=1280', '-ResY=720',
             '-nosplash', '-seconds=15', f'-abslog={log}', '-ExecCmds=' + commands]
    with (out / (map_name+'.console.log')).open('w') as console:
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
        print(f'{map_name}: capture/placement failed exit={code}; inspect {log}')
        failed = True
        continue
    # Rec.709 luma of exported sRGB pixels, normalized 0..1 (not scene-linear illuminance).
    pixels = subprocess.check_output([converter, str(shot), '-colorspace', 'sRGB', '-depth', '8', 'rgb:-'])
    if not pixels or len(pixels) % 3:
        sys.exit(f'Invalid RGB pixel output for {shot}')
    values = [(0.2126*r + 0.7152*g + 0.0722*b)/255 for r,g,b in zip(pixels[0::3],pixels[1::3],pixels[2::3])]
    print(f'{map_name}: mean={mean(values):.6f} median={median(values):.6f} image={shot}')
print(f'Evidence: {out}')
sys.exit(1 if failed else 0)
PY
