"""Original, deterministic B1 visual geometry, in meters; no reference image input.

Coordinates are the numeric LHVisualKit local coordinates divided by 100:
X length/forward, Y lateral (sconce projects +Y), Z up. Mesh origin stays zero.
All aesthetic dimensions are prototype presentation choices, not game mechanics.
"""
import math
import random
import bpy
from mathutils import Vector, Matrix


class Builder:
    def __init__(self, materials, seed=7507):
        self.materials = list(materials.values())
        self.slots = {key: i for i, key in enumerate(materials)}
        self.rng = random.Random(seed)
        self.vertices, self.faces, self.surfaces, self.colors = [], [], [], []

    def polyhedron(self, verts, faces, material='stone', tint=None, transform=None):
        offset = len(self.vertices)
        if transform:
            verts = [transform @ Vector(v) for v in verts]
        self.vertices.extend(verts)
        if tint is None:
            shade = self.rng.uniform(.50, 1.16)
            tint = (shade, shade * self.rng.uniform(.93, 1.04), shade * self.rng.uniform(.86,1.02), 1)
        for face in faces:
            self.faces.append(tuple(i + offset for i in face))
            self.surfaces.append(self.slots[material])
            self.colors.append(tuple(min(1,max(0,v)) for v in tint))

    def block(self, center, size, material='stone', bevel=.025, irregular=.012,
              yaw=0, tint=None, pitch=0):
        """Chamfered stone/plank, with nonrepeating clipped corners and shoulders."""
        hx, hy, hz = [v / 2 for v in size]
        b = min(bevel, hx * .35, hy * .35, hz * .35)
        # Eight-point chamfered XY outline; all perimeter points stay in bounds.
        outline = [(-hx+b,-hy), (hx-b,-hy), (hx,-hy+b), (hx,hy-b),
                   (hx-b,hy), (-hx+b,hy), (-hx,hy-b), (-hx,-hy+b)]
        verts = []
        for ring, z in enumerate((-hz, -hz+b, hz-b, hz)):
            for x, y in outline:
                inset = b * .7 if ring in (0, 3) else 0
                j = self.rng.uniform(0, irregular)
                xx = math.copysign(max(0, abs(x)-inset-j), x)
                yy = math.copysign(max(0, abs(y)-inset-j*.35), y)
                # Preserve exact extents on the middle shoulders.
                if ring in (1, 2) and abs(x) == hx: xx = x
                if ring in (1, 2) and abs(y) == hy: yy = y
                zz=z
                if irregular:
                    zz += (1 if z<0 else -1)*self.rng.uniform(0,min(irregular,hz*.3))
                    if ring in (1,2): yy -= math.copysign(self.rng.uniform(0,irregular*.15),yy)
                verts.append((xx, yy, zz))
        faces = [tuple(reversed(range(8))), tuple(range(24,32))]
        for ring in range(3):
            for i in range(8):
                j = (i+1) % 8
                faces.append((ring*8+i, ring*8+j, (ring+1)*8+j, (ring+1)*8+i))
        transform = Matrix.Translation(Vector(center)) @ Matrix.Rotation(yaw,4,'Z') @ Matrix.Rotation(-pitch,4,'Y')
        self.polyhedron(verts, faces, material, tint, transform)

    def lathe(self, profile, material='iron', segments=16, center=(0,0,0), tint=None,
              phase=0, start=0, end=math.tau, caps=True):
        verts = []
        for z, radius in profile:
            for i in range(segments+1):
                a = phase + start + (end-start)*i/segments
                verts.append((center[0]+radius*math.cos(a), center[1]+radius*math.sin(a),center[2]+z))
        n = segments+1
        faces = []
        for k in range(len(profile)-1):
            for i in range(segments):
                a = k*n+i
                faces.append((a,a+1,a+n+1,a+n))
        if caps:
            faces.extend([tuple(reversed(range(n))),tuple((len(profile)-1)*n+i for i in range(n))])
        if end-start < math.tau-.0001:
            faces.extend([tuple(k*n for k in reversed(range(len(profile)))),
                          tuple(k*n+segments for k in range(len(profile)))])
        self.polyhedron(verts,faces,material,tint)

    def mesh(self, name):
        mesh = bpy.data.meshes.new(name)
        mesh.from_pydata(self.vertices, [], self.faces)
        mesh.update()
        for mat in self.materials: mesh.materials.append(mat)
        uv = mesh.uv_layers.new(name='UVMap')
        colors = mesh.color_attributes.new(name='Tint',type='FLOAT_COLOR',domain='CORNER')
        # Meter-sized triplanar UV projection, stored as ordinary mesh UVs.
        for p, surface, tint in zip(mesh.polygons,self.surfaces,self.colors):
            p.material_index = surface
            axis = max(range(3), key=lambda a: abs(p.normal[a]))
            for li in p.loop_indices:
                co = mesh.vertices[mesh.loops[li].vertex_index].co
                if axis == 2: tex = (co.x,co.y)
                elif axis == 1: tex = (co.x,co.z)
                else: tex = (co.y,co.z)
                # Timber grain follows long planks horizontally, staves vertically.
                if surface == self.slots.get('timber'):
                    if name=='Barrel' and axis!=2: tex=(tex[1],tex[0])
                    tex = (tex[0]*1.25,tex[1]*1.25)
                uv.data[li].uv = tex
                colors.data[li].color = tint
        obj = bpy.data.objects.new(name, mesh)
        bpy.context.collection.objects.link(obj)
        return obj


