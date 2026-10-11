"""Headless player import. Invoked by build/build-art.sh; writes only local art."""
import hashlib, json, os, struct, copy
from pathlib import Path
import unreal as u
root=Path(u.Paths.project_dir()).resolve()
source=root/'artsource/player/output'
content=root/'Content/Lighthaven/Art/Player'
dest='/Game/Lighthaven/Art/Player'
tools=u.AssetToolsHelpers.get_asset_tools()
assets=u.EditorAssetLibrary
editor=u.MaterialEditingLibrary
kinds=('player_a','player_b')
signature=hashlib.sha256(Path(__file__).read_bytes()+u.SystemLibrary.get_engine_version().encode())
for path in [root/'artsource/player/appearances.json',root/'Source/Lighthaven/Visual/Player/LHPlayerVisual.cpp',root/'Binaries/Linux/libUnrealEditor-Lighthaven.so']+[source/(kind+suffix) for kind in kinds for suffix in ('.glb','_base.png','_normal.png','_orm.png')]:
    signature.update(path.name.encode()); signature.update(path.read_bytes())
signature=signature.hexdigest()
stamp=content/'import-receipt.json'
def inventory():
    return {str(p.relative_to(content)):dict(bytes=p.stat().st_size,sha256=hashlib.sha256(p.read_bytes()).hexdigest()) for p in sorted(content.rglob('*.uasset'))}
if stamp.exists():
    old=json.loads(stamp.read_text())
    if old['signature']==signature and old['assets']==inventory():
        u.log('PLAYER_IMPORT_UNCHANGED'); raise SystemExit(0)
locked=[p for p in content.rglob('*.uasset') if not p.stat().st_mode & 0o200]
if locked and os.environ.get('LH_ART_REIMPORT')!='1': raise RuntimeError('Changed player inputs require LH_ART_REIMPORT=1')
for p in locked: p.chmod(p.stat().st_mode | 0o200)
def import_one(path,name,folder):
    task=u.AssetImportTask()
    for key,value in dict(filename=str(path),destination_path=folder,destination_name=name,automated=True,replace_existing=True,save=False).items(): task.set_editor_property(key,value)
    tools.import_asset_tasks([task])
    objects=task.get_objects()
    if not objects: raise RuntimeError('Empty import '+str(path))
    return objects

def write_glb(path,doc,data):
    encoded=json.dumps(doc,separators=(',',':')).encode(); encoded+=b' '*((-len(encoded))%4)
    payload=struct.pack('<II',len(encoded),0x4e4f534a)+encoded+struct.pack('<II',len(data),0x004e4942)+data
    path.write_bytes(struct.pack('<III',0x46546c67,2,12+len(payload))+payload)

def split_parts(path):
    raw=path.read_bytes(); size=struct.unpack_from('<I',raw,12)[0]; doc=json.loads(raw[20:20+size]); offset=20+size
    length,tag=struct.unpack_from('<II',raw,offset); data=raw[offset+8:offset+8+length]
    result=[]
    for label,node_name in [('body','body'),('Cropped','Hair.Cropped'),('Tied','Hair.Tied')]:
        nodes=[n for n in doc['nodes'] if n.get('name')==node_name and 'mesh' in n]
        assert len(nodes)==1, (node_name,[n.get('name') for n in doc['nodes'] if 'mesh' in n])
        node=nodes[0]; assert set(node)<= {'name','mesh','skin'}, 'Mesh transforms need explicit bounds handling'
        mesh=doc['meshes'][node['mesh']]
        low=[float('inf')]*3; high=[-float('inf')]*3
        for primitive in mesh['primitives']:
            acc=doc['accessors'][primitive['attributes']['POSITION']]
            # Canonical glTF is already reflected; Unreal maps X,Y,Z -> X,Z,Y.
            for c,g in enumerate((0,2,1)):
                low[c]=min(low[c],100*acc['min'][g]); high[c]=max(high[c],100*acc['max'][g])
        part=copy.deepcopy(doc); part['meshes']=[copy.deepcopy(mesh)]
        for n in part['nodes']:
            if n.get('name')==node_name and 'mesh' in n: n['mesh']=0
            elif 'mesh' in n: n.pop('mesh'); n.pop('skin',None)
        if label!='body': part.pop('animations',None)
        output=path.with_name(path.stem+'-'+label+'.glb'); write_glb(output,part,data)
        result.append((label,output,low,high))
    return result
