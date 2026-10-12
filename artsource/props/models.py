"""W5-23 original prop and mesh-effect silhouettes. No gameplay rules."""
import math
import random
from mathutils import Matrix,Vector
import geometry as g
import materials as m
ITEMS=['RustedDirk','AshwoodFlatbow','WoodenArrows','Torch','PotionOfMana','ClothVest','ClothPants','LightHeal','DecayingBatWings','GoblinLeatherArmor','IronRing','GoblinBlade','IronKey','FlowingBlackRobe']
PROPS=['LootSack','CorpseLootBadge','ChestClosed','ChestOpen']
EFFECTS=['FireDart','FireImpact','StoneShard','StoneShatter','DustDevil','HealLight','Light','MeleeSpark','SlashArc','ArrowFlight','ImpactPuff']
ALL=[('Item.'+n,n) for n in ITEMS]+[('Prop.'+n,n) for n in PROPS]+[('FX.'+n,n) for n in EFFECTS]
SOCKETS={'RustedDirk':'Socket.Weapon.R','GoblinBlade':'Socket.Weapon.R','AshwoodFlatbow':'Socket.Bow.L','WoodenArrows':'Socket.Quiver.Back','Torch':'Socket.Bow.L','ArrowFlight':'Socket.Arrow'}

def blade(goblin=False):
    length=.59 if goblin else .37
    g.tube('Leather wrapped grip',[(-.08,0,0),(.075,0,0)],.023,'leather')
    for i in range(7):g.ring('Grip binding',(-.064+i*.019,0,0),.024,.003,'thread' if i in (0,6) else 'leather','X',12)
    g.ico('Iron pommel',(-.095,0,0),(.033,.034,.037),'iron')
    g.tube('Bent guard',[(.09,0,-.09),(.07,0,-.05),(.075,0,.05),(.105,0,.09)],.014,'iron',6)
    if goblin:
        outline=[(.10,.043),(.29,.057),(.48,.11),(.68,.18),(.63,.035),(.55,-.065),(.44,-.082),(.42,-.047),(.38,-.08),(.26,-.071),(.10,-.045)]
        verts=[(x,y,z) for y in (-.013,.013) for x,z in outline];n=len(outline)
        faces=[tuple(range(n-1,-1,-1)),tuple(range(n,2*n))]
        faces += [(i,(i+1)%n,(i+1)%n+n,i+n) for i in range(n)]
        g.mesh('Hooked goblin cleaver',verts,faces,'iron')
        g.tube('Honed cleaver edge',[(x,0,z) for x,z in outline[3:]],[.003]*len(outline[3:]),'edge',4)
        for x,z in ((.2,.013),(.29,.02),(.37,.03),(.5,.045)):
            g.ico('Rust scars',(x,-.014,z),(.025,.002,.011),'rust',1)
        for x in (.16,.205):g.ico('Blade rivet',(x,-.017,0),(.01,.005,.01),'edge')
        return
    # Flattened diamond section with bright edge and recessed rust islands.
    stations=[(.095,.045),(.15,.047),(.31,.034),(length+.085,.002)] if not goblin else [(.095,.055),(.23,.086),(.42,.068),(.52,.043),(.68,.002)]
    verts=[]
    for x,w in stations:verts.extend([(x,0,-w),(x,-.016,0),(x,0,w),(x,.016,0)])
    faces=[(3,2,1,0)]
    for i in range(len(stations)-1):
        for j in range(4):faces.append((i*4+j,i*4+(j+1)%4,(i+1)*4+(j+1)%4,(i+1)*4+j))
    faces.append(tuple((len(stations)-1)*4+j for j in range(4)))
    g.mesh('Forged blade',verts,faces,'iron')
    for sign in (-1,1):
        pts=[(x,.0001,sign*w) for x,w in stations]
        g.tube('Honed chipped edge',pts,.0028,'edge',4)
    rng=random.Random(523)
    for i in range(9):
        x=.12+i*(length-.08)/9;z=rng.uniform(-.018,.018)
        g.ico('Oxidation',(x,-.0155,z),(.014,.0015,.006),'rust')
    if goblin:
        for x in (.18,.24,.3):g.ring('Scavenged ferrules',(x,0,0),.034,.004,'rust','X',8)

