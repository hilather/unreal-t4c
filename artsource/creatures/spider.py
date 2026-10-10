"""Original W5-08e giant spider, metres / +X forward / grounded root.

Eight three-segment legs, separate abdomen, armored face and rooted bristles.
Concept sheets are visual targets only; no external pixels or geometry are read.
All scale and animation choices are prototype presentation values.
"""
import math
import random
import bpy
from mathutils import Vector, Quaternion
import models as M


def material(name, dark, light, roughness=.72):
    m=bpy.data.materials.new(name);m.use_nodes=True
    n=m.node_tree.nodes;l=m.node_tree.links;p=n.get('Principled BSDF')
    tex=n.new('ShaderNodeTexCoord');noise=n.new('ShaderNodeTexNoise')
    noise.inputs['Scale'].default_value=25;noise.inputs['Detail'].default_value=3.5
    l.new(tex.outputs['Object'],noise.inputs['Vector'])
    ramp=M.ramp(n,[(.20,dark),(.78,light)])
    l.new(noise.outputs['Fac'],ramp.inputs[0]);l.new(ramp.outputs[0],p.inputs['Base Color'])
    fine=n.new('ShaderNodeTexNoise');fine.inputs['Scale'].default_value=95
    l.new(tex.outputs['Object'],fine.inputs['Vector'])
    bump=n.new('ShaderNodeBump');bump.inputs['Distance'].default_value=.0018;bump.inputs['Strength'].default_value=.65
    l.new(fine.outputs['Fac'],bump.inputs['Height']);l.new(bump.outputs['Normal'],p.inputs['Normal'])
    if any(word in name for word in ('chitin','carapace','ridges')):
        # Fine interrupted creases break up the broad armor values. They bake
        # into the normal/base atlas; no reference pixels are used.
        cells=n.new('ShaderNodeTexVoronoi');cells.feature='DISTANCE_TO_EDGE';cells.inputs['Scale'].default_value=58
        l.new(tex.outputs['Object'],cells.inputs['Vector'])
        wear=M.ramp(n,[(.006,(.48,.45,.40)),(.040,(1,1,1))])
        l.new(cells.outputs['Distance'],wear.inputs[0])
        patina=n.new('ShaderNodeMixRGB');patina.blend_type='MULTIPLY';patina.inputs[0].default_value=.52
        l.new(ramp.outputs[0],patina.inputs[1]);l.new(wear.outputs[0],patina.inputs[2]);l.new(patina.outputs[0],p.inputs['Base Color'])
        crease=n.new('ShaderNodeBump');crease.inputs['Distance'].default_value=.0025;crease.inputs['Strength'].default_value=.32
        l.new(cells.outputs['Distance'],crease.inputs['Height']);l.new(bump.outputs['Normal'],crease.inputs['Normal']);l.new(crease.outputs['Normal'],p.inputs['Normal'])
    p.inputs['Roughness'].default_value=roughness
    return m


def bristles(name,points,radii,count,mat,bone):
    """Seeded tapered hair blades rooted on each chitin tube, not floating cards."""
    verts=[];faces=[]
    for _ in range(count):
        i=random.randrange(len(points)-1);t=random.random()
        a,b=Vector(points[i]),Vector(points[i+1]);axis=(b-a).normalized()
        cross=axis.cross(Vector((0,0,1)))
        if cross.length<.1:cross=axis.cross(Vector((0,1,0)))
        cross.normalize();up=axis.cross(cross).normalized();theta=random.random()*math.tau
        normal=cross*math.cos(theta)+up*math.sin(theta)
        radius=radii[i]*(1-t)+radii[i+1]*t
        p=a.lerp(b,t)+normal*radius*.94
        length=random.uniform(.022,.042)
        tip=p+normal*length-axis*length*.45
        offset=cross*.0018;idx=len(verts)
        verts.extend([p-offset,p+offset,tip, p+up*.0012,p-up*.0012])
        faces.extend([(idx,idx+1,idx+2),(idx+3,idx+4,idx+2)])
    M.mesh(name,verts,faces,mat,bone)



