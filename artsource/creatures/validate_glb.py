"""Validate exported GLB bytes independently of Blender; stdlib only."""
import argparse
import roster
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


def validate(path):
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

    if path.stem in ('balork','goblin','goblin_warrior'):
        nodes={node.get('name') for node in d['nodes']}
        assert path.stem+'_weapon' in nodes and path.stem+'_body' in nodes, 'equipment must remain removable'
        assert ('Weapon_Main' if path.stem=='balork' else 'Weapon_R') in nodes, 'missing weapon bone'
    assert d['skins'] and len(d['animations']) == 5
    assert {a['name'] for a in d['animations']} == {'idle','move','attack','hit','death'}
    for skin in d['skins']:
        assert len(set(skin['joints'])) == len(skin['joints'])
        assert all(0 <= joint < len(d['nodes']) for joint in skin['joints'])
        if 'inverseBindMatrices' in skin:
            assert len(accessor(skin['inverseBindMatrices'])) == len(skin['joints'])
    triangles = 0
    uv_samples = {}
    for mesh in d['meshes']:
        for primitive in mesh['primitives']:
            assert primitive.get('mode',4) == 4
            attrs = primitive['attributes']
            assert {'POSITION','NORMAL','TEXCOORD_0','JOINTS_0','WEIGHTS_0'} <= attrs.keys()
            values = {name: accessor(index) for name,index in attrs.items()}
            count = len(values['POSITION'])
            assert all(len(rows) == count for rows in values.values())
            indices = accessor(primitive['indices'])
            assert len(indices)%3 == 0 and all(0 <= row[0] < count for row in indices)
            triangles += len(indices)//3
            samples = uv_samples.setdefault(primitive['material'], [])
            uvs = values['TEXCOORD_0']
            assert all(0 <= x <= 1 for row in uvs for x in row), 'UV outside baked atlas'
            # Interior samples use exported UVs and indices, not texture background.
            barycentric = [(i/6, j/6, (6-i-j)/6)
                           for i in range(1,5) for j in range(1,6-i)]
            for start in range(0,len(indices),3):
                tri = [uvs[indices[start+k][0]] for k in range(3)]
                samples.extend(tuple(sum(weights[k]*tri[k][axis] for k in range(3))
                                     for axis in range(2)) for weights in barycentric)
            assert all(abs(sum(row)-1) < .001 and min(row) >= 0 for row in values['WEIGHTS_0'])
            assert all(abs(sum(x*x for x in row)-1) < .02 for row in values['NORMAL'])
            if 'TANGENT' in values:
                assert all(abs(sum(x*x for x in row[:3])-1) < .02 and abs(abs(row[3])-1) < .001 for row in values['TANGENT'])
                assert all(abs(sum(a*b for a,b in zip(normal,tangent))) < .02
                           for normal,tangent in zip(values['NORMAL'],values['TANGENT']))
            skin_nodes = [n for n in d['nodes'] if n.get('mesh') == d['meshes'].index(mesh) and 'skin' in n]
            assert skin_nodes, 'mesh has no skin binding'
            for node in skin_nodes:
                joint_count = len(d['skins'][node['skin']]['joints'])
                assert all(all(0 <= j < joint_count for j in row) for row in values['JOINTS_0'])
    assert 3000 <= triangles <= roster.budget(path.stem), (path.name,triangles)
    root = next(i for i,n in enumerate(d['nodes']) if n.get('name') == 'root')
    durations = {}
    def same(a,b,rotation=False):
        direct = max(abs(x-y) for x,y in zip(a,b))
        opposite = max(abs(x+y) for x,y in zip(a,b)) if rotation else float('inf')
        return min(direct,opposite) < 1e-4
    for action in d['animations']:
        moving, last = False, 0
        for channel in action['channels']:
            sampler = action['samplers'][channel['sampler']]
            assert sampler.get('interpolation','LINEAR') in ('LINEAR','STEP'), 'expected sampled animation'
            times = [row[0] for row in accessor(sampler['input'])]
            values = accessor(sampler['output'])
            assert len(times) == len(values) and len(times) >= 2
            assert abs(times[0]) < 1e-6 and all(a < b for a,b in zip(times,times[1:]))
            last = max(last,times[-1])
            rotation = channel['target']['path'] == 'rotation'
            if rotation:
                assert all(abs(sum(x*x for x in row)-1) < .001 for row in values), 'nonunit animated quaternion'
            varies = any(not same(values[0], row, rotation) for row in values)
            if channel['target']['node'] == root:
                assert not varies, (path.name,'root drift',action['name'])
            moving |= varies
            if action['name'] in ('idle','move'):
                assert same(values[0],values[-1],rotation), (path.name,action['name'],'loop discontinuity',channel['target'])
            if action['name'] == 'death':
                assert times[-1] >= 2-1e-6, 'death channel ends before hold'
                segment = max(i for i,t in enumerate(times) if t <= 1.5+1e-6)
                if abs(times[segment]-1.5) < 1e-6 or sampler.get('interpolation','LINEAR') == 'STEP':
                    boundary = values[segment]
                else:
                    a,b = values[segment:segment+2]
                    t = (1.5-times[segment])/(times[segment+1]-times[segment])
                    if rotation:
                        dot = sum(x*y for x,y in zip(a,b))
                        if dot < 0: b = tuple(-x for x in b); dot = -dot
                        theta = math.acos(min(1,max(-1,dot)))
                        if theta > 1e-6:
                            wa,wb = math.sin((1-t)*theta)/math.sin(theta),math.sin(t*theta)/math.sin(theta)
                        else: wa,wb = 1-t,t
                    else: wa,wb = 1-t,t
                    boundary = tuple(wa*x+wb*y for x,y in zip(a,b))
                held = [row for t,row in zip(times,values) if t >= 1.5-1e-6]
                assert held and all(same(boundary,row,rotation) for row in held), (path.name,'death hold',channel['target'])
        assert moving, (path.name,action['name'],'static clip')
        durations[action['name']] = last
    for name, expected in [('idle',2),('move',.8),('attack',1.4),('hit',.6),('death',2)]:
        assert abs(durations[name]-expected) < 1e-5, (path.name,durations)
    image_data = [view(im['bufferView']) for im in d['images']]
    images = [png_red(data, decode=False) for data in image_data]
    assert len(images) == 3
    allowed_dimensions={(512,512),(1024,1024)}
    if path.stem=='balork':allowed_dimensions.add((2048,2048))
    assert all(dim in allowed_dimensions for dim,_ in images)
    ao = []
    for material_index, material in enumerate(d['materials']):
        occlusion = material['occlusionTexture']
        orm = material['pbrMetallicRoughness']['metallicRoughnessTexture']
        assert occlusion['index'] == orm['index'], 'AO not bound to ORM'
        assert occlusion.get('strength',1) > 0
        (width,height), reds = png_red(image_data[d['textures'][orm['index']]['source']])
        # glTF UV/image origin is top-left; Blender exporter converts its V.
        occupied = {min(height-1,int(v*height))*width + min(width-1,int(u*width))
                    for u,v in uv_samples[material_index]}
        sampled = sorted(reds[pixel] for pixel in occupied)
        assert len(sampled) >= 100, 'insufficient occupied texels'
        low, high = sampled[len(sampled)//20], sampled[19*len(sampled)//20]
        assert max(sampled)-min(sampled) >= 8 and len(set(sampled)) >= 8, 'occupied AO neutral/constant'
        assert high-low >= 3, 'occupied AO variation confined to outliers'
        ao.append({'minimum':min(sampled),'maximum':max(sampled),
                   'distinct_values':len(set(sampled)),'occupied_texels_sampled':len(sampled),
                   'percentile_5':low,'percentile_95':high,
                   'mean':round(sum(sampled)/len(sampled),2)})
    return dict(creature=path.stem,triangles=triangles,animation_seconds=durations,
                embedded_png_dimensions=[dim for dim,_ in images],ao_red_statistics=ao,
                skin_joints=len(d['skins'][0]['joints']),bytes=len(blob),
                checked=['finite_attributes','normalized_normals_weights','joint_index_bounds',
                         'constant_root','loop_endpoints','death_hold_45_60','bound_nonconstant_ORM_AO'])


if __name__ == '__main__':
    out = Path(__file__).resolve().parent/'output'
    parser=argparse.ArgumentParser();parser.add_argument('--only',choices=roster.KINDS);parser.add_argument('--batch2',action='store_true')
    args=parser.parse_args()
    kinds=(args.only,) if args.only else roster.BATCH2 if args.batch2 else roster.PILOT
    print(json.dumps([validate(out/(kind+'.glb')) for kind in kinds],indent=2))
