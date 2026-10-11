"""W5-20 Blender source/30-fps GLB animation audit (no texture bake).

Example, using the same script for a baseline directory and the candidate:
  blender -b --factory-startup --python-exit-code 1 --python artsource/creatures/audit_animation.py -- \
    --source /tmp/baseline/creatures --output /tmp/baseline-audit.json
  blender -b --factory-startup --python-exit-code 1 --python artsource/creatures/audit_animation.py -- \
    --output /tmp/candidate-audit.json --baseline /tmp/baseline-audit.json

GLBs are temporary. Animation bytes are actual exported buffer payloads, not
predictions of Unreal compressed .uasset size. JSON metadata is counted separately.
The f30 key check establishes a retained authored anchor, not gameplay damage.
"""
import argparse
import hashlib
import json
import math
import struct
import sys
import tempfile
from pathlib import Path

import bpy

CLIPS = {'idle': 60, 'move': 24, 'attack': 42, 'hit': 18, 'death': 60}
COMPONENT_BYTES = {5120: 1, 5121: 1, 5122: 2, 5123: 2, 5125: 4, 5126: 4}
COMPONENT_COUNT = {'SCALAR': 1, 'VEC2': 2, 'VEC3': 3, 'VEC4': 4, 'MAT4': 16}


def digest(value):
    return hashlib.sha256(json.dumps(value, sort_keys=True, separators=(',', ':')).encode()).hexdigest()


def curves(action):
    return [curve for layer in action.layers for strip in layer.strips
            for bag in strip.channelbags for curve in bag.fcurves]


def matrix_values(matrix):
    return [float(component) for row in matrix for component in row]


def near(a, b, tolerance=1e-5, rotation=False):
    direct = max(abs(x-y) for x, y in zip(a, b))
    opposite = max(abs(x+y) for x, y in zip(a, b)) if rotation else math.inf
    return min(direct, opposite) <= tolerance


