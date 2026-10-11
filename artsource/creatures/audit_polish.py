"""Compare W5-08f GLBs against a saved baseline; Python stdlib only.

Run with --before DIRECTORY --after DIRECTORY --report FILE.
The report is written even when a budget or rig regression is detected.
"""
import argparse
import json
import struct
from pathlib import Path

from validate_glb import png_red

KINDS = ('slime', 'bat', 'dungeon_bat', 'giant_bat', 'undead_bat')


def snapshot(path):
    blob = path.read_bytes()
    magic, version, total = struct.unpack_from('<III', blob)
    assert magic == 0x46546C67 and version == 2 and total == len(blob)
    chunks = {}
    offset = 12
    while offset < len(blob):
        length, tag = struct.unpack_from('<II', blob, offset)
        assert length % 4 == 0 and offset + 8 + length <= len(blob)
        assert tag not in chunks
        chunks[tag] = blob[offset + 8:offset + 8 + length]
        offset += 8 + length
    document = json.loads(chunks[0x4E4F534A])
    binary = chunks[0x004E4942]

    def view(index):
        item = document['bufferViews'][index]
        assert item.get('buffer', 0) == 0
        start = item.get('byteOffset', 0)
        end = start + item['byteLength']
        assert 0 <= start <= end <= len(binary)
        return binary[start:end]

    geometry_views = set()
    triangles = 0
    for mesh in document['meshes']:
        for primitive in mesh['primitives']:
            assert primitive.get('mode', 4) == 4
            indices = document['accessors'][primitive['indices']]
            assert indices['count'] % 3 == 0
            triangles += indices['count'] // 3
            for index in [primitive['indices'], *primitive['attributes'].values()]:
                accessor = document['accessors'][index]
                assert 'sparse' not in accessor
                geometry_views.add(accessor['bufferView'])
    images = [view(image['bufferView']) for image in document['images']]
    dimensions = [png_red(data, decode=False)[0] for data in images]
    channels = {}
    for material in document['materials']:
        pbr = material['pbrMetallicRoughness']
        for channel, reference in (
                ('base', pbr['baseColorTexture']),
                ('normal', material['normalTexture']),
                ('orm', pbr['metallicRoughnessTexture'])):
            source = document['textures'][reference['index']]['source']
            entry = {'bytes': len(images[source]), 'dimensions': dimensions[source]}
            if channel in channels:
                assert channels[channel] == entry, 'materials must share atlas channels'
            channels[channel] = entry
    nodes = document['nodes']
    names = [node.get('name', '') for node in nodes]
    assert len(set(names)) == len(names), 'named node hierarchy must be unambiguous'
    parents = {names[child]: names[parent]
               for parent, node in enumerate(nodes) for child in node.get('children', [])}
    hierarchy = {name: parents.get(name) for name in names}
    skins = [{'name': skin.get('name'),
              'joints': [names[index] for index in skin['joints']],
              'skeleton': names[skin['skeleton']] if 'skeleton' in skin else None}
             for skin in document['skins']]
    return {'triangles': triangles,
            'geometry_buffer_bytes': sum(document['bufferViews'][index]['byteLength']
                                         for index in geometry_views),
            'glb_bytes': len(blob), 'image_count': len(images),
            'texture_bytes': sum(map(len, images)), 'channels': channels,
            'actions': sorted(action['name'] for action in document['animations']),
            'skins': skins, 'node_parent_hierarchy': hierarchy,
            'socket_names': sorted(name for name in names if 'socket' in name.lower())}


def compare(before, after):
    failures = []
    for key in ('triangles', 'geometry_buffer_bytes', 'glb_bytes', 'texture_bytes'):
        if after[key] > before[key]:
            failures.append(f'{key} increased: {before[key]} -> {after[key]}')
    if after['image_count'] != 3:
        failures.append('expected exactly three embedded images')
    for channel in ('base', 'normal', 'orm'):
        if tuple(after['channels'][channel]['dimensions']) != (512, 512):
            failures.append(f'{channel} atlas must be 512px square')
        old, new = before['channels'][channel]['bytes'], after['channels'][channel]['bytes']
        if new > old:
            failures.append(f'{channel} PNG bytes increased: {old} -> {new}')
    for key in ('actions', 'skins', 'node_parent_hierarchy', 'socket_names'):
        if before[key] != after[key]:
            failures.append(f'{key} changed')
    return failures


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--before', type=Path, required=True)
    parser.add_argument('--after', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    report = {'passed': True, 'creatures': {}}
    for kind in KINDS:
        before = snapshot(args.before / (kind + '.glb'))
        after = snapshot(args.after / (kind + '.glb'))
        failures = compare(before, after)
        report['creatures'][kind] = {'before': before, 'after': after,
                                    'passed': not failures, 'failures': failures}
        report['passed'] &= not failures
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({'passed': report['passed'], 'report': str(args.report),
                      'failures': {kind: entry['failures']
                                   for kind, entry in report['creatures'].items()
                                   if entry['failures']}}))
    return 0 if report['passed'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
