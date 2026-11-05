"""Hopper: waves up and down and shoots straight at the closest player."""

import common


def on_spawn(e):
    common.start_moving(e, -300.0)


def on_update(e, dt):
    common.sine(e, 220.0, 2.0)
    if common.cooldown(e, "shot", 2.0, 3.5):
        e.fire_at_player("enemy_bullet", 520.0)
