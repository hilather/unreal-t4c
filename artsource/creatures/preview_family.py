"""Imported bat family comparison: common scale, studio view and grayscale.

Prototype W5-08e review conditions, not the fixed gameplay camera. Run only
once bat, dungeon_bat, giant_bat and undead_bat GLBs have been built.
"""
import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
import bpy
import math
import build

bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
bpy.context.scene.render.fps=30
kinds=('bat','dungeon_bat','giant_bat','undead_bat')
for index,kind in enumerate(kinds):
    before=set(bpy.data.actions)
    bpy.ops.import_scene.gltf(filepath=str(build.OUT/(kind+'.glb')))
    rig=next(o for o in bpy.context.selected_objects if o.type=='ARMATURE')
    for track in rig.animation_data.nla_tracks:track.mute=True
    idle=next(a for a in set(bpy.data.actions)-before if a.name.split('.')[0]=='idle')
    rig.animation_data.action=idle;rig.animation_data.action_slot=idle.slots[0]
    rig.location.y=(index-1.5)*1.65
    bpy.context.scene.frame_set(0)
    bpy.ops.object.text_add(location=(.65,(index-1.5)*1.65,.008))
    label=bpy.context.object;label.data.body=kind.replace('_',' ')
    label.data.size=.16;label.data.align_x='CENTER';label.rotation_euler.z=math.pi/2
cam=build.stage()
for obj in list(bpy.context.scene.objects):
    if obj.type=='LIGHT':bpy.data.objects.remove(obj,do_unlink=True)
for pos,energy,color in [((5,-2,4),4.0,(1,.79,.59)),((-4,2,5),2.0,(.58,.76,1))]:
    bpy.ops.object.light_add(type='SUN',location=pos);light=bpy.context.object
    light.data.energy=energy;light.data.angle=.14;light.data.color=color;build.aim(light,(0,0,0))
cam.data.type='ORTHO';cam.data.ortho_scale=7.55;cam.location=(6,0,9.42)
build.aim(cam,(0,0,.85))
for obj in bpy.context.scene.objects:
    if obj.type=='FONT':
        obj.location.x=.40;obj.location.z=.42;obj.rotation_euler=cam.rotation_euler
build.render('bat_family_color')
for mat in bpy.data.materials:
    if not mat.use_nodes:continue
    nodes,links=mat.node_tree.nodes,mat.node_tree.links;p=nodes.get('Principled BSDF')
    if p is None:continue
    base=p.inputs['Base Color']
    if base.is_linked:
        source=base.links[0].from_socket;gray=nodes.new('ShaderNodeRGBToBW')
        links.new(source,gray.inputs[0]);links.new(gray.outputs[0],base)
    else:
        r,g,b,a=base.default_value;y=.2126*r+.7152*g+.0722*b;base.default_value=(y,y,y,a)
for obj in bpy.context.scene.objects:
    if obj.type=='LIGHT':obj.data.color=(1,1,1)
bpy.context.scene.world.node_tree.nodes.get('Background').inputs['Color'].default_value=(.018,.018,.018,1)
build.render('bat_family_grayscale')
