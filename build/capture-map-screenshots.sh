#!/usr/bin/env bash
set -euo pipefail
# Development packaged binary or UnrealEditor -game; host display required.
export LH_CAPTURE_HELPER_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
exec python3 - "$@" <<'PY'
import argparse, os, re, shutil, subprocess, sys, time
from pathlib import Path
from statistics import mean, median
sys.dont_write_bytecode = True
sys.path.insert(0, os.environ['LH_CAPTURE_HELPER_DIR'])
from lh_capture_common import launch, retry_view, self_test
if sys.argv[1:] == ['--self-test']:
    self_test()
    sys.exit(0)
converter = shutil.which('magick') or shutil.which('convert')
if not converter:
    sys.exit('ImageMagick is required for luminance measurements; no packages installed by this script')
parser = argparse.ArgumentParser(description='Capture three fixed CameraActor room views per generated map')
parser.add_argument('binary', type=Path)
parser.add_argument('--overview', action='store_true', help='Compatibility option; three room views are always captured')
options = parser.parse_args()
if os.getuid() == 0:
    sys.exit('Run as the normal desktop user')
if not (os.environ.get('DISPLAY') or os.environ.get('WAYLAND_DISPLAY')):
    sys.exit('A host X11/Wayland display is required for rendered captures')
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
shots = [(area, f'room{i}', (0,0,0), 0) for area, *_ in rows for i in range(1,4)]
failed = False
measurements = ["map_view\tmean\tmedian\tnear_black\timage"]
for area, view, position, yaw in shots:
    map_name = 'L_' + area
    x,y,z = position
    label = map_name + '-' + view
    shot = out / (label + '.png')
    log = out / (label + '.log')
    # ViewActor selects a generated CameraActor; Default uses its CalcCamera.
    # No pawn teleport, spring-arm collision or control rotation affects this view.
    commands = ','.join([
        'EnableCheats', f'ViewActor LH_Capture_{view[-1]}', 'Camera Default',
        'getall PlayerCameraManager ViewTarget',
        'getall PlayerCameraManager',
        'r.HighResScreenshotDelay 120', f'HighResShot 1280x720 filename={shot}'])
    print(f'{label}: ViewActor LH_Capture_{view[-1]} FOV65', flush=True)
    args = [str(binary)]
    if 'UnrealEditor' in binary.name:
        args += [str(root / 'Lighthaven.uproject'), '-game']
    args += ['/Game/Lighthaven/Maps/' + map_name, '-windowed', '-ResX=1280', '-ResY=720',
             '-nosplash', '-seconds=15', f'-abslog={log}', '-ExecCmds=' + commands]
    def capture_attempt(number):
        # Remove stale output before every launch; a previous image cannot pass a retry.
        # Preserve failed evidence under attempt-specific names; successful paths stay canonical.
        for path in (shot, log, out / (label + '.console.log')):
            if path.exists():
                path.rename(path.with_name(path.stem + f'.attempt{number-1}' + path.suffix))
        with (out / (label+'.console.log')).open('w') as console:
            result = launch(args, console, out / (label + f'.attempt{number}.process.txt'))
        text = log.read_text(errors='replace') if log.exists() else ''
        accepted = (result['exit'] == 0 and shot.exists()
                    and re.search(r'ViewTarget[^\n]*Target=[^\n]*LH_Capture_' + view[-1], text)
                    and 'Unrecognized property' not in text and 'ImportText (' not in text)
        if not accepted:
            print(f'{label}: attempt {number} capture/placement failed; {result}; inspect {log}', flush=True)
        if accepted:
            try:
                # Rec.709 luma of exported sRGB pixels, normalized 0..1 (not scene-linear illuminance).
                pixels = subprocess.check_output([converter, str(shot), '-colorspace', 'sRGB', '-depth', '8', 'rgb:-'])
                if not pixels or len(pixels) % 3:
                    raise ValueError(f'Invalid RGB pixel output for {shot}')
                values = [(0.2126*r + 0.7152*g + 0.0722*b)/255 for r,g,b in zip(pixels[0::3],pixels[1::3],pixels[2::3])]
                measurements.append(f'{label}\t{mean(values):.6f}\t{median(values):.6f}\t{sum(v < .02 for v in values)/len(values):.6f}\t{shot}')
                print(f'{label}: mean={mean(values):.6f} median={median(values):.6f} near_black={sum(v < .02 for v in values)/len(values):.6f} image={shot}')
            except (OSError, subprocess.CalledProcessError, ValueError) as error:
                accepted = False
                result['measurement_error'] = str(error)
        return bool(accepted), result
    captured, attempts = retry_view(label, capture_attempt, out / 'attempts.jsonl')
    if not captured:
        failed = True
        continue
(out / 'luminance.tsv').write_text('\n'.join(measurements) + '\n')
print(f'Evidence: {out}')
sys.exit(1 if failed else 0)
PY
