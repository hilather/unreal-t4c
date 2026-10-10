"""Run with Blender --background --factory-startup --python this_file -- [options]."""
import argparse
import hashlib
import json
import math
import struct
from pathlib import Path
import sys

import bpy
from mathutils import Vector

sys.path.insert(0, str(Path(__file__).resolve().parent))
from materials import bake_materials, export_piece
from geometry import PIECES, build_piece

PREFIX = 'Presentation.Environment.Shared.'
REVISION = 'W5-07b-v1'


def canonical_bounds(name):
    if name == 'Wall400': return ((0,-.2,0),(4,0,4))
    if name == 'Floor400': return ((0,0,-.2),(4,4,0))
    if name.startswith('Arch'): return ((-2,-.2,0),(2,0,4))
    if name.startswith('Stair'):
        descending = name.endswith('Descending')
        angle = math.atan2(-1.2 if descending else 1.2,6)
        dx,dz = .2*math.sin(angle),-.2*math.cos(angle)
        return ((min(0,dx),-1.5,(-1.2 if descending else 0)+dz),
                (6+max(0,dx),1.5,0 if descending else 1.2))
    if name == 'Sconce': return ((-.125,-.06,-.2),(.125,.34,.26))
    if name == 'Barrel': return ((-.31,-.31,0),(.31,.31,1))
    if name == 'Crate': return ((-.41,-.41,0),(.41,.41,.8))
    if name == 'Table': return ((-.8,-.45,0),(.8,.45,.8))
    if name == 'Bench': return ((-.8,-.225,0),(.8,.225,.45))
    coords=[]
    for i in range(5):
        a=math.radians(i*27)
        for x in (-.08,.08):
            for y in (-.12,.12):
                coords.append((i*.18-.36+x*math.cos(a)-y*math.sin(a),
                               (i%2)*.20-.10+x*math.sin(a)+y*math.cos(a)))
    return ((min(p[0] for p in coords),min(p[1] for p in coords),0),
            (max(p[0] for p in coords),max(p[1] for p in coords),.12))


def bounds(obj):
    return [[min(v.co[a] for v in obj.data.vertices) for a in range(3)],
            [max(v.co[a] for v in obj.data.vertices) for a in range(3)]]


def finish_piece(obj, name):
    # Exact canonical envelope, with no compensating object scale or origin change.
    before=bounds(obj)
    target=canonical_bounds(name)
    for v in obj.data.vertices:
        for a in range(3):
            v.co[a]=target[0][a]+(v.co[a]-before[0][a])/(before[1][a]-before[0][a])*(target[1][a]-target[0][a])
    obj.data.update()
    obj.data.calc_loop_triangles()
    for actual, expected in zip(bounds(obj),target):
        assert all(abs(a-e)<1e-5 for a,e in zip(actual,expected)), name
    assert obj.location.length==0 and tuple(obj.scale)==(1,1,1)


def flame_material():
    mat=bpy.data.materials.new('flame')
    mat.use_nodes=True
    shader=mat.node_tree.nodes.get('Principled BSDF')
    shader.inputs['Base Color'].default_value=(1,.34,.025,1)
    shader.inputs['Emission Color'].default_value=(1,.21,.012,1)
    shader.inputs['Emission Strength'].default_value=5
    shader.inputs['Roughness'].default_value=.85
    return mat


def configure_render(samples):
    scene=bpy.context.scene
    scene.render.engine='CYCLES'
    scene.cycles.device='CPU'
    scene.cycles.samples=samples
    scene.cycles.use_denoising=True
    scene.cycles.seed=7507
    scene.cycles.use_animated_seed=False
    scene.cycles.max_bounces=6
    scene.render.resolution_x=1280
    scene.render.resolution_y=720
    scene.render.resolution_percentage=100
    scene.render.image_settings.file_format='PNG'
    scene.render.image_settings.color_mode='RGB'
    scene.render.image_settings.color_depth='8'
    scene.render.film_transparent=False
    scene.render.threads_mode='FIXED'
    scene.render.threads=8
    scene.view_settings.view_transform='AgX'
    scene.view_settings.look='AgX - Medium High Contrast'
    scene.view_settings.exposure=.65
    scene.world.use_nodes=True
    scene.world.node_tree.nodes['Background'].inputs[0].default_value=(.10,.14,.20,1)
    scene.world.node_tree.nodes['Background'].inputs[1].default_value=.22