def wall(b, length=4, height=4, x=0, z=0):
    # Mortar must sit behind the entire bevel, otherwise it occludes chipped edges.
    b.block((x+length/2,-.1,z+height/2),(length,.10,height),bevel=.008,irregular=0,tint=(.36,.34,.30,1))
    rows = max(1,round(height/.4))
    step = height/rows
    for row in range(rows):
        left = 0
        while left < length-.001:
            width = min(length-left, b.rng.uniform(.57,.94) if left or row%2==0 else .36)
            gap = .022
            jitter=b.rng.uniform(-.014,.014)
            b.block((x+left+width/2,-.1,z+(row+.5)*step),
                    (max(.015,width-gap),.2,step-.01+jitter),bevel=.038,irregular=.045)
            left += width


def floor(b, length=4):
    first_vertex=len(b.vertices)
    b.block((length/2,length/2,-.1375),(length,length,.125),bevel=.008,irregular=0,tint=(.40,.38,.35,1))
    rows = 7
    for row in range(rows):
        y0 = row*length/rows
        x = 0
        while x < length-.001:
            w = min(length-x,b.rng.uniform(.48,.95))
            h = length/rows
            # A dry dusty, moderately irregular flagstone pavement, at exactly Z0.
            if w>.65 and b.rng.random()<.25:
                # Occasional split flags break the regular rows without changing the tile boundary.
                for half in (0,1):
                    b.block((x+w*(.25+.5*half),y0+h/2,-.04),
                            (w*.5-.016,h-.014,.08),bevel=.022,irregular=.024)
            else:
                b.block((x+w/2,y0+h/2,-.04),(max(.01,w-.014),h-.014,.08),
                        bevel=.022,irregular=.024)
            x += w
    # A continuous, boundary-preserving warp gives worn flags irregular joints.
    for i in range(first_vertex,len(b.vertices)):
        x,y,z=b.vertices[i]
        b.vertices[i]=(x+.038*math.sin(y*4.1+x*2.8)*math.sin(math.pi*x/length),
                       y+.035*math.sin(x*3.7+y*4.2)*math.sin(math.pi*y/length),z)


def arch(b, width):
    side = (4-width)/2
    # Uprights and horizontal courses preserve the rectangular 3m clear aperture.
    for x in (-2, width/2):
        wall(b,side,3,x=x)
    # Shallow segmental archivolt entirely ABOVE the existing rectangular aperture.
    segments=11
    verts, faces=[],[]
    for i in range(segments):
        xa=-width/2+i*width/segments+.006
        xb=-width/2+(i+1)*width/segments-.006
        low=lambda x:3+.22*max(0,1-(2*x/width)**2)
        v=[(xa,-.2,low(xa)),(xb,-.2,low(xb)),(xb,-.2,3.57),(xa,-.2,3.57),
           (xa,0,low(xa)),(xb,0,low(xb)),(xb,0,3.57),(xa,0,3.57)]
        # Slightly wedge-shaped individual voussoirs; bevel applied to this strip below.
        b.polyhedron(v,[(0,3,2,1),(4,5,6,7),(0,1,5,4),(3,7,6,2),(0,4,7,3),(1,2,6,5)])
    # No backing across the arch: the opening is a real shallow segmental silhouette.
    wall(b,4,.43,x=-2,z=3.57)
    for xx in (-2+side/2,2-side/2):
        b.block((xx,-.1,3.27),(side,.2,.54),bevel=.02,irregular=.008)


def stairs(b, descending=False):
    run, rise, width = 6, (-1.2 if descending else 1.2), 3
    angle = math.atan2(rise,run)
    # Only the Unreal collider remains smooth. A visible slope would cover the risers.
    bottom=min(0,rise)-.2*math.cos(angle)
    b.block((run/2,0,bottom+.05),(run,width,.1),
            bevel=.007,irregular=0,tint=(.60,.59,.55,1))
    for i in range(12):
        top=(i if descending else i+1)*rise/12
        height=top-bottom
        for j in range(4):
            # Solid stone below each tread avoids a floating sheet-like stair silhouette.
            b.block(((i+.5)*run/12,(j-1.5)*width/4,(top+bottom)/2),
                    (run/12-.02,width/4-.008,height),bevel=.015,irregular=.009)


