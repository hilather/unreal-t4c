"""Deterministic tileable Cycles CPU bakes and compact shared-texture GLBs."""
import json
import math
import os
from pathlib import Path
import struct
import tempfile

import bpy


def _node(tree, kind):
    return tree.nodes.new(kind)


def _math(tree, operation, a, b=None):
    node = _node(tree, 'ShaderNodeMath')
    node.operation = operation
    for index, value in enumerate((a, b)):
        if value is not None:
            if isinstance(value, (float, int)):
                node.inputs[index].default_value = value
            else:
                tree.links.new(value, node.inputs[index])
    return node.outputs[0]


def _procedural(name):
    mat = bpy.data.materials.new('_bake_' + name)
    mat.use_nodes = True
    tree = mat.node_tree
    tree.nodes.clear()
    uv = _node(tree, 'ShaderNodeTexCoord')
    split = _node(tree, 'ShaderNodeSeparateXYZ')
    tree.links.new(uv.outputs['UV'], split.inputs[0])
    # Embed UV on a torus: both texture values and derivatives repeat at edges.
    u = _math(tree, 'MULTIPLY', split.outputs['X'], math.tau)
    v = _math(tree, 'MULTIPLY', split.outputs['Y'], math.tau)
    coords = _node(tree, 'ShaderNodeCombineXYZ')
    for socket, source in zip(coords.inputs, (_math(tree, 'COSINE', u), _math(tree, 'SINE', u), _math(tree, 'COSINE', v))):
        tree.links.new(source, socket)
    noise = _node(tree, 'ShaderNodeTexNoise')
    noise.noise_dimensions = '4D'
    noise.inputs['Scale'].default_value = 3.5
    noise.inputs['Detail'].default_value = 4
    tree.links.new(coords.outputs[0], noise.inputs['Vector'])
    tree.links.new(_math(tree, 'SINE', v), noise.inputs['W'])
    fine = _node(tree, 'ShaderNodeTexNoise')
    fine.noise_dimensions = '4D'
    fine.inputs['Scale'].default_value = 32
    fine.inputs['Detail'].default_value = 3
    tree.links.new(coords.outputs[0], fine.inputs['Vector'])
    tree.links.new(_math(tree, 'SINE', v), fine.inputs['W'])
    tone = noise.outputs['Fac']
    height = tone
    if name == 'stone':
        edge = _node(tree, 'ShaderNodeTexVoronoi')
        edge.feature = 'DISTANCE_TO_EDGE'
        edge.inputs['Scale'].default_value = 7
        tree.links.new(coords.outputs[0], edge.inputs['Vector'])
        # Only scattered portions of cell boundaries survive, avoiding a web.
        cracks = _math(tree, 'MULTIPLY',
                       _math(tree, 'LESS_THAN', edge.outputs['Distance'], .028),
                       _math(tree, 'GREATER_THAN', noise.outputs['Fac'], .57))
        pores = _math(tree, 'LESS_THAN', fine.outputs['Fac'], .31)
        height = _math(tree, 'ADD', _math(tree, 'MULTIPLY', tone, .7),
                       _math(tree, 'MULTIPLY', fine.outputs['Fac'], .22))
        pits = _math(tree, 'ADD', _math(tree, 'MULTIPLY', cracks, .36),
                     _math(tree, 'MULTIPLY', pores, .20))
        height = _math(tree, 'SUBTRACT', height, pits)
        tone = _math(tree, 'SUBTRACT',
                     _math(tree, 'ADD', _math(tree, 'MULTIPLY', tone, .84),
                           _math(tree, 'MULTIPLY', fine.outputs['Fac'], .16)),
                     _math(tree, 'MULTIPLY', pits, .65))
    elif name == 'timber':
        # Periodic slow noise bends the tight grain into irregular knot swirls.
        phase = _math(tree, 'ADD', _math(tree, 'MULTIPLY', v, 28),
                      _math(tree, 'ADD', _math(tree, 'MULTIPLY', tone, 17),
                            _math(tree, 'MULTIPLY', _math(tree, 'SINE', u), 3)))
        grain = _math(tree, 'POWER',
                      _math(tree, 'ADD', _math(tree, 'MULTIPLY',
                            _math(tree, 'SINE', phase), .5), .5), 7)
        height = _math(tree, 'SUBTRACT',
                       _math(tree, 'MULTIPLY', tone, .35),
                       _math(tree, 'MULTIPLY', grain, .18))
        tone = _math(tree, 'SUBTRACT',
                     _math(tree, 'ADD', _math(tree, 'MULTIPLY', tone, .85), .12),
                     _math(tree, 'MULTIPLY', grain, .28))
    ramp = _node(tree, 'ShaderNodeValToRGB')
    colors = {'stone': ((.065, .055, .037, 1), (.39, .345, .245, 1)),
              'timber': ((.019, .016, .011, 1), (.105, .075, .043, 1)),
              'iron': ((.025, .03, .032, 1), (.12, .095, .067, 1))}
    for stop, color in zip(ramp.color_ramp.elements, colors[name]):
        stop.color = color
        stop.position = .2 if stop.position == 0 else .8
    if name == 'stone':
        mineral = ramp.color_ramp.elements.new(.53)
        mineral.color = (.19, .18, .14, 1)
    tree.links.new(tone, ramp.inputs[0])
    roughness = _math(tree, 'ADD', _math(tree, 'MULTIPLY', noise.outputs['Fac'], .16),
                      .79 if name == 'stone' else .72 if name == 'timber' else .48)
    shader = _node(tree, 'ShaderNodeBsdfPrincipled')
    bump = _node(tree, 'ShaderNodeBump')
    bump.inputs['Strength'].default_value = .65 if name == 'stone' else .48
    bump.inputs['Distance'].default_value = .045 if name == 'stone' else .016
    tree.links.new(height, bump.inputs['Height'])
    tree.links.new(bump.outputs['Normal'], shader.inputs['Normal'])
    output = _node(tree, 'ShaderNodeOutputMaterial')
    tree.links.new(shader.outputs[0], output.inputs['Surface'])
    return mat, ramp.outputs['Color'], roughness, shader, output


