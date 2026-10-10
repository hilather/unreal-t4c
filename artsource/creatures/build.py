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
import roster
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

def export_asset(kind,mesh,rig):
    """Keep carried equipment removable without duplicating textures or rigs."""
    import bmesh
    weapon_groups={'balork':{'Weapon_Main'},'goblin':{'Weapon_R'},'goblin_warrior':{'Weapon_R'}}
    copies=[]
    if kind in weapon_groups:
        groups={g.index:g.name for g in mesh.vertex_groups}
        weapon={v.index for v in mesh.data.vertices
                if any(groups[g.group] in weapon_groups[kind] and g.weight>.999 for g in v.groups)}
        assert weapon,(kind,'missing removable weapon vertices')
        for name,keep in [('body',set(range(len(mesh.data.vertices)))-weapon),('weapon',weapon)]:
            obj=mesh.copy();obj.data=mesh.data.copy();obj.name=kind+'_'+name
            bpy.context.collection.objects.link(obj)
            bm=bmesh.new();bm.from_mesh(obj.data);bm.verts.ensure_lookup_table()
            bmesh.ops.delete(bm,geom=[v for v in bm.verts if v.index not in keep],context='VERTS')
            bm.to_mesh(obj.data);bm.free();obj.data.update();copies.append(obj)
        assert sum(len(o.data.polygons) for o in copies)==len(mesh.data.polygons),(kind,'weapon split lost faces')
    bpy.ops.object.select_all(action='DESELECT');rig.select_set(True)
    for obj in copies or [mesh]:obj.select_set(True)
    bpy.context.view_layer.objects.active=rig
    bpy.ops.export_scene.gltf(filepath=str(OUT/(kind+'.glb')),export_format='GLB',use_selection=True,export_animations=True,export_animation_mode='NLA_TRACKS',export_force_sampling=True)
    for obj in copies:
        data=obj.data;bpy.data.objects.remove(obj,do_unlink=True);bpy.data.meshes.remove(data)
    mesh.select_set(True);bpy.context.view_layer.objects.active=mesh


def build(kind, bake_assets=True):
    bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
    parts,bones=roster.create(kind)
    bpy.ops.object.select_all(action='DESELECT')
    for o in parts:o.select_set(True)
    bpy.context.view_layer.objects.active=parts[0];bpy.ops.object.join();mesh=bpy.context.object;mesh.name=kind
    bpy.ops.object.transform_apply(location=True,rotation=True,scale=True)
    bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.mesh.remove_doubles(threshold=.00001);bpy.ops.uv.smart_project(island_margin=.0005,margin_method='FRACTION');bpy.ops.object.mode_set(mode='OBJECT')
    tri=mesh.modifiers.new('Triangles','TRIANGULATE');bpy.ops.object.modifier_apply(modifier=tri.name)
    triangles=len(mesh.data.polygons)
    assert 3000 <= triangles <= roster.budget(kind),(kind,'triangle budget exceeded',triangles)
    positions=[mesh.matrix_world@v.co for v in mesh.data.vertices]
    rest_min=[min(v[j] for v in positions) for j in range(3)];rest_max=[max(v[j] for v in positions) for j in range(3)]
    low,high=roster.limits(kind)
    assert all(rest_min[j]>=low[j]-.0001 and rest_max[j]<=high[j]+.0001 for j in range(3)),(kind,'rest envelope exceeded',rest_min,rest_max)

    mesh['rest_min_cm']=[round(v*100,3) for v in rest_min];mesh['rest_max_cm']=[round(v*100,3) for v in rest_max]
    if bake_assets:
        # Bake the generated shading into an atlas; no source-image pixels are used.
        scene=bpy.context.scene;scene.render.fps=30;scene.render.engine='CYCLES';scene.cycles.device='CPU';scene.cycles.samples=8
        images={}
        for channel in ['base','normal','orm']:
            im=bpy.data.images.new(kind+'_'+channel,width=1024,height=1024,alpha=False)
            if channel!='base':im.colorspace_settings.name='Non-Color'
            for m in mesh.data.materials:
                n=m.node_tree.nodes; node=n.new('ShaderNodeTexImage');node.image=im;n.active=node
            if channel=='base':
                # Pure base pigment, including metallic weapon surfaces. A diffuse
                # BSDF bake can blacken metals; emission here adds no baked light.
                restore=[]
                for mat in mesh.data.materials:
                    nodes=mat.node_tree.nodes;links=mat.node_tree.links
                    principled=nodes.get('Principled BSDF');out=nodes.get('Material Output')
                    previous=out.inputs['Surface'].links[0].from_socket
                    emission=nodes.new('ShaderNodeEmission');base=principled.inputs['Base Color']
                    if base.is_linked:links.new(base.links[0].from_socket,emission.inputs['Color'])
                    else:emission.inputs['Color'].default_value=base.default_value
                    links.new(emission.outputs[0],out.inputs['Surface'])
                    restore.append((mat,previous,out,emission))
                bpy.ops.object.bake(type='EMIT',margin=2)
                for mat,previous,out,emission in restore:
                    mat.node_tree.links.new(previous,out.inputs['Surface']);mat.node_tree.nodes.remove(emission)
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
                    metal=p.inputs['Metallic']
                    if metal.is_linked:links.new(metal.links[0].from_socket,combine.inputs[2])
                    else:combine.inputs[2].default_value=metal.default_value
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
    scene=bpy.context.scene;scene.render.fps=30
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
        if kind in roster.BATCH2:frames=list(range(end+1))
        death_hold=None
        for f in frames:
            scene.frame_set(f)
            for b in rig.pose.bones:b.rotation_mode='XYZ';b.rotation_euler=(0,0,0);b.location=(0,0,0);b.scale=(1,1,1)
            body=rig.pose.bones['body'];phase=math.sin(math.tau*f/end)
            if kind in roster.BATCH2:
                roster.module(kind).pose(kind,rig,clip,f,end)
            elif clip in ['idle','move']:
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
            for b in ([] if kind in roster.BATCH2 else rig.pose.bones):
                if b.name.startswith('tail') and clip in ['idle','move']:
                    rotate_world(b,(0,0,1),.025*phase)
                if b.name=='jaw' and clip=='attack':rotate_world(b,(0,1,0),.32 if f==30 else 0)
                if b.name.startswith('wingtip') and clip in ['idle','move']:
                    rotate_world(b,(1,0,0),.12*phase*(-1 if '-1' in b.name else 1))
            if clip=='death' and death_hold is not None and f>=45:
                for b in rig.pose.bones:
                    b.location,b.rotation_euler,b.scale=[value.copy() for value in death_hold[b.name]]
            elif clip=='death' and (kind in ['rat','bat'] or kind in roster.BATCH2) and f>0:
                # Every vertex is descended from body. Land the authored roll/fall
                # by measuring the evaluated sole, never by clamping vertices.
                bpy.context.view_layer.update()
                ev=mesh.evaluated_get(bpy.context.evaluated_depsgraph_get())
                minimum=min((ev.matrix_world@v.co).z for v in ev.data.vertices)
                if kind=='rat' or kind in roster.BATCH2 or minimum<.001 or f==30:
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
    if bake_assets:export_asset(kind,mesh,rig)
    for t in rig.animation_data.nla_tracks:t.mute=True
    scene.render.fps=30
    return mesh,rig,actions,triangles

