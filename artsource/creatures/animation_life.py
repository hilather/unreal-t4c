"""W5-20 animation craft with W5-20b attack dispatch, sampled at 30 fps on the unchanged production rigs.

Prototype presentation tuning, LH_Prototype_v1, 2026-10-11, source_url null.
Root, rest geometry, socket hierarchy, clip spans and the f30 impact stay fixed.
Curves are eased *before* integer-frame baking, not exporter Bezier overshoot.
"""
import math
import bpy
from mathutils import Vector, Quaternion, Matrix

BATS = ('bat', 'dungeon_bat', 'giant_bat', 'undead_bat')


def curve(f, keys):
    """Authored breakdowns with eased arcs, exact endpoints and deliberate holds."""
    if f <= keys[0][0]: return keys[0][1]
    for (a, x), (b, y) in zip(keys, keys[1:]):
        if f <= b:
            t=(f-a)/(b-a);t=t*t*(3-2*t)
            return x+(y-x)*t
    return keys[-1][1]


def rot(b, xyz):
    q=Quaternion((0,0,1),xyz[2])@Quaternion((0,1,0),xyz[1])@Quaternion((1,0,0),xyz[0])
    basis=b.bone.matrix_local.to_quaternion()
    b.rotation_euler=(basis.inverted()@q@basis).to_euler('XYZ')


def move(b, xyz):
    b.location=b.bone.matrix_local.to_3x3().inverted()@Vector(xyz)


def planted(b, names, offsets=None):
    """Counter the hip motion at limb roots; toe steps remain authored in place."""
    bpy.context.view_layer.update()
    for name in names:
        bone=b[name];m=bone.matrix.copy()
        m.translation=bone.bone.head_local+Vector((offsets or {}).get(name,(0,0,0)))
        bone.matrix=m
    bpy.context.view_layer.update()


def aim_segment(bone, head, tail):
    rest=bone.bone.matrix_local
    direction=(tail-head).normalized()
    q=rest.to_quaternion()@Vector((0,1,0))
    rotation=q.rotation_difference(direction)@rest.to_quaternion()
    bone.matrix=Matrix.Translation(head)@rotation.to_matrix().to_4x4()
    bpy.context.view_layer.update()


def grasp(b, side, target, big=False):
    """Two-bone analytic hand placement; baked transforms, no exported constraint."""
    upper=b[('upperarm_' if big else 'upper_')+side]
    fore=b[('forearm_' if big else 'fore_')+side]
    hand=b['hand_'+side]
    bpy.context.view_layer.update()
    shoulder=upper.head.copy()
    # Target is the wrist; preserve the authored palm-to-shaft offset separately.
    d=target-shoulder;distance=d.length;axis=d.normalized()
    a=upper.bone.length;c=fore.bone.length
    reach=min(a+c-.0001,max(abs(a-c)+.0001,distance))
    along=(a*a-c*c+reach*reach)/(2*reach)
    height=math.sqrt(max(0,a*a-along*along))
    pole=fore.head.copy()-shoulder
    across=(pole-axis*pole.dot(axis)).normalized()
    elbow=shoulder+axis*along+across*height
    aim_segment(upper,shoulder,elbow)
    aim_segment(fore,elbow,target)
    # If a stylized rest arm is short, preserve the grip with a small forearm stretch.
    fore.scale.y=(target-elbow).length/c
    bpy.context.view_layer.update()
    m=hand.matrix.copy();m.translation=target;hand.matrix=m
    bpy.context.view_layer.update()


def weapon_pose(b, location, angles, big=False):
    name='Weapon_Main' if big else 'Weapon_R';weapon=b[name]
    q=Quaternion((0,0,1),angles[2])@Quaternion((0,1,0),angles[1])@Quaternion((1,0,0),angles[0])
    rest=weapon.bone.matrix_local
    transform=Matrix.Translation(Vector(location))@q.to_matrix().to_4x4()@Matrix.Translation(-rest.translation)
    # Both wrists follow the same rigid weapon transform; hands never release mid-swing.
    for side in ('R','L'):
        target=transform@b['hand_'+side].bone.head_local
        grasp(b,side,target,big)
        hand=b['hand_'+side]
        hand.matrix=transform@hand.bone.matrix_local
        bpy.context.view_layer.update()
    weapon.matrix=transform@rest
    bpy.context.view_layer.update()


