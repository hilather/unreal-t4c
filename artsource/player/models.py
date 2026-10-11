"""Original W5-12 tailored starters. Procedural geometry, no image/mesh inputs.
All dimensions, materials and sculpt decisions are Prototype visual tuning.
Metres; +X forward, +Y left, +Z up. Fixed seed 51212 per construction.
"""
import math
import random
import bpy
from mathutils import Vector

PARTS=[]
M={}
TAU=math.tau

def mat(name,color,rough=.7,grain=110,bump=.0008,metal=0):
    m=bpy.data.materials.new(name);m.use_nodes=True
    n=m.node_tree.nodes;l=m.node_tree.links;p=n.get('Principled BSDF')
    p.inputs['Roughness'].default_value=rough;p.inputs['Metallic'].default_value=metal
    tex=n.new('ShaderNodeTexCoord');noise=n.new('ShaderNodeTexNoise')
    noise.inputs['Scale'].default_value=grain;noise.inputs['Detail'].default_value=3
    l.new(tex.outputs['Object'],noise.inputs['Vector'])
    ramp=n.new('ShaderNodeValToRGB');ramp.color_ramp.elements[0].position=.15;ramp.color_ramp.elements[1].position=.85
    ramp.color_ramp.elements[0].color=(*[c*.66 for c in color],1)
    ramp.color_ramp.elements[1].color=(*[c*1.13 for c in color],1)
    l.new(noise.outputs['Fac'],ramp.inputs[0]);l.new(ramp.outputs[0],p.inputs['Base Color'])
    detail=n.new('ShaderNodeBump');detail.inputs['Strength'].default_value=.32;detail.inputs['Distance'].default_value=bump
    l.new(noise.outputs['Fac'],detail.inputs['Height']);l.new(detail.outputs[0],p.inputs['Normal'])
    if name.startswith('linen'):
        wave=n.new('ShaderNodeTexWave');wave.bands_direction='Z';wave.inputs['Scale'].default_value=290
        l.new(tex.outputs['Object'],wave.inputs['Vector'])
        weave=n.new('ShaderNodeBump');weave.inputs['Strength'].default_value=.14;weave.inputs['Distance'].default_value=.00035
        l.new(wave.outputs[0],weave.inputs['Height']);l.new(detail.outputs[0],weave.inputs['Normal'])
        folds=n.new('ShaderNodeTexWave');folds.bands_direction='Z';folds.inputs['Scale'].default_value=9;folds.inputs['Distortion'].default_value=7;folds.inputs['Detail Scale'].default_value=1.3
        l.new(tex.outputs['Object'],folds.inputs['Vector'])
        cloth=n.new('ShaderNodeBump');cloth.inputs['Strength'].default_value=.16;cloth.inputs['Distance'].default_value=.002
        l.new(folds.outputs['Fac'],cloth.inputs['Height']);l.new(weave.outputs[0],cloth.inputs['Normal']);l.new(cloth.outputs[0],p.inputs['Normal'])
    return m

def mesh(name,verts,faces,material,bone='chest',weights=None,category='body'):
    d=bpy.data.meshes.new(name);d.from_pydata(verts,[],faces);d.update()
    o=bpy.data.objects.new(name,d);bpy.context.collection.objects.link(o);d.materials.append(M[material] if isinstance(material,str) else material)
    for f in d.polygons:f.use_smooth=True
    rows=weights or [{bone:1} for _ in verts]
    for key in sorted({k for row in rows for k in row}):
        g=o.vertex_groups.new(name=key)
        for i,row in enumerate(rows):
            if row.get(key,0)>0:g.add([i],row[key],'REPLACE')
    o['category']=category;PARTS.append(o);return o

def ellipsoid(name,c,r,material,bone='chest',seg=16,rings=10,category='body'):
    v=[(c[0],c[1],c[2]+r[2])];f=[]
    for i in range(1,rings):
        t=math.pi*i/rings
        for j in range(seg):
            a=TAU*j/seg;v.append((c[0]+r[0]*math.sin(t)*math.cos(a),c[1]+r[1]*math.sin(t)*math.sin(a),c[2]+r[2]*math.cos(t)))
    end=len(v);v.append((c[0],c[1],c[2]-r[2]))
    for j in range(seg):f.extend([(0,1+j,1+(j+1)%seg),(end,1+(rings-2)*seg+(j+1)%seg,1+(rings-2)*seg+j)])
    for i in range(rings-2):
        for j in range(seg):a=1+i*seg+j;b=1+i*seg+(j+1)%seg;f.append((a,a+seg,b+seg,b))
    return mesh(name,v,f,material,bone,category=category)

