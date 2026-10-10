"""Re-render gameplay cameras from the exported files, not source scene objects."""
import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
import build
import bpy,math
for kind in ['rat','bat','slime']:
    bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
    before=set(bpy.data.actions)
    bpy.ops.import_scene.gltf(filepath=str(build.OUT/(kind+'.glb')))
    rig=next(o for o in bpy.context.scene.objects if o.type=='ARMATURE')
    tracks=rig.animation_data.nla_tracks
    for track in tracks:track.mute=True
    idle=next(a for a in set(bpy.data.actions)-before if a.name.startswith('idle'))
    rig.animation_data.action=idle;rig.animation_data.action_slot=idle.slots[0]
    bpy.context.scene.frame_set(0)
    cam=build.stage();cam.data.type='PERSP';cam.data.angle=math.radians(45)
    cam.location=(4.867,-4.867,10.73);build.aim(cam,(0,0,.9))
    bpy.context.scene.render.engine='CYCLES';bpy.context.scene.cycles.device='CPU'
    build.render(kind+'_gameplay')
