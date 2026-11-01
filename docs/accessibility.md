# Accessibility

This page lists, for each category of disability, what the game and its documentation provide today
and what is planned. Features are inspired by settings found in current games (remapping, colorblind
modes, assist options, visual sound cues).

## Physical and motor disabilities

| Feature | Status |
|---|---|
| Hold Space to fire continuously (no rapid tapping needed) | Available |
| Short control scheme: four directions and one fire button | Available |
| 2 s of invulnerability after respawn, to recover calmly | Available |
| Key bindings remappable in the engine (`KeyBindings::bind`) | Available in the engine; settings screen planned |
| Saved custom bindings (JSON settings file) | Planned |
| Gamepad support | Planned |
| Toggle fire (press once to start or stop) | Planned |
| Adjustable game speed in assist mode | Planned |

## Audio and visual disabilities

| Feature | Status |
|---|---|
| No information is conveyed by sound only (the game has no audio yet) | Available |
| Logical 1920 × 1080 view scaled to any window size | Available |
| Dark background with bright, contrasted sprites | Available |
| Four distinct player ships | Available (by color) |
| Player number displayed near each ship, so players are not told apart by color alone | Planned |
| Colorblind-friendly palettes and high-contrast mode | Planned |
| Visual cues for every future sound effect (hit flash, on-screen indicator) | Planned with audio |
| Option to disable flashes and screen shake | Planned with effects |
| Scalable HUD text | Planned with the HUD |

## Mental and cognitive disabilities

| Feature | Status |
|---|---|
| Simple rules: avoid Bydos, shoot them, three lives | Available |
| Automatic new round after a game over, no menu to navigate | Available |
| Consistent controls documented in [Running](getting-started/running.md) | Available |
| HUD showing lives and score permanently | Planned |
| Difficulty settings (enemy speed and spawn rate) | Planned |
| Short in-game tutorial screen | Planned |

## Documentation accessibility

- Text-based documentation in Markdown, rendered as a website with a search bar and a navigation
  outline, readable by screen readers.
- Clear heading hierarchy and tables instead of text in images.
- Diagrams written as Mermaid text, so their content stays readable in the source.
- Short pages, one topic per page, with a glossary in [Onboarding](contributing/onboarding.md).

## Implementation notes

- Accessibility options are client settings: they never change server rules, so all players still
  play the same game.
- Remapping goes through `engine::input::KeyBindings`; actions are independent from physical keys.
- Visual options belong to the client systems and the renderer, not to gameplay components.
