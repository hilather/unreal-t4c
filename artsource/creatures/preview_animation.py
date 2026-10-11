"""W5-20 source-animation contact sheets rendered with CPU Cycles.

Run with Blender, e.g. blender -b -t 4 --python preview_animation.py --
--only rat --output /absolute/path --samples 8 --size 320.
Gameplay panels use an orthographic crop at 55 degrees down / 45 degrees
 yaw, not the fixed-distance perspective gameplay camera. Each creature gets
one 20-panel attack PNG and one five-panel idle PNG. No export or bake occurs.
With --reactions, render only one hit/death sheet per selected creature.
"""
import argparse
import math
import sys
from pathlib import Path

import bpy
from mathutils import Vector

sys.path.insert(0, str(Path(__file__).resolve().parent))
import build
import roster

ATTACK_FRAMES = (0, 12, 22, 26, 28, 30, 32, 35, 38, 42)
IDLE_FRAMES = (0, 12, 27, 43, 60)
HIT_FRAMES = (0, 3, 6, 10, 18)
DEATH_FRAMES = (0, 10, 23, 32, 60)


def view_rotation(elevation, yaw):
    """Camera local axes in source world coordinates."""
    elevation, yaw = map(math.radians, (elevation, yaw))
    outward = Vector((math.cos(elevation) * math.cos(yaw),
                      -math.cos(elevation) * math.sin(yaw), math.sin(elevation)))
    return (-outward).to_track_quat('-Z', 'Y')


def freeze(mesh, rig, action, frames):
    for track in rig.animation_data.nla_tracks:
        track.mute = True
    rig.animation_data.action = action
    rig.animation_data.action_slot = action.slots[0]
    result = []
    for frame in frames:
        bpy.context.scene.frame_set(frame)
        bpy.context.view_layer.update()
        evaluated = mesh.evaluated_get(bpy.context.evaluated_depsgraph_get())
        data = bpy.data.meshes.new_from_object(evaluated, preserve_all_data_layers=True,
                                               depsgraph=bpy.context.evaluated_depsgraph_get())
        obj = bpy.data.objects.new(f'{action.name}_f{frame}', data)
        obj.matrix_world = mesh.matrix_world.copy()
        obj['preview_clip'] = action.name.rsplit('_', 1)[-1]
        # new_from_object retains evaluated material slots and vertex deformation.
        bpy.context.collection.objects.link(obj)
        obj.hide_render = True
        result.append((frame, obj))
    return result


def label(text, x, y, size, material):
    curve = bpy.data.curves.new('Panel label', 'FONT')
    curve.body = text
    curve.size = size
    curve.align_x = 'CENTER'
    obj = bpy.data.objects.new('Panel label', curve)
    bpy.context.collection.objects.link(obj)
    obj.location = (x, y, 20)
    curve.materials.append(material)
    return obj


