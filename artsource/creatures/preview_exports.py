"""Re-render evidence cameras from the exported files, not source scene objects."""
import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
import build
import bpy,math
import argparse
parser=argparse.ArgumentParser();parser.add_argument('--only',choices=['rat','bat','slime']);parser.add_argument('--all-views',action='store_true')
args=parser.parse_args(sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else [])
for kind in ([args.only] if args.only else ['rat','bat','slime']):
    bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
    bpy.context.scene.render.fps=30
    before=set(bpy.data.actions)
    bpy.ops.import_scene.gltf(filepath=str(build.OUT/(kind+'.glb')))
    rig=next(o for o in bpy.context.scene.objects if o.type=='ARMATURE')
    tracks=rig.animation_data.nla_tracks
    for track in tracks:track.mute=True
    imported=set(bpy.data.actions)-before
    actions={clip:next(a for a in imported if a.name.split('.')[0]==clip) for clip in ['idle','move','attack','hit','death']}
    idle=actions['idle']
    rig.animation_data.action=idle;rig.animation_data.action_slot=idle.slots[0]
    bpy.context.scene.frame_set(0)
    bpy.context.scene.render.engine='CYCLES';bpy.context.scene.cycles.device='CPU'
    if args.all_views:
        mesh=next(o for o in bpy.context.scene.objects if o.type=='MESH' and any(m.type=='ARMATURE' for m in o.modifiers))
        build.previews(kind,mesh,rig,actions)
        continue
    cam=build.stage(kind);cam.data.type='PERSP';cam.data.angle=math.radians(45)
    cam.location=(4.867,-4.867,10.73);build.aim(cam,(0,0,.9))
    bpy.context.scene.render.engine='CYCLES';bpy.context.scene.cycles.device='CPU'
    build.render(kind+'_gameplay')
