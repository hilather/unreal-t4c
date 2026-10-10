"""Original, seeded anatomical studies; no photographs or concept pixels as input.

Metres, X forward, Z up. The presentation envelopes are authored prototype targets.
Every hair blade is rooted in the surface; the baked coat carries the dense detail.
"""
import math
import random
import bpy
from mathutils import Vector

PARTS = []
TAU = math.tau


def ramp(nodes, values):
    node = nodes.new('ShaderNodeValToRGB')
    for i, (position, color) in enumerate(values):
        e = node.color_ramp.elements[i] if i < 2 else node.color_ramp.elements.new(position)
        e.position = position
        e.color = (*color, 1)
    return node


def shader(name, kind):
    m = bpy.data.materials.new(name)
    m.use_nodes = True
    n, l = m.node_tree.nodes, m.node_tree.links
    p = n.get('Principled BSDF')
    tex = n.new('ShaderNodeTexCoord')
    noise = n.new('ShaderNodeTexNoise')
    noise.inputs['Detail'].default_value = 4
    noise.inputs['Roughness'].default_value = .72
    l.new(tex.outputs['Object'], noise.inputs['Vector'])
    bump = n.new('ShaderNodeBump')
    bump.inputs['Strength'].default_value = .35
    bump.inputs['Distance'].default_value = .0012
    l.new(noise.outputs['Fac'], bump.inputs['Height'])
    l.new(bump.outputs['Normal'], p.inputs['Normal'])
    if kind in ('fur', 'fur_tip'):
        # Anisotropic, broken strokes aligned along the length of the animal.
        stretch = n.new('ShaderNodeVectorMath'); stretch.operation = 'MULTIPLY'
        stretch.inputs[1].default_value = (12, 800, 800)
        l.new(tex.outputs['Object'], stretch.inputs[0])
        l.new(stretch.outputs[0], noise.inputs['Vector'])
        noise.inputs['Scale'].default_value = 1
        noise.inputs['Detail'].default_value = 2
        noise.inputs['Roughness'].default_value = .55
        color = ramp(n, [(.22,(.012,.006,.003)),(.45,(.043,.022,.010)),(.62,(.12,.067,.028)),(.79,(.24,.15,.067))])
        l.new(noise.outputs['Fac'], color.inputs[0])
        sep = n.new('ShaderNodeSeparateXYZ'); l.new(tex.outputs['Object'],sep.inputs[0])
        height = n.new('ShaderNodeMapRange');height.inputs['From Min'].default_value=.055; height.inputs['From Max'].default_value=.24
        height.inputs['To Min'].default_value=.48; height.inputs['To Max'].default_value=1
        l.new(sep.outputs['Z'],height.inputs['Value'])
        mult = n.new('ShaderNodeMixRGB');mult.blend_type='MULTIPLY';mult.inputs[0].default_value=.72
        l.new(color.outputs[0],mult.inputs[1]);l.new(height.outputs[0],mult.inputs[2]);l.new(mult.outputs[0],p.inputs['Base Color'])
        p.inputs['Roughness'].default_value = .83
        bump.inputs['Strength'].default_value=.48
        bump.inputs['Distance'].default_value=.0018
    elif kind in ('skin','tail','ear'):
        noise.inputs['Scale'].default_value=180
        color=ramp(n,[(.22,(.105,.044,.029)),(.72,(.34,.17,.105))])
        l.new(noise.outputs['Fac'],color.inputs[0]);l.new(color.outputs[0],p.inputs['Base Color'])
        p.inputs['Roughness'].default_value=.57
        if kind=='tail':
            wave=n.new('ShaderNodeTexWave');wave.wave_type='BANDS';wave.bands_direction='X'
            wave.inputs['Scale'].default_value=210;wave.inputs['Distortion'].default_value=3
            l.new(tex.outputs['Object'],wave.inputs['Vector']);l.new(wave.outputs['Fac'],bump.inputs['Height'])
            bump.inputs['Strength'].default_value=.6
        if kind=='ear':p.inputs['Base Color'].default_value=(.3,.12,.075,1)
    elif kind=='membrane':
        noise.inputs['Scale'].default_value=30
        color=ramp(n,[(.2,(.075,.032,.012)),(.48,(.23,.12,.048)),(.78,(.43,.26,.12))])
        l.new(noise.outputs['Fac'],color.inputs[0])
        veins=n.new('ShaderNodeTexVoronoi');veins.feature='DISTANCE_TO_EDGE';veins.inputs['Scale'].default_value=72
        l.new(tex.outputs['Object'],veins.inputs['Vector'])
        veincolor=ramp(n,[(.006,(.14,.08,.045)),(.035,(1,1,1))]);l.new(veins.outputs['Distance'],veincolor.inputs[0])
        mix=n.new('ShaderNodeMixRGB');mix.blend_type='MULTIPLY';mix.inputs[0].default_value=.55
        l.new(color.outputs[0],mix.inputs[1]);l.new(veincolor.outputs[0],mix.inputs[2]);l.new(mix.outputs[0],p.inputs['Base Color'])
        l.new(veins.outputs['Distance'],bump.inputs['Height']);bump.inputs['Distance'].default_value=.0005
        p.inputs['Roughness'].default_value=.70
    elif kind=='slime':
        noise.inputs['Scale'].default_value=13
        # Thickness-color approximation: dark interior, lime thin tissue, bright bubble rims.
        color=ramp(n,[(.18,(.001,.009,.0004)),(.4,(.005,.034,.001)),(.65,(.024,.10,.003)),(.83,(.073,.18,.009))])
        l.new(noise.outputs['Fac'],color.inputs[0])
        bubbles=n.new('ShaderNodeTexVoronoi');bubbles.inputs['Scale'].default_value=19
        l.new(tex.outputs['Object'],bubbles.inputs['Vector'])
        bubblecolor=ramp(n,[(.07,(.013,.067,.001)),(.20,(.002,.011,.0004)),(.245,(.074,.17,.006)),(.29,(.011,.06,.001)),(.43,(.008,.042,.001))])
        l.new(bubbles.outputs['Distance'],bubblecolor.inputs[0])
        mix=n.new('ShaderNodeMixRGB');mix.inputs[0].default_value=.28
        l.new(color.outputs[0],mix.inputs[1]);l.new(bubblecolor.outputs[0],mix.inputs[2]);l.new(mix.outputs[0],p.inputs['Base Color'])
        rough=n.new('ShaderNodeMapRange');rough.inputs['To Min'].default_value=.16;rough.inputs['To Max'].default_value=.27
        l.new(noise.outputs['Fac'],rough.inputs[0]);l.new(rough.outputs[0],p.inputs['Roughness'])
        bump.inputs['Strength'].default_value=.3;bump.inputs['Distance'].default_value=.007
    else:
        colors={'eye':((.003,.002,.001),.12),'claw':((.5,.37,.23),.43),'mouth':((.033,.005,.003),.65),'tooth':((.63,.51,.31),.35)}
        c,r=colors[kind];p.inputs['Base Color'].default_value=(*c,1);p.inputs['Roughness'].default_value=r
        bump.inputs['Strength'].default_value=0
    return m


