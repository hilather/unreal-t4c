#!/usr/bin/env bash
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/lh-env.sh"
: "${BLENDER_ROOT:?Set BLENDER_ROOT to the pinned Blender 5.2.2 directory}"
cd "$LH_PROJECT_ROOT"
mkdir -p Saved/ArtExport
if [[ ${LH_ART_PLAYER_ONLY:-0} != 1 ]]; then
mkdir -p artsource/blender/.config
if [[ ${LH_ART_SKIP_EXPORT:-0} != 1 ]]; then
XDG_CONFIG_HOME="$PWD/artsource/blender/.config" "$BLENDER_ROOT/blender" \
  --background --factory-startup --python-exit-code 1 --python artsource/blender/build_env.py \
  -- --output Saved/ArtExport/env --skip-renders
fi
python3 artsource/blender/validate_env.py Saved/ArtExport/env
# Use the engine's existing PythonScript commandlet and AssetTools/Interchange.
# No project/plugin configuration is modified. Keep scratch scripts in Saved/.
cat > Saved/ArtExport/import_b1.py <<'PY'
import hashlib
import json
import struct
import shutil
import os
from pathlib import Path
import unreal as u

def main():
    root = Path(u.Paths.project_dir()).resolve()
    source = root / 'Saved/ArtExport/env'
    dest = '/Game/Lighthaven/Art/Env/B1'
    manifest = json.loads((source / 'manifest.json').read_text())
    assets = u.EditorAssetLibrary
    editor = u.MaterialEditingLibrary
    tools = u.AssetToolsHelpers.get_asset_tools()
    
    # A successful same-input run does no import/save, preserving package bytes.
    # Include tool source and engine version so edits cannot silently reuse stale assets.
    signature = hashlib.sha256((Path(__file__).read_bytes() +
        (source / 'manifest.json').read_bytes() + u.SystemLibrary.get_engine_version().encode())).hexdigest()
    stamp = root / 'Content/Lighthaven/Art/Env/B1/import-receipt.json'
    content = root / 'Content/Lighthaven/Art/Env/B1'
    def inventory():
        return {str(p.relative_to(content)): {'bytes': p.stat().st_size,
                'sha256': hashlib.sha256(p.read_bytes()).hexdigest()}
                for p in sorted(content.rglob('*.uasset'))}
    if stamp.exists():
        old = json.loads(stamp.read_text())
        if old['signature'] == signature and old['assets'] == inventory():
            u.log('B1_IMPORT_UNCHANGED: package bytes preserved')
            return
    
    # LFS lockable packages are read-only on a fresh clone. Never silently chmod.
    locked = [p for p in content.rglob('*.uasset') if not p.stat().st_mode & 0o200]
    if locked and os.environ.get('LH_ART_REIMPORT') != '1':
        raise RuntimeError('Changed B1 inputs require LH_ART_REIMPORT=1 (explicit owner write permission)')
    for p in locked:
        p.chmod(p.stat().st_mode | 0o200)

    def import_one(filename, name, folder=dest):
        task = u.AssetImportTask()
        task.set_editor_property('filename', str(filename))
        task.set_editor_property('destination_path', folder)
        task.set_editor_property('destination_name', name)
        task.set_editor_property('automated', True)
        task.set_editor_property('replace_existing', True)
        task.set_editor_property('save', False)
        tools.import_asset_tasks([task])
        objects = task.get_objects()
        if not objects:
            raise RuntimeError('Import produced no objects: '+str(filename))
        return objects
    
    textures = {}
    for filename in sorted(manifest['textures']):
        path = Path(filename)
        tex = next(o for o in import_one(source / path, 'T_'+path.stem, dest+'/Textures') if isinstance(o, u.Texture2D))
        normal = 'normal' in path.stem.lower()
        orm = path.stem.lower().endswith('_orm')
        tex.set_editor_property('srgb', not (normal or orm))
        tex.set_editor_property('compression_settings', u.TextureCompressionSettings.TC_NORMALMAP if normal else
            u.TextureCompressionSettings.TC_MASKS if orm else u.TextureCompressionSettings.TC_DEFAULT)
        if normal:
            tex.set_editor_property('flip_green_channel', True)  # external PNG: OpenGL +Y -> Unreal -Y
        textures[path.stem] = tex
    
    master = assets.load_asset(dest+'/M_B1') if assets.does_asset_exist(dest+'/M_B1') else None
    if master is None:
        master = tools.create_asset('M_B1',dest,u.Material,u.MaterialFactoryNew())
        def texture_node(name, sample):
            node = editor.create_material_expression(master,u.MaterialExpressionTextureSampleParameter2D)
            node.set_editor_property('parameter_name', name)
            node.set_editor_property('texture', sample)
            return node
        base = texture_node('Base', next(v for k,v in textures.items() if 'base' in k.lower()))
        normal = texture_node('Normal', next(v for k,v in textures.items() if 'normal' in k.lower()))
        normal.set_editor_property('sampler_type', u.MaterialSamplerType.SAMPLERTYPE_NORMAL)
        orm = texture_node('ORM', next(v for k,v in textures.items() if k.lower().endswith('_orm')))
        orm.set_editor_property('sampler_type', u.MaterialSamplerType.SAMPLERTYPE_MASKS)
        color = editor.create_material_expression(master,u.MaterialExpressionVertexColor)
        mult = editor.create_material_expression(master,u.MaterialExpressionMultiply)
        editor.connect_material_expressions(base,'RGB',mult,'A')
        editor.connect_material_expressions(color,'RGB',mult,'B')
        tint = editor.create_material_expression(master,u.MaterialExpressionVectorParameter)
        tint.set_editor_property('parameter_name','FlameColor')
        tint.set_editor_property('default_value',u.LinearColor(1,.21,.012,1))
        flame = editor.create_material_expression(master,u.MaterialExpressionScalarParameter)
        flame.set_editor_property('parameter_name','Flame')
        flame.set_editor_property('default_value',0.0)
        blend = editor.create_material_expression(master,u.MaterialExpressionLinearInterpolate)
        editor.connect_material_expressions(mult,'',blend,'A')
        editor.connect_material_expressions(tint,'',blend,'B')
        editor.connect_material_expressions(flame,'',blend,'Alpha')
        editor.connect_material_property(blend,'',u.MaterialProperty.MP_BASE_COLOR)
        emission = editor.create_material_expression(master,u.MaterialExpressionMultiply)
        editor.connect_material_expressions(tint,'',emission,'A')
        editor.connect_material_expressions(flame,'',emission,'B')
        editor.connect_material_property(emission,'',u.MaterialProperty.MP_EMISSIVE_COLOR)
        editor.connect_material_property(normal,'RGB',u.MaterialProperty.MP_NORMAL)
        for channel, prop in [('R',u.MaterialProperty.MP_AMBIENT_OCCLUSION),('G',u.MaterialProperty.MP_ROUGHNESS),('B',u.MaterialProperty.MP_METALLIC)]:
            editor.connect_material_property(orm,channel,prop)
        master.set_editor_property('blend_mode',u.BlendMode.BLEND_MASKED)
        master.set_editor_property('used_with_instanced_static_meshes',True)
        world = editor.create_material_expression(master,u.MaterialExpressionWorldPosition)
        clip = editor.create_material_expression(master,u.MaterialExpressionCustom)
        clip.set_editor_property('output_type',u.CustomMaterialOutputType.CMOT_FLOAT1)
        clip.set_editor_property('code','float3 p = W - O; float3 q = abs(float3(dot(p,X),dot(p,Y),dot(p,Z))); return all(q <= E + 0.01) ? 1.0 : 0.0;')
        inputs=[]
        for name in ['W','O','X','Y','Z','E']:
            inp=u.CustomInput()
            inp.set_editor_property('input_name',name)
            inputs.append(inp)
        clip.set_editor_property('inputs',inputs)
        editor.connect_material_expressions(world,'',clip,'W')
        for name,param,value in [('O','ClipOrigin',(0,0,0)),('X','ClipX',(1,0,0)),('Y','ClipY',(0,1,0)),('Z','ClipZ',(0,0,1)),('E','ClipExtent',(1e8,1e8,1e8))]:
            node=editor.create_material_expression(master,u.MaterialExpressionVectorParameter)
            node.set_editor_property('parameter_name',param)
            node.set_editor_property('default_value',u.LinearColor(*value,0))
            editor.connect_material_expressions(node,'RGB',clip,name)
        editor.connect_material_property(clip,'',u.MaterialProperty.MP_OPACITY_MASK)
        editor.recompile_material(master)
        
    master.set_editor_property('used_with_instanced_static_meshes',True)

    materials = {}
    for family in ['stone','timber','iron','flame']:
        name = 'MI_B1_'+family
        mat = (assets.load_asset(dest+'/'+name) if assets.does_asset_exist(dest+'/'+name) else None) or tools.create_asset(name,dest,u.MaterialInstanceConstant,u.MaterialInstanceConstantFactoryNew())
        editor.set_material_instance_parent(mat,master)
        for parameter, suffix in [('Base','base'),('Normal','normal'),('ORM','orm')]:
            matching = [v for k,v in textures.items() if ('timber' if family=='flame' else family) in k.lower() and k.lower().endswith('_'+('basecolor' if suffix=='base' else suffix))]
            if len(matching)!=1:
                raise RuntimeError('Expected one texture for '+family+' '+suffix)
            editor.set_material_instance_texture_parameter_value(mat,parameter,matching[0])
        editor.set_material_instance_scalar_parameter_value(mat,'Flame',1.0 if family=='flame' else 0.0)
        materials[family] = mat
    
    # Native glTF conversion maps (X,Y,Z)->(X,Z,Y). Blender's output is
    # (author X, author Z, -author Y), so reflect glTF Z in an import-only
    # copy, including normals/tangents and winding. Source exports untouched.
    normalized = root / 'Saved/ArtExport/unreal-env'
    normalized.mkdir(exist_ok=True)
    shutil.copytree(source/'textures',normalized/'textures',dirs_exist_ok=True)
    def canonical_glb(filename):
        raw=(source/filename).read_bytes()
        json_size=struct.unpack_from('<I',raw,12)[0]
        doc=json.loads(raw[20:20+json_size])
        offset=20+json_size
        bin_size,kind=struct.unpack_from('<II',raw,offset)
        assert kind==0x004e4942
        data=bytearray(raw[offset+8:offset+8+bin_size])
        seen=set()
        for mesh in doc['meshes']:
            for primitive in mesh['primitives']:
                for semantic in ['POSITION','NORMAL','TANGENT']:
                    index=primitive['attributes'].get(semantic)
                    if index is None or index in seen: continue
                    seen.add(index)
                    acc=doc['accessors'][index]; view=doc['bufferViews'][acc['bufferView']]
                    assert acc['componentType']==5126 and 'sparse' not in acc
                    width=4 if semantic=='TANGENT' else 3
                    start=view.get('byteOffset',0)+acc.get('byteOffset',0)
                    stride=view.get('byteStride',width*4)
                    for i in range(acc['count']):
                        for component in ([2,3] if semantic=='TANGENT' else [2]):
                            pos=start+i*stride+component*4
                            struct.pack_into('<f',data,pos,-struct.unpack_from('<f',data,pos)[0])
                    if 'min' in acc:
                        low,high=acc['min'][2],acc['max'][2]
                        acc['min'][2],acc['max'][2]=-high,-low
                index=primitive['indices']
                if index in seen: continue
                seen.add(index)
                acc=doc['accessors'][index]; view=doc['bufferViews'][acc['bufferView']]
                fmt={5121:'B',5123:'H',5125:'I'}[acc['componentType']]
                size=struct.calcsize(fmt); start=view.get('byteOffset',0)+acc.get('byteOffset',0)
                assert acc['count']%3==0 and 'byteStride' not in view
                for i in range(0,acc['count'],3):
                    pos=start+i*size
                    a,b,c=struct.unpack_from('<'+fmt*3,data,pos)
                    struct.pack_into('<'+fmt*3,data,pos,a,c,b)
        encoded=json.dumps(doc,separators=(',',':')).encode()
        encoded+=b' '*((-len(encoded))%4)
        payload=struct.pack('<II',len(encoded),0x4e4f534a)+encoded+struct.pack('<II',len(data),kind)+data
        path=normalized/filename
        path.write_bytes(struct.pack('<III',0x46546c67,2,12+len(payload))+payload)
        return path

    for entry in manifest['pieces'].values():
        name = Path(entry['file']).stem
        objects = import_one(canonical_glb(entry['file']), 'SM_'+name)
        mesh = next(o for o in objects if isinstance(o,u.StaticMesh))
        # Interchange may use the glTF node name instead of destination_name.
        wanted = dest+'/SM_'+name
        if mesh.get_path_name().split('.')[0] != wanted:
            if assets.does_asset_exist(wanted):
                raise RuntimeError('Unexpected Interchange naming conflict at '+wanted)
            if not assets.rename_asset(mesh.get_path_name(),wanted):
                raise RuntimeError('Could not normalize mesh asset name')
        bounds = mesh.get_bounding_box()
        for actual, expected in [(bounds.min,entry['bounds_cm'][0]),(bounds.max,entry['bounds_cm'][1])]:
            if max(abs(a-b) for a,b in zip([actual.x,actual.y,actual.z],expected)) > .2:
                raise RuntimeError('Axis/units/pivot mismatch: '+name+' '+str(bounds))
        slots = mesh.get_editor_property('static_materials')
        for i,slot in enumerate(slots):
            label = str(slot.get_editor_property('material_slot_name')).lower()
            family = next((f for f in materials if f in label),None)
            if family is None:
                raise RuntimeError('Unmapped material slot (including flame): '+name+' '+label)
            mesh.set_material(i,materials[family])
        nanite = mesh.get_editor_property('nanite_settings')
        nanite.set_editor_property('enabled',False)
        mesh.set_editor_property('nanite_settings',nanite)
        assets.save_loaded_asset(mesh)
    
    # The catalog is importer-owned. Remove Interchange's temporary material
    # graphs/duplicate textures after meshes point at the four shared instances.
    keep={'M_B1'} | {'MI_B1_'+f for f in materials} | {'SM_'+Path(e['file']).stem for e in manifest['pieces'].values()}
    keep_paths={dest+'/'+name for name in keep} | {dest+'/Textures/T_'+Path(p).stem for p in manifest['textures']}
    generated=assets.list_assets(dest,recursive=True,include_folder=False)
    generated.sort(key=lambda p: (0 if '/Materials/' in p else 1,p))
    for path in generated:
        if path.split('.')[0] not in keep_paths and not assets.delete_asset(path):
            raise RuntimeError('Could not remove temporary Interchange asset '+path)
    assets.save_directory(dest,only_if_is_dirty=True,recursive=True)
    files = inventory()
    total = sum(v['bytes'] for v in files.values())
    for path, info in files.items():
        u.log('B1_ASSET_BYTES '+path+' '+str(info['bytes']))
    if total>15_000_000:
        raise RuntimeError('Imported assets exceed 15MB: '+str(total))
    stamp.write_text(json.dumps({'signature':signature,'assets':files,'total_bytes':total},indent=2)+'\n')
    u.log('B1_IMPORT_TOTAL_BYTES '+str(total))

