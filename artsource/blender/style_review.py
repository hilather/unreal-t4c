"""Cycles CPU art reviews from actual exported style GLBs; not Unreal captures.

The mock layouts are deliberately small art studies, not authoritative map plans.
No special render-only surface materials, scenery meshes or concept inputs.
"""
import argparse
import math
from pathlib import Path
import sys

import bpy
from mathutils import Vector

sys.path.insert(0, str(Path(__file__).resolve().parent))
from build_env import camera, light, configure_render, bounds, neutral_ground


def _clear_scene_except(pieces):
    keep=set(pieces.values())
    for obj in list(bpy.data.objects):
        if obj not in keep:
            bpy.data.objects.remove(obj,do_unlink=True)
    for obj in keep:
        obj.hide_render=True


def _stage(pieces):
    placed=[]
    def put(name, location, yaw=0, scale=(1,1,1)):
        obj=bpy.data.objects.new('Study_'+name, pieces[name].data)
        bpy.context.collection.objects.link(obj)
        obj.location=location
        obj.rotation_euler.z=math.radians(yaw)
        obj.scale=scale
        placed.append(obj)
        return obj
    return put,placed


def room(style,pieces,directory,tag):
    _clear_scene_except(pieces)
    put,placed=_stage(pieces)
    church=style=='Church'
    width=8 if church else 12
    depth=12 if church or style=='B4Ritual' else 8
    for x in range(0,width,4):
        for y in range(0,depth,4): put('Floor400',(x,y,0))
    for x in range(0,width,4):
        if (style=='B2Damp' and x==0) or (style=='B3Crypt' and x==8):
            continue  # leave a real opening for the arch, not a wall behind it
        put('Wall400',(x,depth,0))
    for y in range(0,depth,4): put('Wall400',(0,y,0),90)
    # The entry edge and right side are cut away, like the game's visual camera.
    for x in range(0,width,4):
        if x!=4: put('Wall400',(x,0,0),scale=(1,1,.22))
    for y in range(0,depth,4): put('Wall400',(width,y,0),90,scale=(1,1,.22))

    def torch(x,y,z,yaw,power):
        put('Sconce',(x,y,z),yaw)
        a=math.radians(yaw)
        light('Torch',(x-.36*math.sin(a),y+.36*math.cos(a),z+.16),power,
              (1,.53,.24),radius=.12)

    if church:
        put('Carpet',(4,5.65,.003),scale=(.56,.46,1))
        put('Floor400',(2,9.8,.20),scale=(1,.50,1))
        put('Altar',(4,10.8,.20))
        for y in (3.1,5.4,7.7):
            for x in (1.7,6.3): put('Bench',(x,y,.006),scale=(1.25,1,1))
        for y in (2.2,6.4,10.7):
            for x in (.36,7.64): put('Pillar',(x,y,0))
        put('Arch320',(2,0,0),scale=(.8,1,1))
        # Leaf set partly open on its hinge, outside the clear central aisle.
        put('DoorLeafPreview',(.75,.02,0),62,scale=(.76,1,1))
        put('Table',(1.0,10.6,0),90,scale=(.8,.8,1))
        put('Barrel',(.75,1.5,0))
        for x in (1,7): torch(x,11.78,2.6,180,95)
        torch(.22,5,2.4,-90,65)
        light('Afternoon through open roof',(7,-1,9),1500,(1,.83,.61),
              area=7,target=(3,6,0))
        light('Soft sky',(-4,7,9),650,(.65,.76,1),area=6,target=(4,6,1))
        light('Front reflection',(10,-5,4),260,(1,.86,.65),area=7,target=(4,6,1))
        camera((20,-22,23),(4,5.8,1.25),24)
        world=(.17,.15,.12,1); strength=.16; exposure=.65
    elif style=='B2Damp':
        put('Arch320',(2,depth,0))
        # Small raised landing against damp retaining stone.
        put('Floor400',(.20,5.3,.65),scale=(.7,.60,1))
        put('Stair600x120',(1.6,1.4,0),90,scale=(.64,.82,.54))
        put('Wall400',(2.85,5.3,0),90,scale=(.6,1,.16))
        for x,y in ((.65,.7),(.7,1.7),(10.7,7),(11.25,6.05)):
            put('Barrel',(x,y,0),yaw=x*21)
        for x,y,z in ((8.8,7.2,0),(9.65,7.1,0),(9.55,7.15,.8)):
            put('Crate',(x,y,z),yaw=x*3)
        put('Table',(6.2,7.2,0))
        put('Bench',(6.2,6.25,0),-8)
        for x,y in ((.7,3),(4.4,7.4),(10.3,6.5),(10.8,1.2),(3.5,.7)):
            put('Debris',(x,y,.003),yaw=x*44,scale=(1.7,1.2,1))
        torch(.22,2.8,2.35,-90,150)
        torch(6.6,7.78,2.5,180,175)
        torch(10.2,7.78,2.4,180,120)
        light('Damp cool bounce',(6,2,9),520,(.48,.68,.66),area=8,target=(6,4,0))
        light('Pool glint',(11,-2,5),480,(.60,.78,.84),area=5,target=(7,3,0))
        light('Reflected high opening',(-5,18,12),110,(.62,.76,.82),area=5,target=(6,4,0))
        camera((22,-22,23),(5.9,3.8,1.1),24)
        world=(.055,.10,.095,1); strength=.10; exposure=.85
    elif style=='B3Crypt':
        put('Arch240',(10,8,0))
        put('Table',(.9,6.8,0),90)
        put('Bench',(2.1,6.8,0),90)
        put('Stair600x120Descending',(9,1.3,0),-90,scale=(.4,.78,.45))
        put('Crate',(.7,.8,0),12)
        for x,y in ((.65,4.5),(1.2,5.1),(3.6,7.6),(6.4,7.4),(11,6.4),(10.7,1.1)):
            put('Debris',(x,y,.002),yaw=x*17,scale=(1.8,1.2,.6))
        torch(.22,3.0,2.35,-90,120)
        torch(4.05,7.78,2.60,180,120)
        torch(8.02,7.78,2.60,180,100)
        light('Dusty cold skylight',(5,1,10),920,(.64,.72,.85),area=8,target=(6,5,1))
        light('Limestone bounce',(11,-4,6),380,(.86,.84,.76),area=8,target=(5,4,1))
        camera((22,-22,23),(5.9,3.8,1.1),24)
        world=(.12,.14,.18,1); strength=.14; exposure=.65
    else:
        # Boss arch replaces one wall region in the study; do not stack it on a wall.
        for obj in list(placed):
            if obj.name.startswith('Study_Wall400') and obj.location.y==12:
                placed.remove(obj); bpy.data.objects.remove(obj,do_unlink=True)
        put('Wall400',(0,12,0),scale=(.75,1,1))
        put('Wall400',(9,12,0),scale=(.75,1,1))
        put('ArchBoss500',(6,12,0))
        put('Floor400',(4,9.2,.55),scale=(1,.65,1))
        put('Stair600x120',(6,6,0),90,scale=(.55,.88,.46))
        put('Altar',(6,10.5,.55))
        for x,y in ((.7,1.7),(1.4,2.1),(11.3,6.3),(10.9,7.1),(2.2,11.1)):
            put('Debris',(x,y,.005),yaw=y*38,scale=(1.4,1.6,1))
        put('Table',(.8,9.8,0),90)
        put('Crate',(.65,7.9,0),8)
        for x in (3.25,8.75): torch(x,11.78,2.8,180,230)
        torch(.22,4,2.5,-90,200)
        light('Basalt edge light',(8,1,10),880,(.53,.63,.82),area=8,target=(6,6,0))
        light('Ember bounce',(6,-4,6),410,(1,.59,.36),area=7,target=(6,5,0))
        camera((24,-25,27),(5.9,5.6,1.25),28)
        world=(.07,.08,.12,1); strength=.09; exposure=1.0
    scene=bpy.context.scene
    scene.world.node_tree.nodes['Background'].inputs[0].default_value=world
    scene.world.node_tree.nodes['Background'].inputs[1].default_value=strength
    scene.view_settings.exposure=exposure
    scene.render.filepath=str(directory/(style+'_room_'+tag+'.png'))
    bpy.ops.render.render(write_still=True)
    return len(placed)