def mesh(name, verts, faces, mat, bone='body', weights=None):
    data=bpy.data.meshes.new(name);data.from_pydata(verts,[],faces);data.update()
    obj=bpy.data.objects.new(name,data);bpy.context.collection.objects.link(obj);data.materials.append(mat)
    for f in data.polygons:f.use_smooth=True
    if weights:
        groups={key:obj.vertex_groups.new(name=key) for key in sorted(set(k for row in weights for k in row))}
        for i,row in enumerate(weights):
            for key,w in row.items():
                if w>0:groups[key].add([i],w,'REPLACE')
    else:
        g=obj.vertex_groups.new(name=bone);g.add(list(range(len(verts))),1,'REPLACE')
    PARTS.append(obj)
    return obj


def ellipsoid(name,center,axes,mat,bone='body',seg=16,rings=9):
    v=[(center[0],center[1],center[2]+axes[2])];f=[]
    for i in range(1,rings):
        t=math.pi*i/rings
        for j in range(seg):
            a=TAU*j/seg;v.append(tuple(center[k]+axes[k]*[math.sin(t)*math.cos(a),math.sin(t)*math.sin(a),math.cos(t)][k] for k in range(3)))
    bottom=len(v);v.append((center[0],center[1],center[2]-axes[2]))
    for j in range(seg):f.append((0,1+j,1+(j+1)%seg));f.append((bottom,1+(rings-2)*seg+(j+1)%seg,1+(rings-2)*seg+j))
    for i in range(rings-2):
        for j in range(seg):a=1+i*seg+j;b=1+i*seg+(j+1)%seg;f.append((a,a+seg,b+seg,b))
    return mesh(name,v,f,mat,bone)


