# Building

The build is driven by CMake. The first configuration downloads and compiles the libraries listed
in `vcpkg.json`, which takes several minutes; later builds reuse them.

## Linux

From the repository root:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
```

With vcpkg installed outside the repository:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build --parallel 2
```

Binaries: `build/bin/r-type_server` and `build/bin/r-type_client`.

## Windows (PowerShell)

The `out/` folder is ignored by Git and is used for the Windows build:

```powershell
cmake -S . -B out -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake"
cmake --build out --config Release --parallel 4
```

Binaries: `out\bin\Release\r-type_server.exe` and `out\bin\Release\r-type_client.exe`.

## What gets built

| Target | Kind | Links |
|---|---|---|
| `engine_core` | library | — |
| `engine_ecs` | library | `engine_core` |
| `engine_physics` | library | — |
| `engine_net` | library | `engine_core`, Asio (private) |
| `engine_render` | library | `engine_core`, SFML Graphics |
| `engine_input` | library | `engine_core`, SFML Window |
| `rtype_shared` | library | `engine_core`, `engine_ecs`, `engine_net` |
| `r-type_server` | executable | `rtype_shared`, `engine_physics` |
| `r-type_client` | executable | `rtype_shared`, `engine_render`, `engine_input` |

The server never links a rendering, input or audio module. After the client is built, CMake copies
`assets/` next to the client executable.

## Troubleshooting

| Symptom | Fix |
|---|---|
| `CMAKE_TOOLCHAIN_FILE does not exist` | Install vcpkg or fix `VCPKG_ROOT`, then delete `build/CMakeCache.txt` |
| Changing vcpkg installation has no effect | Delete `build/CMakeCache.txt` and `build/CMakeFiles` |
| SFML fails to configure on Linux | Install the system packages listed in [Installation](installation.md) |

## Build options (engine modularity)

| Option | Default | Effect |
|---|---|---|
| `ENGINE_BUILD_CLIENT` | `ON` | builds render, audio, input, client modules and `r-type_client` (needs SFML) |
| `ENGINE_BUILD_SERVER` | `ON` | builds scripting, simulation modules and `r-type_server` (needs Python) |

Example, a server-only build for a dedicated machine:

```sh
cmake -S . -B build-server -DENGINE_BUILD_CLIENT=OFF
cmake --build build-server
```