def camera(location, target, scale):
    data=bpy.data.cameras.new('ReviewCamera')
    obj=bpy.data.objects.new('ReviewCamera',data)
    bpy.context.collection.objects.link(obj)
    obj.location=location
    obj.rotation_euler=(Vector(target)-obj.location).to_track_quat('-Z','Y').to_euler()
    data.type='ORTHO'
    data.ortho_scale=scale
    data.lens=50
    bpy.context.scene.camera=obj
    return obj


def light(name, location, power, color, radius=.18, area=0, target=(0,0,0)):
    data=bpy.data.lights.new(name,'AREA' if area else 'POINT')
    data.energy=power
    data.color=color
    if area: data.shape='DISK'; data.size=area
    else: data.shadow_soft_size=radius
    obj=bpy.data.objects.new(name,data)
    bpy.context.collection.objects.link(obj)
    obj.location=location
    if area: obj.rotation_euler=(Vector(target)-obj.location).to_track_quat('-Z','Y').to_euler()
    return obj


def neutral_ground():
    mat=bpy.data.materials.new('review_ground')
    mat.diffuse_color=(.055,.058,.06,1)
    mat.use_nodes=True
    mat.node_tree.nodes['Principled BSDF'].inputs['Base Color'].default_value=(.055,.058,.06,1)
    mat.node_tree.nodes['Principled BSDF'].inputs['Roughness'].default_value=.85
    bpy.ops.mesh.primitive_plane_add(size=200,location=(0,0,-.23))
    obj=bpy.context.object
    obj.name='ReviewGround'
    obj.data.materials.append(mat)
    return obj


def render_individuals(pieces, directory):
    ground=neutral_ground()
    cam=camera((8,-12,9),(0,0,1),8)
    key=light('Softbox',(2,-5,7),1400,(1,.79,.57),area=5)
    fill=light('CoolFill',(-4,1,6),900,(.57,.71,1),area=4)
    rim=light('Rim',(1,4,6),1100,(1,.9,.75),area=3)
    for name,obj in pieces.items():
        obj.hide_render=False
        lo,hi=bounds(obj)
        center=Vector([(a+b)/2 for a,b in zip(lo,hi)])
        size=max(hi[a]-lo[a] for a in range(3))
        direction=Vector((1.2,-1.8,1.25))
        if name=='Stair600x120': direction=Vector((-1.2,-1.8,1.25))
        if name=='Sconce': direction=Vector((1.0,1.8,.7))
        cam.location=center+direction*max(size,1)
        cam.rotation_euler=(center-cam.location).to_track_quat('-Z','Y').to_euler()
        inverse=cam.rotation_euler.to_quaternion().inverted()
        corners=[inverse@(Vector((x,y,z))-center) for x in (lo[0],hi[0]) for y in (lo[1],hi[1]) for z in (lo[2],hi[2])]
        span=[max(p[a] for p in corners)-min(p[a] for p in corners) for a in range(2)]
        cam.data.ortho_scale=max(span[0],span[1]*16/9)*1.16
        ground.location.z=lo[2]-.012
        bpy.context.scene.render.filepath=str(directory/(name+'.png'))
        bpy.ops.render.render(write_still=True)
        obj.hide_render=True
    for obj in (ground,cam,key,fill,rim): bpy.data.objects.remove(obj,do_unlink=True)


