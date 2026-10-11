"""Prototype source attack motion audit; no bake/export or gameplay assertions.

Run with Blender --python-exit-code 1 --python this_file -- --output /abs/report.json.
--source selects an alternate creatures source directory for baseline comparison.
Distances use world-space bone heads AND tails, including scale-driven motion.
Readiness requires >=10% rest largest extent overall and >=8% rest height
on body/chest/head over f0/f28/f30 pairs. Slime uses >=20% silhouette
one-minus-IoU because its bone axis misses squash/stretch. f32 is diagnostic followthrough.
These are explicit prototype heuristics, not a substitute for visual review.
"""
import argparse
import itertools
import json
import math
import sys
from pathlib import Path

import bpy
import numpy as np
from mathutils import Vector

FRAMES = (0, 28, 30, 32)
OVERALL_MIN = .10
CORE_MIN = .08
SILHOUETTE_MIN = .20
MASK_SIZE = 160


def silhouette(geometry, triangles):
    """Rasterize actual evaluated faces in fixed union framing, two source views.

    Binary orthographic occupancy includes occlusion naturally through union;
    camera framing is identical at every pose, so translation remains visible.
    Pixel-centre triangle coverage is a deterministic 160px prototype measure.
    """
    result = {}
    for title, elevation, yaw in (('gameplay_55_45', 55, 45), ('three_quarter_25_45', 25, 45)):
        elev, angle = map(math.radians, (elevation, yaw))
        outward = Vector((math.cos(elev)*math.cos(angle),
                          -math.cos(elev)*math.sin(angle), math.sin(elev)))
        rotation = (-outward).to_track_quat('-Z', 'Y').inverted().to_matrix()
        basis = np.array([list(row) for row in rotation], dtype=float)
        projected = {f: (vertices @ basis.T)[:, :2] for f, vertices in geometry.items()}
        combined = np.concatenate(list(projected.values()))
        lo, hi = combined.min(axis=0), combined.max(axis=0)
        span = max(float((hi-lo).max()), 1e-9)*1.05
        center = (lo+hi)/2
        masks = {}
        for frame, vertices in projected.items():
            pixels = (vertices-center)/span*MASK_SIZE+MASK_SIZE/2
            mask = np.zeros((MASK_SIZE, MASK_SIZE), dtype=bool)
            for indices in triangles:
                points = pixels[indices]
                a, b, c = points
                if abs((b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0])) < 1e-10:
                    continue
                low = np.maximum(0, np.floor(points.min(axis=0)).astype(int))
                high = np.minimum(MASK_SIZE-1, np.ceil(points.max(axis=0)).astype(int))
                if np.any(high < low):
                    continue
                xx, yy = np.meshgrid(np.arange(low[0], high[0]+1)+.5,
                                     np.arange(low[1], high[1]+1)+.5)
                edges = []
                for index in range(3):
                    a, b = points[index], points[(index+1)%3]
                    edges.append((xx-a[0])*(b[1]-a[1])-(yy-a[1])*(b[0]-a[0]))
                coverage = ((edges[0]>=0)&(edges[1]>=0)&(edges[2]>=0)) | (
                    (edges[0]<=0)&(edges[1]<=0)&(edges[2]<=0))
                mask[low[1]:high[1]+1, low[0]:high[0]+1] |= coverage
            masks[frame] = mask
        pairs = {}
        for first, second in itertools.combinations((0, 28, 30), 2):
            union = int(np.count_nonzero(masks[first] | masks[second]))
            intersection = int(np.count_nonzero(masks[first] & masks[second]))
            pairs[f'{first}_{second}'] = {
                'one_minus_iou': round(1-intersection/union, 6) if union else 0,
                'intersection_pixels': intersection, 'union_pixels': union}
        result[title] = {'mask_size': MASK_SIZE, 'fixed_union_span_cm': round(span*100, 6),
                         'occupied_pixels': {str(f): int(np.count_nonzero(m)) for f, m in masks.items()},
                         'pairs': pairs}
    return result


