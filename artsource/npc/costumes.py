"""Low-poly role layers fitted to the existing player rest bodies.
Original costume extrapolation, not historically verified NPC apparel.
"""
import math


def create(m, outfit, extras):
    m.PARTS.clear()
    if outfit in ('priest', 'scholar'):
        robe(m, outfit)
    elif outfit == 'coat':
        coat(m)
    elif outfit == 'apron':
        apron(m)
    elif outfit == 'tabard':
        tabard(m)
    elif outfit == 'ranger':
        mantle(m, short=True)
        panel(m, 'ranger jerkin', [(1.40,.15,.13),(1.30,.164,.16),(1.09,.139,.13),(.98,.153,.16)], 'Accent')
    elif outfit == 'traveller':
        mantle(m)
    for extra in extras:
        {'stole':stole, 'hood':hood, 'satchel':satchel, 'book':book,
         'staff':staff, 'spear':spear, 'hammer':hammer, 'bow':bow,
         'sword':sword, 'bottles':bottles, 'medallion':medallion,
         'beard':beard, 'longbeard':lambda m:beard(m, True),
         'cap':cap, 'coif':coif}[extra](m)
    return list(m.PARTS)


def panel(m, name, rows, mat, ycenter=0, bone=None):
    # Curved front sheet (z, forward x, half width); solid edging is modeled.
    verts=[];faces=[];weights=[]
    for z,x,width in rows:
        for j in range(9):
            t=j/8*2-1
            verts.append((x-.045*t*t+.004*math.cos(t*13+z*7),ycenter+t*width,z))
            weights.append({bone:1} if bone else m.weights(z))
    for i in range(len(rows)-1):
        for j in range(8):
            a=i*9+j;faces.append((a,a+9,a+10,a+1))
    ob=m.mesh(name,verts,faces,mat,weights=weights)
    for side in (-1,1):
        points=[(x-.045,ycenter+side*w,z) for z,x,w in rows]
        m.tube(name+' edging',points,.005,'Trim',bone or 'pelvis',sides=5,
               weights=[{bone:1} if bone else m.weights(z) for z,_,_ in rows])
    return ob


def robe(m, kind):
    rows=[(.075,0,0,.221,.285),(.12,0,0,.224,.285),(.27,0,0,.206,.261),
          (.46,0,0,.183,.242),(.67,0,0,.166,.220),(.88,0,0,.146,.198),
          (1.035,0,0,.13,.178),(1.09,0,0,.133,.181),(1.24,0,0,.149,.212),
          (1.36,0,0,.134,.219),(1.43,0,0,.087,.202),(1.475,0,0,.061,.064)]
    m.loft(kind+' robe',rows,'Cloth',sides=48,fold=.008,cap=False,
           weights=[m.weights(z) for z,*_ in rows])
    m.ring('embroidered hem',.116,.234,.294,'Trim',r=.009,sides=48)
    m.ring('lower hem seam',.087,.232,.293,'Accent',r=.005,sides=48)
    # Raglan sleeves narrow toward the elbow then open into modest bells.
    for s,side in [(1,'L'),(-1,'R')]:
        rows=[(1.425,-.003,s*.190,.071,.080),(1.37,-.003,s*.218,.09,.086),
              (1.29,0,s*.25,.084,.081),(1.19,0,s*.283,.078,.080),
              (1.13,.007,s*.30,.074,.081),(1.05,.01,s*.322,.079,.088),
              (.975,.018,s*.342,.086,.095)]
        m.loft('bell sleeve '+side,rows,'Cloth',sides=24,fold=.005,cap=False,
               weights=[{'upper_arm.'+side:1} if z>1.2 else {'forearm.'+side:1} for z,*_ in rows])
        m.ring('sleeve braid '+side,.985,.090,.098,'Trim','forearm.'+side,cx=.018,cy=s*.342,r=.008)
    m.ring('rope cincture',1.065,.143,.19,'Trim',r=.012)
    for s in (-1,1):
        m.tube('hanging girdle',[(.156,s*.07,1.06),(.174,s*.078,.9),(.19,s*.06,.72)],[.009,.009,.014],'Trim','pelvis')
    if kind=='scholar':
        mantle(m,short=True)
        panel(m,'scholar central facing',[(1.42,.118,.055),(1.28,.16,.06),(1.10,.15,.052),(.86,.175,.06),(.49,.201,.071),(.17,.24,.082)],'Accent')