def sheet(kind, poses, views, output, size, samples):
    """Place frozen source meshes in camera coordinates for one sheet render."""
    scene = bpy.context.scene
    scene.render.engine = 'CYCLES'
    scene.cycles.device = 'CPU'
    scene.cycles.samples = samples
    scene.cycles.use_denoising = True
    scene.render.image_settings.file_format = 'PNG'
    scene.render.image_settings.color_mode = 'RGB'
    scene.render.image_settings.color_depth = '8'
    scene.render.image_settings.compression = 85
    scene.render.resolution_percentage = 100
    scene.view_settings.look = 'AgX - Medium High Contrast'
    scene.world.use_nodes = True
    background = scene.world.node_tree.nodes.get('Background')
    background.inputs['Color'].default_value = (.035, .045, .065, 1)
    background.inputs['Strength'].default_value = .65
    rotations = [(title, view_rotation(elev, yaw).inverted())
                 for title, elev, yaw in views]
    projected = [rotation @ (obj.matrix_world @ vertex.co)
                 for _, rotation in rotations for _, obj in poses
                 for vertex in obj.data.vertices]
    lo = Vector(tuple(min(v[i] for v in projected) for i in range(3)))
    hi = Vector(tuple(max(v[i] for v in projected) for i in range(3)))
    center = (lo + hi) * .5
    span = max(hi.x - lo.x, hi.y - lo.y, .1)
    tile = span * 1.35
    cols = 5
    rows_per_view = math.ceil(len(poses) / cols)
    rows = rows_per_view * len(views)
    material = bpy.data.materials.new('Sheet label emission')
    material.use_nodes = True
    nodes = material.node_tree.nodes
    nodes.clear()
    emission = nodes.new('ShaderNodeEmission')
    emission.inputs['Color'].default_value = (.8, .86, .95, 1)
    material.node_tree.links.new(emission.outputs[0], nodes.new('ShaderNodeOutputMaterial').inputs['Surface'])
    objects = []
    for view_index, (title, rotation) in enumerate(rotations):
        for index, (frame, source) in enumerate(poses):
            col, row = index % cols, view_index * rows_per_view + index // cols
            x = (col - (cols - 1) / 2) * tile
            y = ((rows - 1) / 2 - row) * tile
            obj = bpy.data.objects.new(source.name + '_' + title, source.data)
            bpy.context.collection.objects.link(obj)
            obj.matrix_world = rotation.to_matrix().to_4x4() @ source.matrix_world
            obj.location += Vector((x - center.x, y - center.y + tile * .045, -center.z))
            objects.append(obj)
            suffix = ' IMPACT' if 'attack' in source.name and frame == 30 else ''
            clip = source['preview_clip']
            objects.append(label(f'{title} | {clip} f{frame}{suffix}', x, y - tile * .44,
                                 tile * .047, material))
    bpy.ops.object.camera_add(location=(0, 0, 40))
    camera = bpy.context.object
    camera.data.type = 'ORTHO'
    camera.data.ortho_scale = cols * tile
    scene.camera = camera
    objects.append(camera)
    for direction, energy, color in [((3, -4, 6), 3, (1, .85, .7)),
                                      ((-4, 2, 4), 1.5, (.65, .8, 1))]:
        bpy.ops.object.light_add(type='SUN', location=direction)
        sun = bpy.context.object
        sun.data.energy = energy
        sun.data.color = color
        sun.data.angle = .12
        build.aim(sun, (0, 0, 0))
        objects.append(sun)
    scene.render.resolution_x = cols * size
    scene.render.resolution_y = rows * size
    scene.render.filepath = str(output)
    bpy.ops.render.render(write_still=True)
    for obj in objects:
        bpy.data.objects.remove(obj, do_unlink=True)
    scene.camera = None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--only', choices=roster.KINDS)
    parser.add_argument('--reactions', action='store_true',
                        help='Render only a combined five-hit/five-death pose sheet')
    parser.add_argument('--output', type=Path,
                        default=Path(__file__).resolve().parent / 'output' / 'w5-20')
    parser.add_argument('--samples', type=int, default=8)
    parser.add_argument('--size', type=int, default=320, help='Pixels per square panel')
    args = parser.parse_args(sys.argv[sys.argv.index('--') + 1:] if '--' in sys.argv else [])
    if args.samples < 1 or args.size < 64:
        parser.error('--samples must be positive and --size must be at least 64')
    if not args.output.is_absolute():
        parser.error('--output must be an absolute path')
    args.output.mkdir(parents=True, exist_ok=True)
    for kind in [args.only] if args.only else roster.KINDS:
        mesh, rig, actions, _ = build.build(kind, bake_assets=False)
        bpy.context.scene.render.fps = 30
        if args.reactions:
            reactions = freeze(mesh, rig, actions['hit'], HIT_FRAMES)
            reactions += freeze(mesh, rig, actions['death'], DEATH_FRAMES)
            mesh.hide_render = True
            rig.hide_render = True
            sheet(kind, reactions, [('three-quarter 25/45', 25, 45)],
                  args.output / f'{kind}_reactions.png', args.size, args.samples)
            print(f'W5-20 source reaction sheet complete: {kind}', flush=True)
            continue
        attack = freeze(mesh, rig, actions['attack'], ATTACK_FRAMES)
        idle = freeze(mesh, rig, actions['idle'], IDLE_FRAMES)
        mesh.hide_render = True
        rig.hide_render = True
        sheet(kind, attack, [('gameplay crop 55/45', 55, 45), ('three-quarter 25/45', 25, 45)],
              args.output / f'{kind}_attack.png', args.size, args.samples)
        sheet(kind, idle, [('idle three-quarter 25/45', 25, 45)],
              args.output / f'{kind}_idle.png', args.size, args.samples)
        print(f'W5-20 source sheets complete: {kind}', flush=True)


if __name__ == '__main__':
    main()
