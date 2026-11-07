# R-Type

A networked multiplayer shoot'em up written in C++20 on top of our own data-driven game engine.
Up to four players join an authoritative server, fight Bydos through three stages and their bosses.

The engine is generic: games are described with JSON files and Python scripts, without any C++.
The repository ships two games built on it:

| Game | Folder | Mode | Executables |
|---|---|---|---|
| **R-Type** | `game/` | Online, client/server over UDP | `r-type_server`, `r-type_client` |
| **Skyhop** | `game2/` | Solo, no server needed | `skyhop` |

## Features

- Engine split into independent modules: ECS, physics, network, rendering, audio, input, scripting
- Games defined by data: entities, components, systems, scenes and HUD in JSON, behaviours in Python
- Authoritative server at 60 ticks per second, binary UDP protocol with sessions, rooms and validation
- Lobby with public or code-protected rooms, room rules (difficulty, friendly fire, cheats), kicks
- Three stages, waves, bosses, power-ups, the Force, charged shots, terrain
- Solo mode: the same engine runs a game in memory, without network
- In-game developer console (F1), debug overlay (F3), pause menu, gamepad support
- Accessibility settings: remappable controls, text scale, volumes, hold-to-fire toggle

## Supported platforms

| OS | Compiler | Status |
|---|---|---|
| Linux (Ubuntu, also under WSL 2) | GCC or Clang with C++20 support | Supported |
| Windows 10/11 | MSVC 2022 | Supported |

## Dependencies

Installed automatically by [vcpkg](https://vcpkg.io) from `vcpkg.json`:
**SFML 3** (window, graphics, audio), **Asio** (UDP), **nlohmann-json** (data files),
**pybind11** and **Python 3** (scripting).

You need a C++20 compiler, CMake 3.21+ and vcpkg. On Linux, also the X11, OpenGL and udev development
packages. Step by step: [Installation](docs/getting-started/installation.md).

## Quick start

### Linux

```sh
git clone https://github.com/microsoft/vcpkg.git .vcpkg
./.vcpkg/bootstrap-vcpkg.sh -disableMetrics
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel "$(nproc)"
```

### Windows (PowerShell, `VCPKG_ROOT` set)

```powershell
cmake -S . -B out\windows -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake"
cmake --build out\windows --config Release --parallel 4
```

The first configuration downloads and builds the dependencies, it can take several minutes.
Details and options: [Building](docs/getting-started/building.md).

## Usage

Binaries are in `build/bin/` on Linux and `out\windows\bin\Release\` on Windows.

```sh
# R-Type: one server, then one client per player
./r-type_server -p 4242
./r-type_client -h 127.0.0.1 -p 4242

# Skyhop: solo, nothing else to start
./skyhop
```

On a local network, start the server on one machine: it prints the address to use, for example
`-h 192.168.1.67 -p 4242`. Then start the clients on the other machines with that address.

| Action | Keyboard | Gamepad |
|---|---|---|
| Move | Arrow keys | Left stick or D-pad |
| Fire (hold to charge) | Space | A |
| Action (R-Type: launch or recall the Force) | Left Shift | B |
| Pause | Escape | Select |
| Console / debug overlay | F1 / F3 | - |

Controls and accessibility options are set in `data/rtype_settings.json`. All options:
[Running](docs/getting-started/running.md).

## Documentation

The full documentation is a searchable website generated with MkDocs:

```sh
pip install mkdocs-material
mkdocs serve          # then open http://127.0.0.1:8000
```

| I want to... | Read |
|---|---|
| Understand the project | [Architecture overview](docs/architecture/overview.md) |
| Join the team | [Onboarding](docs/contributing/onboarding.md) |
| Create a new game | [Create a game from scratch](docs/guides/create-a-game.md) |
| Add an enemy or a level | [Guides](docs/guides/add-an-enemy.md) |
| Write a client for our server | [Network protocol (RFC)](docs/protocol.md) |
| Know why we chose our tools | [Comparative study](docs/comparative-study/index.md) |
| Check accessibility | [Accessibility](docs/accessibility.md) |
| Fix a problem | [Troubleshooting](docs/troubleshooting.md) |

## License

School project made at EPITECH. All rights reserved by EPITECH and the authors.
Third-party assets keep their own license: the Skyhop graphics and sounds are from
[Kenney](https://kenney.nl) under CC0 (see `game2/assets/skyhop/`).

## Authors

| Name | GitHub |
|---|---|
| Mohamed Erraji | [Mimo1709](https://github.com/Mimo1709) |
| Milo Bousquet | [Origanire](https://github.com/Origanire) |
| Nathan Muzay | [NathanMuzay](https://github.com/NathanMuzay) |
| Erwan Ababsa | [Erouanee](https://github.com/Erouanee) |
