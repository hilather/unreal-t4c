"""Headless static props import, validated upstream; no gameplay mutation."""
import copy,json,os,struct,sys
from pathlib import Path
import unreal as u
root=Path(u.Paths.project_dir()).resolve()
sys.dont_write_bytecode=True
sys.path.insert(0,str(root/'Source/Lighthaven/Visual/Props'))
from props_receipt import signature,inventory
source=root/'artsource/props/output'; content=root/'Content/Lighthaven/Art/Props'; dest='/Game/Lighthaven/Art/Props'
engine=json.loads((Path(os.environ['UE_ROOT'])/'Engine/Build/Build.version').read_text())
sig=signature(root,engine); stamp=content/'import-receipt.json'
if stamp.exists():
    old=json.loads(stamp.read_text())
    if old['signature']==sig and old['assets'] and old['assets']==inventory(content):
        u.log('PROPS_IMPORT_UNCHANGED'); raise SystemExit(0)
    stamp.unlink()
for p in content.rglob('*.uasset'): p.chmod(p.stat().st_mode|0o200)
assets=u.EditorAssetLibrary; tools=u.AssetToolsHelpers.get_asset_tools(); editor=u.MaterialEditingLibrary
manifest=json.loads((source/'manifest.json').read_text()); keep=set()
def load(path): return assets.load_asset(path) if assets.does_asset_exist(path) else None
def remember(obj): keep.add(obj.get_path_name().split('.')[0]); return obj
def import_one(path,name,folder):
    task=u.AssetImportTask()
    for k,v in dict(filename=str(path),destination_path=folder,destination_name=name,automated=True,replace_existing=True,save=False).items(): task.set_editor_property(k,v)
    tools.import_asset_tasks([task]); return task.get_objects()
def normalized(obj,name,folder):
    path=folder+'/'+name
    if obj.get_path_name().split('.')[0]!=path and not assets.rename_asset(obj.get_path_name(),path): raise RuntimeError('Rename '+path)
    return remember(obj)
def material(name):
    obj=load(dest+'/Materials/'+name) or tools.create_asset(name,dest+'/Materials',u.Material,u.MaterialFactoryNew())
    obj.set_editor_property('two_sided',True); return remember(obj)
def expression(mat,cls,key):
    # Loaded material expressions can be rooted during commandlet startup.
    # Reuse named nodes instead of deleting rooted expressions (UE 5.8 assertion).
    candidates=[n for n in editor.get_material_expressions(mat) if isinstance(n,cls)]
    node=next((n for n in candidates if n.get_editor_property('desc')==key),None)
    if node is None: node=next((n for n in candidates if not n.get_editor_property('desc')),None)
    if node is None: node=editor.create_material_expression(mat,cls)
    node.set_editor_property('desc',key)
    return node

def constant(mat,value,prop):
    node=expression(mat,u.MaterialExpressionConstant,str(prop)); node.set_editor_property('r',value); editor.connect_material_property(node,'',prop)
textures={}
for channel in ('base','normal','orm'):
    tex=next(o for o in import_one(source/'textures'/('props_'+channel+'.png'),'T_'+channel,dest+'/Textures') if isinstance(o,u.Texture2D))
    normalized(tex,'T_'+channel,dest+'/Textures'); tex.set_editor_property('srgb',channel=='base')
    tex.set_editor_property('compression_settings',u.TextureCompressionSettings.TC_NORMALMAP if channel=='normal' else u.TextureCompressionSettings.TC_MASKS if channel=='orm' else u.TextureCompressionSettings.TC_DEFAULT)
    if channel=='normal': tex.set_editor_property('flip_green_channel',True)
    textures[channel]=tex
atlas=material('M_WornAtlas')
for channel,tex in textures.items():
    node=expression(atlas,u.MaterialExpressionTextureSample,channel); node.set_editor_property('texture',tex)
    if channel=='normal': node.set_editor_property('sampler_type',u.MaterialSamplerType.SAMPLERTYPE_NORMAL)
    if channel=='orm': node.set_editor_property('sampler_type',u.MaterialSamplerType.SAMPLERTYPE_MASKS)
    for pin,prop in ([('RGB',u.MaterialProperty.MP_BASE_COLOR)] if channel=='base' else [('RGB',u.MaterialProperty.MP_NORMAL)] if channel=='normal' else [('R',u.MaterialProperty.MP_AMBIENT_OCCLUSION),('G',u.MaterialProperty.MP_ROUGHNESS),('B',u.MaterialProperty.MP_METALLIC)]): editor.connect_material_property(node,pin,prop)
