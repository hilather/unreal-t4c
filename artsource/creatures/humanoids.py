"""Original W5-08e goblin family and atrocity, in metres, +X forward.

These are authored prototype presentation models, not recovered T4C assets.
The concept sheet is a visual target only. No image or external geometry input.
Public API: create(kind) -> (parts, bone definitions); pose(kind, rig, clip, f, end).
"""
import math
import random
import bpy
from mathutils import Vector, Quaternion, Matrix
import models as M


def material(name, low, high, rough=.72, skin=False):
    m=bpy.data.materials.new(name);m.use_nodes=True
    n=m.node_tree.nodes;l=m.node_tree.links;p=n.get('Principled BSDF')
    p.inputs['Roughness'].default_value=rough
    if 'forged iron' in name.lower() or 'bevel' in name.lower():p.inputs['Metallic'].default_value=.58
    tex=n.new('ShaderNodeTexCoord');noise=n.new('ShaderNodeTexNoise')
    noise.inputs['Scale'].default_value=31 if skin else 48
    noise.inputs['Detail'].default_value=3;noise.inputs['Roughness'].default_value=.72
    l.new(tex.outputs['Object'],noise.inputs['Vector'])
    c=M.ramp(n,[(.18,low),(.80,high)])
    l.new(noise.outputs['Fac'],c.inputs[0]);color=c.outputs[0]
    if skin:
        # Broad anatomical value separation survives both bake and far camera.
        sep=n.new('ShaderNodeSeparateXYZ');l.new(tex.outputs['Object'],sep.inputs[0])
        front=n.new('ShaderNodeMapRange');front.inputs['From Min'].default_value=-.22
        front.inputs['From Max'].default_value=.28
        front.inputs['To Min'].default_value=.54;front.inputs['To Max'].default_value=1.15
        l.new(sep.outputs['X'],front.inputs['Value'])
        mul=n.new('ShaderNodeMixRGB');mul.blend_type='MULTIPLY';mul.inputs[0].default_value=1
        l.new(color,mul.inputs[1]);l.new(front.outputs[0],mul.inputs[2]);color=mul.outputs[0]
        pores=n.new('ShaderNodeTexNoise');pores.inputs['Scale'].default_value=180
        pores.inputs['Detail'].default_value=2;l.new(tex.outputs['Object'],pores.inputs['Vector'])
        bump=n.new('ShaderNodeBump');bump.inputs['Strength'].default_value=.33
        bump.inputs['Distance'].default_value=.004
        l.new(pores.outputs['Fac'],bump.inputs['Height']);l.new(bump.outputs[0],p.inputs['Normal'])
    l.new(color,p.inputs['Base Color'])
    return m


def solid(name,points,faces,mat,bone='body'):
    obj=M.mesh(name,points,faces,mat,bone)
    for p in obj.data.polygons:p.use_smooth=False
    return obj


def plate(name,outline,depth,mat,bone='body'):
    """Front-facing edged plate: outline is a closed sequence of XYZ vertices."""
    front=[tuple(Vector(p)+Vector((depth/2,0,0))) for p in outline]
    back=[tuple(Vector(p)-Vector((depth/2,0,0))) for p in outline]
    n=len(front);v=front+back
    faces=[tuple(range(n)),tuple(range(n,2*n))[::-1]]
    faces.extend((i,(i+1)%n,(i+1)%n+n,i+n) for i in range(n))
    return solid(name,v,faces,mat,bone)


def ridge(name,points,widths,heights,normal,mat,bone='body'):
    """A broad, angular raised hide plate, embedded at its two outer seams."""
    v=[];faces=[];normal=Vector(normal).normalized()
    for i,point in enumerate(points):
        p=Vector(point)
        tangent=(Vector(points[min(i+1,len(points)-1)])-Vector(points[max(0,i-1)])).normalized()
        across=tangent.cross(normal).normalized()
        v.extend([p-across*widths[i],p+normal*heights[i],p+across*widths[i]])
    for i in range(len(points)-1):
        a=i*3;faces.extend([(a,a+3,a+4,a+1),(a+1,a+4,a+5,a+2)])
    faces.extend([(0,1,2),tuple(range(len(v)-3,len(v)))[::-1]])
    return solid(name,v,faces,mat,bone)


