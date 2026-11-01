# Rendering

## Engine side

| Class | Role |
|---|---|
| `Window` | Owns the SFML window (1280 × 720, 60 FPS limit), handles close and Escape, reports focus. Its view always shows the logical 1920 × 1080 world, whatever the window size. |
| `TextureManager` | Loads textures by name; throws `RenderError` if a file is missing or unknown. |
| `IRenderer` | Interface: `clear`, `drawRect`, `drawTexture`, `display`. |
| `SfmlRenderer` | `IRenderer` implementation using SFML. |

Game code draws through `IRenderer` only. SFML types never appear in game components.

## Game side

`RenderSystem` clears the screen, draws every `Position + Star` entity as a small rectangle, then
every `Position + Sprite` entity as a textured sprite.

| Entity | Sprite sheet | Frame |
|---|---|---|
| Player ship | `sprites/players/r-typesheet42.gif` | 33 × 17 grid: one row per player color, one column per tilt (2 = flat, 3–4 up, 0–1 down), scale 3 |
| Player missile | `sprites/projectiles/r-typesheet1.gif` | 16 × 4 at (249, 90), scaled to 48 × 12 |
| Bydo | `sprites/enemies/r-typesheet5.gif` | 33 × 36, scaled to 66 × 72 |

Sheets are described by `SpriteSheet` constants in `SpriteSheet.hpp`; `frame(column, row)` returns
the source rectangle.

## Assets

CMake copies `assets/` next to the client executable after each build. At startup the client looks
for `assets/` beside the executable, then in the current directory.