def arrow(origin=(0,0,0),length=.72):
    start=len(g.PARTS)
    g.tube('Ash shaft',[(.035,0,0),(length-.07,0,0)],.006,'ashwood',6)
    g.tube('Nock',[(0,0,0),(.055,0,0)],.008,'bone',6)
    g.mesh('Barbed arrowhead',[(length,0,0),(length-.1,-.009,-.033),(length-.074,0,0),(length-.1,.009,.033),(length-.08,-.012,0),(length-.08,.012,0)],[(0,1,4),(0,4,3),(0,3,5),(0,5,1),(1,2,4),(4,2,3),(3,2,5),(5,2,1)],'edge')
    for a in (0,math.tau/3,math.tau*2/3):
        v=[(.03,0,0),(.145,0,0),(.11,.034,0),(.045,.03,0)]
        o=g.mesh('Split feather',v,[(0,1,2,3)],'thread');o.rotation_euler.x=a
    for o in g.PARTS[start:]:o.location+=Vector(origin)

def bow():
    for sign in (-1,1):
        pts=[(.016,0,sign*.035),(.045,0,sign*.22),(.017,0,sign*.43),(-.065,0,sign*.62),(-.145,0,sign*.72)]
        g.tube('Ash limb',pts,[.028,.03,.025,.016,.009],'ashwood',8,.62)
        g.tube('Heartwood ridge',[(x+.012,y-.014,z) for x,y,z in pts],[.006,.006,.005,.003,.002],'thread',5)
        g.ring('Horn tip',(-.14,0,sign*.708),.014,.004,'bone','Z',12)
    g.tube('Grip',[(.016,0,-.08),(.016,0,.08)],.031,'leather',8,.7)
    for i in range(8):g.ring('Grip binding',(.016,0,-.07+i*.02),.03,.003,'thread','Z',12)
    g.group('String.Upper');g.tube('Linen string upper',[(-.145,0,.72),(-.19,0,0)],.0028,'thread',5)
    g.group('String.Lower');g.tube('Linen string lower',[(-.19,0,0),(-.145,0,-.72)],.0028,'thread',5)
    g.group('NockedArrow');arrow((-.19,-.035,0))

def quiver():
    g.lathe('Leather quiver',[(-.39,.065,.055),(-.34,.075,.06),(.08,.087,.065),(.12,.085,.062)],'leather',16,fold=.02,open_top=True)
    for z in (-.34,.07,.115):g.ring('Bound rim',(0,0,z),.081,.01,'ashwood','Z',16)
    g.box('Reinforcement',(0,-.062,-.12),(.055,.018,.4),'leather')
    for i in range(9):g.ico('Seam stitch',(.034,-.067,-.31+i*.04),(.003,.003,.009),'thread')
    g.box('Strap buckle',(.078,0,-.18),(.018,.072,.06),'brass')
    for i,(x,y,z) in enumerate([(-.035,-.025,-.3),(.025,-.02,-.28),(-.025,.018,-.33),(.03,.03,-.31)]):
        start=len(g.PARTS);arrow(length=.7)
        matrix=Matrix.Translation((x,y,z+.7))@Matrix.Rotation(math.pi/2,4,'Y')
        g.transform(g.PARTS[start:],matrix)

def flame(center,scale=1):
    fire=m.effect('Ember','DC661F',1.4,.82);core=m.effect('FireCore','FFC66C',2.3,1)
    x,y,z=center
    g.tube('Amber flame',[(x,y,z),(x-.015*scale,y,z+.09*scale),(x+.03*scale,y,z+.20*scale),(x+.01*scale,y,z+.31*scale)],[.058*scale,.063*scale,.031*scale,.001*scale],fire,7)
    g.tube('Pale heart',[(x,y-.025*scale,z),(x+.01*scale,y-.025*scale,z+.08*scale),(x-.015*scale,y-.025*scale,z+.18*scale)],[.035*scale,.031*scale,.001*scale],core,6)

def torch():
    g.tube('Split oak haft',[(0,0,-.21),(.006,0,.24)],[.019,.027],'ashwood',7)
    for z in (.13,.17,.21):g.ring('Pitch cloth',(0,0,z),.045,.014,'soot','Z',12)
    for y in (-.038,.038):g.tube('Iron basket',[(0,y,.13),(0,y,.27)],.006,'iron',5)
    g.group('Flame');flame((0,0,.245))