def render_room(pieces, directory, revision):
    placed=[]
    def put(name, location, yaw=0, scale=(1,1,1)):
        obj=bpy.data.objects.new('Room_'+name,pieces[name].data)
        bpy.context.collection.objects.link(obj)
        obj.location=location
        obj.rotation_euler.z=math.radians(yaw)
        obj.scale=scale
        placed.append(obj)
        return obj
    def torch(x,y,z,yaw=0,power=125):
        obj=put('Sconce',(x,y,z),yaw)
        a=math.radians(yaw)
        light('Amber torch',(x-.38*math.sin(a),y+.38*math.cos(a),z+.18),power,(1,.24,.035),radius=.11)
    # A concept-comparison diorama, NOT the registry-authored B1 gameplay topology.
    for x in (0,4,8):
        for y in (0,4,8): put('Floor400',(x,y,0))
    for x in (4,8): put('Wall400',(x,12,0),scale=(1,2.3,1))
    for y in (0,4,8): put('Wall400',(0,y,0),90,scale=(1,2.3,1))
    # Low cutaway foreground sides expose the floor; far walls retain canonical height.
    for x in (0,4): put('Wall400',(x,0,0),scale=(1,2.3,.26))
    for y in (0,4,8): put('Wall400',(12,y,0),90,scale=(1,2.3,.26))
    # Elevated rear-left landing with steps into the main chamber.
    put('Floor400',(.2,8,1.2),scale=(.75,1,1))
    put('Wall400',(3.0,8,0),90,scale=(1,1,.3))
    put('Stair600x120', (1.7,4,0),90,scale=(.6666667,1,1))
    put('Arch320',(2,12,1.2),scale=(1,2.3,.80))
    put('Floor400',(.2,12,1.2),scale=(.75,1,1))
    put('Wall400',(3.0,12,0),90,scale=(1,1,.3))
    put('Wall400',(0,12,1.2),90,scale=(1,2.3,.8))
    put('Wall400',(3.2,12,1.2),90,scale=(1,2.3,.8))
    # Lower right exit stair and a small dim landing beyond the main floor.
    put('Stair600x120Descending',(9.5,0,0),-90,scale=(.67,1,1))
    put('Floor400',(8,-6,-1.2),scale=(.75,1,1))
    put('Wall400',(8,-6,-1.2),90,scale=(1.5,1,.45))
    put('Wall400',(11,-6,-1.2),90,scale=(1.5,1,.45))
    put('Arch240',(9.5,-.1,0),scale=(1,1,.55))
    for x,y,yaw in ((4.5,11.1,0),(7,11.1,0),(11.1,7.9,90)):
        put('Table',(x,y,0),yaw)
    put('Bench',(4.5,10.3,0),-8)
    put('Bench',(10.2,5.7,0),66)
    put('Crate',(6.7,11.1,0),4,scale=(.65,.65,.65))
    for x,y,yaw in ((.70,1.5,0),(.75,2.4,20),(3.7,9.5,12),
                    (8.35,11.25,-12),(10.7,11,0),(11.3,10.2,22),(10.3,6.4,6)):
        put('Barrel',(x,y,0),yaw)
    for x,y,z,yaw in ((.65,3.5,0,3),(1.3,3.6,0,-6),(.75,3.6,.8,12),
                       (11,4,0,9),(10.1,4.3,0,-14),(11,4,.8,4),(5.8,11.1,0,5)):
        put('Crate',(x,y,z),yaw)
    import random
    rng=random.Random(7507)
    for i in range(54):
        side=i%4
        along=rng.uniform(.35,11.6)
        edge=rng.uniform(.15,.95)
        x,y=((edge,along),(along,12-edge),(12-edge,along),(along,edge))[side]
        # Keep the stair foot and central walking floor readable.
        if 0<x<3.3 and 3.8<y<8.3: continue
        put('Debris',(x,y,.005),rng.uniform(0,360),scale=(rng.uniform(.7,1.8),rng.uniform(.7,1.5),rng.uniform(.6,1.4)))
    for cx,cy in ((5.8,10.5),(10.4,9.3),(.8,3.1)):
        for i in range(13):
            a=rng.random()*math.tau
            r=rng.uniform(.1,1.15)
            put('Debris',(cx+math.cos(a)*r,cy+math.sin(a)*r,.01),rng.uniform(0,360),
                scale=(rng.uniform(.5,1.6),rng.uniform(.6,1.6),rng.uniform(.6,1.8)))
    torch(.48,2.5,2.0,-90,165)
    torch(.48,9.5,2.8,-90,160)
    torch(.48,14.4,3.1,-90,125)
    torch(4.3,11.52,2.2,180,185)
    torch(9.4,11.52,2.3,180,190)
    torch(8.2,-3.0,.65,-90,140)
    light('Cool air',(7,5,13),440,(.46,.61,.82),area=9,target=(6,6,0))
    light('Front bounce',(8,-5,7),220,(.68,.73,.82),area=8,target=(6,5,0))
    camera((23,-23,25),(5.5,4.3,1.1),29.2)
    bpy.context.scene.world.node_tree.nodes['Background'].inputs[1].default_value=.026
    bpy.context.scene.render.filepath=str(directory/('B1_room_'+revision+'.png'))
    bpy.ops.render.render(write_still=True)
    return len(placed)


