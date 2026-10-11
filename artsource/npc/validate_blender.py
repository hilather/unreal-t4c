"""Audit actual NPC GLBs: blender --background --factory-startup --threads 8 --python validate_blender.py."""
import json
import math
from pathlib import Path
import sys
sys.dont_write_bytecode = True
import bpy

HERE=Path(__file__).resolve().parent
sys.path.insert(0,str(HERE))
from validate_glb import PARENTS, roster_ids


def evaluate(meshes):
    bpy.context.view_layer.update()
    depsgraph=bpy.context.evaluated_depsgraph_get()
    points=[]
    for mesh in meshes:
        obj=mesh.evaluated_get(depsgraph)
        points.extend(tuple(obj.matrix_world@v.co) for v in obj.data.vertices)
    assert points and all(math.isfinite(x) for p in points for x in p)
    return points,[min(p[a] for p in points) for a in range(3)],[max(p[a] for p in points) for a in range(3)]


def validate(entry,output):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.context.scene.render.fps=30
    bpy.ops.import_scene.gltf(filepath=str(output/entry['file']))
    rigs=[o for o in bpy.context.scene.objects if o.type=='ARMATURE']
    assert len(rigs)==1
    rig=rigs[0]
    assert set(rig.data.bones.keys())==set(PARENTS)
    for name,parent in PARENTS.items():
        bone=rig.data.bones[name]
        assert (bone.parent.name if bone.parent else None)==parent
    meshes=[o for o in bpy.context.scene.objects if o.type=='MESH' and any(m.type=='ARMATURE' and m.object==rig for m in o.modifiers)]
    assert len(meshes)>=3
    actions={}
    for action in bpy.data.actions:
        name=action.name.rsplit('|',1)[-1].split('.')[0]
        if name in ('idle','talk'):
            assert name not in actions
            actions[name]=action
    assert set(actions)=={'idle','talk'}
    rig.animation_data_create()
    for track in rig.animation_data.nla_tracks:
        track.mute=True
    result={'id':entry['id'],'evaluated_meshes':[m.name for m in meshes],'actions':{}}
    baseline_root=None
    baseline_feet=None
    scale=entry.get('scale',1)
    for name,action in actions.items():
        rig.animation_data.action=action
        if getattr(action,'slots',None):
            rig.animation_data.action_slot=action.slots[0]
        assert abs(action.frame_range[0])<1e-4 and abs(action.frame_range[1]-60)<1e-4
        samples=[]
        first=None
        maximum_motion=0
        maximum_feet_motion=0
        for frame in range(61):
            bpy.context.scene.frame_set(frame)
            points,lower,upper=evaluate(meshes)
            root=rig.matrix_world@rig.pose.bones['root'].matrix
            matrix=[float(x) for row in root for x in row]
            feet=[tuple(rig.matrix_world@rig.pose.bones['foot.'+s].head) for s in ('L','R')]
            if baseline_root is None:
                baseline_root=matrix
                baseline_feet=feet
            assert max(abs(a-b) for a,b in zip(matrix,baseline_root))<1e-6, (name,frame,'root drift')
            assert abs(root.translation.z)<1e-6
            foot_delta=max(abs(a-b) for foot,base in zip(feet,baseline_feet) for a,b in zip(foot,base))
            maximum_feet_motion=max(maximum_feet_motion,foot_delta)
            assert foot_delta<1e-5, (name,frame,'feet drift',foot_delta)
            assert lower[2]>=-.025 and upper[2]<=2.5, (name,frame,lower,upper)
            assert upper[2]-lower[2]>1.35*scale, (name,frame,'upright height')
            if first is None:
                first=points
                assert abs(lower[2]-.008*scale)<.025, 'rest feet floor clearance'
            delta=max(abs(a-b) for p,q in zip(first,points) for a,b in zip(p,q))
            maximum_motion=max(maximum_motion,delta)
            samples.append({'frame':frame,'min':lower,'max':upper})
        assert delta<1e-5, (name,'evaluated loop endpoints',delta)
        assert maximum_motion>1e-4, (name,'static evaluated mesh')
        result['actions'][name]={'samples':samples,'loop_max_delta_m':delta,'maximum_motion_m':maximum_motion,'maximum_foot_head_displacement_m':maximum_feet_motion}
    # Feet project toward +X in the authored coordinate convention.
    for side in ('L','R'):
        bone=rig.data.bones['foot.'+side]
        assert (rig.matrix_world@bone.tail_local).x>(rig.matrix_world@bone.head_local).x, '+X forward foot orientation'
    result['root_world_matrix']=baseline_root
    result['success']=True
    return result


def main():
    output=HERE/'output'
    if '--' in sys.argv:
        arguments=sys.argv[sys.argv.index('--')+1:]
        if arguments:
            output=Path(arguments[0]).resolve()
    entries=json.loads((output/'manifest.json').read_text())['npcs']
    report={'unit':'metres','fps':30,'assets':[],'exceptions':[],'limitations':['Samples integer frames only; does not prove absence of self-intersections.'],'limits_note':'Prototype art validation limits; no gameplay rules.'}
    assert sorted(e['id'] for e in entries)==roster_ids()
    for entry in entries:
        try:
            report['assets'].append(validate(entry,output))
        except Exception as error:
            report['exceptions'].append(entry['id']+': '+repr(error))
    report['success']=not report['exceptions'] and len(report['assets'])==len(entries)
    path=output/'blender-validation.json'
    path.write_text(json.dumps(report,indent=2)+'\n')
    print('NPC pose audit:',path,'PASS' if report['success'] else 'FAIL')
    assert report['success'],report['exceptions']

if __name__=='__main__':
    main()