main()
PY
"$UE_ROOT/Engine/Binaries/Linux/UnrealEditor-Cmd" "$LH_PROJECT" \
  -run=pythonscript "-Script=$PWD/Saved/ArtExport/import_b1.py" -EnablePlugins=PythonScriptPlugin \
  -stdout -FullStdOutLogOutput -nullrhi -unattended -nosound -nop4 '-DDC=(Local)' "-LocalDataCachePath=$PWD/Saved/DerivedDataCache" \
  '-ini:Engine:[ConsoleVariables]:HomeScreen.EnableHomeScreen=0'

# Creature source remains owned by the visual lane; only ignored outputs are written.
mkdir -p Saved/CreatureBlenderConfig
for batch in pilot batch2; do
  args=()
  if [[ $batch == batch2 ]]; then args+=(--batch2); fi
  if [[ ${LH_ART_SKIP_EXPORT:-0} != 1 ]]; then
  XDG_CONFIG_HOME="$PWD/Saved/CreatureBlenderConfig" "$BLENDER_ROOT/blender" \
    --background --factory-startup --threads 3 --python-exit-code 1 \
    --python artsource/creatures/build.py -- --no-render "${args[@]}"
  fi
  python3 artsource/creatures/validate_glb.py "${args[@]}" > "Saved/ArtExport/creatures-$batch-validation.json"
