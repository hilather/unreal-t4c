"""Original procedural creature pilot. Blender 5.2, no external asset inputs.
Run: blender -b --factory-startup --python ArtSource/Creatures/build.py
"""
import bpy, math, random, json
from pathlib import Path
from mathutils import Vector
random.seed(508)
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
parts=[]
def sphere(name, pos, scale, mat, bone='body', seg=24,rings=12):
    bpy.ops.mesh.primitive_uv_sphere_add(segments=seg,ring_count=rings,location=pos)
    o=bpy.context.object; o.name=name; o.scale=scale
    bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    o.data.materials.append(mat)
    if name=='Head':
        for v in o.data.vertices:
            taper=1-.3*max(0,v.co.x/scale[0]);v.co.y*=taper;v.co.z*=taper
    if name=='Ear':
        for v in o.data.vertices:
            v.co.x*=1-.35*max(0,v.co.z/scale[2])
    if name in ['Back','Chest','Torso']:
        for v in o.data.vertices:
            v.co *= 1 + .035*math.sin(v.co.x*430+v.co.z*120)*math.cos(v.co.y*380)
    for f in o.data.polygons:f.use_smooth=True
    g=o.vertex_groups.new(name=bone);g.add(list(range(len(o.data.vertices))),1,'REPLACE');parts.append(o)
    return o

def tube(name, points, radius, mat, bone):
    verts=[];faces=[]; N=8
    for i,p in enumerate(points):
        tangent=Vector(points[min(i+1,len(points)-1)])-Vector(points[max(0,i-1)])
        tangent.normalize(); u=tangent.cross(Vector((0,0,1))).normalized(); v=tangent.cross(u).normalized()
        r=radius*(1-.75*i/(len(points)-1))
        for j in range(N): verts.append(Vector(p)+r*(math.cos(j*math.tau/N)*u+math.sin(j*math.tau/N)*v))
    for i in range(len(points)-1):
        for j in range(N):a=i*N+j;b=i*N+(j+1)%N;faces.append((a,b,b+N,a+N))
    faces.extend([tuple(range(N-1,-1,-1)),tuple((len(points)-1)*N+j for j in range(N))])
    mesh=bpy.data.meshes.new(name);mesh.from_pydata(verts,[],faces);mesh.update()
    o=bpy.data.objects.new(name,mesh);bpy.context.collection.objects.link(o);o.data.materials.append(mat)
    g=o.vertex_groups.new(name=bone);g.add(list(range(len(verts))),1,'REPLACE');parts.append(o)
    for f in mesh.polygons:f.use_smooth=True
    return o

def tufts(center, axes, mat, count):
    verts=[];faces=[]
    for i in range(count):
        a=random.uniform(0,math.tau); z=random.uniform(-.15,.98);r=math.sqrt(1-z*z)
        normal=Vector((r*math.cos(a),r*math.sin(a),z));p=Vector(center)+Vector(tuple(normal[j]*axes[j] for j in range(3)))
        side=normal.cross(Vector((1,0,0))).normalized()*.004
        tip=p+normal*.007+Vector((-.009,0,0));base=len(verts)
        verts.extend([p-side,p+side,p+Vector((.004,0,0)),tip]);faces.extend([(base,base+1,base+3),(base+1,base+2,base+3),(base+2,base,base+3)])
    me=bpy.data.meshes.new('Short original fur tufts');me.from_pydata(verts,[],faces);me.update()
    o=bpy.data.objects.new('Fur silhouette',me);bpy.context.collection.objects.link(o);o.data.materials.append(mat)
    g=o.vertex_groups.new(name='body');g.add(list(range(len(verts))),1,'REPLACE');parts.append(o)

