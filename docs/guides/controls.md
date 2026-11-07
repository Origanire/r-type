# Controls and gamepad

## Keyboard

Keys are set in `data/<game>_settings.json`:

```json
"controls": {"up": "Up", "down": "Down", "left": "Left", "right": "Right", "fire": "Space", "action": "LShift"},
"toggleFire": false
```

Accepted names: `A` to `Z`, `0` to `9`, `Up`, `Down`, `Left`, `Right`, `Space`, `Enter`, `Tab`, `Backspace`,
`LShift`, `RShift`, `LControl`, `RControl`, `LAlt`, `RAlt`. Keys are read by position (scancode), so `W` is the
same physical key on QWERTY and AZERTY keyboards.

With `"toggleFire": true`, fire is switched on and off by a press instead of being held.

Fixed keys:

| Key | Effect |
|---|---|
| Escape | The scene's `escapeAction` (pause, back, quit) |
| Enter | Confirm the focused button |
| Arrows | Move the focus in menus |
| F1 | Console |
| F3 | Debug overlay |

## Gamepad

Any gamepad recognised by the system works (XInput and DirectInput on Windows, joystick devices on Linux).
Gamepad number 0 (the first one connected) is used.

```json
"gamepad": {"fire": 0, "action": 1, "deadzone": 35}
```

| Input | In game | In menus |
|---|---|---|
| Left stick, D-pad | Move | Move the focus |
| Button `fire` (0, A on Xbox pads) | Fire | Confirm |
| Button `action` (1, B on Xbox pads) | Action | - |
| Select / Back (button 6) | Pause | Back |

`deadzone` (0 to 100) is the part of the stick travel ignored, to avoid drifting with worn sticks.
Button numbers depend on the pad: if a button does not respond, try other numbers in the settings file.

## What the server receives

Whatever the device, the client sends the same 6 buttons (up, down, left, right, fire, action) in `INPUT`
messages ([Protocol](../protocol.md)). What `action` does is decided by the game's scripts.
