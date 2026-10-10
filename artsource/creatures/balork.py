"""Original Balork sculpt, pigmented wing membranes and grounded demon rig.

Prototype presentation W5-08e / LH_Prototype_v1 / 2026-10-10 / source_url null.
The concept is viewed by the author only; this module reads no image or asset.
"""
import math
import random
import bpy
from mathutils import Vector
import models as m


def material(name, dark, light, rough=.65, scale=9, metal=0):
    mat=bpy.data.materials.new('Balork '+name);mat.use_nodes=True
    n,l=mat.node_tree.nodes,mat.node_tree.links;p=n.get('Principled BSDF')
    p.inputs['Roughness'].default_value=rough;p.inputs['Metallic'].default_value=metal
    tc=n.new('ShaderNodeTexCoord');noise=n.new('ShaderNodeTexNoise')
    noise.inputs['Scale'].default_value=scale;noise.inputs['Detail'].default_value=3.5
    noise.inputs['Roughness'].default_value=.72;l.new(tc.outputs['Object'],noise.inputs[0])
    ramp=m.ramp(n,[(.22,dark),(.75,light)])
    l.new(noise.outputs['Fac'],ramp.inputs[0]);l.new(ramp.outputs[0],p.inputs['Base Color'])
    grain=n.new('ShaderNodeTexNoise');grain.inputs['Scale'].default_value=160;grain.inputs['Detail'].default_value=2
    l.new(tc.outputs['Object'],grain.inputs[0])
    bump=n.new('ShaderNodeBump');bump.inputs['Strength'].default_value=.28;bump.inputs['Distance'].default_value=.003
    l.new(grain.outputs['Fac'],bump.inputs['Height']);l.new(bump.outputs[0],p.inputs['Normal'])
    if name=='wing pigmentation':
        # Coarse rust-red islands crossed by finer organic veins, never emission.
        ramp.color_ramp.elements[0].position=.41;ramp.color_ramp.elements[1].position=.60
        vor=n.new('ShaderNodeTexVoronoi');vor.feature='DISTANCE_TO_EDGE';vor.inputs['Scale'].default_value=24
        l.new(tc.outputs['Object'],vor.inputs[0]);vein=m.ramp(n,[(.012,(.12,.08,.075)),(.045,(1,1,1))])
        l.new(vor.outputs['Distance'],vein.inputs[0]);mix=n.new('ShaderNodeMixRGB');mix.blend_type='MULTIPLY';mix.inputs[0].default_value=.6
        l.new(ramp.outputs[0],mix.inputs[1]);l.new(vein.outputs[0],mix.inputs[2]);l.new(mix.outputs[0],p.inputs['Base Color'])
    return mat


def fuse(parts,mat,bones):
    bpy.ops.object.select_all(action='DESELECT')
    for o in parts:o.select_set(True)
    obj=parts[0];bpy.context.view_layer.objects.active=obj;bpy.ops.object.join()
    for o in parts:m.PARTS.remove(o)
    obj.name='Balork continuous anatomy';obj.data.remesh_voxel_size=.012
    bpy.ops.object.voxel_remesh()
    mod=obj.modifiers.new('Muscle blending','SMOOTH');mod.factor=.75;mod.iterations=4;bpy.ops.object.modifier_apply(modifier=mod.name)
    mod=obj.modifiers.new('Skin budget','DECIMATE');mod.ratio=min(1,5700/(2*len(obj.data.polygons)));bpy.ops.object.modifier_apply(modifier=mod.name)
    obj.data.materials.clear();obj.data.materials.append(mat)
    for f in obj.data.polygons:f.use_smooth=True
    obj.vertex_groups.clear()
    names=['body','chest','head']+[f'{limb}_{side}' for side in ('L','R') for limb in ('upperarm','forearm','hand','thigh','shin','foot')]
    groups={name:obj.vertex_groups.new(name=name) for name in names}
    def distance(p,a,b):
        a,b=Vector(a),Vector(b);d=b-a;t=max(0,min(1,(p-a).dot(d)/d.length_squared))
        return (p-a-d*t).length
    for v in obj.data.vertices:
        p=v.co
        if p.z>2.19:names2=['head','chest']
        elif abs(p.y)>.43 and p.z>1.10:names2=['chest']+[f'{limb}_{"L" if p.y>0 else "R"}' for limb in ('upperarm','forearm','hand')]
        elif p.z<1.21:names2=['body']+[f'{limb}_{"L" if p.y>0 else "R"}' for limb in ('thigh','shin','foot')]
        else:names2=['body','chest']
        ds=sorted((distance(p,*bones[name][:2]),name) for name in names2)[:2]
        ws=[math.exp(-d*18) for d,name in ds];total=sum(ws)
        for (d,name),w in zip(ds,ws):groups[name].add([v.index],w/total,'REPLACE')
    m.PARTS.append(obj)
    return obj


