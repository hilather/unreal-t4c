"""Real-input NPC kit importer. Run after the player import via build-art.sh."""
import ast, copy, hashlib, json, os, struct, sys
from pathlib import Path
import unreal as u
root=Path(u.Paths.project_dir()).resolve()
source=root/'artsource/npc/output'
content=root/'Content/Lighthaven/Art/Npc'
dest='/Game/Lighthaven/Art/Npc'
assets=u.EditorAssetLibrary
tools=u.AssetToolsHelpers.get_asset_tools()
editor=u.MaterialEditingLibrary
def load(path):
    return assets.load_asset(path) if assets.does_asset_exist(path) else None
manifest=json.loads((source/'manifest.json').read_text())
roster=json.loads((root/'artsource/npc/roster.json').read_text())['npcs']
assert len(manifest['npcs'])==len(roster)==15
# Reuse the exact player basis conversion, without running its import side effects.
player_import=root/'Source/Lighthaven/Visual/Player/import_player.py'
tree=ast.parse(player_import.read_text())
exec(compile(ast.Module(body=[n for n in tree.body if isinstance(n,ast.FunctionDef) and n.name in ('canonical_player','write_glb')],type_ignores=[]),str(player_import),'exec'))
sys.dont_write_bytecode=True
sys.path.insert(0,str(root/'Source/Lighthaven/Visual/Npc'))
from npc_receipt import signature as input_signature, inventory as asset_inventory
signature=input_signature(root)
def inventory(): return asset_inventory(content)
stamp=content/'import-receipt.json'
if stamp.exists():
    old=json.loads(stamp.read_text())
    if old['signature']==signature and old['assets'] and old['assets']==inventory():
        u.log('NPC_IMPORT_UNCHANGED'); raise SystemExit(0)
# A stale success receipt must never describe a failed partial reimport.
if stamp.exists(): stamp.unlink()
for p in content.rglob('*.uasset'): p.chmod(p.stat().st_mode|0o200)
def import_one(path,name,folder):
    task=u.AssetImportTask()
    for key,value in dict(filename=str(path),destination_path=folder,destination_name=name,automated=True,replace_existing=True,save=False).items(): task.set_editor_property(key,value)
    tools.import_asset_tasks([task]); objects=task.get_objects()
    if not objects: raise RuntimeError('Empty import '+str(path))
    return objects
# Deduplicate identical PNG bytes against existing player textures.
textures={}
for channel,info in manifest['textures'].items():
    path=source/info['file']; digest=hashlib.sha256(path.read_bytes()).hexdigest(); tex=None
    for png in (root/'artsource/player/output').glob('*.png'):
        if hashlib.sha256(png.read_bytes()).hexdigest()!=digest: continue
        kind,part=png.stem.rsplit('_',1)
        tex=load('/Game/Lighthaven/Art/Player/'+kind+'/T_'+part)
        if tex: break
    if not tex:
        tex=next(o for o in import_one(path,'T_'+channel,dest) if isinstance(o,u.Texture2D))
        tex.set_editor_property('srgb',channel=='base')
        if channel=='normal':
            tex.set_editor_property('compression_settings',u.TextureCompressionSettings.TC_NORMALMAP)
            tex.set_editor_property('flip_green_channel',True)
    textures[channel]=tex
masters={}
for cloth in (False,True):
    name='M_Textile' if cloth else 'M_Solid'
    mat=load(dest+'/'+name) or tools.create_asset(name,dest,u.Material,u.MaterialFactoryNew())
    editor.delete_all_material_expressions(mat)
    tint=editor.create_material_expression(mat,u.MaterialExpressionVectorParameter); tint.set_editor_property('parameter_name','Tint')
    if cloth:
        base=editor.create_material_expression(mat,u.MaterialExpressionTextureSampleParameter2D); base.set_editor_property('parameter_name','base'); base.set_editor_property('texture',textures['base'])
        mult=editor.create_material_expression(mat,u.MaterialExpressionMultiply)
        editor.connect_material_expressions(base,'RGB',mult,'A'); editor.connect_material_expressions(tint,'RGB',mult,'B'); editor.connect_material_property(mult,'',u.MaterialProperty.MP_BASE_COLOR)
        normal=editor.create_material_expression(mat,u.MaterialExpressionTextureSampleParameter2D); normal.set_editor_property('parameter_name','normal'); normal.set_editor_property('texture',textures['normal']); normal.set_editor_property('sampler_type',u.MaterialSamplerType.SAMPLERTYPE_NORMAL); editor.connect_material_property(normal,'RGB',u.MaterialProperty.MP_NORMAL)
    else: editor.connect_material_property(tint,'RGB',u.MaterialProperty.MP_BASE_COLOR)
    for name,prop in [('Roughness',u.MaterialProperty.MP_ROUGHNESS),('Metallic',u.MaterialProperty.MP_METALLIC)]:
        node=editor.create_material_expression(mat,u.MaterialExpressionScalarParameter); node.set_editor_property('parameter_name',name); editor.connect_material_property(node,'',prop)
    mat.set_editor_property('used_with_skeletal_mesh',True); editor.recompile_material(mat); masters[cloth]=mat