def source_snapshot(kind, mesh, rig, actions, roster):
    failures = []
    geometry = {'vertices': [tuple(v.co) for v in mesh.data.vertices],
                'faces': [tuple(p.vertices) for p in mesh.data.polygons],
                'weights': [[(mesh.vertex_groups[g.group].name, g.weight) for g in v.groups]
                            for v in mesh.data.vertices]}
    bones = {bone.name: {'parent': bone.parent.name if bone.parent else None,
                         'rest_matrix': matrix_values(bone.matrix_local),
                         'head': list(bone.head_local), 'tail': list(bone.tail_local)}
             for bone in rig.data.bones}
    source = {'triangles': len(mesh.data.polygons), 'geometry_sha256': digest(geometry),
              'rig': bones, 'rig_sha256': digest(bones),
              'rest_min_cm': list(mesh['rest_min_cm']),
              'rest_max_cm': list(mesh['rest_max_cm']), 'actions': {}}
    if set(actions) != set(CLIPS):
        failures.append('source action names differ from five required clips')
    for clip, action in actions.items():
        rig.animation_data.action = action
        rig.animation_data.action_slot = action.slots[0]
        start, end = map(int, action.frame_range)
        if (start, end) != (0, CLIPS[clip]):
            failures.append(f'{clip}: source range is {start}..{end}')
        low, high = roster.limits(kind, clip)
        overall_low, overall_high = [math.inf]*3, [-math.inf]*3
        violations, frames, anchors = [], {}, {}
        for frame in range(start, end+1):
            bpy.context.scene.frame_set(frame)
            bpy.context.view_layer.update()
            deps = bpy.context.evaluated_depsgraph_get()
            evaluated = mesh.evaluated_get(deps)
            positions = [evaluated.matrix_world @ vertex.co for vertex in evaluated.data.vertices]
            minimum = [min(p[axis] for p in positions) for axis in range(3)]
            maximum = [max(p[axis] for p in positions) for axis in range(3)]
            for axis in range(3):
                overall_low[axis] = min(overall_low[axis], minimum[axis])
                overall_high[axis] = max(overall_high[axis], maximum[axis])
                if (minimum[axis] < low[axis] - (.0005 if axis < 2 else 0)
                        or maximum[axis] > high[axis] + .0005):
                    violations.append({'frame': frame, 'axis': axis,
                                       'min_m': minimum[axis], 'max_m': maximum[axis]})
            frames[frame] = {bone.name: matrix_values(bone.matrix_basis) for bone in rig.pose.bones}
            if clip == 'attack' and frame in (0, 20, 29, 30, 33, 42):
                anchors[str(frame)] = {bone.name: [round(float(c), 6) for c in
                                                    rig.matrix_world @ bone.head]
                                       for bone in rig.pose.bones
                                       if bone.name in ('body', 'head', 'jaw')
                                       or bone.name.startswith(('VFX_', 'Weapon_'))}
        root = frames[start]['root']
        static_root = all(near(root, pose['root']) for pose in frames.values())
        if not static_root:
            failures.append(f'{clip}: moving root')
        endpoints_equal = all(near(frames[start][bone], frames[end][bone]) for bone in frames[start])
        loop = None
        if clip in ('idle', 'move'):
            loop = endpoints_equal
            if not loop:
                failures.append(f'{clip}: source endpoints do not loop')
        death_hold = None
        if clip == 'death':
            death_hold = all(near(frames[45][bone], frames[frame][bone])
                             for frame in range(45, 61) for bone in frames[45])
            if not death_hold:
                failures.append('death: source f45..60 hold changed')
        keyed_contact = None
        if clip == 'attack':
            keyed_contact = all(any(abs(key.co.x-30) < 1e-6 for key in curve.keyframe_points)
                                for curve in curves(action))
            if not keyed_contact:
                failures.append('attack: a curve is missing the authored f30 key')
        if violations:
            failures.append(f'{clip}: {len(violations)} source envelope violations')
        source['actions'][clip] = {'frames': [start, end], 'seconds': (end-start)/30,
                                  'source_keys': sum(len(curve.keyframe_points) for curve in curves(action)),
                                  'min_cm': [round(v*100, 4) for v in overall_low],
                                  'max_cm': [round(v*100, 4) for v in overall_high],
                                  'limits_cm': {'min': [v*100 for v in low], 'max': [v*100 for v in high]},
                                  'envelope_violations': violations,
                                  'constant_root': static_root, 'loop_endpoints_equal': loop,
                                  'clip_endpoints_equal': endpoints_equal,
                                  'death_hold_f45_f60': death_hold,
                                  'authored_contact_f30_key': keyed_contact}
        if anchors:
            source['actions'][clip]['anchor_positions_m'] = anchors
    return source, failures


