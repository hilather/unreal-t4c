"""Independent Blender GLB pose audit; run after build.py has exported both files."""
import hashlib
import json
import math
from pathlib import Path
import sys

import bpy

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import build

ENDS = {'idle': 60, 'move': 30, 'run': 24, 'melee': 24,
        'bow': 30, 'cast': 30, 'hit': 6, 'death': 30}
SOCKETS = {'Socket.Weapon.R', 'Socket.Bow.L', 'Socket.Arrow', 'Socket.Quiver.Back'}


def matrix_values(matrix):
    return [float(value) for row in matrix for value in row]


def geometry_hash(kind):
    mesh, bones = build.geometry(kind)
    mesh.data.calc_loop_triangles()
    payload = {
        'vertices': [list(vertex.co) for vertex in mesh.data.vertices],
        'triangles': [list(triangle.vertices) for triangle in mesh.data.loop_triangles],
        'uv': [list(loop.uv) for loop in mesh.data.uv_layers.active.data],
        'groups': [group.name for group in mesh.vertex_groups],
        'weights': [[(entry.group, float(entry.weight)) for entry in vertex.groups]
                    for vertex in mesh.data.vertices],
        'bones': [(name, list(head), list(tail), parent)
                  for name, (head, tail, parent) in bones.items()],
    }
    return hashlib.sha256(json.dumps(payload, separators=(',', ':')).encode()).hexdigest()


def evaluate(meshes):
    bpy.context.view_layer.update()
    depsgraph = bpy.context.evaluated_depsgraph_get()
    points = []
    for mesh in meshes:
        evaluated = mesh.evaluated_get(depsgraph)
        points.extend(tuple(evaluated.matrix_world @ vertex.co)
                      for vertex in evaluated.data.vertices)
    if not points:
        raise ValueError('No evaluated mesh vertices')
    lower = [min(point[axis] for point in points) for axis in range(3)]
    upper = [max(point[axis] for point in points) for axis in range(3)]
    return points, lower, upper


