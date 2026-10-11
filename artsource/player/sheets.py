"""Render CPU Cycles evidence grids from the actual exported player GLBs."""
import argparse
import math
from pathlib import Path
import sys

import bpy
from mathutils import Vector

sys.path.insert(0, str(Path(__file__).resolve().parent))
import build
import preview

POSES = [('idle', 0), ('move', 7), ('run', 6), ('melee', 15),
         ('bow', 21), ('cast', 21), ('hit', 2), ('death', 30)]
TONES = ['LightWarm', 'MediumWarm', 'DeepWarm']


def load(kind):
    before_objects, before_actions = set(bpy.data.objects), set(bpy.data.actions)
    bpy.ops.import_scene.gltf(filepath=str(build.OUT / (kind + '.glb')))
    objects = list(set(bpy.data.objects) - before_objects)
    rig = next(obj for obj in objects if obj.type == 'ARMATURE')
    for track in rig.animation_data.nla_tracks:
        track.mute = True
    actions = {}
    for action in set(bpy.data.actions) - before_actions:
        name = action.name.rsplit('|', 1)[-1].split('.')[0].removeprefix('player_')
        actions[name] = action
    return objects, rig, actions


def freeze(objects, rig, actions, clip, frame, style, position, tone=None):
    rig.animation_data.action = actions[clip]
    if actions[clip].slots:
        rig.animation_data.action_slot = actions[clip].slots[0]
    bpy.context.scene.frame_set(frame)
    bpy.context.view_layer.update()
    depsgraph = bpy.context.evaluated_depsgraph_get()
    for obj in objects:
        if obj.type != 'MESH' or not any(modifier.type == 'ARMATURE' and modifier.object == rig
                                        for modifier in obj.modifiers):
            continue
        if 'Hair.' in obj.name and not obj.name.startswith('Hair.' + style):
            continue
        evaluated = obj.evaluated_get(depsgraph)
        data = bpy.data.meshes.new_from_object(evaluated, preserve_all_data_layers=True, depsgraph=depsgraph)
        data.transform(evaluated.matrix_world)
        frozen = bpy.data.objects.new('Sheet ' + clip + ' ' + obj.name, data)
        bpy.context.collection.objects.link(frozen)
        frozen.location = position
        frozen.rotation_euler.z = -.35
        if tone:
            for index, material in enumerate(data.materials):
                if material and material.name.startswith('SkinTint'):
                    data.materials[index] = tinted(material, tone)


def tinted(source, tone):
    """Replace the imported base-color factor with the exact requested tint."""
    material = source.copy()
    material.name = 'Sheet SkinTint ' + tone
    nodes, links = material.node_tree.nodes, material.node_tree.links
    principled = next(node for node in nodes if node.type == 'BSDF_PRINCIPLED')
    base = principled.inputs['Base Color']

    def image_upstream(socket, seen):
        for link in socket.links:
            node = link.from_node
            if node in seen:
                continue
            seen.add(node)
            if node.type == 'TEX_IMAGE':
                return node
            for inlet in node.inputs:
                texture = image_upstream(inlet, seen)
                if texture:
                    return texture
        return None

    texture = image_upstream(base, set())
    if texture is None:
        raise ValueError('Imported SkinTint has no base-color texture')
    mix = nodes.new('ShaderNodeMixRGB')
    mix.blend_type = 'MULTIPLY'
    mix.inputs[0].default_value = 1
    mix.inputs[2].default_value = (*build.linear(build.SKINS[tone]), 1)
    links.new(texture.outputs['Color'], mix.inputs[1])
    links.new(mix.outputs[0], base)
    return material


def remove_originals(objects):
    for obj in objects:
        bpy.data.objects.remove(obj, do_unlink=True)


