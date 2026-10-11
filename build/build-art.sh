#!/usr/bin/env bash
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/lh-env.sh"
: "${BLENDER_ROOT:?Set BLENDER_ROOT to the pinned Blender 5.2.2 directory}"
cd "$LH_PROJECT_ROOT"
mkdir -p Saved/ArtExport
if [[ ${LH_ART_PLAYER_ONLY:-0} != 1 && ${LH_ART_NPC_ONLY:-0} != 1 ]]; then
mkdir -p artsource/blender/.config
if [[ ${LH_ART_SKIP_EXPORT:-0} != 1 ]]; then
XDG_CONFIG_HOME="$PWD/artsource/blender/.config" "$BLENDER_ROOT/blender" \
  --background --factory-startup --python-exit-code 1 --python artsource/blender/build_env.py \
  -- --output Saved/ArtExport/env --skip-renders
fi
python3 artsource/blender/validate_styles.py Saved/ArtExport/env
# Use the engine's existing PythonScript commandlet and AssetTools/Interchange.
# No project/plugin configuration is modified. Keep scratch scripts in Saved/.
cat > Saved/ArtExport/import_env.py <<'PY'
import hashlib
import json
import struct
import shutil
import os
from pathlib import Path
import unreal as u
# Exact legacy importer bytes retain compatibility with the committed B1 receipt.
import base64, zlib
LEGACY_B1 = zlib.decompress(base64.b64decode('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')).decode()

SHARED_TEXTURES = {}