def potion():
    g.lathe('Opaque glazed flask',[(0,.048,.034),(.016,.078,.052),(.10,.082,.055),(.16,.052,.043),(.19,.025,.025),(.23,.026,.026)],'mana',16)
    for z,r in ((.016,.06),(.177,.03),(.229,.03)):g.ring('Pewter collar',(0,0,z),r,.005,'edge','Z',16)
    g.lathe('Cork',[(.223,.022,.022),(.26,.024,.024)],'ashwood',10)
    g.tube('Neck cord',[(0,-.031,.218),(.026,-.047,.14)],.004,'thread',5)
    g.ico('Wax seal',(.029,-.051,.133),(.024,.007,.025),'seal',2)
    g.mesh('Mana lozenge',[(-.02,-.058,.093),(0,-.058,.064),(.02,-.058,.093),(0,-.058,.122)],[(0,1,2,3)],'thread')

def vest(goblin=False):
    material='leather' if goblin else 'linen'
    torso=g.lathe('Vest torso',[(0,.125,.22),(.07,.117,.20),(.18,.127,.21),(.30,.14,.23),(.40,.10,.215)],material,32,fold=.055,open_top=True,open_bottom=True)
    for v in torso.data.vertices:
        if v.co.z>.39:v.co.z-=.10*abs(v.co.x/.11)
    top=[tuple(v.co) for v in torso.data.vertices[-32:]]
    g.tube('Bound neckline',top+[top[0]],.007,'leather' if goblin else 'thread',5)
    # Neck opening and open shoulder straps distinguish vest from generic solid tunic.
    for sign in (-1,1):
        g.tube('Shoulder yoke',[(.11,sign*.15,.33),(.055,sign*.20,.46),(-.07,sign*.20,.46),(-.12,sign*.15,.35)],.035,material,6,1.25)
        g.tube('Hem binding',[(.122,sign*.02,.01),(.1,sign*.18,.025),(-.08,sign*.205,.02)],.008,'thread',6)
    for i in range(5):
        z=.16+i*.041
        g.tube('Cross lacing',[(.144,-.035,z),(.15,.035,z+.032)],.0035,'thread',5)
        g.tube('Cross lacing',[(.144,.035,z),(.15,-.035,z+.032)],.0035,'thread',5)
    if goblin:
        for y in (-.17,-.085,0,.085,.17):
            g.box('Overlapping armor scales',(.141,y,.20),(.023,.075,.14),'iron',.012)
        for sign in (-1,1):
            g.ico('Shoulder rivet',(.092,sign*.2,.43),(.012,.012,.012),'brass')
            g.tube('Jagged horn',[(.01,sign*.22,.44),(.02,sign*.3,.52)],[.025,.001],'bone',5)
    else:
        patch=g.box('Mended patch',(.124,.126,.096),(.006,.06,.069),'leather',.002)
        for z in (.07,.092,.114):g.tube('Patch stitches',[(.13,.09,z),(.132,.11,z+.008)],.002,'thread',4)

def pants():
    g.lathe('Waist',[(.46,.105,.185),(.56,.11,.188),(.65,.105,.18),(.69,.10,.18)],'linen',24,fold=.06,open_top=True,open_bottom=True)
    for sign in (-1,1):
        g.lathe('Trouser leg',[(0,.064,.068),(.10,.075,.069),(.22,.082,.081),(.27,.08,.076),(.33,.083,.09),(.44,.097,.094),(.55,.10,.095)],'linen',24,center=(0,sign*.10,0),fold=.095,open_top=True,open_bottom=True)
        g.tube('Leg seam',[(.064,sign*.12,.04),(.082,sign*.15,.24),(.1,sign*.17,.47)],.004,'thread',5)
        g.ring('Ankle tie',(0,sign*.10,.06),.068,.006,'leather','Z',16)
    g.tube('Waist cord',[(.112,-.17,.69),(.112,0,.684),(.112,.17,.69)],.008,'leather',5)
    for sign in (-1,1):g.tube('Hanging drawstring',[(.114,0,.685),(.115,sign*.031,.6)],.003,'thread',5)

def scroll():
    # Curved parchment middle with tightly curled ends, a simple embossed rising ray.
    verts=[]
    for j in range(9):
        z=j*.035;x=.012*math.sin(j*math.pi/8)
        verts.extend([(x,-.105,z),(x,.105,z)])
    g.mesh('Unfurled parchment',verts,[(j*2,j*2+1,j*2+3,j*2+2) for j in range(8)],'parchment')
    for z in (0,.28):g.tube('Rolled parchment',[(0,-.127,z),(0,.127,z)],.025,'parchment',12)
    for z in (.09,.13,.17):g.tube('Ink lines',[(.018,-.06,z),(.018,.058,z+.003)],.0025,'leather',4)
    g.tube('Ray sigil',[(.017,0,.19),(.017,0,.245)],.007,'brass',5)
    g.ico('Wax seal',(.021,0,.052),(.008,.024,.024),'seal',2)

