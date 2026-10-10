"""Validate exported B1 artifacts without Blender: python validate_env.py OUTPUT."""
import argparse
import hashlib
import json
import math
from pathlib import Path
import struct
import sys
import zlib


def require(condition, message):
    if not condition:
        raise ValueError(message)


def artifact(root, name):
    path = (root / name).resolve()
    require(path.is_relative_to(root.resolve()) and path.is_file(), f'Missing/unsafe artifact: {name}')
    return path


def verify_file(root, name, entry):
    data = artifact(root, name).read_bytes()
    require(len(data) == entry['bytes'], f'{name}: byte count mismatch')
    require(hashlib.sha256(data).hexdigest() == entry['sha256'], f'{name}: hash mismatch')
    return data


def png_size(data):
    require(data[:8] == b'\x89PNG\r\n\x1a\n', 'Invalid PNG signature')
    offset, size, compressed, ended = 8, None, bytearray(), False
    while offset < len(data):
        require(offset + 12 <= len(data), 'Truncated PNG chunk')
        length, = struct.unpack_from('>I', data, offset)
        kind = data[offset + 4:offset + 8]
        end = offset + 8 + length
        require(end + 4 <= len(data), 'Truncated PNG payload')
        payload = data[offset + 8:end]
        crc, = struct.unpack_from('>I', data, end)
        require(zlib.crc32(kind + payload) == crc, 'PNG CRC mismatch')
        if kind == b'IHDR':
            require(size is None and offset == 8 and length == 13, 'Invalid PNG IHDR')
            size = list(struct.unpack_from('>II', payload))
        if kind == b'IDAT':
            compressed.extend(payload)
        offset = end + 4
        if kind == b'IEND':
            require(length == 0 and offset == len(data), 'Invalid PNG ending')
            ended = True
            break
    require(ended and size and compressed, 'Incomplete PNG')
    require(bool(zlib.decompress(compressed)), 'Empty PNG pixels')
    return size


IDENTITY = [1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1.]
# Frozen LHVisualKit envelope contract, centimeters; independent of exporter code.
CANONICAL_BOUNDS = {
    'Wall400': [[0, -20, 0], [400, 0, 400]],
    'Floor400': [[0, 0, -20], [400, 400, 0]],
    'Arch240': [[-200, -20, 0], [200, 0, 400]],
    'Arch320': [[-200, -20, 0], [200, 0, 400]],
    'Sconce': [[-12.5, -6, -20], [12.5, 34, 26]],
    'Barrel': [[-31, -31, 0], [31, 31, 100]],
    'Crate': [[-41, -41, 0], [41, 41, 80]],
    'Table': [[-80, -45, 0], [80, 45, 80]],
    'Bench': [[-80, -22.5, 0], [80, 22.5, 45]],
    'Debris': [[-44, -23.525558, 0], [49.884814, 24.324003, 12]],
    'Stair600x120': [[0, -150, -19.611613], [603.922323, 150, 120]],
    'Stair600x120:descending': [[-3.922323, -150, -139.611613], [600, 150, 0]],
}


def multiply(a, b):
    return [sum(a[k * 4 + row] * b[col * 4 + k] for k in range(4))
            for col in range(4) for row in range(4)]


def node_matrix(node):
    if 'matrix' in node:
        require(not any(k in node for k in ('translation', 'rotation', 'scale')), 'Node mixes matrix and TRS')
        result = node['matrix']
    else:
        t, s = node.get('translation', [0, 0, 0]), node.get('scale', [1, 1, 1])
        q = node.get('rotation', [0, 0, 0, 1])
        require(len(t) == 3 and len(s) == 3 and len(q) == 4, 'Invalid node TRS')
        x, y, z, w = q
        require(abs(sum(v * v for v in q) - 1) < 1e-5, 'Nonunit node quaternion')
        result = [(1-2*y*y-2*z*z)*s[0], (2*x*y+2*z*w)*s[0], (2*x*z-2*y*w)*s[0], 0,
                  (2*x*y-2*z*w)*s[1], (1-2*x*x-2*z*z)*s[1], (2*y*z+2*x*w)*s[1], 0,
                  (2*x*z+2*y*w)*s[2], (2*y*z-2*x*w)*s[2], (1-2*x*x-2*y*y)*s[2], 0,
                  *t, 1]
    require(len(result) == 16 and all(math.isfinite(v) for v in result), 'Invalid node matrix')
    require(all(abs(result[i] - IDENTITY[i]) < 1e-6 for i in (3, 7, 11, 15)), 'Nonaffine node matrix')
    return result