meshes={}; clips={}; evidence={}; keep={o.get_path_name().split('.')[0] for o in [*textures.values(),*masters.values()]}
for kit in manifest['kits']:
    body=kit['body']; kind=Path(kit['file']).stem
    player=load('/Game/Lighthaven/Art/Player/player_'+body.lower()+'/body/player_'+body.lower()+'-body/SkeletalMeshes/SK_body')
    if not player: raise RuntimeError('Missing player body '+body)
    raw=canonical_player(kind).read_bytes(); size=struct.unpack_from('<I',raw,12)[0]; doc=json.loads(raw[20:20+size]); length,tag=struct.unpack_from('<II',raw,20+size); data=raw[28+size:28+size+length]
    for index,key in enumerate(kit['mesh_keys']):
        node=next(n for n in doc['nodes'] if n.get('name')==key and 'mesh' in n)
        assert set(node)<= {'name','mesh','skin','extras'}, 'Unexpected module transform'
        meshdoc=doc['meshes'][node['mesh']]; part=copy.deepcopy(doc); part['meshes']=[copy.deepcopy(meshdoc)]
        for n in part['nodes']:
            if n.get('name')==key and 'mesh' in n: n['mesh']=0
            elif 'mesh' in n: n.pop('mesh'); n.pop('skin',None)
        if index: part.pop('animations',None)
        # Keep semantic slots but omit automatic duplicate texture imports.
        # NPC material instances below reference the shared images directly.
        used=sorted({p['material'] for p in part['meshes'][0]['primitives']})
        part['materials']=[copy.deepcopy(doc['materials'][i]) for i in used]
        for primitive in part['meshes'][0]['primitives']: primitive['material']=used.index(primitive['material'])
        for material in part['materials']:
            for field in ('normalTexture','occlusionTexture','emissiveTexture','extensions'): material.pop(field,None)
            pbr=material.get('pbrMetallicRoughness',{})
            pbr.pop('baseColorTexture',None); pbr.pop('metallicRoughnessTexture',None)
        for field in ('images','textures','samplers'): part.pop(field,None)
        name=key.replace('.','_'); path=root/'Saved/ArtExport'/('npc_'+name+'.glb'); write_glb(path,part,data)
        objects=import_one(path,'SK_'+name,dest+'/Modules/'+name)
        imported=[o for o in objects if isinstance(o,u.SkeletalMesh)]
        if len(imported)!=1: raise RuntimeError('Expected one module '+key)
        mesh=imported[0]
        if not u.LHNpcVisualComponent.share_player_rig(mesh,player,None): raise RuntimeError('Player bind mismatch '+key)
        low=[min(100*doc['accessors'][p['attributes']['POSITION']]['min'][g] for p in meshdoc['primitives']) for g in (0,2,1)]
        high=[max(100*doc['accessors'][p['attributes']['POSITION']]['max'][g] for p in meshdoc['primitives']) for g in (0,2,1)]
        box=mesh.get_bounds(); a=box.origin-box.box_extent; b=box.origin+box.box_extent
        actual=[(a.x,a.y,a.z),(b.x,b.y,b.z)]; error=max(abs(x-y) for row,want in zip(actual,(low,high)) for x,y in zip(row,want))
        evidence[key]=dict(source_cm=[low,high],actual_cm=actual,max_error_cm=error)
        if error>.5: raise RuntimeError('NPC bounds mismatch '+key+': '+str(error))
        slots=mesh.get_editor_property('materials')
        for slot in slots: slot.material_interface=masters[False]
        mesh.set_editor_property('materials',slots)
        if not u.LHPlayerVisualComponent.compact_imported_mesh(mesh): raise RuntimeError('Cannot compact '+key)
        meshes[key]=mesh; keep.add(mesh.get_path_name().split('.')[0])
        if not index:
            for action in ('idle','talk'):
                found=[o for o in objects if isinstance(o,u.AnimSequence) and action in o.get_name().lower()]
                if len(found)!=1 or not u.LHNpcVisualComponent.share_player_rig(mesh,player,found[0]): raise RuntimeError('Missing compatible '+action)
                clips[body,action]=found[0]; keep.add(found[0].get_path_name().split('.')[0])
