"""Cycles CPU 720p evidence from actual NPC exports, never an Unreal capture."""
import argparse
import importlib.util
import json
import math
from pathlib import Path
import sys
sys.dont_write_bytecode=True
import bpy
from mathutils import Vector
HERE=Path(__file__).resolve().parent
OUT=HERE/'output'
spec=importlib.util.spec_from_file_location('player_stage',HERE.parent/'player/stage.py')
stage_source=importlib.util.module_from_spec(spec)
spec.loader.exec_module(stage_source)


def aim(ob,target):
    ob.rotation_euler=(Vector(target)-ob.location).to_track_quat('-Z','Y').to_euler()


def clear():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.context.scene.render.fps=30


def load(npc):
    objects_before=set(bpy.data.objects)
    actions_before=set(bpy.data.actions)
    bpy.ops.import_scene.gltf(filepath=str(OUT/(npc+'.glb')))
    objects=list(set(bpy.data.objects)-objects_before)
    rig=next(ob for ob in objects if ob.type=='ARMATURE')
    for track in rig.animation_data.nla_tracks:
        track.mute=True
    actions={a.name.rsplit('|',1)[-1].split('.')[0]:a for a in set(bpy.data.actions)-actions_before}
    return objects,rig,actions


def freeze(objects,rig,actions,clip='idle',frame=0,pos=(0,0,0),yaw=-.35,scale=1):
    rig.animation_data.action=actions[clip]
    rig.animation_data.action_slot=actions[clip].slots[0]
    bpy.context.scene.frame_set(frame)
    bpy.context.view_layer.update()
    deps=bpy.context.evaluated_depsgraph_get()
    result=[]
    for ob in objects:
        if ob.type!='MESH' or not any(mod.type=='ARMATURE' and mod.object==rig for mod in ob.modifiers):
            continue
        evaluated=ob.evaluated_get(deps)
        data=bpy.data.meshes.new_from_object(evaluated,preserve_all_data_layers=True,depsgraph=deps)
        data.transform(evaluated.matrix_world)
        frozen=bpy.data.objects.new('Review.'+ob.name,data)
        bpy.context.collection.objects.link(frozen)
        frozen.location=pos
        frozen.rotation_euler.z=yaw
        frozen.scale=(scale,)*3
        result.append(frozen)
    return result


def remove(objects):
    for ob in objects:
        bpy.data.objects.remove(ob,do_unlink=True)


def matte(name,color,rough=.85):
    return stage_source.material(name,color,rough,40)


def cube(name,pos,scale,material,bevel=.025):
    bpy.ops.mesh.primitive_cube_add(size=1,location=pos)
    ob=bpy.context.object
    ob.name=name
    ob.scale=scale
    bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    ob.data.materials.append(material)
    if bevel:
        mod=ob.modifiers.new('soft dressed edges','BEVEL');mod.width=bevel;mod.segments=2
    return ob


def lighting(gameplay=False,lineup=False):
    scene=bpy.context.scene
    scene.render.engine='CYCLES'
    scene.cycles.device='CPU'
    scene.cycles.samples=12
    scene.cycles.seed=516
    scene.cycles.use_denoising=True
    if lineup:
        scene.cycles.max_bounces=4
        scene.cycles.diffuse_bounces=2
        scene.cycles.glossy_bounces=2
    scene.render.resolution_x=1280
    scene.render.resolution_y=720
    scene.render.resolution_percentage=100
    scene.render.image_settings.file_format='PNG'
    scene.render.image_settings.color_mode='RGB'
    scene.render.image_settings.compression=75
    scene.view_settings.look='AgX - Medium High Contrast'
    scene.world=bpy.data.worlds.new('Review world')
    scene.world.use_nodes=True
    bg=scene.world.node_tree.nodes.get('Background')
    bg.inputs[0].default_value=(.13,.16,.20,1)
    bg.inputs[1].default_value=.22 if gameplay else .32
    bpy.ops.mesh.primitive_plane_add(size=200)
    floor=bpy.context.object
    floor.data.materials.append(stage_source.gameplay_floor_material() if gameplay else matte('studio charcoal',(.028,.033,.036)))
    if lineup:
        lamps=[]
        for x in (0,3.7,7.4):
            lamps.extend([((x+3,0,5),1300,(1,.88,.74),10),((x-2,0,4),900,(.76,.83,1),8)])
    elif gameplay:
        lamps=[((1.5,-1.4,3.1),850,(1,.68,.39),1.1),((-1.5,2,4.4),340,(.63,.71,.84),5)]
    else:
        lamps=[((3,-4,5),800,(1,.89,.76),4),((1,4,3.6),550,(.74,.83,1),3),((-3,.6,4),800,(1,.78,.53),3)]
    for loc,power,color,size in lamps:
        bpy.ops.object.light_add(type='AREA',location=loc)
        ob=bpy.context.object;ob.data.energy=power;ob.data.color=color;ob.data.shape='DISK';ob.data.size=size
        aim(ob,(loc[0]-2 if lineup else 0,0,.9))
    bpy.ops.object.camera_add()
    cam=bpy.context.object
    scene.camera=cam
    return cam


