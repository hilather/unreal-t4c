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
       'dungeon_bat':{'fur':((.027,.035,.033),(.10,.115,.10)), 'membrane':((.022,.027,.022),(.095,.115,.085)), 'skin':((.051,.046,.032),(.135,.12,.079))},
       'giant_bat':{'fur':((.025,.016,.010),(.09,.051,.027)), 'membrane':((.070,.030,.010),(.28,.145,.058)), 'skin':((.076,.038,.018),(.22,.13,.055))},
       'undead_bat':{'fur':((.018,.026,.026),(.075,.094,.085)), 'membrane':((.047,.061,.052),(.145,.18,.141)), 'skin':((.046,.059,.052),(.133,.163,.125))},
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
        width=mathnode(n,l,'LESS_THAN',distance,.023)
        extent=mathnode(n,l,'LESS_THAN',y,.275)
        dorsal=mathnode(n,l,'GREATER_THAN',separate.outputs['Z'],1.012)
        mark=mathnode(n,l,'MULTIPLY',mathnode(n,l,'MULTIPLY',width,extent),dorsal)
    elif kind=='giant_bat' and family in ('membrane','skin'):
        # Continuous ochre leading edge, including the shoulder-to-wrist section.
        slope=mathnode(n,l,'MULTIPLY',y,-.24)
        leading=mathnode(n,l,'ADD',slope,.066)
        delta=mathnode(n,l,'SUBTRACT',leading,x)
        mark=mathnode(n,l,'LESS_THAN',delta,.028)
    elif kind=='undead_bat' and family=='membrane':
        # Broad pale outer panels against the intact charcoal inner membrane.
        edge=n.new('ShaderNodeMapRange');edge.interpolation_type='SMOOTHSTEP'
        edge.inputs['From Min'].default_value=.18;edge.inputs['From Max'].default_value=.215
        l.new(y,edge.inputs['Value']);mark=edge.outputs[0]
    if mark is not None:
        pale={'dungeon_bat':(.43,.37,.245),'giant_bat':(.43,.29,.115),'undead_bat':(.49,.50,.38)}[kind]
        variation=M.ramp(n,[(.18,tuple(v*.65 for v in pale)),(.85,pale)])
        l.new(noise.outputs['Fac'],variation.inputs[0])
        mix=n.new('ShaderNodeMixRGB');l.new(mark,mix.inputs[0]);l.new(base,mix.inputs[1]);l.new(variation.outputs[0],mix.inputs[2]);base=mix.outputs[0]
    l.new(base,p.inputs['Base Color'])
    if family=='membrane':p.inputs['Roughness'].default_value=.78


def shape(co,kind,body=False):
    sx,sy,sz,z0=PRESETS[kind];x,y,z=co
    if kind=='dungeon_bat' and abs(y)>.335:
        # Broad clipped tips retain every wing bone and a continuous membrane.
        y=math.copysign(.335+(abs(y)-.335)*.20,y)
    if kind=='giant_bat' and body:
        y*=1.20
        z=1.05+(z-1.05)*1.12
    return (x*sx,y*sy,z0+(z-1.05)*sz)



def wing_plane(co,kind):
    # Put the inherited steep membranes into an upward-facing rest plane for
    # the fixed isometric camera. Pigmentation retains pre-rotation coordinates.
    x,y,z=co;z-=PRESETS[kind][3];c=math.cos(.50);s=math.sin(.50)
    return (c*x+s*z,y,-s*x+c*z+PRESETS[kind][3])

def create(kind):
    assert kind in PRESETS
    M.PARTS=[];random.seed({'dungeon_bat':5801,'giant_bat':5802,'undead_bat':5803}[kind])
    m={k:M.shader(kind+' '+k,k,'bat') for k in ['fur','fur_tip','skin','tail','ear','eye','claw','mouth','tooth','membrane']}
    for name in ('fur','fur_tip','skin','ear','membrane'):recolor(m[name],kind,name)
    bones={'root':((0,0,0),(0,0,.05),None)}
    bat(m,bones)
    for obj in M.PARTS:
        membrane=obj.name.startswith('Four scalloped membrane panels')
        if kind=='undead_bat' and membrane:
            # Exactly one broad additional shallow notch on each outer trailing arc.
            # This is opaque modeled topology, never transparency or a wound decal.
            for vertex in obj.data.vertices:
                idx=vertex.index
                if idx>=99:continue
                row,col=divmod(idx,11);t=row/8;u=col/10
                notch=max(0,1-abs(u-.60)/.18)*.26*t**3
                wrist=Vector((.035,math.copysign(.135,vertex.co.y),1.155))
                vertex.co=vertex.co.lerp(wrist,notch)
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
        p=wing_plane(shape((-.025,s*.387,1.225),kind),kind)
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
        amplitude=.19 if clip=='idle' else .27
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
