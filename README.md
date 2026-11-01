# R-Type

A networked multiplayer horizontal shoot'em up, written in C++20 on top of our own game engine.
Up to four players join an authoritative server, shoot missiles and fight Bydos.

## Features

- Reusable, modular game engine: ECS, physics, UDP networking, SFML rendering, input
- Authoritative server with a fixed 60 Hz simulation, multithreaded network I/O
- Binary UDP protocol with sessions, heartbeats, timeouts and packet validation
- Four distinguishable players, missiles, Bydos with three movement patterns
- Collisions, lives, respawn with invulnerability, score, game over and new rounds
- Scrolling starfield

## Supported platforms

| OS | Server | Client |
|---|---|---|
| Linux (Ubuntu) | Supported | Supported |
| Windows 10/11 (MSVC) | Supported | Supported |

## Dependencies

Fetched automatically by [vcpkg](https://vcpkg.io) from `vcpkg.json`: **SFML 3**, **Asio**,
**nlohmann-json**. You only need a C++20 compiler (GCC, Clang or MSVC), CMake 3.21+, vcpkg, and on
Linux the X11/OpenGL/udev development packages. Full instructions:
[Installation](docs/getting-started/installation.md).

## Quick start

### Linux

```sh
git clone https://github.com/microsoft/vcpkg.git .vcpkg
./.vcpkg/bootstrap-vcpkg.sh -disableMetrics
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
```

### Windows (PowerShell, with `VCPKG_ROOT` set)

```powershell
cmake -S . -B out -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake"
cmake --build out --config Release --parallel 4
```

More details: [Building](docs/getting-started/building.md).

## Usage

```sh
./build/bin/r-type_server -p 4242
./build/bin/r-type_client -h 127.0.0.1 -p 4242
```

Start the server, then one client per player, each in its own terminal. On Windows the binaries are
in `out\bin\Release\`. Arrow keys move, Space fires, Escape quits.
See [Running](docs/getting-started/running.md) for LAN play and all options.

## Documentation

| Topic | Link |
|---|---|
| Documentation home | [docs/index.md](docs/index.md) |
| Architecture | [docs/architecture/overview.md](docs/architecture/overview.md) |
| Network protocol (RFC) | [docs/protocol.md](docs/protocol.md) |
| Contributing | [docs/contributing/onboarding.md](docs/contributing/onboarding.md) |
| Comparative study | [docs/comparative-study/index.md](docs/comparative-study/index.md) |
| Accessibility | [docs/accessibility.md](docs/accessibility.md) |

The documentation is also a searchable website: run `mkdocs serve` (see [docs/index.md](docs/index.md)).

## License

This project is an Epitech school project. No open-source license has been chosen yet; all rights
reserved by the authors.

## Authors

| Name | GitHub |
|---|---|
| Mohamed Erraji | [Mimo1709](https://github.com/Mimo1709) |
| Milo Bousquet | [Origanire](https://github.com/Origanire) |
| Nathan Muzay | [NathanMuzay](https://github.com/NathanMuzay) |
| Erwan Abab | [Erouanee](https://github.com/Erouanee) |