def tube(name,points,radii,mat,bone='body',sides=6,weights=None):
    v=[];f=[];vw=[]
    for i,p in enumerate(points):
        tangent=(Vector(points[min(i+1,len(points)-1)])-Vector(points[max(0,i-1)])).normalized()
        axis=Vector((0,0,1)) if abs(tangent.z)<.95 else Vector((0,1,0))
        u=tangent.cross(axis).normalized();w=tangent.cross(u).normalized()
        r=radii[i] if isinstance(radii,(list,tuple)) else radii
        for j in range(sides):v.append(Vector(p)+r*(math.cos(TAU*j/sides)*u+math.sin(TAU*j/sides)*w));vw.append(weights[i] if weights else {})
    for i in range(len(points)-1):
        for j in range(sides):a=i*sides+j;b=i*sides+(j+1)%sides;f.append((a,b,b+sides,a+sides))
    f.extend([tuple(range(sides-1,-1,-1)),tuple((len(points)-1)*sides+j for j in range(sides))])
    return mesh(name,v,f,mat,bone,vw if weights else None)


def loft(name,sections,mat,bone='body',sides=24):
    # x, lateral radius, vertical radius, center height
    v=[];f=[]
    for x,ry,rz,z in sections:
        for j in range(sides):
            a=TAU*j/sides;v.append((x,ry*math.cos(a),z+rz*math.sin(a)))
    for i in range(len(sections)-1):
        for j in range(sides):a=i*sides+j;b=i*sides+(j+1)%sides;f.append((a,b,b+sides,a+sides))
    f.extend([tuple(range(sides-1,-1,-1)),tuple((len(sections)-1)*sides+j for j in range(sides))])
    return mesh(name,v,f,mat,bone)


def coat(name,surface,count,mat,bone='body',length=.021):
    v=[];f=[]
    for _ in range(count):
        p,normal,tangent=surface();normal=Vector(normal).normalized();tangent=Vector(tangent).normalized();p=Vector(p)
        side=normal.cross(tangent).normalized()
        ln=length*random.uniform(.55,1.35);width=ln*random.uniform(.05,.085)
        # Lance-shaped two-triangle blades: buried root, raised middle, swept tip.
        base=len(v);root=p-normal*.0015;middle=p+tangent*ln*.45+normal*.002
        tip=p+tangent*ln+normal*.004
        v.extend([root,middle-side*width,tip,middle+side*width]);f.extend([(base,base+1,base+2),(base,base+2,base+3)])
    return mesh(name,v,f,mat,bone)


def ellipsoid_coat(center,axes):
    def surface():
        a=random.uniform(0,TAU);z=random.uniform(-.8,1);r=math.sqrt(1-z*z)
        q=Vector((r*math.cos(a),r*math.sin(a),z));p=Vector(center)+Vector(tuple(q[k]*axes[k] for k in range(3)))
        normal=Vector(tuple(q[k]/axes[k] for k in range(3))).normalized()
        direction=Vector((-1,0,-.25));tangent=(direction-normal*normal.dot(direction)).normalized()
        return p,normal,tangent
    return surface


def ear(name,center,side,mat,bone='head',pointed=False):
    # A cupped, genuinely thin pinna; its central fold is shaded by geometric AO.
    v=[];f=[];N=20
    for r in [0,.38,.75,1]:
        for j in range(N):
            a=TAU*j/N
            width=.030 if not pointed else .031
            height=.030 if not pointed else .046
            yy=width*r*math.cos(a)*(1-.62*max(0,math.sin(a)) if pointed else 1)
            if pointed:yy+=.016*r*max(0,math.sin(a))
            zz=height*r*math.sin(a)
            v.append((center[0]+.013*(1-r*r)+.003*math.sin(a),center[1]+side*yy,center[2]+zz))
    for i in range(3):
        for j in range(N):a=i*N+j;b=i*N+(j+1)%N;f.append((a,b,b+N,a+N))
    if side>0:f=[tuple(reversed(face)) for face in f]
    obj=mesh(name,v,f,mat,bone)
    # Merge repeated center vertices happens in build. Thin two-sided surface, no balloon.
    rim=[v[3*N+j] for j in range(N)]+[v[3*N]]
    tube(name+' rolled rim',rim,.0017,mat,bone,5)
    return obj


