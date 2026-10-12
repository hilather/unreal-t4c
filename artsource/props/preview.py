"""CPU Cycles export review: icons, item/effect sheets and actual source-rig sockets."""
import argparse
import importlib.util
import json
import math
import struct
import sys
import tempfile
from pathlib import Path
import bpy
from mathutils import Vector,Matrix
sys.dont_write_bytecode=True
sys.path.insert(0,str(Path(__file__).resolve().parent))
import build,models
ROOT=Path(__file__).resolve().parent
DIR=Vector((5,-8,4.8)).normalized()
ICON_ROWS=[('Item.'+n,n) for n in models.ITEMS]+[('Spell.'+n,n) for n in ('Light','FireDart','HealLight','StoneShard','DustDevil')]+[('Attack.Melee.Basic','SlashArc'),('Attack.Ranged.Bow','AshwoodFlatbow')]


def aim(obj,target):obj.rotation_euler=(Vector(target)-obj.location).to_track_quat('-Z','Y').to_euler()

def stage(res=(128,128),transparent=True,samples=32):
    s=bpy.context.scene;s.render.engine='CYCLES';s.cycles.device='CPU';s.cycles.samples=samples;s.cycles.seed=523;s.cycles.use_denoising=True
    s.render.resolution_x,s.render.resolution_y=res;s.render.resolution_percentage=100
    s.render.image_settings.file_format='PNG';s.render.image_settings.color_mode='RGBA';s.render.image_settings.color_depth='8'
    s.render.film_transparent=transparent;s.view_settings.view_transform='AgX';s.view_settings.look='AgX - Medium High Contrast';s.view_settings.exposure=0
    s.world.use_nodes=True;p=s.world.node_tree.nodes.get('Background');p.inputs['Color'].default_value=(.13,.16,.19,1);p.inputs['Strength'].default_value=.32
    for loc,power,color,size in [((3,-4,6),700,(1,.83,.65),4),((-4,-2,3),400,(.61,.75,1),4),((1,4,5),900,(1,.77,.48),3)]:
        bpy.ops.object.light_add(type='AREA',location=loc);o=bpy.context.object;o.data.energy=power;o.data.color=color;o.data.shape='DISK';o.data.size=size;aim(o,(0,0,0))
    bpy.ops.object.camera_add(location=DIR*7);cam=bpy.context.object;cam.data.type='ORTHO';cam.data.ortho_scale=1.42;aim(cam,(0,0,0));s.camera=cam
    return cam

def load(output,name):
    prior=set(bpy.data.objects);bpy.ops.import_scene.gltf(filepath=str(output/(name+'.glb')))
    return [o for o in bpy.data.objects if o not in prior and o.type=='MESH']

def center_scale(objects,cam,width=1.12,center=Vector((0,0,0))):
    bpy.context.view_layer.update();basis=cam.rotation_euler.to_matrix().inverted()
    points=[basis@(o.matrix_world@Vector(v)) for o in objects for v in o.bound_box]
    lo=Vector([min(p[a] for p in points) for a in range(3)]);hi=Vector([max(p[a] for p in points) for a in range(3)])
    scale=width/max(hi.x-lo.x,hi.y-lo.y);mid=cam.rotation_euler.to_matrix()@((lo+hi)*.5)
    for o in objects:o.matrix_world=Matrix.Translation(center)@Matrix.Scale(scale,4)@Matrix.Translation(-mid)@o.matrix_world

def render(path):
    bpy.context.scene.render.filepath=str(path);bpy.ops.render.render(write_still=True)
    # Blender writes wall-clock/render timing into PNG ancillary metadata even
    # with burned-in stamps disabled. Remove metadata, retaining pixel chunks
    # and color profiles byte-for-byte; provenance belongs in the manifest.
    data=path.read_bytes();clean=bytearray(data[:8]);offset=8
    while offset<len(data):
        size=struct.unpack_from('>I',data,offset)[0];tag=data[offset+4:offset+8]
        if tag not in (b'tEXt',b'zTXt',b'iTXt',b'tIME',b'eXIf'):
            clean.extend(data[offset:offset+12+size])
        offset+=12+size
    path.write_bytes(clean)

