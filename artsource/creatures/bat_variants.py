"""W5-08e original bat variants from the authored W5-08d anatomical bat.

Shared rig and membrane topology; distinct proportion changes and baked large
pigmentation marks. All positions are prototype metres. No external image input.
"""
import math
import random
import bpy
from mathutils import Vector, Quaternion
import models as M
from bat_polish import bat

PRESETS={
    'dungeon_bat':(1.12,1.31,1.12,1.095),
    'giant_bat':(1.70,2.05,1.52,1.29),
    'undead_bat':(1.29,1.30,1.18,1.14),
}


def mathnode(nodes,links,op,a,b=None):
    node=nodes.new('ShaderNodeMath');node.operation=op
    if isinstance(a,(float,int)):node.inputs[0].default_value=a
    else:links.new(a,node.inputs[0])
    if b is not None:
        if isinstance(b,(float,int)):node.inputs[1].default_value=b
        else:links.new(b,node.inputs[1])
    return node.outputs[0]


def recolor(mat,kind,part):
    n=mat.node_tree.nodes;l=mat.node_tree.links;p=n.get('Principled BSDF')
    sx,sy,sz,z0=PRESETS[kind]
    tex=n.new('ShaderNodeAttribute');tex.attribute_name='variant_design_coordinate'
    offset=n.new('ShaderNodeVectorMath');offset.operation='SUBTRACT';offset.inputs[1].default_value=(0,0,z0-1.05*sz)
    l.new(tex.outputs['Vector'],offset.inputs[0])
    coord=n.new('ShaderNodeVectorMath');coord.operation='DIVIDE';coord.inputs[1].default_value=(sx,sy,sz);l.new(offset.outputs[0],coord.inputs[0])
    separate=n.new('ShaderNodeSeparateXYZ');l.new(coord.outputs[0],separate.inputs[0])
    x=separate.outputs['X'];y=mathnode(n,l,'ABSOLUTE',separate.outputs['Y'])
    noise=n.new('ShaderNodeTexNoise');noise.inputs['Scale'].default_value=35;noise.inputs['Detail'].default_value=3
    l.new(coord.outputs[0],noise.inputs['Vector'])
    colors={
       'dungeon_bat':{'fur':((.009,.018,.031),(.026,.045,.068)), 'membrane':((.008,.016,.030),(.027,.048,.072)), 'skin':((.022,.030,.042),(.063,.077,.097))},
       'giant_bat':{'fur':((.025,.016,.010),(.09,.051,.027)), 'membrane':((.070,.030,.010),(.28,.145,.058)), 'skin':((.076,.038,.018),(.22,.13,.055))},
       'undead_bat':{'fur':((.22,.225,.185),(.52,.51,.39)), 'membrane':((.20,.215,.18),(.52,.53,.42)), 'skin':((.28,.28,.215),(.61,.59,.45))},
    }
    family='fur' if part in ('fur','fur_tip') else 'membrane' if part=='membrane' else 'skin'
    ramp=M.ramp(n,[(.2,colors[kind][family][0]),(.8,colors[kind][family][1])]);l.new(noise.outputs['Fac'],ramp.inputs[0]);base=ramp.outputs[0]
    if family=='fur':
        # A paler underside and dark dorsal coat remain visible beneath the mark.
        zmap=n.new('ShaderNodeMapRange');zmap.inputs['From Min'].default_value=.99;zmap.inputs['From Max'].default_value=1.12;zmap.inputs['To Min'].default_value=.72;zmap.inputs['To Max'].default_value=0
        l.new(separate.outputs['Z'],zmap.inputs['Value'])
        belly=n.new('ShaderNodeMixRGB');l.new(zmap.outputs[0],belly.inputs[0]);l.new(base,belly.inputs[1]);belly.inputs[2].default_value=(*[c*1.5 for c in colors[kind]['fur'][1]],1);base=belly.outputs[0]
        grain=M.ramp(n,[(.20,(.38,.38,.38)),(.50,(1,1,1)),(.80,(1.8,1.8,1.8))])
        l.new(n.get('Noise Texture').outputs['Fac'],grain.inputs[0])
        striation=n.new('ShaderNodeMixRGB');striation.blend_type='MULTIPLY';striation.inputs[0].default_value=1
        l.new(base,striation.inputs[1]);l.new(grain.outputs[0],striation.inputs[2]);base=striation.outputs[0]
    mark=None
    if kind=='dungeon_bat' and family in ('fur','membrane'):
        # One broad connected dorsal V, centred over the shoulders, in pigmentation.
        target=mathnode(n,l,'ADD',mathnode(n,l,'MULTIPLY',y,.27),-.063)
        distance=mathnode(n,l,'ABSOLUTE',mathnode(n,l,'SUBTRACT',x,target))
        width=mathnode(n,l,'LESS_THAN',distance,.035)
        extent=mathnode(n,l,'LESS_THAN',y,.29)
        dorsal=mathnode(n,l,'GREATER_THAN',separate.outputs['Z'],1.012)
        mark=mathnode(n,l,'MULTIPLY',mathnode(n,l,'MULTIPLY',width,extent),dorsal)
    elif kind=='giant_bat' and family in ('membrane','skin'):
        # Continuous ochre leading edge, including the shoulder-to-wrist section.
        slope=mathnode(n,l,'MULTIPLY',y,-.115)
        leading=mathnode(n,l,'ADD',slope,.070)
        delta=mathnode(n,l,'SUBTRACT',leading,x)
        mark=mathnode(n,l,'LESS_THAN',delta,.055)
    if mark is not None:
        pale={'dungeon_bat':(.37,.43,.48),'giant_bat':(.43,.29,.115),'undead_bat':(.49,.50,.38)}[kind]
        variation=M.ramp(n,[(.18,tuple(v*.65 for v in pale)),(.85,pale)])
        l.new(noise.outputs['Fac'],variation.inputs[0])
        mix=n.new('ShaderNodeMixRGB');l.new(mark,mix.inputs[0]);l.new(base,mix.inputs[1]);l.new(variation.outputs[0],mix.inputs[2]);base=mix.outputs[0]
    l.new(base,p.inputs['Base Color'])
    if family=='membrane':p.inputs['Roughness'].default_value=.78