def rat(m,bones):
    bones['body']=((-.05,0,.1),(-.05,.1,.1),'root')
    bones['head']=((.055,0,.12),(.16,0,.10),'body')
    bones['jaw']=((.125,0,.073),(.207,0,.064),'head')
    sections=[(-.235,.009,.014,.128),(-.218,.052,.068,.146),(-.184,.078,.091,.151),(-.135,.086,.093,.15),(-.075,.077,.084,.146),(-.015,.062,.065,.138),(.048,.047,.053,.128),(.083,.023,.032,.12)]
    loft('Long back and narrow shoulders',sections,m['fur'])
    def back_surface():
        x=random.uniform(-.222,.065);i=next(i for i in range(len(sections)-1) if sections[i][0]<=x<=sections[i+1][0]);a,b=sections[i:i+2];t=(x-a[0])/(b[0]-a[0]);s=[a[k]*(1-t)+b[k]*t for k in range(4)]
        theta=random.uniform(-.4,math.pi+.4);p=(x,s[1]*math.cos(theta),s[3]+s[2]*math.sin(theta))
        slope=[(b[k]-a[k])/(b[0]-a[0]) for k in range(4)]
        along=Vector((1,slope[1]*math.cos(theta),slope[3]+slope[2]*math.sin(theta)))
        around=Vector((0,-s[1]*math.sin(theta),s[2]*math.cos(theta)))
        return p,around.cross(along),-along
    coat('Dense swept guard hairs',back_surface,1220,m['fur_tip'],length=.014)
    headsections=[(.046,.035,.036,.125),(.075,.043,.041,.118),(.115,.038,.037,.103),(.158,.030,.028,.085),(.193,.016,.015,.071),(.216,.006,.007,.068)]
    loft('Tapered wedge skull',headsections,m['fur'],'head',20)
    coat('Cheek hairs',ellipsoid_coat((.096,0,.109),(.045,.037,.037)),210,m['fur_tip'],'head',.010)
    ellipsoid('Lower jaw',(.169,0,.062),(.035,.017,.008),m['skin'],'jaw',12,6)
    ellipsoid('Nose',(.216,0,.07),(.006,.009,.006),m['skin'],'head',12,6)
    for s in [-1,1]:
        ear('Thin rounded ear',(.072,s*.043,.178),s,m['ear'])
        ellipsoid('Beady eye',(.119,s*.034,.125),(.007,.0045,.0065),m['eye'],'head',12,7)
        ellipsoid('Hind haunch',(-.161,s*.057,.102),(.060,.032,.061),m['fur'],'body',16,9)
        coat('Haunch guard hairs',ellipsoid_coat((-.161,s*.057,.102),(.060,.032,.061)),130,m['fur_tip'],length=.01)
        for front in [False,True]:
            bn=('front' if front else 'rear')+str(s)
            x=.062 if front else -.155
            bones[bn]=((x,s*.055,.09),(x+.02,s*.085,.032),'body')
            if front:pts=[(.058,s*.046,.126),(.038,s*.065,.07),(.081,s*.087,.022)]
            else:pts=[(-.145,s*.073,.103),(-.194,s*.085,.060),(-.153,s*.096,.024)]
            tube('Angled slender limb',pts,[.014,.01,.0055],m['fur'],bn,8)
            end=pts[-1];tube('Bare wrist',[end,(end[0]+.013,s*.099,.012)],[.006,.006],m['skin'],bn,6)
            for j in range(4):
                y=s*(.09+j*.008);x0=end[0]+.011;x1=x0+.023+(1-abs(j-1.5)/2)*.009
                tube('Pink articulated toe',[(x0,y,.012),((x0+x1)*.5,y+s*.004,.008),(x1,y+s*.006,.005)],[.0032,.0025,.0014],m['skin'],bn,5)
                tube('Pale curved claw',[(x1,y+s*.006,.006),(x1+.005,y+s*.007,.006),(x1+.007,y+s*.007,.002)],[.0017,.0012,.0002],m['claw'],bn,5)
        for j in range(4):
            tube('Fine whisker',[(.197,s*.013,.078+j*.003),(.204-j*.009,s*.056,.084+j*.007),(.189-j*.013,s*(.087+j*.006),.09+j*.007)],[.00045,.0003,.00006],m['claw'],'head',3)
    bones['tail']=((-.219,0,.104),(-.37,0,.04),'body')
    bones['tail_mid']=((-.37,0,.04),(-.52,.035,.015),'tail')
    bones['tail_tip']=((-.52,.035,.015),(-.66,.06,.012),'tail_mid')
    pts=[];r=[];weights=[]
    for i in range(39):
        t=i/38;pts.append((-.218-.442*t,.051*math.sin(t*2.1),.012+.092*(1-t)**3))
        r.append(.011*(1-t)**1.3+.0008)
        if t<.4: w=min(1,t/.4);weights.append({'tail':1-w,'tail_mid':w})
        else: w=(t-.4)/.6;weights.append({'tail_mid':1-w,'tail_tip':w})
    tube('Long ring-scaled tapering tail',pts,r,m['tail'],sides=7,weights=weights)


