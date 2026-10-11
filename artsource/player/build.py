"""W5-12 source construction, CPU Cycles atlas bake and sampled GLB export.
Bake/export wiring is adapted from ../creatures/build.py, kept local to this task.
"""
import argparse
import hashlib
import json
import sys
from array import array
from pathlib import Path
import bpy
import bmesh
sys.path.insert(0,str(Path(__file__).resolve().parent))
import models
import animation
OUT=Path(__file__).resolve().parent/'output';OUT.mkdir(exist_ok=True)
SKINS={'LightWarm':'BD8E72','MediumWarm':'8B5A40','DeepWarm':'51362C'}

def linear(hexcolor):
    return tuple(((v/255+.055)/1.055)**2.4 if v/255>.04045 else v/255/12.92 for v in bytes.fromhex(hexcolor))

def clear():
    bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
    bpy.data.orphans_purge(do_local_ids=True,do_linked_ids=True,do_recursive=True)

def geometry(kind):
    clear();parts,bones=models.create(kind)
    for o in parts:
        g=o.vertex_groups.new(name='part_'+o['category']);g.add(list(range(len(o.data.vertices))),1,'REPLACE')
    bpy.ops.object.select_all(action='DESELECT')
    for o in parts:o.select_set(True)
    bpy.context.view_layer.objects.active=parts[0];bpy.ops.object.join();mesh=bpy.context.object;mesh.name='combined'
    bpy.ops.object.transform_apply(location=True,rotation=True,scale=True)
    mesh.data.calc_loop_triangles()
    reduction=mesh.modifiers.new('Budget reduction','DECIMATE');reduction.ratio=min(1,13800/len(mesh.data.loop_triangles))
    bpy.ops.object.modifier_apply(modifier=reduction.name)
    tri=mesh.modifiers.new('Triangles','TRIANGULATE');bpy.ops.object.modifier_apply(modifier=tri.name)
    mesh.data.validate(verbose=True)
    bm=bmesh.new();bm.from_mesh(mesh.data)
    degenerate=[face for face in bm.faces if face.calc_area()<1e-10]
    bmesh.ops.delete(bm,geom=degenerate,context='FACES')
    bm.to_mesh(mesh.data);bm.free();mesh.data.update()
    bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.mesh.normals_make_consistent(inside=False)
    bpy.ops.uv.smart_project(angle_limit=1.15,island_margin=.003,margin_method='FRACTION',area_weight=.5)
    bpy.ops.object.mode_set(mode='OBJECT')
    # Sharp local corners keep tangent space defined on narrow leather trims.
    # Correct source shading before normal baking, never patch exported data.
    mesh.data.calc_tangents()
    sharp=[p for p in mesh.data.polygons if any(mesh.data.loops[i].tangent.length<.9 for i in p.loop_indices)]
    for p in sharp:p.use_smooth=False
    mesh.data.free_tangents();mesh.data.update();mesh.data.calc_tangents()
    assert all(loop.tangent.length>.9 for loop in mesh.data.loops),'Undefined source tangent'
    mesh['tangent_corner_faces_flattened']=len(sharp)
    mesh.data.free_tangents()
    return mesh,bones

def split(mesh):
    result=[]
    for category in ('body','Hair.Cropped','Hair.Tied'):
        group=mesh.vertex_groups['part_'+category].index
        keep={v.index for v in mesh.data.vertices if any(g.group==group and g.weight>.99 for g in v.groups)}
        o=mesh.copy();o.data=mesh.data.copy();o.name=category;bpy.context.collection.objects.link(o)
        bm=bmesh.new();bm.from_mesh(o.data);bm.verts.ensure_lookup_table()
        bmesh.ops.delete(bm,geom=[v for v in bm.verts if v.index not in keep],context='VERTS');bm.to_mesh(o.data);bm.free();o.data.validate(verbose=True);o.data.update()
        for g in list(o.vertex_groups):
            if g.name.startswith('part_'):o.vertex_groups.remove(g)
        result.append(o)
    bpy.data.objects.remove(mesh,do_unlink=True)
    return result

def baked_material(name,images,tint=None):
    m=bpy.data.materials.new(name);m.use_nodes=True;m.use_backface_culling=False
    n=m.node_tree.nodes;l=m.node_tree.links;p=n.get('Principled BSDF')
    for ch,im in images.items():
        node=n.new('ShaderNodeTexImage');node.image=im
        if ch=='base':
            if tint:
                mix=n.new('ShaderNodeMix');mix.data_type='RGBA';mix.blend_type='MULTIPLY';mix.inputs[0].default_value=1;mix.inputs[7].default_value=(*tint,1)
                l.new(node.outputs['Color'],mix.inputs[6]);l.new(mix.outputs[2],p.inputs['Base Color'])
            else:l.new(node.outputs['Color'],p.inputs['Base Color'])
        elif ch=='normal':
            normal=n.new('ShaderNodeNormalMap');l.new(node.outputs['Color'],normal.inputs['Color']);l.new(normal.outputs[0],p.inputs['Normal'])
        else:
            sep=n.new('ShaderNodeSeparateColor');l.new(node.outputs['Color'],sep.inputs[0]);l.new(sep.outputs[1],p.inputs['Roughness']);l.new(sep.outputs[2],p.inputs['Metallic'])
            group=bpy.data.node_groups.get('glTF Material Output')
            if group is None:
                from io_scene_gltf2.blender.com.material_helpers import create_settings_group
                group=create_settings_group('glTF Material Output')
            occ=n.new('ShaderNodeGroup');occ.node_tree=group;l.new(sep.outputs[0],occ.inputs['Occlusion'])
    return m