def fluted_tube(name,points,radii,mat,bone):
    # Thick longitudinal chitin flutes catch warm grazing light at gameplay size.
    obj=M.tube(name,points,radii,mat,bone,12)
    for i,vert in enumerate(obj.data.vertices):
        ring,side=divmod(i,12)
        center=Vector(points[ring])
        vert.co=center+(vert.co-center)*(1.20 if side%2==0 else .84)
    return obj

def create(kind='giant_spider'):
    assert kind=='giant_spider'
    M.PARTS=[];random.seed(50851)
    shell=material('Spider charcoal brown textured chitin',(.0055,.005,.004),(.030,.027,.021))
    front=material('Spider lighter worn front carapace',(.010,.009,.007),(.055,.048,.035))
    ridge=material('Spider ivory gray worn ridges',(.039,.033,.024),(.14,.119,.079))
    joint=material('Spider dark flexible joints',(.005,.006,.005),(.024,.025,.019),.85)
    hair=material('Spider sparse warm guard bristles',(.088,.061,.034),(.30,.23,.14),.83)
    rust=material('Spider restrained rust fangs',(.019,.004,.002),(.072,.021,.008),.42)
    eye=material('Spider eight polished red eyes',(.20,.001,.0004),(.72,.018,.003),.20)
    bones={'root':((0,0,0),(0,0,.06),None),
           'body':((.04,0,.31),(.04,.1,.31),'root'),
           'abdomen':((-.11,0,.37),(-.42,0,.40),'body'),
           'head':((.23,0,.32),(.46,0,.29),'body'),
           'fang-1':((.45,-.078,.24),(.52,-.09,.13),'head'),
           'fang1':((.45,.078,.24),(.52,.09,.13),'head')}
    M.tube('Narrow visible abdomen pedicel',[(-.21,0,.34),(-.08,0,.34),(.01,0,.34)],[.075,.065,.115],joint,'body',14)
    M.loft('Distinct raised pear-shaped abdomen',[(-.59,.02,.04,.38),(-.55,.13,.135,.39),(-.46,.214,.198,.405),(-.31,.231,.219,.41),(-.19,.19,.183,.40),(-.115,.118,.115,.37),(-.08,.045,.055,.35)],shell,'abdomen',28)
    M.loft('Broad low ridged cephalothorax',[(-.15,.07,.075,.33),(-.07,.156,.125,.34),(.07,.18,.135,.34),(.20,.155,.125,.335),(.31,.13,.104,.325),(.37,.063,.065,.31)],front,'body',24)
    # Articulated abdominal plates emphasize the separated rear mass in oblique view.
    sections=[(-.55,.13,.135,.39),(-.46,.214,.198,.405),(-.31,.231,.219,.41),(-.19,.19,.183,.40),(-.115,.118,.115,.37)]
    for row in range(7):
        x=-.51+row*.057
        i=next(i for i in range(len(sections)-1) if sections[i][0]<=x<=sections[i+1][0])
        a,b=sections[i:i+2];t=(x-a[0])/(b[0]-a[0]);rx=a[1]*(1-t)+b[1]*t;rz=a[2]*(1-t)+b[2]*t;cz=a[3]*(1-t)+b[3]*t
        points=[]
        for j in range(11):
            theta=-.12+math.pi*j/10
            points.append((x,rx*math.cos(theta),cz+rz*math.sin(theta)+.003))
        plate=M.tube('Low broad abdominal plate edge %d'%row,points,[.005+.002*math.sin(math.pi*j/10) for j in range(11)],shell,'abdomen',5)
        for vertex in plate.data.vertices:
            center=Vector(points[vertex.index//5]);delta=vertex.co-center
            vertex.co=center+Vector((delta.x*2.6,delta.y*.32,delta.z*.32))
    # Front brow is a flattened wedge, with staggered eye sockets, never a sphere face.
    M.loft('Faceted forward brow shield',[(.25,.12,.080,.35),(.32,.123,.075,.34),(.39,.104,.059,.31),(.445,.065,.038,.29)],front,'head',16)
    for s in [-1,1]:
        for j,(x,y,z,r) in enumerate([(.39,.040,.357,.020),(.417,.072,.326,.017),(.36,.091,.366,.012),(.437,.026,.321,.011)]):
            M.ellipsoid('Raised dark eye socket', (x-.003,s*y,z), (r*1.3,r*1.15,r*1.05),joint,'head',8,5)
            M.ellipsoid('Polished crimson eye %d %d'%(s,j),(x+.008,s*y,z+.006),(r*.73,r*.78,r*.81),eye,'head',10,6)
        fang='fang'+str(s)
        M.tube('Massive cheek chelicera',[(.377,s*.081,.277),(.432,s*.101,.237),(.477,s*.091,.202)],[.035,.032,.022],front,fang,10)
        M.tube('Curved inner biting fang',[(.477,s*.091,.207),(.51,s*.09,.169),(.515,s*.061,.130),(.501,s*.038,.146)],[.023,.018,.008,.0006],rust,fang,9)
        M.tube('Outside articulated palp',[(.325,s*.113,.266),(.42,s*.166,.239),(.478,s*.155,.165),(.512,s*.135,.192)],[.02,.022,.012,.001],shell,'head',7)
        for j in range(3):
            M.tube('Raised radial thorax ridge',[(.015,s*(.04+j*.03),.465),(.15,s*(.045+j*.034),.451),(.278,s*(.045+j*.029),.415)],[.008,.007,.002],ridge,'body',5)
    for s in [-1,1]:
        for i in range(4):
            x=.265+i*.039;y=s*(.115-i*.008);z=.387-i*.016
            M.tube('Brow shield worn lateral spine',[(x,y,z),(x+.012,y+s*.021,z+.019),(x+.028,y+s*.031,z+.020)],[.010,.007,.0005],ridge,'head',5)
    starts=[(.255,.12,.31),(.15,.157,.335),(.02,.165,.345),(-.10,.13,.34)]
    knees=[(.53,.395,.51),(.295,.53,.585),(-.235,.535,.58),(-.485,.40,.535)]
    ankles=[(.72,.60,.135),(.42,.745,.145),(-.30,.747,.14),(-.555,.575,.13)]
    feet=[(.79,.65,.003),(.455,.813,.003),(-.335,.816,.003),(-.607,.623,.003)]
    for s in [-1,1]:
        for i in range(4):
            a=Vector((starts[i][0],s*starts[i][1],starts[i][2]));k=Vector((knees[i][0],s*knees[i][1],knees[i][2]));an=Vector((ankles[i][0],s*ankles[i][1],ankles[i][2]));ft=Vector((feet[i][0],s*feet[i][1],feet[i][2]))
            upper='leg%d_%s_upper'%(i,s);lower='leg%d_%s_lower'%(i,s);toe='leg%d_%s_tip'%(i,s)
            bones[upper]=(tuple(a),tuple(k),'body');bones[lower]=(tuple(k),tuple(an),upper);bones[toe]=(tuple(an),tuple(ft),lower)
            M.ellipsoid('Deep-set coxa %d %s'%(i,s),a,(.06,.052,.048),joint,upper,10,5)
            mid=a.lerp(k,.47);mid.z+=.022
            points=[a,a.lerp(k,.15),mid,a.lerp(k,.84),k];r=[.049,.052,.045,.029,.023]
            fluted_tube('Massy tapered femur %d %s'%(i,s),points,r,shell,upper)
            M.ellipsoid('Armored bent knee %d %s'%(i,s),k,(.036,.034,.033),joint,lower,10,5)
            kcap=k+Vector((0,0,.015));M.ellipsoid('Knee dorsal shell %d %s'%(i,s),kcap,(.031,.030,.025),front,lower,8,5)
            lowpoints=[k,k.lerp(an,.12),k.lerp(an,.5),k.lerp(an,.85),an];lowr=[.027,.032,.025,.016,.012]
            fluted_tube('Long sculpted tibia %d %s'%(i,s),lowpoints,lowr,front,lower)
            # A narrow crest reads as worn chitin under warm light, with no white piping.
            crest=[p+Vector((.012,0,.015)) for p in lowpoints[1:4]]
            M.tube('Tibial chitin crest',crest,[.005,.005,.0015],ridge,lower,5)
            M.tube('Tapered grounded tarsus %d %s'%(i,s),[an,an.lerp(ft,.45),ft],[.014,.010,.0008],rust,toe,7)
            bristles('Rooted femur guard hairs %d %s'%(i,s),points,r,52,hair,upper)
            bristles('Rooted tibia guard hairs %d %s'%(i,s),lowpoints,lowr,38,hair,lower)
    M.coat('Sparse backward-swept abdominal bristles',M.ellipsoid_coat((-.31,0,.40),(.27,.223,.212)),180,hair,'abdomen',.029)
    # Attachment points are nondeforming bones and do not imply combat notifies.
    bones['VFX_Bite']=((.51,0,.19),(.55,0,.19),'head')
    bones['VFX_Hit']=((.05,0,.47),(.05,0,.52),'body')
    bones['UI_Anchor']=((-.20,0,.73),(-.20,0,.78),'body')
    return M.PARTS,bones


def rotate(bone,axis,angle):
    local=bone.bone.matrix_local.to_3x3().inverted()@Vector(axis)
    bone.rotation_euler=Quaternion(local.normalized(),angle).to_euler('XYZ')


def translate(bone,xyz):
    bone.location=bone.bone.matrix_local.to_3x3().inverted()@Vector(xyz)


def pose(kind,rig,clip,frame,end):
    b=rig.pose.bones;phase=math.sin(math.tau*frame/end)
    if clip in ('idle','move'):
        bob=.004*phase if clip=='idle' else .006*math.sin(math.tau*frame/end*2)
        translate(b['body'],(0,0,bob))
        rotate(b['abdomen'],(0,1,0),.012*phase)
        for s in [-1,1]:
            for i in range(4):
                step=math.sin(math.tau*frame/end+(i%2+(s>0))*math.pi) if clip=='move' else 0
                lift=max(0,step)*.032
                translate(b['leg%d_%s_upper'%(i,s)],(.018*step,0,lift-bob))
    elif clip=='attack':
        if frame<20:t=frame/20;recoil=-.038*t;rise=.024*t
        elif frame<30:recoil=-.038;rise=.024
        elif frame==30:recoil=.038;rise=.010
        else:t=(frame-30)/12;recoil=.038*(1-t);rise=.01*(1-t)
        translate(b['head'],(recoil,0,rise))
        rotate(b['head'],(0,1,0),-.10*(rise/.024))
        for s in [-1,1]:rotate(b['fang'+str(s)],(0,1,0),-.25 if frame==30 else .08*(rise/.024))
    elif clip=='hit':
        t=math.sin(math.pi*frame/end);translate(b['body'],(-.021*t,0,-.023*t));rotate(b['abdomen'],(0,1,0),.10*t)
        for s in [-1,1]:
            for i in range(4):translate(b['leg%d_%s_upper'%(i,s)],(.018*t,0,.022*t))
    elif clip=='death':
        t=min(1,frame/30);t=t*t*(3-2*t)
        translate(b['body'],(0,0,-.23*t));b['body'].scale=(1,1-.50*t,1-.15*t);rotate(b['head'],(0,1,0),.15*t)
        for s in [-1,1]:
            for i in range(4):
                rotate(b['leg%d_%s_upper'%(i,s)],(1,0,0),-.20*s*t)
                rotate(b['leg%d_%s_lower'%(i,s)],(1,0,0),.77*s*t)
                rotate(b['leg%d_%s_tip'%(i,s)],(1,0,0),.70*s*t)