done
cat > Saved/ArtExport/import_creatures.py <<'PY'
import hashlib, json, os, struct
from pathlib import Path
import unreal as u

root=Path(u.Paths.project_dir()).resolve()
source=root/'artsource/creatures/output'
content=root/'Content/Lighthaven/Art/Creatures'
dest='/Game/Lighthaven/Art/Creatures'
kinds=('rat','bat','slime','goblin','giant_spider','balork','goblin_warrior','atrocity','dungeon_bat','giant_bat','undead_bat')
tools=u.AssetToolsHelpers.get_asset_tools()
assets=u.EditorAssetLibrary
signature=hashlib.sha256(Path(__file__).read_bytes()+u.SystemLibrary.get_engine_version().encode())
# Hash every input consumed below, including external textures and the bounds
# evidence. A texture-only edit must not reuse a receipt for older materials.
for kind in kinds:
    for suffix in ('.glb', '_base.png', '_normal.png', '_orm.png', '-validation.json'):
        path=source/(kind+suffix)
        signature.update(path.name.encode())
        signature.update(path.read_bytes())
signature=signature.hexdigest()
stamp=content/'import-receipt.json'
def inventory():
    return {str(p.relative_to(content)):dict(bytes=p.stat().st_size,sha256=hashlib.sha256(p.read_bytes()).hexdigest())
            for p in sorted(content.rglob('*.uasset'))}
