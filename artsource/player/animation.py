"""Original prototype player rig and presentation clips, in metres, Z up.

The caller owns geometry, weights and export. NLA tracks are muted while
authoring; unmute them for the glTF NLA_TRACKS exporter.
"""
import math

import bpy
from mathutils import Quaternion, Vector


def make_rig(bones, meshes):
    """Build the supplied ordered rest skeleton and bind already weighted meshes."""
    bpy.ops.object.select_all(action='DESELECT')
    data = bpy.data.armatures.new('player_skeleton')
    rig = bpy.data.objects.new('player_rig', data)
    bpy.context.collection.objects.link(rig)
    bpy.context.view_layer.objects.active = rig
    rig.select_set(True)
    bpy.ops.object.mode_set(mode='EDIT')
    for name, (head, tail, parent) in bones.items():
        bone = data.edit_bones.new(name)
        bone.head, bone.tail = head, tail
        if parent:
            bone.parent = data.edit_bones[parent]
        bone.use_deform = not name.startswith('Socket.') and name != 'root'
    bpy.ops.object.mode_set(mode='OBJECT')
    for mesh in meshes:
        world = mesh.matrix_world.copy()
        mesh.parent = rig
        mesh.matrix_world = world
        modifier = mesh.modifiers.new('Skin', 'ARMATURE')
        modifier.object = rig
    for bone in rig.pose.bones:
        bone.rotation_mode = 'QUATERNION'
    return rig


def _rotate(rig, name, axis, angle):
    bone = rig.pose.bones.get(name)
    if bone is not None:
        local_axis = bone.bone.matrix_local.to_3x3().inverted() @ Vector(axis)
        bone.rotation_quaternion = Quaternion(local_axis.normalized(), angle)


def _translate(rig, name, delta):
    bone = rig.pose.bones[name]
    bone.location = bone.bone.matrix_local.to_3x3().inverted() @ Vector(delta)


def _envelope(frame, keys):
    for (a, va), (b, vb) in zip(keys, keys[1:]):
        if frame <= b:
            return va + (vb - va) * (frame - a) / (b - a)
    return keys[-1][1]


def _ground(rig, meshes):
    """Put the lowest evaluated surface on the floor by moving the pelvis."""
    bpy.context.view_layer.update()
    depsgraph = bpy.context.evaluated_depsgraph_get()
    minimum = math.inf
    for mesh in meshes:
        evaluated = mesh.evaluated_get(depsgraph)
        minimum = min(minimum, min(
            ((evaluated.matrix_world @ vertex.co).z
             for vertex in evaluated.data.vertices), default=math.inf))
    if math.isfinite(minimum):
        pelvis = rig.pose.bones['pelvis']
        # Its parent is the stationary root, so the rest basis maps translation.
        pelvis.location += pelvis.bone.matrix_local.to_3x3().inverted() @ Vector((0, 0, -minimum))
        bpy.context.view_layer.update()


def _world_axis(rig, name, direction):
    """Orient a bone's longitudinal +Y axis in armature/world space."""
    bone = rig.pose.bones[name]
    rest = bone.bone.matrix_local.to_quaternion()
    axis = bone.bone.tail_local - bone.bone.head_local
    desired = axis.normalized().rotation_difference(Vector(direction).normalized()) @ rest
    inherited = rest
    if bone.parent:
        inherited = (bone.parent.matrix.to_quaternion()
                     @ bone.parent.bone.matrix_local.to_quaternion().inverted() @ rest)
    bone.rotation_quaternion = inherited.inverted() @ desired
    bpy.context.view_layer.update()


def _bow_pose(rig, draw):
    """Bake a two-segment reach into FK quaternions; no exported IK constraint."""
    bpy.context.view_layer.update()
    scale = rig.data.bones['head'].tail_local.z / 1.74
    for side, target in [('L', (.40, -.026, 1.53)), ('R', (.02, -.035, 1.57))]:
        upper = rig.pose.bones['upper_arm.' + side]
        fore = rig.pose.bones['forearm.' + side]
        shoulder = upper.head.copy()
        reach = Vector(target) * scale - shoulder
        first, second = upper.bone.length, fore.bone.length
        distance = min(first + second - 1e-5, max(abs(first - second) + 1e-5, reach.length))
        direction = reach.normalized()
        # The pole stays lateral to the torso and slightly below the wrist.
        pole = Vector((0, 1 if side == 'L' else -1, -.35))
        sideways = pole - direction * pole.dot(direction)
        if sideways.length < 1e-6:
            sideways = Vector((0, 0, 1)).cross(direction)
        sideways.normalize()
        along = (first * first - second * second + distance * distance) / (2 * distance)
        height = math.sqrt(max(0, first * first - along * along))
        elbow = shoulder + direction * along + sideways * height
        wrist = shoulder + direction * distance
        _world_axis(rig, upper.name, elbow - shoulder)
        _world_axis(rig, fore.name, wrist - elbow)
        _world_axis(rig, 'hand.' + side, (1, 0, 0))
    _world_axis(rig, 'Socket.Bow.L', (0, 0, 1))
    _world_axis(rig, 'Socket.Arrow', (1, 0, 0))
    names = ['upper_arm.L', 'forearm.L', 'hand.L', 'upper_arm.R', 'forearm.R',
             'hand.R', 'Socket.Bow.L', 'Socket.Arrow']
    identity = Quaternion((1, 0, 0, 0))
    full = {name: rig.pose.bones[name].rotation_quaternion.copy() for name in names}
    for name in names:
        rig.pose.bones[name].rotation_quaternion = identity.slerp(full[name], draw)
    bpy.context.view_layer.update()