def tube(name,points,radii,material,bone='chest',sides=8,category='body',weights=None):
    v=[];f=[];w=[]
    for i,point in enumerate(points):
        tangent=(Vector(points[min(i+1,len(points)-1)])-Vector(points[max(0,i-1)])).normalized()
        basis=Vector((0,0,1)) if abs(tangent.z)<.95 else Vector((1,0,0))
        u=tangent.cross(basis).normalized();a=tangent.cross(u).normalized();r=radii[i] if isinstance(radii,(list,tuple)) else radii
        for j in range(sides):v.append(Vector(point)+r*(math.cos(TAU*j/sides)*u+math.sin(TAU*j/sides)*a));w.append(weights[i] if weights else {bone:1})
    for i in range(len(points)-1):
        for j in range(sides):a=i*sides+j;b=i*sides+(j+1)%sides;f.append((a,b,b+sides,a+sides))
    f.extend([tuple(range(sides-1,-1,-1)),tuple((len(points)-1)*sides+j for j in range(sides))])
    return mesh(name,v,f,material,bone,weights=w,category=category)

def loft(name,rows,material,bone='pelvis',sides=24,fold=0,weights=None,cap=True):
    """Sections (z,cx,cy,depth_x,width_y). Rings with authored cloth creases."""
    v=[];f=[];w=[]
    for i,(z,cx,cy,rx,ry) in enumerate(rows):
        for j in range(sides):
            a=TAU*j/sides;wr=fold*(math.sin(a*9+z*18)+.5*math.sin(a*15-z*25))
            v.append((cx+(rx+wr)*math.cos(a),cy+(ry+wr)*math.sin(a),z+fold*.3*math.sin(a*7+i)))
            w.append(weights[i] if weights else {bone:1})
    for i in range(len(rows)-1):
        for j in range(sides):a=i*sides+j;b=i*sides+(j+1)%sides;f.append((a,b,b+sides,a+sides))
    if cap:f.extend([tuple(range(sides-1,-1,-1)),tuple((len(rows)-1)*sides+j for j in range(sides))])
    return mesh(name,v,f,material,bone,weights=w)

def ring(name,z,rx,ry,material,bone='pelvis',cx=0,cy=0,r=.004,sides=36):
    p=[(cx+rx*math.cos(TAU*j/sides),cy+ry*math.sin(TAU*j/sides),z) for j in range(sides+1)]
    return tube(name,p,r,material,bone,sides=5)

def weights(z):
    if z<1.07:return {'pelvis':1}
    if z<1.24:
        t=(z-1.07)/.17;return {'pelvis':1-t,'spine':t}
    t=min(1,(z-1.24)/.13);return {'spine':1-t,'chest':t}