if stamp.exists():
    old=json.loads(stamp.read_text())
    if old['signature']==signature and old['assets']==inventory():
        u.log('CREATURE_IMPORT_UNCHANGED: package bytes preserved')
        raise SystemExit(0)
# A fully imported but over-budget candidate stays local, with a scratch inventory.
# Same-input rejection must preserve its bytes as well; it is never an approved receipt.
candidate=root/'Saved/ArtExport/creature-import-candidate.json'
if candidate.exists():
    old=json.loads(candidate.read_text())
    current=inventory()
    if old['signature']==signature and old['assets']==current:
        total=sum(v['bytes'] for v in current.values())
        if total>25_000_000:
            u.log('CREATURE_IMPORT_UNCHANGED_REJECTED: package bytes preserved')
            raise RuntimeError('Creature asset budget exceeded: '+str(total))
locked=[p for p in content.rglob('*.uasset') if not p.stat().st_mode & 0o200]
if locked and os.environ.get('LH_ART_REIMPORT')!='1':
    raise RuntimeError('Changed creature inputs require LH_ART_REIMPORT=1')
for p in locked:
    p.chmod(p.stat().st_mode | 0o200)
def canonical_creature(kind):
    """Reflect the complete glTF rig into the project's authored Unreal basis."""
    raw=(source/(kind+'.glb')).read_bytes()
    json_size=struct.unpack_from('<I',raw,12)[0]
    doc=json.loads(raw[20:20+json_size])
    offset=20+json_size
    bin_size,tag=struct.unpack_from('<II',raw,offset)
    assert tag==0x004e4942
    data=bytearray(raw[offset+8:offset+8+bin_size])
    seen={}
    def reflect(index,components,width):
        if index in seen:
            assert seen[index]==(components,width), 'Accessor used with incompatible basis transforms'
            return
        seen[index]=(components,width)
        acc=doc['accessors'][index]; view=doc['bufferViews'][acc['bufferView']]
        assert acc['componentType']==5126 and 'sparse' not in acc
        start=view.get('byteOffset',0)+acc.get('byteOffset',0)
        stride=view.get('byteStride',width*4)
        for i in range(acc['count']):
            for c in components:
                pos=start+i*stride+c*4
                struct.pack_into('<f',data,pos,-struct.unpack_from('<f',data,pos)[0])
        if 'min' in acc:
            for c in components:
                low,high=acc['min'][c],acc['max'][c]
                acc['min'][c],acc['max'][c]=-high,-low
    # Column-major matrices transform as S M S, S=diag(1,1,-1,1).
    matrix_components=tuple(c*4+r for c in range(4) for r in range(4) if (r==2)!=(c==2))
    for node in doc['nodes']:
        if 'translation' in node: node['translation'][2]*=-1
        if 'rotation' in node:
            node['rotation'][0]*=-1; node['rotation'][1]*=-1
        if 'matrix' in node:
            for c in matrix_components: node['matrix'][c]*=-1
    for skin in doc['skins']:
        if 'inverseBindMatrices' in skin:
            reflect(skin['inverseBindMatrices'],matrix_components,16)
    for animation in doc['animations']:
        for channel in animation['channels']:
            path=channel['target']['path']
            sampler=animation['samplers'][channel['sampler']]
            assert sampler.get('interpolation','LINEAR') in ('LINEAR','STEP')
            if path=='translation': reflect(sampler['output'],(2,),3)
            elif path=='rotation': reflect(sampler['output'],(0,1),4)
            else: assert path=='scale', 'Unsupported morph animation'
    reversed_indices=set()
    for mesh in doc['meshes']:
        for primitive in mesh['primitives']:
            assert not primitive.get('targets') and primitive.get('mode',4)==4
            for semantic,components,width in (('POSITION',(2,),3),('NORMAL',(2,),3),('TANGENT',(2,3),4)):
                if semantic in primitive['attributes']:
                    reflect(primitive['attributes'][semantic],components,width)
            index=primitive['indices']
            if index in reversed_indices: continue
            reversed_indices.add(index)
            acc=doc['accessors'][index]; view=doc['bufferViews'][acc['bufferView']]
            fmt={5121:'B',5123:'H',5125:'I'}[acc['componentType']]
            size=struct.calcsize(fmt); start=view.get('byteOffset',0)+acc.get('byteOffset',0)
            assert acc['count']%3==0 and 'byteStride' not in view
            for i in range(0,acc['count'],3):
                pos=start+i*size
                a,b,c=struct.unpack_from('<'+fmt*3,data,pos)
                struct.pack_into('<'+fmt*3,data,pos,a,c,b)
    encoded=json.dumps(doc,separators=(',',':')).encode()
    encoded+=b' '*((-len(encoded))%4)
    payload=struct.pack('<II',len(encoded),0x4e4f534a)+encoded+struct.pack('<II',len(data),tag)+data
    normalized=root/'Saved/ArtExport/unreal-creatures'
    normalized.mkdir(parents=True,exist_ok=True)
    path=normalized/(kind+'.glb')
    path.write_bytes(struct.pack('<III',0x46546c67,2,12+len(payload))+payload)
    return path

