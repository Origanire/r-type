# Libraries and tools

## Language: C++20

| Option | Strengths | Weaknesses |
|---|---|---|
| **C++20** | Native performance, deterministic memory (RAII), direct access to every game library, `std::span`, `std::bit_cast`, defaulted comparisons | Memory safety is the developer's responsibility |
| C | Simple, portable | No RAII, no containers, more error-prone code |
| Rust | Memory safety | Smaller game ecosystem for this scope, not the course language |
| C# / Java | Productivity | Garbage-collector pauses, heavier runtime |

**Choice:** C++20, required by the project and suited to a real-time engine. Modern features
(`span`, `bit_cast`, `= default` comparisons, `contains`) remove most manual buffer handling.

## Graphics, window and input: SFML 3

| Option | Strengths | Weaknesses |
|---|---|---|
| **SFML 3** | Object-oriented C++ API, 2D sprites/textures/windows/keyboard in one library, cross-platform, available in vcpkg | 2D only, smaller community than SDL |
| SDL2 / SDL3 | Very widespread, excellent platform support, controllers | C API, rendering helpers more basic |
| Raylib | Very simple, includes audio and 3D | C API, global state, less structured for an engine |
| Full engines (Unity, Godot, Unreal) | Complete tooling | Forbidden by the subject |

**Choice:** SFML 3. It covers the 2D needs with a C++ API and stays behind `IRenderer` and the input
module, so it could be replaced without touching game code.

## Networking: Asio (standalone)

| Option | Strengths | Weaknesses |
|---|---|---|
| **Asio** | Portable sockets on Linux and Windows, asynchronous model, header-only, basis of the C++ Networking TS | Callback style takes some learning |
| BSD sockets / Winsock | No dependency | Two APIs to maintain, manual error handling, must be wrapped anyway |
| SFML Network | Already a dependency of the client | Would make the server depend on SFML; less control |
| ENet | Reliable channels on UDP | Hides part of the protocol design required by the subject |

**Choice:** Asio, wrapped in `engine_net` as a private dependency. One code path for both OSes and a
background thread that never blocks the game loop.

## Package manager: vcpkg

| Option | Strengths | Weaknesses |
|---|---|---|
| **vcpkg** | Manifest mode (`vcpkg.json`), pinned baseline, CMake toolchain integration, binary caching, good Windows support | First build compiles dependencies |
| Conan | Powerful profiles, prebuilt binaries | Python tooling, more configuration |
| CMake CPM | No external tool | Rebuilds dependencies from sources per project, less suited to large libraries such as SFML |

**Choice:** vcpkg with a pinned `builtin-baseline`: identical library versions on every machine and
no system-wide installation.

## Build system: CMake

| Option | Strengths | Weaknesses |
|---|---|---|
| **CMake** | Industry standard, generates Makefiles, Ninja and Visual Studio solutions, native vcpkg support | Verbose syntax |
| Meson | Clean syntax, fast | Less integrated with vcpkg and IDEs |
| xmake | Integrated package manager | Smaller ecosystem |

**Choice:** CMake. One library target per engine module enforces the dependency rules at build time.

## Documentation: Markdown + MkDocs Material

| Option | Strengths | Weaknesses |
|---|---|---|
| **MkDocs Material** | Plain Markdown in the repository, search bar, navigation, Mermaid diagrams, static site | Needs Python to build the site |
| Sphinx | Very powerful | reStructuredText, heavier setup |
| Doxygen | Generated from code | API reference only; not a substitute for higher-level documentation |
| GitBook / wiki | Collaborative editing | Documentation separated from the code history |

**Choice:** Markdown files versioned with the code, rendered by MkDocs Material. GitHub also renders
them directly.

## JSON: nlohmann-json (reserved)

Declared in `vcpkg.json` for configuration and data-driven content (enemy definitions, game rules).
It is not used by the code yet; see [Storage](storage.md).
