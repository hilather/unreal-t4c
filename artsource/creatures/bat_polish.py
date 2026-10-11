"""W5-08d bat anatomy; all geometry is authored from functions, not image input.

W5-08f broad, shallow wing planes keep membranes visible from above. The
existing bones and anatomical anchors are retained; tiny fur cards are reduced
to fund a stronger silhouette and the undead rib detail without mesh growth.
"""
import math
import random
from mathutils import Vector
from models import ellipsoid, coat, ellipsoid_coat, loft, tube, mesh


def pinna(name, side, m):
    """Thin forward-facing ear cup, tapered and swept outwards above the ruff."""
    count = 16
    verts, faces = [], []
    # Four rings: the interior is behind the lip, not a bulging center disk.
    for radius in [0, .36, .73, 1]:
        for j in range(count):
            a = math.tau*j/count
            zz = .048*radius*math.sin(a)
            yy = .030*radius*math.cos(a)*(1-.66*max(0, math.sin(a)))
            yy += .020*radius*max(0, math.sin(a))
            verts.append((.055-.011*(1-radius*radius)-.011*max(0, math.sin(a)),
                          side*(.048+yy), 1.166+zz))
    for ring in range(3):
        for j in range(count):
            a = ring*count+j; b = ring*count+(j+1)%count
            faces.append((a,b,b+count,a+count))
    if side > 0:
        faces = [tuple(reversed(face)) for face in faces]
    mesh(name+' recessed bowl', verts, faces, m['ear'], 'head')
    rim = verts[-count:]+[verts[-count]]
    tube(name+' thin rolled helix', rim, .0015, m['skin'], 'head', 5)
    # Delicate branching cartilage grows out of the concha and follows the bowl.
    for ridge in range(3):
        points=[]
        for i in range(6):
            t=i/5
            z=1.131+.061*t
            y=.046+(.010+.005*ridge)*math.sin(t*math.pi*.84)+.013*t
            x=.049-.002*ridge-.004*t
            points.append((x,side*y,z))
        tube(name+' cartilage ridge '+str(ridge),points,
             [.0018*(1-i/7) for i in range(6)],m['skin'],'head',4)
    # The tragus partly occludes the dark concha instead of painting a dot.
    tube(name+' pointed tragus',[(.056,side*.035,1.129),(.064,side*.040,1.142),(.061,side*.044,1.150)],
         [.0038,.0027,.0004],m['skin'],'head',5)