def attack(kind,b,f):
    import animation_attack_second
    if animation_attack_second.attack(kind,b,f):
        return
    if kind in ('goblin', 'goblin_warrior', 'atrocity'):
        import animation_humanoids_second
        animation_humanoids_second.attack(kind,b,f)
        return
    wind=curve(f,[(0,0),(8,.22),(20,.85),(25,1),(28,1),(30,0),(42,0)])
    hit=curve(f,[(0,0),(28,0),(30,1),(32,1),(35,.72),(42,0)])
    lag=curve(f,[(0,0),(12,.15),(26,1),(30,.1),(33,-.65),(37,-.28),(42,0)])
    if kind=='slime':
        sx=curve(f,[(0,1),(18,1.06),(25,.77),(28,.73),(30,1.10),(32,1.10),(35,.91),(38,1.025),(42,1)])
        sz=curve(f,[(0,1),(12,.78),(25,1.43),(28,1.50),(30,.48),(32,.45),(35,1.15),(38,.91),(42,1)])
        b['body'].scale=(sx,1-.13*wind,sz)
        move(b['body'],(-.024*wind+.014*hit,0,0))
        rot(b['body'],(0,-.08*wind+.10*hit,.035*lag))
    elif kind=='balork':
        turn=curve(f,[(0,0),(14,.22),(25,.70),(28,.70),(30,-.48),(32,-.68),(34,-.68),(38,-.24),(42,0)])
        move(b['body'],(-.06*wind+.065*hit,.025*lag,-.10*wind-.04*hit))
        rot(b['body'],(0,-.055*wind+.075*hit,turn*.32))
        rot(b['chest'],(0,-.10*wind+.13*hit,turn*.68))
        rot(b['head'],(0,.09*wind-.12*hit,-turn*.45))
        for s,side in ((1,'L'),(-1,'R')):
            rot(b['wing_'+side],(s*.18*wind-s*.09*hit,-.12*lag,s*(.22*wind+.10*hit)))
            rot(b['wingtip_'+side],(s*.10*lag,0,s*.14*lag))
            rot(b['thigh_'+side],(0,-.12*wind+s*.12*hit,0))
            rot(b['shin_'+side],(0,.18*wind-s*.08*hit,0))
        planted(b,['thigh_L','thigh_R'],{'thigh_L':(.13*hit,0,.05*wind),'thigh_R':(-.065*wind,0,0)})
        rot(b['tail'],(0,-.08*wind,-.28*lag));rot(b['tail_tip'],(0,.08*hit,-.30*lag))
        weapon_pose(b,(.52-.20*wind+.055*hit,-.50,.1*wind+1.29-.06*hit),(.12*wind-.10*hit,.08*lag,turn),True)