def aim(o,target):o.rotation_euler=(Vector(target)-o.location).to_track_quat('-Z','Y').to_euler()
def gameplay_floor_material():
    """Prototype visual W5-08d: metre-scale slate slabs, procedural only."""
    name='Gameplay slate W5-08d'
    existing=bpy.data.materials.get(name)
    if existing:return existing
    m=bpy.data.materials.new(name);m.use_nodes=True
    n=m.node_tree.nodes;l=m.node_tree.links;p=n.get('Principled BSDF')
    p.inputs['Roughness'].default_value=.92
    coordinates=n.new('ShaderNodeTexCoord')
    brick=n.new('ShaderNodeTexBrick');brick.offset=.5;brick.offset_frequency=2
    brick.inputs['Scale'].default_value=1
    brick.inputs['Brick Width'].default_value=.7;brick.inputs['Row Height'].default_value=.5
    brick.inputs['Mortar Size'].default_value=.008;brick.inputs['Mortar Smooth'].default_value=.004
    brick.inputs['Color1'].default_value=(.012,.016,.020,1)
    brick.inputs['Color2'].default_value=(.025,.030,.035,1)
    brick.inputs['Mortar'].default_value=(.006,.008,.010,1)
    l.new(coordinates.outputs['Object'],brick.inputs['Vector'])
    noise=n.new('ShaderNodeTexNoise');noise.inputs['Scale'].default_value=18;noise.inputs['Detail'].default_value=2
    l.new(coordinates.outputs['Object'],noise.inputs['Vector'])
    variation=n.new('ShaderNodeValToRGB')
    variation.color_ramp.elements[0].color=(.78,.78,.78,1)
    variation.color_ramp.elements[1].color=(1,1,1,1)
    l.new(noise.outputs['Fac'],variation.inputs[0])
    tint=n.new('ShaderNodeMixRGB');tint.blend_type='MULTIPLY';tint.inputs[0].default_value=1
    l.new(brick.outputs['Color'],tint.inputs[1]);l.new(variation.outputs['Color'],tint.inputs[2]);l.new(tint.outputs[0],p.inputs['Base Color'])
    grain=n.new('ShaderNodeBump');grain.inputs['Strength'].default_value=.12;grain.inputs['Distance'].default_value=.001
    l.new(noise.outputs['Fac'],grain.inputs['Height'])
    grout=n.new('ShaderNodeBump');grout.invert=True;grout.inputs['Strength'].default_value=.25;grout.inputs['Distance'].default_value=.003
    l.new(brick.outputs['Fac'],grout.inputs['Height']);l.new(grain.outputs['Normal'],grout.inputs['Normal']);l.new(grout.outputs['Normal'],p.inputs['Normal'])
    return m