def bat(m,bones,undead=False):
    # The muzzle and brow share the procedural fur grain, with a restrained
    # warmer dark mask. This is material shading, never a painted eye ring.
    facial_fur=m['fur'].copy();facial_fur.name='Bat warm facial fur'
    nodes=facial_fur.node_tree.nodes;links=facial_fur.node_tree.links
    base=nodes.get('Principled BSDF').inputs['Base Color']
    upstream=base.links[0].from_socket
    tint=nodes.new('ShaderNodeMixRGB');tint.blend_type='MULTIPLY'
    tint.inputs[0].default_value=1
    tint.inputs[2].default_value=(.60,.50,.40,1)
    links.new(upstream,tint.inputs[1]);links.new(tint.outputs[0],base)
    bones['body']=((0,0,1),(0,.1,1),'root')
    bones['head']=((.02,0,1.055),(.1,0,1.075),'body')
    bones['jaw']=((.091,0,1.045),(.14,0,1.043),'head')
    # A tapered lower body and strong shoulder/chest ruff replace the small ball.
    sections=[(-.105,.006,.012,1.003),(-.085,.033,.045,1.009),
              (-.048,.052,.063,1.025),(-.010,.063,.073,1.035),
              (.028,.056,.068,1.047),(.055,.033,.040,1.059),
              (.069,.012,.014,1.066)]
    loft('Tapered furry chest and lifted shoulders',sections,m['fur'],sides=24)
    def chest_surface():
        x=random.uniform(-.092,.059)
        i=next(i for i in range(len(sections)-1) if sections[i][0]<=x<=sections[i+1][0])
        a,b=sections[i:i+2];t=(x-a[0])/(b[0]-a[0]);s=[a[k]*(1-t)+b[k]*t for k in range(4)]
        theta=random.uniform(0,math.tau)
        p=(x,s[1]*math.cos(theta),s[3]+s[2]*math.sin(theta))
        slopes=[(b[k]-a[k])/(b[0]-a[0]) for k in range(4)]
        along=Vector((1,slopes[1]*math.cos(theta),slopes[3]+slopes[2]*math.sin(theta)))
        around=Vector((0,-s[1]*math.sin(theta),s[2]*math.cos(theta)))
        return p,around.cross(along),-along
    coat('Layered shoulder and chest ruff',chest_surface,350,m['fur_tip'],length=.014)
    # A shorter domed cranium flows into cheek shelves and a paired pug muzzle.
    # Sampling this same loft roots the coat on the actual surface; an ellipsoid
    # proxy underneath the skull buries most of the hair and leaves a bald ball.
    head_sections=[(.014,.023,.026,1.093),(.037,.036,.036,1.097),
        (.062,.043,.029,1.100),(.089,.041,.027,1.098),
        (.109,.034,.024,1.091),(.127,.024,.013,1.080),(.144,.007,.006,1.080)]
    loft('Sculpted cranium and cheek taper',head_sections,m['fur'],'head',24)
    def face_surface():
        x=random.uniform(.022,.128)
        i=next(i for i in range(len(head_sections)-1) if head_sections[i][0]<=x<=head_sections[i+1][0])
        a,b=head_sections[i:i+2];t=(x-a[0])/(b[0]-a[0]);q=[a[k]*(1-t)+b[k]*t for k in range(4)]
        theta=random.uniform(-.4,math.pi+.4)
        p=(x,q[1]*math.cos(theta),q[3]+q[2]*math.sin(theta))
        slopes=[(b[k]-a[k])/(b[0]-a[0]) for k in range(4)]
        along=Vector((1,slopes[1]*math.cos(theta),slopes[3]+slopes[2]*math.sin(theta)))
        around=Vector((0,-q[1]*math.sin(theta),q[2]*math.cos(theta)))
        # Fur sweeps into the neck. Slight side sweep opens the cheek silhouette.
        normal=around.cross(along).normalized()
        direction=Vector((-1,.22*math.cos(theta),-.18))
        tangent=direction-normal*normal.dot(direction)
        return p,normal,tangent
    coat('Rooted forehead cheek and neck fringe',face_surface,230,m['fur_tip'],'head',.009)
    ellipsoid('Recessed mouth',(.131,0,1.068),(.017,.018,.0055),m['mouth'],'jaw',12,6)
    ellipsoid('Small soft chin',(.125,0,1.062),(.019,.016,.0055),m['fur'],'jaw',12,6)
    # Folded nostril wings and a narrow lance-shaped leaf, deliberately tiny.
    mesh('Small triangular nose',[(.150,0,1.085),(.142,-.009,1.088),
        (.142,.009,1.088),(.145,0,1.077),(.140,0,1.086)],
        [(0,1,2),(0,3,1),(0,2,3),(4,2,1),(4,1,3),(4,3,2)],m['eye'],'head')
    mesh('Small upright nose leaf',[(.145,-.005,1.085),(.147,0,1.100),
        (.145,.005,1.085),(.151,0,1.091),(.143,0,1.091)],
        [(0,1,3),(1,2,3),(2,0,3),(1,0,4),(2,1,4),(0,2,4)],m['skin'],'head')
    for s in [-1,1]:
        # Low cheek/muzzle cushions flank the small nose, with short rooted fur.
        cheek_center=(.129,s*.010,1.079)
        cheek_axes=(.015,.011,.007)
        ellipsoid('Lateral muzzle cushion',cheek_center,cheek_axes,facial_fur,'head',12,6)
        coat('Short velvet muzzle fur',ellipsoid_coat(cheek_center,cheek_axes),
             60,facial_fur,'head',.004)
        ellipsoid('Inset alert bat eye',(.114,s*.026,1.104),
                  (.008,.006,.007),m['eye'],'head',12,7)
        # A triangular brow shelf is buried into the loft at its upper roots.
        # Only its thin outer lip overhangs the eye: no detached brow sausages.
        brow=[(.089,s*.022,1.111),(.099,s*.029,1.115),
              (.110,s*.029,1.112),(.119,s*.023,1.105),
              (.112,s*.031,1.108),(.103,s*.032,1.111),
              (.093,s*.027,1.110)]
        faces=[(0,1,6),(1,5,6),(1,2,5),(2,4,5),(2,3,4)]
        if s<0:faces=[tuple(reversed(f)) for f in faces]
        mesh('Integrated brow and eye fold',brow,faces,facial_fur,'head')
        # Tiny nasolabial crease sinks partly into each cushion, readable by AO.
        tube('Muzzle fold',[(.136,s*.009,1.085),(.141,s*.012,1.081),(.138,s*.018,1.075)],
             [.0011,.0009,.0002],facial_fur,'head',4)
        tube('Small exposed canine',[(.141,s*.015,1.075),(.143,s*.015,1.065),(.142,s*.014,1.062)],
             [.0028,.0013,.0002],m['tooth'],'head',6)
        tube('Small lower incisor',[(.140,s*.006,1.065),(.142,s*.006,1.071)],
             [.0014,.0003],m['tooth'],'jaw',5)
        pinna('Ridged pointed pinna',s,m)
        wing='wing'+str(s);tipbone='wingtip'+str(s)
        bones[wing]=((-.008,s*.035,1.042),(.105,s*.135,1.075),'body')
        bones[tipbone]=((.105,s*.135,1.075),(.095,s*.395,1.125),wing)
        wrist=Vector((.105,.135,1.075))
        tips=[Vector((.095,.395,1.125)),Vector((-.155,.345,1.045)),Vector((-.201,.215,1.000)),Vector((-.16,.085,.970)),Vector((-.047,.036,.982))]
        v=[];faces=[];weights=[];U=8;R=6
        for panel in range(len(tips)-1):
            a,b=tips[panel:panel+2];base=len(v)
            for i in range(R+1):
                t=i/R
                for j in range(U+1):
                    u=j/U
                    # Scallop is a taut catenary-like curve pulled towards the wrist.
                    edge=a.lerp(b,u);edge=edge.lerp(wrist,.17*math.sin(math.pi*u))
                    q=wrist.lerp(edge,t);q.z+=.014*math.sin(math.pi*t)*math.sin(math.pi*u)
                    v.append((q.x,s*q.y,q.z));w=max(0,min(1,(q.y-.13)/.17))
                    weights.append({wing:1-w,tipbone:w})
            for i in range(R):
                for j in range(U):
                    # Coarse missing wedges remain real silhouette tears at 512px.
                    if undead and ((panel==0 and i>=R-2 and j in (3,4)) or
                                   (panel==1 and i>=R-2 and j==5) or
                                   (panel==2 and i==R-1 and j in (2,3))):continue
                    k=base+i*(U+1)+j;faces.append((k,k+1,k+U+2,k+U+1))
        if s>0:faces=[tuple(reversed(face)) for face in faces]
        mesh('Four scalloped membrane panels',v,faces,m['membrane'],weights=weights)
        tube('Arm and leading spar',[(-.015,s*.035,1.046),(.014,s*.078,1.068),(.105,s*.135,1.075),(.095,s*.395,1.125)],[.006,.005,.004,.001],m['skin'],wing,7,
             [{wing:1},{wing:1},{wing:1},{tipbone:1}])
        for tip in tips[1:]:
            pts=[];ws=[]
            for i in range(7):
                t=i/6;q=wrist.lerp(tip,t);q.z+=.004*math.sin(math.pi*t);pts.append((q.x,s*q.y,q.z+.001))
                w=max(0,min(1,(q.y-.13)/.17));ws.append({wing:1-w,tipbone:w})
            tube('Spreading finger',pts,[.0035*(1-i/7)+.0007 for i in range(7)],m['skin'],wing,5,ws)
        tube('Exposed muscular thumb',[(.103,s*.133,1.074),(.131,s*.141,1.098),(.144,s*.140,1.105)],[.0055,.0045,.003],m['skin'],wing,7)
        tube('Ivory hooked thumb claw',[(.141,s*.140,1.104),(.151,s*.137,1.110),(.155,s*.131,1.103),(.153,s*.129,1.093)],[.0038,.003,.0018,.0002],m['claw'],wing,7)
        tube('Hind ankle',[(-.041,s*.025,.974),(-.09,s*.035,.946),(-.099,s*.039,.927)],[.005,.004,.002],m['skin'],'body',6)
        for j in range(3):tube('Hind claw',[(-.099,s*(.032+j*.007),.928),(-.084,s*(.032+j*.007),.92),(-.078,s*(.032+j*.007),.926)],[.002,.0014,.0002],m['claw'],'body',5)