def shape(co,kind,body=False):
    sx,sy,sz,z0=PRESETS[kind];x,y,z=co
    if kind=='dungeon_bat' and abs(y)>.36:
        # Broad clipped tips retain every wing bone and a continuous membrane.
        y=math.copysign(.36+(abs(y)-.36)*.25,y)
    if kind=='giant_bat' and body:
        y*=1.20
        z=1.05+(z-1.05)*1.12
    return (x*sx,y*sy,z0+(z-1.05)*sz)



def wing_plane(co,kind):
    # W5-08f common geometry already authors a shallow open membrane plane.
    # A second tilt would turn the chord edge-on; keep socket coordinates exact.
    return tuple(co)


def create(kind):
    assert kind in PRESETS
    M.PARTS=[];random.seed({'dungeon_bat':5801,'giant_bat':5802,'undead_bat':5803}[kind])
    m={k:M.shader(kind+' '+k,k,'bat') for k in ['fur','fur_tip','skin','tail','ear','eye','claw','mouth','tooth','membrane']}
    for name in ('fur','fur_tip','skin','ear','membrane'):recolor(m[name],kind,name)
    bones={'root':((0,0,0),(0,0,.05),None)}
    bat(m,bones,undead=kind=='undead_bat')
    if kind=='undead_bat':
        # Fewer hair cards fund the exposed rib cage without triangle growth.
        for obj in list(M.PARTS):
            if obj.name.startswith('Layered shoulder and chest ruff'):
                M.PARTS.remove(obj);bpy.data.objects.remove(obj,do_unlink=True)
        cavity=M.shader('Undead dark rib cage recess','mouth','bat')
        cavity.node_tree.nodes.get('Principled BSDF').inputs['Base Color'].default_value=(.018,.025,.023,1)
        for obj in M.PARTS:
            if obj.name.startswith('Tapered furry chest'):obj.data.materials[0]=cavity
        ivory=M.shader('Undead exposed ivory ribs','tooth','bat')
        ivory.node_tree.nodes.get('Principled BSDF').inputs['Base Color'].default_value=(.63,.63,.48,1)
        for index,(x,ry,rz,zc) in enumerate([(-.075,.042,.051,1.016),(-.047,.055,.066,1.026),(-.019,.065,.075,1.033),(.01,.064,.075,1.041)]):
            points=[(x,ry*math.cos(math.pi*j/9),zc+rz*math.sin(math.pi*j/9)) for j in range(10)]
            M.tube('Exposed dorsal rib '+str(index),points,.0048,ivory,'body',5)
        M.tube('Exposed spinal ridge',[(-.085,0,1.067),(-.045,0,1.095),(.015,0,1.118)],[.005,.006,.005],ivory,'body',6)
        glow=M.shader('Undead cold eye pigment','eye','bat')
        glow.node_tree.nodes.get('Principled BSDF').inputs['Base Color'].default_value=(.12,.52,.72,1)
        glow['w5_emissive']=(.15,.50,.70)
        for obj in M.PARTS:
            if obj.name.startswith('Inset alert bat eye'):
                obj.data.materials[0]=glow
    for obj in M.PARTS:
        body=all(not group.name.startswith('wing') for group in obj.vertex_groups)
        design=obj.data.attributes.new(name='variant_design_coordinate',type='FLOAT_VECTOR',domain='POINT')
        for vertex in obj.data.vertices:
            vertex.co=shape(vertex.co,kind,body)
            design.data[vertex.index].vector=vertex.co.copy()
            if not body:vertex.co=wing_plane(vertex.co,kind)
    for name,(head,tail,parent) in list(bones.items()):
        if name=='root':continue
        body=not name.startswith('wing')
        head,tail=shape(head,kind,body),shape(tail,kind,body)
        if not body:head,tail=wing_plane(head,kind),wing_plane(tail,kind)
        bones[name]=(head,tail,parent)
    z0=PRESETS[kind][3]
    mouth=shape((.152,0,1.076),kind,True)
    bones['VFX_Bite']=(mouth,(mouth[0]+.04,mouth[1],mouth[2]),'head')
    bones['VFX_Hit']=((0,0,z0),(.04,0,z0),'body')
    bones['UI_Anchor']=((0,0,z0+.33),(0,0,z0+.37),'body')
    for s,label in [(-1,'R'),(1,'L')]:
        p=wing_plane(shape((.095,s*.395,1.125),kind),kind)
        bones['WingTip_'+label]=(p,(p[0]+.02,p[1],p[2]),'wingtip'+str(s))
    return M.PARTS,bones