def build(kind):
    global parts
    bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False);parts=[]
    fur=material('Umber fur',(.10,.055,.025),.83,35)
    skin=material('Rose skin',(.25,.11,.065),.62,65)
    dark=material('Eyes',(.009,.006,.004),.17,30)
    membrane=material('Leather membrane',(.40,.25,.12),.65,12)
    green=material('Jelly',(.04,.22,.008),.14,9)
    bones={'root':((0,0,0),(0,0,.05),None),'body':((0,0,0),(0,.1,0),'root')}
    if kind=='rat':
        bones['body']=((0,0,.1),(0,.1,.1),'root')
        sphere('Back',(-.025,0,.135),(.18,.125,.111),fur)
        tufts((-.025,0,.135),(.18,.123,.105),fur,160)
        sphere('Chest',(.10,0,.12),(.09,.085,.09),fur)
        sphere('Head',(.145,0,.115),(.075,.048,.048),fur,'head')
        sphere('Muzzle',(.200,0,.095),(.02,.034,.025),skin,'head')
        bones['head']=((.10,0,.13),(.22,0,.13),'body')
        for s in [-1,1]:
            sphere('Ear',(.13,s*.046,.17),(.028,.014,.032),skin,'head',16,8)
            sphere('Eye',(.185,s*.043,.14),(.009,.006,.009),dark,'head',12,6)
            for x,label in [(-.11,'rear'),(.10,'front')]:
                bn=label+str(s);bones[bn]=((x,s*.07,.09),(x,s*.09,.09),'body')
                sphere('Leg',(x,s*.085,.055),(.027,.022,.045),fur,bn,16,8)
                sphere('Foot',(x+.015,s*.10,.014),(.035,.025,.014),skin,bn,16,8)
                for y in [-.013,0,.013]:tube('Toe',[(x+.02,s*.1+y,.014),(x+.045,s*.1+y,.009)],.004,skin,bn)
        for side in [-1,1]:
            for j in range(3):
                tube('Whisker',[(.212,side*.025,.10+j*.003),(.20-j*.009,side*(.08+j*.013),.105+j*.006)],.0008,skin,'head')
        bones['tail']=((-.18,0,.08),(-.45,0,.03),'body')
        tube('Tail',[(-.18,0,.08),(-.28,.005,.045),(-.40,.02,.025),(-.52,.035,.013),(-.65,.04,.01),(-.672,.015,.012)],.012,skin,'tail')
    elif kind=='bat':
        bones['body']=((0,0,1),(0,.1,1),'root')
        bones['head']=((.04,0,1.07),(.14,0,1.07),'body')
        sphere('Torso',(0,0,1),(.075,.05,.08),fur)
        tufts((0,0,1),(.075,.05,.08),fur,70)
        sphere('Head',(.08,0,1.08),(.058,.055,.05),fur,'head')
        sphere('Muzzle',(.13,0,1.065),(.03,.033,.022),skin,'head',16,8)
        for s in [-1,1]:
            sphere('Ear',(.055,s*.043,1.16),(.022,.018,.073),skin,'head',16,8)
            sphere('Eye',(.12,s*.038,1.10),(.008,.006,.008),dark,'head',12,6)
            bn='wing'+str(s);bones[bn]=((0,s*.045,1.02),(0,s*.4,1.08),'body')
            # Scalloped outline with four finger tips and inset trailing valleys.
            outline=[(.025,.06,1.06),(.015,.18,1.18),(-.02,.396,1.13),(-.065,.32,1.01),(-.12,.30,.99),(-.09,.23,.98),(-.17,.20,.91),(-.11,.14,.95),(-.16,.075,.92)]
            curved=[]
            for k,a in enumerate(outline):
                b=outline[(k+1)%len(outline)]
                curved.append(a)
                if k>=2:
                    curved.append(tuple((a[c]+b[c])*.5 + (.014 if c==0 else -.012 if c==1 else .006) for c in range(3)))
            finger_tips=[outline[i] for i in [1,2,4,6,8]]
            outline=curved
            center=Vector((-.04,.16,1.04));verts=[tuple((center.x,s*center.y,center.z))]
            verts += [(x,s*y,z) for x,y,z in outline]
            faces=[(0,i+1,(i+1)%len(outline)+1) for i in range(len(outline))]
            me=bpy.data.meshes.new('Wing');me.from_pydata(verts,[],faces);me.update()
            o=bpy.data.objects.new('Scalloped membrane',me);bpy.context.collection.objects.link(o);o.data.materials.append(membrane)
            for f in me.polygons:f.use_smooth=True
            g=o.vertex_groups.new(name=bn);g.add(list(range(len(verts))),1,'REPLACE');parts.append(o)
            bpy.context.view_layer.objects.active=o;o.select_set(True)
            mod=o.modifiers.new('Membrane tessellation','SUBSURF');mod.subdivision_type='SIMPLE';mod.levels=2;bpy.ops.object.modifier_apply(modifier=mod.name)
            mod=o.modifiers.new('Thickness','SOLIDIFY');mod.thickness=.002;bpy.ops.object.modifier_apply(modifier=mod.name);o.select_set(False)
            for tip in finger_tips:
                tube('Finger',[(.025,s*.06,1.06),((tip[0]+.025)*.5,s*(tip[1]+.06)*.5,(tip[2]+1.06)*.5),(tip[0],s*tip[1],tip[2])],.004,skin,bn)
            tube('Foot',[(-.045,s*.025,.94),(-.08,s*.03,.89),(-.04,s*.04,.89)],.009,skin,'body')
    else:
        # One continuous manifold pooled shell, asymmetrical rings; no intersecting lobes.
        verts=[(0,0,0)]; faces=[];N=64;R=28
        for i in range(R):
            t=i/(R-1);rad=.44*(1-t)**.65
            for j in range(N):
                a=j*math.tau/N; wob=1+.045*math.sin(5*a)+.03*math.cos(9*a)
                verts.append((rad*wob*math.cos(a)-.045*t,rad*wob*math.sin(a),max(.008,.008+.392*math.sin(t*math.pi/2)**2.5+.06*math.sin(3*a+.8)*math.sin(t*math.pi))))
        for j in range(N):faces.append((0,1+(j+1)%N,1+j))
        for i in range(R-1):
            for j in range(N):a=1+i*N+j;b=1+i*N+(j+1)%N;faces.append((a,b,b+N,a+N))
        xymax=max(abs(v[j]) for v in verts for j in [0,1]);verts=[(v[0]*.44/xymax,v[1]*.44/xymax,v[2]) for v in verts]
        me=bpy.data.meshes.new('Pooled slime');me.from_pydata(verts,[],faces);me.update()
        o=bpy.data.objects.new('Slime',me);bpy.context.collection.objects.link(o);o.data.materials.append(green);parts.append(o)
        base=o.vertex_groups.new(name='root');peak=o.vertex_groups.new(name='body')
        for v in me.vertices:
            w=min(1,max(0,(v.co.z-.04)/.24));base.add([v.index],1-w,'REPLACE');peak.add([v.index],w,'REPLACE')
        for f in me.polygons:f.use_smooth=True
    bpy.ops.object.select_all(action='DESELECT')
    for o in parts:o.select_set(True)
    bpy.context.view_layer.objects.active=parts[0];bpy.ops.object.join();mesh=bpy.context.object;mesh.name=kind
    bpy.ops.object.transform_apply(location=True,rotation=True,scale=True)
    bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.mesh.remove_doubles(threshold=.00001);bpy.ops.uv.smart_project(island_margin=.02);bpy.ops.object.mode_set(mode='OBJECT')
    tri=mesh.modifiers.new('Triangles','TRIANGULATE');bpy.ops.object.modifier_apply(modifier=tri.name)
    triangles=len(mesh.data.polygons)
    # Bake the generated shading into an atlas; no source-image pixels are used.
    scene=bpy.context.scene;scene.render.fps=30;scene.render.engine='CYCLES';scene.cycles.device='CPU';scene.cycles.samples=8
    images={}
    for channel in ['base','normal','orm']:
        im=bpy.data.images.new(kind+'_'+channel,width=512,height=512,alpha=False)
        if channel!='base':im.colorspace_settings.name='Non-Color'
        for m in mesh.data.materials:
            n=m.node_tree.nodes; node=n.new('ShaderNodeTexImage');node.image=im;n.active=node
        if channel=='base':
            scene.render.bake.use_pass_direct=False;scene.render.bake.use_pass_indirect=False;scene.render.bake.use_pass_color=True
            bpy.ops.object.bake(type='DIFFUSE',margin=8)
        elif channel=='normal':bpy.ops.object.bake(type='NORMAL',margin=8)
        else:
            # Bake constant AO=1 / species roughness / metal=0 through emission.
            for m in mesh.data.materials:
                n=m.node_tree.nodes;p=n.get('Principled BSDF');out=n.get('Material Output')
                e=n.new('ShaderNodeEmission');e.inputs[0].default_value=(1,p.inputs['Roughness'].default_value,0,1)
                m.node_tree.links.new(e.outputs[0],out.inputs[0])
            bpy.ops.object.bake(type='EMIT',margin=8)
        im.filepath_raw=str(OUT/(kind+'_'+channel+'.png'));im.file_format='PNG';im.save();images[channel]=im
    baked=bpy.data.materials.new(kind+'_baked');baked.use_nodes=True;n=baked.node_tree.nodes;l=baked.node_tree.links;p=n.get('Principled BSDF')
    for ch in images:
        node=n.new('ShaderNodeTexImage');node.image=images[ch]
        if ch=='base':l.new(node.outputs['Color'],p.inputs['Base Color'])
        elif ch=='normal':
            norm=n.new('ShaderNodeNormalMap');l.new(node.outputs['Color'],norm.inputs[1]);l.new(norm.outputs[0],p.inputs['Normal'])
        else:
            sep=n.new('ShaderNodeSeparateColor');l.new(node.outputs['Color'],sep.inputs[0]);l.new(sep.outputs[1],p.inputs['Roughness']);l.new(sep.outputs[2],p.inputs['Metallic'])
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
        frames=[0,end//4,end//2,3*end//4,end] if clip not in ['attack','death'] else ([0,20,29,30,42] if clip=='attack' else [0,15,30,45,60])
        for f in frames:
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
                        b.location.x=.015*step;b.location.z=.012*max(0,step)
                    if b.name.startswith('wing'):b.rotation_euler.y=(.25 if clip=='move' else .12)*(-1 if '-1' in b.name else 1)*(1 if f==end//2 else -1)
            elif clip=='attack':
                a={0:0,20:-.12,29:-.12,30:.16,42:0}[f]
                if kind=='slime':body.location.x=a*.35;body.scale=(1+a,1,1-a)
                else:rig.pose.bones['head'].rotation_euler.x=a
                for b in rig.pose.bones:
                    if b.name.startswith('wing'):b.rotation_euler.y=a*2
            elif clip=='hit':
                if kind=='slime':body.scale.z=.85 if f==end//2 else 1
                else:body.rotation_euler.z=.10 if f==end//2 else 0
            elif clip=='death' and f>=15:
                if kind=='slime':body.scale=(1,1,.12)
                elif kind=='bat':body.location.z=-.90;body.scale=(1,.6,.6)
                else:body.rotation_euler.x=math.pi/2;body.location.z=.10 if f==15 else .035;body.scale=(1,.60,.70)
            for b in rig.pose.bones:
                for prop in ['location','rotation_euler','scale']:b.keyframe_insert(data_path=prop,frame=f,group=b.name)
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
def stage():
    s=bpy.context.scene;s.render.resolution_x=1280;s.render.resolution_y=720;s.render.resolution_percentage=100;s.cycles.samples=12;s.cycles.use_denoising=True
    s.world.color=(.025,.025,.025)
    bpy.ops.mesh.primitive_plane_add(size=200);floor=bpy.context.object;floor.name='Studio floor';floor.data.materials.append(material('Slate',(.055,.065,.075),.9,8))
    for pos,power,size,color in [((2,-3,4),450,4,(1,.78,.55)),((-2,2,3),600,3,(.60,.78,1)),((-1,-1,2),120,2,(1,1,1))]:
        bpy.ops.object.light_add(type='AREA',location=pos);o=bpy.context.object;o.data.energy=power;o.data.shape='DISK';o.data.size=size;o.data.color=color;aim(o,(0,0,.3))
    bpy.ops.object.camera_add();cam=bpy.context.object;s.camera=cam;return cam

def render(name):
    bpy.context.scene.render.filepath=str(OUT/(name+'.png'));bpy.ops.render.render(write_still=True)
def main(kinds=('rat','bat','slime')):
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
            bounds[clip]={'min_cm':[round(x*100,2) for x in lo],'max_cm':[round(x*100,2) for x in hi]}
        cam=stage();target=(0,0,1.03 if kind=='bat' else .15)
        rig.animation_data.action=actions['idle'];rig.animation_data.action_slot=actions['idle'].slots[0];bpy.context.scene.frame_set(0)
        cam.data.type='ORTHO';cam.data.ortho_scale=1.35;cam.location=(1.4,-1.8,target[2]+1.15);aim(cam,target);render(kind+'_hero')
        cam.data.type='PERSP';cam.data.angle=math.radians(45);cam.location=(4.867,-4.867,10.73);aim(cam,(0,0,.9));render(kind+'_gameplay')
        # Five true skinned copies sampled at representative action keys, in one sheet.
        rig.animation_data.action=None;rig.hide_render=True;mesh.hide_render=True
        for i,(clip,frame) in enumerate([('idle',15),('move',6),('attack',30),('hit',9),('death',60)]):
            r=rig.copy();r.data=rig.data.copy();bpy.context.collection.objects.link(r);r.hide_render=False;r.animation_data_clear();r.animation_data_create();r.animation_data.action=actions[clip];r.animation_data.action_slot=actions[clip].slots[0]
            o=mesh.copy();o.data=mesh.data.copy();bpy.context.collection.objects.link(o);o.hide_render=False;o.parent=r;o.modifiers.get('Skin').object=r
            # Evaluate each source action at its own frame, freeze into pose transforms.
            bpy.context.scene.frame_set(frame);transforms={b.name:(b.location.copy(),b.rotation_euler.copy(),b.scale.copy()) for b in r.pose.bones};r.animation_data_clear()
            for b in r.pose.bones:b.location,b.rotation_euler,b.scale=transforms[b.name]
            bpy.context.view_layer.update()
            # Bake this representative pose to geometry before later frame changes.
            deps=bpy.context.evaluated_depsgraph_get();posed=bpy.data.meshes.new_from_object(o.evaluated_get(deps));o.modifiers.clear();o.data=posed
            r.location=(0,(i-2)*1.25,0)
            bpy.ops.object.text_add(location=(.6,(i-2)*1.25-.28,.005));txt=bpy.context.object;txt.data.body=clip;txt.data.size=.12
        cam.data.type='ORTHO';cam.data.ortho_scale=7;cam.location=(5,-1,5);aim(cam,(0,0,.5));render(kind+'_actions')
        # Validate the actual file in a clean scene.
        bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
        before=set(bpy.data.actions);bpy.ops.import_scene.gltf(filepath=str(OUT/(kind+'.glb')))
        imported=[o for o in bpy.context.scene.objects if o.type=='ARMATURE'];names=sorted(a.name for a in set(bpy.data.actions)-before)
        assert imported,kind+' missing armature';assert len(names)>=5,(kind,names)
        skinned=[o for o in bpy.context.scene.objects if o.type=='MESH' and any(m.type=='ARMATURE' for m in o.modifiers)]
        assert skinned,kind+' missing skin'
        RESULT.append(dict(creature=kind,triangles=tris,bones=len(imported[0].data.bones),texture_size=512,animation_bounds=bounds,imported_actions=names,glb_bytes=(OUT/(kind+'.glb')).stat().st_size))
    previous=json.loads((OUT/'validation.json').read_text()) if (OUT/'validation.json').exists() else []
    merged={r['creature']:r for r in previous};merged.update({r['creature']:r for r in RESULT})
    (OUT/'validation.json').write_text(json.dumps(list(merged.values()),indent=2))

if __name__=='__main__':main()