def stage(kind=None, gameplay=False):
    s=bpy.context.scene;s.render.engine='CYCLES';s.cycles.device='CPU'
    s.render.resolution_x=1280;s.render.resolution_y=720;s.render.resolution_percentage=100;s.cycles.samples=12;s.cycles.use_denoising=True
    s.view_settings.look='AgX - Medium High Contrast'
    # Swap only tagged staging; geometry and the existing evidence camera survive.
    for o in list(s.objects):
        if o.get('creature_preview_stage'):bpy.data.objects.remove(o,do_unlink=True)
    s.world.use_nodes=True
    background=s.world.node_tree.nodes.get('Background')
    background.inputs['Color'].default_value=(.12,.17,.25,1) if gameplay else (.018,.018,.018,1)
    background.inputs['Strength'].default_value=.1 if gameplay else 1
    bpy.ops.mesh.primitive_plane_add(size=200);floor=bpy.context.object
    floor.name='Gameplay stone floor' if gameplay else 'Studio floor';floor['creature_preview_stage']=True
    if gameplay:floor.data.materials.append(gameplay_floor_material())
    else:
        studio=bpy.data.materials.get('Studio slate W5-08d') or material('Studio slate W5-08d',(.014,.017,.020),.9,8)
        floor.data.materials.append(studio)
    if gameplay:
        # Prototype visual W5-08d: warm local lamp with restrained cool fill.
        bpy.ops.object.light_add(type='POINT',location=(1.5,-1.4,2.4));o=bpy.context.object
        o.name='Gameplay warm point';o['creature_preview_stage']=True
        o.data.energy=650;o.data.shadow_soft_size=.12;o.data.color=(1,.64,.37)
        lights=[((-2,2,3),120,3,(.48,.62,1))]
    else:lights=[((2,-3,4),450,4,(1,.78,.55)),((-2,2,3),600,3,(.60,.78,1)),((-1,-1,2),120,2,(1,1,1))]
    for pos,power,size,color in lights:
        if kind=='slime' and not gameplay:size*=.3
        bpy.ops.object.light_add(type='AREA',location=pos);o=bpy.context.object;o['creature_preview_stage']=True
        o.data.energy=power;o.data.shape='DISK';o.data.size=size;o.data.color=color;aim(o,(0,0,.3))
    cam=s.camera
    if cam is None or cam.name not in s.objects:
        bpy.ops.object.camera_add();cam=bpy.context.object;s.camera=cam
    return cam

def render(name):
    bpy.context.scene.render.filepath=str(OUT/(name+'.png'));bpy.ops.render.render(write_still=True)
