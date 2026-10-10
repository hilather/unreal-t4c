"""Original procedural creature pilot. Blender 5.2, no external asset inputs.
Run: blender -b --factory-startup --python artsource/creatures/build.py
"""
import bpy, math, json
from pathlib import Path
from mathutils import Vector, Quaternion
import sys, argparse
from array import array
sys.path.insert(0,str(Path(__file__).resolve().parent))
import models
OUT=Path(__file__).resolve().parent/'output'; OUT.mkdir(exist_ok=True)
RESULT=[]
def material(name, color, rough=.7, scale=70):
    m=bpy.data.materials.new(name); m.use_nodes=True
    n=m.node_tree.nodes; l=m.node_tree.links; p=n.get('Principled BSDF')
    p.inputs['Roughness'].default_value=rough
    noise=n.new('ShaderNodeTexNoise'); noise.inputs['Scale'].default_value=scale
    noise.inputs['Detail'].default_value=3
    ramp=n.new('ShaderNodeValToRGB')
    ramp.color_ramp.elements[0].color=(*[v*.22 for v in color],1)
    ramp.color_ramp.elements[1].color=(*color,1)
    l.new(noise.outputs['Fac'],ramp.inputs[0]); l.new(ramp.outputs[0],p.inputs['Base Color'])
    bump=n.new('ShaderNodeBump'); bump.inputs['Strength'].default_value=.22; bump.inputs['Distance'].default_value=.001
    l.new(noise.outputs['Fac'],bump.inputs['Height']); l.new(bump.outputs[0],p.inputs['Normal'])
    return m
def translate_world(bone, delta):
    bone.location=bone.bone.matrix_local.to_3x3().inverted()@Vector(delta)

def rotate_world(bone, axis, angle):
    local_axis=bone.bone.matrix_local.to_3x3().inverted()@Vector(axis)
    bone.rotation_euler=Quaternion(local_axis.normalized(),angle).to_euler('XYZ')

