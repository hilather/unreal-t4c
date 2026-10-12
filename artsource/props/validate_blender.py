"""Reimport every delivered GLB in Blender and check the manifest independently."""
import argparse
import json
import math
import sys
from pathlib import Path

import bpy


def require(condition, message):
    if not condition:
        raise ValueError(message)


def validate(output):
    output = output.resolve()
    manifest = json.loads((output/'manifest.json').read_text())
    require(len(manifest['assets']) == 29, 'expected 29 assets')
    results, loaded_images = [], set()
    for entry in manifest['assets']:
        bpy.ops.object.select_all(action='SELECT')
        bpy.ops.object.delete(use_global=False)
        for collection in (bpy.data.meshes, bpy.data.materials, bpy.data.images, bpy.data.actions):
            for block in list(collection):
                collection.remove(block)
        bpy.ops.import_scene.gltf(filepath=str(output/entry['file']))
        objects = list(bpy.context.scene.objects)
        require(all(o.type == 'MESH' for o in objects), entry['id']+' unexpected nonmesh object')
        require(sorted(o.name for o in objects) == sorted(entry['nodes']), entry['id']+' named parts mismatch')
        triangles, positions, materials = 0, [], set()
        for obj in objects:
            require(obj.parent is None, entry['id']+' unexpected parent')
            require(all(abs(obj.matrix_world[r][c]-(1 if r == c else 0)) < 1e-5 for r in range(4) for c in range(4)), entry['id']+' transform/pivot not identity')
            require(not obj.modifiers and not obj.animation_data, entry['id']+' nonstatic mesh')
            obj.data.calc_loop_triangles()
            triangles += len(obj.data.loop_triangles)
            positions.extend(tuple(obj.matrix_world @ vertex.co) for vertex in obj.data.vertices)
            require(all(slot.material is not None for slot in obj.material_slots), entry['id']+' missing material')
            materials.update(slot.material.name for slot in obj.material_slots)
        require(triangles == entry['triangles'], entry['id']+' triangle count mismatch')
        require(sorted(materials) == sorted(entry['materials']), entry['id']+' material names mismatch')
        bounds = {'min':[min(v[a] for v in positions) for a in range(3)], 'max':[max(v[a] for v in positions) for a in range(3)]}
        require(all(math.isfinite(x) for row in bounds.values() for x in row), entry['id']+' nonfinite bounds')
        for key in ('min','max'):
            require(all(abs(a-b) < 1e-5 for a,b in zip(bounds[key], entry['bounds_m'][key])), entry['id']+' bounds mismatch')
        require(all(abs(bounds['max'][a]-bounds['min'][a]-entry['size_m'][a]) < 1e-5 for a in range(3)), entry['id']+' dimensions mismatch')
        images = []
        for image in bpy.data.images:
            path = Path(bpy.path.abspath(image.filepath)).resolve()
            relative = path.relative_to(output).as_posix()
            require(relative in {t['file'] for t in manifest['textures']}, entry['id']+' unexpected texture path')
            require(path.is_file() and tuple(image.size) == (256,256) and image.has_data, entry['id']+' texture not loaded at 256x256')
            require(len(image.pixels) == 256*256*4, entry['id']+' texture pixel load failed')
            images.append(relative)
            loaded_images.add(relative)
        if 'Props.WornAtlas' in materials:
            require(set(images) == {t['file'] for t in manifest['textures']}, entry['id']+' atlas textures missing on import')
        results.append({'id':entry['id'], 'mesh_parts':len(objects), 'triangles':triangles, 'images':sorted(images)})
    require(loaded_images == {t['file'] for t in manifest['textures']}, 'shared texture import incomplete')
    return {'assets':len(results), 'triangles':sum(r['triangles'] for r in results), 'loaded_images':sorted(loaded_images), 'reimport':results}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=Path(__file__).resolve().parent/'output')
    args = parser.parse_args(sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else [])
    print(json.dumps(validate(args.output), indent=2))
