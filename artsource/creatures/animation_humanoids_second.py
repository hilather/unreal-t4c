"""W5-20b hip-driven attacks on the existing one-spine humanoid rigs.

Prototype presentation tuning, LH_Prototype_v1, 2026-10-11, source_url null.
No rig, rest mesh, socket, clip length or contact-frame changes. The body bone
is the only spine joint: its hip pivot carries the shoulders through the turn.
"""
import math
import bpy
from mathutils import Matrix, Vector


def plant_feet(b, offsets):
    """Solve the unchanged legs from moving hips to authored ankle contacts.

    Unlike pinning a thigh root, ankle placement keeps the pelvis connected
    during the large weight transfer. This is baked FK, with no new constraint.
    """
    from animation_life import aim_segment
    bpy.context.view_layer.update()
    for side, offset in offsets.items():
        thigh, shin, foot = (b[prefix + side] for prefix in ('thigh_', 'shin_', 'foot_'))
        hip = thigh.head.copy()
        target = foot.bone.head_local + Vector(offset)
        direction = target - hip
        axis = direction.normalized()
        a, c = thigh.bone.length, shin.bone.length
        reach = min(a + c - .0001, max(abs(a - c) + .0001, direction.length))
        along = (a*a - c*c + reach*reach) / (2*reach)
        height = math.sqrt(max(0, a*a - along*along))
        # The rest knee determines the bend plane, avoiding a first-frame pop.
        pole = shin.bone.head_local - thigh.bone.head_local
        across = (pole - axis * pole.dot(axis)).normalized()
        knee = hip + axis * along + across * height
        aim_segment(thigh, hip, knee)
        aim_segment(shin, knee, target)
        shin.scale.y = (target - knee).length / c
        bpy.context.view_layer.update()
        foot.matrix = Matrix.Translation(target) @ foot.bone.matrix_local.to_3x3().to_4x4()
        bpy.context.view_layer.update()


def attack(kind, b, f):
    from animation_life import curve, rot, move, weapon_pose
    # Build clears every pose channel first. Avoid an IK rest-pose discrepancy
    # at either end; the loop/transition boundary remains the original rest.
    if f <= 0 or f >= 42:
        return

    heavy = kind == 'atrocity'
    warrior = kind == 'goblin_warrior'
    wind = curve(f, [(0, 0), (8, .12), (20, .70 if heavy else .88),
                     (26, 1), (28, 1), (30, 0), (42, 0)])
    drive = curve(f, [(0, 0), (28, 0), (30, 1), (33, 1.20 if heavy else 1.08),
                      (36, .82 if heavy else .55), (39, .24), (42, 0)])
    lag = curve(f, [(0, 0), (12, .25), (26, 1), (29, .65),
                    (32, -.85), (35, -.70), (39, .15), (42, 0)])
    # Lead with the neck while the hips finish committing to the impact.
    lead = curve(f, [(0, 0), (27, 0), (29, 1), (30, .72),
                     (33, .25), (38, -.18), (42, 0)])
    step = curve(f, [(0, 0), (24, 0), (28, .40), (30, 1),
                     (35, 1), (39, .4), (42, 0)])
    lift = curve(f, [(0, 0), (22, 0), (26, 1), (28, .85),
                     (30, 0), (35, 0), (38, .65), (42, 0)])

    if heavy:
        move(b['body'], (-.16*wind + .20*drive, -.025*wind + .04*drive,
                         -.14*wind - .10*drive))
        rot(b['body'], (.055*lag, -.31*wind + .39*drive,
                        -.62*wind + .47*drive))
        rot(b['head'], (-.045*lag, .18*wind - .18*drive - .08*lead,
                        .26*wind - .18*drive))
        move(b['head'], (.035*lead, 0, -.012*lead))
        plant_feet(b, {'L': (.24*step, .018*step, .10*lift),
                       'R': (-.065*wind, -.012*wind, 0)})
        for s, side in ((1, 'L'), (-1, 'R')):
            # The left shoulder leads by two frames; both claws arrive at f30.
            raise_arm = curve(f, [(0, 0), (14 if s == 1 else 16, .25),
                                  (25 if s == 1 else 27, 1), (28, 1),
                                  (30, 0), (42, 0)])
            rake = curve(f, [(0, 0), (28, 0), (30, 1),
                             (33, 1.22 if s == 1 else 1.14), (36, .80), (42, 0)])
            rot(b['upper_'+side], (-s*.32*raise_arm + s*.06*rake,
                                   -1.10*raise_arm - .70*rake,
                                   s*.14*raise_arm - s*.20*rake))
            rot(b['fore_'+side], (0, -.36*raise_arm + .04*rake,
                                  -s*.08*raise_arm - s*.12*rake))
            rot(b['hand_'+side], (0, .18*raise_arm - .15*rake,
                                  s*.10*lag))
            for j in range(4):
                rot(b[f'claw_{side}_{j}'], (0, -.20*raise_arm + .25*rake
                                           + .035*j*lag, 0))
        return

    move(b['body'], (-.13*wind + .16*drive, -.025*wind + .025*drive,
                     -.080*wind - .055*drive))
    rot(b['body'], (.04*lag, -.24*wind + .38*drive,
                    (-.60 if warrior else -.70)*wind + .48*drive))
    rot(b['head'], (-.025*lag, .12*wind - .13*drive - .055*lead,
                    .27*wind - .16*drive))
    move(b['head'], (.022*lead, 0, 0))
    for s, side in ((1, 'L'), (-1, 'R')):
        rot(b['ear_'+side], (s*.22*lag, -.045*lead, s*.16*lag))
    plant_feet(b, {'L': (.19*step, .015*step, .065*lift),
                   'R': (-.050*wind, -.010*wind, 0)})

    if warrior:
        # Pull the forked polearm above the trailing shoulder, then chop.
        pitch = curve(f, [(0, 0), (15, -.30), (25, -.67), (28, -.67),
                          (30, 1.12), (33, 1.34), (36, 1.00), (42, 0)])
        location = (.391 - .32*wind + .19*drive,
                    -.30 + .10*wind + .035*drive,
                    .758 + .27*wind - .015*drive)
        yaw = -.28*wind + .19*drive
    else:
        # The ordinary goblin levels its spear during the coil and then thrusts
        # it as the hip and leading shoulder travel onto the advanced foot.
        pitch = curve(f, [(0, 0), (12, .45), (24, 1.15), (28, 1.15),
                          (30, 1.40), (33, 1.49), (36, 1.18), (42, 0)])
        location = (.391 - .35*wind + .25*drive,
                    -.30 - .035*wind + .08*drive,
                    .758 + .10*wind + .010*drive)
        yaw = -.32*wind + .15*drive
    # Existing world-space weapon solver maintains both palm-to-shaft offsets.
    weapon_pose(b, location, (0, pitch, yaw))
