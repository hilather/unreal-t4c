"""Presentation-only roster and A-01 clearance rulers (metres).

W5-08e, LH_Prototype_v1, 2026-10-10, source_url null. These are art
constraints from docs/implementation/art/creatures, never gameplay tuning.
"""
import importlib

PILOT = ('rat', 'bat', 'slime')
BATCH2 = ('goblin', 'giant_spider', 'balork', 'goblin_warrior', 'atrocity',
          'dungeon_bat', 'giant_bat', 'undead_bat')
KINDS = PILOT + BATCH2
# rear, front, half width, rest ceiling, animated ceiling
RULERS = {
    'rat': (.675, .225, .14, .25, .35),
    'bat': (.225, .225, .4, 1.45, 1.45),
    'slime': (.45, .45, .45, .4, .6),
    'goblin': (.45, .65, .425, 1.85, 2),
    'giant_spider': (.65, .85, .85, .65, .9),
    'balork': (1, 1, 2.2, 3.1, 3.1),
    'goblin_warrior': (.5, .7, .525, 2.1, 2.15),
    'atrocity': (.65, .95, .9, 1.9, 2.1),
    'dungeon_bat': (.275, .275, .5, 1.55, 1.55),
    'giant_bat': (.425, .425, .85, 1.9, 1.9),
    'undead_bat': (.3, .3, .55, 1.65, 1.65),
}

def limits(kind, clip=None):
    rear, front, half, rest, live = RULERS[kind]
    if clip == 'death' and kind in ('goblin', 'goblin_warrior'):
        rear = front = .95 if kind == 'goblin' else 1.05
        half = .5 if kind == 'goblin' else .55
    return ((-rear, -half, 0 if clip is None else -.0007),
            (front, half, rest if clip is None else live))

def budget(kind):
    return 20000 if kind == 'balork' else (8000 if kind in PILOT else 10000)

def module(kind):
    name = ('models' if kind in PILOT else 'humanoids' if kind in
            ('goblin', 'goblin_warrior', 'atrocity') else
            'spider' if kind == 'giant_spider' else
            'balork' if kind == 'balork' else 'bat_variants')
    return importlib.import_module(name)

def create(kind):
    return module(kind).create(kind)

def hero(kind):
    """Framing only; gameplay camera always uses the inherited fixed view."""
    if kind == 'rat': return ((-.10, 0, .13), 1.25, (1.4, -1.8, 1.13))
    if kind in ('bat', 'dungeon_bat', 'giant_bat', 'undead_bat'):
        z = {'bat':1.075,'dungeon_bat':1.10,'giant_bat':1.3,'undead_bat':1.15}[kind]
        width = {'bat':1.12,'dungeon_bat':1.45,'giant_bat':2.45,'undead_bat':1.6}[kind]
        return ((0, 0, z), width, (2.5, -.75, z+.72))
    if kind == 'slime': return ((0, 0, .15), 1.25, (1.4, -1.8, 1.15))
    if kind == 'giant_spider': return ((.02, 0, .3), 2.35, (2.5, -2, 1.7))
    if kind == 'balork': return ((0, 0, 1.55), 6.3, (6, -3.5, 3.6))
    if kind == 'atrocity': return ((0, 0, 1.00), 4.0, (3.6, -2.6, 2.2))
    return ((0, 0, .93), 3.7, (3.8, -2.8, 2.0))