def idle(kind,b,f):
    t=math.tau*f/60;breath=math.sin(t);sway=math.sin(t*2)*.5
    look=curve(f,[(0,0),(10,0),(23,1),(32,.9),(46,-.5),(54,0),(60,0)])
    twitch=curve(f,[(0,0),(15,0),(17,1),(19,-.45),(22,0),(42,0),(44,.65),(47,0),(60,0)])
    if kind=='slime':
        b['body'].scale=(1+.028*breath,1-.026*breath,1+.075*breath+.018*math.sin(2*t))
        rot(b['body'],(0,.012*sway,.016*breath))
    elif kind in BATS:
        factor=1.25 if kind=='giant_bat' else 1
        move(b['body'],(.006*math.sin(t),0,.026*factor*breath))
        rot(b['body'],(.022*breath,.018*sway,0));rot(b['head'],(0,.035*breath,.13*look))
        for s in (-1,1):
            rot(b['wing'+str(s)],(s*(.13*breath+.025*sway),0,s*.025*look))
            lag=math.sin(t-.5)-math.sin(-.5)
            rot(b['wingtip'+str(s)],(s*.055*lag,0,s*.015*twitch))
        rot(b['jaw'],(0,.035*max(0,twitch),0))
    elif kind=='rat':
        b['body'].scale=(1,1+.012*breath,1+.023*breath)
        rot(b['head'],(0,-.035*breath-.018*twitch,.13*look))
        rot(b['jaw'],(0,.045*max(0,twitch),0))
        for i,name in enumerate(('tail','tail_mid','tail_tip')):
            rot(b[name],(0,0,.042*(math.sin(t-i*.55)-math.sin(-i*.55))))
        planted(b,['front-1','front1','rear-1','rear1'])
    elif kind=='giant_spider':
        move(b['body'],(0,0,.007*breath));rot(b['abdomen'],(0,.026*breath,.013*look))
        rot(b['head'],(0,.028*twitch,.06*look))
        for s in (-1,1):rot(b['fang'+str(s)],(0,.09*max(0,twitch),s*.025*look))
        offsets={f'leg{i}_{s}_upper':(0,0,.025*max(0,twitch) if i==0 and s==-1 else 0) for s in (-1,1) for i in range(4)}
        planted(b,offsets,offsets)
    else:
        big=kind=='balork';beast=kind=='atrocity'
        move(b['body'],(0,.012*breath,-.009*(1-math.cos(t))))
        rot(b['body'],(.008*breath,.012*breath,.009*sway))
        rot(b['head'],(0,.025*breath,.075*look))
        planted(b,['thigh_L','thigh_R'])
        if big:
            rot(b['chest'],(0,.018*breath,.012*sway))
            for s,side in ((1,'L'),(-1,'R')):
                rot(b['wing_'+side],(s*.025*breath,0,s*.05*breath))
                rot(b['wingtip_'+side],(0,0,s*.025*(math.sin(t-.6)+math.sin(.6))))
            rot(b['tail'],(0,0,.018*breath));rot(b['tail_tip'],(0,0,.024*(math.sin(t-.6)+math.sin(.6))))
        elif beast:
            for s,side in ((1,'L'),(-1,'R')):
                rot(b['upper_'+side],(0,-.018*breath,s*.016*sway))
                for j in range(4):rot(b[f'claw_{side}_{j}'],(0,.075*max(0,twitch)*(1-j*.12),0))
        else:
            for s,side in ((1,'L'),(-1,'R')):rot(b['ear_'+side],(s*.10*twitch,0,s*.025*breath))


def hit_pose(kind,b,f):
    snap=curve(f,[(0,0),(3,1),(5,.90),(9,-.18),(13,.08),(18,0)])
    lag=curve(f,[(0,0),(5,1),(7,.90),(12,-.12),(18,0)])
    if kind=='slime':
        b['body'].scale=(1-.12*snap,1+.065*snap,1-.30*snap)
        rot(b['body'],(.045*lag,-.11*snap,.045*snap))
    else:
        size=1 if kind in ('rat',)+BATS else 2 if kind=='giant_spider' else 3
        move(b['body'],(-(.004 if kind in BATS else .014)*size*snap,.004*size*lag,0))
        rot(b['body'],(.025*snap,-(.04 if kind in BATS else .065)*snap,.035*snap));rot(b['head'],(0,-.10*lag,.12*lag))
        if kind in BATS:
            for s in (-1,1):
                rot(b['wing'+str(s)],(s*.22*lag,0,s*.07*snap))
                rot(b['wingtip'+str(s)],(-s*.15*snap,0,0))
        elif kind=='balork':
            rot(b['chest'],(0,-.06*snap,-.055*lag))
            for s,side in ((1,'L'),(-1,'R')):rot(b['wing_'+side],(s*.08*lag,0,s*.09*lag))
        elif kind=='rat':
            for name in ('tail','tail_mid','tail_tip'):rot(b[name],(0,0,-.075*lag))


