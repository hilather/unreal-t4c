"""W5-16 deterministic modular NPC construction using the W5-12 player source.
Blender only. No player output files are read or written. Metres/+X front/+Z up.
"""
import argparse
import hashlib
import importlib.util
import json
import math
from pathlib import Path
import struct
import sys
sys.dont_write_bytecode = True
import bpy
import bmesh

HERE = Path(__file__).resolve().parent
OUT = HERE / 'output'
OUT.mkdir(exist_ok=True)
sys.path.insert(0, str(HERE))
import costumes
import materials

def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod

models = module('npc_player_models', HERE.parent / 'player/models.py')
animation = module('npc_player_animation', HERE.parent / 'player/animation.py')
SEED = 516
TEMPLATES = {}
BONES = {}
PALETTE = {}
IMAGES = {}


def clear():
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)


def normalize(parts, key, budget):
    """Join one reusable module, reduce/unwrap once; keep semantic slots/weights."""
    bpy.ops.object.select_all(action='DESELECT')
    for ob in parts:
        ob.select_set(True)
    bpy.context.view_layer.objects.active = parts[0]
    bpy.ops.object.join()
    ob = bpy.context.object
    ob.name = key
    ob.data.name = key
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    ob.data.calc_loop_triangles()
    if len(ob.data.loop_triangles) > budget:
        dec = ob.modifiers.new('module budget', 'DECIMATE')
        dec.ratio = budget / len(ob.data.loop_triangles)
        bpy.ops.object.modifier_apply(modifier=dec.name)
    tri = ob.modifiers.new('triangles', 'TRIANGULATE')
    bpy.ops.object.modifier_apply(modifier=tri.name)
    ob.data.validate()
    bm = bmesh.new()
    bm.from_mesh(ob.data)
    bmesh.ops.delete(bm, geom=[f for f in bm.faces if f.calc_area() < 1e-10], context='FACES')
    bm.to_mesh(ob.data)
    bm.free()
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.mesh.normals_make_consistent(inside=False)
    bpy.ops.uv.smart_project(angle_limit=1.15, island_margin=.004, area_weight=.5)
    bpy.ops.object.mode_set(mode='OBJECT')
    ob.data.update()
    ob.data.calc_tangents()
    for p in ob.data.polygons:
        if any(ob.data.loops[i].tangent.length < .9 for i in p.loop_indices):
            p.use_smooth = False
    ob.data.free_tangents()
    ob.data.update()
    ob.data.calc_tangents()
    assert all(l.tangent.length > .9 for l in ob.data.loops), key
    ob.data.free_tangents()
    # No modifier, reference image, constraint or procedural shader dependency.
    TEMPLATES[key] = dict(data=ob.data, groups=[g.name for g in ob.vertex_groups],
                          slots=[s.material.name.split('.')[-1] for s in ob.material_slots])
    bpy.data.objects.remove(ob, do_unlink=True)


def player_templates(body):
    parts, bones = models.create('player_' + body.lower())
    BONES[body] = bones
    semantic = {'linen':'Cloth', 'linen_edge':'Trim', 'pants':'Pants',
                'leather':'Leather', 'edge':'Edge', 'stitch':'Stitch', 'metal':'Metal',
                'skin':'Skin', 'lip':'Lip', 'crease':'Crease', 'hair':'Hair',
                'hair_light':'Hair', 'eye':'Eye', 'iris':'Iris', 'pupil':'Pupil'}
    old = {mat: semantic[key] for key, mat in models.M.items()}
    groups = {name: [] for name in ('Head','Body','Legs','Hair.Cropped','Hair.Tied')}
    for ob in parts:
        for slot in ob.material_slots:
            slot.material = PALETTE[old[slot.material]]
        if ob['category'].startswith('Hair.'):
            category = ob['category']
        else:
            used = {g.name for g in ob.vertex_groups}
            if used <= {'head','neck'}:
                category = 'Head'
            elif used and all(g.startswith(('thigh.','shin.','foot.')) for g in used):
                category = 'Legs'
            else:
                category = 'Body'
        groups[category].append(ob)
    for name, objects in groups.items():
        budget = {'Head':2100,'Body':3900,'Legs':1900,'Hair.Cropped':800,'Hair.Tied':1100}[name]
        normalize(objects, body + '.' + name, budget)


def fitted(co, body):
    x,y,z=co
    if body == 'B':
        y *= .89 if 1.12 < z < 1.51 else .95
        x *= .95
        z *= .955
    return x,y,z


