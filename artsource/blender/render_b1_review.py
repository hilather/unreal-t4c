"""Render exported B1 art without rebaking. Presentation mock-ups, not game captures.

Run after build_env.py: Blender --background --factory-startup --python-exit-code 1
--python artsource/blender/render_b1_review.py -- --source Saved/ArtExport/env
The study clips a real exported wall at 120 cm and adds segmented masonry caps.
No concept pixels or gameplay data are input. All staging is Prototype art tuning.
"""
import argparse
import math
from pathlib import Path
import sys
import bpy
import bmesh
from mathutils import Vector
sys.path.insert(0, str(Path(__file__).resolve().parent))
from build_env import configure_render, render_room, render_individuals, camera, light
from geometry import PIECES, Builder


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source',type=Path,default=Path('Saved/ArtExport/env'))
    parser.add_argument('--samples',type=int,default=32)
    args=parser.parse_args(sys.argv[sys.argv.index('--')+1:])
    output=args.source.resolve()/'renders'
    output.mkdir(exist_ok=True)
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)
    configure_render(args.samples)
    pieces={}
    for name in PIECES:
        before=set(bpy.data.objects)
        bpy.ops.import_scene.gltf(filepath=str(args.source.resolve()/(name+'.glb')))
        obj=next(o for o in set(bpy.data.objects)-before if o.type=='MESH')
        obj.hide_render=True
        pieces[name]=obj
    render_room(pieces,output,'w509g_final')
    # Keep only hidden library meshes for a separate close-up of the real tile,
    # wall crop and original kit props. This deliberately avoids height scaling.
    for obj in list(bpy.data.objects):
        if obj not in pieces.values(): bpy.data.objects.remove(obj,do_unlink=True)
    def put(name,where,yaw=0):
        obj=bpy.data.objects.new('Study_'+name,pieces[name].data.copy())
        bpy.context.collection.objects.link(obj)
        obj.location=where
        obj.rotation_euler.z=math.radians(yaw)
        return obj
    put('Floor400',(0,0,0))
    wall=put('Wall400',(0,4,0))
    bm=bmesh.new();bm.from_mesh(wall.data)
    cut=bmesh.ops.bisect_plane(bm,geom=list(bm.verts)+list(bm.edges)+list(bm.faces),
        dist=.000001,plane_co=(0,0,1.2),plane_no=(0,0,1),clear_outer=True,clear_inner=False)
    bm.to_mesh(wall.data);bm.free();wall.data.update()
    # Same 6cm depth, 1.6cm joints and length pattern as native coping.
    stone=pieces['Wall400'].data.materials[0]
    builder=Builder({'stone':stone})
    cursor=0
    index=0
    while cursor<4-.00001:
        end=min(4,cursor+(.72,1.06,.88)[index%3])
        if 4-end<.16:end=4
        left=cursor+(.008 if cursor else 0)
        right=end-(.008 if end<4 else 0)
        tone=.78+.04*(index%5)
        builder.block(((left+right)/2,-.1,1.17),(right-left,.2,.06),
            bevel=0,irregular=0,tint=(tone,tone*.96,tone*.89,1))
        cursor=end;index+=1
    cap=builder.mesh('Study_coping');cap.location=(0,4,0)
    put('Barrel',(.65,3.4,0),17)
    put('Crate',(1.43,3.41,0),-7)
    put('Debris',(2.04,3.55,0),11)
    put('Debris',(.3,2.92,0),76)
    put('Bench',(3.1,3.45,0))
    put('Sconce',(2.4,3.99,.90),180)
    light('Warm torch',(2.4,3.67,1.04),80,(1,.48,.20),radius=.12)
    light('Warm bounce',(2,1,5),145,(.86,.68,.47),area=5,target=(2,2,0))
    camera((7,-6,8),(2,2.05,.2),6.9)
    bpy.context.scene.render.filepath=str(output/'B1_stone_cutaway_w509g.png')
    bpy.ops.render.render(write_still=True)
    for obj in list(bpy.data.objects):
        if obj not in pieces.values(): bpy.data.objects.remove(obj,do_unlink=True)
    render_individuals({'Arch320':pieces['Arch320']},output)
    print('B1_REVIEW_RENDERED: room, 120cm wall/floor study, arch; Cycles CPU, AgX, mock-ups only')


if __name__=='__main__': main()
