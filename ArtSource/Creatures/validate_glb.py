"""Inspect actual GLB container data independently of Blender scene state."""
import json, struct
from pathlib import Path
out=Path(__file__).resolve().parent/'output'
results=[]
for kind in ('rat','bat','slime'):
    p=out/(kind+'.glb');blob=p.read_bytes()
    magic,version,total=struct.unpack_from('<III',blob)
    assert magic==0x46546c67 and version==2 and total==len(blob)
    length,chunk=struct.unpack_from('<II',blob,12);assert chunk==0x4e4f534a
    d=json.loads(blob[20:20+length]);binary=blob[28+length:]
    def accessor(index):
        a=d['accessors'][index];v=d['bufferViews'][a['bufferView']];count={'SCALAR':1,'VEC2':2,'VEC3':3,'VEC4':4,'MAT4':16}[a['type']]
        fmt,size={5126:('f',4),5123:('H',2),5121:('B',1)}[a['componentType']]
        offset=v.get('byteOffset',0)+a.get('byteOffset',0);stride=v.get('byteStride',count*size)
        rows=[struct.unpack_from('<'+fmt*count,binary,offset+i*stride) for i in range(a['count'])]
        if a.get('normalized'):rows=[tuple(x/(65535 if size==2 else 255) for x in row) for row in rows]
        return rows
    assert d['skins'] and len(d['animations'])==5
    names={a['name'] for a in d['animations']};assert names=={'idle','move','attack','hit','death'},names
    triangles=sum(d['accessors'][q['indices']]['count']//3 for m in d['meshes'] for q in m['primitives'])
    assert 3000<=triangles<=8000,(kind,triangles)
    for m in d['meshes']:
        for q in m['primitives']:
            assert {'JOINTS_0','WEIGHTS_0'} <= q['attributes'].keys()
            assert all(abs(sum(row)-1)<.001 for row in accessor(q['attributes']['WEIGHTS_0']))
    root=next(i for i,n in enumerate(d['nodes']) if n.get('name')=='root')
    for action in d['animations']:
        moving=False
        for channel in action['channels']:
            sam=action['samplers'][channel['sampler']];values=accessor(sam['output'])
            varies=any(any(abs(a-b)>1e-6 for a,b in zip(values[0],row)) for row in values)
            if channel['target']['node']==root:assert not varies,(kind,'root drift')
            moving=moving or varies
        assert moving,(kind,action['name'],'static clip')
    durations={a['name']:max(d['accessors'][s['input']]['max'][0] for s in a['samplers']) for a in d['animations']}
    for name,expected in [('idle',2),('move',.8),('attack',1.4),('hit',.6),('death',2)]:assert abs(durations[name]-expected)<1e-5,(kind,durations)
    dims=[]
    for im in d['images']:
        view=d['bufferViews'][im['bufferView']];data=binary[view.get('byteOffset',0):view.get('byteOffset',0)+view['byteLength']]
        assert data[:8]==b'\x89PNG\r\n\x1a\n';wh=struct.unpack_from('>II',data,16);assert wh==(512,512),wh;dims.append(wh)
    assert len(dims)==3,dims
    results.append(dict(creature=kind,triangles=triangles,animation_seconds=durations,embedded_png_dimensions=dims,skin_joints=len(d['skins'][0]['joints']),bytes=len(blob)))
print(json.dumps(results,indent=2))
