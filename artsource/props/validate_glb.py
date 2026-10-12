"""Independently validate the static prop delivery using only Python's stdlib."""
import argparse
import hashlib
import json
import math
import struct
import zlib
from pathlib import Path


def require(condition, message):
    if not condition:
        raise ValueError(message)


def png(data, icon=True):
    require(data[:8] == b'\x89PNG\r\n\x1a\n', 'PNG signature')
    pos, packed, header, ended = 8, bytearray(), None, False
    while pos < len(data):
        require(pos + 12 <= len(data), 'truncated PNG chunk')
        size = struct.unpack_from('>I', data, pos)[0]
        require(pos + 12 + size <= len(data), 'PNG chunk range')
        tag, payload = data[pos+4:pos+8], data[pos+8:pos+8+size]
        require(zlib.crc32(tag + payload) & 0xffffffff == struct.unpack_from('>I', data, pos+8+size)[0], 'PNG CRC')
        if header is None:
            require(tag == b'IHDR' and size == 13, 'PNG IHDR')
            header = struct.unpack('>IIBBBBB', payload)
        elif tag == b'IHDR':
            raise ValueError('duplicate PNG IHDR')
        if tag == b'IDAT':
            packed.extend(payload)
        pos += size + 12
        if tag == b'IEND':
            require(size == 0 and pos == len(data), 'PNG IEND/trailing bytes')
            ended = True
            break
    require(ended and header is not None, 'PNG end/header')
    w,h,bits,color,compression,filtering,interlace = header
    require((w,h) == ((128,128) if icon else (256,256)) and bits == 8 and color in ((6,) if icon else (2,6)) and compression == filtering == interlace == 0, 'PNG dimensions/format')
    channels = 4 if color == 6 else 3
    raw = zlib.decompress(packed)
    stride = w * channels
    require(len(raw) == h * (stride+1), 'PNG decoded size')
    previous, alpha = bytearray(stride), []
    for y in range(h):
        start = y*(stride+1)
        mode, row = raw[start], bytearray(raw[start+1:start+1+stride])
        require(mode <= 4, 'PNG filter')
        for x in range(stride):
            left, above, corner = row[x-channels] if x >= channels else 0, previous[x], previous[x-channels] if x >= channels else 0
            p = left + above - corner
            distances = (abs(p-left), abs(p-above), abs(p-corner))
            prediction = (0, left, above, (left+above)//2, (left, above, corner)[distances.index(min(distances))])[mode]
            row[x] = (row[x]+prediction) & 255
        if channels == 4:
            alpha.extend(row[3::4])
        previous = row
    if icon:
        require(0 in alpha and max(alpha) > 0, 'icon needs transparent background and visible pixels')


def validate_glb(path):
    blob = path.read_bytes()
    require(len(blob) >= 20, 'GLB too short')
    require(struct.unpack_from('<III', blob) == (0x46546c67, 2, len(blob)), 'GLB header')
    chunks, pos = [], 12
    while pos < len(blob):
        require(pos+8 <= len(blob), 'GLB chunk header range')
        size, tag = struct.unpack_from('<II', blob, pos)
        require(size % 4 == 0 and pos+8+size <= len(blob), 'GLB chunk range/alignment')
        chunks.append((tag, blob[pos+8:pos+8+size]))
        pos += 8+size
    require([tag for tag, _ in chunks] == [0x4e4f534a, 0x004e4942], 'expected JSON and BIN chunks')
    d, binary = json.loads(chunks[0][1]), chunks[1][1]
    require(d.get('asset', {}).get('version') == '2.0', 'glTF version')
    require(not d.get('animations') and not d.get('skins'), 'static props cannot have animations/skins')
    require(len(d.get('buffers', [])) == 1 and 'uri' not in d['buffers'][0], 'embedded buffer required')
    declared = d['buffers'][0]['byteLength']
    require(0 <= len(binary)-declared <= 3, 'buffer length/padding')
    for im in d.get('images', []):
        require(im.get('uri') in ('textures/props_base.png','textures/props_normal.png','textures/props_orm.png') and 'bufferView' not in im, 'unexpected image resource')
    for texture in d.get('textures', []):
        require(0 <= texture.get('source', -1) < len(d.get('images', [])), 'texture source index')
    for material in d.get('materials', []):
        name = material.get('name', '')
        if name.startswith('FX.'):
            require(material.get('alphaMode', 'OPAQUE') in ('OPAQUE','BLEND'), 'FX alpha mode')
            def numeric_factors(value):
                if isinstance(value, dict):
                    require(not any('Texture' in key for key in value), 'FX materials cannot reference textures')
                    for child in value.values():
                        numeric_factors(child)
                elif isinstance(value, list):
                    require(all(isinstance(x, (int,float)) and math.isfinite(x) for x in value), 'FX numeric factors')
                elif isinstance(value, (int,float)):
                    require(math.isfinite(value), 'FX nonfinite factor')
            numeric_factors(material)
            require(len(material.get('emissiveFactor', [0,0,0])) == 3, 'FX emissive factor')
            continue
        require(name == 'Props.WornAtlas', 'unexpected ordinary surface material')
        require(material.get('alphaMode', 'OPAQUE') == 'OPAQUE', 'opaque atlas material required')
        pbr = material['pbrMetallicRoughness']
        references = [(pbr['baseColorTexture'], 'textures/props_base.png'),
                      (material['normalTexture'], 'textures/props_normal.png'),
                      (pbr['metallicRoughnessTexture'], 'textures/props_orm.png'),
                      (material['occlusionTexture'], 'textures/props_orm.png')]
        for reference, uri in references:
            require(reference.get('texCoord', 0) == 0, 'atlas must use UV0')
            require(0 <= reference['index'] < len(d.get('textures', [])), 'material texture index')
            source = d['textures'][reference['index']]['source']
            require(d['images'][source]['uri'] == uri, 'material shared atlas path')
    views = d.get('bufferViews', [])
    for v in views:
        require(v.get('buffer', 0) == 0 and v.get('byteOffset', 0) >= 0 and v['byteLength'] >= 0 and v.get('byteOffset', 0)+v['byteLength'] <= declared, 'bufferView range')

    def accessor(index):
        require(isinstance(index, int) and 0 <= index < len(d.get('accessors', [])), 'accessor index')
        a = d['accessors'][index]
        require('sparse' not in a and 'bufferView' in a, 'dense accessor required')
        require(0 <= a['bufferView'] < len(views), 'accessor bufferView')
        v = views[a['bufferView']]
        components = {'SCALAR':1, 'VEC2':2, 'VEC3':3, 'VEC4':4, 'MAT2':4, 'MAT3':9, 'MAT4':16}[a['type']]
        fmt, width = {5120:('b',1),5121:('B',1),5122:('h',2),5123:('H',2),5125:('I',4),5126:('f',4)}[a['componentType']]
        offset, count = a.get('byteOffset', 0), a['count']
        stride = v.get('byteStride', components*width)
        require(count > 0 and offset >= 0 and offset % width == 0 and stride >= components*width and stride % width == 0, 'accessor layout')
        require(offset+(count-1)*stride+components*width <= v['byteLength'], 'accessor range')
        rows = [struct.unpack_from('<'+fmt*components, binary, v.get('byteOffset', 0)+offset+i*stride) for i in range(count)]
        require(all(math.isfinite(x) for row in rows for x in row), 'nonfinite accessor')
        return rows

    for i in range(len(d.get('accessors', []))):
        accessor(i)
    nodes = d.get('nodes', [])
    mesh_nodes = []
    for n in nodes:
        require('skin' not in n, 'node skin')
        require(all(0 <= c < len(nodes) for c in n.get('children', [])), 'child node index')
        for key, expected in [('translation',[0,0,0]), ('rotation',[0,0,0,1]), ('scale',[1,1,1]), ('matrix',[1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1])]:
            actual = n.get(key, expected)
            require(len(actual) == len(expected) and all(abs(a-b) < 1e-6 for a,b in zip(actual, expected)), 'nonidentity node transform')
        if 'mesh' in n:
            require(0 <= n['mesh'] < len(d.get('meshes', [])), 'node mesh index')
            mesh_nodes.append(n['mesh'])
    require(sorted(mesh_nodes) == list(range(len(d.get('meshes', [])))) and mesh_nodes, 'every mesh must have one node')
    triangles, positions = 0, []
    for mesh in d['meshes']:
        for p in mesh['primitives']:
            require(p.get('mode', 4) == 4 and 'indices' in p, 'indexed triangles required')
            attrs = p['attributes']
            require({'POSITION', 'NORMAL', 'TEXCOORD_0'} <= attrs.keys(), 'positions, normals and UV0 required')
            require(d['accessors'][attrs['POSITION']]['type'] == 'VEC3' and d['accessors'][attrs['NORMAL']]['type'] == 'VEC3', 'position/normal shape')
            require(all(d['accessors'][attrs[key]]['componentType'] == 5126 for key in ('POSITION','NORMAL')), 'floating point position/normal required')
            values = {name:accessor(i) for name,i in attrs.items()}
            verts, normals = values['POSITION'], values['NORMAL']
            require(d['accessors'][attrs['TEXCOORD_0']]['type'] == 'VEC2' and all(0 <= x <= 1 for row in values['TEXCOORD_0'] for x in row), 'atlas UV0 range/shape')
            require(all(len(v) == len(verts) for v in values.values()), 'attribute count')
            require(all(abs(sum(x*x for x in n)-1) < .02 for n in normals), 'normalized normals')
            ia = d['accessors'][p['indices']]
            require(ia['type'] == 'SCALAR' and ia['componentType'] in (5121,5123,5125) and not ia.get('normalized'), 'index accessor type')
            indices = [v[0] for v in accessor(p['indices'])]
            require(len(indices)%3 == 0 and all(0 <= i < len(verts) for i in indices), 'triangle indices')
            for i in range(0, len(indices), 3):
                a,b,c = [verts[j] for j in indices[i:i+3]]
                u,v = [b[k]-a[k] for k in range(3)], [c[k]-a[k] for k in range(3)]
                cross = [u[1]*v[2]-u[2]*v[1], u[2]*v[0]-u[0]*v[2], u[0]*v[1]-u[1]*v[0]]
                require(sum(x*x for x in cross) > 1e-20, 'degenerate triangle')
            require(0 <= p.get('material', -1) < len(d.get('materials', [])), 'primitive material')
            triangles += len(indices)//3
            positions.extend((x,-z,y) for x,y,z in verts)
    bounds = {'min':[min(v[a] for v in positions) for a in range(3)], 'max':[max(v[a] for v in positions) for a in range(3)]}
    return dict(triangles=triangles, bounds_m=bounds, size_m=[bounds['max'][a]-bounds['min'][a] for a in range(3)], nodes=[n.get('name','') for n in nodes], materials=[m.get('name','') for m in d.get('materials', [])])


def validate(output, compare=None, meshes_only=False):
    manifest = json.loads((output/'manifest.json').read_text())
    assets, icons = manifest['assets'], [] if meshes_only else manifest['icons']
    expected = {'Item.'+x for x in 'RustedDirk AshwoodFlatbow WoodenArrows Torch PotionOfMana ClothVest ClothPants LightHeal DecayingBatWings GoblinLeatherArmor IronRing GoblinBlade IronKey FlowingBlackRobe'.split()}
    expected |= {'Prop.'+x for x in 'LootSack CorpseLootBadge ChestClosed ChestOpen'.split()}
    expected |= {'FX.'+x for x in 'FireDart FireImpact StoneShard StoneShatter DustDevil HealLight Light MeleeSpark SlashArc ArrowFlight ImpactPuff'.split()}
    expected_icons = {x for x in expected if x.startswith('Item.')}
    expected_icons |= {'Spell.'+x for x in 'Light FireDart HealLight StoneShard DustDevil'.split()}
    expected_icons |= {'Attack.Melee.Basic','Attack.Ranged.Bow'}
    require(len(assets) == len(expected) and {a['id'] for a in assets} == expected, 'expected 29 prop ids')
    if not meshes_only:
        require(len(icons) == len(expected_icons) and {a['id'] for a in icons} == expected_icons, 'expected 21 icon ids')
    textures = manifest['textures']
    require(len(textures) == 3 and {t['file'] for t in textures} == {'textures/props_base.png','textures/props_normal.png','textures/props_orm.png'}, 'shared texture inventory')
    files, total = [], 0
    for entries, suffix in [(assets,'.glb'), (icons,'.png'), (textures,'.png')]:
        for entry in entries:
            relative = Path(entry['file'])
            require(not relative.is_absolute() and '..' not in relative.parts and relative.suffix == suffix, 'unsafe/unexpected asset path')
            path = output/relative
            data = path.read_bytes()
            require(len(data) == entry['bytes'] and hashlib.sha256(data).hexdigest() == entry['sha256'], str(relative)+' byte/hash mismatch')
            files.append(relative)
            total += len(data)
            if suffix == '.png':
                png(data, icon=entries is icons)
            else:
                measured = validate_glb(path)
                for key in ('triangles','nodes','materials'):
                    require(measured[key] == entry[key], str(relative)+' manifest '+key)
                for key in ('min','max'):
                    require(len(entry['bounds_m'][key]) == 3 and all(abs(a-b) < 1e-5 for a,b in zip(measured['bounds_m'][key], entry['bounds_m'][key])), str(relative)+' bounds '+key)
                require(len(entry['size_m']) == 3 and all(abs(a-b) < 1e-5 for a,b in zip(measured['size_m'],entry['size_m'])), str(relative)+' size')
    require(len(set(files)) == len(files), 'duplicate delivery files')
    require(total < 6_000_000, 'mesh and icon byte budget')
    inventory = [p for p in output.rglob('*') if p.is_file()]
    require(sum(p.stat().st_size for p in inventory) < 40*1024*1024, 'overall output budget')
    require(sum(p.suffix.lower() == '.png' for p in inventory) <= 30, 'PNG inventory budget')
    require({p.relative_to(output) for p in inventory if p.suffix.lower() == '.glb'} == {Path(a['file']) for a in assets}, 'unmanifested mesh')
    if compare:
        other = json.loads((compare/'manifest.json').read_text())
        other_files = {Path(e['file']) for key in ('assets','textures') for e in other[key]}
        if not meshes_only:
            other_files.update(Path(e['file']) for e in other['icons'])
        require(set(files) == other_files, 'comparison runtime inventory differs')
        for relative in files:
            require((output/relative).read_bytes() == (compare/relative).read_bytes(), 'determinism mismatch: '+str(relative))
    return dict(assets=len(assets), icons=len(icons), mesh_texture_icon_bytes=total, deterministic_comparison=bool(compare))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=Path(__file__).resolve().parent/'output')
    parser.add_argument('--compare', type=Path)
    parser.add_argument('--meshes-only', action='store_true', help='validate and compare meshes and shared textures without icons')
    args = parser.parse_args()
    try:
        print(json.dumps(validate(args.output, args.compare, args.meshes_only), indent=2))
    except (ValueError, KeyError, IndexError, struct.error, OSError, zlib.error) as exc:
        parser.exit(1, 'Validation failed: '+str(exc)+'\n')
