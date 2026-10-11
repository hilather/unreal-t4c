"""Original W5-14 style art. No reference pixels or external assets are inputs.

Prototype presentation only; frozen kit envelopes remain in canonical meters.
Basement structural pieces/props reuse B1 construction and deterministic seeds.
The signatures replace existing Wall400/Floor400/Altar IDs, not phantom props.
"""
import math
from geometry import Builder, PIECES, build_piece, wall, floor, arch

STYLE_PIECES = {
    'Church': PIECES + ['Altar', 'Pillar', 'DoorLeafPreview', 'Carpet'],
    'B2Damp': list(PIECES),
    'B3Crypt': list(PIECES),
    'B4Ritual': PIECES + ['Altar', 'ArchBoss500'],
}


def canonical_style_bounds(style, name):
    # Deferred import avoids a cycle with build_env's dispatch.
    from build_env import canonical_bounds
    special = {
        'Altar': ((-1, -.8, 0), (1, .8, 1.2)),
        'Pillar': ((-.3, -.3, 0), (.3, .3, 4)),
        'DoorLeafPreview': ((0, -.05, 0), (1.6, .05, 3)),
        'ArchBoss500': ((-3, -.2, 0), (3, 0, 4)),
        'Carpet': ((-2, -10.5, 0), (2, 10.5, .01)),
    }
    return special[name] if name in special else canonical_bounds(name)


def _wall_church(b):
    # Low robust masonry, lime plaster above, modest pegged timber framing.
    wall(b, height=1.55)
    b.block((2, -.1, 2.775), (4, .13, 2.45), 'plaster', .019, .018,
            tint=(.91, .87, .77, 1))
    for z, h in ((1.59, .13), (3.88, .24)):
        b.block((2, -.1, z), (4, .2, h), 'timber', .016, .007,
                tint=(.82, .79, .72, 1))
    for x in (.10, 2, 3.9):
        b.block((x, -.1, 2.74), (.17, .20, 2.28), 'timber', .012, .005)
        for z in (1.72, 3.79):
            for y in (-.202, .002):
                b.block((x, y, z), (.028, .005, .028), 'timber', .004, 0,
                        tint=(.92, .82, .61, 1))
    # Exposed small patches of stone where the bottom plaster has fallen away.
    for x, z, w, h in ((.55, 1.76, .40, .15), (3.42, 1.83, .52, .27)):
        outline=[(-.50,-.5),(.50,-.5),(.47,.09),(.31,.10),(.33,.36),
                 (.10,.5),(-.08,.28),(-.32,.35),(-.42,.03)]
        for y in (-.175, -.025):
            verts=[(x+u*w,y+d,z+v*h) for d in (-.0175,.0175) for u,v in outline]
            n=len(outline)
            faces=[tuple(range(n)),tuple(reversed(range(n,2*n)))]
            faces.extend((i,i+n,(i+1)%n+n,(i+1)%n) for i in range(n))
            b.polyhedron(verts,faces,tint=(.68,.62,.50,1))


def _pillar(b):
    for z, width, height in ((.12, .60, .24), (.30, .56, .12),
                              (3.67, .56, .14), (3.86, .60, .28)):
        b.block((0, 0, z), (width, width, height), bevel=.027, irregular=.008)
    # Square, load-bearing timber: deliberately not a cathedral column.
    b.block((0, 0, 1.98), (.40, .40, 3.24), 'timber', .028, .010)
    for z in (.52, 3.45):
        for y in (-.211, .211):
            b.block((0, y, z), (.44, .035, .06), 'iron', .006, .002)


def _door(b):
    for i in range(7):
        b.block(((i+.5)*1.6/7, 0, 1.5), (1.6/7-.009, .064, 3),
                'timber', .011, .005)
    for z in (.60, 2.40):
        for y in (-.043, .043):
            b.block((.80, y, z), (1.58, .014, .10), 'iron', .008, .002)
            for x in (.12, .48, 1.11, 1.47):
                b.block((x, y*1.14, z), (.035, .008, .034), 'iron', .005, 0)
    # Small handle hoop lies in the XZ plane, within the 10cm leaf depth.
    for side in (-1, 1):
        for i in range(16):
            a=(i+.5)*math.tau/16
            b.block((1.32+.068*math.sin(a), side*.045, 1.43+.085*math.cos(a)),
                    (.027, .009, .037), 'iron', .007, .001, pitch=-a)