def text_material():
    mat=bpy.data.materials.get('review lettering')
    if mat:return mat
    mat=bpy.data.materials.new('review lettering');mat.use_nodes=True
    n,l=mat.node_tree.nodes,mat.node_tree.links
    e=n.new('ShaderNodeEmission');e.inputs[0].default_value=(.70,.66,.53,1)
    l.new(e.outputs[0],n.get('Material Output').inputs[0])
    return mat


def label(cam,body,x,y,size=.075):
    data=bpy.data.curves.new('review caption','FONT');data.body=body;data.size=size
    ob=bpy.data.objects.new('Caption',data);bpy.context.collection.objects.link(ob)
    ob.parent=cam;ob.location=(x,y,-5);ob.rotation_euler=(0,0,0)
    data.materials.append(text_material())


def render(name):
    path=OUT/(name+'.png')
    bpy.context.scene.render.filepath=str(path)
    bpy.ops.render.render(write_still=True)
    print('NPC_RENDERED',path,flush=True)


def hero(row):
    clear()
    objects,rig,actions=load(row['id'])
    freeze(objects,rig,actions,pos=(0,-.66,0))
    freeze(objects,rig,actions,'talk',30,pos=(0,1.03,0),yaw=2.5,scale=.78)
    remove(objects)
    tall='spear' in row['extras']
    cam=lighting();cam.data.type='ORTHO';cam.data.ortho_scale=5.1 if tall else 4.6
    cam.location=(7,0,2.95);aim(cam,(0,0,1.08 if tall else 1.03))
    label(cam,row['id'].removeprefix('NPC.'),-2.08,1.24 if tall else 1.13,.12)
    label(cam,row['role'].split(';')[0],-2.08,1.08 if tall else .97,.057)
    label(cam,'IDLE / FRONT',-1.13,-1.20 if tall else -1.08,.052)
    label(cam,'TALK / REAR',.60,-1.20 if tall else -1.08,.052)
    label(cam,'W5-16  /  CYCLES CPU  /  EXPORTED GLB',-2.08,-1.32 if tall else -1.21,.043)
    render(row['id']+'_hero')


def lineup(rows):
    clear()
    plinth=matte('lineup plinth',(.08,.073,.056))
    for index,row in enumerate(rows):
        col=index%5;band=index//5
        pos=(band*3.9,(col-2)*2.48,.045)
        objects,rig,actions=load(row['id'])
        freeze(objects,rig,actions,pos=pos,yaw=-.28)
        remove(objects)
        cube('lineup stone', (pos[0],pos[1],.0),(.95,1.10,.08),plinth)
    cam=lighting(lineup=True);cam.data.type='ORTHO';cam.data.ortho_scale=15.8
    target=Vector((3.9,0,.7));cam.location=target+Vector((13,0,10));aim(cam,target)
    label(cam,'LIGHTHAVEN  /  THE FIFTEEN',-7.45,4.09,.26)
    label(cam,'Shared player bodies  |  Original role costumes  |  Cycles CPU / exported GLBs',-7.45,3.82,.115)
    # World labels remain beside each plinth at the camera-facing edge.
    for index,row in enumerate(rows):
        col=index%5;band=index//5
        data=bpy.data.curves.new('NPC caption','FONT');data.body=row['id'].removeprefix('NPC.');data.align_x='CENTER';data.size=.15
        ob=bpy.data.objects.new('NPC caption',data);bpy.context.collection.objects.link(ob)
        ob.location=(band*3.9+.89,(col-2)*2.48,.07);ob.rotation_euler=cam.rotation_euler;data.materials.append(text_material())
    render('lineup')


