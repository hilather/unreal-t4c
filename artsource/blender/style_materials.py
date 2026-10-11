"""Original procedural surfaces, genuine Cycles CPU bakes; compact 512px PNGs.

New styles quantize baked RGB to five bits/channel (ORM four), then losslessly
compress with PNG zlib level nine. No downloaded or generated image assets.
"""
import struct
import zlib
from pathlib import Path
import bpy
import materials as legacy

PALETTES = {
 'Church': ((.105,.065,.032,1),(.44,.30,.155,1)),
 'B2Damp': ((.024,.039,.044,1),(.125,.17,.105,1)),
 'B3Crypt': ((.22,.185,.13,1),(.62,.55,.405,1)),
 'B4Ritual': ((.012,.014,.018,1),(.085,.071,.065,1)),
}

def compact_png(path, bits=5):
    """Quantize saved eight-bit RGB/RGBA samples; preserve true 512px pixels."""
    data=Path(path).read_bytes(); offset=8; compressed=b''; header=None
    while offset<len(data):
        n=struct.unpack_from('>I',data,offset)[0]; kind=data[offset+4:offset+8]; payload=data[offset+8:offset+8+n]; offset+=n+12
        if kind==b'IHDR': header=payload
        elif kind==b'IDAT': compressed+=payload
    w,h,depth,color,*_=struct.unpack('>IIBBBBB',header)
    assert depth==8 and color in (2,6)
    channels=3 if color==2 else 4; stride=w*channels
    raw=zlib.decompress(compressed); previous=bytearray(stride); rows=[]; cursor=0
    def paeth(a,b,c):
        p=a+b-c; pa,pb,pc=abs(p-a),abs(p-b),abs(p-c)
        return a if pa<=pb and pa<=pc else b if pb<=pc else c
    for y in range(h):
        filt=raw[cursor]; cursor+=1; row=bytearray(raw[cursor:cursor+stride]); cursor+=stride
        for x in range(stride):
            a=row[x-channels] if x>=channels else 0; b=previous[x]; c=previous[x-channels] if x>=channels else 0
            predictor=(0,a,b,(a+b)//2,paeth(a,b,c))[filt]
            row[x]=(row[x]+predictor)&255
        previous=row[:]; q=bytearray(stride); levels=(1<<bits)-1
        for x,value in enumerate(row): q[x]=round(round(value*levels/255)*255/levels)
        # Sub filtering keeps grain and normal gradients small for zlib.
        rows.append(bytes([1])+bytes((q[x]-(q[x-channels] if x>=channels else 0))&255 for x in range(stride)))
    def chunk(kind,payload): return struct.pack('>I',len(payload))+kind+payload+struct.pack('>I',zlib.crc32(kind+payload)&0xffffffff)
    Path(path).write_bytes(b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',header)+chunk(b'IDAT',zlib.compress(b''.join(rows),9))+chunk(b'IEND',b''))

def bake_style_materials(style,output,resolution=512):
    original=legacy._procedural
    def procedural(name):
        mat,color,roughness,shader,out=original(name)
        if name=='stone':
            ramp=next(n for n in mat.node_tree.nodes if n.type=='VALTORGB')
            low,high=PALETTES[style]
            for stop in ramp.color_ramp.elements:
                t=max(0,min(1,(stop.position-.2)/.6))
                stop.color=tuple(a+(b-a)*t for a,b in zip(low,high))
            if style=='B2Damp':
                for node in mat.node_tree.nodes:
                    if node.type=='MATH' and node.operation=='ADD' and abs(node.inputs[1].default_value-.79)<.001: node.inputs[1].default_value=.35
        return mat,color,roughness,shader,out
    legacy._procedural=procedural
    try: result=legacy.bake_materials(output,resolution)
    finally: legacy._procedural=original
    # Re-load quantized originals so exporter uses exactly the shared PNG bytes.
    for path in (Path(output)/'textures').glob('*.png'):
        compact_png(path,4 if '_orm' in path.name else 5)
    for image in bpy.data.images:
        if image.filepath_raw and Path(image.filepath_raw).parent==Path(output).resolve()/'textures': image.reload()
    if style=='Church':
        old_samples=bpy.context.scene.cycles.samples
        bpy.context.scene.cycles.samples=1
        # Two original albedo bakes share existing normal/ORM maps: no redundant
        # 512px normals, while aged plaster and cloth have distinct surface color.
        bpy.ops.object.select_all(action='DESELECT')
        bpy.ops.mesh.primitive_plane_add(size=2)
        plane=bpy.context.object
        for name,source,colors in (
            ('plaster','stone',((.32,.28,.19,1),(.72,.66,.50,1))),
            ('cloth','timber',((.065,.009,.012,1),(.24,.043,.047,1)))):
            mat,color,rough,shader,out=original(source)
            ramp=next(n for n in mat.node_tree.nodes if n.type=='VALTORGB')
            for stop in ramp.color_ramp.elements:
                t=max(0,min(1,(stop.position-.2)/.6))
                stop.color=tuple(a+(b-a)*t for a,b in zip(*colors))
            tree=mat.node_tree
            if name=='cloth':
                # Periodic crossed threads modulate the original mottled dye.
                uv=legacy._node(tree,'ShaderNodeTexCoord')
                split=legacy._node(tree,'ShaderNodeSeparateXYZ')
                tree.links.new(uv.outputs['UV'],split.inputs[0])
                import math
                warp=legacy._math(tree,'SINE',legacy._math(tree,'MULTIPLY',split.outputs['X'],math.tau*96))
                weft=legacy._math(tree,'SINE',legacy._math(tree,'MULTIPLY',split.outputs['Y'],math.tau*96))
                weave=legacy._math(tree,'ADD',legacy._math(tree,'MULTIPLY',legacy._math(tree,'MULTIPLY',warp,weft),.5),.5)
                tone=ramp.inputs[0].links[0].from_socket
                blended=legacy._math(tree,'ADD',legacy._math(tree,'MULTIPLY',tone,.78),legacy._math(tree,'MULTIPLY',weave,.22))
                tree.links.new(blended,ramp.inputs[0])
            target=legacy._node(tree,'ShaderNodeTexImage')
            image=legacy._image(name+'_basecolor',resolution,Path(output).resolve()/'textures')
            target.image=image; tree.nodes.active=target
            emit=legacy._node(tree,'ShaderNodeEmission'); tree.links.new(color,emit.inputs['Color']); tree.links.new(emit.outputs[0],out.inputs['Surface'])
            plane.data.materials.clear(); plane.data.materials.append(mat)
            bpy.ops.object.bake(type='EMIT',margin=0,use_clear=True)
            image.save(); compact_png(image.filepath_raw,5); image.reload()
            source_nodes=[n for n in result[source].node_tree.nodes if n.type=='TEX_IMAGE']
            normal=next(n.image for n in source_nodes if '_normal' in n.image.name)
            orm=next(n.image for n in source_nodes if '_orm' in n.image.name)
            result[name]=legacy._final(name,[image,normal,orm])
            bpy.data.materials.remove(mat)
        bpy.data.objects.remove(plane,do_unlink=True)
        bpy.context.scene.cycles.samples=old_samples
    else:
        result['plaster']=result['stone']; result['cloth']=result['timber']
    mat=bpy.data.materials.new('oxide'); mat.use_nodes=True
    shader=mat.node_tree.nodes.get('Principled BSDF')
    shader.inputs['Base Color'].default_value=(.14,.033,.019,1)
    shader.inputs['Roughness'].default_value=.82
    result['oxide']=mat
    return result