def face(female):
    # A continuous shaped skull: jaw plane, cheek shelf, temples, brow and crown.
    widths=[(1.506,.041,.035),(1.524,.064,.054),(1.548,.078,.065),(1.58,.086,.072),(1.611,.094,.075),(1.640,.093,.073),(1.665,.092,.074),(1.69,.094,.076),(1.715,.081,.067),(1.735,.057,.047),(1.744,.015,.014)]
    v=[];f=[];sides=40
    for z,rx,ry in widths:
        for j in range(sides):
            a=TAU*j/sides;x=rx*math.cos(a)-.008;y=ry*math.sin(a)
            if x>0:
                # Broad flattened face with orbital hollows and raised cheeks.
                x=rx*(.84+.16*math.cos(a))-.008
                x-=.018*math.exp(-((abs(y)-.036)/.022)**2-((z-1.642)/.018)**2)
                x+=.011*math.exp(-((abs(y)-.047)/.025)**2-((z-1.611)/.02)**2)
            if female:y*=.93
            v.append((x,y,z))
    for i in range(len(widths)-1):
        for j in range(sides):a=i*sides+j;b=i*sides+(j+1)%sides;f.append((a,b,b+sides,a+sides))
    f.extend([tuple(range(sides-1,-1,-1)),tuple((len(widths)-1)*sides+j for j in range(sides))])
    mesh('Face.B' if female else 'Face.A',v,f,'skin','head')
    # Wedge nose with a bridge and defined alae, not a separate round bead.
    loft('nose bridge',[(1.593,.087,0,.015,.019),(1.604,.101,0,.024,.016),(1.619,.096,0,.02,.011),(1.642,.087,0,.009,.009),(1.655,.082,0,.003,.007)],'skin','head',sides=12)
    for s in (-1,1):
        ellipsoid('nostril',(.099,s*.014,1.596),(.006,.004,.0025),'crease','head',seg=8,rings=5)
        # Narrow almond-shaped inset eyes, with iris and dark upper lids.
        eye_y=s*(.034 if female else .037)
        ellipsoid('eye white',(.066,eye_y,1.640),(.009,.017,.006),'eye','head',seg=16,rings=8)
        ellipsoid('hazel iris',(.0745,eye_y,1.640),(.0024,.006,.006),'iris','head',seg=12,rings=6)
        ellipsoid('pupil',(.076,eye_y,1.640),(.001,.0028,.0035),'pupil','head',seg=10,rings=6)
        for top in (True,False):
            p=[]
            for j in range(7):
                t=j/6;y=eye_y+(t-.5)*.038
                p.append((.073+.001*math.sin(math.pi*t),y,1.640+(1 if top else -1)*.006*math.sin(math.pi*t)))
            tube('upper lid' if top else 'lower lid',p,.0022,'skin','head',sides=5)
        tube('eyebrow',[(.079,s*.018,1.662),(.086,s*.032,1.668),(.078,s*.048,1.665),(.069,s*.056,1.660)],[.0028,.004,.003,.0008],'hair','head',sides=6)
        ellipsoid('ear',(-.005,s*.077,1.626),(.023,.012,.034),'skin','head',seg=12,rings=9)
        tube('ear rim',[(.008,s*.083,1.647),(.016,s*.085,1.63),(.012,s*.086,1.611),(.002,s*.08,1.602)],.004,'skin','head',sides=6)
    # Upper and lower lip ribbons follow a restrained cupid bow.
    for z,rr in [(1.569,.0038),(1.563,.0048)]:
        tube('lip',[(.075,-.024,z),(.080,-.012,z+.0015),(.082,0,z-.001),(.080,.012,z+.0015),(.075,.024,z)],[.001,rr,rr,rr,.001],'lip','head',sides=6)
    tube('mouth seam',[(.077,-.024,1.567),(.084,0,1.566),(.077,.024,1.567)],[.0006,.0011,.0006],'crease','head',sides=5)

def hair(style):
    category='Hair.'+style
    # Close-fit sculpted cap opens at brows/temples and reaches the nape.
    v=[(-.006,0,1.784)];f=[];segments=36;rings=9
    for i in range(1,rings+1):
        for j in range(segments):
            a=TAU*j/segments
            end=1.37 if math.cos(a)>.45 else 1.91 if math.cos(a)<-.4 else 1.67
            t=(i/rings)*end;ruff=.0025*math.sin(j*2.4+i*.5)
            v.append((-.006+(.113+ruff)*math.sin(t)*math.cos(a),(.093+ruff)*math.sin(t)*math.sin(a),1.659+(.125+ruff)*math.cos(t)))
    for j in range(segments):f.append((0,1+j,1+(j+1)%segments))
    for i in range(rings-1):
        for j in range(segments):a=1+i*segments+j;b=1+i*segments+(j+1)%segments;f.append((a,a+segments,b+segments,b))
    mesh(category+'.cap',v,f,'hair','head',category=category)
    # Broad overlapping sculpted locks, rooted on the cap rather than rope strands.
    for k in range(23):
        angle0=TAU*k/23
        verts=[];faces=[];length=1.26 if math.cos(angle0)>.35 else 1.73
        for i in range(7):
            t=i/6;theta=.15+t*length
            center=angle0+.52*math.sin(math.pi*t)+.12*math.sin(k*2.31)
            breadth=.17*(math.sin(math.pi*t)**.5)+.018
            for j in range(5):
                lateral=j/2-1;ang=center+lateral*breadth
                lift=.002+.0045*(1-lateral*lateral)*math.sin(math.pi*t)
                verts.append((-.006+(.115+lift)*math.sin(theta)*math.cos(ang),(.096+lift)*math.sin(theta)*math.sin(ang),1.659+(.128+lift)*math.cos(theta)))
        for i in range(6):
            for j in range(4):
                v=i*5+j;faces.append((v,v+1,v+6,v+5))
        mesh(category+'.swept lock',verts,faces,'hair_light' if k%5==0 else 'hair','head',category=category)
    if style=='Tied':
        ellipsoid(category+'.bun',(-.071,0,1.761),(.055,.059,.041),'hair','head',seg=18,rings=10,category=category)
        for j in range(8):
            a=TAU*j/8
            tube(category+'.bun ridge',[(-.046,.046*math.cos(a),1.76+.028*math.sin(a)),(-.087,.047*math.cos(a+.4),1.772+.029*math.sin(a+.4)),(-.116,.021*math.cos(a+.9),1.773+.018*math.sin(a+.9))],[.003,.005,.001],'hair_light' if j%3==0 else 'hair','head',sides=5,category=category)
        for s in (-1,1):
            tube(category+'.temple wisp',[(.062,s*.061,1.707),(.08,s*.072,1.675),(.055,s*.077,1.633),(.061,s*.072,1.609)],[.005,.007,.004,.0005],'hair','head',sides=6,category=category)

