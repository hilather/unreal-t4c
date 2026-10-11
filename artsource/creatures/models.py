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


def slime_height(x, y, r):
    # W5-08f: one connected off-centre dome, with low shoulders feeding the
    # pooled skirt. All dimensions are prototype presentation tuning in metres.
    dome=.352*math.exp(-(((x+.035)/.235)**2+((y-.04)/.225)**2)**1.35)
    shoulder=.185*math.exp(-(((x-.18)/.17)**2+((y+.13)/.155)**2)**1.4)
    rear=.12*math.exp(-(((x+.20)/.145)**2+((y+.17)/.14)**2)**1.3)
    mass=(dome**5+shoulder**5+rear**5)**.2
    folds=.009*math.sin(x*28+2*math.sin(y*15))*math.sin(y*24+x*9)*math.sin(math.pi*r)
    return max(.005,.009+mass+folds)*(1-.55*r**12)


def ramp(nodes, values):
    node = nodes.new('ShaderNodeValToRGB')
    for i, (position, color) in enumerate(values):
        e = node.color_ramp.elements[i] if i < 2 else node.color_ramp.elements.new(position)
        e.position = position
        e.color = (*color, 1)
    return node


def shader(name, kind, creature='rat'):
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
        # Broad anatomical values survive minification; the fine strokes do not
        # carry identity. Dark saddle, warm flanks, pale underside, silver tips.
        color = ramp(n, [(.20,(.32,.27,.23)),(.48,(.73,.64,.52)),(.68,(1.15,1.03,.88)),(.84,(1.55,1.4,1.17))])
        l.new(noise.outputs['Fac'], color.inputs[0])
        sep = n.new('ShaderNodeSeparateXYZ'); l.new(tex.outputs['Object'],sep.inputs[0])
        height = n.new('ShaderNodeMapRange')
        height.inputs['From Min'].default_value=.05 if creature=='rat' else .935
        height.inputs['From Max'].default_value=.247 if creature=='rat' else 1.14
        l.new(sep.outputs['Z'],height.inputs['Value'])
        region=ramp(n,[(.02,(.23,.17,.11)),(.27,(.185,.102,.046)),(.50,(.105,.052,.023)),(.76,(.057,.032,.018)),(1,(.055,.038,.025))])
        if creature=='bat':
            region.color_ramp.elements[-1].color=(.095,.041,.014,1)
        l.new(height.outputs[0],region.inputs[0])
        coat_color=region.outputs[0]
        if creature=='rat':
            across=n.new('ShaderNodeMath');across.operation='ABSOLUTE';l.new(sep.outputs['Y'],across.inputs[0])
            ridge=n.new('ShaderNodeMapRange');ridge.inputs['From Max'].default_value=.024;ridge.inputs['To Min'].default_value=1;ridge.inputs['To Max'].default_value=0;l.new(across.outputs[0],ridge.inputs['Value'])
            high=n.new('ShaderNodeMapRange');high.inputs['From Min'].default_value=.19;high.inputs['From Max'].default_value=.236;l.new(sep.outputs['Z'],high.inputs['Value'])
            mask=n.new('ShaderNodeMath');mask.operation='MULTIPLY';l.new(ridge.outputs[0],mask.inputs[0]);l.new(high.outputs[0],mask.inputs[1])
            tips=n.new('ShaderNodeMixRGB');l.new(mask.outputs[0],tips.inputs[0]);l.new(coat_color,tips.inputs[1]);tips.inputs[2].default_value=(.12,.087,.055,1);coat_color=tips.outputs[0]
        mult = n.new('ShaderNodeMixRGB');mult.blend_type='MULTIPLY';mult.inputs[0].default_value=1
        l.new(color.outputs[0],mult.inputs[1]);l.new(coat_color,mult.inputs[2]);l.new(mult.outputs[0],p.inputs['Base Color'])
        p.inputs['Roughness'].default_value = .83
        bump.inputs['Strength'].default_value=.48
        bump.inputs['Distance'].default_value=.0018
    elif kind in ('skin','tail','ear'):
        noise.inputs['Scale'].default_value=180
        color=ramp(n,[(.22,(.16,.062,.044)),(.72,(.34,.17,.12))])
        l.new(noise.outputs['Fac'],color.inputs[0]);l.new(color.outputs[0],p.inputs['Base Color'])
        p.inputs['Roughness'].default_value=.57
        if kind=='tail':
            wave=n.new('ShaderNodeTexWave');wave.wave_type='BANDS';wave.bands_direction='X'
            wave.inputs['Scale'].default_value=45;wave.inputs['Distortion'].default_value=.65
            l.new(tex.outputs['Object'],wave.inputs['Vector']);l.new(wave.outputs['Fac'],bump.inputs['Height'])
            bump.inputs['Strength'].default_value=.3
            rings=ramp(n,[(.18,(.62,.55,.50)),(.38,(1,1,1)),(.85,(1,1,1))])
            l.new(wave.outputs['Fac'],rings.inputs[0])
            mix=n.new('ShaderNodeMixRGB');mix.blend_type='MULTIPLY';mix.inputs[0].default_value=.45
            l.new(color.outputs[0],mix.inputs[1]);l.new(rings.outputs[0],mix.inputs[2]);l.new(mix.outputs[0],p.inputs['Base Color'])
        if kind=='ear':
            color.color_ramp.elements[0].color=(.19,.061,.045,1)
            color.color_ramp.elements[1].color=(.46,.235,.18,1)
            p.inputs['Roughness'].default_value=.62
    elif kind=='membrane':
        noise.inputs['Scale'].default_value=30
        color=ramp(n,[(.2,(.030,.008,.002)),(.48,(.090,.026,.006)),(.78,(.19,.073,.022))])
        l.new(noise.outputs['Fac'],color.inputs[0])
        veins=n.new('ShaderNodeTexVoronoi');veins.feature='DISTANCE_TO_EDGE';veins.inputs['Scale'].default_value=72
        l.new(tex.outputs['Object'],veins.inputs['Vector'])
        veincolor=ramp(n,[(.006,(.14,.08,.045)),(.035,(1,1,1))]);l.new(veins.outputs['Distance'],veincolor.inputs[0])
        mix=n.new('ShaderNodeMixRGB');mix.blend_type='MULTIPLY';mix.inputs[0].default_value=.55
        l.new(color.outputs[0],mix.inputs[1]);l.new(veincolor.outputs[0],mix.inputs[2]);l.new(mix.outputs[0],p.inputs['Base Color'])
        l.new(veins.outputs['Distance'],bump.inputs['Height']);bump.inputs['Distance'].default_value=.0005
        p.inputs['Roughness'].default_value=.70
    elif kind=='slime':
        noise.inputs['Scale'].default_value=9
        # Opaque thickness illusion: dense central mass and a pale thin rim.
        # Deliberately no Voronoi rings: they read as painted polka dots.
        color=ramp(n,[(.20,(.003,.022,.0015)),(.46,(.009,.062,.003)),(.72,(.028,.12,.007)),(.86,(.055,.18,.011))])
        l.new(noise.outputs['Fac'],color.inputs[0])
        sep=n.new('ShaderNodeSeparateXYZ');l.new(tex.outputs['Object'],sep.inputs[0])
        depth=n.new('ShaderNodeMapRange');depth.inputs['From Min'].default_value=.025;depth.inputs['From Max'].default_value=.18
        depth.inputs['To Min'].default_value=1;depth.inputs['To Max'].default_value=0
        l.new(sep.outputs['Z'],depth.inputs['Value'])
        rim=ramp(n,[(.18,(.075,.24,.012)),(.78,(.19,.42,.035))]);l.new(noise.outputs['Fac'],rim.inputs[0])
        mix=n.new('ShaderNodeMixRGB');l.new(depth.outputs[0],mix.inputs[0]);l.new(color.outputs[0],mix.inputs[1]);l.new(rim.outputs[0],mix.inputs[2])
        # Only four irregular soft inclusions, colored beneath the surface film.
        previous=mix.outputs[0]
        for x,y,radius in [(.12,-.10,.017),(-.12,-.145,.013),(.18,.075,.011),(-.19,.115,.014)]:
            center=(x,y,slime_height(x,y,math.hypot(x,y)/.41)-.008)
            dist=n.new('ShaderNodeVectorMath');dist.operation='DISTANCE';dist.inputs[1].default_value=center;l.new(tex.outputs['Object'],dist.inputs[0])
            fade=n.new('ShaderNodeMapRange');fade.interpolation_type='SMOOTHERSTEP';fade.inputs['From Min'].default_value=radius*.2;fade.inputs['From Max'].default_value=radius*2.2;fade.inputs['To Min'].default_value=.42;fade.inputs['To Max'].default_value=0;l.new(dist.outputs['Value'],fade.inputs['Value'])
            bubble=n.new('ShaderNodeMixRGB');l.new(fade.outputs[0],bubble.inputs[0]);l.new(previous,bubble.inputs[1]);bubble.inputs[2].default_value=(.12,.23,.04,1);previous=bubble.outputs[0]
        l.new(previous,p.inputs['Base Color'])
        rough=n.new('ShaderNodeMapRange');rough.inputs['To Min'].default_value=.14;rough.inputs['To Max'].default_value=.22
        l.new(noise.outputs['Fac'],rough.inputs[0]);l.new(rough.outputs[0],p.inputs['Roughness'])
        bump.inputs['Strength'].default_value=.16;bump.inputs['Distance'].default_value=.0025
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
        ln=(length(p) if callable(length) else length)*random.uniform(.55,1.35);width=ln*random.uniform(.025,.055)
        # Lance-shaped two-triangle blades: buried root, raised middle, swept tip.
        base=len(v);root=p-normal*.001;middle=p+tangent*ln*.45+normal*(ln*.14)
        tip=p+tangent*ln+normal*(ln*.28)
        v.extend([root,middle-side*width,tip,middle+side*width]);f.extend([(base,base+1,base+2),(base,base+2,base+3)])
    return mesh(name,v,f,mat,bone)