def barrel(b):
    profile=[(0,.245),(.07,.27),(.25,.295),(.5,.302),(.75,.295),(.94,.27),(1,.245)]
    for i in range(18):
        b.lathe(profile,'timber',segments=2,start=i*math.tau/18+.008,end=(i+1)*math.tau/18-.008)
    for z,r in ((.14,.292),(.76,.31)):
        # Hollow hoops with turned edges and varied dark hammered faces.
        b.lathe([(z,r-.014),(z+.012,r),(z+.068,r),(z+.08,r-.014)],'iron',segments=36,caps=False)
        for i in range(12):
            a=i*math.tau/12
            b.block((r*math.cos(a),r*math.sin(a),z+.04),(.015,.015,.018),'iron',.003,0)
    for i in range(5):
        y=(i-2)*.093
        length=2*math.sqrt(max(.001,.235**2-y*y))
        b.block((0,y,.978),(length,.09,.028),'timber',.005,.002)
    b.lathe([(.989,.03),(.999,.03)],'timber',12,center=(.09,0,0),tint=(.5,.48,.43,1))


def crate(b):
    # Recessed planks on every face, structural timber rims, dark nail heads.
    for sign in (-1,1):
        for i in range(5):
            c=(i-2)*.151
            b.block((c,sign*.374,.4),(.145,.038,.76),'timber',.006,.002)
            b.block((sign*.374,c,.4),(.038,.145,.76),'timber',.006,.002)
        for z in (.055,.745):
            b.block((0,sign*.39,z),(.82,.04,.11),'timber',.007,.003)
            b.block((sign*.39,0,z),(.04,.82,.11),'timber',.007,.003)
        # Diagonal front and back braces, 45deg in XZ plane.
        b.block((0,sign*.40,.4),(.87,.02,.09),'timber',.006,.002,pitch=.76*sign)
        for x in (-.34,.34):
            for z in (.055,.745):
                b.block((x,sign*.408,z),(.018,.004,.018),'iron',.003,0,tint=(.65,.63,.60,1))
    for i in range(5):
        b.block(((i-2)*.154,0,.78),(.15,.76,.04),'timber',.006,.002)
    b.block((0,0,.018),(.76,.76,.036),'timber',.004,0)


def furniture(b, bench=False):
    w,h=(.45,.45) if bench else (.9,.8)
    boards=3 if bench else 5
    for i in range(boards):
        b.block((0,(i-(boards-1)/2)*w/boards,h-.05),(1.6,w/boards-.006,.1),'timber',.012,.005)
    for x in (-.6,.6):
        for y in (-w/2+.08,w/2-.08):
            b.block((x,y,(h-.1)/2),(.12,.12,h-.1),'timber',.013,.005)
            b.block((x,y,h-.001),(.014,.016,.002),'iron',.001,0)
        b.block((x,0,.15),(.085,w-.04,.09),'timber',.009,.003)
    b.block((0,0,.19),(1.28,.07,.1),'timber',.009,.004)
    if not bench:
        for y in (-w/2+.05,w/2-.05):
            b.block((0,y,h-.17),(1.40,.065,.17),'timber',.009,.003)


def sconce(b):
    b.block((0,-.033,0),(.25,.054,.40),'iron',.045,.004)
    for z in (-.14,.14):
        b.block((0,-.002,z),(.034,.005,.034),'iron',.008,0,tint=(1.1,1.05,1,1))
    b.block((0,.105,-.10),(.035,.24,.042),'iron',.012,.002)
    b.block((0,.24,-.04),(.042,.042,.13),'timber',.008,.002)
    b.lathe([(-.01,.041),(.025,.087),(.06,.09),(.07,.084)],'iron',12,center=(0,.25,0),caps=False)
    # Four curled cage prongs frame a small flame rather than a rectangular proxy.
    for a in (0,math.pi/2,math.pi,math.pi*1.5):
        b.block((.073*math.cos(a),.25+.073*math.sin(a),.073),(.012,.012,.08),'iron',.003,0)
    for center,profile in [((0,.25,0),[(.04,.036),(.10,.054),(.17,.033),(.26,.002)]),
                           ((.022,.255,0),[(.065,.027),(.14,.024),(.22,.001)])]:
        b.lathe(profile,'flame',7,center=center,tint=(1,1,1,1))


def debris(b):
    for i in range(5):
        b.block((i*.18-.36,(i%2)*.20-.10,.06),(.16,.24,.12),bevel=.032,irregular=.025,yaw=math.radians(i*27))
    # Small fragments stay inside the canonical five-box extent.
    for i in range(13):
        b.block((b.rng.uniform(-.34,.34),b.rng.uniform(-.14,.14),.02),
                (b.rng.uniform(.025,.06),b.rng.uniform(.025,.055),.04),bevel=.01,irregular=.008,yaw=b.rng.random()*math.tau)


PIECES = ['Wall400','Floor400','Stair600x120','Stair600x120Descending',
          'Arch240','Arch320','Sconce','Barrel','Crate','Debris','Table','Bench']


def build_piece(name, materials):
    b=Builder(materials, 7507 + PIECES.index(name)*101)
    if name=='Wall400': wall(b)
    elif name=='Floor400': floor(b)
    elif name.startswith('Stair'): stairs(b,name.endswith('Descending'))
    elif name.startswith('Arch'): arch(b,int(name[4:])/100)
    elif name=='Sconce': sconce(b)
    elif name=='Barrel': barrel(b)
    elif name=='Crate': crate(b)
    elif name=='Debris': debris(b)
    else: furniture(b,name=='Bench')
    return b.mesh(name)
