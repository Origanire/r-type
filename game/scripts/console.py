"""Server console commands, typed by the room owner in the in-game console (F1).

Cheats only work when the room was created with CHEATS set to ON.
"""

import powerup

HELP = "help, god, heal, give <double|spread|heal|shield|force>, spawn <prefab>, skip"


def run(world, ship, text):
    words = text.split()
    name, args = words[0], words[1:]
    if name == "help":
        return HELP
    if world.rule("cheats") != "ON":
        return "cheats are OFF for this room"
    if name == "skip":
        world.set("skip", 1)
        return "skipping to the next step of the stage"
    if name == "spawn" and len(args) == 1:
        world.spawn(args[0], 1600.0, 500.0)
        return "spawned " + args[0]
    if ship is None:
        return "your ship is not in play"
    if name == "god":
        ship.set_invulnerable(100000.0)
        return "god mode on"
    if name == "heal":
        ship.heal(1000)
        return "healed"
    if name == "give" and len(args) == 1:
        powerup.apply(args[0], ship)
        return "gave " + args[0]
    return "unknown command, try help"