def main():
    args=sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else []
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,default=Path('Saved/ArtExport/env'))
    parser.add_argument('--samples',type=int,default=24)
    parser.add_argument('--skip-renders',action='store_true')
    parser.add_argument('--room-only',action='store_true',help='Build/export everything, render only the room for art iteration')
    parser.add_argument('--render-piece',choices=PIECES,help='Build/export everything, render one named piece for art iteration')
    parser.add_argument('--render-tag',default='final')
    opt=parser.parse_args(args)
    output=opt.output.resolve()
    output.mkdir(parents=True,exist_ok=True)
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)
    configure_render(opt.samples)
    materials=bake_materials(output,512)
    materials['flame']=flame_material()
    pieces={}
    manifest={'revision':REVISION,'style':'B1Cellar','seed':7507,
              'blender_version':bpy.app.version_string,
              'units':'glTF meters; canonical dimensions below in Unreal centimeters',
              'axes':'Authoring X,Y,Z = canonical local X,Y,Z. glTF positions = (X,Z,-Y).',
              'collision':'None; retain existing Unreal colliders and navigation.',
              'normal_convention':'OpenGL tangent +Y; verify importer converts for Unreal.',
              'pieces':{}}
    for name in PIECES:
        obj=build_piece(name,materials)
        finish_piece(obj,name)
        pieces[name]=obj
        path=export_piece(obj,output/(name+'.glb'))
        lo,hi=bounds(obj)
        raw=path.read_bytes()
        gltf=json.loads(raw[20:20+struct.unpack_from('<I',raw,12)[0]])
        triangles=sum(gltf['accessors'][p['indices']]['count']//3 for m in gltf['meshes'] for p in m['primitives'])
        assert triangles==len(obj.data.loop_triangles), name
        descending=name.endswith('Descending')
        entry={'piece_id':PREFIX+name.replace('Descending',''),
               'variant':'descending' if descending else 'default',
               'file':path.name,'bytes':len(raw),'sha256':hashlib.sha256(raw).hexdigest(),
               'bounds_cm':[[round(v*100,6) for v in xyz] for xyz in (lo,hi)],
               'dimensions_cm':[round((hi[i]-lo[i])*100,6) for i in range(3)],
               'pivot_cm':[0,0,0],'triangles':triangles,
               'material_slots':len(gltf.get('materials',[])),
               'texture_files':sorted(i['uri'] for i in gltf.get('images',[]))}
        if name.startswith('Stair'): entry['run_width_signed_rise_cm']=[600,300,-120 if descending else 120]
        key=entry['piece_id']+(':descending' if descending else '')
        manifest['pieces'][key]=entry
        obj.hide_render=True
    files=sorted((output/'textures').glob('*.png'))
    manifest['textures']={str(p.relative_to(output)):{'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'resolution':[512,512]} for p in files}
    manifest['texture_bytes']=sum(p.stat().st_size for p in files)
    manifest['glb_bytes']=sum(e['bytes'] for e in manifest['pieces'].values())
    manifest['payload_bytes']=manifest['texture_bytes']+manifest['glb_bytes']
    assert manifest['payload_bytes']<15_000_000,manifest['payload_bytes']
    # Exercise Blender's independent importer against the actual shared-texture GLBs.
    imported_count=0
    for name,original in pieces.items():
        previous=set(bpy.data.objects)
        bpy.ops.import_scene.gltf(filepath=str(output/(name+'.glb')))
        imported=set(bpy.data.objects)-previous
        meshes=[o for o in imported if o.type=='MESH']
        assert len(meshes)==1, name
        restored=meshes[0]
        restored.data.calc_loop_triangles()
        assert len(restored.data.loop_triangles)==len(original.data.loop_triangles), name
        assert restored.location.length<1e-6 and (restored.scale-Vector((1,1,1))).length<1e-6, name
        assert all(abs(a-b)<2e-5 for xyz,expected in zip(bounds(restored),bounds(original)) for a,b in zip(xyz,expected)), name
        for mat in restored.data.materials:
            for node in mat.node_tree.nodes:
                if node.type=='TEX_IMAGE': assert node.image and node.image.size[:]==(512,512),name
        for obj in imported: bpy.data.objects.remove(obj,do_unlink=True)
        imported_count+=1
    manifest['blender_roundtrip_checked']=imported_count
    if not opt.skip_renders:
        renders=output/'renders'
        renders.mkdir(exist_ok=True)
        if opt.render_piece: render_individuals({opt.render_piece:pieces[opt.render_piece]},renders)
        else:
            if not opt.room_only: render_individuals(pieces,renders)
            manifest['mock_room_instances']=render_room(pieces,renders,opt.render_tag)
        manifest['render_settings']={'engine':'Cycles','device':'CPU','samples':opt.samples,'denoise':True,'width':1280,'height':720}
        manifest['renders']=sorted(p.name for p in renders.glob('*.png'))
    (output/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
    assert manifest['payload_bytes']+(output/'manifest.json').stat().st_size<=15_000_000
    print('B1_EXPORT_SUMMARY '+json.dumps({'pieces':len(pieces),'texture_bytes':manifest['texture_bytes'],'glb_bytes':manifest['glb_bytes'],'payload_bytes':manifest['payload_bytes']}))


if __name__=='__main__': main()