def fuse_rat_anatomy(parts, mat):
    """Unify intersecting muscle masses, then restore explicit deformation zones.

    Voxel remesh is applied at build time only. No runtime sculpt modifier or
    high-poly mesh is exported; the final continuous skin is ~1750 triangles.
    """
    bpy.ops.object.select_all(action='DESELECT')
    for obj in parts:obj.select_set(True)
    obj=parts[0];bpy.context.view_layer.objects.active=obj
    bpy.ops.object.join()
    for part in parts:PARTS.remove(part)
    obj.name='Continuous rat shoulder neck and haunch skin'
    obj.data.remesh_voxel_size=.0035
    bpy.ops.object.voxel_remesh()
    sm=obj.modifiers.new('Blend anatomical transitions','SMOOTH');sm.factor=1.15;sm.iterations=5
    bpy.ops.object.modifier_apply(modifier=sm.name)
    dec=obj.modifiers.new('Authored skin budget','DECIMATE');dec.ratio=min(1,1750/(len(obj.data.polygons)*2))
    bpy.ops.object.modifier_apply(modifier=dec.name)
    obj.data.materials.clear();obj.data.materials.append(mat)
    for poly in obj.data.polygons:poly.use_smooth=True
    obj.vertex_groups.clear();PARTS.append(obj)
    def weights(co):
        x,y,z=co
        head=max(0,min(1,(x-.024)/.047))
        if x<.095:head*=max(0,min(1,(z-.075)/.025))
        limb=max(0,min(.9,(.099-z)/.044))*(1-head)
        bn=('front' if x>-.055 else 'rear')+str(1 if y>0 else -1)
        return {'head':head,'body':1-limb-head,bn:limb}
    def skin(ob):
        ob.vertex_groups.clear()
        groups={bn:ob.vertex_groups.new(name=bn) for bn in ['body','head','front-1','front1','rear-1','rear1']}
        for vert in ob.data.vertices:
            for bn,w in weights(vert.co).items():
                if w>0:groups[bn].add([vert.index],w,'REPLACE')
    skin(obj)
    # Root each guard hair on the final fused surface, including the neck and
    # haunch, instead of leaving smooth balls between independent fur patches.
    obj.data.calc_loop_triangles()
    triangles=[tri for tri in obj.data.loop_triangles if tri.center.z>.065 and tri.center.x<.19 and tri.normal.z>-.55]
    import bisect
    cumulative=[];total=0
    for tri in triangles:total+=tri.area;cumulative.append(total)
    def surface():
        tri=triangles[bisect.bisect_left(cumulative,random.random()*total)]
        a,b,c=[obj.data.vertices[i] for i in tri.vertices]
        u=math.sqrt(random.random());v=random.random()
        p=a.co*(1-u)+b.co*(u*(1-v))+c.co*(u*v)
        normal=(a.normal*(1-u)+b.normal*(u*(1-v))+c.normal*(u*v)).normalized()
        direction=Vector((-1,0,-.15))
        tangent=(direction-normal*normal.dot(direction)).normalized()
        return p,normal,tangent
    fur=coat('Continuous swept anatomical coat',surface,1600,mat,length=lambda p:.005 if p.x>.09 else (.009 if p.x>.025 else .013))
    skin(fur)
    return obj


