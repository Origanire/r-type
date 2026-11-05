"""Stage 3: a terrain tunnel with ground turrets, splitters and mine fields, a heavy squad, then the Core."""

DURATION = 90.0
BOSS = ("boss_core", 2000.0, 392.0)
MINE_FIELD_TIME = 33.0
SQUAD_TIME = 55.0
TERRAIN_TIME = 3.0
FLOOR_Y = 960.0
CEILING_Y = 80.0

RANDOM = (0.6, 1.3, [
    ("bydo_sine", 2, 150.0, 850.0),
    ("pod", 3, 0.0, 1000.0),
    ("hopper", 2, 200.0, 800.0),
    ("chaser", 2, 100.0, 900.0),
    ("splitter", 1, 150.0, 850.0),
    ("carrier", 1, 200.0, 800.0),
])

WAVES = [
    (5.0, "heavy", 3, 1950.0, 150.0, 0.0, 350.0),
    (15.0, "splitter", 3, 1950.0, 200.0, 220.0, 280.0),
    (25.0, "turret", 3, 1950.0, 100.0, 100.0, 400.0),
    (45.0, "chaser", 6, 1950.0, 100.0, 120.0, 160.0),
    (70.0, "splitter", 4, 1950.0, 150.0, 180.0, 220.0),
    (80.0, "hopper", 8, 1950.0, 150.0, 100.0, 100.0),
]


def events(world, time, director):
    if time >= TERRAIN_TIME and not getattr(director, "terrain_done", False):
        director.terrain_done = True
        world.spawn("terrain_stage3", 1920.0, 0.0)
    if time >= TERRAIN_TIME + 4.0 and world.every("ground_turret", 6.0) and world.can_spawn():
        world.spawn("ground_turret", 1950.0, FLOOR_Y - 80.0)
    if time >= MINE_FIELD_TIME and not getattr(director, "mines_done", False):
        director.mines_done = True
        for row in range(4):
            for column in range(3):
                world.spawn("mine", 1950.0 + column * 260.0, 120.0 + row * 240.0 + column * 60.0)
    if time >= SQUAD_TIME and not getattr(director, "squad_done", False):
        director.squad_done = True
        for index in range(4):
            world.spawn("heavy", 1950.0 + index * 160.0, 100.0 + index * 240.0)
