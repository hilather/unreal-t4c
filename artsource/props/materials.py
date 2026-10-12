"""Original worn pigment atlas; real CPU Cycles bakes, no reference pixels."""
from pathlib import Path
import bpy

PALETTE = [
    ('iron','525656',.64,.78), ('edge','9EAAA5',.38,.82),
    ('rust','794A2D',.91,.22), ('ashwood','735238',.79,0),
    ('leather','39291F',.85,0), ('linen','8D8063',.94,0),
    ('thread','CAB58A',.82,0), ('parchment','C5AF7B',.9,0),
    ('robe','222B32',.94,0), ('bone','958264',.76,0),
    ('mana','226F8C',.28,.25), ('membrane','554536',.95,0),
    ('stone','787970',.97,0), ('brass','9B7841',.5,.72),
    ('soot','151B1C',.96,0), ('seal','803E2D',.65,0),
]
INDEX={row[0]:i for i,row in enumerate(PALETTE)}

def linear(h):
    return tuple(((x/255+.055)/1.055)**2.4 if x>10 else x/255/12.92 for x in bytes.fromhex(h))

def source(name):
    old=bpy.data.materials.get('Source.'+name)
    if old:return old
    _,h,rough,metal=PALETTE[INDEX[name]]
    m=bpy.data.materials.new('Source.'+name);m.use_nodes=True
    n=m.node_tree.nodes;l=m.node_tree.links;p=n.get('Principled BSDF')
    p.inputs['Roughness'].default_value=rough;p.inputs['Metallic'].default_value=metal
    uv=n.new('ShaderNodeTexCoord');scale=n.new('ShaderNodeVectorMath');scale.operation='MULTIPLY'
    scale.inputs[1].default_value=(18,180,4) if name=='ashwood' else (100,100,30)
    l.new(uv.outputs['UV'],scale.inputs[0])
    noise=n.new('ShaderNodeTexNoise');noise.inputs['Scale'].default_value=1;noise.inputs['Detail'].default_value=3
    l.new(scale.outputs[0],noise.inputs['Vector'])
    ramp=n.new('ShaderNodeValToRGB');c=linear(h)
    ramp.color_ramp.elements[0].position=.18;ramp.color_ramp.elements[0].color=(*(x*.48 for x in c),1)
    ramp.color_ramp.elements[1].position=.82;ramp.color_ramp.elements[1].color=(*(min(1,x*1.32) for x in c),1)
    l.new(noise.outputs['Fac'],ramp.inputs[0]);l.new(ramp.outputs[0],p.inputs['Base Color'])
    bump=n.new('ShaderNodeBump');bump.inputs['Strength'].default_value=.24;bump.inputs['Distance'].default_value=.002
    l.new(noise.outputs['Fac'],bump.inputs['Height']);l.new(bump.outputs[0],p.inputs['Normal'])
    return m

def effect(name,h,strength=2,alpha=1):
    old=bpy.data.materials.get('FX.'+name)
    if old:return old
    m=bpy.data.materials.new('FX.'+name);m.use_nodes=True;m.diffuse_color=(*linear(h),alpha)
    p=m.node_tree.nodes.get('Principled BSDF');p.inputs['Base Color'].default_value=(*linear(h),1)
    p.inputs['Emission Color'].default_value=(*linear(h),1);p.inputs['Emission Strength'].default_value=strength
    p.inputs['Roughness'].default_value=.72;p.inputs['Alpha'].default_value=alpha
    m.use_backface_culling=False
    if alpha<1:m.surface_render_method='DITHERED'
    m['emissive_srgb']='#'+h;m['emission_strength']=strength;m['opacity']=alpha
    return m

def baked(images):
    m=bpy.data.materials.new('Props.WornAtlas');m.use_nodes=True;m.use_backface_culling=False
    n=m.node_tree.nodes;l=m.node_tree.links;p=n.get('Principled BSDF')
    for channel,im in images.items():
        t=n.new('ShaderNodeTexImage');t.image=im;t.interpolation='Linear';t.extension='EXTEND'
        if channel=='base':l.new(t.outputs['Color'],p.inputs['Base Color'])
        elif channel=='normal':
            norm=n.new('ShaderNodeNormalMap');l.new(t.outputs['Color'],norm.inputs['Color']);l.new(norm.outputs[0],p.inputs['Normal'])
        else:
            split=n.new('ShaderNodeSeparateColor');l.new(t.outputs['Color'],split.inputs[0]);l.new(split.outputs[1],p.inputs['Roughness']);l.new(split.outputs[2],p.inputs['Metallic'])
            from io_scene_gltf2.blender.com.material_helpers import create_settings_group
            group=bpy.data.node_groups.get('glTF Material Output') or create_settings_group('glTF Material Output')
            occ=n.new('ShaderNodeGroup');occ.node_tree=group;l.new(split.outputs[0],occ.inputs['Occlusion'])
    return m

def bake(output):
    output=Path(output);(output/'textures').mkdir(parents=True,exist_ok=True)
    verts=[];faces=[]
    for i in range(16):
        x=i%4;y=i//4;j=len(verts)
        verts.extend([(x,y,0),(x+1,y,0),(x+1,y+1,0),(x,y+1,0)]);faces.append((j,j+1,j+2,j+3))
    data=bpy.data.meshes.new('Atlas bake surface');data.from_pydata(verts,[],faces);data.update()
    obj=bpy.data.objects.new('Atlas bake surface',data);bpy.context.collection.objects.link(obj)
    uv=data.uv_layers.new(name='UVMap')
    for i,row in enumerate(PALETTE):
        data.materials.append(source(row[0]));data.polygons[i].material_index=i
        for loop in data.polygons[i].loop_indices:
            v=data.vertices[data.loops[loop].vertex_index].co;uv.data[loop].uv=(v.x/4,v.y/4)
    bpy.ops.object.select_all(action='DESELECT');obj.select_set(True);bpy.context.view_layer.objects.active=obj
    s=bpy.context.scene;s.render.engine='CYCLES';s.cycles.device='CPU';s.cycles.samples=16;s.cycles.seed=523
    s.render.bake.use_clear=True;s.render.bake.use_selected_to_active=False
    images={}
    for channel in ('base','normal','orm'):
        im=bpy.data.images.new('props_'+channel,width=256,height=256,alpha=False)
        if channel!='base':im.colorspace_settings.name='Non-Color'
        restore=[]
        for m in data.materials:
            n=m.node_tree.nodes;l=m.node_tree.links;p=n.get('Principled BSDF');out=n.get('Material Output')
            t=n.new('ShaderNodeTexImage');t.image=im;n.active=t
            if channel!='normal':
                prior=out.inputs['Surface'].links[0].from_socket;e=n.new('ShaderNodeEmission')
                if channel=='base':l.new(p.inputs['Base Color'].links[0].from_socket,e.inputs['Color'])
                else:
                    e.inputs['Color'].default_value=(1,p.inputs['Roughness'].default_value,p.inputs['Metallic'].default_value,1)
                l.new(e.outputs[0],out.inputs['Surface']);restore.append((m,prior,out,e))
        bpy.ops.object.bake(type='NORMAL' if channel=='normal' else 'EMIT',margin=0)
        for m,prior,out,e in restore:m.node_tree.links.new(prior,out.inputs['Surface']);m.node_tree.nodes.remove(e)
        im.filepath_raw=str(output/'textures'/('props_'+channel+'.png'));im.file_format='PNG';im.save();images[channel]=im
    bpy.data.objects.remove(obj,do_unlink=True)
    return baked(images)