def death(kind,rig,f):
    b=rig.pose.bones
    fall=curve(f,[(0,0),(5,.015),(10,.07),(17,.46),(23,.91),(26,1),(30,.96),(36,1),(45,1)])
    settle=curve(f,[(0,0),(23,0),(27,1),(32,-.35),(38,.12),(45,0)])
    if kind=='slime':
        b['body'].scale=(1+.025*fall,1+.025*fall,1-.88*fall+.06*settle)
        rot(b['body'],(0,.035*settle,.02*settle))
    elif kind=='rat':
        rot(b['body'],(1.52*fall,0,.035*fall))
        b['body'].scale=(1,1-.4*fall,1-.3*fall)
        for side in (-1,1):
            for prefix in ('front','rear'):rot(b[prefix+str(side)],(0,(.55 if prefix=='front' else -.45)*fall,.12*side*settle))
        rot(b['head'],(0,.12*fall,.06*settle))
        for i,name in enumerate(('tail','tail_mid','tail_tip')):rot(b[name],(0,.08*settle,.02*fall+.03*settle))
    elif kind in BATS:
        z={'bat':1.0,'dungeon_bat':1.095,'giant_bat':1.29,'undead_bat':1.14}[kind]
        size={'bat':1,'dungeon_bat':1.12,'giant_bat':1.70,'undead_bat':1.29}[kind]
        tuck=curve(f,[(0,0),(10,0),(17,.014),(22,.014),(30,0),(45,0)])*size
        tuck+=({'dungeon_bat':.011,'giant_bat':.020}.get(kind,0)
               *curve(f,[(0,0),(10,0),(20,1),(60,1)]))
        move(b['body'],(tuck,0,-(z-.08)*fall));rot(b['body'],(.95*fall,.13*settle,.08*fall))
        b['body'].scale=(1,1-.28*fall,1-.30*fall)
        for s in (-1,1):
            rot(b['wing'+str(s)],(s*(.65*fall+.11*settle),0,s*.08*fall))
            rot(b['wingtip'+str(s)],(s*.10*settle,0,s*.95*fall))
        rot(b['head'],(0,.18*fall+.10*settle,0))
    else:
        import roster
        # Existing corpse construction remains the anchor, now with a weighted
        # collapse and secondary settling instead of a single uniform ease.
        lo,hi=0.,1.
        for _ in range(24):
            t=(lo+hi)*.5
            if t*t*(3-2*t)<fall:lo=t
            else:hi=t
        roster.module(kind).pose(kind,rig,'death',30*(lo+hi)*.5,60)
        if kind=='giant_spider':
            rot(b['abdomen'],(0,.10*settle,.06*fall))
            for s in (-1,1):rot(b['fang'+str(s)],(0,.35*fall+.12*settle,0))
        elif kind=='balork':
            rot(b['head'],(0,.65*fall+.10*settle,.08*fall))
            rot(b['tail_tip'],(0,.16*settle,.14*fall))
        elif kind=='atrocity':
            for side in ('L','R'):
                for j in range(4):rot(b[f'claw_{side}_{j}'],(0,.25*fall+.12*settle,0))
        else:
            for s,side in ((1,'L'),(-1,'R')):rot(b['ear_'+side],(s*.15*settle,0,s*.14*fall))


def pose(kind,rig,clip,frame,end):
    if clip=='move':return False  # Established gait, speed and loop remain intact.
    b=rig.pose.bones
    if clip=='attack':attack(kind,b,frame)
    elif clip=='idle':idle(kind,b,frame)
    elif clip=='hit':hit_pose(kind,b,frame)
    elif clip=='death':death(kind,rig,frame)
    else:return False
    return True
