"""Small original mesh constructions. Metres, +X forward, +Z up."""
import math
import bpy
import bmesh
from mathutils import Vector
import materials as mat
PARTS=[]
GROUP='Body'

def begin():
    global PARTS,GROUP
    PARTS=[];GROUP='Body'

def group(name):
    global GROUP
    GROUP=name

def add(o,name,material):
    o.name=name;o['part']=GROUP
    o.data.materials.append(mat.source(material) if isinstance(material,str) else material)
    PARTS.append(o);return o

def mesh(name,verts,faces,material):
    d=bpy.data.meshes.new(name);d.from_pydata(verts,[],faces);d.update()
    o=bpy.data.objects.new(name,d);bpy.context.collection.objects.link(o)
    return add(o,name,material)

def box(name,loc,size,material,bevel=.005):
    bpy.ops.mesh.primitive_cube_add(size=1,location=loc);o=bpy.context.object;o.scale=size
    bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    if bevel:
        mod=o.modifiers.new('Worn corners','BEVEL');mod.width=bevel;mod.segments=1
        bpy.ops.object.modifier_apply(modifier=mod.name)
    return add(o,name,material)

def ico(name,loc,size,material,sub=1):
    bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=sub,radius=1,location=loc);o=bpy.context.object;o.scale=size
    bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    return add(o,name,material)

def tube(name,points,radii,material,sides=8,flatten=1):
    verts=[]
    for i,p in enumerate(points):
        p=Vector(p);axis=Vector(points[min(len(points)-1,i+1)])-Vector(points[max(0,i-1)])
        axis.normalize();ref=Vector((0,1,0)) if abs(axis.y)<.9 else Vector((1,0,0))
        a=axis.cross(ref).normalized();b=axis.cross(a).normalized()
        r=radii[i] if isinstance(radii,(list,tuple)) else radii
        for j in range(sides):verts.append(tuple(p+r*(math.cos(j*math.tau/sides)*a+flatten*math.sin(j*math.tau/sides)*b)))
    faces=[tuple(range(sides-1,-1,-1))]
    for i in range(len(points)-1):
        for j in range(sides):faces.append((i*sides+j,i*sides+(j+1)%sides,(i+1)*sides+(j+1)%sides,(i+1)*sides+j))
    faces.append(tuple((len(points)-1)*sides+j for j in range(sides)))
    o=mesh(name,verts,faces,material)
    for p in o.data.polygons:
        p.use_smooth=len(p.vertices)==4
    return o

def ring(name,center,radius,thickness,material,axis='Z',segments=24):
    pts=[]
    for j in range(segments+1):
        a=j*math.tau/segments;v=(radius*math.cos(a),radius*math.sin(a),0)
        if axis=='X':v=(0,v[0],v[1])
        if axis=='Y':v=(v[0],0,v[1])
        pts.append(tuple(center[k]+v[k] for k in range(3)))
    # Closed ring without coincident end caps.
    o=tube(name,pts,thickness,material,6)
    bm=bmesh.new();bm.from_mesh(o.data)
    bmesh.ops.remove_doubles(bm,verts=list(bm.verts),dist=1e-7)
    duplicate=[f for f in bm.faces if len(f.verts)==6]
    bmesh.ops.delete(bm,geom=duplicate,context='FACES_ONLY');bm.to_mesh(o.data);bm.free()
    return o

def lathe(name,profile,material,sides=16,center=(0,0,0),fold=0,open_top=False,open_bottom=False):
    verts=[]
    for k,(z,rx,ry) in enumerate(profile):
        for j in range(sides):
            a=j*math.tau/sides;w=1+fold*math.cos(a*7+.22*k)
            verts.append((center[0]+rx*math.cos(a)*w,center[1]+ry*math.sin(a)*w,center[2]+z))
    faces=[] if open_bottom else [tuple(range(sides-1,-1,-1))]
    for k in range(len(profile)-1):
        for j in range(sides):faces.append((k*sides+j,k*sides+(j+1)%sides,(k+1)*sides+(j+1)%sides,(k+1)*sides+j))
    if not open_top:faces.append(tuple((len(profile)-1)*sides+j for j in range(sides)))
    o=mesh(name,verts,faces,material)
    for p in o.data.polygons:p.use_smooth=len(p.vertices)==4
    return o

def transform(objects,matrix):
    for o in objects:o.matrix_world=matrix@o.matrix_world

def finish(atlas):
    """Triangulate, remove degenerate construction faces, pack UVs into swatches, join by component."""
    groups={}
    for o in PARTS:
        bpy.ops.object.select_all(action='DESELECT');o.select_set(True);bpy.context.view_layer.objects.active=o
        bpy.ops.object.transform_apply(location=True,rotation=True,scale=True)
        bm=bmesh.new();bm.from_mesh(o.data)
        bmesh.ops.remove_doubles(bm,verts=list(bm.verts),dist=1e-7)
        bmesh.ops.recalc_face_normals(bm,faces=list(bm.faces))
        bmesh.ops.triangulate(bm,faces=list(bm.faces))
        bmesh.ops.delete(bm,geom=[f for f in bm.faces if f.calc_area()<1e-11],context='FACES')
        bm.to_mesh(o.data);bm.free();o.data.update()
        source=o.data.materials[0];uv=o.data.uv_layers.new(name='UVMap') if not o.data.uv_layers else o.data.uv_layers[0]
        if source.name.startswith('Source.'):
            index=mat.INDEX[source.name.split('.')[1]];col=index%4;row=index//4
            coords=[v.co for v in o.data.vertices]
            lo=[min(v[a] for v in coords) for a in range(3)];hi=[max(v[a] for v in coords) for a in range(3)]
            for p in o.data.polygons:
                axes=sorted(range(3),key=lambda a:abs(p.normal[a]))[:2]
                for k in p.loop_indices:
                    v=o.data.vertices[o.data.loops[k].vertex_index].co
                    uvs=[(v[a]-lo[a])/max(1e-5,hi[a]-lo[a]) for a in axes]
                    uv.data[k].uv=((col+.055+.89*uvs[0])/4,(row+.055+.89*uvs[1])/4)
            o.data.materials[0]=atlas
        groups.setdefault(o['part'],[]).append(o)
    result=[]
    for name,parts in groups.items():
        bpy.ops.object.select_all(action='DESELECT')
        for o in parts:o.select_set(True)
        bpy.context.view_layer.objects.active=parts[0];bpy.ops.object.join();o=bpy.context.object;o.name=name
        bpy.context.scene.cursor.location=(0,0,0);bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
        o.data.calc_loop_triangles();result.append(o)
    return result