def details(style,pieces,directory,tag):
    _clear_scene_except(pieces)
    put,placed=_stage(pieces)
    # Close material/shape study at human scale. All objects are exported assets.
    put('Floor400',(0,0,0))
    put('Wall400',(0,4,0))
    if style=='Church':
        put('Pillar',(.32,3.15,0))
        put('Altar',(2.5,2.75,0),scale=(.83,.83,.83))
        put('Bench',(1.3,.6,0))
        put('DoorLeafPreview',(4.2,2.0,0),20,scale=(.7,1,.85))
    elif style=='B2Damp':
        put('Barrel',(.55,3.15,0))
        put('Crate',(1.4,3.4,0),12)
        put('Debris',(3.3,3.1,.002),15,scale=(1.3,1.5,1))
    elif style=='B3Crypt':
        put('Debris',(.6,3.2,.002),24,scale=(1.6,1.2,.7))
        put('Bench',(3.0,.8,0))
    else:
        put('Altar',(2.0,3.0,0),scale=(.77,.77,.77))
        put('Debris',(.8,.6,.002),23)
    put('Sconce',(.20,3.77,2.50),180)
    light('Material key',(2,-3,7),820,(1,.82,.63),area=5,target=(2,2,1))
    light('Material sky',(-3,4,6),600,(.61,.74,1),area=4,target=(2,2,1))
    light('Material torch',(.2,3.4,2.8),70,(1,.50,.20))
    if style=='B2Damp':
        light('Wet surface reflection',(-4,11,7),140,(.70,.82,1),area=4,target=(2,2,0))
    camera((9,-11,9),(2,2,1.85),12.1)
    scene=bpy.context.scene
    scene.view_settings.exposure=.6
    scene.world.node_tree.nodes['Background'].inputs[0].default_value=(.10,.12,.15,1)
    scene.world.node_tree.nodes['Background'].inputs[1].default_value=.14
    scene.render.filepath=str(directory/(style+'_details_'+tag+'.png'))
    bpy.ops.render.render(write_still=True)
    return len(placed)


