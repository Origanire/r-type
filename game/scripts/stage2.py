"""Stage 2: heavies, turrets, chasers, a snake and an ambush from behind, then the Warship."""

DURATION = 75.0
BOSS = ("boss_warship", 2000.0, 324.0)
AMBUSH_TIME = 38.0

RANDOM = (0.9, 1.8, [
    ("bydo_straight", 2, 0.0, 1000.0),
    ("bydo_sine", 2, 150.0, 850.0),
    ("pod", 2, 0.0, 1000.0),
    ("chaser", 1, 100.0, 900.0),
    ("carrier", 1, 150.0, 850.0),
])

WAVES = [
    (5.0, "heavy", 2, 1950.0, 300.0, 0.0, 400.0),
    (14.0, "turret", 2, 1950.0, 200.0, 0.0, 550.0),
    (24.0, "chaser", 4, 1950.0, 150.0, 200.0, 220.0),
    (30.0, "snake_head", 1, 1950.0, 500.0, 0.0, 0.0),
    (56.0, "carrier", 2, 1950.0, 250.0, 300.0, 500.0),
    (50.0, "heavy", 3, 1950.0, 150.0, 200.0, 350.0),
    (62.0, "turret", 3, 1950.0, 120.0, 120.0, 380.0),
]


def events(world, time, director):
    if time >= AMBUSH_TIME and not getattr(director, "ambush_done", False):
        director.ambush_done = True
        for index in range(5):
            pod = world.spawn("pod", -150.0 - index * 90.0, 150.0 + index * 180.0)
            pod.set_velocity(420.0, 0.0)
