"""Player ship: friendly fire rule, charged shot (hold fire, release after CHARGE_SECONDS) and HUD values."""

CHARGE_SECONDS = 1.2
CHARGED_SPEED = 1300.0


def missile(world):
    return "player_missile_ff" if world.rule("friendlyFire") == "ON" else "player_missile"


def on_spawn(e):
    e.set_weapon(missile(e.world), 0.2)
    e.set_player_value("weapon", 0)
    e.set_player_value("force", 0)


def on_update(e, dt):
    held = e.input_held("fire")
    e.set_player_value("charge", int(100 * min(1.0, held / CHARGE_SECONDS)))
    if e.input_released("fire") >= CHARGE_SECONDS:
        e.fire("player_missile_charged", 0.0, CHARGED_SPEED)