def stole(m):
    for s in (-1,1):
        rows=[(1.465,.070,.032),(1.41,.132,.035),(1.29,.170,.036),(1.13,.157,.037),(.99,.158,.040),(.79,.180,.043),(.48,.214,.044)]
        panel(m,'priest stole',rows,'Accent',s*.097)
        for z in (.51,.55):
            m.tube('stole gold bars',[(.217,s*.097-.04,z),(.224,s*.097+.04,z)],.006,'Trim','pelvis',sides=6)
        for j in range(5):
            y=s*.097+(j-2)*.015
            m.tube('stole fringe',[(.217,y,.486),(.218,y,.45)],.0025,'Trim','pelvis',sides=5)


def mantle(m, short=False):
    # Open-front shoulder cape, avoiding the face and right-hand props.
    rows=[(1.47,.073,.078),(1.425,.113,.22),(1.345,.165,.29),
          (1.26 if short else 1.1,.175 if short else .205,.30 if short else .31),
          (1.23 if short else .78,.18 if short else .22,.29 if short else .29)]
    v=[];f=[];w=[];sides=34
    for z,rx,ry in rows:
        for j in range(sides+1):
            a=.65+(math.tau-1.3)*j/sides
            v.append((rx*math.cos(a),ry*math.sin(a),z+.006*math.cos(j*2)))
            w.append(m.weights(z))
    for i in range(len(rows)-1):
        for j in range(sides):
            a=i*(sides+1)+j;f.append((a,a+1,a+sides+2,a+sides+1))
    m.mesh('shoulder mantle',v,f,'Accent',weights=w)
    for j in (0,sides):
        m.tube('mantle binding',[v[i*(sides+1)+j] for i in range(len(rows))],.006,'Trim',weights=[m.weights(z) for z,*_ in rows])
    m.tube('mantle clasp',[(.092,-.049,1.454),(.105,.049,1.454)],.007,'Metal','chest')


def coat(m):
    # Open, knee-length coat with broad turned lapels.
    rows=[(.49,.158,.215),(.68,.155,.203),(.89,.140,.195),(1.07,.129,.178),
          (1.23,.155,.207),(1.39,.14,.22),(1.455,.064,.084)]
    v=[];f=[];w=[];sides=40
    for z,rx,ry in rows:
        for j in range(sides+1):
            a=.29+(math.tau-.58)*j/sides
            v.append((rx*math.cos(a)+.005*math.sin(a*13+z*7),ry*math.sin(a),z))
            w.append(m.weights(z))
    for i in range(len(rows)-1):
        for j in range(sides):
            a=i*(sides+1)+j;f.append((a,a+1,a+sides+2,a+sides+1))
    m.mesh('merchant coat',v,f,'Cloth',weights=w)
    for s in (-1,1):
        panel(m,'turned lapel',[(1.455,.087,.025),(1.35,.161,.055),(1.2,.166,.046),(1.05,.142,.025)],'Accent',s*.085)
        m.tube('coat opening seam',[(rx*math.cos(.29)+.007,s*ry*math.sin(.29),z) for z,rx,ry in rows],.007,'Trim',weights=[m.weights(z) for z,*_ in rows])
    for z in (1.09,1.17,1.25):
        m.ellipsoid('coat button',(.163,-.061,z),(.005,.008,.008),'Metal',seg=8,rings=6)


def apron(m):
    panel(m,'smith apron',[(1.39,.142,.115),(1.25,.163,.132),(1.075,.153,.155),(.89,.173,.173),(.68,.181,.171),(.56,.169,.147)],'Accent')
    for s in (-1,1):
        m.tube('apron shoulder strap',[(.147,s*.10,1.38),(.087,s*.114,1.45),(-.075,s*.113,1.44),(-.136,s*.12,1.16)],.017,'Leather','chest')
    panel(m,'apron pocket',[(.98,.183,.088),(.84,.195,.09)],'Leather')
    m.ring('apron belt',1.07,.151,.19,'Leather',r=.014)


def tabard(m):
    panel(m,'front tabard',[(1.45,.09,.137),(1.38,.153,.17),(1.25,.158,.174),(1.09,.144,.14),(.94,.163,.165),(.74,.184,.17),(.64,.17,.15)],'Accent')
    for s,side in [(1,'L'),(-1,'R')]:
        m.ellipsoid('leather pauldron',(0,s*.212,1.405),(.11,.105,.055),'Leather','upper_arm.'+side,seg=16,rings=8)
        m.tube('pauldron rim',[(.091,s*.145,1.424),(.109,s*.23,1.417),(.07,s*.302,1.385)],.006,'Metal','upper_arm.'+side)
    m.ring('tabard belt',1.052,.157,.19,'Leather',r=.018)
    medallion(m)


