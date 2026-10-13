# R-Type

A networked multiplayer horizontal shoot'em up, written in C++20 on top of our own game engine.

> Work in progress: up to four clients share server-controlled movement. Combat is not implemented yet.

## Features

- Reusable ECS engine and SFML rendering/input
- Binary UDP protocol, four-player sessions and authoritative movement
- Full snapshots, heartbeats, disconnects and inactivity timeouts
- Local scrolling starfield

## Supported platforms

| OS | Status |
|---|---|
| Linux (Ubuntu) | Supported |
| Windows | Intended; build and crossplay validation pending |

## Dependencies

All libraries are fetched by [vcpkg](https://vcpkg.io) in manifest mode (`vcpkg.json`). Nothing needs to be installed on the system except:

- a C++20 compiler (GCC, Clang or MSVC)
- CMake 3.21 or newer
- vcpkg installed in `.vcpkg/`, or an external installation selected with `CMAKE_TOOLCHAIN_FILE`
- on Linux, the system development packages SFML needs (X11, OpenGL, udev)

| Library | Used for |
|---|---|
| SFML | Rendering, audio, input (client only) |
| Asio (standalone) | Networking |
| nlohmann-json | Configuration and game data |

## Build

### First-time setup on Linux (Ubuntu / Debian)

Install the build tools and system development packages required by SFML:

```sh
sudo apt update
sudo apt install build-essential cmake ninja-build git curl zip unzip tar pkg-config \
    libx11-dev libxrandr-dev libxcursor-dev libxi-dev libudev-dev libgl1-mesa-dev
```

From the project root, install vcpkg locally (the `.vcpkg/` directory is ignored by Git):

```sh
git clone https://github.com/microsoft/vcpkg.git .vcpkg
./.vcpkg/bootstrap-vcpkg.sh -disableMetrics
```

If vcpkg is already installed elsewhere, use its absolute path for `VCPKG_ROOT` instead.
See the [official vcpkg setup guide](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-bash).

### Configure and compile

Run from the project root. CMake automatically uses the local `.vcpkg/` installation:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release &&
cmake --build build --parallel 2
```

For vcpkg installed elsewhere, explicitly select its toolchain:

```sh
export VCPKG_ROOT="/absolute/path/to/vcpkg"
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release &&
cmake --build build --parallel 2
```

The first configuration automatically downloads and builds the libraries in `vcpkg.json`.
Binaries are written to `build/bin/`.

The old command with an empty `VCPKG_ROOT` produces the path
`/scripts/buildsystems/vcpkg.cmake`. CMake now replaces that path with the local
`.vcpkg/` toolchain automatically, including when it is already in the cache.

If you change to another vcpkg installation, clear the generated CMake cache
before configuring again:

```sh
rm -rf build/CMakeCache.txt build/CMakeFiles
```

## Usage

```sh
./build/bin/r-type_server -p 4242
# In another terminal; repeat for a second client:
./build/bin/r-type_client -h 127.0.0.1 -p 4242
```

Use arrow keys to move and Escape to quit. The client accepts numeric IPv4 addresses.
Space is reserved for shooting, which is not implemented yet. All players currently use the same ship sprite.

## Tests

```sh
ctest --test-dir build --output-on-failure
```

The codec test checks wire bytes and malformed packets. With Python 3 available, the UDP
integration test starts a real server and checks multiple clients, movement, timeouts and departures.

## Documentation

See [doc/](doc/) for the project documentation, including [RESEAU.md](doc/RESEAU.md).

## License

<!-- TODO -->

## Authors

<!-- TODO: team members -->
thread byte shiftin numero paquet