def split_weapon(path,kind):
    """Bake rigid equipment into its named bone's local space, without changing source."""
    if kind not in ('goblin','goblin_warrior','balork'):
        return path,None,None,None
    raw=path.read_bytes(); size=struct.unpack_from('<I',raw,12)[0]
    doc=json.loads(raw[20:20+size]); offset=20+size
    length,tag=struct.unpack_from('<II',raw,offset)
    data=bytearray(raw[offset+8:offset+8+length])
    def values(index):
        acc=doc['accessors'][index]; view=doc['bufferViews'][acc['bufferView']]
        width={'VEC3':3,'VEC4':4,'MAT4':16}[acc['type']]
        fmt,unit={5126:('f',4),5123:('H',2),5121:('B',1)}[acc['componentType']]
        start=view.get('byteOffset',0)+acc.get('byteOffset',0); stride=view.get('byteStride',width*unit)
        return acc,start,stride,fmt,width,[struct.unpack_from('<'+fmt*width,data,start+i*stride) for i in range(acc['count'])]
    def write(filename,d,b):
        encoded=json.dumps(d,separators=(',',':')).encode(); encoded+=b' '*((-len(encoded))%4)
        payload=struct.pack('<II',len(encoded),0x4e4f534a)+encoded+struct.pack('<II',len(b),tag)+b
        filename.write_bytes(struct.pack('<III',0x46546c67,2,12+len(payload))+payload)
    weapon_node=next(n for n in doc['nodes'] if n.get('name')==kind+'_weapon')
    body_node=next(n for n in doc['nodes'] if n.get('name')==kind+'_body')
    assert set(weapon_node)=={'name','mesh','skin'}, 'Unexpected weapon mesh transform'
    bone='Weapon_Main' if kind=='balork' else 'Weapon_R'
    bone_index=next(i for i,n in enumerate(doc['nodes']) if n.get('name')==bone)
    skin=doc['skins'][weapon_node['skin']]
    joint=skin['joints'].index(bone_index)
    matrix=values(skin['inverseBindMatrices'])[-1][joint]
    points=[]
    for primitive in doc['meshes'][body_node['mesh']]['primitives']:
        points.extend((p[0]*100,p[2]*100,p[1]*100) for p in values(primitive['attributes']['POSITION'])[-1])
    spec=dict(min_cm=[min(p[c] for p in points) for c in range(3)],max_cm=[max(p[c] for p in points) for c in range(3)])
    weapon_mesh=doc['meshes'][weapon_node.pop('mesh')]
    weapon_node.pop('skin')
    doc['meshes']=[doc['meshes'][body_node['mesh']]]; body_node['mesh']=0
    body=path.with_name(kind+'-body.glb'); write(body,doc,data)
    for primitive in weapon_mesh['primitives']:
        attrs=primitive['attributes']
        joints=values(attrs.pop('JOINTS_0'))[-1]
        weights=values(attrs.pop('WEIGHTS_0'))[-1]
        assert all(all(w<1.e-6 or (j==joint and abs(w-1)<1.e-6) for j,w in zip(js,ws)) for js,ws in zip(joints,weights)), 'Equipment is not rigid to its named bone'
        for semantic in ('POSITION','NORMAL','TANGENT'):
            if semantic not in attrs: continue
            acc,start,stride,fmt,width,rows=values(attrs[semantic])
            transformed=[]
            for i,row in enumerate(rows):
                xyz=[sum(matrix[c*4+r]*row[c] for c in range(3))+(matrix[12+r] if semantic=='POSITION' else 0) for r in range(3)]
                if semantic!='POSITION':
                    norm=sum(x*x for x in xyz)**.5; xyz=[x/norm for x in xyz]
                result=xyz+list(row[3:]); transformed.append(result)
                struct.pack_into('<'+fmt*width,data,start+i*stride,*result)
            if 'min' in acc:
                acc['min']=[min(p[c] for p in transformed) for c in range(width)]
                acc['max']=[max(p[c] for p in transformed) for c in range(width)]
    doc['meshes']=[weapon_mesh]; doc['nodes']=[dict(name='SM_Weapon',mesh=0)]
    doc['scenes']=[dict(nodes=[0])]; doc['scene']=0
    doc.pop('skins'); doc.pop('animations')
    weapon=path.with_name(kind+'-weapon.glb'); write(weapon,doc,data)
    return body,weapon,bone,spec