def fused(parts,mat,bones,budget,name,voxel=.012):
    bpy.ops.object.select_all(action='DESELECT')
    for p in parts:p.select_set(True)
    bpy.context.view_layer.objects.active=parts[0];bpy.ops.object.join();obj=bpy.context.object
    for p in parts:M.PARTS.remove(p)
    obj.name=name;obj.data.remesh_voxel_size=voxel;bpy.ops.object.voxel_remesh()
    smooth=obj.modifiers.new('Blend real muscle transitions','SMOOTH');smooth.factor=.85;smooth.iterations=4
    bpy.ops.object.modifier_apply(modifier=smooth.name)
    dec=obj.modifiers.new('Retain anatomical planes within budget','DECIMATE')
    dec.ratio=min(1,budget/(len(obj.data.polygons)*2));bpy.ops.object.modifier_apply(modifier=dec.name)
    obj.data.materials.clear();obj.data.materials.append(mat);obj.vertex_groups.clear()
    radii={'body':.21,'head':.13}
    for s in ['L','R']:
        radii.update({f'upper_{s}':.12,f'fore_{s}':.085,f'hand_{s}':.065,
                      f'thigh_{s}':.14,f'shin_{s}':.09,f'foot_{s}':.09})
    if name.startswith('Atrocity'):
        radii={k:v*1.5 for k,v in radii.items()}
    groups={k:obj.vertex_groups.new(name=k) for k in radii}
    for v in obj.data.vertices:
        scores=[]
        for bn,r in radii.items():
            a,b,_=bones[bn];a=Vector(a);seg=Vector(b)-a
            t=max(0,min(1,(v.co-a).dot(seg)/seg.length_squared))
            distance=(v.co-a-seg*t).length
            score=math.exp(-3*(distance/r)**2)
            scores.append((score,bn))
        scores=sorted(scores,reverse=True)[:3];total=sum(s for s,_ in scores)
        if total<1e-30:scores=[(1, min(radii,key=lambda bn:(v.co-Vector(bones[bn][0])).length))];total=1
        for score,bn in scores:
            w=score/total
            if w>.001:groups[bn].add([v.index],w,'REPLACE')
    for p in obj.data.polygons:p.use_smooth=True
    M.PARTS.append(obj)
    return obj


def biped_bones(atrocity=False):
    if atrocity:
        hip=(-.25,0,.82);chest=(-.01,0,1.40);neck=(.31,0,1.30);head=(.56,0,1.17)
    else:
        hip=(-.12,0,.62);chest=(.025,0,.94);neck=(.12,0,1.03);head=(.22,0,1.15)
    b={'root':((0,0,0),(0,0,.1),None),'body':(hip,chest,'root'),'head':(neck,head,'body')}
    for s,side in [(1,'L'),(-1,'R')]:
        if atrocity:
            shoulder=(-.035,s*.40,1.36);elbow=(.13,s*.66,.94);wrist=(.43,s*.68,.53);hand=(.58,s*.70,.43)
            thigh=(-.25,s*.20,.85);knee=(.035,s*.32,.49);ankle=(-.19,s*.34,.12);toe=(.14,s*.34,.055)
        else:
            shoulder=(.005,s*.205,.91);elbow=(.12,s*.285,.70)
            wrist=(.36,-.285,.76) if side=='R' else (.31,-.145,.71)
            hand=(.38,-.30,.75) if side=='R' else (.365,-.245,.73)
            thigh=(-.12,s*.135,.62);knee=(.18,s*.23,.37);ankle=(-.025,s*.24,.105);toe=(.17,s*.255,.04)
        b[f'upper_{side}']=(shoulder,elbow,'body')
        b[f'fore_{side}']=(elbow,wrist,f'upper_{side}')
        b[f'hand_{side}']=(wrist,hand,f'fore_{side}')
        b[f'thigh_{side}']=(thigh,knee,'body');b[f'shin_{side}']=(knee,ankle,f'thigh_{side}')
        b[f'foot_{side}']=(ankle,toe,f'shin_{side}')
    return b