def glb_snapshot(path):
    blob = path.read_bytes()
    magic, version, total = struct.unpack_from('<III', blob)
    assert magic == 0x46546C67 and version == 2 and total == len(blob)
    chunks, offset = {}, 12
    while offset < len(blob):
        length, tag = struct.unpack_from('<II', blob, offset)
        assert offset+8+length <= len(blob)
        chunks[tag] = blob[offset+8:offset+8+length]
        offset += 8+length
    doc = json.loads(chunks[0x4E4F534A])
    binary = chunks[0x004E4942]

    def accessor(index):
        item = doc['accessors'][index]
        assert 'sparse' not in item and item['componentType'] == 5126
        view = doc['bufferViews'][item['bufferView']]
        width = COMPONENT_COUNT[item['type']]
        start = view.get('byteOffset', 0)+item.get('byteOffset', 0)
        stride = view.get('byteStride', width*4)
        return [struct.unpack_from('<'+'f'*width, binary, start+i*stride) for i in range(item['count'])]

    def payload(indices):
        return sum(doc['accessors'][i]['count'] * COMPONENT_BYTES[doc['accessors'][i]['componentType']]
                   * COMPONENT_COUNT[doc['accessors'][i]['type']] for i in indices)

    failures, all_accessors, actions = [], set(), {}
    if {a['name'] for a in doc['animations']} != set(CLIPS):
        failures.append('GLB action names differ from five required clips')
    root = next(i for i, node in enumerate(doc['nodes']) if node.get('name') == 'root')
    for animation in doc['animations']:
        clip = animation['name']
        indices = {sampler[key] for sampler in animation['samplers'] for key in ('input', 'output')}
        all_accessors.update(indices)
        end, moving = 0, False
        for channel in animation['channels']:
            sampler = animation['samplers'][channel['sampler']]
            times = [row[0] for row in accessor(sampler['input'])]
            values = accessor(sampler['output'])
            if not times or len(times) != len(values):
                failures.append(f'{clip}: invalid sampler')
                continue
            if (abs(times[0]) > 1e-6 or any(a >= b for a, b in zip(times, times[1:]))
                    or any(abs(t*30-round(t*30)) > 1e-4 for t in times)):
                failures.append(f'{clip}: samples are not on the 30-fps grid')
            end = max(end, times[-1])
            rotation = channel['target']['path'] == 'rotation'
            if rotation and any(abs(sum(c*c for c in row)-1) > .001 for row in values):
                failures.append(f'{clip}: nonunit quaternion')
            varies = any(not near(values[0], row, rotation=rotation) for row in values)
            moving |= varies
            if channel['target']['node'] == root and varies:
                failures.append(f'{clip}: GLB root drift')
            if clip in ('idle', 'move') and not near(values[0], values[-1], rotation=rotation):
                failures.append(f'{clip}: GLB loop endpoint mismatch')
            if clip == 'death':
                # Force-sampled moving tracks contain f45; optimized constants
                # may retain endpoints only, which must both agree.
                held = [row for t, row in zip(times, values) if t >= 1.5-1e-6]
                before = max(i for i, t in enumerate(times) if t <= 1.5+1e-6)
                if not held or not all(near(values[before], row, rotation=rotation) for row in held):
                    failures.append('death: GLB f45..60 hold mismatch')
            if sampler.get('interpolation', 'LINEAR') not in ('LINEAR', 'STEP'):
                failures.append(f'{clip}: animation is not sampled')
        if abs(end-CLIPS[clip]/30) > 1e-5:
            failures.append(f'{clip}: GLB duration {end}')
        if not moving:
            failures.append(f'{clip}: GLB clip is static')
        actions[clip] = {'seconds': end, 'channels': len(animation['channels']),
                         'accessor_payload_bytes': payload(indices)}
    views = {doc['accessors'][i]['bufferView'] for i in all_accessors}
    names = [node.get('name', '') for node in doc['nodes']]
    parents = {names[child]: names[parent] for parent, node in enumerate(doc['nodes'])
               for child in node.get('children', [])}
    return {'animation_accessor_bytes': payload(all_accessors),
            'animation_buffer_view_bytes': sum(doc['bufferViews'][i]['byteLength'] for i in views),
            'animation_json_bytes': len(json.dumps(doc['animations'], separators=(',', ':')).encode()),
            'untextured_glb_bytes': len(blob), 'actions': actions,
            'node_parent_hierarchy': {name: parents.get(name) for name in names},
            'skins': [[names[i] for i in skin['joints']] for skin in doc['skins']]}, failures



