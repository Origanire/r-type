# Coding conventions

These rules keep the code consistent, whoever wrote it. Follow them in every pull request.

## Language

- Code, comments, commit messages and documentation are written in **English**.
- C++20. No compiler extensions.

## Naming

| Element | Style | Example |
|---|---|---|
| Classes, structs, enums | `PascalCase` | `SparseArray`, `MessageType` |
| Functions and methods | `camelCase` | `spawnEntity()`, `addComponent()` |
| Variables and parameters | `camelCase` | `entityCount` |
| Private members | `m_` + `camelCase` | `m_components` |
| Constants (`constexpr`) | `UPPER_SNAKE_CASE` | `MAX_PLAYERS` |
| `enum class` values | `PascalCase` | `MessageType::Connect` |
| Namespaces | `lowercase` | `engine::ecs` |

## Namespaces

| Code | Namespace |
|---|---|
| `engine/<module>/` | `engine::<module>` (for example `engine::ecs`, `engine::net`) |
| `game/shared/` | `rtype` |
| `game/server/` | `rtype::server` |
| `game/client/` | `rtype::client` |

- Never write `using namespace` in a header.
- The engine knows nothing about R-Type: code in `engine/` never includes anything from `game/`.
- The server never uses the render, audio, input or UI modules.

## Files

- One main class per file, the file has the name of the class: `SparseArray.hpp`.
- A header that contains real code has a `.cpp` with the same name. Templates, plain data structs,
  interfaces and enums stay in the header only. The ECS is header-only.
- Headers go in `include/`, sources in `src/`, following the existing folders.
- Every header starts with `#pragma once`.
- Includes use the path from the `include/` folder: `#include "engine/ecs/Registry.hpp"`.
- When a folder receives its first real file, delete its `.gitkeep`.
- Every new `.cpp` is added to the root `CMakeLists.txt` in the same pull request.

## Formatting

- 4 spaces, no tabs.
- Opening brace on a new line for functions and classes, on the same line for `if`, `for`, `while`:

```cpp
void Server::run()
{
    while (m_running) {
        update();
    }
}
```

## C++ rules

- No `new` / `delete`: use RAII, `std::unique_ptr` and containers.
- `nullptr`, `enum class`, `const` references for objects passed as parameters.
- Fixed-size integers (`std::uint16_t`, `std::uint32_t`…) for anything sent on the network.
- Network data is written and read field by field, never by copying a struct (see the [protocol](protocol/rfc-rtype.md)).
- Times are computed from a delta time (`position += velocity * dt`), never from the frame rate.
- Every thread function catches its exceptions: an uncaught exception in a thread kills the program.

## Git workflow

- One issue = one branch = one pull request.
- Branches start from `dev` and are named `feature/<topic>` (or `fix/<topic>` for a bug).
- Pull requests target `dev`. `dev` is merged into `main` when it is stable; `main` is what gets evaluated.
- A pull request towards `dev` does not close its issue automatically: close it by hand after the merge.
- Commit messages: short, in English, imperative form, as in the existing history
  (`Add Registry`, `Fix packet size check`).
- A tag is created on `main` before each defense.