def goblin(variant):
    warrior=variant=='goblin_warrior';bones=biped_bones()
    m={
      'skin':material('Iron red skin',(.075,.008,.005),(.22,.032,.015),.7,True),
      'lip':material('Deep mouth and eyelid shadow',(.018,.002,.001),(.05,.007,.004)),
      'ear':material('Warm inner ear cartilage',(.18,.014,.006),(.40,.054,.023)),
      'cloth':material('Coarse dark umber waistcloth',(.025,.018,.011),(.075,.05,.028),.93),
      'leather':material('Worn leather harness',(.023,.014,.007),(.075,.046,.021),.8),
      'linen':material('Broad dusty pale warrior back band',(.26,.22,.14),(.53,.44,.29),.9),
      'iron':material('Restrained pale forged iron',(.22,.24,.23),(.51,.53,.50),.48),
      'edge':material('Blade bevel',(.39,.41,.38),(.65,.66,.60),.38),
      'wood':material('Dark ash spear shaft',(.024,.012,.005),(.085,.043,.014),.87),
      'bone':material('Dull ivory teeth and claws',(.27,.19,.09),(.58,.46,.25),.58),
      'eye':material('Amber eyes in recessed sockets',(.48,.12,.001),(.85,.36,.009),.35),
      'pupil':material('Black slit pupils',(.002,.001,.0005),(.006,.002,.001),.35)
    }
    body=[]
    e=lambda name,c,a,b='body',seg=16,rings=10:body.append(M.ellipsoid(name,c,a,m['skin'],b,seg,rings))
    t=lambda name,p,r,b='body':body.append(M.tube(name,p,r,m['skin'],b,12))
    e('Pelvis',(-.105,0,.61),(.14,.16,.125))
    e('Narrow wiry abdomen',(-.06,0,.76),(.125,.13,.20))
    e('Rib cage leaned over hips',(.005,0,.9),(.13,.19,.145))
    e('Trapezius and neck',(.065,0,1.015),(.115,.135,.11))
    for s,side in [(1,'L'),(-1,'R')]:
        e('Pectoral plane',(.098,s*.075,.89),(.054,.079,.095))
        e('Deltoid',(.01,s*.19,.92),(.103,.091,.10))
        for section,r in [('upper',[.073,.062,.042]),('fore',[.042,.059,.029]),('thigh',[.087,.083,.055]),('shin',[.059,.053,.027])]:
            bn=f'{section}_{side}';a,z,_=bones[bn];mid=tuple((Vector(a)+Vector(z))*.5)
            t('Blended '+bn,[a,mid,z],r,bn)
        e('Anatomical elbow union',bones[f'fore_{side}'][0],(.052,.052,.052),f'fore_{side}',12,8)
        e('Defined patella union',bones[f'shin_{side}'][0],(.063,.055,.063),f'shin_{side}',12,8)
        wrist,hand,_=bones[f'hand_{side}']
        e('Bony grasping palm',hand,(.055,.041,.047),f'hand_{side}',12,8)
        # Right hand wraps the shaft; left support remains distinct below it.
        for j in range(4):
            z=.733+j*.019 if side=='R' else .69+j*.016
            y=-.30 if side=='R' else -.24
            M.tube('Curled grip finger',[(.395,y-.022,z),(.420,y-.041,z),(.397,y-.058,z-.006)],
                   [.011,.010,.007],m['skin'],f'hand_{side}',6)
        e('Long claw foot',(.062,s*.248,.065),(.14,.066,.048),f'foot_{side}',14,8)
        for j in range(3):
            y=s*(.208+j*.040)
            M.tube('Articulated toe',[(.12,y,.067),(.21,y,.048),(.235,y,.035)],[.021,.018,.009],m['skin'],f'foot_{side}',7)
            M.tube('Small horn toenail',[(.21,y,.049),(.25,y,.036),(.268,y,.020)],[.014,.010,.001],m['bone'],f'foot_{side}',6)
    # The angular facial wedge is anatomically separate from the high cranial dome.
    e('Cranium',(.148,0,1.11),(.106,.093,.105),'head',20,12)
    e('Forward cheek and maxilla',(.239,0,1.064),(.079,.080,.051),'head',16,10)
    e('Underslung pointed jaw',(.239,0,1.019),(.086,.065,.029),'head',16,8)
    for s in [-1,1]:
        e('Cheekbone',(.242,s*.065,1.088),(.05,.039,.038),'head',12,8)
        t('Angular heavy brow',[(.19,s*.091,1.16),(.266,s*.064,1.145),(.285,s*.029,1.133)], [.028,.027,.018],'head')
    t('Long hooked nose bridge',[(.248,0,1.15),(.291,0,1.12),(.335,0,1.07),(.322,0,1.05)],[.032,.033,.026,.014],'head')
    fused(body,m['skin'],bones,4400,'Goblin continuous shoulders limbs and facial anatomy')
    # Deliberately retain sharp facial planes after the continuous-skin pass.
    for s in [-1,1]:
        plate('Sharp overhanging scowl brow',[(.258,s*.013,1.143),(.300,s*.049,1.151),(.272,s*.104,1.178),(.245,s*.112,1.158),(.291,s*.062,1.130)],.018,m['skin'],'head')
        plate('Swept cheek plane',[(.276,s*.062,1.105),(.252,s*.096,1.095),(.293,s*.067,1.05),(.313,s*.038,1.065)],.012,m['skin'],'head')
        M.tube('Raised neck tendon',[(.121,s*.082,1.035),(.146,s*.067,.982),(.12,s*.11,.92)],[.018,.013,.007],m['skin'],'body',7)
        M.tube('Forehead cartilage ridge',[(.143,s*.043,1.211),(.207,s*.029,1.191),(.252,s*.022,1.158)],[.018,.014,.009],m['skin'],'head',7)
    solid('Faceted hooked nasal ridge',[(.269,-.021,1.156),(.269,.021,1.156),(.350,0,1.073),(.319,-.029,1.061),(.319,.029,1.061)],[(0,1,2),(0,2,3),(1,4,2),(2,4,3),(0,3,4,1)],m['skin'],'head')
    # Flattened leaf ears with inset cartilage, not conical cartoon balloons.
    for s,side in [(1,'L'),(-1,'R')]:
        bn='ear_'+side;bones[bn]=((.14,s*.075,1.12),(.04,s*.34,1.245),'head')
        outline=[(.14,s*.082,1.075),(.165,s*.16,1.11),(.033,s*.405,1.25),(.068,s*.19,1.205),(.105,s*.081,1.185)]
        plate('Long pointed ear',outline,.015,m['skin'],bn)
        plate('Inset ear fold',[(.159,s*.105,1.108),(.165,s*.16,1.125),(.054,s*.345,1.224),(.12,s*.18,1.173)],.002,m['ear'],bn)
        M.tube('Ear raised helix',[outline[0],outline[1],outline[2],outline[3],outline[4]],[.009,.009,.001,.007,.01],m['skin'],bn,6)
        M.ellipsoid('Recessed almond eye',(.279,s*.051,1.114),(.013,.022,.011),m['lip'],'head',12,7)
        M.ellipsoid('Amber iris',(.288,s*.051,1.116),(.007,.013,.007),m['eye'],'head',10,7)
        M.ellipsoid('Slit pupil',(.294,s*.049,1.116),(.003,.0025,.006),m['pupil'],'head',8,6)
        M.ellipsoid('Nostril',(.327,s*.017,1.069),(.004,.006,.004),m['lip'],'head',8,6)
    M.ellipsoid('Narrow snarling mouth',(.303,0,1.04),(.025,.063,.012),m['lip'],'head',16,6)
    for s in [-1,1]:
        for j in range(4):
            y=s*(.012+j*.013);x=.325-.22*y*y
            M.tube('Visible uneven sharp tooth',[(x,y,1.048),(x+.003,y,1.031-(.01 if j==3 else 0))],[.0055,.0008],m['bone'],'head',5)
    # Ragged cloth panels occupy a continuous belt silhouette and split for legs.
    for s in [-1,1]:
        for j in range(3):
            y=s*(.015+j*.042)
            plate('Layered tattered waist wrap',[(.018,y-.029,.65),(.018,y+.029,.65),(.07,y+.03,.42),(.086,y,.36+.018*j),(.07,y-.03,.43)],.009,m['cloth'])
        plate('Rear waist leather panel',[(-.231,s*.012,.645),(-.216,s*.135,.645),(-.22,s*.145,.46),(-.235,s*.06,.405)],.011,m['cloth'])
    belt=[]
    for j in range(25):
        a=math.tau*j/24;belt.append((-.09+.149*math.cos(a),.173*math.sin(a),.645))
    M.tube('Broad waist belt',belt,.025,m['leather'],'body',6)
    M.ellipsoid('Waist clasp',(.069,0,.647),(.021,.033,.037),m['iron'],'body',10,7)
    # Narrow asymmetrical harness leaves the ordinary shoulders bare.
    M.tube('Diagonal chest strap',[(.079,-.125,.96),(.145,-.056,.88),(.099,.052,.72),(.027,.10,.65)],[.014,.016,.016,.014],m['leather'],'body',6)
    if warrior:
        for s in [-1,1]:
            # Thick curved leather shoulder shell sits outside the deltoid skin.
            for layer in range(2):
                v=[];faces=[]
                for back in [0,1]:
                    for j in range(5):
                        x=-.17+j*.087
                        for k in range(4):
                            y=.13+k*.093+layer*.021
                            z=1.072-.22*(y-.13)-.95*abs(x-.015)**1.1-layer*.045-back*.028
                            v.append((x,s*y,z))
                for j in range(4):
                    for k in range(3):
                        a=j*4+k;faces.extend([(a,a+4,a+5,a+1),(a+20,a+21,a+25,a+24)])
                edge=[0,4,8,12,16,17,18,19,15,11,7,3,2,1]
                for j,a in enumerate(edge):
                    c=edge[(j+1)%len(edge)];faces.append((a,c,c+20,a+20))
                solid('Broad curved layered leather shoulder shell',v,faces,m['leather'])
            for j in range(3):M.ellipsoid('Dull iron yoke rivet',(.121,s*(.19+j*.07),.98-j*.014),(.009,.01,.009),m['iron'],'body',8,5)
        # The band follows the back arc continuously across the shoulders.
        M.tube('Single broad continuous pale back band',[(-.18,-.34,.962),(-.186,-.22,1.007),(-.192,0,1.012),(-.186,.22,1.007),(-.18,.34,.962)],.033,m['linen'],'body',6)
    shaft_x=.391;shaft_y=-.30;bottom=.13 if warrior else .11;top=2.09 if warrior else 1.84
    bones['Weapon_R']=((shaft_x,shaft_y,.758),(shaft_x,shaft_y,1.00),'hand_R')
    bones['Weapon_Main']=((shaft_x,shaft_y,.758),(shaft_x,shaft_y,.82),'Weapon_R')
    bones['Weapon_Support']=((shaft_x,shaft_y,.715),(shaft_x,shaft_y,.76),'Weapon_R')
    bones['VFX_WeaponTip']=((shaft_x,shaft_y,top),(shaft_x,shaft_y,top+.02),'Weapon_R')
    bn='Weapon_R'
    M.tube('Straight dark polearm shaft',[(shaft_x,shaft_y,bottom),(shaft_x,shaft_y,top-.29)],[.014,.011],m['wood'],bn,10)
    M.tube('Iron butt ferrule',[(shaft_x,shaft_y,bottom),(shaft_x,shaft_y,bottom+.13)],[.016,.016],m['iron'],bn,8)
    for j in range(9):
        z=top-.44+j*.022
        M.tube('Spiral binding',[(shaft_x+.016*math.cos(a),shaft_y+.016*math.sin(a),z+.016*a/math.tau) for a in [math.tau*k/12 for k in range(13)]],.003,m['leather'],bn,4)
    half=.08 if warrior else .037;start=top-.32
    # Diamond-section leaf blade with real central ridge and bevel material.
    v=[(shaft_x,shaft_y,top),(shaft_x,shaft_y-half,start+.11),(shaft_x+.022,shaft_y,start+.13),
       (shaft_x,shaft_y+half,start+.11),(shaft_x,shaft_y,start), (shaft_x-.014,shaft_y,start+.13)]
    solid('Ridged pale leaf spear blade',v,[(0,1,2),(0,2,3),(1,4,2),(2,4,3),(0,5,1),(0,3,5),(1,5,4),(3,4,5)],m['iron'],bn)
    if warrior:
        for s in [-1,1]:
            plate('Wide forked warrior blade',[(shaft_x,shaft_y+s*.025,start+.04),(shaft_x,shaft_y+s*.105,start+.105),(shaft_x,shaft_y+s*.115,start+.29),(shaft_x,shaft_y+s*.052,start+.19)],.015,m['edge'],bn)
    bones['VFX_Hit']=((.09,0,.86),(.16,0,.86),'body');bones['UI_Anchor']=((0,0,1.34),(0,0,1.39),'body')
    return M.PARTS,bones


