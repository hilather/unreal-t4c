"""Reconstruct all modules twice and compare geometry/UV/weight/rest-bone hashes.
Run with the same pinned Blender/version/thread count as build.py. Does not
rewrite the exports, textures or manifest; both reconstructions must also
match the delivered manifest. This is not a cross-platform byte guarantee.
"""
import hashlib
import json
from pathlib import Path
import sys
sys.dont_write_bytecode=True
import bpy
sys.path.insert(0,str(Path(__file__).resolve().parent))
import build


def construct(roster):
    build.clear()
    build.TEMPLATES.clear()
    build.BONES.clear()
    bpy.data.orphans_purge(do_local_ids=True,do_linked_ids=True,do_recursive=True)
    build.IMAGES={channel:bpy.data.images.load(str(build.OUT/('NPC_Textile_'+channel+'.png'))) for channel in ('base','normal')}
    build.IMAGES['normal'].colorspace_settings.name='Non-Color'
    build.PALETTE=build.materials.palette('Template',roster[0],build.IMAGES)
    for body in ('A','B'):build.player_templates(body)
    for row in roster:
        build.costume_template(row['body'],row['outfit'])
        for extra in row['extras']:build.costume_template(row['body'],extra,True)
    modules={key:build.mesh_digest(value['data']) for key,value in build.TEMPLATES.items()}
    bones={body:{name:dict(head=list(h),tail=list(t),parent=p) for name,(h,t,p) in data.items()} for body,data in build.BONES.items()}
    return modules,bones

roster=json.loads((build.HERE/'roster.json').read_text())['npcs']
manifest=json.loads((build.OUT/'manifest.json').read_text())
first,bones1=construct(roster)
second,bones2=construct(roster)
expected={key:value['geometry_sha256'] for key,value in manifest['modules'].items()}
report=dict(blender=bpy.app.version_string,threads=bpy.context.scene.render.threads,
            module_count=len(first),first=first,second=second,
            repeat_differences=[key for key in first if first[key]!=second[key]],
            export_differences=[key for key in first if first[key]!=expected.get(key)],
            rest_bones_match=bones1==bones2==manifest['skeletons'],
            limitations='Same-host pinned build; not a cross-Blender/hardware binary guarantee.')
report['success']=not report['repeat_differences'] and not report['export_differences'] and report['rest_bones_match']
(build.OUT/'geometry-validation.json').write_text(json.dumps(report,indent=2)+'\n')
print('NPC geometry audit',json.dumps({k:v for k,v in report.items() if k not in ('first','second')}),flush=True)
assert report['success'], 'geometry reconstruction differs; inspect geometry-validation.json'