def costume_template(body, name, is_extra=False):
    key = body + ('.Prop.' if is_extra else '.Outfit.') + name
    if key in TEMPLATES:
        return key
    models.M.clear()
    models.M.update(PALETTE)
    parts = costumes.create(models, '' if is_extra else name, [name] if is_extra else [])
    assert parts, name
    for ob in parts:
        for v in ob.data.vertices:
            v.co = fitted(v.co, body)
    normalize(parts, key, 2200 if name in ('priest','scholar') else 1200)
    return key


def instantiate(key, palette):
    template = TEMPLATES[key]
    ob = bpy.data.objects.new(key, template['data'])
    bpy.context.collection.objects.link(ob)
    for name in template['groups']:
        ob.vertex_groups.new(name=name)
    for slot, name in zip(ob.material_slots, template['slots']):
        slot.link = 'OBJECT'
        slot.material = palette[name]
    ob['mesh_key'] = key
    return ob


def make_actions(rig):
    scene = bpy.context.scene
    scene.render.fps = 30
    rig.animation_data_create()
    actions = {}
    for clip in ('idle','talk'):
        action = bpy.data.actions.new('NPC_' + clip)
        actions[clip] = action
        rig.animation_data.action = action
        for frame in range(61):
            scene.frame_set(frame)
            t = 0 if frame == 60 else frame / 60
            phase = math.tau * t
            envelope = math.sin(math.pi * t) ** 2
            for bone in rig.pose.bones:
                bone.location = (0,0,0)
                bone.rotation_quaternion = (1,0,0,0)
                bone.scale = (1,1,1)
            # Feet/pelvis/root and right-hand staff remain fixed in both clips.
            rig.pose.bones['chest'].scale = (1+.002*envelope,1+.003*envelope,1+.002*envelope)
            animation._rotate(rig, 'head', (0,0,1), .025*math.sin(phase))
            if clip == 'talk':
                animation._rotate(rig, 'upper_arm.L', (0,1,0), -.38*envelope)
                animation._rotate(rig, 'forearm.L', (0,1,0), -.82*envelope)
                animation._rotate(rig, 'hand.L', (1,0,0), .25*envelope*math.sin(phase))
                animation._rotate(rig, 'head', (0,1,0), .065*envelope*math.sin(phase*2))
            for bone in rig.pose.bones:
                for prop in ('location','rotation_quaternion','scale'):
                    bone.keyframe_insert(data_path=prop, frame=frame, group=bone.name)
        for layer in action.layers:
            for strip in layer.strips:
                for bag in strip.channelbags:
                    for curve in bag.fcurves:
                        for key in curve.keyframe_points:
                            key.interpolation = 'LINEAR'
        track = rig.animation_data.nla_tracks.new()
        track.name = clip
        track.strips.new(clip, 0, action)
        track.mute = True
    rig.animation_data.action = actions['idle']
    rig.animation_data.action_slot = actions['idle'].slots[0]
    scene.frame_set(0)
    bpy.context.view_layer.update()
    return actions


def assemble(row):
    palette = materials.palette(row['id'], row, IMAGES)
    body = row['body']
    keys = [body + '.' + part for part in ('Head','Body','Legs')]
    if row['hair_style'] != 'Bald':
        keys.append(body + '.Hair.' + row['hair_style'])
    keys.append(costume_template(body, row['outfit']))
    for prop in row['extras']:
        keys.append(costume_template(body, prop, True))
    meshes = [instantiate(key, palette) for key in keys]
    rig = animation.make_rig(BONES[body], meshes)
    rig.name = row['id']
    # Export at canonical scale. Blender also bakes ancestor scale into skins;
    # the standard glTF scene-parent scale is assigned after its export below.
    parent = bpy.data.objects.new(row['id'] + '.Stature', None)
    bpy.context.collection.objects.link(parent)
    rig.parent = parent
    rig['npc_id'] = row['id']
    rig['body_bind_pose'] = body
    actions = make_actions(rig)
    return meshes, rig, actions, keys