for kind in kinds:
    folder=dest+'/'+kind
    body_file,weapon_file,weapon_bone,body_bounds=split_weapon(canonical_creature(kind),kind)
    task=u.AssetImportTask()
    for key,value in dict(filename=str(body_file),destination_path=folder,
                          destination_name='SK_'+kind,automated=True,replace_existing=True,save=False).items():
        task.set_editor_property(key,value)
    tools.import_asset_tasks([task])
    objects=task.get_objects()
    meshes=[o for o in objects if isinstance(o,u.SkeletalMesh)]
    # Bounds below verify the full-rig conversion, including asymmetric meshes.
    if len(meshes)!=1:
        raise RuntimeError(kind+': expected one combined skeletal mesh; got '+str(len(meshes)))
    mesh=meshes[0]
    wanted=folder+'/SK_'+kind
    if mesh.get_path_name().split('.')[0]!=wanted and not assets.rename_asset(mesh.get_path_name(),wanted):
        raise RuntimeError('Cannot normalize skeletal name '+kind)
    spec=body_bounds or json.loads((source/(kind+'-validation.json')).read_text())['rest_bounds']
    box=mesh.get_bounds()
    low=box.origin-box.box_extent
    high=box.origin+box.box_extent
    u.log('CREATURE_IMPORT_BOUNDS '+kind+' '+json.dumps(dict(
        actual_min_cm=[low.x,low.y,low.z], actual_max_cm=[high.x,high.y,high.z],
        expected_min_cm=spec['min_cm'], expected_max_cm=spec['max_cm'])))
    for actual,expected in ((low,spec['min_cm']),(high,spec['max_cm'])):
        if max(abs(a-b) for a,b in zip((actual.x,actual.y,actual.z),expected))>.5:
            raise RuntimeError(kind+': native glTF axis/unit/bounds mismatch: '+str(box))
    clips=[o for o in objects if isinstance(o,u.AnimSequence)]
    for action in ('idle','move','attack','hit','death'):
        matches=[a for a in clips if action in a.get_name().lower()]
        if len(matches)!=1 or matches[0].get_editor_property('skeleton')!=mesh.get_editor_property('skeleton'):
            raise RuntimeError(kind+': missing/ambiguous compatible action '+action)
        wanted=folder+'/A_'+action
        if matches[0].get_path_name().split('.')[0]!=wanted and not assets.rename_asset(matches[0].get_path_name(),wanted):
            raise RuntimeError('Cannot normalize animation '+action)
    for path in assets.list_assets(folder,recursive=True,include_folder=False):
        obj=assets.load_asset(path)
        if isinstance(obj,u.Texture2D):
            name=obj.get_name().lower()
            normal='normal' in name
            orm='orm' in name or 'occlusion' in name or 'metallic' in name
            obj.set_editor_property('srgb',not(normal or orm))
            obj.set_editor_property('compression_settings',u.TextureCompressionSettings.TC_NORMALMAP if normal else u.TextureCompressionSettings.TC_MASKS if orm else u.TextureCompressionSettings.TC_DEFAULT)
            if normal: obj.set_editor_property('flip_green_channel',True)
    editor=u.MaterialEditingLibrary
    textures={}
    for channel in ('base','normal','orm'):
        texture_task=u.AssetImportTask()
        for key,value in dict(filename=str(source/(kind+'_'+channel+'.png')),destination_path=folder,
                              destination_name='T_'+channel,automated=True,replace_existing=True,save=False).items():
            texture_task.set_editor_property(key,value)
        tools.import_asset_tasks([texture_task])
        tex=next(o for o in texture_task.get_objects() if isinstance(o,u.Texture2D))
        tex.set_editor_property('srgb',channel=='base')
        tex.set_editor_property('compression_no_alpha',True)
        tex.set_editor_property('mip_gen_settings',u.TextureMipGenSettings.TMGS_FROM_TEXTURE_GROUP)
        tex.set_editor_property('lod_group',u.TextureGroup.TEXTUREGROUP_CHARACTER_NORMAL_MAP if channel=='normal' else
                                u.TextureGroup.TEXTUREGROUP_CHARACTER_SPECULAR if channel=='orm' else u.TextureGroup.TEXTUREGROUP_CHARACTER)
        tex.set_editor_property('compression_settings',u.TextureCompressionSettings.TC_NORMALMAP if channel=='normal' else u.TextureCompressionSettings.TC_MASKS if channel=='orm' else u.TextureCompressionSettings.TC_DEFAULT)
        if channel=='normal': tex.set_editor_property('flip_green_channel',True)
        textures[channel]=tex
    master=assets.load_asset(dest+'/M_Creature') if assets.does_asset_exist(dest+'/M_Creature') else None
    if master is None:
        master=tools.create_asset('M_Creature',dest,u.Material,u.MaterialFactoryNew())
        for channel in ('base','normal','orm'):
            node=editor.create_material_expression(master,u.MaterialExpressionTextureSampleParameter2D)
            node.set_editor_property('parameter_name',channel)
            node.set_editor_property('texture',textures[channel])
            if channel=='normal': node.set_editor_property('sampler_type',u.MaterialSamplerType.SAMPLERTYPE_NORMAL)
            if channel=='orm': node.set_editor_property('sampler_type',u.MaterialSamplerType.SAMPLERTYPE_MASKS)
            if channel=='base': editor.connect_material_property(node,'RGB',u.MaterialProperty.MP_BASE_COLOR)
            elif channel=='normal': editor.connect_material_property(node,'RGB',u.MaterialProperty.MP_NORMAL)
            else:
                for c,prop in [('R',u.MaterialProperty.MP_AMBIENT_OCCLUSION),('G',u.MaterialProperty.MP_ROUGHNESS),('B',u.MaterialProperty.MP_METALLIC)]:
                    editor.connect_material_property(node,c,prop)
        master.set_editor_property('used_with_skeletal_mesh',True)
        editor.recompile_material(master)
    material=assets.load_asset(folder+'/MI_Creature') if assets.does_asset_exist(folder+'/MI_Creature') else tools.create_asset('MI_Creature',folder,u.MaterialInstanceConstant,u.MaterialInstanceConstantFactoryNew())
    editor.set_material_instance_parent(material,master)
    for channel,texture in textures.items(): editor.set_material_instance_texture_parameter_value(material,channel,texture)
    if weapon_file:
        weapon_task=u.AssetImportTask()
        for key,value in dict(filename=str(weapon_file),destination_path=folder,
                              destination_name='SM_Weapon',automated=True,replace_existing=True,save=False).items():
            weapon_task.set_editor_property(key,value)
        tools.import_asset_tasks([weapon_task])
        weapon=next(o for o in weapon_task.get_objects() if isinstance(o,u.StaticMesh))
        wanted=folder+'/SM_Weapon'
        if weapon.get_path_name().split('.')[0]!=wanted and not assets.rename_asset(weapon.get_path_name(),wanted):
            raise RuntimeError('Cannot normalize weapon '+kind)
        for i in range(len(weapon.get_editor_property('static_materials'))): weapon.set_material(i,material)
        if not u.LHMonsterVisual.configure_weapon_socket(mesh,weapon_bone):
            raise RuntimeError('Cannot configure named weapon socket '+kind)
        assets.save_loaded_asset(weapon)
    # Python cannot call UE 5.8's BlueprintSetter directly. The native helper
    # updates the persistent SkeletalMaterialsInfo cache through SetMaterials.
    if not u.LHMonsterVisual.configure_creature_material(mesh,material):
        raise RuntimeError('Cannot bind skeletal materials '+kind)
    if not assets.save_loaded_asset(mesh,only_if_is_dirty=False):
        raise RuntimeError('Cannot persist skeletal material bindings '+kind)
    keep={o.get_path_name().split('.')[0] for o in [material,*textures.values()]}
    generated=assets.list_assets(folder,recursive=True,include_folder=False)
    generated.sort(key=lambda p: (0 if isinstance(assets.load_asset(p),u.MaterialInterface) else 1,p))
    for path in generated:
        obj=assets.load_asset(path)
        if isinstance(obj,(u.MaterialInterface,u.Texture2D)) and path.split('.')[0] not in keep:
            if not assets.delete_asset(path): raise RuntimeError('Cannot remove duplicate material/texture '+path)
    assets.save_loaded_asset(master)
    if not assets.save_directory(folder,only_if_is_dirty=True,recursive=True):
        raise RuntimeError('Saving creature failed '+kind)
    # Last save of this body: material cleanup must finish before omitting the
    # redundant DDC-restorable vertex arrays. Retain all MeshDescription data.
    if not u.LHMonsterVisual.compact_creature_storage(mesh):
        raise RuntimeError('Cannot compact rebuildable creature cache '+kind)
    if not assets.save_loaded_asset(mesh,only_if_is_dirty=False):
        raise RuntimeError('Cannot persist compact creature '+kind)