def main(style):
    root = Path(u.Paths.project_dir()).resolve()
    source = root / 'Saved/ArtExport/env' / style
    style_label = 'B1' if style == 'B1Cellar' else style
    dest = '/Game/Lighthaven/Art/Env/' + style_label
    manifest = json.loads((source / 'manifest.json').read_text())
    assets = u.EditorAssetLibrary
    editor = u.MaterialEditingLibrary
    tools = u.AssetToolsHelpers.get_asset_tools()
    
    shared = root / 'Content/Lighthaven/Art/Env/Shared'
    def shared_inventory():
        return {str(p.relative_to(shared)): {'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()} for p in sorted(shared.rglob('*.uasset'))}
    # A successful same-input run does no import/save, preserving package bytes.
    # Include tool source and engine version so edits cannot silently reuse stale assets.
    signature = hashlib.sha256((Path(__file__).read_bytes() +
        (source / 'manifest.json').read_bytes() + u.SystemLibrary.get_engine_version().encode())).hexdigest()
    stamp = root / ('Content/Lighthaven/Art/Env/'+style_label+'/import-receipt.json')
    content = root / ('Content/Lighthaven/Art/Env/'+style_label)
    def inventory():
        return {str(p.relative_to(content)): {'bytes': p.stat().st_size,
                'sha256': hashlib.sha256(p.read_bytes()).hexdigest()}
                for p in sorted(content.rglob('*.uasset'))}
    if stamp.exists():
        old = json.loads(stamp.read_text())
        if old['signature'] in ({signature, hashlib.sha256((LEGACY_B1.encode() + (source / 'manifest.json').read_bytes() + u.SystemLibrary.get_engine_version().encode())).hexdigest()} if style == 'B1Cellar' else {signature}) and old['assets'] == inventory() and all(shared_inventory().get(k)==v for k,v in old.get('shared_assets',{}).items()):
            for filename in manifest['textures']:
                digest=hashlib.sha256((source/filename).read_bytes()).hexdigest()
                path=dest+'/Textures/T_'+Path(filename).stem
                if not assets.does_asset_exist(path): path='/Game/Lighthaven/Art/Env/Shared/Textures/T_'+digest[:24]
                if assets.does_asset_exist(path): SHARED_TEXTURES[digest]=assets.load_asset(path)
            u.log(style_label+'_IMPORT_UNCHANGED: package bytes preserved')
            return
    
    # LFS lockable packages are read-only on a fresh clone. Never silently chmod.
    locked = [p for p in content.rglob('*.uasset') if not p.stat().st_mode & 0o200]
    if locked and os.environ.get('LH_ART_REIMPORT') != '1':
        raise RuntimeError('Changed B1 inputs require LH_ART_REIMPORT=1 (explicit owner write permission)')
    for p in locked:
        p.chmod(p.stat().st_mode | 0o200)

    if style!='B1Cellar':
        # Keep the JSON receipt on disk; delete_directory reports failure for
        # non-asset files even after deleting all packages.
        for existing in assets.list_assets(dest,recursive=True,include_folder=False):
            if not assets.delete_asset(existing): raise RuntimeError('Cannot replace changed style asset '+existing)

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
        digest=hashlib.sha256((source/path).read_bytes()).hexdigest()
        tex=SHARED_TEXTURES.get(digest)
        if tex is None and style!='B1Cellar':
            existing='/Game/Lighthaven/Art/Env/Shared/Textures/T_'+digest[:24]
            if assets.does_asset_exist(existing): tex=assets.load_asset(existing)
        if tex is None:
            folder=dest+'/Textures' if style=='B1Cellar' else '/Game/Lighthaven/Art/Env/Shared/Textures'
            asset_name='T_'+path.stem if style=='B1Cellar' else 'T_'+digest[:24]
            tex=next(o for o in import_one(source/path,asset_name,folder) if isinstance(o,u.Texture2D))
            SHARED_TEXTURES[digest]=tex
        if tex.get_path_name().startswith(dest+'.') or tex.get_path_name().startswith(dest+'/') or style!='B1Cellar' and '/Shared/' in tex.get_path_name():
            normal = 'normal' in path.stem.lower()
            orm = path.stem.lower().endswith('_orm')
            tex.set_editor_property('srgb', not (normal or orm))
            tex.set_editor_property('compression_settings', u.TextureCompressionSettings.TC_NORMALMAP if normal else
                u.TextureCompressionSettings.TC_MASKS if orm else u.TextureCompressionSettings.TC_DEFAULT)
            if normal:
                tex.set_editor_property('flip_green_channel', True)  # external PNG: OpenGL +Y -> Unreal -Y
        textures[path.stem] = tex
    
    master_path = '/Game/Lighthaven/Art/Env/B1/M_B1'
    master = assets.load_asset(master_path) if assets.does_asset_exist(master_path) else None
    if master is None and style != 'B1Cellar':
        raise RuntimeError('B1 master must be imported before other styles')
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
        
    if style=='B1Cellar': master.set_editor_property('used_with_instanced_static_meshes',True)

    # Scalar-only Blender surfaces use tiny generated data textures on the
    # existing master. No graph edit or B1 package save is required.
    if style!='B1Cellar':
        import zlib
        def constant_texture(key, rgba, normal=False, srgb=False):
            def chunk(tag,data):
                return struct.pack('>I',len(data))+tag+data+struct.pack('>I',zlib.crc32(tag+data)&0xffffffff)
            png=b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',1,1,8,6,0,0,0))+chunk(b'IDAT',zlib.compress(b'\0'+bytes(rgba)))+chunk(b'IEND',b'')
            digest=hashlib.sha256(key.encode()+png).hexdigest()[:24]
            folder='/Game/Lighthaven/Art/Env/Shared/Textures'; name='T_'+digest
            path=folder+'/'+name
            if assets.does_asset_exist(path): return assets.load_asset(path)
            scratch=root/'Saved/ArtExport'/('constant-'+digest+'.png'); scratch.write_bytes(png)
            tex=next(o for o in import_one(scratch,name,folder) if isinstance(o,u.Texture2D))
            tex.set_editor_property('srgb',srgb)
            tex.set_editor_property('compression_settings',u.TextureCompressionSettings.TC_NORMALMAP if normal else u.TextureCompressionSettings.TC_DEFAULT if srgb else u.TextureCompressionSettings.TC_MASKS)
            assets.save_loaded_asset(tex)
            return tex
        textures['wet_basecolor']=textures['stone_basecolor']
        textures['wet_normal']=constant_texture('flat-normal',(128,128,255,255),normal=True)
        textures['wet_orm']=constant_texture('wet-orm',(255,26,0,255))
        def srgb_byte(value):
            return round(255*(12.92*value if value<=.0031308 else 1.055*value**(1/2.4)-.055))
        textures['oxide_basecolor']=constant_texture('oxide-base',tuple(srgb_byte(v) for v in (.14,.033,.019))+(255,),srgb=True)
        textures['oxide_normal']=textures['wet_normal']
        textures['oxide_orm']=constant_texture('oxide-orm',(255,209,0,255))
    materials = {}
    families = sorted({Path(t).stem.rsplit('_',1)[0] for t in manifest['textures']}) + (['flame'] if style=='B1Cellar' else ['flame','wet','oxide'])
    for family in families:
        name = 'MI_'+style_label+'_'+family
        mat = (assets.load_asset(dest+'/'+name) if assets.does_asset_exist(dest+'/'+name) else None) or tools.create_asset(name,dest,u.MaterialInstanceConstant,u.MaterialInstanceConstantFactoryNew())
        editor.set_material_instance_parent(mat,master)
        for parameter, suffix in [('Base','base'),('Normal','normal'),('ORM','orm')]:
            texture_family={'flame':'timber'}.get(family,family)
            if family in ('plaster','cloth') and suffix!='base': texture_family='stone' if family=='plaster' else 'timber'
            matching = [v for k,v in textures.items() if k.lower()==texture_family+'_'+('basecolor' if suffix=='base' else suffix)]
            if len(matching)!=1:
                raise RuntimeError('Expected one texture for '+family+' '+suffix)
            editor.set_material_instance_texture_parameter_value(mat,parameter,matching[0])
        editor.set_material_instance_scalar_parameter_value(mat,'Flame',1.0 if family=='flame' else 0.0)
        materials[family] = mat
    
    # Native glTF conversion maps (X,Y,Z)->(X,Z,Y). Blender's output is
    # (author X, author Z, -author Y), so reflect glTF Z in an import-only
    # copy, including normals/tangents and winding. Source exports untouched.
    normalized = root / 'Saved/ArtExport/unreal-env' / style
    normalized.mkdir(parents=True,exist_ok=True)
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
        common = name in ('Sconce','Barrel','Crate','Table','Bench') or style in ('Church','B2Damp') and name in ('Debris','Arch240','Arch320','Stair600x120','Stair600x120Descending')
        if style!='B1Cellar' and common:
            b1_source=root/'Saved/ArtExport/env/B1Cellar'/entry['file']
            if (source/entry['file']).read_bytes()!=b1_source.read_bytes():
                raise RuntimeError('Shared mesh changed: '+style+'/'+name)
            if not assets.does_asset_exist('/Game/Lighthaven/Art/Env/B1/SM_'+name):
                raise RuntimeError('Shared B1 mesh missing: '+name)
            continue
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
                raise RuntimeError('Unmapped material slot (including flame): '+name+' '+style_label)
            mesh.set_material(i,materials[family])
        nanite = mesh.get_editor_property('nanite_settings')
        nanite.set_editor_property('enabled',False)
        mesh.set_editor_property('nanite_settings',nanite)
        assets.save_loaded_asset(mesh)
    
    # The catalog is importer-owned. Remove Interchange's temporary material
    # graphs/duplicate textures after meshes point at the four shared instances.
    keep={'M_B1'} | {'MI_'+style_label+'_'+f for f in materials} | {'SM_'+Path(e['file']).stem for e in manifest['pieces'].values()}
    keep_paths={dest+'/'+name for name in keep} | {dest+'/Textures/T_'+Path(p).stem for p in manifest['textures']}
    generated=assets.list_assets(dest,recursive=True,include_folder=False)
    generated.sort(key=lambda p: (0 if '/Materials/' in p else 1,p))
    for path in generated:
        if path.split('.')[0] not in keep_paths and not assets.delete_asset(path):
            raise RuntimeError('Could not remove temporary Interchange asset '+path)
    assets.save_directory(dest,only_if_is_dirty=True,recursive=True)
    assets.save_directory('/Game/Lighthaven/Art/Env/Shared',only_if_is_dirty=True,recursive=True)
    files = inventory()
    total = sum(v['bytes'] for v in files.values())
    for path, info in files.items():
        u.log(style_label+'_ASSET_BYTES '+path+' '+str(info['bytes']))
    if total>15_000_000:
        raise RuntimeError('Imported assets exceed 15MB: '+str(total))
    stamp.write_text(json.dumps({'signature':signature,'assets':files,'total_bytes':total,'shared_assets':shared_inventory()},indent=2)+'\n')
    u.log(style_label+'_IMPORT_TOTAL_BYTES '+str(total))

for style in ('B1Cellar','Church','B2Damp','B3Crypt','B4Ritual'):
    main(style)
new_root = Path(u.Paths.project_dir()) / 'Content/Lighthaven/Art/Env'
totals = {s: sum(p.stat().st_size for p in (new_root/s).rglob('*.uasset')) for s in ('Church','B2Damp','B3Crypt','B4Ritual')}
u.log('ENV_NEW_STYLE_BYTES '+json.dumps(totals))
shared_total=sum(p.stat().st_size for p in (new_root/'Shared').rglob('*.uasset'))
u.log('ENV_SHARED_POOL_BYTES '+str(shared_total))
if sum(totals.values()) + shared_total > 12_000_000:
    raise RuntimeError('Four new styles exceed 12MB: '+str(totals))
PY
"$UE_ROOT/Engine/Binaries/Linux/UnrealEditor-Cmd" "$LH_PROJECT" \
  -run=pythonscript "-Script=$PWD/Saved/ArtExport/import_env.py" -EnablePlugins=PythonScriptPlugin \
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
# One-time migration of the committed pre-W5-08f receipt. Rebuild its exact
# historic sources locally; the importer verifies the complete legacy signature
# before trusting these exports as per-kind input evidence.
creature_legacy_receipt=$(python3 - <<'PYLEGACY'
import json
from pathlib import Path
p=Path('Content/Lighthaven/Art/Creatures/import-receipt.json')
print('1' if p.exists() and not json.loads(p.read_text()).get('inputs') else '0')
PYLEGACY
)
if [[ $creature_legacy_receipt == 1 && -z ${LH_ART_LEGACY_CREATURE_SOURCE:-} ]]; then
  legacy_root="$PWD/Saved/ArtExport/legacy-creature-source-9ef79e4"
  if [[ ! -f $legacy_root/artsource/creatures/output/undead_bat-validation.json ]]; then
    mkdir -p "$legacy_root"
    git archive 9ef79e46e4bc8902b731e078fe177a186565865a artsource/creatures | tar -x -C "$legacy_root"
    for batch in pilot batch2; do
      args=(); if [[ $batch == batch2 ]]; then args+=(--batch2); fi
      XDG_CONFIG_HOME="$PWD/Saved/CreatureBlenderConfig" "$BLENDER_ROOT/blender" \
        --background --factory-startup --threads 3 --python-exit-code 1 \
        --python "$legacy_root/artsource/creatures/build.py" -- --no-render "${args[@]}"
    done
  fi
  export LH_ART_LEGACY_CREATURE_SOURCE="$legacy_root/artsource/creatures/output"
fi
cat > Saved/ArtExport/import_creatures.py <<'PY'
import hashlib, json, os, struct
from pathlib import Path
prepare=os.environ.get('LH_CREATURE_PREPARE')=='1'
if not prepare: import unreal as u
root=Path.cwd().resolve() if prepare else Path(u.Paths.project_dir()).resolve()
source=root/'artsource/creatures/output'
content=root/'Content/Lighthaven/Art/Creatures'
dest='/Game/Lighthaven/Art/Creatures'
kinds=('rat','bat','slime','goblin','giant_spider','balork','goblin_warrior','atrocity','dungeon_bat','giant_bat','undead_bat')
tools=None if prepare else u.AssetToolsHelpers.get_asset_tools()
assets=None if prepare else u.EditorAssetLibrary
# Stable per-creature input fingerprints permit bounded selective reimport.
# A legacy receipt has no per-kind input evidence; an optional archived source
# directory can supply that evidence without blessing current inputs as historic.
engine=(os.environ['LH_ART_ENGINE_VERSION'] if prepare else u.SystemLibrary.get_engine_version()).encode()
# Bump when import semantics change; receipt migration retains legacy settings.
importer_version='W5-15b-selective-v1'
def fingerprint(directory, kind):
    digest=hashlib.sha256(engine)
    for suffix in ('.glb', '_base.png', '_normal.png', '_orm.png'):
        path=directory/(kind+suffix)
        digest.update(path.name.encode()); digest.update(path.read_bytes())
    # Only rest_bounds is consumed by this importer. Global source-provenance
    # hashes in the validation report change even for byte-identical creatures.
    bounds=json.loads((directory/(kind+'-validation.json')).read_text())['rest_bounds']
    digest.update(json.dumps(bounds,sort_keys=True,separators=(',',':')).encode())
    return digest.hexdigest()

inputs={kind:fingerprint(source,kind) for kind in kinds}
signature=hashlib.sha256(json.dumps(inputs,sort_keys=True).encode()).hexdigest()
stamp=content/'import-receipt.json'
def inventory():
    return {str(p.relative_to(content)):dict(bytes=p.stat().st_size,sha256=hashlib.sha256(p.read_bytes()).hexdigest())
            for p in sorted(content.rglob('*.uasset'))}
old=json.loads(stamp.read_text()) if stamp.exists() else {}
current=inventory()
previous=old.get('inputs',{})
if old and not previous:
    archive=os.environ.get('LH_ART_LEGACY_CREATURE_SOURCE')
    if not archive:
        raise RuntimeError('Legacy creature receipt lacks per-kind input evidence; set LH_ART_LEGACY_CREATURE_SOURCE to the archived export matching that receipt')
    import base64, zlib
    legacy_script=zlib.decompress(base64.b64decode('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'))
    legacy_signature=hashlib.sha256(legacy_script+engine)
    for kind in kinds:
        for suffix in ('.glb', '_base.png', '_normal.png', '_orm.png', '-validation.json'):
            path=Path(archive)/(kind+suffix)
            legacy_signature.update(path.name.encode()); legacy_signature.update(path.read_bytes())
    if legacy_signature.hexdigest()!=old['signature']:
        raise RuntimeError('Archived creature inputs do not match the legacy receipt')
    previous={kind:fingerprint(Path(archive),kind) for kind in kinds}
changed={kind for kind in kinds if old.get('importer_version',importer_version)!=importer_version or inputs[kind]!=previous.get(kind) or
    {p:v for p,v in current.items() if p.startswith(kind+'/')} !=
    {p:v for p,v in old.get('assets',{}).items() if p.startswith(kind+'/')}}
if not changed and old.get('assets')==current:
    print('CREATURE_IMPORT_UNCHANGED: package bytes preserved') if prepare else u.log('CREATURE_IMPORT_UNCHANGED: package bytes preserved')
    raise SystemExit(0)
print('CREATURE_CHANGED_KINDS '+json.dumps(sorted(changed))) if prepare else u.log('CREATURE_CHANGED_KINDS '+json.dumps(sorted(changed)))
if prepare:
    rejected=root/'Saved/ArtExport/creature-import-candidate.json'
    if rejected.exists():
        cached=json.loads(rejected.read_text())
        if cached.get('signature')==signature and cached.get('assets')==current and sum(v['bytes'] for v in current.values())>25_000_000:
            raise RuntimeError('Creature asset budget exceeded; rejected candidate bytes preserved')
    import shutil, time
    locked=[p for kind in changed for p in (content/kind).rglob('*.uasset') if not p.stat().st_mode & 0o200]
    if locked and os.environ.get('LH_ART_REIMPORT')!='1': raise RuntimeError('Changed creature inputs require LH_ART_REIMPORT=1')
    # Move selected folders before engine startup: rooted animation controllers
    # prevent safe in-process deletion/reuse of the old Interchange skeleton.
    backup=root/'Saved/ArtExport/creature-reimport-backup'/str(time.time_ns())
    for kind in sorted(changed):
        if (content/kind).exists():
            backup.mkdir(parents=True,exist_ok=True)
            shutil.move(str(content/kind),str(backup/kind))
    (root/'Saved/ArtExport/creature-reimport-plan.json').write_text(json.dumps(dict(changed=sorted(changed),before=current,backup=str(backup)),indent=2)+'\n')
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
locked=[p for kind in changed for p in (content/kind).rglob('*.uasset') if not p.stat().st_mode & 0o200]
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
    if kind not in changed: continue
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
for kind in set(kinds)-changed:
    if {p:v for p,v in files.items() if p.startswith(kind+'/')} != {p:v for p,v in current.items() if p.startswith(kind+'/')}:
        raise RuntimeError('Unchanged creature packages were modified: '+kind)
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
candidate.write_text(json.dumps(dict(signature=signature,importer_version=importer_version,inputs=inputs,assets=files,total_bytes=total),indent=2)+'\n')
if total>25_000_000:
    raise RuntimeError('Creature asset budget exceeded: '+str(total))
stamp.write_text(json.dumps(dict(signature=signature,importer_version=importer_version,inputs=inputs,assets=files,total_bytes=total),indent=2)+'\n')
u.log('CREATURE_IMPORT_TOTAL_BYTES '+str(total))
PY
# Select and stage changed folders before loading rooted animation controllers.
creature_engine_version=$(python3 - "$UE_ROOT/Engine/Build/Build.version" <<'PYVERSION'
import json,sys
v=json.load(open(sys.argv[1]))
print(f"{v['MajorVersion']}.{v['MinorVersion']}.{v['PatchVersion']}-{v['Changelist']}+{v['BranchName']}")
PYVERSION
)
LH_CREATURE_PREPARE=1 LH_ART_ENGINE_VERSION="$creature_engine_version" python3 Saved/ArtExport/import_creatures.py
"$UE_ROOT/Engine/Binaries/Linux/UnrealEditor-Cmd" "$LH_PROJECT" \
  -run=pythonscript "-Script=$PWD/Saved/ArtExport/import_creatures.py" -EnablePlugins=PythonScriptPlugin \
  -stdout -FullStdOutLogOutput -nullrhi -unattended -nosound -nop4 '-DDC=(Local)' "-LocalDataCachePath=$PWD/Saved/DerivedDataCache" \
  '-ini:Engine:[ConsoleVariables]:HomeScreen.EnableHomeScreen=0'

fi

# Player rig source stays in the visual lane; generated packages stay local.
if [[ ${LH_ART_NPC_ONLY:-0} != 1 ]]; then
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
# The player importer is owned by another lane. Stage its unchanged import
# behavior locally, replacing only its receipt input list. Never hash the
# compiled module: unrelated gameplay changes must not invalidate art packages.
python3 - <<'PYPLAYER_SIGNATURE'
from pathlib import Path
source = Path('Source/Lighthaven/Visual/Player/import_player.py')
text = source.read_text()
old = "root/'Binaries/Linux/libUnrealEditor-Lighthaven.so'"
new = "root/'Source/Lighthaven/Visual/Player/LHPlayerVisual.h'"
if text.count(old) != 1:
    raise RuntimeError('Player receipt input layout changed; review signature adapter')
Path('Saved/ArtExport/import_player.py').write_text(text.replace(old, new))
PYPLAYER_SIGNATURE
"$UE_ROOT/Engine/Binaries/Linux/UnrealEditor-Cmd" "$LH_PROJECT" \
  -run=pythonscript "-Script=$PWD/Saved/ArtExport/import_player.py" -EnablePlugins=PythonScriptPlugin \
  -stdout -FullStdOutLogOutput -nullrhi -unattended -nosound -nop4 '-DDC=(Local)' "-LocalDataCachePath=$PWD/Saved/DerivedDataCache" \
  '-ini:Engine:[ConsoleVariables]:HomeScreen.EnableHomeScreen=0'

fi
# NPC imports depend on real player packages and use their exact body-specific rigs.
# NPC_ONLY is a local iteration aid; normal execution imports player first.
if [[ ${LH_ART_PLAYER_ONLY:-0} != 1 ]]; then
  python3 Source/Lighthaven/Visual/Npc/export_npc.py
  python3 Source/Lighthaven/Visual/Npc/prepare_npc.py
  "$UE_ROOT/Engine/Binaries/Linux/UnrealEditor-Cmd" "$LH_PROJECT" \
    -run=pythonscript "-Script=$PWD/Source/Lighthaven/Visual/Npc/import_npc.py" -EnablePlugins=PythonScriptPlugin -NoCrashDialog -NoAnalytics \
    -stdout -FullStdOutLogOutput -nullrhi -unattended -nosound -nop4 '-DDC=(Local)' "-LocalDataCachePath=$PWD/Saved/DerivedDataCache" \
    '-ini:Engine:[ConsoleVariables]:HomeScreen.EnableHomeScreen=0'
fi
