"""Validate exported GLB bytes independently of Blender; stdlib only."""
import argparse
import hashlib
import re
import json
import math
import struct
import zlib
from pathlib import Path


def png_red(data, decode=True):
    assert data[:8] == b'\x89PNG\r\n\x1a\n', 'embedded image is not PNG'
    pos, compressed = 8, bytearray()
    while pos < len(data):
        size = struct.unpack_from('>I', data, pos)[0]
        tag, payload = data[pos+4:pos+8], data[pos+8:pos+8+size]
        crc = struct.unpack_from('>I', data, pos+8+size)[0]
        assert zlib.crc32(tag + payload) & 0xffffffff == crc, 'PNG CRC'
        if tag == b'IHDR':
            w, h, bits, color, compression, filtering, interlace = struct.unpack('>IIBBBBB', payload)
        if tag == b'IDAT':
            compressed.extend(payload)
        pos += 12 + size
        if tag == b'IEND':
            break
    assert bits == 8 and color in (2, 6) and interlace == 0, 'expected 8-bit RGB(A) PNG'
    channels = 3 if color == 2 else 4
    raw = zlib.decompress(compressed)
    stride = w * channels
    assert len(raw) == h * (stride + 1)
    assert all(raw[y*(stride+1)] <= 4 for y in range(h)), 'invalid PNG filter'
    if not decode:
        return (w, h), None
    # PNG predictors connect bytes in the same channel. Decode red alone.
    previous = bytearray(w)
    red = bytearray()
    for y in range(h):
        start = y * (stride + 1)
        mode = raw[start]
        assert mode <= 4
        row = bytearray(raw[start+1:start+1+stride:channels])
        if mode:
            for x in range(w):
                left = row[x-1] if x else 0
                above = previous[x]
                corner = previous[x-1] if x else 0
                if mode == 1:
                    prediction = left
                elif mode == 2:
                    prediction = above
                elif mode == 3:
                    prediction = (left + above) // 2
                else:
                    p = left + above - corner
                    pa, pb, pc = abs(p-left), abs(p-above), abs(p-corner)
                    prediction = left if pa <= pb and pa <= pc else above if pb <= pc else corner
                row[x] = (row[x] + prediction) & 255
        red.extend(row)
        previous = row
    return (w, h), red


def read(path):
    blob = path.read_bytes()
    magic, version, total = struct.unpack_from('<III', blob)
    assert magic == 0x46546c67 and version == 2 and total == len(blob)
    chunks, pos = {}, 12
    while pos < len(blob):
        length, tag = struct.unpack_from('<II', blob, pos)
        assert length % 4 == 0 and pos+8+length <= len(blob)
        assert tag not in chunks
        chunks[tag] = blob[pos+8:pos+8+length]
        pos += 8+length
    d = json.loads(chunks[0x4e4f534a])
    binary = chunks[0x004e4942]
    assert len(d['buffers']) == 1 and 'uri' not in d['buffers'][0]

    def view(index):
        v = d['bufferViews'][index]
        assert v.get('buffer', 0) == 0
        start = v.get('byteOffset', 0)
        assert start >= 0 and start + v['byteLength'] <= len(binary)
        return binary[start:start+v['byteLength']]

    def accessor(index):
        a = d['accessors'][index]
        assert 'sparse' not in a, 'sparse accessors unsupported'
        v = d['bufferViews'][a['bufferView']]
        data = view(a['bufferView'])
        count = {'SCALAR':1, 'VEC2':2, 'VEC3':3, 'VEC4':4, 'MAT4':16}[a['type']]
        fmt, size = {5126:('f',4),5125:('I',4),5123:('H',2),5122:('h',2),5121:('B',1),5120:('b',1)}[a['componentType']]
        offset, stride = a.get('byteOffset',0), v.get('byteStride',count*size)
        assert stride >= count*size and offset+(a['count']-1)*stride+count*size <= len(data)
        rows = [struct.unpack_from('<'+fmt*count,data,offset+i*stride) for i in range(a['count'])]
        if a.get('normalized'):
            divisor = {5121:255,5123:65535,5120:127,5122:32767}[a['componentType']]
            rows = [tuple(max(-1,x/divisor) for x in row) for row in rows]
        assert all(math.isfinite(x) for row in rows for x in row), 'nonfinite accessor'
        return rows

    return d, view, accessor, len(blob)

