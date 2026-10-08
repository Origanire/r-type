# R-Type

A networked multiplayer horizontal shoot'em up, written in C++20 on top of our own game engine.

> Work in progress: nothing is implemented yet.

## Features

<!-- TODO -->

## Supported platforms

| OS | Status |
|---|---|
| Linux (Ubuntu) | Supported |
| Windows | Supported (crossplay with Linux) |

## Dependencies

All libraries are fetched by [vcpkg](https://vcpkg.io) in manifest mode (`vcpkg.json`). Nothing needs to be installed on the system except:

- a C++20 compiler (GCC, Clang or MSVC)
- CMake 3.21 or newer
- vcpkg, with the `VCPKG_ROOT` environment variable pointing to it
- on Linux, the system development packages SFML needs (X11, OpenGL, udev)

| Library | Used for |
|---|---|
| SFML | Rendering, audio, input (client only) |
| Asio (standalone) | Networking |
| nlohmann-json | Configuration and game data |

## Build

```sh
cmake -B build -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Binaries are written to `build/bin/`.

## Usage

```sh
./r-type_server -p <port>
./r-type_client -h <server ip> -p <port>
```

## Documentation

<!-- TODO: link to the online documentation -->

## License

<!-- TODO -->

## Authors

<!-- TODO: team members -->