def validate(kind):
    result = {'asset': kind, 'passed': [], 'passed_checks': 0, 'exceptions': [], 'actions': {}}

    def check(condition, message):
        if condition:
            result['passed_checks'] += 1
            if ' frame ' not in message:
                result['passed'].append(message)
        else:
            result['exceptions'].append(message)

    first, second = geometry_hash(kind), geometry_hash(kind)
    result['construction_sha256'] = [first, second]
    check(first == second, 'Two independent source constructions have identical vertices, topology, UVs, weights and bones')
    build.clear()
    previous_actions = set(bpy.data.actions)
    bpy.context.scene.render.fps = 30
    bpy.ops.import_scene.gltf(filepath=str(HERE / 'output' / (kind + '.glb')))
    rigs = [obj for obj in bpy.context.scene.objects if obj.type == 'ARMATURE']
    check(len(rigs) == 1, 'One imported armature')
    if len(rigs) != 1:
        return result
    rig = rigs[0]
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == 'MESH'
              and any(modifier.type == 'ARMATURE' and modifier.object == rig
                      for modifier in obj.modifiers)]
    result['evaluated_meshes'] = [obj.name for obj in meshes]
    check(SOCKETS <= set(rig.data.bones.keys()), 'All four attachment sockets imported')
    check('root' in rig.pose.bones, 'Floor root imported')
    if 'root' not in rig.pose.bones:
        return result
    imported = set(bpy.data.actions) - previous_actions
    actions = {}
    for action in imported:
        # NLA export names may gain armature prefixes or Blender numeric suffixes.
        name = action.name.rsplit('|', 1)[-1].split('.')[0]
        name = name.removeprefix('player_')
        if name in ENDS:
            check(name not in actions, 'Unique imported action ' + name)
            actions[name] = action
    result['imported_action_names'] = sorted(action.name for action in imported)
    check(set(actions) == set(ENDS), 'Exactly eight expected player actions found')
    rig.animation_data_create()
    for track in rig.animation_data.nla_tracks:
        track.mute = True
    baseline_root = None
    for clip, end in ENDS.items():
        if clip not in actions:
            continue
        action = actions[clip]
        rig.animation_data.action = action
        if action.slots:
            rig.animation_data.action_slot = action.slots[0]
        check(abs(action.frame_range[0]) < 1e-4 and abs(action.frame_range[1] - end) < 1e-4,
              clip + ': expected 0-to-' + str(end) + ' frame duration')
        samples, first_points, last_points = [], None, None
        for frame in range(end + 1):
            bpy.context.scene.frame_set(frame)
            points, lower, upper = evaluate(meshes)
            root = rig.matrix_world @ rig.pose.bones['root'].matrix
            transform = matrix_values(root)
            if baseline_root is None:
                baseline_root = transform
            check(transform == baseline_root, f'{clip} frame {frame}: exact constant root transform')
            check(abs(root.translation.z) <= 1e-6, f'{clip} frame {frame}: root at floor')
            check(lower[2] >= -.002, f'{clip} frame {frame}: evaluated mesh clears floor')
            sample = {'frame': frame, 'min': lower, 'max': upper}
            if clip == 'idle':
                check(upper[2] - lower[2] > 1.6, f'idle frame {frame}: upright height exceeds 1.6 m')
                sample['foot_heads_world_z'] = {
                    side: (rig.matrix_world @ rig.pose.bones['foot.' + side].head).z
                    for side in ('L', 'R')}
                sample['maximum_horizontal_radius_m'] = max(math.hypot(point[0], point[1]) for point in points)
            samples.append(sample)
            if frame == 0:
                first_points = points
            if frame == end:
                last_points = points
        if clip in ('idle', 'move', 'run'):
            delta = max(abs(a - b) for start, finish in zip(first_points, last_points)
                        for a, b in zip(start, finish))
            check(delta < 1e-5, clip + ': evaluated loop endpoints match within 10 micrometres')
        if clip == 'death':
            check(samples[-1]['max'][2] - samples[-1]['min'][2] < .5,
                  'death: final corpse height below 0.5 m')
            hold = samples[21:]
            hold_delta = max(abs(value - reference) for sample in hold
                             for key in ('min', 'max')
                             for value, reference in zip(sample[key], hold[0][key]))
            result['death_hold_max_bounds_delta_m'] = hold_delta
            check(hold_delta <= 1e-6,
                  'death: evaluated bounds hold within one micrometre on frames 21 through 30')
        result['actions'][clip] = {'samples': samples,
            'min': [min(sample['min'][axis] for sample in samples) for axis in range(3)],
            'max': [max(sample['max'][axis] for sample in samples) for axis in range(3)]}
    result['root_world_matrix'] = baseline_root
    result['game_capsule'] = {'radius_m': .35, 'half_height_m': .9,
        'note': 'Idle mesh radius measured for comparison; visual arms need not fit the collision capsule.'}
    result['limitations'] = ['Does not prove absence of self-intersections or foot sliding.',
                             'Evaluates integer frames only; subframe penetration is not measured.']
    return result


def main():
    report = {'unit': 'metres', 'fps': 30, 'assets': [], 'exceptions': []}
    for kind in ('player_a', 'player_b'):
        try:
            report['assets'].append(validate(kind))
        except Exception as error:
            report['exceptions'].append(kind + ': ' + repr(error))
    report['success'] = not report['exceptions'] and len(report['assets']) == 2 and all(
        not result['exceptions'] for result in report['assets'])
    path = HERE / 'output' / 'export-pose-audit.json'
    path.write_text(json.dumps(report, indent=2) + '\n')
    print('Export pose audit:', path, 'PASS' if report['success'] else 'FAIL')
    assert report['success'], 'Export pose audit failed; see ' + str(path)


if __name__ == '__main__':
    main()