HERE = Path(__file__).resolve().parent
PARENTS = {'root': None, 'pelvis': 'root', 'spine': 'pelvis', 'chest': 'spine', 'neck': 'chest', 'head': 'neck'}
for side in ('L', 'R'):
    for bone, parent in [('thigh','pelvis'),('shin','thigh.'+side),('foot','shin.'+side),('upper_arm','chest'),('forearm','upper_arm.'+side),('hand','forearm.'+side)]:
        PARENTS[bone+'.'+side] = parent
PARENTS.update({'Socket.Weapon.R':'hand.R','Socket.Bow.L':'hand.L','Socket.Arrow':'hand.R','Socket.Quiver.Back':'chest'})


def roster_ids():
    directory = HERE.parents[1] / 'Source/LighthavenEditor/Commandlets'
    return sorted(set(re.findall(r'TEXT\("(NPC\.[A-Za-z]+)"\)', '\n'.join(p.read_text() for p in directory.glob('LHGenerate*Commandlet.cpp')))))


def same(a, b, rotation=False, tolerance=1e-5):
    return min(max(abs(x-y) for x,y in zip(a,b)), max(abs(x+y) for x,y in zip(a,b)) if rotation else math.inf) < tolerance


def validate(path, triangle_cap=25000, row=None):
    d, view, accessor, size = read(path)
    assert len(d['skins']) == 1, 'expected one shared skeleton'
    joints = d['skins'][0]['joints']
    assert len(joints) == len(set(joints)) == 22
    assert all(0 <= j < len(d['nodes']) for j in joints)
    names = {d['nodes'][j]['name']: j for j in joints}
    assert set(names) == set(PARENTS), 'player bone names differ'
    parent_indices = {child: i for i,n in enumerate(d['nodes']) for child in n.get('children', [])}
    for name, parent in PARENTS.items():
        if parent is not None:
            assert parent_indices.get(names[name]) == names[parent], ('bone parent',name)
        else:
            assert parent_indices.get(names[name]) not in joints
    stature_scale = None
    if row is not None:
        scene_roots=d['scenes'][d.get('scene',0)]['nodes']
        assert len(scene_roots)==1, 'one stature parent must own the scene'
        stature=scene_roots[0]
        stature_node=d['nodes'][stature]
        assert stature_node.get('name')==row['id']+'.Stature', 'missing named scene stature parent'
        assert 'matrix' not in stature_node and 'rotation' not in stature_node
        assert same(stature_node.get('translation',[0,0,0]),[0,0,0])
        stature_scale=stature_node.get('scale',[1,1,1])
        assert same(stature_scale,[row['scale']]*3), 'scene parent stature differs from roster'
        def ancestry(index):
            chain=[]
            while True:
                assert index not in chain, 'cyclic node hierarchy'
                chain.append(index)
                if index not in parent_indices:
                    return chain
                index=parent_indices[index]
        root_chain=ancestry(names['root'])
        assert root_chain[-1]==stature, 'floor root not owned by stature parent'
        for index in root_chain[:-1]:
            node=d['nodes'][index]
            assert 'matrix' not in node and same(node.get('scale',[1,1,1]),[1,1,1],tolerance=1e-4), 'stature applied twice in root hierarchy'
            assert same(node.get('translation',[0,0,0]),[0,0,0]), 'floor root ancestor translated'
        for index,node in enumerate(d['nodes']):
            if 'mesh' in node:
                chain=ancestry(index)
                assert chain[-1]==stature, 'mesh not owned by stature parent'
                assert chain[1:]==root_chain[1:], 'meshes and skeleton have different ancestry'
                for ancestor in chain[:-1]:
                    part=d['nodes'][ancestor]
                    assert 'matrix' not in part and same(part.get('scale',[1,1,1]),[1,1,1],tolerance=1e-4), 'stature applied twice in mesh hierarchy'
    assert len(accessor(d['skins'][0]['inverseBindMatrices'])) == 22
    triangles, geometry = 0, {}
    mesh_nodes = [n for n in d['nodes'] if 'mesh' in n]
    assert len(mesh_nodes) >= 3, 'expected modular appearance meshes'
    assert all(n.get('skin') == 0 for n in mesh_nodes), 'every module must be skinned'
    for i, mesh in enumerate(d['meshes']):
        payload = []
        assert any(n['mesh'] == i for n in mesh_nodes), 'unused mesh'
        for p in mesh['primitives']:
            assert p.get('mode',4) == 4
            attrs = p['attributes']
            assert {'POSITION','NORMAL','JOINTS_0','WEIGHTS_0'} <= attrs.keys()
            values = {k:accessor(v) for k,v in attrs.items()}
            count = len(values['POSITION'])
            assert count > 0 and all(len(v) == count for v in values.values())
            indices = accessor(p['indices'])
            assert len(indices)%3 == 0 and all(0 <= x[0] < count for x in indices)
            triangles += len(indices)//3
            assert all(min(w)>=0 and abs(sum(w)-1)<.001 for w in values['WEIGHTS_0'])
            assert all(all(int(j)==j and 0<=j<22 for j in row) for row in values['JOINTS_0'])
            assert all(abs(sum(x*x for x in row)-1)<.02 for row in values['NORMAL'])
            if 'TEXCOORD_0' in values:
                assert all(0 <= x <= 1 for uv in values['TEXCOORD_0'] for x in uv), 'UV outside textile tile'
            if 'TANGENT' in values:
                assert all(abs(sum(x*x for x in t[:3])-1)<.02 and abs(abs(t[3])-1)<.001 for t in values['TANGENT'])
                assert all(abs(sum(a*b for a,b in zip(n,t)))<.02 for n,t in zip(values['NORMAL'],values['TANGENT']))
            material = d['materials'][p['material']]
            if 'baseColorTexture' in material.get('pbrMetallicRoughness',{}):
                assert 'TEXCOORD_0' in attrs
            payload.append({'attributes':values,'indices':indices})
        geometry[mesh.get('name',str(i))] = hashlib.sha256(json.dumps(payload,sort_keys=True,separators=(',',':')).encode()).hexdigest()
    assert 0 < triangles <= triangle_cap, ('prototype triangle cap',triangles)
    durations = {}
    assert len(d['animations']) == 2 and {a['name'] for a in d['animations']} == {'idle','talk'}
    for action in d['animations']:
        moving = False
        for channel in action['channels']:
            sampler = action['samplers'][channel['sampler']]
            assert sampler.get('interpolation','LINEAR') == 'LINEAR'
            times = [v[0] for v in accessor(sampler['input'])]
            values = accessor(sampler['output'])
            assert len(times)==len(values)==61 and all(abs(t-i/30)<1e-5 for i,t in enumerate(times)), 'expected 30fps frames 0..60'
            rotation = channel['target']['path'] == 'rotation'
            if rotation:
                assert all(abs(sum(x*x for x in row)-1)<.001 for row in values)
            varies = any(not same(values[0],v,rotation) for v in values)
            if channel['target']['node'] == names['root']:
                assert not varies, 'root moves'
                if channel['target']['path']=='translation':
                    assert max(abs(x) for x in values[0])<1e-6, 'root above floor'
            assert same(values[0],values[-1],rotation), 'loop discontinuity'
            moving |= varies
        assert moving, 'static animation'
        durations[action['name']] = 2
    images = [view(im['bufferView']) for im in d['images']]
    assert len(images)==2 and all(png_red(im,decode=False)[0]==(512,512) for im in images)
    texture_pairs = set()
    for material in d['materials']:
        pbr = material['pbrMetallicRoughness']
        factor = pbr.get('baseColorFactor',[1]*4)
        if row is not None:
            slot=material.get('name','').rsplit('.',1)[-1]
            expected_colors={'Cloth':row['cloth'],'Accent':row['accent'],'Trim':row['trim'],
                             'Skin':row['skin'],'Hair':row['hair'],'Lip':row['lip'],
                             'Leather':'4D3020','Edge':'785333','Stitch':'A68C61','Metal':'92734A',
                             'Iron':'646C6C','Wood':'67452E','Paper':'D6C69B','Crease':'36251E',
                             'Eye':'B0A08A','Iris':'504031','Pupil':'120F0D','Pants':'4E4836'}
            assert slot in expected_colors, ('unknown material slot',material.get('name'))
            expected=[c/3294.6 if c<=10 else ((c/255+.055)/1.055)**2.4 for c in bytes.fromhex(expected_colors[slot])]+[1]
            assert same(factor,expected), ('material tint differs from roster',path.name,slot,factor,expected)
        assert len(factor)==4 and all(math.isfinite(x) and 0<=x<=1 for x in factor)
        assert all(math.isfinite(pbr.get(k,1)) and 0<=pbr.get(k,1)<=1 for k in ('roughnessFactor','metallicFactor'))
        assert 'occlusionTexture' not in material and 'metallicRoughnessTexture' not in pbr
        if 'baseColorTexture' in pbr:
            assert 'normalTexture' in material
            pair=[]
            for ref in (pbr['baseColorTexture'],material['normalTexture']):
                assert ref.get('texCoord',0)==0
                source=d['textures'][ref['index']]['source']
                assert 0<=source<len(images)
                pair.append(source)
            assert pair[0]!=pair[1]
            texture_pairs.add(tuple(pair))
        else:
            assert 'normalTexture' not in material
    assert len(texture_pairs)==1, 'cloth must share base and normal textures'
    return {'id':path.stem,'bytes':size,'triangles':triangles,'scene_stature_scale':stature_scale,'bone_names':sorted(names),'animation_seconds':durations,'texture_sha256':[hashlib.sha256(im).hexdigest() for im in images],'geometry_sha256':geometry}


