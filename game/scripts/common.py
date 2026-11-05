"""Shared movement and timing helpers for the R-Type behaviour scripts.

Every function receives the entity handle `e` given by the engine. Values that a
script needs to remember between two frames are stored directly on `e`
(for example `e.base_y`), the engine keeps one handle per entity.
"""

import math

LEFT = 180.0


def start_moving(e, vx, vy=0.0):
    """Gives the entity its default velocity, unless whoever spawned it already set one."""
    if e.vx() == 0.0 and e.vy() == 0.0:
        e.set_velocity(vx, vy)


def sine(e, amplitude, frequency):
    """Oscillates vertically around the height the entity had when it appeared."""
    e.set_velocity(e.vx(), amplitude * frequency * math.cos(frequency * e.age()))


def bounce(e, speed, top=0.0, bottom=1080.0):
    """Moves up and down at `speed`, turning back at the top and bottom limits."""
    if e.y() <= top:
        e.set_velocity(e.vx(), abs(speed))
    elif e.y() + e.height() >= bottom:
        e.set_velocity(e.vx(), -abs(speed))
    elif e.vy() == 0.0:
        e.set_velocity(e.vx(), speed)


def follow_player_y(e, speed):
    """Moves vertically toward the closest player, at most `speed` pixels per second."""
    target = e.nearest_player()
    if target is None:
        e.set_velocity(e.vx(), 0.0)
        return 0.0
    gap = target[1] - e.center_y()
    e.set_velocity(e.vx(), max(-speed, min(speed, gap * 3.0)))
    return gap


def cooldown(e, name, minimum, maximum):
    """Returns True once every random delay between `minimum` and `maximum` seconds."""
    key = "next_" + name
    if not hasattr(e, key):
        setattr(e, key, e.age() + e.world.random(minimum, maximum))
        return False
    if e.age() < getattr(e, key):
        return False
    setattr(e, key, e.age() + e.world.random(minimum, maximum))
    return True


def health_ratio(e):
    """Remaining health between 0 and 1."""
    maximum = e.max_health()
    return e.health() / maximum if maximum > 0 else 0.0


def ring(e, prefab, count, speed, offset=0.0):
    """Fires `count` projectiles evenly spread on a full circle."""
    e.fire_spread(prefab, count, 360.0 - 360.0 / count, 180.0 + offset, speed)