def export(row, meshes, rig):
    for track in rig.animation_data.nla_tracks:
        track.mute = False
    rig.animation_data.action = None
    bpy.ops.object.select_all(action='DESELECT')
    rig.select_set(True)
    if rig.parent:
        rig.parent.select_set(True)
    for ob in meshes:
        ob.select_set(True)
    bpy.context.view_layer.objects.active = rig
    file = row['id'] + '.glb'
    bpy.ops.export_scene.gltf(filepath=str(OUT/file), export_format='GLB', use_selection=True,
        export_animations=True, export_animation_mode='NLA_TRACKS',
        export_force_sampling=True, export_tangents=True, export_def_bones=False,
        export_optimize_animation_size=False, export_extras=True)
    if rig.parent:
        # Only a scene transform is assigned. Geometry, skins, inverse binds,
        # textures and animation bytes remain the unmodified Blender export.
        path=OUT/file
        blob=path.read_bytes()
        json_length,json_tag=struct.unpack_from('<II',blob,12)
        assert json_tag==0x4e4f534a
        document=json.loads(blob[20:20+json_length])
        parents=[n for n in document['nodes'] if n.get('name')==rig.parent.name]
        assert len(parents)==1 and 'matrix' not in parents[0]
        parents[0]['scale']=[row['scale']]*3
        encoded=json.dumps(document,separators=(',',':')).encode()
        encoded+=b' '*((-len(encoded))%4)
        remaining=blob[20+json_length:]
        total=20+len(encoded)+len(remaining)
        path.write_bytes(struct.pack('<III',0x46546c67,2,total)+struct.pack('<II',len(encoded),json_tag)+encoded+remaining)
    return file


def mesh_digest(data):
    digest = hashlib.sha256()
    for v in data.vertices:
        digest.update(struct.pack('<3f',*v.co))
        for group in v.groups:
            digest.update(struct.pack('<If',group.group,group.weight))
    for p in data.polygons:
        digest.update(struct.pack('<3I',*p.vertices))
    for uv in data.uv_layers.active.data:
        digest.update(struct.pack('<2f',*uv.uv))
    return digest.hexdigest()


def build(only=None):
    global IMAGES, PALETTE
    clear()
    IMAGES = materials.bake(OUT)
    roster = json.loads((HERE/'roster.json').read_text())['npcs']
    PALETTE = materials.palette('Template', roster[0], IMAGES)
    for body in ('A','B'):
        player_templates(body)
    results=[]
    for row in roster:
        if only and row['id'].removeprefix('NPC.') != only:
            continue
        meshes, rig, actions, keys = assemble(row)
        file = export(row, meshes, rig)
        tris = {key:len(ob.data.polygons) for key,ob in zip(keys,meshes)}
        result = dict(id=row['id'],file=file,body=row['body'],scale=row['scale'],
                      triangles=sum(tris.values()),triangles_by_module=tris,
                      glb_bytes=(OUT/file).stat().st_size,mesh_keys=keys,
                      textures=['NPC_Textile_base.png','NPC_Textile_normal.png'],
                      texture_dimensions=[512,512],actions={'idle':2,'talk':2})
        results.append(result)
        for ob in meshes + ([rig.parent] if rig.parent else []) + [rig]:
            bpy.data.objects.remove(ob,do_unlink=True)
        for action in actions.values():
            bpy.data.actions.remove(action)
        print('NPC_EXPORTED',json.dumps(result),flush=True)
    # Runtime import inventory: each geometry module and clip occurs once per
    # bind body, not once per named NPC. Overlapping outfits are intentional.
    kits=[]
    for body in ('A','B'):
        keys=sorted({key for row in results if row['body']==body for key in row['mesh_keys']})
        if not keys:
            continue
        meshes=[instantiate(key,PALETTE) for key in keys]
        rig=animation.make_rig(BONES[body],meshes)
        rig.name='NPC_Kit_'+body
        actions=make_actions(rig)
        file=export({'id':rig.name},meshes,rig)
        kits.append(dict(file=file,body=body,triangles=sum(len(o.data.polygons) for o in meshes),
                         glb_bytes=(OUT/file).stat().st_size,mesh_keys=keys))
        for ob in meshes+[rig]:
            bpy.data.objects.remove(ob,do_unlink=True)
        for action in actions.values():
            bpy.data.actions.remove(action)
    manifest=dict(task='W5-16', seed=SEED, blender=bpy.app.version_string,
                  blender_hash=bpy.app.build_hash.decode(),unit='metres',forward='+X',up='+Z',
                  npcs=results,kits=kits,modules={key:dict(triangles=len(t['data'].polygons),
                      geometry_sha256=mesh_digest(t['data']),slots=t['slots']) for key,t in TEMPLATES.items()},
                  textures={channel:dict(file=Path(im.filepath_raw).name,bytes=Path(im.filepath_raw).stat().st_size,
                      sha256=hashlib.sha256(Path(im.filepath_raw).read_bytes()).hexdigest()) for channel,im in IMAGES.items()},
                  skeletons={body:{name:dict(head=list(h),tail=list(t),parent=p) for name,(h,t,p) in bones.items()} for body,bones in BONES.items()})
    (OUT/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')

if __name__ == '__main__':
    parser=argparse.ArgumentParser()
    parser.add_argument('--only')
    args=parser.parse_args(sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else [])
    build(args.only)