(root/'Saved/ArtExport/npc-import-bounds.json').write_text(json.dumps(evidence,indent=2)+'\n')
appearance_class=u.load_class(None,'/Script/Lighthaven.LHNpcAppearance')
if not appearance_class: raise RuntimeError('NPC appearance class is not registered; build the editor target')
for row in roster:
    name=row['id'][4:]; built=next(n for n in manifest['npcs'] if n['id']==row['id'])
    colors=dict(Cloth=row['cloth'],Accent=row['accent'],Trim=row['trim'],Skin=row['skin'],Hair=row['hair'],Lip=row['lip'],Leather='4D3020',Edge='785333',Stitch='A68C61',Metal='92734A',Iron='646C6C',Wood='67452E',Paper='D6C69B',Crease='36251E',Eye='B0A08A',Iris='504031',Pupil='120F0D',Pants='4E4836')
    mats={}
    for semantic,hexcolor in colors.items():
        cloth=semantic in ('Cloth','Accent','Trim','Pants'); matname='MI_'+hexcolor+'_'+semantic
        mat=load(dest+'/Materials/'+matname) or tools.create_asset(matname,dest+'/Materials',u.MaterialInstanceConstant,u.MaterialInstanceConstantFactoryNew())
        editor.set_material_instance_parent(mat,masters[cloth])
        def linear(v):
            v=v/255; return v/12.92 if v<=.04045 else ((v+.055)/1.055)**2.4
        editor.set_material_instance_vector_parameter_value(mat,'Tint',u.LinearColor(*(linear(int(hexcolor[i:i+2],16)) for i in (0,2,4)),1))
        editor.set_material_instance_scalar_parameter_value(mat,'Roughness',.86 if cloth else .42 if semantic in ('Metal','Iron') else .64)
        editor.set_material_instance_scalar_parameter_value(mat,'Metallic',.7 if semantic in ('Metal','Iron') else 0)
        mats[semantic]=mat; keep.add(mat.get_path_name().split('.')[0])
    factory=u.DataAssetFactory(); factory.set_editor_property('data_asset_class',appearance_class)
    appearance=load(dest+'/DA_'+name) or tools.create_asset('DA_'+name,dest,appearance_class,factory)
    selected=[meshes[k] for k in built['mesh_keys']]; assigned=[]
    for mesh in selected:
        for slot in mesh.get_editor_property('materials'):
            slotname=str(slot.get_editor_property('imported_material_slot_name'))
            matches=[s for s in mats if slotname==s or slotname.endswith('.'+s) or slotname.endswith('_'+s)]
            if len(matches)!=1: raise RuntimeError('Unknown semantic '+slotname)
            semantic=matches[0]
            assigned.append(mats[semantic])
    low=[min(evidence[k]['source_cm'][0][i] for k in built['mesh_keys'])*row['scale'] for i in range(3)]
    high=[max(evidence[k]['source_cm'][1][i] for k in built['mesh_keys'])*row['scale'] for i in range(3)]
    for key,value in dict(identity=row['id'],stature=row['scale'],SourceMin=u.Vector(*low),SourceMax=u.Vector(*high),meshes=selected,materials=assigned,idle=clips[row['body'],'idle'],gesture=clips[row['body'],'talk']).items(): appearance.set_editor_property(key,value)
    keep.add(appearance.get_path_name().split('.')[0])
# Remove all automatic textures/materials/skeletons, only after rebinding real references.
if not assets.save_directory(dest,only_if_is_dirty=False,recursive=True): raise RuntimeError('Cannot save NPC assets')
discard=[load(path) for path in assets.list_assets(dest,recursive=True,include_folder=False) if path.split('.')[0] not in keep]
discard=[obj for obj in discard if obj]
if discard and not assets.delete_loaded_assets(discard): raise RuntimeError('Cannot remove automatic NPC import duplicates')
if not assets.save_directory(dest,only_if_is_dirty=False,recursive=True): raise RuntimeError('Cannot persist NPC assets')
# Compact after dependency saves, as in the player pipeline; saving an initial
# import can rebuild its editor cache. Preserve the real MeshDescription.
for mesh in meshes.values():
    # Reimport is source-scripted. Drop Interchange's editor-only node/pipeline
    # snapshot, retaining the complete MeshDescription needed for DDC rebuilds.
    mesh.set_editor_property('asset_import_data',None)
    if not u.LHPlayerVisualComponent.compact_imported_mesh(mesh): raise RuntimeError('Cannot compact persisted NPC mesh')
    if not assets.save_loaded_asset(mesh,only_if_is_dirty=False): raise RuntimeError('Cannot save compact NPC mesh')
files=inventory(); total=sum(p['bytes'] for p in files.values())
receipt=dict(signature=signature,assets=files,total_bytes=total,bounds=evidence)
u.log('NPC_BYTES '+str(total))
if not files or total>8_000_000: raise RuntimeError('NPC 8 MB budget exceeded: '+str(total))
stamp.write_text(json.dumps(receipt,indent=2)+'\n')