def wings():
    for s in (-1,1):
        vertices=[(0,0,.025),(.12,s*.15,.06),(.095,s*.30,.1),(.03,s*.25,.017),(-.025,s*.215,.012),(-.055,s*.24,.02),(-.055,s*.14,.02),(-.115,s*.08,.01),(-.1,s*.015,.01)]
        faces=[(0,i,i+1) for i in range(1,len(vertices)-1)]
        g.mesh('Desiccated membrane',vertices,faces,'membrane')
        for j in (2,5,7):g.tube('Wing finger',[vertices[0],vertices[1],vertices[j]],[.009,.006,.002],'bone',5)
        g.tube('Wing edge',[vertices[i] for i in (2,3,4,5,6,7,8)],[.003]*7,'leather',4)
    g.tube('Binding cord',[(0,-.02,.042),(0,.025,.042)],.009,'thread',5)

def iron_ring():
    g.ring('Forged iron ring',(0,0,.019),.015,.004,'iron','Y',24)
    g.ico('Worn signet',(0,-.001,.035),(.010,.006,.007),'edge',1)
    g.box('Signet inset',(0,-.005,.036),(.008,.003,.004),'soot',.0005)

def key():
    g.ring('Open bow',(0,0,.205),.038,.009,'iron','Y',20)
    g.tube('Key shank',[(0,0,.02),(0,0,.17)],.010,'iron',8)
    for z in (.021,.047):g.box('Key tooth',(.02,0,z),(.036,.018,.015),'edge',.002)
    g.ring('Collar',(0,0,.139),.014,.004,'rust','Z',12)

def robe():
    g.lathe('Bell hem',[(0,.19,.30),(.03,.20,.31),(.3,.16,.25),(.65,.13,.19),(.91,.15,.24),(1.08,.11,.23)],'robe',32,fold=.095)
    for s in (-1,1):
        g.tube('Hanging sleeve',[(0,s*.2,1.0),(.01,s*.35,.89),(.065,s*.45,.65)],[.09,.105,.145],'robe',12)
        g.tube('Sleeve piping',[(.07,s*.23,1.0),(.085,s*.37,.85),(.14,s*.45,.62)],.009,'leather',6)
    g.lathe('Raised hood',[(1.03,.09,.125),(1.12,.14,.15),(1.29,.125,.13),(1.37,.08,.08),(1.40,.025,.03)],'robe',24,center=(-.04,0,0),fold=.03)
    # Front inset is a cloth shadow, not a face.
    g.ico('Hood hollow',(.103,0,1.215),(.014,.095,.115),'soot',2)
    rim=[(.106,.1*math.sin(i*math.tau/24),1.215+.12*math.cos(i*math.tau/24)) for i in range(25)]
    g.tube('Hood face binding',rim,.008,'linen',5)
    g.tube('Rope belt',[(.13,-.19,.69),(.145,0,.68),(.13,.19,.69)],.012,'leather',6)
    g.ring('Belt clasp',(.157,0,.68),.026,.005,'brass','X',16)
    g.tube('Stole left',[(.112,-.09,1.06),(.159,-.105,.9),(.15,-.07,.50),(.17,-.08,.2)],.014,'linen',6,.6)
    g.tube('Stole right',[(.112,.09,1.06),(.159,.105,.9),(.15,.07,.5),(.17,.08,.2)],.014,'linen',6,.6)

def sack():
    g.lathe('Gathered sack',[(0,.11,.12),(.055,.22,.19),(.21,.20,.17),(.31,.125,.1),(.35,.07,.06),(.40,.095,.08)],'linen',24,fold=.095)
    for z in (.325,.342):g.ring('Rope knot',(0,0,z),.079,.01,'leather','Z',16)
    for s in (-1,1):g.tube('Rope ends',[(.08,0,.34),(.135,s*.05,.27),(.15,s*.06,.15)],.008,'thread',6)
    g.box('Loot patch',(.188,-.025,.17),(.012,.085,.08),'leather')
    for z in (.15,.18,.20):g.tube('Mended seam',[(.198,-.07,z),(.201,-.045,z+.009)],.0025,'thread',4)