def measure(kind, mesh, rig, action):
    for track in rig.animation_data.nla_tracks:
        track.mute = True
    rig.animation_data.action = action
    rig.animation_data.action_slot = action.slots[0]
    names = [b.name for b in rig.pose.bones if b.name != 'root'
             and not b.name.startswith(('VFX_', 'UI_'))]
    core = [name for name in names if name in ('body', 'chest', 'head')]
    limbs = [name for name in names if name.startswith(
        ('front', 'wing', 'hand', 'fore_', 'forearm_', 'upperarm_', 'leg0_', 'leg1_'))]
    groups = {'overall': names, 'core_body_head': core, 'foreleg_wing_hand': limbs}
    poses, geometry = {}, {}
    for frame in FRAMES:
        bpy.context.scene.frame_set(frame)
        bpy.context.view_layer.update()
        evaluated = rig.evaluated_get(bpy.context.evaluated_depsgraph_get())
        poses[frame] = {name: {endpoint: evaluated.matrix_world @ getattr(
            evaluated.pose.bones[name], endpoint) for endpoint in ('head', 'tail')}
            for name in names}
        if frame in (0, 28, 30):
            evaluated_mesh = mesh.evaluated_get(bpy.context.evaluated_depsgraph_get())
            geometry[frame] = np.array([list(evaluated_mesh.matrix_world @ v.co)
                                       for v in evaluated_mesh.data.vertices])
            if frame == 0:
                evaluated_mesh.data.calc_loop_triangles()
                triangles = np.array([list(t.vertices) for t in evaluated_mesh.data.loop_triangles])
    extents = [(b-a)/100 for a, b in zip(mesh['rest_min_cm'], mesh['rest_max_cm'])]
    height, largest = max(extents[2], 1e-9), max(max(extents), 1e-9)
    pairs = {}
    for first, second in itertools.combinations(FRAMES, 2):
        entry = {}
        for group, bones in groups.items():
            candidates = [((poses[second][name][endpoint]-poses[first][name][endpoint]).length,
                           name, endpoint) for name in bones for endpoint in ('head', 'tail')]
            if not candidates:
                entry[group] = None
                continue
            distance, name, endpoint = max(candidates)
            entry[group] = {'max_cm': round(distance*100, 6), 'bone': name,
                            'endpoint': endpoint,
                            'rest_height_fraction': round(distance/height, 6),
                            'rest_largest_extent_fraction': round(distance/largest, 6)}
        pairs[f'{first}_{second}'] = entry
    contact_pairs = ('0_28', '0_30', '28_30')
    overall = max(pairs[p]['overall']['rest_largest_extent_fraction'] for p in contact_pairs)
    core_change = max(pairs[p]['core_body_head']['rest_height_fraction'] for p in contact_pairs)
    failures = []
    if overall < OVERALL_MIN:
        failures.append('overall motion below 10% rest largest extent')
    if core_change < CORE_MIN:
        failures.append('core body/head motion below 8% rest height')
    silhouettes = silhouette(geometry, triangles)
    silhouette_change = max(p['one_minus_iou'] for view in silhouettes.values()
                            for p in view['pairs'].values())
    # The slime's horizontal body bone cannot measure vertical squash/stretch.
    # Preserve the motion result, and use actual silhouette evidence for its gate.
    bone_failures = list(failures)
    slime = kind == 'slime'
    if slime:
        failures = ([] if silhouette_change >= SILHOUETTE_MIN else
                    ['slime silhouette change below 20% one-minus-IoU'])
    return {'rest_extents_cm': [round(e*100, 6) for e in extents],
            'probe_bones': groups,
            'positions_cm': {str(f): {n: {e: [round(v*100, 6) for v in xyz]
                for e, xyz in endpoints.items()} for n, endpoints in pose.items()}
                for f, pose in poses.items()},
            'pairs': pairs, 'silhouette': silhouettes,
            'bone_readiness_passed': not bone_failures, 'bone_readiness_failures': bone_failures,
            'readiness_method': 'silhouette' if slime else 'bone_endpoints',
            'bone_limitations': 'Bone endpoints do not capture all mesh scale/deformation; slime uses silhouette readiness.',
            'passed': not failures, 'failures': failures}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, default=Path(__file__).resolve().parent)
    parser.add_argument('--only')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args(sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else [])
    if not args.output.is_absolute():
        parser.error('--output must be absolute')
    sys.path.insert(0, str(args.source.resolve()))
    import build
    import roster
    if args.only and args.only not in roster.KINDS:
        parser.error('unknown creature: '+args.only)
    report = {'source': str(args.source.resolve()), 'blender': bpy.app.version_string,
              'frames': list(FRAMES), 'bake_performed': False,
              'thresholds': {'overall_rest_largest_extent_fraction': OVERALL_MIN,
                             'core_rest_height_fraction': CORE_MIN,
                             'slime_silhouette_one_minus_iou': SILHOUETTE_MIN,
                             'readiness_pairs': ['0_28', '0_30', '28_30'],
                             'prototype_heuristic': True},
              'creatures': {}, 'passed': True, 'failures': []}
    args.output.parent.mkdir(parents=True, exist_ok=True)
    for kind in (args.only,) if args.only else roster.KINDS:
        try:
            mesh, rig, actions, _ = build.build(kind, bake_assets=False)
            entry = measure(kind, mesh, rig, actions['attack'])
        except Exception as error:
            entry = {'passed': False, 'failures': [f'{type(error).__name__}: {error}']}
        report['creatures'][kind] = entry
        report['failures'].extend(f'{kind}: {failure}' for failure in entry['failures'])
        report['passed'] = not report['failures']
        args.output.write_text(json.dumps(report, separators=(',', ':'))+'\n')
        print(json.dumps({'creature': kind, 'passed': entry['passed'],
                          'failures': entry['failures']}), flush=True)
    if not report['passed']:
        raise RuntimeError('Attack readability failures recorded in '+str(args.output))


if __name__ == '__main__':
    main()