def fuse_parts(objects,name,material,target,weight_fn,voxel=.003):
    """Build-time voxel union only; final export is ordinary reduced topology."""
    if not objects:return
    bpy.ops.object.select_all(action='DESELECT')
    for o in objects:o.select_set(True)
    bpy.context.view_layer.objects.active=objects[0];bpy.ops.object.join();o=objects[0]
    for ob in objects:PARTS.remove(ob)
    o.name=name;o.data.remesh_voxel_size=voxel;bpy.ops.object.voxel_remesh()
    smooth=o.modifiers.new('Sculpt blend','SMOOTH');smooth.factor=.8;smooth.iterations=4;bpy.ops.object.modifier_apply(modifier=smooth.name)
    o.data.calc_loop_triangles();dec=o.modifiers.new('Sculpt reduction','DECIMATE');dec.ratio=min(1,target/len(o.data.loop_triangles));bpy.ops.object.modifier_apply(modifier=dec.name)
    o.data.materials.clear();o.data.materials.append(M[material]);o.vertex_groups.clear();groups={}
    for vertex in o.data.vertices:
        for key,w in weight_fn(vertex.co).items():
            if w>0:
                if key not in groups:groups[key]=o.vertex_groups.new(name=key)
                groups[key].add([vertex.index],w,'REPLACE')
    for polygon in o.data.polygons:polygon.use_smooth=True
    o['category']='body';PARTS.append(o)
    return o