def badge():
    material=m.effect('LootSilver','A9C4CC',.6,1)
    g.mesh('Loot diamond',[(0,0,.78),(.14,0,.42),(0,.14,.42),(-.14,0,.42),(0,-.14,.42),(0,0,.06)],[(0,1,2),(0,2,3),(0,3,4),(0,4,1),(5,2,1),(5,3,2),(5,4,3),(5,1,4)],material)
    g.ring('Open halo',(0,0,.41),.174,.005,'brass','Z',24)

def chest(opened):
    # Canonical closed X=1m depth, Y=.7m width; latch faces +X.
    g.box('Dark cavity',(0,0,.25),(.93,.64,.42),'soot',.01)
    for s in (-1,1):
        for j in range(5):
            g.box('Front planks',(s*.47,-.28+j*.14,.25),(.05,.133,.43),'ashwood',.007)
        for j in range(7):g.box('Side planks',(-.42+j*.14,s*.33,.25),(.133,.04,.43),'ashwood',.006)
        for y in (-.275,.275):
            g.box('Vertical iron strap',(s*.501,y,.25),(.018,.045,.44),'iron',.003)
            for z in (.09,.26,.42):g.ico('Forged nail',(s*.514,y,z),(.008,.014,.014),'edge')
    g.box('Lower iron rail',(0,0,.04),(1.02,.715,.045),'iron')
    for x in (-.43,.43):
        for y in (-.27,.27):g.box('Iron feet',(x,y,.025),(.095,.09,.05),'iron',.004)
    for s in (-1,1):
        g.box('Upper rim',(s*.477,0,.475),(.055,.71,.04),'iron',.004)
        g.box('Upper rim',(0,s*.333,.475),(.94,.04,.04),'iron',.004)
    for y in (-.275,.275):g.tube('Hinge pin',[(-.505,y-.038,.49),(-.505,y+.038,.49)],.019,'iron',8)
    # Lid pieces author at rest then rotate around rear (-X) hinge for open state.
    first=len(g.PARTS);g.group('Lid')
    for j in range(9):
        x=-.45+j*.1125;z=.505+.13*math.sin((x+.51)/1.02*math.pi)
        o=g.box('Cambered lid plank',(x,0,z),(.112,.70,.045),'ashwood',.006)
        o.rotation_euler.y=-.13*math.cos((x+.51)/1.02*math.pi)
    for y in (-.275,.275):
        pts=[(-.5+i/12,y,.52+.13*math.sin(i/12*math.pi)) for i in range(13)]
        g.tube('Lid strap',pts,.022,'iron',4,.7)
        for x in (-.42,0,.42):g.ico('Lid rivet',(x,y,.55+.13*math.sin((x+.5)*math.pi)),(.012,.012,.009),'edge')
    g.box('Latch plate',(.51,0,.45),(.028,.11,.16),'iron',.006)
    g.box('Pale latch',(.531,0,.475),(.015,.055,.09),'edge',.004)
    g.box('Keyhole',(.54,0,.477),(.003,.015,.029),'soot',.002)
    if opened:
        hinge=Vector((-.5,0,.49));matrix=Matrix.Translation(hinge)@Matrix.Rotation(math.radians(-106),4,'Y')@Matrix.Translation(-hinge)
        g.transform(g.PARTS[first:],matrix)
        # Empty cavity is physical, no loot coins imply authoritative contents.
        g.PARTS[0].scale.z=.13;g.PARTS[0].location.z=.065
    g.group('Body')

def spiral(name,material,radius=.35,height=.7,turns=1.5,width=.055,phase=0):
    verts=[];count=48
    for i in range(count+1):
        t=i/count;a=phase+t*turns*math.tau;r=radius*(.25+.75*t)
        for edge in (-1,1):
            z=.035+height*t+edge*width*(math.sin(math.pi*t)*.8+.2)
            verts.append((r*math.cos(a),r*math.sin(a),z))
    g.mesh(name,verts,[(2*i,2*i+1,2*i+3,2*i+2) for i in range(count)],material)