def stage(columns):
    camera = preview.stage()
    scene = bpy.context.scene
    scene.render.engine = 'CYCLES'
    scene.cycles.device = 'CPU'
    scene.cycles.samples = 16
    scene.cycles.use_denoising = True
    scene.render.resolution_x, scene.render.resolution_y = 1280, 720
    scene.render.resolution_percentage = 100
    for obj in list(scene.objects):
        if obj.type == 'LIGHT':
            bpy.data.objects.remove(obj, do_unlink=True)
    for position, energy, color in [((5,-3,7),3.0,(1,.91,.8)),
                                    ((4,5,3),1.6,(.72,.82,1)),
                                    ((-4,0,5),1.2,(1,1,1))]:
        bpy.ops.object.light_add(type='SUN',location=position)
        light=bpy.context.object
        preview.aim(light,(0,0,.7))
        light.data.energy=energy
        light.data.color=color
        light.data.angle=math.radians(8)
    camera.data.type = 'ORTHO'
    camera.data.ortho_scale = 10.5
    target = Vector((2.65, 0, .65))
    camera.location = target + Vector((8, 0, 6))
    preview.aim(camera, target)
    return camera


def label(camera, text, row, column, columns):
    data = bpy.data.curves.new('Sheet caption', 'FONT')
    data.body = text
    data.align_x = 'CENTER'
    data.align_y = 'CENTER'
    data.size = .145 if columns == 6 else .19
    obj = bpy.data.objects.new('Caption ' + text, data)
    bpy.context.collection.objects.link(obj)
    # The fallen character reaches forward; move its caption below the head.
    caption_x = 2.1 if text.startswith('death') else 1.55
    obj.location = (row * 4.4 + caption_x, (column - (columns - 1) / 2) * (1.4 if columns == 6 else 2.2), .12)
    obj.rotation_euler = camera.rotation_euler
    material = bpy.data.materials.get('Sheet caption emission')
    if material is None:
        material = bpy.data.materials.new('Sheet caption emission')
        material.use_nodes = True
        nodes, links = material.node_tree.nodes, material.node_tree.links
        emission = nodes.new('ShaderNodeEmission')
        emission.inputs['Color'].default_value = (.83, .88, .94, 1)
        emission.inputs['Strength'].default_value = 1
        links.new(emission.outputs[0], nodes.get('Material Output').inputs['Surface'])
    data.materials.append(material)


def actions_sheet(kind):
    build.clear()
    bpy.context.scene.render.fps = 30
    objects, rig, actions = load(kind)
    style = 'Tied' if kind == 'player_b' else 'Cropped'
    for index, (clip, frame) in enumerate(POSES):
        row, column = divmod(index, 4)
        freeze(objects, rig, actions, clip, frame, style,
               (row * 4.4, (column - 1.5) * 2.2, 0))
    remove_originals(objects)
    camera = stage(4)
    for index, (clip, frame) in enumerate(POSES):
        label(camera, f'{clip} / frame {frame}', *divmod(index, 4), 4)
    preview.render(kind + '_actions')


def appearance_sheet():
    build.clear()
    bpy.context.scene.render.fps = 30
    for row, kind in enumerate(('player_a', 'player_b')):
        objects, rig, actions = load(kind)
        for column in range(6):
            style, tone = ('Cropped' if column < 3 else 'Tied'), TONES[column % 3]
            freeze(objects, rig, actions, 'idle', 0, style,
                   (row * 4.4, (column - 2.5) * 1.4, 0), tone)
        remove_originals(objects)
    camera = stage(6)
    for row, body in enumerate(('A', 'B')):
        for column in range(6):
            style = 'Cropped' if column < 3 else 'Tied'
            label(camera, f'{body} / {style}\n{TONES[column % 3]}', row, column, 6)
    preview.render('player_appearance_combinations')


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--view', choices=('actions', 'appearances', 'all'), default='appearances')
    parser.add_argument('--only', choices=('player_a', 'player_b'))
    args = parser.parse_args(sys.argv[sys.argv.index('--') + 1:] if '--' in sys.argv else [])
    if args.view == 'all':
        actions_sheet('player_a');actions_sheet('player_b');appearance_sheet()
    elif args.view == 'appearances':
        appearance_sheet()
    else:
        for kind in (args.only,) if args.only else ('player_a', 'player_b'):
            actions_sheet(kind)