def create(kind):
    PARTS.clear();random.seed(51212);female=kind=='player_b';M.clear()
    M.update({
        'linen':mat('linen',((.43,.38,.30) if female else (.35,.29,.215)),.88,80,.0012),
        'linen_edge':mat('linen edging',(.34,.275,.18),.9,90,.0008),
        'pants':mat('trouser twill',(.12,.076,.044),.86,100,.0009),
        'leather':mat('worn leather',(.082,.041,.019),.61,170,.0009),
        'edge':mat('leather edge wear',(.18,.101,.047),.72,180,.0005),
        'stitch':mat('flax stitch',(.32,.24,.13),.84,120,.0002),
        'metal':mat('aged brass',(.29,.19,.075),.43,90,.0003,.8),
        'skin':mat('skin neutral tint mask',(.83,.80,.76),.65,180,.0003),
        'lip':mat('skin lip neutral tint mask',(.54,.37,.34),.68,160,.0003),
        'crease':mat('warm crevice',(.042,.022,.015),.86,100,0),
        'hair':mat('chestnut hair',(.026,.014,.008),.76,120,.0006),
        'hair_light':mat('hair ridges',(.045,.025,.012),.76,170,.0004),
        'eye':mat('eye ivory',(.46,.40,.30),.35,80,0),
        'iris':mat('hazel',(.062,.038,.013),.32,80,0),
        'pupil':mat('pupil',(.002,.0015,.001),.28,80,0),
    })
    # Neck emerges through an actual open collar and continues into shoulder skin.
    loft('neck',[(1.32,-.006,0,.102,.128),(1.40,-.006,0,.071,.086),(1.46,-.007,0,.056,.055),(1.50,-.006,0,.045,.044),(1.55,-.006,0,.045,.043)],'skin','neck',sides=24)
    # Main tunic silhouette: spare cloth at the chest, gathered waist, split skirt.
    rows=[(.835,0,0,.113,.188),(.89,0,0,.123,.184),(.96,0,0,.122,.171),(1.02,0,0,.112,.15),(1.06,0,0,.108,.149),(1.09,0,0,.120,.156),(1.15,0,0,.121,.164),(1.23,0,0,.13,.185),(1.30,0,0,.134,.201),(1.37,0,0,.119,.205),(1.415,0,0,.092,.196),(1.45,0,0,.057,.081)]
    shirt=loft('tunic',rows,'linen',sides=48,fold=.005,weights=[weights(r[0]) for r in rows],cap=False)
    # Shape the hem into two slightly uneven front tails; open laced V at the collar.
    for vert in shirt.data.vertices:
        co=vert.co
        angle=math.atan2(co.y/.19,co.x/.12)
        drape=.006*math.sin(angle*11+co.z*13)*math.exp(-((co.z-1.12)/.15)**2)
        co.x+=drape*math.cos(angle);co.y+=drape*math.sin(angle)
        if co.z<.90:
            co.z+=.035*math.exp(-(co.y/.025)**2)*max(0,co.x/.12)+.007*math.sin(co.y*35)
        if co.z>1.445:
            forward=max(0,math.cos(angle));co.z-=.105*forward**4;co.x+=.083*forward**6
        if female and co.z>1.1:co.y*=.91
    # Collar facing follows the neckline; cross lacing bridges the opening.
    collar=[]
    for j in range(49):
        a=TAU*j/48;collar.append((.059*math.cos(a)+.083*max(0,math.cos(a))**6,.083*math.sin(a),1.452-.105*max(0,math.cos(a))**4))
    tube('bound collar',collar,.007,'linen_edge','chest',sides=6)
    for i in range(4):
        z=1.362+i*.02;half=.014+i*.006;x=.149-i*.015
        for s in (-1,1):
            tube('collar lace',[(x,s*half,z),(x+.006,-s*(half+.005),z+.019)],.0019,'leather','chest',sides=5)
    # Belt, stitched rolled edges, a real buckle opening and hanging keeper.
    loft('wide belt',[(1.024,0,0,.121,.162),(1.035,0,0,.125,.164),(1.077,0,0,.122,.158),(1.086,0,0,.115,.155)],'leather',sides=40,cap=False)
    for z in (1.035,1.077):ring('belt piping',z,.127,.165,'edge',r=.0023)
    tube('buckle',[(.133,-.035,1.035),(.137,-.04,1.04),(.137,-.04,1.078),(.133,.024,1.078),(.137,.029,1.071),(.137,.029,1.035),(.133,-.035,1.035)],.004,'metal','pelvis',sides=6)
    tube('buckle tongue',[(.142,-.031,1.055),(.145,.021,1.055)],.0025,'metal','pelvis',sides=6)
    # Belt pouch rests on right hip, with a folded flap and button.
    pouch=ellipsoid('belt pouch',(.07,-.167,.966),(.044,.056,.069),'leather','pelvis',seg=20,rings=12)
    ellipsoid('pouch flap',(.101,-.174,1.003),(.016,.052,.035),'edge','pelvis',seg=16,rings=8)
    ellipsoid('pouch button',(.119,-.176,.993),(.002,.005,.005),'metal','pelvis',seg=8,rings=6)
    tube('belt tail',[(.119,.058,1.054),(.137,.096,1.006),(.134,.128,.939),(.107,.145,.895)],[.011,.012,.011,.004],'leather','pelvis',sides=6)
    for s,side in [(1,'L'),(-1,'R')]:
        thigh='thigh.'+side;shin='shin.'+side;foot='foot.'+side;arm='upper_arm.'+side;forearm='forearm.'+side;hand='hand.'+side
        # Trousers include a hip below the tunic, rumpled knee, tucked boot top.
        rows=[(.445,.001,s*.112,.067,.065),(.48,.001,s*.112,.072,.075),(.515,.006,s*.114,.082,.076),(.55,.012,s*.116,.077,.071),(.595,.009,s*.116,.080,.077),(.65,.002,s*.114,.082,.079),(.73,-.004,s*.112,.083,.08),(.81,-.004,s*.105,.087,.089),(.91,-.003,s*.101,.092,.094),(.975,-.003,s*.095,.083,.090)]
        legw=[]
        for row in rows:
            blend=max(0,min(1,(row[0]-.47)/.14));legw.append({shin:1-blend,thigh:blend})
        loft('trousers '+side,rows,'pants',sides=24,fold=.007,weights=legw)
        # Slender worn boots, distinct knee cuff, ankle folds, welt and toe box.
        rows=[(.075,.031,s*.112,.10,.062),(.115,.007,s*.112,.064,.060),(.15,-.008,s*.112,.052,.054),(.195,-.008,s*.112,.057,.06),(.23,-.008,s*.112,.06,.061),(.28,-.004,s*.112,.067,.065),(.34,0,s*.112,.069,.066),(.39,0,s*.112,.07,.066),(.445,0,s*.112,.075,.069),(.475,0,s*.112,.076,.071)]
        bootw=[{foot:1} if r[0]<.115 else {shin:1} for r in rows]
        loft('boot shaft '+side,rows,'leather',sides=24,fold=.002,weights=bootw)
        loft('boot cuff '+side,[(.422,0,s*.112,.075,.073),(.435,0,s*.112,.082,.079),(.475,0,s*.112,.083,.076),(.488,0,s*.112,.077,.072)],'edge',shin,sides=24,fold=.0015)
        # Horizontal ellipsoid with clipped planar sole.
        shoe=ellipsoid('boot toe '+side,(.075,s*.112,.071),(.143,.070,.064),'leather',foot,seg=24,rings=12)
        for vv in shoe.data.vertices:vv.co.z=max(.022,vv.co.z)
        loft('sole '+side,[(.008,.064,s*.112,.151,.073),(.022,.064,s*.112,.15,.075),(.035,.067,s*.112,.142,.071)],'leather',foot,sides=28)
        ring('welt '+side,.035,.144,.073,'edge',foot,cx=.067,cy=s*.112,r=.0025,sides=28)
        for k in range(3):
            z=.155+k*.085
            tube('boot strap '+side,[(.046,s*.053,z+.018),(.064,s*.109,z),(.032,s*.174,z-.013)],.006,'edge',shin,sides=6)
        # Upper arm profile through a softly rolled short sleeve.
        rows=[(1.14,.002,s*.294,.045,.045),(1.18,0,s*.282,.051,.05),(1.225,0,s*.265,.055,.055),(1.275,-.002,s*.245,.067,.061),(1.303,-.002,s*.238,.060,.058)]
        loft('upper arm '+side,rows,'skin',arm,sides=20)
        rows=[(1.278,0,s*.25,.073,.068),(1.29,0,s*.247,.076,.071),(1.34,-.003,s*.227,.08,.078),(1.39,-.003,s*.206,.078,.078),(1.428,-.006,s*.195,.066,.069),(1.441,-.006,s*.184,.023,.032)]
        loft('sleeve '+side,rows,'linen',arm,sides=24,fold=.003)
        ring('sleeve hem '+side,1.284,.075,.070,'linen_edge',arm,cy=s*.25,r=.003)
        rows=[(.944,.021,s*.335,.032,.030),(.98,.013,s*.33,.038,.036),(1.02,.008,s*.321,.041,.038),(1.08,.004,s*.31,.05,.044),(1.14,.002,s*.294,.046,.044),(1.175,0,s*.284,.044,.043)]
        loft('forearm '+side,rows,'skin',forearm,sides=20)
        rows=[(.98,.014,s*.331,.042,.039),(1.005,.011,s*.325,.046,.044),(1.067,.005,s*.31,.054,.049),(1.087,.004,s*.307,.053,.048)]
        loft('bracer '+side,rows,'leather',forearm,sides=20,fold=.0015)
        for k in range(4):
            z=.994+k*.025;cy=s*(.329-k*.006)
            ring('bracer wrap '+side,z,.047+k*.002,.045+k*.001,'edge',forearm,cx=.012,cy=cy,r=.003,sides=20)
        # Palm, four rounded fingers and curled thumb; no mitten sphere.
        ellipsoid('palm '+side,(.021,s*.34,.924),(.032,.036,.049),'skin',hand,seg=16,rings=10)
        for j in range(4):
            yy=s*(.314+j*.017);length=[.048,.058,.055,.042][j]
            tube('finger '+side,[(.025,yy,.904),(.036,yy,.904-length*.55),(.052,yy,.904-length),(.063,yy,.910-length)],[.009,.0085,.007,.004],'skin',hand,sides=7)
        tube('thumb '+side,[(.033,s*.307,.942),(.060,s*.301,.924),(.072,s*.313,.907)],[.014,.012,.008],'skin',hand,sides=8)
    face(female)
    fuse_parts([o for o in PARTS if o.name.startswith(('Face.','nose bridge','ear.')) or o.name=='ear'],'sculpted face','skin',1800,lambda v:{'head':1},.0018)
    for side in ('L','R'):
        def arm_weights(v):
            z=v.z;upper=max(0,min(1,(z-1.14)/.07));palm=max(0,min(1,(.985-z)/.06));fore=1-upper-palm
            return {'upper_arm.'+side:upper,'forearm.'+side:max(0,fore),'hand.'+side:palm}
        fuse_parts([o for o in PARTS if any(o.name.startswith(n+' '+side) for n in ('upper arm','forearm','palm','finger','thumb'))],'continuous arm '+side,'skin',1400,arm_weights,.0025)
    # Give the open tunic thickness before fusing its shoulder seams into sleeves.
    bpy.ops.object.select_all(action='DESELECT');shirt.select_set(True);bpy.context.view_layer.objects.active=shirt
    solid=shirt.modifiers.new('cloth thickness','SOLIDIFY');solid.thickness=.006;bpy.ops.object.modifier_apply(modifier=solid.name)
    def cloth_weights(v):
        arm=max(0,min(1,(abs(v.y)-.17)/.085)) if v.z>1.20 else 0
        body=weights(v.z);body={k:w*(1-arm) for k,w in body.items()}
        if arm:body['upper_arm.'+('L' if v.y>0 else 'R')]=arm
        return body
    fuse_parts([shirt]+[o for o in PARTS if o.name in ('sleeve L','sleeve R')],'continuous tunic','linen',2800,cloth_weights,.0035)
    hair('Cropped');hair('Tied')
    # Proportional body B: shorter and narrower shoulders, not a scaled male head.
    def adjust(v):
        x,y,z=v
        if female:
            y*=.89 if z>1.12 and z<1.51 else .95
            x*=.95
            z*=.955
        return (x,y,z)
    for o in PARTS:
        if o.name.startswith(('wide belt','belt piping')):
            for polygon in o.data.polygons:polygon.use_smooth=False
        for v in o.data.vertices:v.co=adjust(v.co)
    bones={
        'root':((0,0,0),(0,0,.15),None),
        'pelvis':((0,0,.91),(0,0,1.08),'root'),
        'spine':((0,0,1.08),(0,0,1.30),'pelvis'),
        'chest':((0,0,1.30),(0,0,1.46),'spine'),
        'neck':((0,0,1.46),(0,0,1.55),'chest'),
        'head':((0,0,1.55),(0,0,1.74),'neck'),
    }
    for s,side in [(1,'L'),(-1,'R')]:
        bones.update({
            'thigh.'+side:((0,s*.10,.92),(.015,s*.11,.53),'pelvis'),
            'shin.'+side:((.015,s*.11,.53),(0,s*.11,.11),'thigh.'+side),
            'foot.'+side:((0,s*.11,.11),(.14,s*.11,.07),'shin.'+side),
            'upper_arm.'+side:((0,s*.20,1.43),(0,s*.285,1.17),'chest'),
            'forearm.'+side:((0,s*.285,1.17),(.012,s*.33,.97),'upper_arm.'+side),
            'hand.'+side:((.012,s*.33,.97),(.025,s*.34,.86),'forearm.'+side),
        })
    bones.update({
        'Socket.Weapon.R':((.054,-.341,.907),(.154,-.341,.907),'hand.R'),
        'Socket.Bow.L':((.054,.341,.907),(.054,.341,1.007),'hand.L'),
        'Socket.Arrow':((.063,-.328,.911),(.163,-.328,.911),'hand.R'),
        'Socket.Quiver.Back':((-.14,.09,1.28),(-.14,.09,1.38),'chest'),
    })
    bones={name:(adjust(h),adjust(t),parent) for name,(h,t,parent) in bones.items()}
    return list(PARTS),bones