def effects(name):
    amber=m.effect('Ember','DC661F',1.4,.82);core=m.effect('FireCore','FFC66C',2.3,1)
    heal=m.effect('HealGold','BFCB90',1.3,.78);cool=m.effect('LightPearl','90C9D4',1.3,.8)
    dust=m.effect('Dust','9B8C6A',.22,.45);spark=m.effect('Spark','DCAA56',1.5,.9)
    rng=random.Random(523)
    if name=='FireDart':
        g.group('Core');g.tube('Dart heart',[(-.36,0,0),(-.10,0,0),(.19,0,0)],[.002,.07,.004],core,7)
        g.group('Flame')
        for i in range(3):
            a=i*math.tau/3;g.tube('Fire wake',[(-.58,.05*math.cos(a),.05*math.sin(a)),(-.22,.09*math.cos(a),.09*math.sin(a)),(.13,0,0)],[.002,.04,.009],amber,6)
    elif name in ('FireImpact','MeleeSpark'):
        for i in range(9 if name=='FireImpact' else 6):
            a=i*math.tau/9;r=rng.uniform(.18,.42);v=Vector((math.cos(a)*r,math.sin(a)*r,rng.uniform(.02,.35)))
            g.tube('Radiant splinter',[(0,0,0),tuple(v*.6),tuple(v)],[.006,.024,.001],amber if name=='FireImpact' and i%2 else spark,5)
        g.ico('Flash center',(0,0,0),(.095,.095,.09),core,1)
    elif name in ('StoneShard','StoneShatter'):
        positions=[(0,0,0)] if name=='StoneShard' else [(rng.uniform(-.3,.3),rng.uniform(-.3,.3),rng.uniform(.05,.4)) for _ in range(8)]
        for i,p in enumerate(positions):
            g.ico('Broken shale',p,(.22,.078,.09) if name=='StoneShard' else (.035+i*.007,.045,.06),'stone',1)
        if name=='StoneShard':
            g.tube('Pale fracture',[(-.16,-.054,.04),(-.06,-.075,.02),(.09,-.058,.045)],.005,'thread',4)
    elif name=='DustDevil':
        for i in range(3):g.group('Swirl.'+str(i));spiral('Dust band',dust,.34,.77,1.2,.038,i*math.tau/3)
        g.group('Grit')
        for i in range(8):
            a=i*2.1;g.ico('Orbiting grit',(.24*math.cos(a),.24*math.sin(a),.08+i*.09),(.022,.028,.022),'stone')
    elif name=='HealLight':
        g.group('Ring');g.ring('Healing ring',(0,0,.045),.38,.013,heal,'Z',32)
        g.group('Motes')
        for i in range(9):
            a=i*2.4;g.ico('Rising mote',(.28*math.cos(a),.28*math.sin(a),.18+(i%4)*.14),(.018,.018,.045),heal,1)
        g.group('Rays')
        for s in (-1,1):g.tube('Open rising ray',[(0,s*.27,.08),(0,s*.20,.17),(0,s*.16,.36)],[.01,.018,.001],heal,5)
    elif name=='Light':
        g.group('Orb');g.ico('Light orb',(0,0,0),(.11,.11,.11),cool,2)
        g.group('Halo');g.ring('Orb halo',(0,0,0),.195,.006,cool,'Y',32)
        for p in ((.28,0,0),(-.28,0,0),(0,0,.28),(0,0,-.28)):
            p=Vector(p);g.tube('Four rays',[tuple(p*.78),tuple(p),tuple(p*1.14)],[.001,.015,.001],cool,4)
    elif name=='SlashArc':
        verts=[]
        for i in range(33):
            t=i/32;a=-1.1+t*2.2;w=.008+.12*math.sin(t*math.pi)
            for r in (.46,.46+w):verts.append((r*math.cos(a),r*math.sin(a),0))
        g.mesh('Crescent slash',verts,[(2*i,2*i+1,2*i+3,2*i+2) for i in range(32)],spark)
    elif name=='ArrowFlight':arrow()
    elif name=='ImpactPuff':
        dust=m.effect('Puff','9B8C6A',0,.18)
        for i in range(7):
            a=i*math.tau/7;g.ico('Dust puff',(.11*math.cos(a),.11*math.sin(a),.07+(i%2)*.08),(.09,.08,.08),dust,1)

BUILDERS={'RustedDirk':blade,'GoblinBlade':lambda:blade(True),'AshwoodFlatbow':bow,'WoodenArrows':quiver,'Torch':torch,'PotionOfMana':potion,'ClothVest':vest,'ClothPants':pants,'LightHeal':scroll,'DecayingBatWings':wings,'GoblinLeatherArmor':lambda:vest(True),'IronRing':iron_ring,'IronKey':key,'FlowingBlackRobe':robe,'LootSack':sack,'CorpseLootBadge':badge,'ChestClosed':lambda:chest(False),'ChestOpen':lambda:chest(True)}

def create(name,atlas):
    g.begin();BUILDERS[name]() if name in BUILDERS else effects(name)
    return g.finish(atlas)
