"""Carrier: a slow ship that drops a power-up when destroyed, unless the "bonuses" room rule is OFF."""

import common

DROPS = (
    ("powerup_double", 3),
    ("powerup_spread", 2),
    ("powerup_heal", 3),
    ("powerup_shield", 2),
    ("powerup_force", 2),
    ("powerup_drone", 2),
)


def on_spawn(e):
    common.start_moving(e, -170.0)


def on_update(e, dt):
    common.sine(e, 70.0, 1.8)


def on_death(e):
    if e.world.rule("bonuses") == "OFF":
        return
    roll = e.world.random(0.0, sum(weight for _, weight in DROPS))
    for prefab, weight in DROPS:
        roll -= weight
        if roll <= 0.0:
            e.spawn(prefab, e.width() / 4.0, e.height() / 4.0)
            return