def ellipsoid_coat(center,axes):
    def surface():
        a=random.uniform(0,TAU);z=random.uniform(-.8,1);r=math.sqrt(1-z*z)
        q=Vector((r*math.cos(a),r*math.sin(a),z));p=Vector(center)+Vector(tuple(q[k]*axes[k] for k in range(3)))
        normal=Vector(tuple(q[k]/axes[k] for k in range(3))).normalized()
        direction=Vector((-1,0,-.25));tangent=(direction-normal*normal.dot(direction)).normalized()
        return p,normal,tangent
    return surface


def ear(name,center,side,mat,bone='head',pointed=False,outer=None):
    # A cupped, genuinely thin pinna; its central fold is shaded by geometric AO.
    v=[];f=[];N=20
    for r in [0,.38,.75,1]:
        for j in range(N):
            a=TAU*j/N
            width=.027 if not pointed else .031
            height=.032 if not pointed else .046
            yy=width*r*math.cos(a)*(1-.62*max(0,math.sin(a)) if pointed else 1)
            if pointed:yy+=.016*r*max(0,math.sin(a))
            zz=height*r*math.sin(a)
            v.append((center[0]-.012*(1-r*r)+.007*r*math.sin(a),center[1]+side*(yy+.008*r*math.sin(a)),center[2]+zz))
    for i in range(3):
        for j in range(N):a=i*N+j;b=i*N+(j+1)%N;f.append((a,b,b+N,a+N))
    if side>0:f=[tuple(reversed(face)) for face in f]
    obj=mesh(name,v,f,mat,bone)
    if outer:
        mesh(name+' thin furred back',[(x-.0015,y,z) for x,y,z in v],[tuple(reversed(face)) for face in f],outer,bone)
    # Merge repeated center vertices happens in build. Thin two-sided surface, no balloon.
    rim=[v[3*N+j] for j in range(N)]+[v[3*N]]
    tube(name+' rolled rim',rim,.0012,mat,bone,5)
    return obj


