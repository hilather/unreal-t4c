"""Independent prototype W5-08d export-pose and deterministic construction audit."""
import argparse
import hashlib
import json
import sys
from pathlib import Path
import bpy

parser=argparse.ArgumentParser()
parser.add_argument('--source',type=Path,default=Path(__file__).resolve().parent)
parser.add_argument('--construction-only',action='store_true')
args=parser.parse_args(sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else [])
sys.path.insert(0,str(args.source.resolve()))
import models

KINDS=('rat','bat','slime')
LIMITS={'rat':((-.675,-.14,-.0007),(.225,.14,.35)),
        'bat':((-.225,-.4,-.0007),(.225,.4,1.45)),
        'slime':((-.45,-.45,-.0007),(.45,.45,.6))}

def clear():
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)

def signature(kind):
    clear()
    parts,bones=models.create(kind)
    geometry=[]
    for obj in parts:
        groups={g.index:g.name for g in obj.vertex_groups}
        geometry.append({'vertices':[tuple(v.co) for v in obj.data.vertices],
                         'edges':[tuple(e.vertices) for e in obj.data.edges],
                         'polygons':[tuple(p.vertices) for p in obj.data.polygons],
                         'matrix':[tuple(row) for row in obj.matrix_world],
                         'weights':[sorted((groups[g.group],g.weight) for g in v.groups)
                                    for v in obj.data.vertices]})
    payload=json.dumps({'geometry':geometry,'bones':bones},sort_keys=True,separators=(',',':'))
    return hashlib.sha256(payload.encode()).hexdigest()

construction={}
for kind in KINDS:
    first,second=signature(kind),signature(kind)
    assert first==second,(kind,'construction signature mismatch',first,second)
    construction[kind]={'sha256':first,'identical_repeats':2}
if args.construction_only:
    print(json.dumps({'construction':construction},indent=2))
else:
    report={'construction':construction,'bounds_unit':'metres','creatures':{},'violations':[]}
    for kind in KINDS:
        clear();bpy.context.scene.render.fps=30
        before=set(bpy.data.actions)
        bpy.ops.import_scene.gltf(filepath=str(args.source/'output'/(kind+'.glb')))
        rig=next(o for o in bpy.context.scene.objects if o.type=='ARMATURE')
        for track in rig.animation_data.nla_tracks:track.mute=True
        imported=set(bpy.data.actions)-before
        actions={clip:next(a for a in imported if a.name.split('.')[0]==clip)
                 for clip in ('idle','move','attack','hit','death')}
        meshes=[o for o in bpy.context.scene.objects if o.type=='MESH'
                and any(m.type=='ARMATURE' and m.object==rig for m in o.modifiers)]
        assert meshes,(kind,'missing skinned meshes')
        clips={};low,high=LIMITS[kind]
        for clip,action in actions.items():
            rig.animation_data.action=action
            rig.animation_data.action_slot=action.slots[0]
            start,end=round(action.frame_range[0]),round(action.frame_range[1])
            minimum=[float('inf')]*3;maximum=[-float('inf')]*3
            frame_bounds=[]
            for frame in range(start,end+1):
                bpy.context.scene.frame_set(frame);bpy.context.view_layer.update()
                deps=bpy.context.evaluated_depsgraph_get()
                lo=[float('inf')]*3;hi=[-float('inf')]*3
                for obj in meshes:
                    evaluated=obj.evaluated_get(deps)
                    for vertex in evaluated.data.vertices:
                        position=evaluated.matrix_world@vertex.co
                        for axis in range(3):
                            lo[axis]=min(lo[axis],position[axis]);hi[axis]=max(hi[axis],position[axis])
                for axis in range(3):
                    minimum[axis]=min(minimum[axis],lo[axis]);maximum[axis]=max(maximum[axis],hi[axis])
                    # Match source live-envelope tolerance on X/Y and ceiling;
                    # floor tolerance is explicitly -.0007m without extra padding.
                    permitted_low=low[axis]-(.0005 if axis<2 else 0)
                    permitted_high=high[axis]+.0005
                    if lo[axis]<permitted_low or hi[axis]>permitted_high:
                        report['violations'].append({'creature':kind,'clip':clip,'frame':frame,
                            'axis':axis,'minimum':lo[axis],'maximum':hi[axis],
                            'permitted_minimum':permitted_low,'permitted_maximum':permitted_high})
                frame_bounds.append({'frame':frame,'min':lo,'max':hi})
            clips[clip]={'min':minimum,'max':maximum,'frames_checked':len(frame_bounds)}
        report['creatures'][kind]={'limits':{'min':low,'max':high},'actions':clips}
    report['passed']=not report['violations']
    destination=args.source/'output'/'export-pose-audit.json'
    destination.write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps({'report':str(destination),'passed':report['passed'],'violations':len(report['violations'])}))
    assert report['passed'],'Exported poses exceed envelopes; inspect export-pose-audit.json'
