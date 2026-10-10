"""Fast material/form iteration before the bake. Drafts are NOT export evidence."""
import argparse
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
import bpy
import build
import models

parser = argparse.ArgumentParser()
parser.add_argument('--only', choices=['rat', 'bat', 'slime'])
args = parser.parse_args(sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else [])
for kind in ([args.only] if args.only else ['rat', 'bat', 'slime']):
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)
    parts, _ = models.create(kind)
    cam = build.stage(kind)
    target = (-.10, 0, .13) if kind == 'rat' else (0, 0, 1.075 if kind == 'bat' else .15)
    cam.data.type = 'ORTHO'
    cam.data.ortho_scale = 1.12 if kind == 'bat' else 1.25
    cam.location = (2.5, -.55, target[2]+.525) if kind == 'bat' else (1.4, -1.8, target[2]+1.0)
    build.aim(cam, target)
    build.render(kind+'_draft')
