"""Independent standard-library validation of W5-14 style exports."""
import argparse
import json
import struct
from pathlib import Path
from validate_env import require,artifact,verify_file,png_size,glb,CANONICAL_BOUNDS,validate as validate_b1,outward_masonry

BASE={'Wall400','Floor400','Arch240','Arch320','Stair600x120','Stair600x120Descending','Sconce','Barrel','Crate','Table','Bench','Debris'}
EXTRA={'Church':{'Altar','Pillar','DoorLeafPreview','Carpet'},'B2Damp':set(),'B3Crypt':set(),'B4Ritual':{'Altar','ArchBoss500'}}
BOUNDS={'Altar':[[-100,-80,0],[100,80,120]],'Pillar':[[-30,-30,0],[30,30,400]],'DoorLeafPreview':[[0,-5,0],[160,5,300]],'ArchBoss500':[[-300,-20,0],[300,0,400]],'Carpet':[[-200,-1050,0],[200,1050,1]]}

def check_shell_winding(data):
    length=struct.unpack_from('<I',data,12)[0]
    doc=json.loads(data[20:20+length]); binary=data[28+length:]
    def decode(index):
        a=doc['accessors'][index]; view=doc['bufferViews'][a['bufferView']]
        component={5121:'B',5123:'H',5125:'I',5126:'f'}[a['componentType']]
        count={'SCALAR':1,'VEC3':3}[a['type']]; fmt='<'+component*count
        start=view.get('byteOffset',0)+a.get('byteOffset',0)
        stride=view.get('byteStride',struct.calcsize(fmt))
        return [struct.unpack_from(fmt,binary,start+i*stride) for i in range(a['count'])]
    for mesh in doc['meshes']:
        for primitive in mesh['primitives']:
            vertices=decode(primitive['attributes']['POSITION'])
            indices=[row[0] for row in decode(primitive['indices'])]
            outward_masonry(vertices,indices)

def validate(root):
    root=Path(root); manifest_path=artifact(root,'manifest.json'); m=json.loads(manifest_path.read_text()); style=m['style']
    if style=='B1Cellar': return validate_b1(root)[1]
    require(style in EXTRA,'Unknown style')
    expected=BASE|EXTRA[style]
    require({e['file'].removesuffix('.glb') for e in m['pieces'].values()}==expected,'Style inventory mismatch')
    referenced=set(); mesh_bytes=0
    for key,e in m['pieces'].items():
        name=e['file'].removesuffix('.glb'); descending=name.endswith('Descending')
        pid=('Presentation.Environment.Church.' if name in ('Pillar','DoorLeafPreview') else 'Presentation.Environment.Basement.' if name=='ArchBoss500' else 'Presentation.Environment.Shared.')+name.replace('Descending','')
        if name=='Carpet': require(e['piece_id'] is None and key=='auxiliary:Carpet' and e['binding_status']=='auxiliary_unbound','Carpet must remain unbound')
        else: require(e['piece_id']==pid and key==pid+(':descending' if descending else ''),'Registry ID mismatch')
        require(e['variant']==('descending' if descending else 'default'),'Variant mismatch')
        contract=BOUNDS.get(name,CANONICAL_BOUNDS.get(name.replace('Descending',':descending')))
        require(contract is not None and all(abs(a-b)<.002 for row,wanted in zip(e['bounds_cm'],contract) for a,b in zip(row,wanted)),'Frozen bounds mismatch: '+name)
        data=verify_file(root,e['file'],e); referenced.update(glb(data,e)); mesh_bytes+=len(data)
        if name in {'Floor400','Wall400','Altar','Pillar','DoorLeafPreview','Carpet','ArchBoss500'}:
            try: check_shell_winding(data)
            except ValueError as error: raise ValueError(name+': inward/degenerate shell') from error
    require(referenced==set(m['textures']),'Texture inventory mismatch')
    texture_bytes=0
    for name,e in m['textures'].items():
        data=verify_file(root,name,e); require(png_size(data)==e['resolution']==[512,512],'512px PNG required'); texture_bytes+=len(data)
    require(m['blender_roundtrip_checked']==len(expected),'Incomplete importer roundtrip')
    require(m['bake']['engine']=='Cycles' and m['bake']['device']=='CPU','CPU bake contract')
    require(mesh_bytes==m['glb_bytes'] and texture_bytes==m['texture_bytes'] and mesh_bytes+texture_bytes==m['payload_bytes'],'Byte totals mismatch')
    actual={str(p.relative_to(root)) for p in root.rglob('*.glb')}|{str(p.relative_to(root)) for p in (root/'textures').rglob('*') if p.is_file()}
    require(actual=={e['file'] for e in m['pieces'].values()}|referenced,'Unmanifested artifacts')
    total=mesh_bytes+texture_bytes+manifest_path.stat().st_size; require(total<=4_000_000,f'{style} exceeds 4MB: {total}')
    return total

def main():
    parser=argparse.ArgumentParser(description=__doc__); parser.add_argument('output',type=Path,nargs='?',default=Path('Saved/ArtExport/env')); args=parser.parse_args()
    roots=[args.output] if (args.output/'manifest.json').exists() else sorted(p for p in args.output.iterdir() if p.is_dir() and (p/'manifest.json').exists())
    require(bool(roots),'No style manifests')
    for root in roots: print(f'{root.name}: valid, {validate(root):,} bytes including manifest')

if __name__=='__main__': main()