def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--output',type=Path,default=HERE/'output')
    args=parser.parse_args()
    manifest=json.loads((args.output/'manifest.json').read_text())
    entries=manifest['npcs']
    assert sorted(e['id'] for e in entries)==roster_ids(), 'manifest does not match generator roster'
    rows={r['id']:r for r in json.loads((HERE/'roster.json').read_text())['npcs']}
    results=[validate(args.output/e['file'],row=rows[e['id']]) for e in entries]
    kits=manifest['kits']
    assert sorted(k['body'] for k in kits)==['A','B']
    kit_results=[validate(args.output/k['file'],triangle_cap=60000) for k in kits]
    kit_by_body={k['body']:r for k,r in zip(kits,kit_results)}
    for entry,result in zip(entries,results):
        exported=result['geometry_sha256']
        assert set(exported)==set(entry['mesh_keys']), ('exported mesh keys differ',entry['id'],list(exported))
        shared=kit_by_body[entry['body']]['geometry_sha256']
        for key,digest in exported.items():
            assert shared.get(key)==digest, ('kit geometry differs from NPC',entry['id'],key,digest,shared.get(key))
    for entry,result in zip(kits,kit_results):
        assert set(result['geometry_sha256'])==set(entry['mesh_keys'])
        assert entry['triangles']==result['triangles'] and entry['glb_bytes']==result['bytes']
    assert len({tuple(r['texture_sha256']) for r in results+kit_results})==1, 'kit cloth images differ'
    assert sum(r['bytes'] for r in results)<30*1024*1024
    assert len({tuple(r['texture_sha256']) for r in results})==1, 'cloth images differ across NPCs'
    for entry,result in zip(entries,results):
        assert entry['id']==result['id'] and entry['triangles']==result['triangles'] and entry['glb_bytes']==result['bytes']
    report={'success':True,'total_glb_bytes':sum(r['bytes'] for r in results),'assets':results,'kits':kit_results,'limits_note':'Prototype art validation limits; no gameplay rules.'}
    (args.output/'glb-validation.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))

if __name__=='__main__':
    main()