def rat(m,bones):
    bones['body']=((-.05,0,.1),(-.05,.1,.1),'root')
    bones['head']=((.055,0,.12),(.16,0,.10),'body')
    bones['jaw']=((.125,0,.073),(.207,0,.064),'head')
    sections=[(-.235,.009,.014,.128),(-.218,.052,.068,.146),(-.184,.078,.091,.151),(-.135,.086,.093,.15),(-.075,.077,.084,.146),(-.015,.062,.065,.138),(.048,.047,.053,.128),(.083,.023,.032,.12)]
    anatomy=[loft('Long back and narrow shoulders',sections,m['fur'])]
    headsections=[(.014,.049,.053,.132),(.042,.049,.049,.128),(.075,.043,.043,.119),(.115,.038,.037,.103),(.135,.034,.031,.097),(.158,.028,.025,.086),(.178,.022,.020,.079),(.193,.016,.015,.071),(.216,.006,.007,.068)]
    anatomy.append(loft('Tapered wedge skull',headsections,m['fur'],'head',20))
    ellipsoid('Lower jaw',(.169,0,.062),(.035,.017,.008),m['skin'],'jaw',12,6)
    ellipsoid('Nose',(.216,0,.07),(.006,.009,.006),m['skin'],'head',12,6)
    for s in [-1,1]:
        anatomy.append(ellipsoid('Soft whisker pad',(.177,s*.014,.076),(.028,.016,.016),m['fur'],'head',12,7))
        ear('Cupped pink pinna',(.063,s*.044,.177),s,m['ear'],outer=m['fur'])
        ellipsoid('Beady eye',(.119,s*.030,.122),(.006,.004,.0055),m['eye'],'head',12,7)
        anatomy.append(ellipsoid('Rounded powerful hind haunch',(-.16,s*.061,.101),(.062,.039,.066),m['fur'],'body',16,9))
        anatomy.append(ellipsoid('Blended shoulder mass',(.028,s*.041,.102),(.044,.029,.047),m['fur'],'body',12,7))
        for front in [False,True]:
            bn=('front' if front else 'rear')+str(s)
            x=.062 if front else -.155
            bones[bn]=((x,s*.055,.09),(x+.02,s*.085,.032),'body')
            if front:pts=[(.040,s*.049,.126),(.035,s*.066,.075),(.081,s*.087,.022)]
            else:pts=[(-.149,s*.077,.101),(-.190,s*.086,.061),(-.153,s*.096,.024)]
            anatomy.append(tube('Tapered muscular limb',pts,[.026,.016,.0065],m['fur'],bn,8))
            end=pts[-1];tube('Bare wrist',[end,(end[0]+.013,s*.099,.012)],[.006,.006],m['skin'],bn,6)
            for j in range(4):
                y=s*(.09+j*.008);x0=end[0]+.011;x1=x0+.023+(1-abs(j-1.5)/2)*.009
                tube('Pink articulated toe',[(x0,y,.012),((x0+x1)*.5,y+s*.004,.008),(x1,y+s*.006,.005)],[.0032,.0025,.0014],m['skin'],bn,5)
                tube('Pale curved claw',[(x1,y+s*.006,.006),(x1+.005,y+s*.007,.006),(x1+.007,y+s*.007,.002)],[.0017,.0012,.0002],m['claw'],bn,5)
        for j in range(4):
            tube('Fine whisker',[(.197,s*.013,.078+j*.003),(.204-j*.009,s*.056,.084+j*.007),(.189-j*.013,s*(.087+j*.006),.09+j*.007)],[.00045,.0003,.00006],m['claw'],'head',3)
    fuse_rat_anatomy(anatomy,m['fur'])
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
    # Shorten the distal muzzle while preserving the 45 cm body allowance.
    # The broad cheek-to-whisker-pad union avoids a long planar shrew snout.
    def shorten(x):return .10+(x-.10)*.82 if x>.10 else x
    for obj in PARTS:
        for vert in obj.data.vertices:vert.co.x=shorten(vert.co.x)
    for name,(head,tail,parent) in list(bones.items()):
        bones[name]=((shorten(head[0]),head[1],head[2]),(shorten(tail[0]),tail[1],tail[2]),parent)