def bat(m,bones):
    bones['body']=((0,0,1),(0,.1,1),'root');bones['head']=((.02,0,1.055),(.1,0,1.075),'body')
    bones['jaw']=((.091,0,1.045),(.14,0,1.043),'head')
    ellipsoid('Small furry torso',(-.016,0,1.018),(.067,.043,.063),m['fur'],seg=20,rings=12)
    coat('Body ruff',ellipsoid_coat((-.016,0,1.018),(.067,.043,.063)),420,m['fur_tip'],length=.013)
    loft('Short wedge skull',[(.025,.023,.023,1.087),(.055,.042,.032,1.077),(.082,.036,.026,1.07),(.105,.026,.014,1.068),(.12,.014,.009,1.069)],m['fur'],'head',20)
    coat('Brow fur',ellipsoid_coat((.056,0,1.081),(.03,.037,.024)),120,m['fur_tip'],'head',.008)
    ellipsoid('Dark mouth opening',(.111,0,1.055),(.016,.024,.008),m['mouth'],'jaw',12,6)
    ellipsoid('Broad flat nose',(.12,0,1.074),(.007,.014,.007),m['skin'],'head',12,7)
    for s in [-1,1]:
        ellipsoid('Tiny nostril',(.126,s*.007,1.077),(.002,.0025,.0016),m['eye'],'head',8,5)
        ellipsoid('Bat eye',(.083,s*.030,1.083),(.0055,.004,.005),m['eye'],'head',12,7)
        tube('Brow ridge',[(.066,s*.03,1.096),(.083,s*.033,1.092),(.096,s*.026,1.083)],[.0025,.002,.0005],m['fur'],'head',7)
        tube('Small fang',[(.119,s*.019,1.06),(.12,s*.018,1.048)],[.003,.0003],m['tooth'],'head',6)
        ear('Pointed pinna',(.028,s*.036,1.13),s,m['ear'],pointed=True)
        tube('Pinna inner ridge',[(.038,s*.035,1.094),(.048,s*.041,1.123),(.037,s*.039,1.163)],[.0025,.002,.0004],m['skin'],'head',5)
        wing='wing'+str(s);tipbone='wingtip'+str(s)
        bones[wing]=((-.008,s*.035,1.042),(.025,s*.15,1.15),'body')
        bones[tipbone]=((.025,s*.15,1.15),(-.045,s*.388,1.215),wing)
        wrist=Vector((.035,.135,1.155))
        tips=[Vector((-.025,.387,1.225)),Vector((-.15,.318,1.087)),Vector((-.207,.198,.984)),Vector((-.145,.064,.947)),Vector((-.047,.036,.972))]
        v=[];faces=[];weights=[];U=10;R=8
        for panel in range(len(tips)-1):
            a,b=tips[panel:panel+2];base=len(v)
            for i in range(R+1):
                t=i/R
                for j in range(U+1):
                    u=j/U
                    # Scallop is a taut catenary-like curve pulled towards the wrist.
                    edge=a.lerp(b,u);edge=edge.lerp(wrist,.25*math.sin(math.pi*u))
                    q=wrist.lerp(edge,t);q.z+=.014*math.sin(math.pi*t)*math.sin(math.pi*u)
                    v.append((q.x,s*q.y,q.z));w=max(0,min(1,(q.y-.13)/.17))
                    weights.append({wing:1-w,tipbone:w})
            for i in range(R):
                for j in range(U):k=base+i*(U+1)+j;faces.append((k,k+1,k+U+2,k+U+1))
        if s>0:faces=[tuple(reversed(face)) for face in faces]
        mesh('Four scalloped membrane panels',v,faces,m['membrane'],weights=weights)
        tube('Arm and leading spar',[(-.015,s*.035,1.046),(-.036,s*.078,1.118),(.035,s*.135,1.155),(-.025,s*.387,1.225)],[.006,.005,.004,.001],m['skin'],wing,7,
             [{wing:1},{wing:1},{wing:1},{tipbone:1}])
        for tip in tips[1:]:
            pts=[];ws=[]
            for i in range(7):
                t=i/6;q=wrist.lerp(tip,t);q.z+=.004*math.sin(math.pi*t);pts.append((q.x,s*q.y,q.z+.001))
                w=max(0,min(1,(q.y-.13)/.17));ws.append({wing:1-w,tipbone:w})
            tube('Spreading finger',pts,[.0035*(1-i/7)+.0007 for i in range(7)],m['skin'],wing,5,ws)
        tube('Thumb hook',[(.035,s*.135,1.155),(.064,s*.139,1.18),(.073,s*.135,1.172)],[.004,.0025,.0003],m['claw'],wing,5)
        tube('Hind ankle',[(-.041,s*.025,.974),(-.09,s*.035,.946),(-.099,s*.039,.927)],[.005,.004,.002],m['skin'],'body',6)
        for j in range(3):tube('Hind claw',[(-.099,s*(.032+j*.007),.928),(-.084,s*(.032+j*.007),.92),(-.078,s*(.032+j*.007),.926)],[.002,.0014,.0002],m['claw'],'body',5)