def reimport_snapshot(path, kind, source, roster):
    """Evaluate the actual serialized GLB in a cleared scene, including equipment."""
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)
    bpy.context.scene.render.fps = 30
    previous_actions = set(bpy.data.actions)
    bpy.ops.import_scene.gltf(filepath=str(path))
    imported_actions = set(bpy.data.actions) - previous_actions
    rig = next(obj for obj in bpy.context.scene.objects if obj.type == 'ARMATURE')
    for track in rig.animation_data.nla_tracks:
        track.mute = True
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == 'MESH'
              and any(mod.type == 'ARMATURE' and mod.object == rig for mod in obj.modifiers)]
    assert meshes, (kind, 'reimport has no skinned meshes')
    failures, result = [], {'skinned_meshes': sorted(obj.name for obj in meshes), 'actions': {}}
    for clip, expected_end in CLIPS.items():
        matches = [action for action in imported_actions if action.name.split('.')[0] == clip]
        assert len(matches) == 1, (kind, clip, 'ambiguous imported action', [a.name for a in imported_actions])
        action = matches[0]
        rig.animation_data.action = action
        rig.animation_data.action_slot = action.slots[0]
        start, end = (round(value) for value in action.frame_range)
        if (start, end) != (0, expected_end):
            failures.append(f'{clip}: reimport range {start}..{end}')
        low, high = roster.limits(kind, clip)
        overall_low, overall_high = [math.inf]*3, [-math.inf]*3
        poses, violations = {}, []
        for frame in range(start, end+1):
            bpy.context.scene.frame_set(frame)
            bpy.context.view_layer.update()
            deps = bpy.context.evaluated_depsgraph_get()
            minimum, maximum = [math.inf]*3, [-math.inf]*3
            for mesh in meshes:
                evaluated = mesh.evaluated_get(deps)
                for vertex in evaluated.data.vertices:
                    point = evaluated.matrix_world @ vertex.co
                    for axis in range(3):
                        minimum[axis] = min(minimum[axis], point[axis])
                        maximum[axis] = max(maximum[axis], point[axis])
            for axis in range(3):
                overall_low[axis] = min(overall_low[axis], minimum[axis])
                overall_high[axis] = max(overall_high[axis], maximum[axis])
                if (minimum[axis] < low[axis] - (.0005 if axis < 2 else 0)
                        or maximum[axis] > high[axis] + .0005):
                    violations.append({'frame': frame, 'axis': axis,
                                       'min_m': minimum[axis], 'max_m': maximum[axis]})
            poses[frame] = {bone.name: matrix_values(bone.matrix_basis) for bone in rig.pose.bones}
        root_constant = all(near(poses[start]['root'], pose['root']) for pose in poses.values())
        endpoints_equal = all(near(poses[start][bone], poses[end][bone]) for bone in poses[start])
        death_hold = (all(near(poses[45][bone], poses[frame][bone])
                          for frame in range(45, 61) for bone in poses[45])
                      if clip == 'death' else None)
        min_cm, max_cm = [[round(value*100, 4) for value in bounds]
                          for bounds in (overall_low, overall_high)]
        source_bounds = source['actions'][clip]
        maximum_difference_cm = max(abs(a-b) for key, imported in (('min_cm', min_cm), ('max_cm', max_cm))
                                    for a, b in zip(source_bounds[key], imported))
        if not root_constant:
            failures.append(f'{clip}: reimport root drift')
        if clip in ('idle', 'move') and not endpoints_equal:
            failures.append(f'{clip}: reimport loop discontinuity')
        if clip == 'death' and not death_hold:
            failures.append('death: reimport f45..60 hold changed')
        if violations:
            failures.append(f'{clip}: {len(violations)} reimport envelope violations')
        if maximum_difference_cm > .1:
            failures.append(f'{clip}: source/reimport bounds differ by {maximum_difference_cm:.4f} cm')
        result['actions'][clip] = {'frames': [start, end], 'frames_checked': end-start+1,
                                  'min_cm': min_cm, 'max_cm': max_cm,
                                  'maximum_source_bounds_difference_cm': round(maximum_difference_cm, 6),
                                  'envelope_violations': violations, 'constant_root': root_constant,
                                  'loop_endpoints_equal': endpoints_equal if clip in ('idle', 'move') else None,
                                  'death_hold_f45_f60': death_hold}
    return result, failures


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, default=Path(__file__).resolve().parent)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--only')
    parser.add_argument('--baseline', type=Path)
    parser.add_argument('--no-export', action='store_true', help='Only sample source poses and envelopes')
    parser.add_argument('--maximum-increase', type=int, default=1000000)
    args = parser.parse_args(sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else [])
    sys.path.insert(0, str(args.source.resolve()))
    import build
    import roster
    kinds = (args.only,) if args.only else roster.KINDS
    baseline = json.loads(args.baseline.read_text()) if args.baseline else None
    report = {'source': str(args.source.resolve()), 'blender': bpy.app.version_string,
              'fps': 30, 'export_performed': not args.no_export, 'contact_frame': 30, 'death_hold_frames': [45, 60],
              'animation_byte_definition': 'Unique actual glTF sampler input/output accessor payloads; no textures; not Unreal compressed bytes.',
              'creatures': {}, 'failures': [], 'passed': True}
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='w5-20-animation-audit-') as temporary:
        build.OUT = Path(temporary)
        for kind in kinds:
            mesh, rig, actions, triangles = build.build(kind, False)
            source, failures = source_snapshot(kind, mesh, rig, actions, roster)
            rig.animation_data.action = None
            for track in rig.animation_data.nla_tracks:
                track.mute = False
            bpy.context.scene.render.fps = 30
            if args.no_export:
                exported, export_failures = {}, []
            else:
                build.export_asset(kind, mesh, rig)
                exported, export_failures = glb_snapshot(build.OUT / (kind+'.glb'))
            failures.extend(export_failures)
            reimported = None
            if not args.no_export:
                reimported, import_failures = reimport_snapshot(build.OUT / (kind+'.glb'), kind, source, roster)
                failures.extend(import_failures)
            entry = {'source': source, 'export': exported, 'reimport': reimported, 'failures': failures, 'warnings': []}
            if baseline:
                before = baseline['creatures'][kind]
                for key in ('rig_sha256', 'triangles'):
                    if before['source'][key] != source[key]:
                        failures.append('baseline source changed: '+key)
                rest_delta = max(abs(a-b) for key in ('rest_min_cm', 'rest_max_cm')
                                 for a, b in zip(before['source'][key], source[key]))
                entry['rest_bounds_maximum_difference_cm'] = round(rest_delta, 6)
                if rest_delta > .1:
                    failures.append('rest bounds differ from baseline by more than 0.1 cm')
                if before['source']['geometry_sha256'] != source['geometry_sha256']:
                    entry['warnings'].append('Exact joined geometry hash differs; small numerical construction variation is possible. Verify unchanged geometry source files independently; this hash alone does not establish a geometry edit.')
                for key in (() if args.no_export else ('node_parent_hierarchy', 'skins')):
                    if before['export'][key] != exported[key]:
                        failures.append('baseline export changed: '+key)
                if not args.no_export:
                    entry['animation_accessor_bytes_before'] = before['export']['animation_accessor_bytes']
                    entry['animation_accessor_bytes_delta'] = (exported['animation_accessor_bytes']
                                                               - before['export']['animation_accessor_bytes'])
            entry['passed'] = not failures
            report['creatures'][kind] = entry
            report['failures'].extend(f'{kind}: {failure}' for failure in failures)
            report['passed'] = not report['failures']
            args.output.write_text(json.dumps(report, indent=2)+'\n')
            print(json.dumps({'creature': kind, 'animation_bytes': exported.get('animation_accessor_bytes'),
                              'passed': entry['passed'], 'failures': failures, 'warnings': entry['warnings']}), flush=True)
        report['total_animation_accessor_bytes'] = (None if args.no_export else
            sum(e['export']['animation_accessor_bytes'] for e in report['creatures'].values()))
        if baseline and not args.no_export:
            report['total_animation_accessor_bytes_before'] = sum(baseline['creatures'][kind]['export']['animation_accessor_bytes']
                                                                  for kind in kinds)
            report['total_animation_accessor_bytes_delta'] = (report['total_animation_accessor_bytes']
                                                              - report['total_animation_accessor_bytes_before'])
            report['maximum_animation_byte_increase'] = args.maximum_increase
            if report['total_animation_accessor_bytes_delta'] > args.maximum_increase:
                report['failures'].append('aggregate animation-byte increase exceeds requested limit')
        report['passed'] = not report['failures']
        args.output.write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps({'report': str(args.output), 'passed': report['passed'],
                      'animation_bytes': report['total_animation_accessor_bytes'],
                      'delta': report.get('total_animation_accessor_bytes_delta')}), flush=True)
    if not report['passed']:
        raise RuntimeError('Animation audit failures recorded in '+str(args.output))


if __name__ == '__main__':
    main()