editor.recompile_material(atlas)
materials={'Props.WornAtlas':atlas}; meshes={}; evidence={}
for row in manifest['assets']:
    raw=(source/row['file']).read_bytes(); length=struct.unpack_from('<I',raw,12)[0]; doc=json.loads(raw[20:20+length]); binary=bytearray(raw[28+length:])
    # All upstream roots are identity. Combine primitive lists without recentering.
    primitives=[copy.deepcopy(p) for n in doc['nodes'] if 'mesh' in n for p in doc['meshes'][n['mesh']]['primitives']]
    seen=set()
    for p in primitives:
        for semantic in ('POSITION','NORMAL','TANGENT'):
            idx=p['attributes'].get(semantic)
            if idx is None or idx in seen: continue
            seen.add(idx); a=doc['accessors'][idx]; v=doc['bufferViews'][a['bufferView']]; width=4 if semantic=='TANGENT' else 3
            start=v.get('byteOffset',0)+a.get('byteOffset',0); stride=v.get('byteStride',width*4)
            for i in range(a['count']):
                for c in ((2,3) if semantic=='TANGENT' else (2,)):
                    offset=start+i*stride+c*4; struct.pack_into('<f',binary,offset,-struct.unpack_from('<f',binary,offset)[0])
            if 'min' in a: a['min'][2],a['max'][2]=-a['max'][2],-a['min'][2]
        idx=p['indices']
        if idx in seen: continue
        seen.add(idx); a=doc['accessors'][idx]; v=doc['bufferViews'][a['bufferView']]; fmt={5121:'B',5123:'H',5125:'I'}[a['componentType']]; size=struct.calcsize(fmt); start=v.get('byteOffset',0)+a.get('byteOffset',0)
        for i in range(0,a['count'],3):
            a1=start+(i+1)*size; a2=start+(i+2)*size
            binary[a1:a1+size],binary[a2:a2+size]=binary[a2:a2+size],binary[a1:a1+size]
    assigned=[]
    for info in doc['materials']:
        key=info['name']
        if key not in materials:
            mat=material('M_'+key.replace('.','_')); pbr=info.get('pbrMetallicRoughness',{}); color=pbr.get('baseColorFactor',[1,1,1,1]); alpha=color[3]
            mat.set_editor_property('blend_mode',u.BlendMode.BLEND_TRANSLUCENT if info.get('alphaMode')=='BLEND' else u.BlendMode.BLEND_OPAQUE)
            if info.get('alphaMode')=='BLEND':
                mat.set_editor_property('blend_mode',u.BlendMode.BLEND_TRANSLUCENT)
                opacity=expression(mat,u.MaterialExpressionConstant,'opacity'); opacity.set_editor_property('r',alpha)
                fade=expression(mat,u.MaterialExpressionScalarParameter,'presentation-fade'); fade.set_editor_property('parameter_name','PresentationFade'); fade.set_editor_property('default_value',1.0)
                product=expression(mat,u.MaterialExpressionMultiply,'faded-opacity')
                editor.connect_material_expressions(opacity,'',product,'A'); editor.connect_material_expressions(fade,'',product,'B'); editor.connect_material_property(product,'',u.MaterialProperty.MP_OPACITY)
            node=expression(mat,u.MaterialExpressionConstant3Vector,"base"); node.set_editor_property('constant',u.LinearColor(*color[:3],1)); editor.connect_material_property(node,'',u.MaterialProperty.MP_BASE_COLOR)
            emission=info.get('emissiveFactor',[0,0,0]); strength=info.get('extensions',{}).get('KHR_materials_emissive_strength',{}).get('emissiveStrength',1)
            glow=expression(mat,u.MaterialExpressionConstant3Vector,"emission"); glow.set_editor_property('constant',u.LinearColor(*(x*strength for x in emission),1)); editor.connect_material_property(glow,'',u.MaterialProperty.MP_EMISSIVE_COLOR)
            constant(mat,pbr.get('roughnessFactor',.72),u.MaterialProperty.MP_ROUGHNESS); editor.recompile_material(mat); materials[key]=mat
        assigned.append(materials[key])
    low=[min(doc['accessors'][p['attributes']['POSITION']]['min'][g]*100 for p in primitives) for g in (0,2,1)]
    high=[max(doc['accessors'][p['attributes']['POSITION']]['max'][g]*100 for p in primitives) for g in (0,2,1)]
    used=sorted({p.get('material',0) for p in primitives})
    for p in primitives: p['material']=used.index(p.get('material',0))
    doc['meshes']=[dict(name=Path(row['file']).stem,primitives=primitives)]; doc['nodes']=[dict(mesh=0,name=Path(row['file']).stem)]; doc['scenes']=[dict(nodes=[0])]; doc['scene']=0
    doc['materials']=[dict(name=doc['materials'][i]['name']) for i in used]
    for field in ('images','textures','samplers','extensionsUsed','extensionsRequired'): doc.pop(field,None)
    encoded=json.dumps(doc,separators=(',',':')).encode(); encoded+=b' '*(-len(encoded)%4)
    path=root/'Saved/ArtExport'/('props_'+Path(row['file']).name)
    payload=struct.pack('<II',len(encoded),0x4e4f534a)+encoded+struct.pack('<II',len(binary),0x004e4942)+binary
    path.write_bytes(struct.pack('<III',0x46546c67,2,12+len(payload))+payload)
    name='SM_'+Path(row['file']).stem
    imported=[o for o in import_one(path,name,dest+'/Meshes') if isinstance(o,u.StaticMesh)]
    if len(imported)!=1: raise RuntimeError('Expected one combined mesh '+name)
    mesh=normalized(imported[0],name,dest+'/Meshes')
    for i,index in enumerate(used): mesh.set_material(i,assigned[index])
    mesh.set_editor_property('has_navigation_data',False)
    body=mesh.get_editor_property('body_setup')
    if body:
        instance=body.get_editor_property('default_instance')
        instance.set_editor_property('collision_profile_name','NoCollision')
        instance.set_editor_property('collision_enabled',u.CollisionEnabled.NO_COLLISION)
        body.set_editor_property('default_instance',instance)
    box=mesh.get_bounds(); a=box.origin-box.box_extent; b=box.origin+box.box_extent; actual=[(a.x,a.y,a.z),(b.x,b.y,b.z)]
    error=max(abs(x-y) for got,want in zip(actual,(low,high)) for x,y in zip(got,want))
    evidence[row['id']]=dict(source_cm=[low,high],actual_cm=actual,max_error_cm=error)
    if error>.5: raise RuntimeError('Bounds '+name+' '+str(error))
    mesh.set_editor_property('asset_import_data',None)
    # Item keys are full IDs; other props use documented filename stems.
    key=row['id'] if row['id'].startswith('Item.') else Path(row['file']).stem
    meshes[key]=mesh