def canonical_player(kind):
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
    normalized=root/'Saved/ArtExport/unreal-player'
    normalized.mkdir(parents=True,exist_ok=True)
    path=normalized/(kind+'.glb')
    path.write_bytes(struct.pack('<III',0x46546c67,2,12+len(payload))+payload)
    return path

master=assets.load_asset(dest+'/M_Player') if assets.does_asset_exist(dest+'/M_Player') else None
bounds_evidence={}
for kind in kinds:
    folder=dest+'/'+kind
    textures={}
    for channel in ('base','normal','orm'):
        tex=next(o for o in import_one(source/(kind+'_'+channel+'.png'),'T_'+channel,folder) if isinstance(o,u.Texture2D))
        tex.set_editor_property('srgb',channel=='base'); tex.set_editor_property('compression_no_alpha',True)
        tex.set_editor_property('compression_settings',u.TextureCompressionSettings.TC_NORMALMAP if channel=='normal' else u.TextureCompressionSettings.TC_MASKS if channel=='orm' else u.TextureCompressionSettings.TC_DEFAULT)
        if channel=='normal': tex.set_editor_property('flip_green_channel',True)
        textures[channel]=tex
    if master is None:
        master=tools.create_asset('M_Player',dest,u.Material,u.MaterialFactoryNew())
        for channel in ('base','normal','orm'):
            node=editor.create_material_expression(master,u.MaterialExpressionTextureSampleParameter2D)
            node.set_editor_property('parameter_name',channel); node.set_editor_property('texture',textures[channel])
            if channel=='base':
                tint=editor.create_material_expression(master,u.MaterialExpressionVectorParameter); tint.set_editor_property('parameter_name','Tint'); tint.set_editor_property('default_value',u.LinearColor(1,1,1,1))
                mult=editor.create_material_expression(master,u.MaterialExpressionMultiply)
                editor.connect_material_expressions(node,'RGB',mult,'A'); editor.connect_material_expressions(tint,'RGB',mult,'B'); editor.connect_material_property(mult,'',u.MaterialProperty.MP_BASE_COLOR)
            elif channel=='normal':
                node.set_editor_property('sampler_type',u.MaterialSamplerType.SAMPLERTYPE_NORMAL); editor.connect_material_property(node,'RGB',u.MaterialProperty.MP_NORMAL)
            else:
                node.set_editor_property('sampler_type',u.MaterialSamplerType.SAMPLERTYPE_MASKS)
                for c,prop in [('R',u.MaterialProperty.MP_AMBIENT_OCCLUSION),('G',u.MaterialProperty.MP_ROUGHNESS),('B',u.MaterialProperty.MP_METALLIC)]: editor.connect_material_property(node,c,prop)
        master.set_editor_property('used_with_skeletal_mesh',True); editor.recompile_material(master)
    if not assets.save_loaded_asset(master): raise RuntimeError('Cannot persist player master')
    materials={}
    for name,rgb in [('Clothing',(255,255,255)),('LightWarm',(189,142,114)),('MediumWarm',(139,90,64)),('DeepWarm',(81,54,44))]:
        path=folder+'/MI_'+name
        mat=assets.load_asset(path) if assets.does_asset_exist(path) else tools.create_asset('MI_'+name,folder,u.MaterialInstanceConstant,u.MaterialInstanceConstantFactoryNew())
        editor.set_material_instance_parent(mat,master)
        for channel,tex in textures.items(): editor.set_material_instance_texture_parameter_value(mat,channel,tex)
        def linear(v):
            v=v/255; return v/12.92 if v<=.04045 else ((v+.055)/1.055)**2.4
        editor.set_material_instance_vector_parameter_value(mat,'Tint',u.LinearColor(*(linear(v) for v in rgb),1))
        materials[name]=mat
    keep={o.get_path_name().split('.')[0] for o in [*textures.values(),*materials.values()]}
    skeleton=None
    meshes=[]
    for label,path,low,high in split_parts(canonical_player(kind)):
        objects=import_one(path,'SK_'+label,folder+'/'+label)
        imported=[o for o in objects if isinstance(o,u.SkeletalMesh)]
        if len(imported)!=1: raise RuntimeError('Expected one player mesh '+label+': '+str(len(imported)))
        mesh=imported[0]
        probe=u.new_object(u.SkeletalMeshComponent); probe.set_skeletal_mesh_asset(mesh)
        u.log('PLAYER_BONES '+str([str(probe.get_bone_name(i)) for i in range(probe.get_num_bones())]))
        u.log('PLAYER_SLOTS '+str([str(slot.material_slot_name) for slot in mesh.get_editor_property('materials')]))
        if not assets.save_directory(folder,only_if_is_dirty=True,recursive=True): raise RuntimeError('Cannot save import dependencies')
        wanted=folder+'/'+label+'/'+kind+'-'+label+'/SkeletalMeshes/SK_'+label
        if mesh.get_path_name().split('.')[0]!=wanted: raise RuntimeError('Unexpected stable player mesh path '+mesh.get_path_name())
        box=mesh.get_bounds(); actual_low=box.origin-box.box_extent; actual_high=box.origin+box.box_extent
        actual=[(actual_low.x,actual_low.y,actual_low.z),(actual_high.x,actual_high.y,actual_high.z)]
        error=max(abs(a-b) for row,want in zip(actual,(low,high)) for a,b in zip(row,want))
        bounds_evidence[kind+'/'+label]=dict(actual_cm=actual,source_cm=[low,high],max_error_cm=error)
        u.log('PLAYER_BOUNDS '+kind+'/'+label+' '+str(error))
        if error>.5: raise RuntimeError('Player bounds mismatch '+kind+'/'+label+' '+str(bounds_evidence[kind+'/'+label]))
        if label=='body':
            skeleton=mesh.get_editor_property('skeleton')
            clips=[o for o in objects if isinstance(o,u.AnimSequence)]
            for action in ('idle','move','run','melee','bow','cast','hit','death'):
                matches=[a for a in clips if action in a.get_name().lower()]
                if len(matches)!=1 or matches[0].get_editor_property('skeleton')!=skeleton: raise RuntimeError('Missing compatible action '+action)
                wanted=folder+'/body/'+kind+'-body/SkeletalMeshes/SK_body'+action
                if matches[0].get_path_name().split('.')[0]!=wanted: raise RuntimeError('Unexpected stable action path '+matches[0].get_path_name())
        # Hair retains its imported skeleton: all three parts come from the
        # same body-specific bind matrices. Runtime leader-pose maps by bone
        # name; UE 5.8 exposes Skeleton as a read-only Python property.
        if not u.LHPlayerVisualComponent.configure_imported_mesh(mesh,materials['Clothing'],materials['LightWarm']): raise RuntimeError('Cannot bind player materials/sockets')
        meshes.append(mesh)
    # Drop automatic glTF materials and duplicate embedded textures, retaining our shared atlas.
    generated=assets.list_assets(folder,recursive=True,include_folder=False)
    generated.sort(key=lambda p:(0 if isinstance(assets.load_asset(p),u.MaterialInterface) else 1,p))
    for path in generated:
        obj=assets.load_asset(path)
        if isinstance(obj,(u.MaterialInterface,u.Texture2D)) and path.split('.')[0] not in keep:
            if not assets.delete_asset(path): raise RuntimeError('Cannot remove duplicate '+path)
    if not assets.save_directory(folder,only_if_is_dirty=True,recursive=True): raise RuntimeError('Cannot save player')
    for mesh in meshes:
        if not u.LHPlayerVisualComponent.compact_imported_mesh(mesh): raise RuntimeError('Cannot compact player')
        if not assets.save_loaded_asset(mesh,only_if_is_dirty=False): raise RuntimeError('Cannot persist compact player')
assets.save_loaded_asset(master)
files=inventory(); by_type={}
for path,info in files.items():
    obj=assets.load_asset(dest+'/'+path[:-7]); cls=obj.get_class().get_name(); by_type[cls]=by_type.get(cls,0)+info['bytes']
total=sum(info['bytes'] for info in files.values())
receipt=dict(signature=signature,assets=files,total_bytes=total,by_type=by_type,bounds=bounds_evidence)
(root/'Saved/ArtExport/player-import-candidate.json').write_text(json.dumps(receipt,indent=2)+'\n')
u.log('PLAYER_BYTES '+json.dumps(by_type,sort_keys=True)+' TOTAL '+str(total))
if total>10_000_000: raise RuntimeError('Player 10 MB budget exceeded: '+str(total))
stamp.write_text(json.dumps(receipt,indent=2)+'\n')