def hood(m):
    # Open cowl: back/top shell, rolled opening leaves the entire face visible.
    v=[];f=[];sides=32;rings=12
    for i in range(rings+1):
        t=i/rings*math.pi
        for j in range(sides+1):
            a=.59+(math.tau-1.18)*j/sides
            v.append((-.02+.16*math.sin(t)*math.cos(a),.142*math.sin(t)*math.sin(a),1.642+.182*math.cos(t)))
    for i in range(rings):
        for j in range(sides):
            a=i*(sides+1)+j;f.append((a,a+1,a+sides+2,a+sides+1))
    m.mesh('open hood',v,f,'Accent','head')
    for j in (0,sides):
        m.tube('hood rolled edge',[v[i*(sides+1)+j] for i in range(1,rings)],.011,'Trim','head',sides=7)


def beard(m, long=False):
    rows=[(1.61,.085,.067),(1.58,.107,.062),(1.54,.105,.055),(1.50,.081,.045),(1.42 if long else 1.485,.057,.012)]
    v=[];f=[]
    for z,x,w in rows:
        for j in range(17):
            t=j/8-1
            v.append((x-.035*t*t+.003*math.cos(j*math.pi*.85),t*w,z+.005*math.cos(j*1.8)))
    for i in range(len(rows)-1):
        for j in range(16):
            a=i*17+j;f.append((a,a+17,a+18,a+1))
    m.mesh('sculpted beard',v,f,'Hair','head')
    for s in (-1,1):
        m.tube('moustache',[(.10,0,1.586),(.105,s*.021,1.580),(.096,s*.038,1.565)],[.005,.009,.002],'Hair','head')


def satchel(m):
    m.tube('crossbody strap',[(.09,.168,1.426),(.169,.075,1.29),(.163,-.04,1.14),(.141,-.195,.965)],.018,'Leather',weights=[m.weights(z) for z in (1.426,1.29,1.14,.965)],sides=6)
    m.ellipsoid('satchel',(.085,-.217,.857),(.065,.092,.111),'Leather','pelvis',seg=16,rings=10)
    m.ellipsoid('satchel flap',(.142,-.219,.909),(.016,.089,.055),'Edge','pelvis',seg=16,rings=8)
    m.tube('satchel buckle',[(.164,-.238,.892),(.164,-.205,.892),(.166,-.205,.863),(.166,-.238,.863),(.164,-.238,.892)],.0035,'Metal','pelvis',sides=5)


def box(m,name,center,size,mat,bone='pelvis'):
    x,y,z=center;dx,dy,dz=[v/2 for v in size]
    verts=[(x+a*dx,y+b*dy,z+c*dz) for a,b,c in [(-1,-1,-1),(-1,-1,1),(-1,1,-1),(-1,1,1),(1,-1,-1),(1,-1,1),(1,1,-1),(1,1,1)]]
    return m.mesh(name,verts,[(0,2,3,1),(4,5,7,6),(0,1,5,4),(2,6,7,3),(0,4,6,2),(1,3,7,5)],mat,bone)


def book(m):
    box(m,'book pages',(.025,.227,1.02),(.105,.077,.184),'Paper')
    for y in (.183,.271):
        box(m,'book cover',(.025,y,1.02),(.119,.011,.201),'Accent')
    box(m,'book spine',(-.037,.227,1.02),(.013,.092,.201),'Leather')
    for z in (.956,1.085):
        box(m,'book brass corner',(.083,.271,z),(.018,.014,.027),'Metal')


def staff(m):
    p=(.055,-.342)
    m.tube('staff shaft',[(p[0],p[1],.043),(p[0],p[1],1.94)],[.018,.014],'Wood','hand.R',sides=10)
    for z in (.83,.98,1.90):
        m.ring('staff ferrule',z,.019,.019,'Metal','hand.R',cx=p[0],cy=p[1],r=.004,sides=14)
    pts=[(p[0],p[1]+.075*math.cos(j*math.tau/24),2.01+.075*math.sin(j*math.tau/24)) for j in range(25)]
    m.tube('staff ring finial',pts,.010,'Metal','hand.R',sides=6)
    m.ellipsoid('staff amber inset',(p[0],p[1],2.01),(.018,.034,.034),'Trim','hand.R',seg=12,rings=8)


