# Rendering

`Window` owns the SFML window, close/Escape events, focus and a view spanning the logical
1920 x 1080 world. Its default pixel size is 1280 x 720 and its frame limit is 60 FPS.

`TextureManager` loads textures by name and throws `RenderError` for missing resources.
`IRenderer` defines clear, rectangle, texture and display operations. `SfmlRenderer` implements
those operations without requiring game rules or network packet types.

The client loads `assets/sprites/players/r-typesheet42.gif`; its ship uses the rectangle
(66, 0, 33, 17) at scale 3. All players currently use the same sprite. `RenderSystem` draws stars
then ship sprites. Three star layers (120, 60 and 25 stars) use different speeds, sizes and colors.

Snapshots supply ship positions. New IDs create sprites, missing IDs destroy them, and unknown
kinds are skipped. The starfield remains local decoration. CMake copies assets beside the client;
the entry point first searches there, then in the current working directory.

Future work: per-player appearance, enemies/projectile assets, interpolation, animation, HUD,
fonts, resize/aspect-ratio policy and accessibility settings.
