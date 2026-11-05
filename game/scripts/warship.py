"""Warship, boss of stage 2.

Phase 1 (above 50 %): stays at its post and sends walls of lasers covering the whole
                      screen, with one gap the players must reach in time.
Phase 2:              lines up with a player, warns with a blinking "!",
                      then fires a huge straight beam, and rests before the next one.
"""

import math

import boss_common
import common

AIM_SECONDS = 1.6
WARN_SECONDS = 1.0
BLINK_SECONDS = 0.15
BEAM_SECONDS = 1.2
BEAM_GAP = 0.06
BEAM_SPEED = 1400.0
REST_SECONDS = 1.8
WALL_EVERY = 2.2
WALL_SPEED = 800.0
ROW_SPACING = 50.0
GAP_ROWS = 4


def on_update(e, dt):
    if not boss_common.enter(e):
        return
    if boss_common.phase(e, (0.5,)) == 1:
        e.set_velocity(0.0, 50.0 * math.cos(e.age()))
        if e.every("wall", WALL_EVERY):
            laser_wall(e)
        return
    beam_attack(e)


def laser_wall(e):
    world = e.world
    rows = int(world.height() // ROW_SPACING)
    gap_start = int(world.random(1.0, float(rows - GAP_ROWS)))
    for row in range(rows):
        if gap_start <= row < gap_start + GAP_ROWS:
            continue
        if world.can_spawn():
            laser = world.spawn("boss_laser", e.x(), row * ROW_SPACING + ROW_SPACING / 2.0)
            laser.set_velocity(-WALL_SPEED, 0.0)


def beam_attack(e):
    if not hasattr(e, "beam_state"):
        switch(e, "aim")
    elapsed = e.age() - e.beam_since
    if e.beam_state == "aim":
        common.follow_player_y(e, 220.0)
        if elapsed >= AIM_SECONDS:
            e.set_velocity(0.0, 0.0)
            e.world.play_sound("warning")
            switch(e, "warn")
    elif e.beam_state == "warn":
        if e.every("blink", BLINK_SECONDS):
            toggle_warning(e)
        if elapsed >= WARN_SECONDS:
            hide_warning(e)
            switch(e, "fire")
    elif e.beam_state == "fire":
        if e.every("beam", BEAM_GAP):
            e.fire("boss_beam", common.LEFT, BEAM_SPEED)
        if elapsed >= BEAM_SECONDS:
            switch(e, "rest")
    elif elapsed >= REST_SECONDS:
        switch(e, "aim")


def switch(e, state):
    e.beam_state = state
    e.beam_since = e.age()


def toggle_warning(e):
    if getattr(e, "warning", None) is not None:
        hide_warning(e)
        return
    e.warning = e.world.spawn("warning", e.x() - 160.0, e.center_y() - 90.0)


def hide_warning(e):
    if getattr(e, "warning", None) is not None:
        e.warning.destroy()
        e.warning = None


def on_death(e):
    hide_warning(e)