icons={}
for row in manifest['icons']:
    name='T_'+row['id'].replace('.','_'); tex=next(o for o in import_one(source/row['file'],name,dest+'/Icons') if isinstance(o,u.Texture2D))
    normalized(tex,name,dest+'/Icons')
    for k,v in dict(srgb=True,never_stream=True,compression_no_alpha=False,lod_group=u.TextureGroup.TEXTUREGROUP_UI,compression_settings=u.TextureCompressionSettings.TC_EDITOR_ICON,mip_gen_settings=u.TextureMipGenSettings.TMGS_NO_MIPMAPS).items(): tex.set_editor_property(k,v)
    icons[row['id']]=tex
cls=u.load_class(None,'/Script/Lighthaven.LHPropsAssets')
if not cls: raise RuntimeError('Build editor before importing Props')
factory=u.DataAssetFactory(); factory.set_editor_property('data_asset_class',cls)
catalog=load(dest+'/DA_Props') or tools.create_asset('DA_Props',dest,cls,factory)
catalog.set_editor_property('meshes',meshes); catalog.set_editor_property('icons',icons); remember(catalog)
if not assets.save_directory(dest,only_if_is_dirty=False,recursive=True): raise RuntimeError('Save props')
discard=[load(p) for p in assets.list_assets(dest,recursive=True,include_folder=False) if p.split('.')[0] not in keep]
if discard and not assets.delete_loaded_assets([o for o in discard if o]): raise RuntimeError('Remove automatic duplicates')
if not assets.save_directory(dest,only_if_is_dirty=False,recursive=True): raise RuntimeError('Save props dependencies')
files=inventory(content); by_type={}
for path,info in files.items():
    kind=assets.load_asset(dest+'/'+path[:-7]).get_class().get_name(); by_type[kind]=by_type.get(kind,0)+info['bytes']
total=sum(v['bytes'] for v in files.values()); u.log('PROPS_BYTES_BY_TYPE '+json.dumps(by_type,sort_keys=True))
(root/'Saved/ArtExport/props-size-summary.json').write_text(json.dumps(dict(by_type=by_type,total_bytes=total,bounds=evidence),indent=2)+'\n')
if not files or total>6_000_000: raise RuntimeError('Props 6 MB imported budget exceeded: '+str(total))
stamp.write_text(json.dumps(dict(signature=sig,assets=files,total_bytes=total,by_type=by_type,bounds=evidence),indent=2)+'\n')
