"""Re-render evidence cameras from the exported files, not source scene objects."""
import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
import build
import roster
import bpy,math
import argparse
parser=argparse.ArgumentParser();parser.add_argument('--only',choices=roster.KINDS);parser.add_argument('--all-views',action='store_true');parser.add_argument('--hero-only',action='store_true');parser.add_argument('--view',choices=['hero','gameplay','actions'])
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
    meshes=[o for o in bpy.context.scene.objects if o.type=='MESH' and any(m.type=='ARMATURE' for m in o.modifiers)]
    # Evidence displays body and removable equipment together. Join imported
    # mesh copies only inside this disposable preview scene.
    bpy.ops.object.select_all(action='DESELECT')
    for obj in meshes:obj.select_set(True)
    bpy.context.view_layer.objects.active=meshes[0]
    if len(meshes)>1:bpy.ops.object.join()
    mesh=meshes[0]
    views=(args.view,) if args.view else (('hero',) if args.hero_only else (('hero','gameplay','actions') if args.all_views else ('gameplay',)))
    build.previews(kind,mesh,rig,actions,views=views)