def _altar(b, ritual):
    for z, size in ((.12, (2, 1.6, .24)), (.30, (1.83, 1.43, .12)),
                    (.73, (1.55, 1.13, .74)), (1.13, (1.80, 1.40, .14))):
        b.block((0, 0, z), size, bevel=.025, irregular=.011)
    if ritual:
        for y in (-.575, .575):
            # Red mineral ribbons set into the dark altar face, no glow.
            for x in (-.52, .52):
                b.block((x, y, .73), (.037, .011, .65), 'inlay', .004, 0,
                        tint=(.9, .65, .42, 1))
            for pitch in (-math.pi/4, math.pi/4):
                b.block((0, y, .72), (.40, .012, .025), 'inlay', .003, 0,
                        pitch=pitch, tint=(.9, .70, .46, 1))
        _sigil(b, 0, 0, 1.202, .50)
    else:
        # Narrow cloth runner and worn borders leave most of the slab visible.
        b.block((0, 0, 1.204), (.64, 1.31, .012), 'cloth', .004, 0,
                tint=(.78, .62, .53, 1))
        for y in (-.704, .704):
            b.block((0, y, .99), (.64, .012, .41), 'cloth', .006, .002,
                    tint=(.8, .67, .55, 1))
        for x in (-.29, .29):
            b.block((x, 0, 1.212), (.018, 1.31, .003), 'plaster', .001, 0,
                    tint=(.60, .44, .19, 1))


def _carpet(b):
    # Actual RedAisle extent, origin at base center; auxiliary, no catalog ID.
    b.block((0, 0, .004), (4, 21, .008), 'cloth', .002, 0,
            tint=(.73, .57, .50, 1))
    # Faded woven tan double selvedge and transverse end borders.
    for x in (-1.82, -1.74, 1.74, 1.82):
        b.block((x, 0, .009), (.025, 20.65, .002), 'plaster', .001, 0,
                tint=(.46, .31, .14, 1))
    for y in (-10.20, -10.10, 10.10, 10.20):
        b.block((0, y, .009), (3.65, .025, .002), 'plaster', .001, 0,
                tint=(.46, .31, .14, 1))


def _crypt_wall(b):
    # Paired shallow memorial niches, empty: no invented skeleton population.
    wall(b, height=.8)
    wall(b, height=.86, z=3.14)
    for x, width in ((0, .45), (1.55, .9), (3.55, .45)):
        wall(b, width, 2.34, x=x, z=.8)
    for center in (1., 3.):
        # Rear stone screen gives real depth from either side of the thin wall.
        b.block((center, -.1, 1.77), (1.10, .034, 1.94),
                bevel=.01, irregular=.005, tint=(.27, .29, .27, 1))
        for y in (-.185, -.015):
            b.block((center, y, .86), (1.10, .04, .12), bevel=.016, irregular=.005,
                    tint=(.92, .90, .82, 1))
            # A low blank memorial tablet, deliberately without fake lettering.
            b.block((center, y*.55-.045, 1.35), (.65, .025, .64),
                    bevel=.035, irregular=.01, tint=(.50, .52, .48, 1))
        # Wedge voussoirs + filled shoulders above the curved opening.
        for i in range(9):
            a0=i*math.pi/9+.008
            a1=(i+1)*math.pi/9-.008
            x0=center+.55*math.cos(a0); x1=center+.55*math.cos(a1)
            z0=2.30+.55*math.sin(a0); z1=2.30+.55*math.sin(a1)
            v=[(x1,-.2,z1),(x0,-.2,z0),(x0,-.2,3.14),(x1,-.2,3.14),
               (x1,0,z1),(x0,0,z0),(x0,0,3.14),(x1,0,3.14)]
            b.polyhedron(v, [(0,1,2,3),(4,7,6,5),(0,4,5,1),(3,2,6,7),
                            (0,3,7,4),(1,5,6,2)], tint=(.78,.79,.74,1))


def _puddle_floor(b):
    floor(b)
    # Thin uneven pools in the lowest worn portions; closed shallow geometry.
    # The final canonical normalization keeps these inside the visual envelope.
    for cx, cy, sx, sy in ((.88, 2.8, .72, .33), (3.18, .8, .43, .67)):
        count=30
        ring=[]
        for i in range(count):
            a=i*math.tau/count
            r=(.83+.12*math.sin(a*3)+.07*math.cos(a*5))*b.rng.uniform(.94,1)
            ring.append((cx+sx*r*math.cos(a), cy+sy*r*math.sin(a), .002))
        verts=ring+[(x,y,-.007) for x,y,_ in ring]
        faces=[tuple(range(count)),tuple(reversed(range(count, count*2)))]
        faces += [(i,i+count,(i+1)%count+count,(i+1)%count) for i in range(count)]
        b.polyhedron(verts,faces,'wet', tint=(.22,.28,.24,1))


