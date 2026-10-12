"""Deterministic original props, 256px shared CPU-baked atlas and static GLBs."""
import argparse
import hashlib
import json
import struct
import sys
from pathlib import Path
import bpy
sys.path.insert(0,str(Path(__file__).resolve().parent))
import materials
import models
import validate_glb
ROOT=Path(__file__).resolve().parent
OUT=ROOT/'output'

def clear_objects():
    bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)

def record(path,output):
    data=path.read_bytes()
    return {'file':path.relative_to(output).as_posix(),'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest()}

def export(objects,path):
    bpy.ops.object.select_all(action='DESELECT')
    for o in objects:o.select_set(True)
    bpy.context.view_layer.objects.active=objects[0]
    bpy.ops.export_scene.gltf(filepath=str(path),export_format='GLB',use_selection=True,export_animations=False,export_tangents=True,export_extras=False)
    data=path.read_bytes();length=struct.unpack_from('<I',data,12)[0];doc=json.loads(data[20:20+length]);binary=data[28+length:]
    image_views=set()
    for im in doc.get('images',[]):
        view_id=im.pop('bufferView');image_views.add(view_id);v=doc['bufferViews'][view_id]
        name=im['name'].split('.')[0];filename='textures/'+name+'.png'
        target=path.parent/filename;payload=binary[v.get('byteOffset',0):v.get('byteOffset',0)+v['byteLength']]
        target.write_bytes(payload);im['uri']=filename;im.pop('mimeType',None)
    packed=bytearray();views=[];mapping={}
    for i,v in enumerate(doc['bufferViews']):
        if i in image_views:continue
        packed.extend(b'\0'*(-len(packed)%4));mapping[i]=len(views)
        views.append(dict(v,byteOffset=len(packed)));start=v.get('byteOffset',0);packed.extend(binary[start:start+v['byteLength']])
    for a in doc['accessors']:a['bufferView']=mapping[a['bufferView']]
    doc['bufferViews']=views;doc['buffers'][0]['byteLength']=len(packed)
    encoded=json.dumps(doc,separators=(',',':')).encode();encoded+=b' '*(-len(encoded)%4);packed.extend(b'\0'*(-len(packed)%4))
    path.write_bytes(struct.pack('<III',0x46546c67,2,28+len(encoded)+len(packed))+struct.pack('<II',len(encoded),0x4e4f534a)+encoded+struct.pack('<II',len(packed),0x004e4942)+packed)

def run(output):
    output=output.resolve();output.mkdir(parents=True,exist_ok=True)
    clear_objects();atlas=materials.bake(output)
    manifest={'revision':'W5-23-v1','provenance':{'status':'Prototype','ruleset':'LH_Prototype_v1','source':'Original W5-23 presentation construction, inspected supplied concepts only','source_url':None,'date':'2026-10-11'},'blender_version':bpy.app.version_string,'seed':523,'units':'metres','axes':'+X forward, +Z up; glTF (x,z,-y)','assets':[],'textures':[],'icons':[]}
    for identity,name in models.ALL:
        clear_objects();objects=models.create(name,atlas);path=output/(name+'.glb');export(objects,path)
        measured=validate_glb.validate_glb(path)
        row=dict(id=identity,**record(path,output),**measured,socket=models.SOCKETS.get(name),pivot='canonical origin; see importer table')
        manifest['assets'].append(row)
        print('PROP_EXPORT',name,row['triangles'],row['bytes'],flush=True)
    for path in sorted((output/'textures').glob('*.png')):manifest['textures'].append(record(path,output))
    # Regeneration invalidates prior icon evidence until preview.py refreshes it.
    (output/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
    print('PROP_TOTAL',sum(a['bytes'] for a in manifest['assets']+manifest['textures']),flush=True)

if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--output',type=Path,default=OUT)
    args=parser.parse_args(sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else [])
    run(args.output)