def blade(name,outline,x,thick,mat,bone='Weapon_Main'):
    verts=[(x+d,y,z) for d in (-thick/2,thick/2) for y,z in outline]
    n=len(outline);faces=[tuple(range(n-1,-1,-1)),tuple(range(n,2*n))]
    faces.extend((j,(j+1)%n,(j+1)%n+n,j+n) for j in range(n))
    ob=m.mesh(name,verts,faces,mat,bone)
    for f in ob.data.polygons:f.use_smooth=False
    return ob


def create(kind='balork'):
    assert kind=='balork';random.seed(518);m.PARTS=[]
    skin=material('oxblood hide',(.015,.002,.002),(.13,.018,.010),.69,5)
    face=material('warm face and muscle ridges',(.06,.008,.005),(.20,.035,.016),.67,10)
    plate=material('dark carapace',(.014,.011,.012),(.075,.042,.035),.52,12)
    horn=material('aged striated horn',(.12,.074,.039),(.52,.37,.19),.48,24)
    hoof=material('cloven hooves',(.014,.011,.010),(.08,.052,.035),.47,18)
    wing=material('wing pigmentation',(.027,.009,.009),(.46,.049,.022),.69,6.7)
    iron=material('forged iron',(.055,.065,.067),(.20,.22,.20),.47,35,.72)
    edge=material('worn axe edges',(.21,.24,.23),(.58,.56,.45),.36,45,.8)
    leather=material('tattered hide skirt',(.028,.016,.010),(.095,.052,.024),.92,18)
    gold=material('tarnished bronze',(.07,.04,.012),(.32,.19,.046),.5,28,.62)
    mouth=material('mouth and nostrils',(.004,.0004,.0003),(.016,.003,.002),.77,20)
    eye=material('amber eyes',(.36,.10,.002),(.95,.44,.012),.24,15)
    bones={'root':((0,0,0),(0,0,.18),None),'body':((0,0,1.18),(0,0,1.66),'root'),
           'chest':((0,0,1.65),(.04,0,2.12),'body'),'head':((.04,0,2.13),(.16,0,2.56),'chest')}
    for sign,side in ((1,'L'),(-1,'R')):
        def q(x,y,z):return (x,sign*y,z)
        bones.update({f'upperarm_{side}':(q(0,.43,2.04),q(.00,.69,1.63),'chest'),
                      f'forearm_{side}':(q(.00,.69,1.63),q(.45,.55,1.30+(.20 if sign>0 else 0)),f'upperarm_{side}'),
                      f'hand_{side}':(q(.45,.55,1.30+(.20 if sign>0 else 0)),q(.51,.48,1.23+(.20 if sign>0 else 0)),f'forearm_{side}'),
                      f'thigh_{side}':(q(0,.25,1.17),q(.18,.45,.73),'body'),
                      f'shin_{side}':(q(.18,.45,.73),q(-.08,.49,.25),f'thigh_{side}'),
                      f'foot_{side}':(q(-.08,.49,.25),q(.24,.49,.09),f'shin_{side}'),
                      f'wing_{side}':(q(-.19,.37,2.03),q(-.35,.94,2.68),'chest'),
                      f'wingtip_{side}':(q(-.35,.94,2.68),q(-.41,2.1,2.36),f'wing_{side}')})
    bones.update({'tail':((-.18,0,1.12),(-.64,.2,.65),'body'),
                  'tail_tip':((-.64,.2,.65),(-.88,.55,.35),'tail'),
                  'Weapon_Main':((.52,-.50,1.29),(.52,-.22,1.35),'hand_R'),
                  'Weapon_Support':((.52,.50,1.49),(.52,.62,1.514),'Weapon_Main'),
                  'VFX_WeaponTip_A':((.52,1.96,1.90),(.52,2.06,1.90),'Weapon_Main'),
                  'VFX_WeaponTip_B':((.52,-1.97,.94),(.52,-2.07,.94),'Weapon_Main'),
                  'VFX_Hit':((.28,0,1.92),(.38,0,1.92),'chest'),
                  'UI_Anchor':((0,0,3.05),(0,0,3.1),'root'),
                  'WingTip_L':((-.41,2.1,2.36),(-.41,2.15,2.36),'wingtip_L'),
                  'WingTip_R':((-.41,-2.1,2.36),(-.41,-2.15,2.36),'wingtip_R')})
    # Muscular load-bearing forms overlap generously and are unified before skinning.
    m.ellipsoid('Pelvis',(0,0,1.18),(.23,.34,.27),skin)
    m.ellipsoid('Waist',(.02,0,1.50),(.235,.31,.33),skin)
    m.ellipsoid('Thorax',(-.055,0,1.90),(.29,.48,.38),skin)
    m.ellipsoid('Neck',(.03,0,2.18),(.19,.21,.25),skin)
    m.ellipsoid('Skull',(.11,0,2.39),(.18,.19,.24),skin,'head',20,12)
    m.ellipsoid('Tapered jaw',(.25,0,2.24),(.17,.128,.13),skin,'head')
    m.ellipsoid('Forward cheek ridge',(.24,0,2.37),(.18,.185,.105),skin,'head')
    for sign,side in ((1,'L'),(-1,'R')):
        def q(x,y,z):return (x,sign*y,z)
        m.ellipsoid('Pectoral '+side,q(.17,.215,1.96),(.17,.255,.17),skin)
        m.ellipsoid('Deltoid '+side,q(-.01,.47,2.01),(.22,.20,.23),skin)
        m.tube('Upper arm mass '+side,[q(0,.46,2.01),q(.01,.60,1.82),q(0,.69,1.61)],[.17,.17,.12],skin,sides=12)
        m.tube('Forearm mass '+side,[q(0,.69,1.65),q(.19,.62,1.46),q(.45,.55,1.30+(.20 if sign>0 else 0))],[.13,.135,.088],skin,sides=12)
        m.ellipsoid('Palm '+side,q(.50,.51,1.29+(.20 if sign>0 else 0)),(.105,.11,.13),skin)
        m.tube('Thigh mass '+side,[q(0,.23,1.20),q(.10,.34,1.0),q(.18,.45,.74)],[.215,.225,.15],skin,sides=12)
        m.tube('Digitigrade calf '+side,[q(.18,.45,.75),q(.02,.48,.49),q(-.08,.49,.25)],[.15,.13,.08],skin,sides=12)
        m.tube('Pastern '+side,[q(-.08,.49,.27),q(.01,.49,.16),q(.15,.49,.10)],[.085,.09,.085],skin,sides=12)
    for sign in (-1,1):
        for row in range(3):
            m.ellipsoid('Integrated abdominal mass',(.21,.112*sign,1.77-row*.15),(.063,.124-.012*row,.092),skin,seg=12,rings=7)
    anatomy=fuse(list(m.PARTS),skin,bones)
    # Small raised forms are ridges of anatomy, not the primary body construction.
    for sign,side in ((1,'L'),(-1,'R')):
        def q(x,y,z):return (x,sign*y,z)
        m.tube('Clavicle '+side,[q(.24,.03,2.12),q(.235,.23,2.11),q(.12,.43,2.08)],[.03,.037,.018],plate,'chest',8)
        # Split hooves have broad planted mass and an actual cleft.
        for toe in (-1,1):
            y=sign*.49+toe*.065
            verts=[(-.045,y-.057,.001),(.35,y-.064,.001),(.35,y+.064,.001),(-.045,y+.057,.001),
                   (-.08,y-.047,.19),(.24,y-.052,.14),(.24,y+.052,.14),(-.08,y+.047,.19)]
            m.mesh('Cloven hoof '+side+str(toe),verts,[(0,3,2,1),(0,1,5,4),(1,2,6,5),(2,3,7,6),(3,0,4,7),(4,5,6,7)],hoof,'foot_'+side)
        # Grip fingers curl around the shaft; no detached floating weapon.
        for j in range(4):
            yy=sign*.50+(j-1.5)*.044
            zz=1.39+.2*yy
            m.tube('Curled grip '+side+str(j),[(.46,yy,zz+.045),(.57,yy,zz+.025),(.583,yy,zz-.035),(.52,yy,zz-.052)],[.026,.027,.022,.013],skin,'hand_'+side,7)
        # Layered vambraces and shoulder plates.
        m.tube('Forearm cuff '+side,[q(.18,.625,1.48),q(.27,.59,1.405)],[.142,.12],plate,'forearm_'+side,12)
        m.tube('Bronze cuff seam '+side,[q(.26,.595,1.415),q(.28,.588,1.40)],[.125,.124],gold,'forearm_'+side,12)
        m.ellipsoid('Shoulder carapace '+side,q(-.04,.46,2.105),(.21,.23,.15),plate,'chest',16,8)
        for i in range(3):
            root=q(-.09,.36+i*.11,2.18-i*.03)
            m.tube('Shoulder spine '+side+str(i),[root,q(-.17,.4+i*.14,2.30-i*.025),q(-.25,.43+i*.17,2.35-i*.015)],[.053,.026,.001],horn,'chest',8)
        # Angular inset eyes with heavy brows and an elongated nose bridge.
        m.ellipsoid('Deep eye socket '+side,q(.283,.141,2.432),(.054,.045,.035),plate,'head',12,7)
        m.ellipsoid('Amber eye '+side,q(.344,.146,2.438),(.017,.031,.014),eye,'head',12,7)
        m.tube('Severe supraorbital brow '+side,[q(.29,.038,2.49),q(.321,.125,2.483),q(.235,.224,2.45)],[.028,.031,.012],plate,'head',8)
        m.tube('Facial cheek blade '+side,[q(.24,.20,2.42),q(.32,.15,2.31),q(.36,.11,2.24)],[.04,.035,.008],plate,'head',8)
        # Swept paired horns, many rings create a worn layered keratin profile.
        points=[q(.035,.165,2.52),q(-.03,.31,2.61),q(-.115,.395,2.77),q(-.11,.395,2.92),q(-.02,.34,3.025)]
        radii=[.082,.074,.047,.026,.001]
        m.tube('Curved great horn '+side,points,radii,horn,'head',12)
        for i in range(13):
            t=i/13*3.7;k=int(t);u=t-k;p=Vector(points[k]).lerp(Vector(points[k+1]),u)
            r=radii[k]*(1-u)+radii[k+1]*u
            tangent=(Vector(points[k+1])-Vector(points[k])).normalized()
            m.tube('Horn growth ridge '+side+str(i),[p-tangent*.006,p+tangent*.006],[r*1.065,r*1.035],horn,'head',10)
        m.tube('Mandible tusk '+side,[q(.31,.105,2.21),q(.40,.12,2.29),q(.395,.11,2.34)],[.030,.020,.001],horn,'head',9)
        m.tube('Pointed ear '+side,[q(.02,.185,2.39),q(-.025,.30,2.43),q(-.12,.39,2.52)],[.061,.036,.001],skin,'head',8)
    m.tube('Nose bridge',[(.29,0,2.51),(.36,0,2.41),(.435,0,2.345)],[.045,.047,.053],face,'head',8)
    for sign in (-1,1):m.ellipsoid('Nostril',(.445,.033*sign,2.34),(.012,.018,.009),mouth,'head',10,6)
    m.ellipsoid('Snarling mouth inset',(.383,0,2.266),(.036,.114,.040),mouth,'head',16,8)
    for i in range(8):
        y=(i-3.5)*.025
        m.tube('Visible tooth '+str(i),[(.412,y,2.291),(.420,y,2.269 if i%2 else 2.254)],[.009,.001],horn,'head',6)
    m.tube('Chin barb',[(.28,0,2.17),(.30,0,2.10),(.34,0,2.055)],[.061,.032,.001],plate,'head',9)
    # Back crest and tail remain inside the one-metre rear ruler.
    for i in range(6):
        z=1.55+i*.10
        m.tube('Vertebral spike '+str(i),[(-.26,0,z),(-.41,0,z+.05),(-.46,0,z+.09)],[.048,.024,.001],plate,'chest',8)
    m.tube('Articulated tail',[(-.19,0,1.18),(-.46,.1,.85),(-.68,.24,.52),(-.83,.43,.33),(-.89,.57,.42)],[.084,.07,.047,.028,.006],skin,'tail',10,
           weights=[{'body':.5,'tail':.5},{'tail':1},{'tail':.6,'tail_tip':.4},{'tail_tip':1},{'tail_tip':1}])
    m.tube('Tail blade',[(-.84,.50,.36),(-.91,.63,.48),(-.95,.70,.51)],[.035,.058,.001],plate,'tail_tip',7)
    # Belt follows pelvis, tattered panels leave the legs readable.
    for i in range(12):
        a=math.tau*i/12;y=.33*math.sin(a);x=.235*math.cos(a)
        z=1.20-.10*(i%3)
        verts=[(x,y,1.35),(x+.065*math.cos(a),y+.085*math.sin(a),1.11),
               (x+.04*math.cos(a),y+.09*math.sin(a),z-.40),
               (x*.72,y*.85,1.06)]
        m.mesh('Ragged kilt panel '+str(i),verts,[(0,1,2,3)],leather)
    for i in range(14):
        a=math.tau*i/14
        m.ellipsoid('Belt scale '+str(i),(.245*math.cos(a),.345*math.sin(a),1.34),(.045,.055,.055),plate,'body',8,5)
    m.ellipsoid('Belt bronze sigil',(.294,0,1.36),(.029,.095,.09),gold,'body',12,7)
    # Fan membranes: true scalloped topology and load-bearing curved spars.
    for sign,side in ((1,'L'),(-1,'R')):
        def q(x,y,z):return Vector((x,sign*y,z))
        wrist=q(-.36,.92,2.68);shoulder=q(-.20,.37,2.06)
        ends=[q(-.41,2.12,2.35),q(-.54,1.90,1.60),q(-.64,1.43,.85),q(-.59,.84,.62),q(-.34,.39,1.20)]
        m.tube('Wing leading arm '+side,[shoulder,q(-.28,.68,2.38),wrist,ends[0]],[.10,.079,.066,.006],skin,'wing_'+side,10,
               weights=[{'chest':.25,'wing_'+side:.75},{'wing_'+side:1},{'wing_'+side:1},{'wingtip_'+side:1}])
        for k,end in enumerate(ends):
            mid=wrist.lerp(end,.52);mid.x-=.06
            m.tube('Wing finger '+side+str(k),[wrist,mid,end],[.036,.027,.0025],skin,'wingtip_'+side,8)
        for panel in range(4):
            a,b=ends[panel],ends[panel+1];vertices=[];faces=[];weights=[]
            rows,cols=8,12
            for r in range(rows+1):
                u=r/rows
                for c in range(cols+1):
                    t=c/cols;edgepoint=a.lerp(b,t)
                    # Curved recess between each finger creates the scallop.
                    edgepoint=edgepoint.lerp(wrist,.29*math.sin(math.pi*t))
                    p=wrist.lerp(edgepoint,u);p.x-=.065*math.sin(math.pi*u)*math.sin(math.pi*t)
                    vertices.append(tuple(p));tip=max(0,min(1,(abs(p.y)-.7)/.55))
                    weights.append({'wing_'+side:1-tip,'wingtip_'+side:tip})
            for r in range(rows):
                for c in range(cols):
                    i=r*(cols+1)+c;faces.append((i,i+1,i+cols+2,i+cols+1))
            # Mirroring positions reverses winding; retain matching front normals.
            if sign<0:faces=[tuple(reversed(face)) for face in faces]
            m.mesh('Pigmented scalloped panel '+side+str(panel),vertices,faces,wing,weights=weights)
        m.tube('Wing thumb hook '+side,[wrist,q(-.30,.88,2.82),q(-.22,.95,2.89),q(-.19,1.04,2.82)],[.057,.04,.021,.001],horn,'wing_'+side,9)
    # Lateral double-ended axe, each end inside the full-spread transit width.
    m.tube('Axe ash haft',[(.52,-1.92,1.006),(.52,1.91,1.772)],[.036,.036],leather,'Weapon_Main',12)
    for i in range(20):
        y=-.67+i*.07;z=1.39+.2*y
        m.tube('Wrapped grip '+str(i),[(.52,y,z),(.52,y+.026,z+.0052)],[.043,.043],gold if i%5==0 else plate,'Weapon_Main',10)
    outline=[(1.24,1.60),(1.43,1.93),(1.96,2.13),(1.86,1.87),(2.12,1.80),(1.84,1.52),(1.93,1.24),(1.48,1.37)]
    blade('Broad crescent axe head',outline,.52,.073,iron)
    blade('Upper pale cutting bevel',[(1.43,1.93),(1.96,2.13),(1.87,2.045),(1.44,1.88)],.565,.012,edge)
    blade('Lower pale cutting bevel',[(1.93,1.24),(1.48,1.37),(1.51,1.425),(1.85,1.315)],.565,.012,edge)
    blade('Rear spear axe', [(-1.40,1.105),(-1.71,1.32),(-2.06,.93),(-1.62,.93)],.52,.061,iron)
    blade('Rear pale edge',[(-1.71,1.32),(-2.06,.93),(-1.99,.97),(-1.70,1.265)],.56,.01,edge)
    return list(m.PARTS),bones