def _sigil(b, cx, cy, z, radius):
    # Angular open rings and radial dashes, mineral inlay without emission.
    for i in range(8):
        a=(i+.5)*math.tau/8
        r=radius*math.cos(math.pi/8)
        b.block((cx+r*math.cos(a),cy+r*math.sin(a),z),
                (radius*.70,.027,.007), 'inlay', .003, 0, yaw=a+math.pi/2,
                tint=(.84,.68,.54,1))
    for i in range(4):
        a=i*math.pi/2+math.pi/4
        b.block((cx+radius*.43*math.cos(a),cy+radius*.43*math.sin(a),z),
                (radius*.26,.035,.007), 'inlay', .003, 0, yaw=a,
                tint=(.90,.64,.42,1))


def _ritual_floor(b):
    floor(b)
    # Shallow stepped medallion bed carries actual recessed carving channels.
    b.block((2,2,-.003), (1.63,1.63,.026), bevel=.025, irregular=.012,
            tint=(.39,.42,.46,1))
    # Separated triangular sectors: the narrow radial seams are dark grooves.
    for i in range(8):
        a0=i*math.tau/8+.025; a1=(i+1)*math.tau/8-.025
        tri=[(2+.13*math.cos((a0+a1)/2),2+.13*math.sin((a0+a1)/2),.020),
             (2+.79*math.cos(a0),2+.79*math.sin(a0),.020),
             (2+.79*math.cos(a1),2+.79*math.sin(a1),.020)]
        verts=tri+[(x,y,-.004) for x,y,_ in tri]
        b.polyhedron(verts,[(0,1,2),(5,4,3),(0,3,4,1),(1,4,5,2),(2,5,3,0)],
                     tint=(.69,.70,.72,1))
    _sigil(b,2,2,.022,.66)


def _boss_arch(b):
    # Keep the full 500x360cm opening. Only 40cm are available over its head.
    for x in (-3, 2.5):
        wall(b, .5, 4, x=x)
    for i in range(13):
        x=-2.5+(i+.5)*5/13
        bottom=3.60+.08*(1-(x/2.5)**2)
        b.block((x,-.1,(4+bottom)/2), (5/13-.015,.2,4-bottom),
                bevel=.012, irregular=.003)
        if i%3==0:
            for y in (-.199,-.001):
                b.block((x,y,3.83),(.025,.003,.16),'inlay',.001,0,
                        tint=(.95,.72,.52,1))


def _neutralize_tints(obj, style):
    # B1 tints are warm ochre. Limestone/basalt need a neutral mineral base;
    # retain each face's brightness variation rather than shifting textures only.
    colors=obj.data.color_attributes.get('Tint')
    for face in obj.data.polygons:
        mat=obj.data.materials[face.material_index].name.lower()
        if 'stone' not in mat and 'wet' not in mat:
            continue
        for li in face.loop_indices:
            c=colors.data[li].color
            s=c[0]
            if style=='B3Crypt': colors.data[li].color=(s*.99,s,s*.96,1)
            elif style=='B4Ritual': colors.data[li].color=(s*.94,s*.97,s,1)
            elif style=='B2Damp' and obj.name=='Wall400':
                co=obj.data.vertices[obj.data.loops[li].vertex_index].co
                moss=max(0,1-co.z/1.65) * (.6+.4*math.sin(co.x*4.2)**2)
                colors.data[li].color=(s*(1-.30*moss),s*(1-.10*moss),s*(1-.48*moss),1)


def build_style_piece(style, name, materials):
    custom={
        ('Church','Wall400'): _wall_church,
        ('Church','Pillar'): _pillar,
        ('Church','DoorLeafPreview'): _door,
        ('Church','Carpet'): _carpet,
        ('Church','Altar'): lambda b:_altar(b,False),
        ('B2Damp','Floor400'): _puddle_floor,
        ('B3Crypt','Wall400'): _crypt_wall,
        ('B4Ritual','Floor400'): _ritual_floor,
        ('B4Ritual','Altar'): lambda b:_altar(b,True),
        ('B4Ritual','ArchBoss500'): _boss_arch,
    }
    fn=custom.get((style,name))
    if fn:
        b=Builder(materials, 7514+STYLE_PIECES[style].index(name)*101)
        fn(b)
        obj=b.mesh(name)
    else:
        obj=build_piece(name,materials)
    if style=='Church' and name in ('Pillar','DoorLeafPreview'):
        uv=obj.data.uv_layers.active.data
        for face in obj.data.polygons:
            if 'timber' in obj.data.materials[face.material_index].name.lower() and abs(face.normal.z)<.7:
                for li in face.loop_indices:
                    u,v=uv[li].uv
                    uv[li].uv=(v,u)
    _neutralize_tints(obj, style)
    return obj