def render_style(style,pieces,directory,tag):
    directory=Path(directory)
    directory.mkdir(parents=True,exist_ok=True)
    count=room(style,pieces,directory,tag)
    details(style,pieces,directory,tag)
    _clear_scene_except(pieces)
    return count


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source',type=Path,default=Path('Saved/ArtExport/env'))
    parser.add_argument('--styles',nargs='+',default=['Church','B2Damp','B3Crypt','B4Ritual'])
    parser.add_argument('--samples',type=int,default=16)
    parser.add_argument('--tag',default='final')
    parser.add_argument('--room-only',action='store_true')
    parser.add_argument('--details-only',action='store_true')
    opt=parser.parse_args(sys.argv[sys.argv.index('--')+1:])
    for style in opt.styles:
        bpy.ops.wm.read_factory_settings(use_empty=True)
        if bpy.context.scene.world is None:
            bpy.context.scene.world=bpy.data.worlds.new('World')
        configure_render(opt.samples)
        pieces={}
        source=opt.source/style
        for path in sorted(source.glob('*.glb')):
            before=set(bpy.data.objects)
            bpy.ops.import_scene.gltf(filepath=str(path.resolve()))
            meshes=[o for o in set(bpy.data.objects)-before if o.type=='MESH']
            assert len(meshes)==1,path
            pieces[path.stem]=meshes[0]
            meshes[0].hide_render=True
        assert pieces,source
        directory=source/'renders'; directory.mkdir(exist_ok=True)
        if not opt.details_only: room(style,pieces,directory,opt.tag)
        if not opt.room_only: details(style,pieces,directory,opt.tag)
    print('W5_14_REVIEW_COMPLETE')


if __name__=='__main__': main()
