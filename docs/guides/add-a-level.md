# Add a level or a boss

Levels are Python modules run by `game/scripts/campaign.py`, which is attached to the invisible
`director` entity of the server scene.

## A new stage

Create `game/scripts/stage4.py`:

```python
DURATION = 80.0                                   # seconds before the boss arrives
BOSS = ("boss_core", 2000.0, 392.0)               # prefab, x, y

RANDOM = (0.8, 1.5, [                             # min / max delay between random spawns
    ("pod", 3, 0.0, 1000.0),                      # prefab, weight, min y, max y
    ("hopper", 2, 200.0, 800.0),
])

WAVES = [                                         # time, prefab, count, x, y, spacing x, spacing y
    (5.0, "heavy", 2, 1950.0, 300.0, 0.0, 400.0),
    (20.0, "splitter", 3, 1950.0, 200.0, 220.0, 280.0),
]


def events(world, time, director):                # optional: scripted moments
    if time >= 40.0 and not getattr(director, "ambush_done", False):
        director.ambush_done = True
        for index in range(4):
            pod = world.spawn("pod", -150.0, 150.0 + index * 220.0)
            pod.set_velocity(420.0, 0.0)
```

Then add it to the list in `campaign.py`:

```python
import stage4
STAGES = (stage1, stage2, stage3, stage4)
```

The campaign handles the rest: the "STAGE N" intro, the progress bar, the boss health bar, the
transition to the next stage and the victory after the last one.

## A new boss

A boss is an enemy whose script changes behaviour with its health. `boss_common.py` gives:

- `enter(e)`: flies in from the right and stops at the `stop_x` param, returns `True` once arrived;
- `phase(e, (0.6, 0.25))`: 1 above 60 % health, 2 above 25 %, 3 below;
- `burst(e, name, size, gap, every_min, every_max, shoot)`: bursts of shots.

```python
import boss_common
import common


def on_update(e, dt):
    if not boss_common.enter(e):
        return
    if boss_common.phase(e, (0.5,)) == 1:
        common.bounce(e, 150.0)
        if e.every("fan", 2.0):
            e.fire_spread("boss_orb", 7, 80.0, common.LEFT, 400.0)
    elif e.every("ring", 1.2):
        common.ring(e, "enemy_bullet", 16, 380.0)
```

Declare its prefab like any enemy (with `"destroyOnHit": false` in `ContactDamage` and no
`DespawnOffscreen`) and reference it in the `BOSS` line of a stage.