def _pose(rig, clip, frame, end):
    for bone in rig.pose.bones:
        bone.location = (0, 0, 0)
        bone.rotation_quaternion = (1, 0, 0, 0)
        bone.scale = (1, 1, 1)
    phase = math.tau * frame / end
    if clip == 'idle':
        breath = .5 - .5 * math.cos(phase)
        rig.pose.bones['chest'].scale = (1 + .003 * breath, 1 + .005 * breath, 1 + .002 * breath)
    elif clip in ('move', 'run'):
        running = clip == 'run'
        swing = .64 if running else .34
        for side, sign in [('L', 1), ('R', -1)]:
            step = math.sin(phase) * sign
            lift = max(0, -step)
            _rotate(rig, 'thigh.' + side, (0, 1, 0), -swing * step)
            _rotate(rig, 'shin.' + side, (0, 1, 0), -(1.05 if running else .52) * lift)
            _rotate(rig, 'foot.' + side, (0, 1, 0), swing * step + (.45 if running else .2) * lift)
            _rotate(rig, 'upper_arm.' + side, (0, 1, 0), (.52 if running else .29) * step)
            _rotate(rig, 'forearm.' + side, (0, 1, 0), -.75 if running else -.14)
        _rotate(rig, 'spine', (0, 1, 0), .10 if running else .015)
    elif clip == 'melee':
        wind = _envelope(frame, [(0, 0), (7, 1), (11, 1), (15, 0), (24, 0)])
        strike = _envelope(frame, [(0, 0), (11, 0), (15, 1), (18, .8), (24, 0)])
        _rotate(rig, 'chest', (0, 0, 1), -.32 * wind + .32 * strike)
        _rotate(rig, 'upper_arm.R', (0, 1, 0), -2.25 * wind - .95 * strike)
        _rotate(rig, 'forearm.R', (0, 1, 0), -.95 * wind - .18 * strike)
        _rotate(rig, 'upper_arm.L', (0, 1, 0), -.25 * wind - .4 * strike)
        _rotate(rig, 'forearm.L', (0, 1, 0), -.5 * max(wind, strike))
    elif clip == 'bow':
        draw = _envelope(frame, [(0, 0), (10, .85), (17, 1), (21, 1), (23, .75), (30, 0)])
        _bow_pose(rig, draw)
    elif clip == 'cast':
        charge = _envelope(frame, [(0, 0), (9, .8), (17, 1), (21, 1), (24, .7), (30, 0)])
        for side, sign in [('L', 1), ('R', -1)]:
            bone = rig.pose.bones['upper_arm.' + side]
            rest = bone.bone.matrix_local.to_quaternion()
            world = Quaternion((1, 0, 0), sign * .2 * charge) @ Quaternion((0, 1, 0), -2.1 * charge)
            bone.rotation_quaternion = rest.inverted() @ world @ rest
            _rotate(rig, 'forearm.' + side, (0, 1, 0), -.35 * charge)
            _rotate(rig, 'hand.' + side, (0, 1, 0), .3 * charge)
    elif clip == 'hit':
        recoil = _envelope(frame, [(0, 0), (2, 1), (6, 0)])
        _rotate(rig, 'spine', (0, 1, 0), -.15 * recoil)
        _rotate(rig, 'head', (0, 1, 0), -.12 * recoil)
        for side in ('L', 'R'):
            _rotate(rig, 'forearm.' + side, (0, 1, 0), -.35 * recoil)
    elif clip == 'death':
        t = min(1, frame / 19.5)
        fall = t * t * (3 - 2 * t)
        _rotate(rig, 'pelvis', (0, 1, 0), math.pi / 2 * fall)
        _rotate(rig, 'spine', (0, 1, 0), .08 * fall)
        for side in ('L', 'R'):
            # Keep hands above the fallen torso's underside so grounding lands
            # the chest, rather than suspending it on the fingertips.
            _rotate(rig, 'upper_arm.' + side, (0, 1, 0), .12 * fall)
            _rotate(rig, 'forearm.' + side, (0, 1, 0), .12 * fall)
            _rotate(rig, 'shin.' + side, (0, 1, 0), -.18 * fall)


def make_actions(rig, meshes):
    """Return eight sampled 30 fps actions and matching export NLA tracks."""
    bpy.context.scene.render.fps = 30
    rig.animation_data_create()
    actions = {}
    for clip, end in [('idle', 60), ('move', 30), ('run', 24),
                      ('melee', 24), ('bow', 30), ('cast', 30),
                      ('hit', 6), ('death', 30)]:
        action = bpy.data.actions.new('player_' + clip)
        actions[clip] = action
        rig.animation_data.action = action
        for frame in range(end + 1):
            bpy.context.scene.frame_set(frame)
            # Endpoint reuses frame zero exactly, including grounding.
            sample = 0 if clip in ('idle', 'move', 'run') and frame == end else frame
            _pose(rig, clip, sample, end)
            _ground(rig, meshes)
            for bone in rig.pose.bones:
                if bone.name == 'root':
                    continue
                for prop in ('location', 'rotation_quaternion', 'scale'):
                    bone.keyframe_insert(data_path=prop, frame=frame, group=bone.name)
        for layer in action.layers:
            for strip in layer.strips:
                for bag in strip.channelbags:
                    for curve in bag.fcurves:
                        for key in curve.keyframe_points:
                            key.interpolation = 'LINEAR'
        track = rig.animation_data.nla_tracks.new()
        track.name = clip
        track.strips.new(clip, 0, action)
        track.mute = True
    rig.animation_data.action = None
    _pose(rig, 'idle', 0, 60)
    _ground(rig, meshes)
    bpy.context.scene.frame_set(0)
    return actions