def _image(name, resolution, texture_dir, data=False):
    image = bpy.data.images.new(name, width=resolution, height=resolution, alpha=False)
    image.filepath_raw = str(texture_dir / (name + '.png'))
    image.file_format = 'PNG'
    if data:
        image.colorspace_settings.name = 'Non-Color'
    return image


def _final(name, images):
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    tree = mat.node_tree
    shader = tree.nodes.get('Principled BSDF')
    nodes = []
    for image in images:
        node = _node(tree, 'ShaderNodeTexImage')
        node.image = image
        node.extension = 'REPEAT'
        nodes.append(node)
    tint = _node(tree, 'ShaderNodeVertexColor')
    tint.layer_name = 'Tint'
    multiply = _node(tree, 'ShaderNodeMixRGB')
    multiply.blend_type = 'MULTIPLY'
    multiply.inputs[0].default_value = 1
    tree.links.new(nodes[0].outputs['Color'], multiply.inputs[1])
    tree.links.new(tint.outputs['Color'], multiply.inputs[2])
    tree.links.new(multiply.outputs[0], shader.inputs['Base Color'])
    normal = _node(tree, 'ShaderNodeNormalMap')
    tree.links.new(nodes[1].outputs['Color'], normal.inputs['Color'])
    tree.links.new(normal.outputs['Normal'], shader.inputs['Normal'])
    channels = _node(tree, 'ShaderNodeSeparateColor')
    tree.links.new(nodes[2].outputs['Color'], channels.inputs[0])
    tree.links.new(channels.outputs['Green'], shader.inputs['Roughness'])
    tree.links.new(channels.outputs['Blue'], shader.inputs['Metallic'])
    # Use the exporter's full interface: its importer also reuses this group.
    from io_scene_gltf2.blender.com.material_helpers import create_settings_group, get_gltf_node_name
    group = bpy.data.node_groups.get(get_gltf_node_name())
    if group is None:
        group = create_settings_group(get_gltf_node_name())
    else:
        # Repair an older Occlusion-only group without breaking existing links.
        template = create_settings_group('_gltf_interface_template')
        existing = {item.name for item in group.interface.items_tree
                    if item.item_type == 'SOCKET' and item.in_out == 'INPUT'}
        for item in template.interface.items_tree:
            if item.item_type == 'SOCKET' and item.in_out == 'INPUT' and item.name not in existing:
                socket = group.interface.new_socket(name=item.name, in_out='INPUT',
                                                    socket_type=item.socket_type)
                socket.default_value = item.default_value
        bpy.data.node_groups.remove(template)
    occlusion = _node(tree, 'ShaderNodeGroup')
    occlusion.node_tree = group
    tree.links.new(channels.outputs['Red'], occlusion.inputs['Occlusion'])
    return mat