def previews(kind, mesh, rig, actions, hero_only=False, views=None):
    views=set(views if views is not None else (('hero',) if hero_only else ('hero','gameplay','actions')))
    assert views <= {'hero','gameplay','actions'},views
    cam=stage(kind);target,hero_scale,hero_location=roster.hero(kind)
    rig.animation_data.action=actions['idle'];rig.animation_data.action_slot=actions['idle'].slots[0];bpy.context.scene.frame_set(0)
    if 'hero' in views:
        cam.data.type='ORTHO';cam.data.ortho_scale=hero_scale
        cam.location=hero_location
        aim(cam,target);render(kind+'_hero')
    if 'gameplay' in views:
        stage(kind,gameplay=True)
        cam.data.type='PERSP';cam.data.angle=math.radians(45);cam.location=(4.867,-4.867,10.73);aim(cam,(0,0,.9));render(kind+'_gameplay')
    if 'actions' not in views:return
    stage(kind)
    # Uniform studio illumination across the entire contact-sheet row. Local
    # hero lamps fall off before reaching the first/last large boss copies.
    for obj in list(bpy.context.scene.objects):
        if obj.type=='LIGHT' and obj.get('creature_preview_stage'):
            bpy.data.objects.remove(obj,do_unlink=True)
    for position,energy,color in [((5,-2,4),4.0,(1,.79,.59)),((-4,2,5),2.0,(.58,.76,1))]:
        bpy.ops.object.light_add(type='SUN',location=position);light=bpy.context.object
        light['creature_preview_stage']=True;light.data.energy=energy;light.data.angle=.14;light.data.color=color;aim(light,(0,0,0))
    # Five true skinned copies sampled at representative action keys, in one sheet.
    rig.animation_data.action=None;rig.hide_render=True;mesh.hide_render=True
    spacing=max(1.18,roster.RULERS[kind][2]*2.35)
    for i,(clip,frame) in enumerate([('idle',15),('move',6),('attack',30),('hit',9),('death',60)]):
        r=rig.copy();r.data=rig.data.copy();bpy.context.collection.objects.link(r);r.hide_render=False;r.animation_data_clear();r.animation_data_create();r.animation_data.action=actions[clip];r.animation_data.action_slot=actions[clip].slots[0]
        o=mesh.copy();o.data=mesh.data.copy();bpy.context.collection.objects.link(o);o.hide_render=False;o.parent=r;next(mod for mod in o.modifiers if mod.type=='ARMATURE').object=r
        # Evaluate each source action at its own frame, freeze into pose transforms.
        bpy.context.scene.frame_set(frame);transforms={b.name:(b.location.copy(),b.rotation_euler.copy(),b.scale.copy()) for b in r.pose.bones};r.animation_data_clear()
        for b in r.pose.bones:b.location,b.rotation_euler,b.scale=transforms[b.name]
        bpy.context.view_layer.update()
        # Bake this representative pose to geometry before later frame changes.
        deps=bpy.context.evaluated_depsgraph_get();posed=bpy.data.meshes.new_from_object(o.evaluated_get(deps));o.modifiers.clear();o.data=posed
        r.location=(0,(i-2)*spacing,0)
        if kind=='rat':r.rotation_mode='XYZ';r.rotation_euler.z-=.65
        bpy.ops.object.text_add(location=(max(.6,roster.RULERS[kind][1]+.2),(i-2)*spacing,.005));txt=bpy.context.object;txt.data.body=clip;txt.data.size=.10*spacing;txt.data.align_x='CENTER';txt.rotation_euler.z=math.pi/2
    cam.data.type='ORTHO';cam.data.ortho_scale=spacing*5.4;cam.location=(9,0,6);aim(cam,(0,0,target[2]*.65));render(kind+'_actions')

def main(kinds=('rat','bat','slime'), do_render=True, geometry_only=False):
    for kind in kinds:
        mesh,rig,actions,tris=build(kind,not geometry_only)
        bounds={}
        for clip,act in actions.items():
            rig.animation_data.action=act;rig.animation_data.action_slot=act.slots[0];lo=[1e9]*3;hi=[-1e9]*3
            for f in range(int(act.frame_range[0]),int(act.frame_range[1])+1):
                bpy.context.scene.frame_set(f);ev=mesh.evaluated_get(bpy.context.evaluated_depsgraph_get())
                for v in ev.data.vertices:
                    co=ev.matrix_world@v.co
                    for j in range(3):lo[j]=min(lo[j],co[j]);hi[j]=max(hi[j],co[j])
            low,high=roster.limits(kind,clip)
            assert all(lo[j]>=low[j]-(.0005 if j<2 else 0) and hi[j]<=high[j]+.0005 for j in range(3)),(kind,clip,'animated envelope exceeded',lo,hi)
            bounds[clip]={'min_cm':[round(x*100,2) for x in lo],'max_cm':[round(x*100,2) for x in hi]}
        if geometry_only:
            (OUT/(kind+'-geometry-check.json')).write_text(json.dumps({'kind':kind,'triangles':tris,'bounds':bounds},indent=2)+'\n')
            continue
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
        RESULT.append(dict(creature=kind,triangles=tris,bones=len(imported[0].data.bones),texture_size=1024,rest_bounds=rest_bounds,baked_ranges=baked_ranges,bake={'ao':'Cycles geometric AO shader','ao_distance_m':.065,'ao_samples':32,'uv_margin':.0005,'uv_margin_method':'FRACTION','bake_margin_px':2,'channels':'AO/roughness/metallic'},animation_bounds=bounds,imported_actions=names,glb_bytes=(OUT/(kind+'.glb')).stat().st_size))
        (OUT/(kind+'-validation.json')).write_text(json.dumps(RESULT[-1],indent=2)+'\n')
        previous=json.loads((OUT/'validation.json').read_text()) if (OUT/'validation.json').exists() else []
        merged={r['creature']:r for r in previous};merged.update({r['creature']:r for r in RESULT})
        (OUT/'validation.json').write_text(json.dumps(list(merged.values()),indent=2))

if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--only',choices=roster.KINDS);parser.add_argument('--geometry-only',action='store_true');parser.add_argument('--batch2',action='store_true');parser.add_argument('--no-render',action='store_true');parser.add_argument('--render',action='store_true')
    args=parser.parse_args(sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else [])
    main((args.only,) if args.only else (roster.BATCH2 if args.batch2 else roster.PILOT),not args.no_render,args.geometry_only)