def pose(kind,rig,clip,frame,end):
    from build import translate_world,rotate_world
    p=rig.pose.bones;t=frame/end;phase=math.sin(math.tau*t)
    if clip in ('idle','move'):
        rotate_world(p['chest'],(0,1,0),.012*phase)
        for sign,side in ((1,'L'),(-1,'R')):
            rotate_world(p['wing_'+side],(0,0,1),sign*.025*phase)
            rotate_world(p['wingtip_'+side],(0,0,1),sign*.025*phase)
            if clip=='move':
                step=phase*sign
                translate_world(p['thigh_'+side],(.055*step,0,.055*max(0,step)))
                rotate_world(p['thigh_'+side],(0,1,0),.085*step)
                rotate_world(p['shin_'+side],(0,1,0),-.10*step)
        rotate_world(p['tail'],(0,0,1),.04*phase)
    elif clip=='attack':
        if frame<=20:a=-.065*frame/20
        elif frame<=29:a=-.065
        elif frame<=30:a=.10
        else:a=.10*(42-frame)/12
        translate_world(p['chest'],(a,0,0));rotate_world(p['head'],(0,1,0),-.45*a)
        for side in ('L','R'):translate_world(p['forearm_'+side],(a*.45,0,-a*.20))
    elif clip=='hit':
        a=math.sin(math.pi*t);rotate_world(p['chest'],(0,1,0),-.065*a);rotate_world(p['head'],(0,1,0),-.08*a)
    elif clip=='death':
        u=min(1,frame/30);u=u*u*(3-2*u)
        # Kneel and slump; no root travel, flight, ragdoll or death reward event.
        translate_world(p['body'],(0,0,-.65*u));rotate_world(p['chest'],(0,1,0),.30*u)
        rotate_world(p['head'],(0,1,0),.65*u)
        for sign,side in ((1,'L'),(-1,'R')):
            rotate_world(p['thigh_'+side],(0,1,0),-1.10*u)
            rotate_world(p['shin_'+side],(0,1,0),1.85*u)
            rotate_world(p['wing_'+side],(1,0,0),-sign*.45*u)
            translate_world(p['wing_'+side],(.055*u,0,0))
            rotate_world(p['wingtip_'+side],(0,0,1),sign*.045*u)
        rotate_world(p['tail'],(0,1,0),-.2*u)