def bake_materials(output_dir, resolution=512):
    """Bake nine PNGs under output_dir/textures and return three glTF materials."""
    texture_dir = Path(output_dir).resolve() / 'textures'
    texture_dir.mkdir(parents=True, exist_ok=True)
    scene = bpy.context.scene
    old_engine, old_device = scene.render.engine, scene.cycles.device
    selected = list(bpy.context.selected_objects)
    active = bpy.context.view_layer.objects.active
    scene.render.engine = 'CYCLES'
    scene.cycles.device = 'CPU'
    old_samples = scene.cycles.samples
    scene.cycles.samples = 1
    bpy.ops.object.select_all(action='DESELECT')
    bpy.ops.mesh.primitive_plane_add(size=2)
    plane = bpy.context.object
    result = {}
    try:
        for name in ('stone', 'timber', 'iron'):
            mat, color, roughness, shader, output = _procedural(name)
            plane.data.materials.clear()
            plane.data.materials.append(mat)
            tree = mat.node_tree
            target = _node(tree, 'ShaderNodeTexImage')
            tree.nodes.active = target
            emission = _node(tree, 'ShaderNodeEmission')
            images = [_image(name + '_basecolor', resolution, texture_dir),
                      _image(name + '_normal', resolution, texture_dir, True),
                      _image(name + '_orm', resolution, texture_dir, True)]
            rough = _image('_roughness', resolution, texture_dir, True)
            for image, source in ((images[0], color), (rough, roughness)):
                target.image = image
                tree.links.new(source, emission.inputs['Color'])
                tree.links.new(emission.outputs[0], output.inputs['Surface'])
                bpy.ops.object.bake(type='EMIT', margin=0, use_clear=True)
            target.image = images[1]
            tree.links.new(shader.outputs[0], output.inputs['Surface'])
            bpy.ops.object.bake(type='NORMAL', normal_space='TANGENT', margin=0, use_clear=True)
            pixels = list(rough.pixels[:])
            metallic = .85 if name == 'iron' else 0
            for index in range(0, len(pixels), 4):
                pixels[index:index + 4] = [1, pixels[index], metallic, 1]
            images[2].pixels[:] = pixels
            for image in images:
                image.save()
            bpy.data.images.remove(rough)
            result[name] = _final(name, images)
            bpy.data.materials.remove(mat)
    finally:
        bpy.data.objects.remove(plane, do_unlink=True)
        scene.render.engine, scene.cycles.device = old_engine, old_device
        scene.cycles.samples = old_samples
        for obj in selected:
            obj.select_set(True)
        bpy.context.view_layer.objects.active = active
    return result


def export_piece(obj, path):
    """Export standard glTF coordinates; retain shared external PNGs in a GLB."""
    path = Path(path).resolve()
    path.parent.mkdir(parents=True, exist_ok=True)
    selected = list(bpy.context.selected_objects)
    active = bpy.context.view_layer.objects.active
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    try:
        # Temporary exports stay beside the deliverable (normally under Saved/).
        with tempfile.TemporaryDirectory(dir=path.parent) as scratch:
            intermediate = Path(scratch) / 'piece.glb'
            bpy.ops.export_scene.gltf(filepath=str(intermediate), export_format='GLB',
                                      use_selection=True, export_image_format='AUTO',
                                      export_vertex_color='NAME', export_vertex_color_name='Tint',
                                      export_texcoords=True, export_normals=True)
            data = intermediate.read_bytes()
            json_length = struct.unpack_from('<I', data, 12)[0]
            doc = json.loads(data[20:20 + json_length])
            binary = data[28 + json_length:]
            image_views = set()
            textures = path.parent / 'textures'
            textures.mkdir(exist_ok=True)
            for image in doc.get('images', []):
                view_index = image.pop('bufferView')
                image_views.add(view_index)
                view = doc['bufferViews'][view_index]
                name = image.get('name', 'texture')
                filename = name if name.endswith('.png') else name + '.png'
                payload = binary[view.get('byteOffset', 0):view.get('byteOffset', 0) + view['byteLength']]
                target = textures / filename
                if target.exists() and target.read_bytes() != payload:
                    raise ValueError('Shared texture differs: ' + str(target))
                if not target.exists():
                    target.write_bytes(payload)
                image['uri'] = 'textures/' + filename
                image.pop('mimeType', None)
            packed, views, mapping = bytearray(), [], {}
            for index, view in enumerate(doc.get('bufferViews', [])):
                if index in image_views:
                    continue
                packed.extend(b'\0' * (-len(packed) % 4))
                mapping[index] = len(views)
                new_view = dict(view, byteOffset=len(packed))
                start = view.get('byteOffset', 0)
                packed.extend(binary[start:start + view['byteLength']])
                views.append(new_view)
            for accessor in doc.get('accessors', []):
                if 'bufferView' in accessor:
                    accessor['bufferView'] = mapping[accessor['bufferView']]
                for item in accessor.get('sparse', {}).values():
                    if isinstance(item, dict) and 'bufferView' in item:
                        item['bufferView'] = mapping[item['bufferView']]
            doc['bufferViews'] = views
            doc['buffers'][0]['byteLength'] = len(packed)
            encoded = json.dumps(doc, separators=(',', ':')).encode()
            encoded += b' ' * (-len(encoded) % 4)
            packed.extend(b'\0' * (-len(packed) % 4))
            length = 12 + 8 + len(encoded) + 8 + len(packed)
            path.write_bytes(struct.pack('<III', 0x46546C67, 2, length) +
                             struct.pack('<II', len(encoded), 0x4E4F534A) + encoded +
                             struct.pack('<II', len(packed), 0x004E4942) + packed)
    finally:
        obj.select_set(False)
        for item in selected:
            item.select_set(True)
        bpy.context.view_layer.objects.active = active
    return path
