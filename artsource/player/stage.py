"""Review stage reused verbatim from W5-10c creatures/build.py at a3de69a.
Local copy avoids importing that asset builder and its output/cache side effects.
"""
import math
import bpy
from mathutils import Vector

def material(name, color, rough=.7, scale=70):
    m=bpy.data.materials.new(name); m.use_nodes=True
    n=m.node_tree.nodes; l=m.node_tree.links; p=n.get('Principled BSDF')
    p.inputs['Roughness'].default_value=rough
    noise=n.new('ShaderNodeTexNoise'); noise.inputs['Scale'].default_value=scale
    noise.inputs['Detail'].default_value=3
    ramp=n.new('ShaderNodeValToRGB')
    ramp.color_ramp.elements[0].color=(*[v*.22 for v in color],1)
    ramp.color_ramp.elements[1].color=(*color,1)
    l.new(noise.outputs['Fac'],ramp.inputs[0]); l.new(ramp.outputs[0],p.inputs['Base Color'])
    bump=n.new('ShaderNodeBump'); bump.inputs['Strength'].default_value=.22; bump.inputs['Distance'].default_value=.001
    l.new(noise.outputs['Fac'],bump.inputs['Height']); l.new(bump.outputs[0],p.inputs['Normal'])
    return m

def aim(o,target):o.rotation_euler=(Vector(target)-o.location).to_track_quat('-Z','Y').to_euler()

def gameplay_floor_material():
    """Prototype visual W5-08d: metre-scale slate slabs, procedural only."""
    name='Gameplay slate W5-08d'
    existing=bpy.data.materials.get(name)
    if existing:return existing
    m=bpy.data.materials.new(name);m.use_nodes=True
    n=m.node_tree.nodes;l=m.node_tree.links;p=n.get('Principled BSDF')
    p.inputs['Roughness'].default_value=.92
    coordinates=n.new('ShaderNodeTexCoord')
    brick=n.new('ShaderNodeTexBrick');brick.offset=.5;brick.offset_frequency=2
    brick.inputs['Scale'].default_value=1
    brick.inputs['Brick Width'].default_value=.7;brick.inputs['Row Height'].default_value=.5
    brick.inputs['Mortar Size'].default_value=.008;brick.inputs['Mortar Smooth'].default_value=.004
    brick.inputs['Color1'].default_value=(.012,.016,.020,1)
    brick.inputs['Color2'].default_value=(.025,.030,.035,1)
    brick.inputs['Mortar'].default_value=(.006,.008,.010,1)
    l.new(coordinates.outputs['Object'],brick.inputs['Vector'])
    noise=n.new('ShaderNodeTexNoise');noise.inputs['Scale'].default_value=18;noise.inputs['Detail'].default_value=2
    l.new(coordinates.outputs['Object'],noise.inputs['Vector'])
    variation=n.new('ShaderNodeValToRGB')
    variation.color_ramp.elements[0].color=(.78,.78,.78,1)
    variation.color_ramp.elements[1].color=(1,1,1,1)
    l.new(noise.outputs['Fac'],variation.inputs[0])
    tint=n.new('ShaderNodeMixRGB');tint.blend_type='MULTIPLY';tint.inputs[0].default_value=1
    l.new(brick.outputs['Color'],tint.inputs[1]);l.new(variation.outputs['Color'],tint.inputs[2]);l.new(tint.outputs[0],p.inputs['Base Color'])
    grain=n.new('ShaderNodeBump');grain.inputs['Strength'].default_value=.12;grain.inputs['Distance'].default_value=.001
    l.new(noise.outputs['Fac'],grain.inputs['Height'])
    grout=n.new('ShaderNodeBump');grout.invert=True;grout.inputs['Strength'].default_value=.25;grout.inputs['Distance'].default_value=.003
    l.new(brick.outputs['Fac'],grout.inputs['Height']);l.new(grain.outputs['Normal'],grout.inputs['Normal']);l.new(grout.outputs['Normal'],p.inputs['Normal'])
    return m

def stage(kind=None, gameplay=False):
    s=bpy.context.scene;s.render.engine='CYCLES';s.cycles.device='CPU'
    s.render.resolution_x=1280;s.render.resolution_y=720;s.render.resolution_percentage=100;s.cycles.samples=12;s.cycles.use_denoising=True
    s.view_settings.look='AgX - Medium High Contrast'
    # Swap only tagged staging; geometry and the existing evidence camera survive.
    for o in list(s.objects):
        if o.get('creature_preview_stage'):bpy.data.objects.remove(o,do_unlink=True)
    s.world.use_nodes=True
    background=s.world.node_tree.nodes.get('Background')
    background.inputs['Color'].default_value=(.12,.17,.25,1) if gameplay else (.018,.018,.018,1)
    background.inputs['Strength'].default_value=.1 if gameplay else 1
    bpy.ops.mesh.primitive_plane_add(size=200);floor=bpy.context.object
    floor.name='Gameplay stone floor' if gameplay else 'Studio floor';floor['creature_preview_stage']=True
    if gameplay:floor.data.materials.append(gameplay_floor_material())
    else:
        studio=bpy.data.materials.get('Studio slate W5-08d') or material('Studio slate W5-08d',(.014,.017,.020),.9,8)
        floor.data.materials.append(studio)
    if gameplay:
        # Prototype visual W5-08d: warm local lamp with restrained cool fill.
        bpy.ops.object.light_add(type='POINT',location=(1.5,-1.4,2.4));o=bpy.context.object
        o.name='Gameplay warm point';o['creature_preview_stage']=True
        o.data.energy=650;o.data.shadow_soft_size=.12;o.data.color=(1,.64,.37)
        lights=[((-2,2,3),120,3,(.48,.62,1))]
    else:lights=[((2,-3,4),450,4,(1,.78,.55)),((-2,2,3),600,3,(.60,.78,1)),((-1,-1,2),120,2,(1,1,1))]
    for pos,power,size,color in lights:
        if kind=='slime' and not gameplay:size*=.3
        bpy.ops.object.light_add(type='AREA',location=pos);o=bpy.context.object;o['creature_preview_stage']=True
        o.data.energy=power;o.data.shape='DISK';o.data.size=size;o.data.color=color;aim(o,(0,0,.3))
    cam=s.camera
    if cam is None or cam.name not in s.objects:
        bpy.ops.object.camera_add();cam=bpy.context.object;s.camera=cam
    return cam
