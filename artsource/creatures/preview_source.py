"""Fast material/form iteration before the bake. Drafts are NOT export evidence."""
import argparse
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
import bpy
import build
import roster

parser = argparse.ArgumentParser()
parser.add_argument('--only', choices=roster.KINDS)
args = parser.parse_args(sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else [])
for kind in ([args.only] if args.only else ['rat', 'bat', 'slime']):
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)
    parts, _ = roster.create(kind)
    cam = build.stage(kind)
    target, scale, location = roster.hero(kind)
    cam.data.type='ORTHO'
    cam.data.ortho_scale=scale
    cam.location=location
    build.aim(cam, target)
    build.render(kind+'_draft')