def glb(data, entry):
    require(len(data) >= 28, 'Truncated GLB')
    require(struct.unpack_from('<III', data) == (0x46546C67, 2, len(data)), 'Invalid GLB header')
    chunks, offset = [], 12
    while offset < len(data):
        require(offset + 8 <= len(data), 'Truncated GLB chunk header')
        length, kind = struct.unpack_from('<II', data, offset)
        require(length % 4 == 0 and offset + 8 + length <= len(data), 'Invalid GLB chunk length')
        chunks.append((kind, data[offset + 8:offset + 8 + length]))
        offset += 8 + length
    require([c[0] for c in chunks] == [0x4E4F534A, 0x004E4942], 'Expected JSON and BIN chunks')
    doc, binary = json.loads(chunks[0][1]), chunks[1][1]
    require(doc['asset']['version'] == '2.0', 'Invalid glTF version')
    require(not doc.get('extensionsRequired') and not doc.get('animations') and not doc.get('skins'), 'Unsupported compressed/animated mesh')
    buffers = doc['buffers']
    require(len(buffers) == 1 and 'uri' not in buffers[0], 'Expected one embedded geometry buffer')
    length = buffers[0]['byteLength']
    require(0 < length <= len(binary) <= length + 3, 'Invalid buffer length')
    views = doc['bufferViews']
    for view in views:
        start, count = view.get('byteOffset', 0), view['byteLength']
        require(view['buffer'] == 0 and start >= 0 and count > 0 and start + count <= length, 'Buffer view out of bounds')
        if 'byteStride' in view:
            require(4 <= view['byteStride'] <= 252 and view['byteStride'] % 4 == 0, 'Invalid byte stride')
    formats = {5120: 'b', 5121: 'B', 5122: 'h', 5123: 'H', 5125: 'I', 5126: 'f'}
    sizes = {'SCALAR': 1, 'VEC2': 2, 'VEC3': 3, 'VEC4': 4}
    decoded = []
    for accessor in doc['accessors']:
        require('sparse' not in accessor and accessor['type'] in sizes, 'Unsupported sparse/matrix accessor')
        require(accessor['componentType'] in formats, 'Invalid accessor component type')
        require(0 <= accessor['bufferView'] < len(views), 'Accessor view index out of bounds')
        view = views[accessor['bufferView']]
        fmt = '<' + formats[accessor['componentType']] * sizes[accessor['type']]
        width, count = struct.calcsize(fmt), accessor['count']
        component_width = struct.calcsize('<' + formats[accessor['componentType']])
        start, stride = accessor.get('byteOffset', 0), view.get('byteStride', width)
        require(count > 0 and start >= 0 and stride >= width and start % component_width == 0
                and (view.get('byteOffset', 0) + start) % component_width == 0
                and start + (count - 1) * stride + width <= view['byteLength'], 'Accessor out of bounds/alignment')
        values = [struct.unpack_from(fmt, binary, view.get('byteOffset', 0) + start + i * stride) for i in range(count)]
        require(all(math.isfinite(v) for row in values for v in row), 'Nonfinite accessor values')
        for key, operation in (('min', min), ('max', max)):
            if key in accessor:
                actual = [operation(row[a] for row in values) for a in range(sizes[accessor['type']])]
                require(len(accessor[key]) == len(actual) and all(abs(a-b) <= 1e-5 * max(1, abs(a)) for a, b in zip(actual, accessor[key])), 'Accessor extrema mismatch')
        decoded.append(values)
    meshes, materials = doc['meshes'], doc.get('materials', [])
    require(len(materials) == entry['material_slots'], 'Material count mismatch')
    positions, triangles, visited, used_meshes = [], 0, set(), set()

    def visit(index, parent):
        nonlocal triangles
        require(0 <= index < len(doc['nodes']), 'Node index out of bounds')
        require(index not in visited, 'Cyclic/shared scene node')
        visited.add(index)
        node = doc['nodes'][index]
        local = node_matrix(node)
        world = multiply(parent, local)
        # Canonical export requires origin zero and no compensating object transforms.
        require(all(abs(a-b) < 1e-6 for a, b in zip(local, IDENTITY)), 'Export changed mesh origin/object transform')
        if 'mesh' in node:
            mesh_index = node['mesh']
            require(0 <= mesh_index < len(meshes), 'Mesh index out of bounds')
            require(mesh_index not in used_meshes, 'Unexpected mesh instancing')
            used_meshes.add(mesh_index)
            require('weights' not in node, 'Unexpected morph weights')
            for primitive in meshes[mesh_index]['primitives']:
                require(primitive.get('mode', 4) == 4 and not primitive.get('targets'), 'Expected ordinary triangles')
                attributes = primitive['attributes']
                require(all(0 <= a < len(decoded) for a in attributes.values()), 'Attribute accessor index out of bounds')
                pi = attributes['POSITION']
                require(doc['accessors'][pi]['type'] == 'VEC3' and doc['accessors'][pi]['componentType'] == 5126, 'Invalid positions')
                vertices = decoded[pi]
                require('NORMAL' in attributes and 'TEXCOORD_0' in attributes, 'Missing normals/UVs')
                normals = decoded[attributes['NORMAL']]
                require(doc['accessors'][attributes['NORMAL']]['type'] == 'VEC3'
                        and all(abs(sum(c*c for c in normal) - 1) < .002 for normal in normals), 'Nonunit vertex normals')
                require(all(len(decoded[a]) == len(vertices) for a in attributes.values()), 'Attribute vertex count mismatch')
                ai = primitive['indices']
                require(0 <= ai < len(decoded), 'Index accessor index out of bounds')
                acc = doc['accessors'][ai]
                require(acc['type'] == 'SCALAR' and acc['componentType'] in (5121, 5123, 5125) and not acc.get('normalized'), 'Invalid indices')
                indices = [row[0] for row in decoded[ai]]
                require(len(indices) % 3 == 0 and all(0 <= i < len(vertices) for i in indices), 'Triangle indices out of bounds')
                require(0 <= primitive['material'] < len(materials), 'Material index out of bounds')
                material = materials[primitive['material']]
                if 'baseColorTexture' in material.get('pbrMetallicRoughness', {}):
                    require('COLOR_0' in attributes, 'Textured surface missing vertex tint COLOR_0')
                triangles += len(indices) // 3
                for point in vertices:
                    transformed = [sum(world[col * 4 + row] * point[col] for col in range(3)) + world[12 + row] for row in range(3)]
                    positions.append([transformed[0] * 100, -transformed[2] * 100, transformed[1] * 100])
        for child in node.get('children', []):
            visit(child, world)

    require(len(doc['scenes']) == 1, 'Unexpected extra scenes')
    require(doc.get('scene', 0) == 0, 'Scene index out of bounds')
    for index in doc['scenes'][doc.get('scene', 0)]['nodes']:
        visit(index, IDENTITY)
    require(len(visited) == len(doc['nodes']) and used_meshes == set(range(len(meshes))), 'Unreachable nodes/meshes')
    require(positions and triangles == entry['triangles'], 'Triangle count mismatch/empty geometry')
    bounds = [[op(p[a] for p in positions) for a in range(3)] for op in (min, max)]
    require(all(abs(a-b) < .002 for actual, expected in zip(bounds, entry['bounds_cm']) for a, b in zip(actual, expected)), 'Exported canonical bounds mismatch')
    require(all(abs(bounds[1][a] - bounds[0][a] - entry['dimensions_cm'][a]) < .002 for a in range(3)), 'Exported dimensions mismatch')
    require(entry['pivot_cm'] == [0, 0, 0], 'Manifest pivot changed')
    images = doc.get('images', [])
    require(all('uri' in i and 'bufferView' not in i for i in images), 'Expected external texture images')
    names = sorted(i['uri'] for i in images)
    require(names == entry['texture_files'], 'Texture inventory mismatch')
    for texture in doc.get('textures', []):
        require(0 <= texture['source'] < len(images), 'Texture source out of bounds')
    return names