files=inventory()
# Report actual Unreal classes rather than inferring types from filenames.
by_type={}
by_creature={}
for path,info in files.items():
    obj=assets.load_asset(dest+'/'+path[:-7])
    asset_type=obj.get_class().get_name()
    by_type[asset_type]=by_type.get(asset_type,0)+info['bytes']
    creature=path.split('/')[0] if '/' in path else 'shared'
    by_creature[creature]=by_creature.get(creature,0)+info['bytes']
(root/'Saved/ArtExport/creature-size-summary.json').write_text(json.dumps(
    dict(by_type=by_type,by_creature=by_creature,packages=len(files)),indent=2)+'\n')
u.log('CREATURE_BYTES_BY_TYPE '+json.dumps(by_type,sort_keys=True))
files=inventory()
total=sum(v['bytes'] for v in files.values())
for kind in kinds:
    u.log('CREATURE_ASSET_BYTES '+kind+' '+str(sum(v['bytes'] for p,v in files.items() if p.startswith(kind+'/'))))
candidate.write_text(json.dumps(dict(signature=signature,assets=files,total_bytes=total),indent=2)+'\n')
if total>25_000_000:
    raise RuntimeError('Creature asset budget exceeded: '+str(total))
stamp.write_text(json.dumps(dict(signature=signature,assets=files,total_bytes=total),indent=2)+'\n')
u.log('CREATURE_IMPORT_TOTAL_BYTES '+str(total))
PY
"$UE_ROOT/Engine/Binaries/Linux/UnrealEditor-Cmd" "$LH_PROJECT" \
  -run=pythonscript "-Script=$PWD/Saved/ArtExport/import_creatures.py" -EnablePlugins=PythonScriptPlugin \
  -stdout -FullStdOutLogOutput -nullrhi -unattended -nosound -nop4 '-DDC=(Local)' "-LocalDataCachePath=$PWD/Saved/DerivedDataCache" \
  '-ini:Engine:[ConsoleVariables]:HomeScreen.EnableHomeScreen=0'

