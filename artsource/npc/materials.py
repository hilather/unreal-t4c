"""Original neutral textile tile, baked in Cycles; tint factors stay editable.
No concept pixels, downloaded textures, geometry AO or per-NPC atlases.
All values are Prototype presentation tuning (W5-16/LH_Prototype_v1).
"""
from pathlib import Path
import bpy

SEED = 516

def linear(value):
    return tuple(((c / 255 + .055) / 1.055) ** 2.4 if c > 10 else c / 3294.6
                 for c in bytes.fromhex(value))

def bake(out):
    scene = bpy.context.scene
    scene.render.engine = 'CYCLES'
    scene.cycles.device = 'CPU'
    scene.cycles.samples = 8
    scene.cycles.seed = SEED
    bpy.ops.mesh.primitive_plane_add(size=2)
    plane = bpy.context.object
    source = bpy.data.materials.new('Textile bake source')
    source.use_nodes = True
    plane.data.materials.append(source)
    n, l = source.node_tree.nodes, source.node_tree.links
    p, output = n.get('Principled BSDF'), n.get('Material Output')
    uv = n.new('ShaderNodeTexCoord')
    noise = n.new('ShaderNodeTexNoise')
    noise.inputs['Scale'].default_value = 90
    noise.inputs['Detail'].default_value = 2
    l.new(uv.outputs['UV'], noise.inputs['Vector'])
    ramp = n.new('ShaderNodeValToRGB')
    ramp.color_ramp.elements[0].color = (.60, .60, .60, 1)
    ramp.color_ramp.elements[1].color = (.95, .95, .95, 1)
    l.new(noise.outputs['Fac'], ramp.inputs[0])
    weave = n.new('ShaderNodeTexWave')
    weave.bands_direction = 'X'
    weave.inputs['Scale'].default_value = 125
    l.new(uv.outputs['UV'], weave.inputs['Vector'])
    bump = n.new('ShaderNodeBump')
    bump.inputs['Strength'].default_value = .22
    bump.inputs['Distance'].default_value = .001
    l.new(noise.outputs['Fac'], bump.inputs['Height'])
    fine = n.new('ShaderNodeBump')
    fine.inputs['Strength'].default_value = .14
    fine.inputs['Distance'].default_value = .00035
    l.new(weave.outputs['Fac'], fine.inputs['Height'])
    l.new(bump.outputs[0], fine.inputs['Normal'])
    l.new(fine.outputs[0], p.inputs['Normal'])
    images = {}
    for channel in ('base', 'normal'):
        im = bpy.data.images.new('NPC_Textile_' + channel, width=512, height=512, alpha=False)
        if channel == 'normal':
            im.colorspace_settings.name = 'Non-Color'
        node = n.new('ShaderNodeTexImage')
        node.image = im
        n.active = node
        if channel == 'base':
            emission = n.new('ShaderNodeEmission')
            l.new(ramp.outputs[0], emission.inputs[0])
            l.new(emission.outputs[0], output.inputs[0])
            bpy.ops.object.bake(type='EMIT', margin=2)
            l.new(p.outputs[0], output.inputs[0])
            n.remove(emission)
        else:
            bpy.ops.object.bake(type='NORMAL', margin=2)
        im.filepath_raw = str(out / ('NPC_Textile_' + channel + '.png'))
        im.file_format = 'PNG'
        im.save()
        images[channel] = im
    bpy.data.objects.remove(plane, do_unlink=True)
    return images


def material(name, color, images=None, rough=.8, metal=0):
    m = bpy.data.materials.new(name)
    m.use_nodes = True
    m.diffuse_color = (*linear(color), 1)
    n, l = m.node_tree.nodes, m.node_tree.links
    p = n.get('Principled BSDF')
    p.inputs['Base Color'].default_value = m.diffuse_color
    p.inputs['Roughness'].default_value = rough
    p.inputs['Metallic'].default_value = metal
    if images:
        base = n.new('ShaderNodeTexImage')
        base.image = images['base']
        mix = n.new('ShaderNodeMix')
        mix.data_type = 'RGBA'
        mix.blend_type = 'MULTIPLY'
        mix.inputs[0].default_value = 1
        mix.inputs[7].default_value = m.diffuse_color
        l.new(base.outputs[0], mix.inputs[6])
        l.new(mix.outputs[2], p.inputs['Base Color'])
        tex = n.new('ShaderNodeTexImage')
        tex.image = images['normal']
        normal = n.new('ShaderNodeNormalMap')
        normal.inputs['Strength'].default_value = .7
        l.new(tex.outputs[0], normal.inputs['Color'])
        l.new(normal.outputs[0], p.inputs['Normal'])
    return m


def palette(prefix, row, images):
    colors = dict(Cloth=row['cloth'], Accent=row['accent'], Trim=row['trim'],
                  Skin=row['skin'], Hair=row['hair'], Lip=row['lip'],
                  Leather='4D3020', Edge='785333', Stitch='A68C61', Metal='92734A',
                  Iron='646C6C', Wood='67452E', Paper='D6C69B', Crease='36251E',
                  Eye='B0A08A', Iris='504031', Pupil='120F0D', Pants='4E4836')
    result = {}
    for key, color in colors.items():
        cloth = key in ('Cloth', 'Accent', 'Trim', 'Pants')
        result[key] = material(prefix + '.' + key, color, images if cloth else None,
                               .86 if cloth else .42 if key in ('Metal', 'Iron') else .64,
                               .7 if key in ('Metal', 'Iron') else 0)
    return result