def atrocity():
    bones=biped_bones(True)
    skin=material('Charcoal violet ridged hide',(.008,.006,.011),(.037,.028,.047),.74,True)
    chitin=material('Dark angular layered hide ridges',(.005,.004,.009),(.023,.017,.033),.56)
    sinew=material('Raised violet tendon edges',(.015,.010,.02),(.046,.035,.059),.64,True)
    horn=material('Aged ochre keratin',(.16,.10,.015),(.41,.29,.055),.56)
    dark=material('Deep facial fissures',(.006,.002,.004),(.016,.005,.006),.8)
    eye=material('Sunken warm atrocity eyes',(.4,.025,.003),(.78,.10,.004),.4)
    body=[]
    def e(n,c,a,b='body',seg=16,rings=10):body.append(M.ellipsoid(n,c,a,skin,b,seg,rings))
    def t(n,p,r,b='body'):body.append(M.tube(n,p,r,skin,b,12))
    e('Haunch pelvis',(-.25,0,.84),(.25,.27,.24))
    e('Oblique low abdomen',(-.15,0,1.06),(.24,.26,.32))
    e('Deep hunched rib cage',(-.015,0,1.35),(.34,.40,.28))
    e('Arched shoulder hump',(-.14,0,1.47),(.31,.29,.23))
    e('Heavy hanging neck',(.26,0,1.33),(.24,.18,.17))
    e('Narrow sloping cranium',(.43,0,1.29),(.23,.14,.13),'head',18,10)
    e('Long bony muzzle',(.59,0,1.17),(.145,.10,.08),'head',16,9)
    e('Narrow mandible',(.565,0,1.105),(.15,.09,.052),'head',14,8)
    for s,side in [(1,'L'),(-1,'R')]:
        e('Massive curved deltoid',(-.03,s*.37,1.39),(.24,.21,.22))
        e('Pectoral under shoulder',(.21,s*.19,1.30),(.11,.19,.13))
        e('Broad latissimus',(-.23,s*.20,1.15),(.17,.15,.25))
        for j in range(3):
            e('Separated descending rib mass',(.055+j*.04,s*(.12+j*.015),1.00+j*.092),(.13,.14,.052))
        for section,r in [('upper',[.18,.17,.105]),('fore',[.105,.12,.065]),('thigh',[.17,.19,.10]),('shin',[.10,.085,.047])]:
            bn=f'{section}_{side}';a,z,_=bones[bn]
            mid=tuple((Vector(a)+Vector(z))*.5+Vector((-.025,0,.02)))
            t('Continuous '+bn,[a,mid,z],r,bn)
        e('Large rounded elbow union',bones[f'fore_{side}'][0],(.113,.108,.11),f'fore_{side}',12,8)
        e('Bony knee union',bones[f'shin_{side}'][0],(.115,.102,.115),f'shin_{side}',12,8)
        t('Continuous carpal bridge',[(.35,s*.675,.67),(.445,s*.69,.535),(.545,s*.70,.455)],[.102,.087,.077],f'fore_{side}')
        e('Knuckled long hand',(.54,s*.70,.46),(.14,.105,.085),f'hand_{side}',14,9)
        e('Splayed long foot',(-.03,s*.35,.075),(.22,.105,.063),f'foot_{side}',16,8)
        t('Angular cheek and jaw ridge',[(.39,s*.12,1.30),(.55,s*.10,1.205),(.68,s*.062,1.15)],[.04,.035,.015],'head')
        t('Swept predatory brow',[(.38,s*.125,1.38),(.51,s*.095,1.31),(.58,s*.050,1.272)],[.039,.032,.021],'head')
    skin_obj=fused(body,skin,bones,4700,'Atrocity continuous muscular shoulder and limb anatomy',.017)
    def dermal_shield(name,outline):
        # Project broad polygonal shields onto the final muscle surface. The
        # buried perimeter and bevel ring grow out of the skin rather than
        # leaving unrelated stones floating above an otherwise smooth body.
        cx=sum(x for x,y in outline)/len(outline);cy=sum(y for x,y in outline)/len(outline)
        def surface(x,y,lift):
            hit,p,normal,_=skin_obj.ray_cast(Vector((x,y,2.2)),Vector((0,0,-1)))
            if not hit:_,p,normal,_=skin_obj.closest_point_on_mesh(Vector((x,y,1.5)))
            return p+normal*lift
        v=[surface(x,y,.002) for x,y in outline]
        v.extend(surface(cx+(x-cx)*.83,cy+(y-cy)*.83,.024) for x,y in outline)
        v.append(surface(cx,cy,.05));n=len(outline);f=[]
        for i in range(n):
            j=(i+1)%n;f.extend([(i,j,j+n,i+n),(i+n,j+n,2*n)])
        solid(name,v,f,chitin)
    # Broad keratin ridges break up the hump and scapulae; avoid a smooth ape back.
    for j in range(5):
        x=-.37+j*.088
        dermal_shield('Integrated overlapping dorsal shield',[(x-.064,-.063),(x-.046,.064),(x+.048,.10),(x+.086,0),(x+.048,-.10)])
    for s,side in [(1,'L'),(-1,'R')]:
        for j in range(3):
            y=.205+j*.092;x=-.12+j*.018
            dermal_shield('Broad integrated scapular shield',[(x-.12,s*(y-.035)),(x-.16,s*(y+.036)),
                (x-.035,s*(y+.092)),(x+.135,s*(y+.075)),(x+.20,s*(y-.006)),(x+.02,s*(y-.046))])
        ridge('Pectoral crest',[(.20,s*.06,1.40),(.326,s*.18,1.36),(.23,s*.31,1.27)],
              [.018,.066,.028],[.008,.034,.012],(1,0,.25),sinew)
        ridge('Long separated upper arm crest',[(.15,s*.45,1.36),(.245,s*.535,1.16),(.218,s*.65,.974)],
              [.025,.079,.022],[.013,.038,.010],(1,s*.2,.05),chitin,f'upper_{side}')
        M.tube('Upper arm sinew',[(.13,s*.49,1.33),(.236,s*.575,1.17),(.22,s*.672,.995)],
               [.023,.028,.011],sinew,f'upper_{side}',7)
        for j in range(3):
            y=s*(.655+(j-1)*.025)
            M.tube('Long visible forearm tendon',[(.225,y,.948),(.34,y+s*.007,.79),(.458,y+s*.025,.60),(.55,y+s*.041,.465)],
                   [.012,.018,.014,.005],sinew,sides=7,
                   weights=[{f'fore_{side}':1},{f'fore_{side}':1},{f'fore_{side}':.75,f'hand_{side}':.25},{f'hand_{side}':1}])
        ridge('Elongated outer forearm hide plate',[(.22,s*.72,.955),(.365,s*.75,.77),(.466,s*.745,.61)],
              [.025,.066,.026],[.014,.039,.012],(.75,s*.6,.1),chitin,f'fore_{side}')
        ridge('Angular kneecap',[(.105,s*.32,.575),(.148,s*.32,.498),(.12,s*.32,.425)],
              [.032,.059,.024],[.006,.026,.01],(1,0,0),chitin,f'shin_{side}')
        ridge('Narrow shin ridge',[(.115,s*.34,.42),(.025,s*.35,.28),(-.095,s*.35,.15)],
              [.026,.041,.012],[.008,.021,.004],(1,0,0),chitin,f'shin_{side}')
    for s,side in [(1,'L'),(-1,'R')]:
        # Four separate curving grasping fingers end in extended claw sickles.
        for j in range(4):
            y=s*(.633+j*.047);offset=abs(j-1.5)*.024
            bn=f'claw_{side}_{j}';bones[bn]=((.56,y,.44),(.71,y,.30),f'hand_{side}')
            M.tube('Jointed long finger',[(.55,y,.465),(.64,y,.42),(.69,y,.345+offset)],[.025,.022,.014],skin,bn,7)
            M.tube('Curving ochre sickle claw',[(.675,y,.365+offset),(.745,y,.305+offset),(.78,y,.205+offset),(.755,y,.14+offset)],[.025,.021,.013,.001],horn,bn,7)
            M.tube('Golden foot talon',[(-.0,s*(.275+j*.052),.087),(.17,s*(.275+j*.052),.085),(.235,s*(.275+j*.052),.04),(.25,s*(.275+j*.052),.008)],[.024,.028,.015,.001],horn,f'foot_{side}',7)
        M.ellipsoid('Deep angular eye socket',(.627,s*.086,1.267),(.025,.021,.016),dark,'head',12,7)
        M.ellipsoid('Tiny sunken amber red eye',(.646,s*.079,1.272),(.010,.010,.008),eye,'head',10,6)
        plate('Knife cheek plane',[(.545,s*.14,1.30),(.671,s*.085,1.219),(.703,s*.077,1.153),(.605,s*.133,1.189)],.023,skin,'head')
        plate('Heavy predatory scowl',[(.553,s*.047,1.371),(.647,s*.066,1.301),(.622,s*.115,1.288),(.518,s*.15,1.34)],.025,skin,'head')
        M.tube('Swept cranial crest',[(.4,s*.075,1.379),(.32,s*.093,1.46),(.22,s*.10,1.51)],[.04,.025,.001],horn,'head',7)
        # Sparse large spines are legible at gameplay distance, mostly backward.
        for j,(x,y,z,length) in enumerate([(-.29,.13,1.63,.26),(-.11,.24,1.59,.27),(-.02,.39,1.48,.22),(-.18,.30,1.36,.17),(-.37,.16,1.18,.18),(.01,.57,1.14,.14)]):
            bn='body' if j<5 else f'upper_{side}'
            M.tube('Large swept ochre dorsal spine',[(x,s*y,z),(x-.05,s*(y+.025),z+length*.47),(x-.14,s*(y+.045),z+length)],[.049,.030,.001],horn,bn,8)
        for j in range(3):
            M.tube('Muzzle fang',[(.689-j*.035,s*.075,1.16),(.705-j*.032,s*.072,1.065-j*.002)],[.015,.001],horn,'head',6)
    M.ellipsoid('Long open mouth fissure',(.64,0,1.12),(.095,.087,.017),dark,'head',16,6)
    for s in [-1,1]:
        for j in range(3):
            M.tube('Central jaw teeth',[(.717,s*(.015+j*.018),1.133),(.728,s*(.015+j*.018),1.104)],[.008,.001],horn,'head',5)
    bones['VFX_Claw_L']=((.72,.70,.3),(.8,.70,.3),'hand_L')
    bones['VFX_Claw_R']=((.72,-.70,.3),(.8,-.70,.3),'hand_R')
    bones['VFX_Hit']=((.24,0,1.24),(.30,0,1.24),'body')
    bones['UI_Anchor']=((0,0,1.98),(0,0,2.04),'body')
    return M.PARTS,bones