def bake(mesh,kind):
    scene=bpy.context.scene;scene.render.engine='CYCLES';scene.cycles.device='CPU';scene.cycles.samples=12;scene.cycles.seed=51212
    scene.render.bake.use_selected_to_active=False;scene.render.bake.use_clear=True
    # Separate alternate hair during baking: no phantom overlap with Cropped.
    group=mesh.vertex_groups['part_Hair.Tied'].index
    offset=[v for v in mesh.data.vertices if any(g.group==group and g.weight>.99 for g in v.groups)]
    for v in offset:v.co.y+=.5
    skin_indices={i for i,m in enumerate(mesh.data.materials) if m.name.startswith(('skin neutral','skin lip'))}
    skin_faces=[f.index for f in mesh.data.polygons if f.material_index in skin_indices]
    images={};stats={}
    for channel in ('base','normal','orm'):
        im=bpy.data.images.new(kind+'_'+channel,width=512,height=512,alpha=False)
        if channel!='base':im.colorspace_settings.name='Non-Color'
        for m in mesh.data.materials:
            node=m.node_tree.nodes.new('ShaderNodeTexImage');node.image=im;m.node_tree.nodes.active=node
        restore=[]
        if channel in ('base','orm'):
            for m in mesh.data.materials:
                n=m.node_tree.nodes;l=m.node_tree.links;p=n.get('Principled BSDF');out=n.get('Material Output')
                previous=out.inputs['Surface'].links[0].from_socket
                e=n.new('ShaderNodeEmission')
                if channel=='base':l.new(p.inputs['Base Color'].links[0].from_socket,e.inputs['Color'])
                else:
                    ao=n.new('ShaderNodeAmbientOcclusion');ao.inputs['Distance'].default_value=.075;ao.samples=32
                    combine=n.new('ShaderNodeCombineColor');combine.mode='RGB';l.new(ao.outputs['AO'],combine.inputs[0])
                    combine.inputs[1].default_value=p.inputs['Roughness'].default_value;combine.inputs[2].default_value=p.inputs['Metallic'].default_value
                    l.new(combine.outputs[0],e.inputs['Color'])
                l.new(e.outputs[0],out.inputs['Surface']);restore.append((m,previous,out,e))
            bpy.ops.object.bake(type='EMIT',margin=3)
            for m,previous,out,e in restore:m.node_tree.links.new(previous,out.inputs['Surface']);m.node_tree.nodes.remove(e)
        else:bpy.ops.object.bake(type='NORMAL',margin=3)
        im.filepath_raw=str(OUT/(kind+'_'+channel+'.png'));im.file_format='PNG';im.save();images[channel]=im
        if channel=='orm':
            pixels=array('f',[0.0])*(512*512*4);im.pixels.foreach_get(pixels)
            occupied=[pixels[i] for i in range(0,len(pixels),4) if pixels[i+1]>.01]
            stats['geometric_ao_range']=[min(occupied),max(occupied)]
    for v in offset:v.co.y-=.5
    clothing=baked_material('ClothingAtlas',images)
    skin=baked_material('SkinTint',images,linear(SKINS['LightWarm']))
    mesh.data.materials.clear();mesh.data.materials.append(clothing);mesh.data.materials.append(skin)
    for face in mesh.data.polygons:face.material_index=1 if face.index in skin_faces else 0
    return stats

def source_tint(mesh):
    for m in mesh.data.materials:
        if m.name.startswith(('skin neutral','skin lip')):
            p=m.node_tree.nodes.get('Principled BSDF');l=m.node_tree.links;n=m.node_tree.nodes
            original=p.inputs['Base Color'].links[0].from_socket
            mix=n.new('ShaderNodeMixRGB');mix.blend_type='MULTIPLY';mix.inputs[0].default_value=1;mix.inputs[2].default_value=(*linear(SKINS['LightWarm']),1)
            l.new(original,mix.inputs[1]);l.new(mix.outputs[0],p.inputs['Base Color'])

def export(meshes,rig,kind):
    for t in rig.animation_data.nla_tracks:t.mute=False
    rig.animation_data.action=None
    bpy.ops.object.select_all(action='DESELECT');rig.select_set(True)
    for o in meshes:o.select_set(True);o.hide_render=False
    bpy.context.view_layer.objects.active=rig
    bpy.ops.export_scene.gltf(filepath=str(OUT/(kind+'.glb')),export_format='GLB',use_selection=True,export_animations=True,export_animation_mode='NLA_TRACKS',export_force_sampling=True,export_tangents=True,export_def_bones=False)
    for t in rig.animation_data.nla_tracks:t.mute=True

def build(kind,do_bake=True,do_actions=True):
    mesh,bones=geometry(kind);triangles=len(mesh.data.polygons)
    if not 8000<=triangles<=15000:raise ValueError(('triangle budget',kind,triangles))
    stats=bake(mesh,kind) if do_bake else {}
    if not do_bake:source_tint(mesh)
    meshes=split(mesh);rig=animation.make_rig(bones,meshes)
    actions=animation.make_actions(rig,meshes) if do_actions else {}
    if do_bake:export(meshes,rig,kind)
    report={'kind':kind,'triangles_all_variants':triangles,'triangles_by_part':{o.name:len(o.data.polygons) for o in meshes},'bones':len(bones),'bake':stats,'unit':'metres','front':'+X','up':'+Z','seed':51212,'texture_resolution':512}
    if do_bake:report['glb_bytes']=(OUT/(kind+'.glb')).stat().st_size
    (OUT/(kind+'-build.json')).write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report))
    return meshes,rig,actions

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--only',choices=('player_a','player_b'));p.add_argument('--geometry-only',action='store_true')
    args=p.parse_args(sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else [])
    for kind in (args.only,) if args.only else ('player_a','player_b'):build(kind,not args.geometry_only)