def validate(root):
    manifest_path = artifact(root, 'manifest.json')
    manifest = json.loads(manifest_path.read_text())
    prefix = 'Presentation.Environment.Shared.'
    expected = {prefix + n for n in ('Wall400', 'Floor400', 'Arch240', 'Arch320', 'Stair600x120', 'Sconce', 'Barrel', 'Crate', 'Table', 'Bench', 'Debris')}
    expected.add(prefix + 'Stair600x120:descending')
    require(set(manifest['pieces']) == expected, 'B1 piece/descending variant inventory mismatch')
    hashes, referenced, total_glb, total_texture = {}, set(), 0, 0
    for key, entry in manifest['pieces'].items():
        require(entry['piece_id'] == key.split(':')[0] and entry['variant'] == ('descending' if ':' in key else 'default'), 'Piece identity mismatch')
        contract = CANONICAL_BOUNDS[key.removeprefix(prefix)]
        require(len(entry['bounds_cm']) == 2 and all(len(row) == 3 for row in entry['bounds_cm'])
                and all(abs(a-b) < .002 for actual, expected in zip(entry['bounds_cm'], contract)
                        for a, b in zip(actual, expected)), f'{key}: frozen canonical bounds mismatch')
        name = entry['file']
        require(name.endswith('.glb') and name not in hashes, 'Invalid/duplicate mesh file')
        data = verify_file(root, name, entry)
        referenced.update(glb(data, entry))
        total_glb += len(data)
        hashes[name] = hashlib.sha256(data).hexdigest()
    require(referenced == set(manifest['textures']), 'Texture manifest differs from GLB references')
    for name, entry in manifest['textures'].items():
        data = verify_file(root, name, entry)
        require(png_size(data) == entry['resolution'] == [512, 512], f'{name}: expected 512px texture')
        total_texture += len(data)
        hashes[name] = hashlib.sha256(data).hexdigest()
    require(total_glb == manifest['glb_bytes'] and total_texture == manifest['texture_bytes']
            and total_glb + total_texture == manifest['payload_bytes'], 'Payload totals mismatch')
    actual_files = {str(p.relative_to(root)) for p in root.rglob('*.glb')} | {str(p.relative_to(root)) for p in (root / 'textures').rglob('*') if p.is_file()}
    require(actual_files == set(hashes), 'Unmanifested/missing payload files')
    total = total_glb + total_texture + manifest_path.stat().st_size
    require(total <= 15_000_000, f'Payload including manifest exceeds 15MB: {total}')
    return hashes, total


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('output', type=Path, nargs='?', default=Path('Saved/ArtExport/env'))
    parser.add_argument('--compare', type=Path, help='Validate another export and compare mesh/texture hashes')
    args = parser.parse_args()
    try:
        hashes, total = validate(args.output)
        if args.compare:
            other, _ = validate(args.compare)
            require(hashes == other, 'Determinism mismatch: ' + ', '.join(sorted(k for k in hashes.keys() | other.keys() if hashes.get(k) != other.get(k))))
        print(f'B1 export valid: 12 meshes, {len(hashes)-12} textures, {total:,} bytes including manifest' + ('; deterministic comparison passed' if args.compare else ''))
    except (ValueError, KeyError, IndexError, TypeError, OSError, struct.error, zlib.error) as error:
        print(f'B1 export INVALID: {error}', file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
