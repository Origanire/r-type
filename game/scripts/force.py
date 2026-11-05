"""The Force: an indestructible satellite that blocks enemy bullets.

Attached in front of its ship, it fires with it. The action key launches it forward;
pressing again recalls it, and it snaps back in front of the ship.
"""

LAUNCH_SPEED = 900.0
RECALL_SPEED = 1100.0
SNAP_DISTANCE = 40.0
OFFSET_X = 100.0


def on_spawn(e):
    e.mode = "attached"


def on_update(e, dt):
    ship = e.parent()
    if ship is None or not ship.alive():
        e.destroy()
        return
    if ship.input_pressed("action"):
        toggle(e, ship)
    if e.mode == "launched":
        e.set_velocity(e.vx() * 0.96, 0.0)
    elif e.mode == "recall":
        recall(e, ship)
    if ship.input_held("fire") > 0.0 and e.every("shot", 0.5):
        e.fire("player_missile", 0.0, 1500.0)


def toggle(e, ship):
    if e.mode == "attached":
        e.mode = "launched"
        e.detach()
        e.set_velocity(LAUNCH_SPEED, 0.0)
        ship.set_player_value("force", 2)
    else:
        e.mode = "recall"


def recall(e, ship):
    target_x = ship.x() + OFFSET_X
    target_y = ship.y()
    dx = target_x - e.x()
    dy = target_y - e.y()
    distance = (dx * dx + dy * dy) ** 0.5
    if distance < SNAP_DISTANCE:
        e.mode = "attached"
        e.attach_to(ship, OFFSET_X, 0.0)
        ship.set_player_value("force", 1)
        return
    e.set_velocity(dx / distance * RECALL_SPEED, dy / distance * RECALL_SPEED)