def create(kind):
    M.PARTS=[];random.seed(50830)
    return atrocity() if kind=='atrocity' else goblin(kind)


def _move(b,delta):b.location=b.bone.matrix_local.to_3x3().inverted()@Vector(delta)
def _rotate(b,axis,angle):b.rotation_euler=Quaternion(b.bone.matrix_local.to_3x3().inverted()@Vector(axis),angle).to_euler('XYZ')


def pose(kind,rig,clip,frame,end):
    """In-place 30 fps prototype motion; f30 is the single attack contact."""
    b=rig.pose.bones;f=frame;wave=math.sin(math.tau*f/end);beast=kind=='atrocity'
    if clip=='idle':
        b['body'].scale=(1+.003*wave,1+.003*wave,1+.003*wave)
        _rotate(b['head'],(0,1,0),.015*wave)
        if beast:
            for side in ['L','R']:
                for j in range(4):_rotate(b[f'claw_{side}_{j}'],(0,1,0),.025*wave)
    elif clip=='move':
        _move(b['body'],(0,0,.025*(1-math.cos(math.tau*f/end*2))))
        for s,side in [(1,'L'),(-1,'R')]:
            p=wave*s
            _move(b[f'thigh_{side}'],(.09*p,0,.035*max(0,p)))
            _rotate(b[f'thigh_{side}'],(0,1,0),-.12*max(0,p))
            _rotate(b[f'shin_{side}'],(0,1,0),.16*max(0,p))
            if beast:_rotate(b[f'upper_{side}'],(0,1,0),-.08*p)
        _rotate(b['head'],(0,1,0),-.025*wave)
    elif clip=='attack':
        # Visible preparatory pull, hold to f29, one fast committed contact at f30.
        pull=min(1,f/20) if f<30 else max(0,1-(f-30)/12)
        impact=1 if f==30 else (max(0,1-(f-30)/12) if f>30 else 0)
        if beast:
            _rotate(b['upper_R'],(0,1,0),.20*pull if f<30 else -.08*impact)
            _rotate(b['fore_R'],(0,1,0),.16*pull if f<30 else -.04*impact)
            _move(b['upper_R'],(0,0,.085*pull if f<30 else 0))
            _rotate(b['head'],(0,1,0),-.05*pull if f<30 else .08*impact)
        else:
            dx=-.07*pull if f<30 else .095*impact
            for side in ['L','R']:_move(b[f'upper_{side}'],(dx,0,-.008*impact))
            _rotate(b['head'],(0,1,0),-.035*pull if f<30 else .045*impact)
            _rotate(b['Weapon_R'],(0,1,0),-.025*pull if f<30 else .085*impact)
    elif clip=='hit':
        t=math.sin(math.pi*f/end)
        _rotate(b['body'],(0,1,0),-.075*t);_rotate(b['head'],(0,0,1),.12*t)
    elif clip=='death':
        t=min(1,f/30);ease=t*t*(3-2*t)
        # Fold inward with one stable final pose. Root stays at the floor origin.
        _move(b['body'],(-.16*ease if beast else 0,0 if beast else .08*ease,-(.87 if beast else .62)*ease))
        _rotate(b['body'],(0 if beast else .35,1,0),(1.4 if beast else 1.2)*ease)
        for s,side in [(1,'L'),(-1,'R')]:
            _rotate(b[f'thigh_{side}'],(0,1,0),(-2.5 if beast else .2)*ease)
            _rotate(b[f'shin_{side}'],(0,1,0),(1.5 if beast else -.6)*ease)
            _move(b[f'thigh_{side}'],(0,-s*(.05 if beast else .14)*ease,0))
            _rotate(b[f'upper_{side}'],(1,0,0),-s*.35*ease)
            _rotate(b[f'fore_{side}'],(0,1,0),(-2.8 if beast else -.5)*ease)
            if beast:_move(b[f'upper_{side}'],Quaternion(Vector((0,1,0)),-1.4*ease)@Vector((-.27*math.sin(math.pi*ease),-s*.065*ease,.12*math.sin(math.pi*ease))))
        _rotate(b['head'],(0,1,0),(-.6 if beast else .20)*ease)
        if beast:_move(b['head'],Quaternion(Vector((0,1,0)),-1.4*ease)@Vector((0,0,.20*ease)))
        if not beast:
            # Lay the rigid polearm longitudinally in the specified death box.
            # Explicit world pose avoids inheriting the body's sideways folding.
            bpy.context.view_layer.update()
            pb=b['Weapon_R'];rest=pb.bone.matrix_local
            zmid=1.11 if kind=='goblin_warrior' else .975
            target=Vector((.391, -.30, .758)).lerp(Vector((.758-zmid,-.30,.045)),ease)
            target.x-=.065*math.sin(math.pi*ease)
            pb.matrix=Matrix.Translation(target)@Quaternion(Vector((0,1,0)),math.pi/2*ease).to_matrix().to_4x4()@rest.to_3x3().to_4x4()
