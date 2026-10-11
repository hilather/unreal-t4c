"""CPU Cycles 720p evidence, from GLB exports unless explicitly --draft."""
import argparse
import math
import sys
from pathlib import Path
import bpy
from mathutils import Vector
sys.path.insert(0,str(Path(__file__).resolve().parent))
import build
# Same creature review stage, localized without builder side effects.
import stage as creature


def aim(o,target):o.rotation_euler=(Vector(target)-o.location).to_track_quat('-Z','Y').to_euler()

def stage(gameplay=False):
    creature.stage(gameplay=gameplay)
    scene=bpy.context.scene;scene.cycles.samples=16;scene.cycles.seed=51212;scene.cycles.use_denoising=True
    scene.render.image_settings.file_format='PNG';scene.view_settings.look='AgX - Medium High Contrast'
    if not gameplay:
        for o in list(scene.objects):
            if o.type=='LIGHT':bpy.data.objects.remove(o,do_unlink=True)
        for loc,energy,color,size in [((3,-4,5),650,(1,.85,.69),3),((0,3,3),180,(.66,.78,1),3),((-3,1,4),350,(1,.76,.48),2)]:
            bpy.ops.object.light_add(type='AREA',location=loc);o=bpy.context.object;o.data.energy=energy;o.data.color=color;o.data.shape='DISK';o.data.size=size;aim(o,(0,0,1))
    bpy.ops.object.camera_add();cam=bpy.context.object;scene.camera=cam;return cam

def load(kind,draft=False):
    if draft:return build.build(kind,False,False)
    build.clear();before=set(bpy.data.actions);bpy.ops.import_scene.gltf(filepath=str(build.OUT/(kind+'.glb')))
    rig=next(o for o in bpy.context.scene.objects if o.type=='ARMATURE')
    for t in rig.animation_data.nla_tracks:t.mute=True
    actions={a.name.rsplit('|',1)[-1].split('.')[0].removeprefix('player_'):a for a in set(bpy.data.actions)-before}
    meshes=[o for o in bpy.context.scene.objects if o.type=='MESH']
    return meshes,rig,actions

def pose(rig,actions,clip='idle',frame=0):
    if not actions:return
    rig.animation_data.action=actions[clip];rig.animation_data.action_slot=actions[clip].slots[0];bpy.context.scene.frame_set(frame);bpy.context.view_layer.update()

def hair(meshes,style):
    for o in meshes:
        if o.name.startswith('Hair.'):o.hide_render=not o.name.startswith('Hair.'+style)

def render(name):
    bpy.context.scene.render.filepath=str(build.OUT/(name+'.png'));bpy.ops.render.render(write_still=True)

def hero(kind,draft=False):
    meshes,rig,actions=load(kind,draft);pose(rig,actions);hair(meshes,'Tied' if kind=='player_b' else 'Cropped')
    cam=stage();cam.data.type='ORTHO';cam.data.ortho_scale=3.8;cam.location=(6.4,-3.8,2.7);aim(cam,(0,0,.94))
    render(kind+('_draft' if draft else '_hero'))

def gameplay(kind):
    meshes,rig,actions=load(kind);pose(rig,actions);hair(meshes,'Tied' if kind=='player_b' else 'Cropped')
    cam=stage(True);cam.data.type='PERSP';cam.data.angle=math.radians(45);cam.location=(4.867,-4.867,10.73);aim(cam,(0,0,.9))
    render(kind+'_gameplay')

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--only',choices=('player_a','player_b'));p.add_argument('--draft',action='store_true');p.add_argument('--all-views',action='store_true');p.add_argument('--view',choices=('hero','gameplay'),default='hero')
    a=p.parse_args(sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else [])
    for kind in (a.only,) if a.only else ('player_a','player_b'):
        if a.all_views:hero(kind,a.draft);gameplay(kind)
        elif a.view=='hero':hero(kind,a.draft)
        else:gameplay(kind)
