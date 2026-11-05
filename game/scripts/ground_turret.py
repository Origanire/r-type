"""Ground turret: scrolls with the terrain, turns its cannon toward the closest player and fires."""

import common

SCROLL_SPEED = 120.0


def on_spawn(e):
    common.start_moving(e, -SCROLL_SPEED)


def on_update(e, dt):
    e.set_angle(e.angle_to_player() + 90.0)
    if common.cooldown(e, "shot", 1.2, 2.0):
        e.fire_at_player("enemy_bullet", 480.0)