def slime(m,bones):
    N=80;R=24;v=[(0,0,0)];f=[];weights=[{'root':1}]
    for i in range(R):
        r=1-i/(R-1)
        for j in range(N):
            a=TAU*j/N;edge=.409*(1+.064*math.sin(5*a+.4)+.032*math.sin(9*a+.3))
            x=r*edge*math.cos(a);y=r*edge*math.sin(a)
            z=slime_height(x,y,r)
            # Low rounded edge lobes feed into several small flattened drips.
            z+=.040*math.exp(-((r-.85)/.095)**2)*(max(0,math.sin(7*a+.7))**4)
            if i==0:z=.0025
            v.append((x,y,z));w=min(1,max(0,(z-.016)/.14));weights.append({'root':1-w,'body':w})
    for j in range(N):f.append((0,1+(j+1)%N,1+j))
    for i in range(R-1):
        for j in range(N):a=1+i*N+j;b=1+i*N+(j+1)%N;f.append((a,b,b+N,a+N))
    obj=mesh('Domed wobbling body with pooled bright rim',v,f,m['slime'],weights=weights)
    # A flat underside must not pull the wet upper lip's normals downward.
    # Keep the outer rim below its adjacent ring to avoid isolated upturned
    # triangular flaps that reflect as black notches in a grazing hero view.
    for poly in obj.data.polygons[:N]:poly.use_smooth=False
    for edge in obj.data.edges:
        if all(1<=i<=N for i in edge.vertices):edge.use_edge_sharp=True


def create(kind):
    global PARTS
    PARTS=[];random.seed({'rat':508,'bat':509,'slime':510}[kind])
    m={k:shader(k,k,kind) for k in ['fur','fur_tip','skin','tail','ear','eye','claw','mouth','tooth','membrane','slime']}
    bones={'root':((0,0,0),(0,0,.05),None),'body':((0,0,0),(0,.1,0),'root')}
    from bat_polish import bat as polished_bat
    {'rat':rat,'bat':polished_bat,'slime':slime}[kind](m,bones)
    if kind=='bat':
        # Shallow wings need only a 2% span reserve inside the 80 cm envelope.
        for obj in PARTS:
            for v in obj.data.vertices:v.co.y*=.98
        bones={name:((head[0],head[1]*.98,head[2]),(tail[0],tail[1]*.98,tail[2]),parent)
               for name,(head,tail,parent) in bones.items()}
    return PARTS,bones