fi

# Player rig source stays in the visual lane; generated packages stay local.
mkdir -p Saved/PlayerBlenderConfig
player_export_current=$(python3 - "$BLENDER_ROOT/blender" <<'PYPLAYER'
import hashlib, json, subprocess, sys
from pathlib import Path
source=Path('artsource/player')
hash=hashlib.sha256(subprocess.check_output([sys.argv[1],'--version']))
for path in sorted(source.glob('*.py'))+[source/'appearances.json']:
    hash.update(path.name.encode()); hash.update(path.read_bytes())
files=[source/'output'/(kind+suffix) for kind in ('player_a','player_b') for suffix in ('.glb','_base.png','_normal.png','_orm.png')]
inventory={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in files if p.is_file()}
current=dict(signature=hash.hexdigest(),outputs=inventory)
Path('Saved/ArtExport/player-export-candidate.json').write_text(json.dumps(current,indent=2)+'\n')
stamp=Path('Saved/ArtExport/player-export-receipt.json')
print('1' if len(inventory)==8 and stamp.exists() and json.loads(stamp.read_text())==current else '0')
PYPLAYER
)
if [[ ${LH_ART_SKIP_EXPORT:-0} != 1 && $player_export_current != 1 ]]; then
  XDG_CONFIG_HOME="$PWD/Saved/PlayerBlenderConfig" "$BLENDER_ROOT/blender" \
    --background --factory-startup --threads 3 --python-exit-code 1 \
    --python artsource/player/build.py
elif [[ $player_export_current == 1 ]]; then
  echo PLAYER_EXPORT_UNCHANGED
fi
python3 artsource/player/validate_glb.py > Saved/ArtExport/player-validation.json
if [[ ${LH_ART_SKIP_EXPORT:-0} != 1 || $player_export_current == 1 ]]; then
python3 - <<'PYPLAYER'
import hashlib, json
from pathlib import Path
receipt=json.loads(Path('Saved/ArtExport/player-export-candidate.json').read_text())
receipt['outputs']={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for kind in ('player_a','player_b') for suffix in ('.glb','_base.png','_normal.png','_orm.png') for p in [Path('artsource/player/output')/(kind+suffix)]}
Path('Saved/ArtExport/player-export-receipt.json').write_text(json.dumps(receipt,indent=2)+'\n')
PYPLAYER
fi
"$UE_ROOT/Engine/Binaries/Linux/UnrealEditor-Cmd" "$LH_PROJECT" \
  -run=pythonscript "-Script=$PWD/Source/Lighthaven/Visual/Player/import_player.py" -EnablePlugins=PythonScriptPlugin \
  -stdout -FullStdOutLogOutput -nullrhi -unattended -nosound -nop4 '-DDC=(Local)' "-LocalDataCachePath=$PWD/Saved/DerivedDataCache" \
  '-ini:Engine:[ConsoleVariables]:HomeScreen.EnableHomeScreen=0'
