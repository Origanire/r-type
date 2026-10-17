# Rendering

`Window` owns the SFML window, close/Escape events, focus and a view spanning the logical
1920 x 1080 world. Its default pixel size is 1280 x 720 and its frame limit is 60 FPS.

`TextureManager` loads textures by name and throws `RenderError` for missing resources.
`IRenderer` defines clear, rectangle, texture and display operations. `SfmlRenderer` implements
those operations without requiring game rules or network packet types.

The client loads `assets/sprites/players/r-typesheet42.gif`, described by `PLAYER_SHIP_SHEET`
(`SpriteSheet.hpp`): a grid of 33 x 17 frames drawn at scale 3. Each row is a ship color and the
player slot read from the snapshot picks the row, so the four players have different ships. Each
column is a tilt: column 2 is flat, columns 3-4 lean up and columns 0-1 lean down.
`ShipAnimationSystem` moves the tilt towards the vertical direction seen between two snapshots, at
8 frames per second, and updates the sprite rectangle. Kind-2 missiles use the basic shot of
`assets/sprites/projectiles/r-typesheet1.gif` (249, 90, 16, 4) at scale 3. `RenderSystem` draws stars
then sprites. Three star layers (120, 60 and 25 stars) use different speeds, sizes and colors.

Snapshots supply ship positions. New IDs create sprites, missing IDs destroy them, and unknown
kinds are skipped. The starfield remains local decoration. CMake copies assets beside the client;
the entry point first searches there, then in the current working directory.

Future work: enemies/projectile assets and animations, interpolation, HUD,
fonts, resize/aspect-ratio policy and accessibility settings.