def slime(m,bones):
    N=72;R=32;v=[(0,0,0)];f=[];weights=[{'root':1}]
    def height(x,y,r):
        mound=.29*math.exp(-(((x+.07)/.21)**2+((y-.055)/.21)**2)*1.25)
        mound+=.15*math.exp(-(((x-.16)/.115)**2+((y+.025)/.16)**2)*1.3)
        mound+=.12*math.exp(-(((x+.205)/.105)**2+((y+.15)/.11)**2))
        mound+=.085*math.exp(-(((x-.015)/.11)**2+((y+.245)/.10)**2))
        folds=.018*math.sin(x*49+2*math.sin(y*19))*math.sin(y*37+x*12)*math.sin(math.pi*r)
        return max(.006,.009+mound+folds)*(1-.45*r**12)
    for i in range(R):
        r=1-i/(R-1)
        for j in range(N):
            a=TAU*j/N;edge=.410*(1+.065*math.sin(5*a+.4)+.026*math.sin(11*a))
            x=r*edge*math.cos(a);y=r*edge*math.sin(a)
            z=height(x,y,r)
            if i==0:z=.008+.003*(1+math.sin(7*a))
            v.append((x,y,z));w=min(1,max(0,(z-.016)/.14));weights.append({'root':1-w,'body':w})
    for j in range(N):f.append((0,1+(j+1)%N,1+j))
    for i in range(R-1):
        for j in range(N):a=1+i*N+j;b=1+i*N+(j+1)%N;f.append((a,b,b+N,a+N))
    mesh('Asymmetric pooled jelly and streaming folds',v,f,m['slime'],weights=weights)
    # Surface-near inclusions model bubbles beneath a thin film, with the same skin.
    for x,y,r in [(.11,-.1,.023),(-.12,-.09,.019),(.20,.065,.022),(-.23,-.13,.014),(.03,.15,.016),(.16,-.2,.013),(-.16,.14,.024),(.25,-.03,.012)]:
        z=height(x,y,math.hypot(x,y)/.41)-r*.75
        obj=ellipsoid('Bubble beneath wet skin',(x,y,z),(r,r,r*.83),m['slime'],seg=12,rings=7)
        obj.vertex_groups.clear();root=obj.vertex_groups.new(name='root');body=obj.vertex_groups.new(name='body')
        for vert in obj.data.vertices:
            w=min(1,max(0,(vert.co.z-.016)/.14));root.add([vert.index],1-w,'REPLACE');body.add([vert.index],w,'REPLACE')


def create(kind):
    global PARTS
    PARTS=[];random.seed({'rat':508,'bat':509,'slime':510}[kind])
    m={k:shader(k,k) for k in ['fur','fur_tip','skin','tail','ear','eye','claw','mouth','tooth','membrane','slime']}
    bones={'root':((0,0,0),(0,0,.05),None),'body':((0,0,0),(0,.1,0),'root')}
    {'rat':rat,'bat':bat,'slime':slime}[kind](m,bones)
    if kind=='bat':
        # Authored 6% resting-span reserve keeps the downward flap inside 80 cm.
        for obj in PARTS:
            for v in obj.data.vertices:v.co.y*=.94
        bones={name:((head[0],head[1]*.94,head[2]),(tail[0],tail[1]*.94,tail[2]),parent)
               for name,(head,tail,parent) in bones.items()}
    return PARTS,bones
