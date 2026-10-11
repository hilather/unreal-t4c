"""W5-08f evidence: imported GLBs, warm slate floor, unchanged game camera.

Prototype review illumination, not an Unreal screenshot or a lighting match
measurement. CPU Cycles 1280x720 / 12 samples / denoised. No source images.
Run after build.py, with --only CREATURE --view hero|gameplay|actions, or --family.
"""
import argparse
import math
import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
import bpy
import build
import roster

KINDS=('bat','dungeon_bat','giant_bat','undead_bat','slime')
p=argparse.ArgumentParser()
p.add_argument('--only',choices=KINDS)
p.add_argument('--view',choices=('hero','gameplay','actions'),default='hero')
p.add_argument('--family',action='store_true')
args=p.parse_args(sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else [])

original_stage=build.stage
def warm_stage(kind=None,gameplay=False):
    cam=original_stage(kind,gameplay)
    if gameplay:
        # B1 captures show light weathered stone, not the earlier black studio
        # slate. Keep torch position/power and the fixed 12m camera unchanged.
        mat=build.gameplay_floor_material()
        brick=next(n for n in mat.node_tree.nodes if n.type=='TEX_BRICK')
        brick.inputs['Color1'].default_value=(.12,.13,.125,1)
        brick.inputs['Color2'].default_value=(.20,.205,.18,1)
        brick.inputs['Mortar'].default_value=(.025,.026,.023,1)
    else:
        # Restrain the old bright studio lamps so pale membranes keep pigment.
        for obj in bpy.context.scene.objects:
            if obj.type=='LIGHT':obj.data.energy*=.35
    return cam
build.stage=warm_stage

def imported(kind):
    before=set(bpy.data.actions)
    bpy.ops.import_scene.gltf(filepath=str(build.OUT/(kind+'.glb')))
    rig=next(o for o in bpy.context.selected_objects if o.type=='ARMATURE')
    for track in rig.animation_data.nla_tracks:track.mute=True
    actions={clip:next(a for a in set(bpy.data.actions)-before if a.name.split('.')[0]==clip)
             for clip in ('idle','move','attack','hit','death')}
    rig.animation_data.action=actions['idle'];rig.animation_data.action_slot=actions['idle'].slots[0]
    mesh=next(o for o in bpy.context.selected_objects if o.type=='MESH')
    return mesh,rig,actions

bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
bpy.context.scene.render.fps=30
if args.family:
    for i,kind in enumerate(KINDS[:4]):
        mesh,rig,actions=imported(kind)
        rig.location.y=(i-1.5)*1.65
    bpy.context.scene.frame_set(0)
    cam=warm_stage(gameplay=True)
    # Common scale and warm directional illumination across the entire family.
    # Single light position cannot illuminate a 6m lineup uniformly.
    for obj in list(bpy.context.scene.objects):
        if obj.type=='LIGHT':bpy.data.objects.remove(obj,do_unlink=True)
    for position,energy,color in [((5,-2,4),2.5,(1,.64,.37)),((-4,2,5),.65,(.48,.62,1))]:
        bpy.ops.object.light_add(type='SUN',location=position);light=bpy.context.object
        light.data.energy=energy;light.data.angle=.15;light.data.color=color;build.aim(light,(0,0,0))
    cam.data.type='ORTHO';cam.data.ortho_scale=7.2;cam.location=(6,0,9.42);build.aim(cam,(0,0,.85))
    for i,kind in enumerate(KINDS[:4]):
        bpy.ops.object.text_add(location=(.45,(i-1.5)*1.65,.45))
        label=bpy.context.object;label.data.body=kind.replace('_',' ')
        label.data.size=.13;label.data.align_x='CENTER';label.rotation_euler=cam.rotation_euler
    build.render('bat_family_warm')
else:
    assert args.only,'provide --only CREATURE or --family'
    mesh,rig,actions=imported(args.only)
    if args.only!='slime':
        original_hero=roster.hero
        def elevated_hero(kind):
            target,scale,location=original_hero(kind)
            return target,scale,(2.2,-.7,target[2]+(2.8 if kind=='undead_bat' else 1.65))
        roster.hero=elevated_hero
    build.previews(args.only,mesh,rig,actions,views=(args.view,))