def spear(m):
    x,y=.055,-.342
    m.tube('ash spear haft',[(x,y,.043),(x,y,1.965)],.015,'Wood','hand.R',sides=10)
    m.mesh('leaf spearhead',[(x,y,2.22),(x,y-.054,2.06),(x+.012,y,2.00),(x,y+.054,2.06),(x-.012,y,2.00)],[(0,1,2),(0,2,3),(0,3,4),(0,4,1),(1,4,2),(2,4,3)],'Iron','hand.R')
    m.tube('spear socket',[(x,y,1.90),(x,y,2.03)],.023,'Iron','hand.R',sides=10)


def hammer(m):
    m.tube('hammer haft',[(.018,-.24,.81),(.018,-.24,1.11)],.016,'Wood','pelvis',sides=8)
    box(m,'hammer head',(.018,-.24,1.105),(.16,.06,.068),'Iron')


def sword(m):
    m.tube('sword scabbard',[(.055,-.231,1.0),(-.11,-.256,.43)],[.025,.016],'Leather','pelvis',sides=8)
    m.tube('sword grip',[(.06,-.23,1.035),(.094,-.225,1.16)],.020,'Leather','pelvis')
    m.tube('sword guard',[(.024,-.31,1.025),(.069,-.158,1.025)],.010,'Iron','pelvis')
    m.ellipsoid('sword pommel',(.096,-.225,1.17),(.024,.025,.025),'Metal','pelvis',seg=10,rings=6)


def bow(m):
    # Back-carried bow and quiver; gesture hand stays free.
    p=[(-.19-.15*math.sin(math.pi*j/20),-.22+.025*math.sin(j*.3),.51+j/20*.97) for j in range(21)]
    m.tube('bow stave',p,[.008+.012*math.sin(math.pi*j/20) for j in range(21)],'Wood','chest',sides=8)
    m.tube('bow string',[p[0],p[-1]],.0018,'Stitch','chest',sides=5)
    m.loft('quiver',[(.92,-.191,.135,.054,.054),(1.38,-.191,.135,.071,.060)],'Leather','chest',sides=16,cap=False)
    for j in range(4):
        x=-.195+(j%2)*.025;y=.108+(j//2)*.025;z=1.52+(j%3)*.024
        m.tube('arrow shaft',[(x,y,1.13),(x,y,z)],.003,'Wood','chest',sides=5)
        m.mesh('arrow fletching',[(x,y,z),(x,y,z-.08),(x-.027,y,z-.064),(x-.022,y,z+.005)],[(0,1,2,3)],'Paper','chest')


def bottles(m):
    for j in range(3):
        y=(j-1)*.075
        m.loft('ceramic potion flask',[(.87,.185,y,.026,.026),(.9,.185,y,.031,.032),(.96,.185,y,.028,.028),(.975,.185,y,.012,.012),(1.0,.185,y,.011,.011)],'Accent' if j%2 else 'Trim',sides=12)
        m.loft('flask stopper',[(.994,.185,y,.013,.013),(1.015,.185,y,.013,.013)],'Wood',sides=10)


def medallion(m):
    m.tube('medallion cord',[(.063,-.075,1.46),(.153,0,1.29),(.063,.075,1.46)],.003,'Metal','chest',sides=6)
    m.ellipsoid('Artherk sun medallion',(.168,0,1.283),(.007,.025,.028),'Metal','chest',seg=16,rings=8)
    for j in range(8):
        a=j*math.tau/8
        m.tube('sun ray',[(.171,.027*math.cos(a),1.283+.03*math.sin(a)),(.171,.038*math.cos(a),1.283+.041*math.sin(a))],.002,'Metal','chest',sides=5)


def cap(m):
    m.loft('felt cap',[(1.694,-.017,0,.12,.101),(1.758,-.020,0,.12,.102),(1.807,-.035,0,.084,.078),(1.823,-.048,0,.015,.016)],'Accent','head',sides=32,fold=.003)
    m.ring('cap band',1.711,.123,.103,'Trim','head',cx=-.017,r=.014)


def coif(m):
    m.loft('linen head wrap',[(1.69,-.016,0,.116,.10),(1.735,-.025,0,.111,.097),(1.793,-.038,0,.073,.065),(1.81,-.038,0,.009,.01)],'Trim','head',sides=32,fold=.002)
    m.tube('coif knot',[(-.11,.015,1.721),(-.139,.025,1.633),(-.125,.038,1.533)],[.025,.024,.012],'Trim','head',sides=10)
