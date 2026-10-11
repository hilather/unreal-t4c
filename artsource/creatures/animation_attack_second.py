"""W5-20b animal attacks on existing joints, Prototype presentation tuning.

LH_Prototype_v1 / 2026-10-11 / source_url null. Metres, +X target, f30 contact.
No root motion, rig/rest changes or gameplay reach is implied by these lunges.
"""
import math
import bpy
from mathutils import Vector


def spider_leg(b, index, side, ankle, pole):
    """Place a two-segment leg against a world-space ankle; retain joint lengths."""
    from animation_life import aim_segment
    upper = b[f'leg{index}_{side}_upper']
    lower = b[f'leg{index}_{side}_lower']
    bpy.context.view_layer.update()
    shoulder = upper.head.copy()
    delta = Vector(ankle) - shoulder
    a, c = upper.bone.length, lower.bone.length
    distance = min(a+c-.0001, max(abs(a-c)+.0001, delta.length))
    axis = delta.normalized()
    along = (a*a-c*c+distance*distance)/(2*distance)
    across = Vector(pole)-shoulder
    across = (across-axis*across.dot(axis)).normalized()
    knee = shoulder+axis*along+across*math.sqrt(max(0, a*a-along*along))
    target = shoulder+axis*distance
    aim_segment(upper, shoulder, knee)
    aim_segment(lower, knee, target)
    toe = b[f'leg{index}_{side}_tip']
    # Restore tarsus orientation in world space after the lower leg solve.
    direction = toe.bone.tail_local-toe.bone.head_local
    aim_segment(toe, target, target+direction)


def attack(kind, b, f):
    from animation_life import curve, rot, move
    if kind not in ('rat', 'giant_spider', 'bat', 'dungeon_bat', 'giant_bat', 'undead_bat'):
        return False
    if f <= 0 or f >= 42:
        return True
    wind=curve(f,[(0,0),(10,.22),(22,.90),(26,1),(28,1),(30,0),(42,0)])
    strike=curve(f,[(0,0),(28,0),(30,1),(32,.95),(35,.35),(39,0),(42,0)])
    if kind == 'rat':
        travel=curve(f,[(0,0),(12,-.035),(24,-.11),(28,-.11),(30,.19),(32,.19),(35,.09),(38,-.025),(42,0)])
        hop=curve(f,[(0,0),(22,-.025),(28,-.025),(29,.075),(30,.035),(32,.025),(35,.105),(38,.045),(42,0)])
        move(b['body'],(travel,0,hop))
        rot(b['body'],(0,-.32*wind+.12*strike,-.065*wind+.035*strike))
        move(b['head'],(.045*strike,0,0))
        rot(b['head'],(0,-.19*wind+.13*strike,0))
        rot(b['jaw'],(0,curve(f,[(0,0),(18,.20),(26,.68),(28,.68),(30,.02),(32,.02),(35,.14),(42,0)]),0))
        for i,name in enumerate(('tail','tail_mid','tail_tip')):
            whip=curve(f-i,[(0,0),(18,.12),(26,.38),(28,.38),(31,-.46),(34,-.38),(38,.10),(40,0)])
            rot(b[name],(0,-.10*wind,whip))
        for s in (-1,1):
            rot(b['front'+str(s)],(0,-.48*wind+.32*strike,0))
            rot(b['rear'+str(s)],(0,.35*wind-.45*strike,0))
    elif kind == 'giant_spider':
        move(b['body'],(-.13*wind+.26*strike,0,.31*wind-.025*strike))
        rot(b['body'],(0,-.60*wind+.16*strike,.035*wind))
        # The abdomen counterbalances the rear-up, then visibly kicks upward.
        kick=curve(f,[(0,0),(28,0),(30,1),(32,1.12),(36,.20),(42,0)])
        rot(b['abdomen'],(0,.30*wind+.48*kick,-.05*wind))
        move(b['head'],(.10*strike,0,0))
        rot(b['head'],(0,-.14*wind+.25*strike,0))
        bpy.context.view_layer.update()
        for s in (-1,1):
            for i in range(4):
                ankle=b[f'leg{i}_{s}_lower'].bone.tail_local.copy()
                if i < 2:
                    # Both front pairs clear the floor; wide palms/forelegs read
                    # as a raised threat rather than a small local joint wiggle.
                    ankle.x += (-.12 if i==0 else -.05)*wind+.28*strike
                    ankle.y += s*(.10 if i==0 else .18)*wind
                    ankle.z += (.92 if i==0 else .75)*wind
                else:
                    ankle.x += .04*strike
                pole=b[f'leg{i}_{s}_lower'].bone.head_local.copy()
                pole.z += (1.15 if i<2 else .18)*wind
                spider_leg(b,i,s,ankle,pole)
            rot(b['fang'+str(s)],(0,.55*wind-.46*strike,s*.14*wind))
    else:
        # Independent breakdown timing: giant hangs longer after its heavy dive;
        # undead arrests/restarts its rise and rebounds in two abrupt impulses.
        size={'bat':1.,'dungeon_bat':1.12,'giant_bat':1.55,'undead_bat':1.18}[kind]
        profiles={
            'bat': ([(0,0),(19,.10),(27,.12),(30,-.22),(32,-.25),(36,.10),(42,0)],
                    [(0,0),(18,.30),(27,.56),(30,-.78),(32,-.66),(36,.34),(42,0)]),
            'dungeon_bat': ([(0,0),(15,.07),(24,.15),(28,.13),(30,-.25),(31,-.28),(35,.13),(38,.06),(42,0)],
                            [(0,0),(14,.25),(24,.67),(28,.60),(30,-.83),(32,-.48),(35,.41),(42,0)]),
            'giant_bat': ([(0,0),(12,.02),(24,.11),(28,.13),(30,-.23),(33,-.27),(38,.065),(42,0)],
                         [(0,0),(16,.17),(26,.48),(28,.50),(30,-.62),(33,-.70),(38,.22),(42,0)]),
            'undead_bat': ([(0,0),(12,.05),(16,.05),(19,.14),(24,.10),(28,.16),(30,-.24),(32,-.24),(34,.07),(36,.035),(38,.10),(42,0)],
                          [(0,0),(12,.36),(17,.36),(20,.66),(24,.43),(28,.65),(30,-.91),(32,-.91),(34,.37),(36,.12),(38,.26),(42,0)])}
        rise_keys, flap_keys=profiles[kind]
        rise=curve(f,rise_keys)*size
        travel=curve(f,[(0,0),(22,-.06),(28,-.09),(30,.16),(32,.18),(36,.04),(42,0)])*size
        move(b['body'],(travel,0,rise))
        rot(b['body'],(.09*wind,-.38*wind+.42*strike,.08*wind-.045*strike))
        rot(b['head'],(0,.13*wind-.19*strike,-.05*wind))
        rot(b['jaw'],(0,curve(f,[(0,0),(22,.28),(28,.64),(30,.02),(32,.02),(37,.10),(42,0)]),0))
        for s in (-1,1):
            flap=curve(f,flap_keys)
            if kind=='undead_bat' and s==1:
                flap=curve(f-.7,flap_keys)  # slightly asymmetric broken timing
            rot(b['wing'+str(s)],(s*flap,0,s*.045*wind))
            tip=curve(f,[(0,0),(19,.12),(28,.36),(30,-.06),(33,-.47),(37,.21),(42,0)])
            rot(b['wingtip'+str(s)],(s*tip,0,-s*.10*wind))
    return True