def icons(output):
    folder=output/'icons';folder.mkdir(exist_ok=True);rows=[]
    for identity,name in ICON_ROWS:
        build.clear_objects();objects=load(output,name)
        if name=='AshwoodFlatbow' and identity.startswith('Item.'):
            for o in list(objects):
                if o.name.startswith('NockedArrow'):objects.remove(o);bpy.data.objects.remove(o,do_unlink=True)
        if identity=='Attack.Melee.Basic':
            blade=load(output,'RustedDirk')
            for o in blade:o.rotation_euler.z=.65;o.location.z=.06
            objects.extend(blade)
        if name in ('RustedDirk','GoblinBlade','AshwoodFlatbow','IronKey','DecayingBatWings'):
            # Rotate the model in its image plane; camera and lighting stay fixed.
            angle=-45 if name=='AshwoodFlatbow' else 65 if name in ('RustedDirk','GoblinBlade') else 28
            rotation=Matrix.Rotation(math.radians(angle),4,DIR)
            for o in objects:o.matrix_world=rotation@o.matrix_world
        # Attack icon uses an arrow crossing its bow; item icon remains undrawn.
        cam=stage();bpy.context.scene.view_settings.exposure=.5;center_scale(objects,cam)
        if name=='SlashArc':
            # The planar arc is visible to the common oblique camera; no UI cross.
            pass
        path=folder/(identity+'.png');render(path);rows.append(dict(id=identity,**build.record(path,output)))
    manifest=json.loads((output/'manifest.json').read_text());manifest['icons']=rows
    (output/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')

def text_obj(body,loc,size,cam,color=(.55,.55,.55,1),align='CENTER'):
    curve=bpy.data.curves.new('Caption','FONT');curve.body=body;curve.align_x=align;curve.size=size
    obj=bpy.data.objects.new('Caption '+body,curve);bpy.context.collection.objects.link(obj);obj.location=Vector(loc)+(cam.rotation_euler.to_matrix()@Vector((0,0,2)));obj.rotation_euler=cam.rotation_euler
    mat=bpy.data.materials.get('Caption')
    if not mat:
        mat=bpy.data.materials.new('Caption');mat.use_nodes=True;p=mat.node_tree.nodes.get('Principled BSDF');p.inputs['Base Color'].default_value=color;p.inputs['Emission Color'].default_value=color;p.inputs['Emission Strength'].default_value=1
    obj.data.materials.append(mat)

def sheet(output,effects=False):
    build.clear_objects();cam=stage((1600,1000),False,32)
    names=models.EFFECTS if effects else models.ITEMS+models.PROPS
    cols=4 if effects else 6;rows=math.ceil(len(names)/cols);gap=2.0
    cam.data.ortho_scale=max(cols*gap+.5,(rows*gap+1.7)*1.6);cam.location=DIR*22;aim(cam,(0,0,0));rot=cam.rotation_euler.to_matrix();right=rot@Vector((1,0,0));up=rot@Vector((0,1,0))
    for o in bpy.context.scene.objects:
        if o.type=='LIGHT':o.location*=4;o.data.energy*=16;o.data.size*=4
    for i,name in enumerate(names):
        center=right*((i%cols-(cols-1)/2)*gap)+up*((rows-1)/2*gap-(i//cols)*gap)
        objects=load(output,name);center_scale(objects,cam,1.45,center)
        text_obj(name,center-up*.91,.102,cam)
    title='MESH EFFECTS / W5-23' if effects else 'LIGHTHAVEN / TRAVELLER\'S POSSESSIONS'
    text_obj(title,up*(rows*gap/2+.26),.20,cam,color=(.65,.58,.44,1))
    text_obj('CPU CYCLES  /  EXPORTED GLB  /  PROTOTYPE PRESENTATION',-up*(rows*gap/2+.48),.10,cam)
    render(output/('effects-sheet.png' if effects else 'items-sheet.png'))

def icon_plane(path,loc,scale):
    im=bpy.data.images.load(str(path),check_existing=True)
    bpy.ops.mesh.primitive_plane_add(size=scale,location=loc);o=bpy.context.object
    material=bpy.data.materials.new('Icon '+path.stem);material.use_nodes=True
    n=material.node_tree.nodes;l=material.node_tree.links;n.clear()
    tex=n.new('ShaderNodeTexImage');tex.image=im;tex.interpolation='Linear'
    emit=n.new('ShaderNodeEmission');l.new(tex.outputs['Color'],emit.inputs[0])
    transparent=n.new('ShaderNodeBsdfTransparent');mix=n.new('ShaderNodeMixShader');l.new(tex.outputs['Alpha'],mix.inputs[0]);l.new(transparent.outputs[0],mix.inputs[1]);l.new(emit.outputs[0],mix.inputs[2])
    out=n.new('ShaderNodeOutputMaterial');l.new(mix.outputs[0],out.inputs[0]);o.data.materials.append(material)

def icon_sheet(output):
    build.clear_objects();cam=stage((1400,1000),False,8)
    # One scene unit is one pixel; view transform Standard preserves rendered thumbnails.
    s=bpy.context.scene;s.view_settings.view_transform='Standard';s.view_settings.look='None'
    cam.location=(0,0,1000);cam.rotation_euler=(0,0,0);cam.data.ortho_scale=1400;cam.data.clip_end=2000
    s.world.node_tree.nodes.get('Background').inputs['Color'].default_value=(.012,.016,.020,1);s.world.node_tree.nodes.get('Background').inputs['Strength'].default_value=1
    for i,(identity,name) in enumerate(ICON_ROWS):
        x=(i%7-3)*192;y=310-(i//7)*195
        icon_plane(output/'icons'/(identity+'.png'),(x,y,0),128)
        text_obj(identity.replace('Attack.','').replace('Item.','').replace('Spell.',''),(x,y-83,1),13,cam)
        small_y=-300-(i//7)*62
        icon_plane(output/'icons'/(identity+'.png'),(x,small_y,0),36)
        text_obj(name,(x+26,small_y-3,1),10,cam,align='LEFT')
    text_obj('LIGHTHAVEN / ITEMS & ABILITIES',(0,453,1),23,cam)
    text_obj('128 px source  |  transparent RGBA  |  shared light rig',(0,418,1),13,cam)
    text_obj('36 px readability strip',(0,-250,1),16,cam)
    render(output/'icons-sheet.png')

def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path);mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod);return mod

def rig_review(output):
    build.clear_objects();player_path=ROOT.parent/'player'
    pm=module('player_models',player_path/'models.py');pa=module('player_animation',player_path/'animation.py')
    ps=module('player_stage',player_path/'stage.py')
    cam=stage((1600,1000),False,24)
    # Source-rig geometry, CPU shading, true gameplay camera per panel (12m /55deg /45yaw /45FOV).
    # Render the four panels in memory, then assemble them within Blender into one PNG.
    scratch=tempfile.TemporaryDirectory(prefix='rig-review-',dir=output)
    panels=[]
    for mode in ('dirk','bow','torch','quiver'):
        build.clear_objects();parts,bones=pm.create('player_a');rig=pa.make_rig(bones,parts)
        for o in parts:
            if o.get('category')=='Hair.Tied':o.hide_render=True
        # Neutral source skin is the same multiplier as the existing player preview.
        for material in bpy.data.materials:
            if material.name.startswith(('skin neutral','skin lip')):
                n=material.node_tree.nodes;l=material.node_tree.links;p=n.get('Principled BSDF')
                original=p.inputs['Base Color'].links[0].from_socket;mix=n.new('ShaderNodeMixRGB');mix.blend_type='MULTIPLY';mix.inputs[0].default_value=1
                mix.inputs[2].default_value=(.509,.270,.168,1);l.new(original,mix.inputs[1]);l.new(mix.outputs[0],p.inputs['Base Color'])
        clip='bow' if mode=='bow' else 'idle';frame=21 if mode=='bow' else 0
        pa._pose(rig,clip,frame,30 if clip=='bow' else 60);bpy.context.view_layer.update()
        attachments=[('WoodenArrows','Socket.Quiver.Back')]
        if mode=='dirk':attachments.append(('RustedDirk','Socket.Weapon.R'))
        if mode=='bow':attachments.append(('AshwoodFlatbow','Socket.Bow.L'))
        if mode=='torch':attachments.append(('Torch','Socket.Bow.L'))
        socket_report=[]
        for name,socket in attachments:
            objects=load(output,name);pb=rig.pose.bones[socket]
            rest=rig.data.bones[socket];placement=Matrix.Translation(rest.head_local)
            transform=rig.matrix_world@pb.matrix@rest.matrix_local.inverted()@placement
            for o in objects:o.matrix_world=transform@o.matrix_world
            if mode=='bow' and name=='AshwoodFlatbow':
                # Nocked arrow follows the authoritative right-hand socket, string center follows nock.
                arrow=next(o for o in objects if o.name.startswith('NockedArrow'))
                ap=rig.pose.bones['Socket.Arrow'];ar=rig.data.bones['Socket.Arrow']
                arrow.matrix_world=rig.matrix_world@ap.matrix@ar.matrix_local.inverted()@Matrix.Translation(ar.head_local)@Matrix.Translation((.19,.035,0))
                # Static exported strings depict rest; deform midpoint only in this review pose.
                world_nock=arrow.matrix_world@Vector((-.19,-.035,0))
                for o in objects:
                    if o.name.startswith('String.'):
                        local=o.matrix_world.inverted()@world_nock
                        for v in o.data.vertices:
                            weight=max(0,1-abs(v.co.z)/.72);v.co+=weight*(local-Vector((-.19,0,0)))
            socket_report.append({'asset':name,'socket':socket,'head_m':list(pb.head)})
        # Repeatable gameplay stage from existing player kit.
        ps.stage(gameplay=True);scene=bpy.context.scene;scene.cycles.samples=24;scene.cycles.seed=523
        scene.render.resolution_x=800;scene.render.resolution_y=500;scene.render.resolution_percentage=100
        scene.render.film_transparent=False;scene.render.image_settings.color_mode='RGBA'
        cam=scene.camera;cam.data.type='PERSP';cam.data.angle=math.radians(45)
        pitch=math.radians(55);yaw=math.radians(45)+(math.pi if mode=='quiver' else 0)
        target=Vector((0,0,.85));cam.location=target+Vector((math.cos(pitch)*math.cos(yaw),math.cos(pitch)*math.sin(yaw),math.sin(pitch)))*12
        aim(cam,target)
        path=Path(scratch.name)/(mode+'.exr')
        scene.render.image_settings.file_format='OPEN_EXR';scene.render.image_settings.color_depth='32';scene.render.filepath=str(path)
        bpy.ops.render.render(write_still=True);panels.append(path)
    # Color managed writing through render_result images is handled by the compositor in this scene.
    # Store each linear in-memory render on a temporary image and compose planes without disk PNGs.
    build.clear_objects();cam=stage((1600,1000),False,8);cam.location=(0,0,1000);cam.rotation_euler=(0,0,0);cam.data.type='ORTHO';cam.data.ortho_scale=1600;cam.data.clip_end=2000
    for i,path in enumerate(panels):
        im=bpy.data.images.load(str(path));assert tuple(im.size)==(800,500)
        bpy.ops.mesh.primitive_plane_add(size=1,location=((-400 if i%2==0 else 400),(250 if i<2 else -250),0));o=bpy.context.object;o.scale=(800,500,1)
        material=bpy.data.materials.new('Rig panel');material.use_nodes=True;n=material.node_tree.nodes;l=material.node_tree.links;tex=n.new('ShaderNodeTexImage');tex.image=im
        emit=n.new('ShaderNodeEmission');l.new(tex.outputs['Color'],emit.inputs[0]);l.new(emit.outputs[0],n.get('Material Output').inputs[0]);o.data.materials.append(material)
        text_obj(('DIRK / idle','BOW / draw','TORCH / idle','QUIVER / reverse view')[i],(o.location.x,o.location.y-215,1),17,cam)
    render(output/'rig-gameplay.png')
    scratch.cleanup()

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--output',type=Path,default=build.OUT);p.add_argument('--view',choices=['all','icons','items','effects','icon-sheet','rig'],default='all')
    args=p.parse_args(sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else []);output=args.output.resolve()
    if args.view in ('all','icons'):icons(output)
    if args.view in ('all','items'):sheet(output)
    if args.view in ('all','effects'):sheet(output,True)
    if args.view in ('all','icon-sheet'):icon_sheet(output)
    if args.view in ('all','rig'):rig_review(output)