def build(kind):
    bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
    parts,bones=models.create(kind)
    bpy.ops.object.select_all(action='DESELECT')
    for o in parts:o.select_set(True)
    bpy.context.view_layer.objects.active=parts[0];bpy.ops.object.join();mesh=bpy.context.object;mesh.name=kind
    bpy.ops.object.transform_apply(location=True,rotation=True,scale=True)
    bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.mesh.remove_doubles(threshold=.00001);bpy.ops.uv.smart_project(island_margin=.0005,margin_method='FRACTION');bpy.ops.object.mode_set(mode='OBJECT')
    tri=mesh.modifiers.new('Triangles','TRIANGULATE');bpy.ops.object.modifier_apply(modifier=tri.name)
    triangles=len(mesh.data.polygons)
    assert 3000 <= triangles <= 8000,(kind,'triangle budget exceeded',triangles)
    positions=[mesh.matrix_world@v.co for v in mesh.data.vertices]
    rest_min=[min(v[j] for v in positions) for j in range(3)];rest_max=[max(v[j] for v in positions) for j in range(3)]
    limits={'rat':((-.675,-.14,0),(.225,.14,.25)), 'bat':((-.225,-.4,0),(.225,.4,1.45)), 'slime':((-.45,-.45,0),(.45,.45,.4))}
    low,high=limits[kind]
    assert all(rest_min[j]>=low[j]-.0001 and rest_max[j]<=high[j]+.0001 for j in range(3)),(kind,'rest envelope exceeded',rest_min,rest_max)

    mesh['rest_min_cm']=[round(v*100,3) for v in rest_min];mesh['rest_max_cm']=[round(v*100,3) for v in rest_max]
    # Bake the generated shading into an atlas; no source-image pixels are used.
    scene=bpy.context.scene;scene.render.fps=30;scene.render.engine='CYCLES';scene.cycles.device='CPU';scene.cycles.samples=8
    images={}
    for channel in ['base','normal','orm']:
        im=bpy.data.images.new(kind+'_'+channel,width=1024,height=1024,alpha=False)
        if channel!='base':im.colorspace_settings.name='Non-Color'
        for m in mesh.data.materials:
            n=m.node_tree.nodes; node=n.new('ShaderNodeTexImage');node.image=im;n.active=node
        if channel=='base':
            scene.render.bake.use_pass_direct=False;scene.render.bake.use_pass_indirect=False;scene.render.bake.use_pass_color=True
            bpy.ops.object.bake(type='DIFFUSE',margin=2)
        elif channel=='normal':bpy.ops.object.bake(type='NORMAL',margin=2)
        else:
            # Geometric AO is evaluated against all joined surfaces in metres.
            for m in mesh.data.materials:
                n=m.node_tree.nodes;p=n.get('Principled BSDF');out=n.get('Material Output');links=m.node_tree.links
                ao=n.new('ShaderNodeAmbientOcclusion');ao.inputs['Distance'].default_value=.065;ao.samples=32
                combine=n.new('ShaderNodeCombineColor');combine.mode='RGB'
                links.new(ao.outputs['AO'],combine.inputs[0])
                rough=p.inputs['Roughness']
                if rough.is_linked:links.new(rough.links[0].from_socket,combine.inputs[1])
                else:combine.inputs[1].default_value=rough.default_value
                combine.inputs[2].default_value=0
                e=n.new('ShaderNodeEmission');links.new(combine.outputs[0],e.inputs[0]);links.new(e.outputs[0],out.inputs[0])
            bpy.ops.object.bake(type='EMIT',margin=2)
        if channel=='orm':
            pixels=array('f',[0.0])*(1024*1024*4);im.pixels.foreach_get(pixels)
            ao=[pixels[i] for i in range(0,len(pixels),4) if pixels[i+1]>.01]
            rough=[pixels[i+1] for i in range(0,len(pixels),4) if pixels[i+1]>.01]
            mesh['ao_range']=[min(ao),max(ao)];mesh['roughness_range']=[min(rough),max(rough)]
        im.filepath_raw=str(OUT/(kind+'_'+channel+'.png'));im.file_format='PNG';im.save();images[channel]=im
    baked=bpy.data.materials.new(kind+'_baked');baked.use_nodes=True;baked.use_backface_culling=False;n=baked.node_tree.nodes;l=baked.node_tree.links;p=n.get('Principled BSDF')
    for ch in images:
        node=n.new('ShaderNodeTexImage');node.image=images[ch]
        if ch=='base':l.new(node.outputs['Color'],p.inputs['Base Color'])
        elif ch=='normal':
            norm=n.new('ShaderNodeNormalMap');l.new(node.outputs['Color'],norm.inputs[1]);l.new(norm.outputs[0],p.inputs['Normal'])
        else:
            sep=n.new('ShaderNodeSeparateColor');l.new(node.outputs['Color'],sep.inputs[0]);l.new(sep.outputs[1],p.inputs['Roughness']);l.new(sep.outputs[2],p.inputs['Metallic'])
            group=bpy.data.node_groups.get('glTF Material Output')
            if group is None:
                from io_scene_gltf2.blender.com.material_helpers import create_settings_group
                group=create_settings_group('glTF Material Output')
            occlusion=n.new('ShaderNodeGroup');occlusion.node_tree=group;l.new(sep.outputs[0],occlusion.inputs['Occlusion'])
    mesh.data.materials.clear();mesh.data.materials.append(baked)
    rigdata=bpy.data.armatures.new(kind+'_skeleton');rig=bpy.data.objects.new(kind+'_rig',rigdata);bpy.context.collection.objects.link(rig)
    bpy.context.view_layer.objects.active=rig;mesh.select_set(False);rig.select_set(True);bpy.ops.object.mode_set(mode='EDIT')
    for name,(head,tail,parent) in bones.items():
        b=rigdata.edit_bones.new(name);b.head=head;b.tail=tail
        if parent:b.parent=rigdata.edit_bones[parent]
    bpy.ops.object.mode_set(mode='OBJECT');mesh.parent=rig;mod=mesh.modifiers.new('Skin','ARMATURE');mod.object=rig
    actions={}
    for clip,end in [('idle',60),('move',24),('attack',42),('hit',18),('death',60)]:
        rig.animation_data_create();rig.animation_data.action=bpy.data.actions.new(kind+'_'+clip);actions[clip]=rig.animation_data.action
        frames=[0,end//4,end//2,3*end//4,end] if clip not in ['attack','death'] else ([0,20,29,30,42] if clip=='attack' else list(range(31))+[45,60])
        death_hold=None
        for f in frames:
            scene.frame_set(f)
            for b in rig.pose.bones:b.rotation_mode='XYZ';b.rotation_euler=(0,0,0);b.location=(0,0,0);b.scale=(1,1,1)
            body=rig.pose.bones['body'];phase=math.sin(math.tau*f/end)
            if clip in ['idle','move']:
                body.scale=(1,1,1+(.025 if clip=='idle' else .07)*phase)
                if kind=='rat':
                    body.scale=(1,1,1+.015*(1-math.cos(math.tau*f/end)))
                    body.location.z=.1*(body.scale.z-1)
                for b in rig.pose.bones:
                    if b.name.startswith(('front','rear')) and clip=='move':
                        step=phase*(1 if ('front' in b.name)==('-1' in b.name) else -1)
                        translate_world(b,(.015*step,0,.012*max(0,step)))
                    if b.name.startswith('wing') and not b.name.startswith('wingtip'):rotate_world(b,(1,0,0),(.25 if clip=='move' else .12)*(-1 if '-1' in b.name else 1)*(1 if f==end//2 else -1))
            elif clip=='attack':
                a={0:0,20:-.12,29:-.12,30:.16,42:0}[f]
                if kind=='slime':body.location.x=a*.35;body.scale=(1+a,1,1-a)
                else:
                    rotate_world(rig.pose.bones['head'],(0,1,0),-a)
                    if kind=='rat' and f==30:translate_world(rig.pose.bones['head'],(-.009,0,0))
                for b in rig.pose.bones:
                    if b.name.startswith('wing') and not b.name.startswith('wingtip'):rotate_world(b,(1,0,0),a*2*(-1 if '-1' in b.name else 1))
            elif clip=='hit':
                if kind=='slime':body.scale.z=.85 if f==end//2 else 1
                else:body.rotation_euler.z=(.08 if kind=='bat' else .10) if f==end//2 else 0
            elif clip=='death':
                t=min(1,f/30);ease=t*t*(3-2*t)
                if kind=='slime':body.scale=(1,1,1-.88*ease)
                elif kind=='bat':body.location.z=-.96*ease;body.scale=(1,1-.4*ease,1-.4*ease)
                else:
                    t=min(1,f/18);roll=t*t*(3-2*t)
                    body.rotation_euler.x=math.pi/2*roll;body.scale=(1,1-.4*roll,1-.3*roll)
            for b in rig.pose.bones:
                if b.name.startswith('tail') and clip in ['idle','move']:
                    rotate_world(b,(0,0,1),.025*phase)
                if b.name=='jaw' and clip=='attack':rotate_world(b,(0,1,0),.32 if f==30 else 0)
                if b.name.startswith('wingtip') and clip in ['idle','move']:
                    rotate_world(b,(1,0,0),.12*phase*(-1 if '-1' in b.name else 1))
            if clip=='death' and death_hold is not None and f>=45:
                for b in rig.pose.bones:
                    b.location,b.rotation_euler,b.scale=[value.copy() for value in death_hold[b.name]]
            elif clip=='death' and kind in ['rat','bat'] and f>0:
                # Every vertex is descended from body. Land the authored roll/fall
                # by measuring the evaluated sole, never by clamping vertices.
                bpy.context.view_layer.update()
                ev=mesh.evaluated_get(bpy.context.evaluated_depsgraph_get())
                minimum=min((ev.matrix_world@v.co).z for v in ev.data.vertices)
                if kind=='rat' or minimum<.001 or f==30:
                    delta=body.bone.matrix_local.to_3x3().inverted()@Vector((0,0,.001-minimum))
                    body.location+=delta
            if clip=='death' and f==30:
                death_hold={b.name:(b.location.copy(),b.rotation_euler.copy(),b.scale.copy()) for b in rig.pose.bones}
            for b in rig.pose.bones:
                for prop in ['location','rotation_euler','scale']:b.keyframe_insert(data_path=prop,frame=f,group=b.name)
        # Linear samples keep authored envelopes and the dead hold exact;
        # automatic Bezier handles can overshoot a flat final key interval.
        for layer in actions[clip].layers:
            for strip in layer.strips:
                for bag in strip.channelbags:
                    for curve in bag.fcurves:
                        for key in curve.keyframe_points:key.interpolation='LINEAR'
        track=rig.animation_data.nla_tracks.new();track.name=clip;strip=track.strips.new(clip,0,actions[clip]);track.mute=True
    rig.animation_data.action=None
    bpy.ops.object.select_all(action='DESELECT');mesh.select_set(True);rig.select_set(True)
    # Exporter's NLA mode must see unmuted tracks.
    for t in rig.animation_data.nla_tracks:t.mute=False
    bpy.ops.export_scene.gltf(filepath=str(OUT/(kind+'.glb')),export_format='GLB',use_selection=True,export_animations=True,export_animation_mode='NLA_TRACKS',export_force_sampling=True)
    for t in rig.animation_data.nla_tracks:t.mute=True
    scene.render.fps=30
    return mesh,rig,actions,triangles

def aim(o,target):o.rotation_euler=(Vector(target)-o.location).to_track_quat('-Z','Y').to_euler()
def stage(kind=None):
    s=bpy.context.scene;s.render.resolution_x=1280;s.render.resolution_y=720;s.render.resolution_percentage=100;s.cycles.samples=12;s.cycles.use_denoising=True
    s.world.color=(.018,.018,.018)
    s.view_settings.look='AgX - Medium High Contrast'
    bpy.ops.mesh.primitive_plane_add(size=200);floor=bpy.context.object;floor.name='Studio floor';floor.data.materials.append(material('Slate',(.014,.017,.020),.9,8))
    for pos,power,size,color in [((2,-3,4),450,4,(1,.78,.55)),((-2,2,3),600,3,(.60,.78,1)),((-1,-1,2),120,2,(1,1,1))]:
        if kind=='slime':size*=.3
        bpy.ops.object.light_add(type='AREA',location=pos);o=bpy.context.object;o.data.energy=power;o.data.shape='DISK';o.data.size=size;o.data.color=color;aim(o,(0,0,.3))
    bpy.ops.object.camera_add();cam=bpy.context.object;s.camera=cam;return cam

def render(name):
    bpy.context.scene.render.filepath=str(OUT/(name+'.png'));bpy.ops.render.render(write_still=True)
def previews(kind, mesh, rig, actions):
    cam=stage(kind);target=(-.10,0,.13) if kind=='rat' else (0,0,1.075 if kind=='bat' else .15)
    rig.animation_data.action=actions['idle'];rig.animation_data.action_slot=actions['idle'].slots[0];bpy.context.scene.frame_set(0)
    cam.data.type='ORTHO';cam.data.ortho_scale=1.12 if kind=='bat' else 1.25
    cam.location=(2.5,-.55,target[2]+.525) if kind=='bat' else (1.4,-1.8,target[2]+1.0)
    aim(cam,target);render(kind+'_hero')
    cam.data.type='PERSP';cam.data.angle=math.radians(45);cam.location=(4.867,-4.867,10.73);aim(cam,(0,0,.9));render(kind+'_gameplay')
    # Five true skinned copies sampled at representative action keys, in one sheet.
    rig.animation_data.action=None;rig.hide_render=True;mesh.hide_render=True
    for i,(clip,frame) in enumerate([('idle',15),('move',6),('attack',30),('hit',9),('death',60)]):
        r=rig.copy();r.data=rig.data.copy();bpy.context.collection.objects.link(r);r.hide_render=False;r.animation_data_clear();r.animation_data_create();r.animation_data.action=actions[clip];r.animation_data.action_slot=actions[clip].slots[0]
        o=mesh.copy();o.data=mesh.data.copy();bpy.context.collection.objects.link(o);o.hide_render=False;o.parent=r;next(mod for mod in o.modifiers if mod.type=='ARMATURE').object=r
        # Evaluate each source action at its own frame, freeze into pose transforms.
        bpy.context.scene.frame_set(frame);transforms={b.name:(b.location.copy(),b.rotation_euler.copy(),b.scale.copy()) for b in r.pose.bones};r.animation_data_clear()
        for b in r.pose.bones:b.location,b.rotation_euler,b.scale=transforms[b.name]
        bpy.context.view_layer.update()
        # Bake this representative pose to geometry before later frame changes.
        deps=bpy.context.evaluated_depsgraph_get();posed=bpy.data.meshes.new_from_object(o.evaluated_get(deps));o.modifiers.clear();o.data=posed
        r.location=(0,(i-2)*1.18,0)
        if kind=='rat':r.rotation_mode='XYZ';r.rotation_euler.z-=.65
        bpy.ops.object.text_add(location=(.6,(i-2)*1.18-.22,.005));txt=bpy.context.object;txt.data.body=clip;txt.data.size=.11;txt.rotation_euler.z=math.pi/2
    cam.data.type='ORTHO';cam.data.ortho_scale=6.2;cam.location=(7,0,5);aim(cam,(0,0,.5));render(kind+'_actions')

def main(kinds=('rat','bat','slime'), do_render=True):
    for kind in kinds:
        mesh,rig,actions,tris=build(kind)
        bounds={}
        for clip,act in actions.items():
            rig.animation_data.action=act;rig.animation_data.action_slot=act.slots[0];lo=[1e9]*3;hi=[-1e9]*3
            for f in range(int(act.frame_range[0]),int(act.frame_range[1])+1):
                bpy.context.scene.frame_set(f);ev=mesh.evaluated_get(bpy.context.evaluated_depsgraph_get())
                for v in ev.data.vertices:
                    co=ev.matrix_world@v.co
                    for j in range(3):lo[j]=min(lo[j],co[j]);hi[j]=max(hi[j],co[j])
            live_limits={'rat':((-.675,-.14,-.0002),(.225,.14,.35)),'bat':((-.225,-.4,-.0002),(.225,.4,1.45)),'slime':((-.45,-.45,-.0002),(.45,.45,.6))}
            low,high=live_limits[kind]
            assert all(lo[j]>=low[j]-.0005 and hi[j]<=high[j]+.0005 for j in range(3)),(kind,clip,'animated envelope exceeded',lo,hi)
            bounds[clip]={'min_cm':[round(x*100,2) for x in lo],'max_cm':[round(x*100,2) for x in hi]}
        if do_render:previews(kind,mesh,rig,actions)
        rest_bounds={'min_cm':list(mesh['rest_min_cm']),'max_cm':list(mesh['rest_max_cm'])}
        baked_ranges={'ao':list(mesh['ao_range']),'roughness':list(mesh['roughness_range'])}
        # Validate the actual file in a clean scene.
        bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
        before=set(bpy.data.actions);bpy.ops.import_scene.gltf(filepath=str(OUT/(kind+'.glb')))
        imported=[o for o in bpy.context.scene.objects if o.type=='ARMATURE'];names=sorted(a.name for a in set(bpy.data.actions)-before)
        assert imported,kind+' missing armature';assert len(names)>=5,(kind,names)
        skinned=[o for o in bpy.context.scene.objects if o.type=='MESH' and any(m.type=='ARMATURE' for m in o.modifiers)]
        assert skinned,kind+' missing skin'
        RESULT.append(dict(creature=kind,triangles=tris,bones=len(imported[0].data.bones),texture_size=1024,rest_bounds=rest_bounds,baked_ranges=baked_ranges,bake={'ao':'Cycles geometric AO shader','ao_distance_m':.065,'ao_samples':32,'uv_margin':.0005,'uv_margin_method':'FRACTION','bake_margin_px':2,'channels':'AO/roughness/metal=0'},animation_bounds=bounds,imported_actions=names,glb_bytes=(OUT/(kind+'.glb')).stat().st_size))
        previous=json.loads((OUT/'validation.json').read_text()) if (OUT/'validation.json').exists() else []
        merged={r['creature']:r for r in previous};merged.update({r['creature']:r for r in RESULT})
        (OUT/'validation.json').write_text(json.dumps(list(merged.values()),indent=2))

if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--only',choices=['rat','bat','slime']);parser.add_argument('--no-render',action='store_true');parser.add_argument('--render',action='store_true')
    args=parser.parse_args(sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else [])
    main((args.only,) if args.only else ('rat','bat','slime'),not args.no_render)