def chapel():
    clear()
    placements=[('NPC.BrotherKiran',(-1.8,0,0),0),('NPC.Kilhiam',(-.5,2,0),-.4),
                ('NPC.Fali',(.7,-1.9,0),.3),('NPC.Murmuntag',(2,1,0),-.6)]
    for npc,pos,yaw in placements:
        objects,rig,actions=load(npc)
        freeze(objects,rig,actions,'talk' if 'Kilhiam' in npc else 'idle',30,pos=pos,yaw=yaw)
        remove(objects)
    cam=lighting(gameplay=True)
    stone=matte('chapel warm limestone',(.20,.17,.12),.95)
    wood=matte('chapel worn oak',(.10,.052,.023),.8)
    cloth=matte('chapel muted red runner',(.105,.027,.018),.94)
    cube('aisle runner',(1,0,.007),(8,1.4,.009),cloth,.004)
    for y in (-2.9,2.9):
        for x in (.4,2.6):
            cube('bench seat',(x,y,.45),(.40,1.32,.12),wood)
            for offset in (-.45,.45):
                cube('bench leg',(x,y+offset,.22),(.25,.14,.44),wood)
    cube('altar foot',(-3.1,0,.12),(1.15,2.4,.24),stone)
    cube('altar',(-3.1,0,.62),(.80,1.92,1.04),stone)
    cube('altar slab',(-3.1,0,1.18),(1.02,2.18,.16),stone)
    # Low surrounding courses frame the floor without obscuring the figures.
    for y in (-4,4):
        cube('chapel wall',(-.2,y,.48),(8.5,.30,.96),stone)
        for x in (-3.8,0,3.8):
            cube('chapel timber post',(x,y,1.12),(.21,.25,2.24),wood)
    cube('rear masonry',(-4.4,0,.8),(.32,8,1.6),stone)
    for y in (-2.8,2.8):
        bpy.ops.object.light_add(type='POINT',location=(-3.65,y,1.92))
        lamp=bpy.context.object;lamp.data.energy=160;lamp.data.color=(1,.52,.21);lamp.data.shadow_soft_size=.08
        cube('lamp backing',(-4.19,y,1.85),(.18,.23,.35),wood)
    cam.data.type='PERSP';cam.data.angle=math.radians(45)
    target=Vector((.0,0,.90))
    # Exact 12 m arm, 55 degree pitch, 45 degree yaw used by the game camera.
    cam.location=target+Vector((12*math.cos(math.radians(55))/math.sqrt(2),-12*math.cos(math.radians(55))/math.sqrt(2),12*math.sin(math.radians(55))))
    aim(cam,target)
    render('chapel_gameplay')

def gestures():
    clear()
    names=['NPC.BrotherKiran','NPC.Fali','NPC.Murmuntag']
    for col,npc in enumerate(names):
        objects,rig,actions=load(npc)
        for band,(clip,frame) in enumerate([('idle',0),('talk',30)]):
            freeze(objects,rig,actions,clip,frame,pos=(band*4.2,(col-1)*2.6,0),yaw=-.6)
        remove(objects)
    cam=lighting(lineup=True);cam.data.type='ORTHO';cam.data.ortho_scale=11.4
    target=Vector((2.1,0,.9));cam.location=target+Vector((12,0,8));aim(cam,target)
    label(cam,'GESTURE REVIEW  /  IDLE ABOVE, TALK BELOW',-5.35,2.91,.18)
    for name,x in [('KIRAN',-2.85),('FALI',-.16),('MURMUNTAG',2.17)]:
        label(cam,name,x,-2.86,.13)
    render('gestures')


if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--view',choices=('hero','lineup','chapel','gestures','all'),default='all');p.add_argument('--only')
    args=p.parse_args(sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else [])
    rows=json.loads((HERE/'roster.json').read_text())['npcs']
    if args.view in ('hero','all'):
        for row in rows:
            if not args.only or row['id'].removeprefix('NPC.')==args.only:hero(row)
    if args.view in ('lineup','all'):lineup(rows)
    if args.view in ('chapel','all'):chapel()
    if args.view in ('gestures','all'):gestures()
