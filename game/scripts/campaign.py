"""Campaign: runs the stages one after the other.

Attached to an invisible "director" entity. Each stage module gives:
    DURATION  seconds before the boss arrives
    BOSS      (prefab, x, y)
    RANDOM    (min interval, max interval, [(prefab, weight, min y, max y), ...])
    WAVES     [(time, prefab, count, x, y, spacing x, spacing y), ...]
    events(world, time, director)  optional scripted moments

It publishes the world values shown by the client HUD: stage, phase, progress, bossHealth.
With the director param "bossRush" set to 1, every stage skips its waves and starts with its boss.
The "difficulty" room rule scales spawn rates and boss health; boss deaths shake the screen.
It also runs the secret bomb (see bomb.py).
"""

import bomb
import common
import stage1
import stage2
import stage3

STAGES = (stage1, stage2, stage3)
INTRO_SECONDS = 3.0
CLEAR_SECONDS = 2.5
SPAWN_X = 1950.0
SHAKE_ON_BOSS_DEATH = 14
SPAWN_RATE = {"EASY": 1.4, "NORMAL": 1.0, "HARD": 0.7}
BOSS_HEALTH = {"EASY": 0.7, "NORMAL": 1.0, "HARD": 1.5}


def on_spawn(d):
    d.playing = False


def on_update(d, dt):
    world = d.world
    match = world.match_phase()
    if match != "playing":
        if match == "over" and world.label("phase") != "victory":
            world.set_label("phase", "defeat")
        d.playing = False
        return
    if not d.playing:
        d.playing = True
        bomb.reset(d.world)
        start_stage(d, 0)
    d.timer += dt
    decay_shake(d.world)
    if d.world.get("skip") > 0:
        d.world.set("skip", 0)
        skip(d)
    bomb.update(d, d.boss if d.state == "boss" else None)
    STATES[d.state](d, dt)


def start_stage(d, index):
    d.stage_index = index
    d.stage = STAGES[index]
    d.next_wave = 0
    d.waves = sorted(d.stage.WAVES)
    d.next_random = 0.0
    d.boss = None
    enter(d, "intro")
    world = d.world
    world.set("stage", index + 1)
    world.set("progress", 0)
    world.set("bossHealth", 0)


def enter(d, state):
    d.state = state
    d.timer = 0.0
    d.world.set_label("phase", state)


def intro(d, dt):
    if d.timer >= INTRO_SECONDS:
        enter(d, "running")


def running(d, dt):
    stage = d.stage
    world = d.world
    if d.number("bossRush", 0.0) > 0.0:
        start_boss(d)
        return
    world.set("progress", int(100 * min(1.0, d.timer / stage.DURATION)))
    while d.next_wave < len(d.waves) and d.waves[d.next_wave][0] <= d.timer:
        spawn_wave(world, d.waves[d.next_wave])
        d.next_wave += 1
    if d.timer >= d.next_random:
        minimum, maximum, table = stage.RANDOM
        rate = SPAWN_RATE.get(world.rule("difficulty"), 1.0)
        d.next_random = d.timer + world.random(minimum, maximum) * rate
        spawn_random(world, table)
    if hasattr(stage, "events"):
        stage.events(world, d.timer, d)
    if d.timer >= stage.DURATION:
        start_boss(d)


def start_boss(d):
    prefab, x, y = d.stage.BOSS
    d.world.set("progress", 100)
    d.boss = d.world.spawn(prefab, x, y)
    scale = BOSS_HEALTH.get(d.world.rule("difficulty"), 1.0)
    health = max(1, int(d.boss.max_health() * scale))
    d.boss.set_health(health, health)
    enter(d, "boss")


def boss(d, dt):
    world = d.world
    world.set("bossHealth", int(100 * common.health_ratio(d.boss)) if d.boss.exists() else 0)
    if d.boss.alive():
        return
    world.set("shake", SHAKE_ON_BOSS_DEATH)
    world.play_sound("bossDeath")
    if d.stage_index + 1 >= len(STAGES):
        enter(d, "victory")
        world.end_match()
    else:
        enter(d, "clear")


def clear(d, dt):
    if d.timer >= CLEAR_SECONDS:
        start_stage(d, d.stage_index + 1)


def victory(d, dt):
    pass


def skip(d):
    if d.state == "running":
        start_boss(d)
    elif d.state == "boss" and d.boss is not None:
        d.boss.kill()


def decay_shake(world):
    if world.get("shake") > 0 and world.every("shake", 0.05):
        world.set("shake", world.get("shake") - 1)


def spawn_wave(world, wave):
    _, prefab, count, x, y, spacing_x, spacing_y = wave
    for index in range(count):
        if world.can_spawn():
            world.spawn(prefab, x + index * spacing_x, y + index * spacing_y)


def spawn_random(world, table):
    if not world.can_spawn() or not table:
        return
    roll = world.random(0.0, sum(entry[1] for entry in table))
    for prefab, weight, low, high in table:
        roll -= weight
        if roll <= 0.0:
            world.spawn(prefab, SPAWN_X, world.random(low, high))
            return


STATES = {"intro": intro, "running": running, "boss": boss, "clear": clear, "victory": victory}
