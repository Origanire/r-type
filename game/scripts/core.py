"""Core, final boss of stage 3.

Phase 1 (above 50 %): circles around its post and fires a rotating spiral.
Phase 2 (above 20 %): adds rings of 12 orbs and a second spiral turning the other way.
Phase 3:              raises a shield (invulnerable 2 s) every few seconds, then
                      answers with fast double rings.
"""

import math

import boss_common
import common

RADIUS = 200.0
TURN_SPEED = 0.9


def on_update(e, dt):
    if not boss_common.enter(e):
        return
    angle = e.age() * TURN_SPEED
    e.set_velocity(-RADIUS * TURN_SPEED * math.sin(angle), RADIUS * TURN_SPEED * math.cos(angle))
    current = boss_common.phase(e, (0.5, 0.2))
    if e.every("spiral", 0.09):
        e.spin = getattr(e, "spin", 0.0) + 17.0
        e.fire("boss_orb", e.spin, 320.0)
        if current >= 2:
            e.fire("boss_orb", -e.spin * 1.3, 300.0)
    if current >= 2 and common.cooldown(e, "ring", 1.4, 2.0):
        common.ring(e, "enemy_bullet", 12, 360.0, e.world.random(0.0, 30.0))
    if current == 3 and common.cooldown(e, "shield", 6.0, 8.0):
        e.set_invulnerable(2.0)
        common.ring(e, "boss_orb", 16, 460.0)
        common.ring(e, "boss_orb", 16, 330.0, 11.25)
