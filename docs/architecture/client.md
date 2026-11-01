# Client

`r-type_client` displays the game and sends the player's input. It never decides gameplay outcomes:
positions, deaths and scores come from the server.

## Structure

| Class | Responsibility | Files |
|---|---|---|
| `main` | Parses `-h` / `-p`, finds the assets folder, runs the client | `game/client/src/main.cpp` |
| `Client` | Window, textures, input, ECS world, systems, snapshot application | `Client.hpp`, `Client.cpp` |
| `NetworkClient` | Connection retries, token, sequences, periodic INPUT/HEARTBEAT, snapshot ordering, timeout | `NetworkClient.hpp`, `NetworkClient.cpp` |
| `SpriteSheet` | Describes a grid of frames in a sprite sheet | `SpriteSheet.hpp`, `SpriteSheet.cpp` |

## Main loop

```cpp
while (m_window.isOpen()) {
    m_window.processEvents();
    m_input.update(m_window.hasFocus());
    m_network.update(m_input.toBits());
    if (auto snapshot = m_network.takeSnapshot())
        applySnapshot(*snapshot);
    m_timestep.addTime(clock.restart());
    while (m_timestep.step())
        m_logic.update(m_timestep.stepSeconds());
    m_render.update(0.f);
}
m_network.disconnect();
```

The client has two `SystemManager`s: `m_logic` runs at a fixed 60 Hz, `m_render` once per frame
(the window is limited to 60 FPS). An unfocused window sends released buttons.

## Components and systems

| Component | Data |
|---|---|
| `Position` | Shared struct |
| `Sprite` | Texture name, source rectangle, scale |
| `Star` | Speed, size, color |
| `ShipTilt` | Color row, vertical direction, current tilt |

| System | Manager | Effect |
|---|---|---|
| `StarfieldSystem` | logic | Scrolls three layers of stars (120, 60, 25) at different speeds |
| `ShipAnimationSystem` | logic | Moves each ship's tilt towards its direction and picks the sprite frame |
| `RenderSystem` | render | Clears, draws stars as rectangles, then sprites, then displays |

## Applying a snapshot

The client keeps a map from network id to local entity.

- Unknown id → create the entity for its kind (`createShip`, `createMissile`, `createBydo`).
- Known id → update its position; for a ship, compare with the previous position to set its tilt
  direction.
- Id absent from the snapshot → destroy the local entity.
- Unknown kind → ignored.

The ship color is the row of the player sheet matching the slot that owns the ship, so the four
players are always distinguishable.

## Connection lifecycle

`NetworkClient` sends `CONNECT` every second, up to five attempts. After `ACCEPT` it sends INPUT about
30 times per second and HEARTBEAT every second. It throws `GameError` (the client exits with code 84)
on rejection, on `SERVER_SHUTDOWN`, or after 5 s without a valid server packet. Closing the window
sends a best-effort `DISCONNECT`.