def rotate(bone,axis,angle):
    local=bone.bone.matrix_local.to_3x3().inverted()@Vector(axis)
    bone.rotation_euler=Quaternion(local.normalized(),angle).to_euler('XYZ')


def translate(bone,xyz):
    bone.location=bone.bone.matrix_local.to_3x3().inverted()@Vector(xyz)


def pose(kind,rig,clip,frame,end):
    b=rig.pose.bones;phase=math.sin(math.tau*frame/end);z0=PRESETS[kind][3]
    if clip in ('idle','move'):
        translate(b['body'],(0,0,.015*phase))
        amplitude=.09 if clip=='idle' else .16
        for s in [-1,1]:
            irregular=.025*math.sin(math.tau*frame/end*2+s) if kind=='undead_bat' else 0
            rotate(b['wing'+str(s)],(1,0,0),s*(amplitude*phase+irregular))
            rotate(b['wingtip'+str(s)],(1,0,0),s*.045*math.sin(math.tau*frame/end-.7))
        if clip=='move':rotate(b['body'],(1,0,0),.018*phase)
    elif clip=='attack':
        if frame<20:a=frame/20*.17;forward=-.01*frame/20
        elif frame<30:a=.17;forward=-.01
        elif frame==30:a=-.20;forward=.032
        else:t=(frame-30)/12;a=-.20*(1-t);forward=.032*(1-t)
        translate(b['head'],(forward,0,0));rotate(b['head'],(0,1,0),.18 if frame==30 else -.04)
        rotate(b['jaw'],(0,1,0),.23 if frame==30 else .07 if frame>=20 and frame<30 else 0)
        for s in [-1,1]:rotate(b['wing'+str(s)],(1,0,0),s*a)
    elif clip=='hit':
        t=math.sin(math.pi*frame/end);rotate(b['body'],(1,0,0),.09*t);translate(b['body'],(-.022*t,0,0))
        for s in [-1,1]:rotate(b['wing'+str(s)],(1,0,0),s*.08*t)
    elif clip=='death':
        t=min(1,frame/30);t=t*t*(3-2*t)
        translate(b['body'],(0,0,-(z0-.10)*t));b['body'].scale=(1,1-.28*t,1-.30*t)
        for s in [-1,1]:
            rotate(b['wing'+str(s)],(1,0,0),s*.18*t)
            rotate(b['wingtip'+str(s)],(0,0,1),-s*.24*t)
        rotate(b['head'],(0,1,0),.18*t)
