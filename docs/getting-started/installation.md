# Installation

The project is self-contained: every third-party library is fetched by
[vcpkg](https://vcpkg.io) in manifest mode from `vcpkg.json`. Only a compiler, CMake, vcpkg and,
on Linux, the low-level system packages SFML needs (X11, OpenGL, udev) are installed on the system.

| Library | Used by | Purpose |
|---|---|---|
| SFML 3 | client only | Window, rendering, keyboard |
| Asio (standalone) | `engine_net` | UDP sockets |
| nlohmann-json | reserved | Planned configuration and game data files |

## Linux (Ubuntu / Debian)

```sh
sudo apt update
sudo apt install build-essential cmake ninja-build git curl zip unzip tar pkg-config \
    libx11-dev libxrandr-dev libxcursor-dev libxi-dev libudev-dev libgl1-mesa-dev \
    autoconf autoconf-archive automake libtool
```

The autotools are only used by vcpkg to build Python (embedded in the server for the gameplay
scripts). The first build compiles Python once, which takes several minutes.

Install vcpkg inside the repository (the `.vcpkg/` folder is ignored by Git):

```sh
git clone https://github.com/microsoft/vcpkg.git .vcpkg
./.vcpkg/bootstrap-vcpkg.sh -disableMetrics
```

CMake detects `.vcpkg/` automatically. An existing vcpkg installation elsewhere also works:
see [Building](building.md).

## Windows

1. Install **Visual Studio 2022** or the **Build Tools for Visual Studio 2022** with the
   "Desktop development with C++" workload.
2. Install **CMake 3.21+** and **Git**.
3. Install vcpkg, for example in `C:\vcpkg`, and set `VCPKG_ROOT`:

```powershell
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat -disableMetrics
setx VCPKG_ROOT C:\vcpkg
```

Open a new terminal after `setx` so that the variable is visible.

!!! note
    WSL is a Linux environment: building inside WSL follows the Linux instructions and does not
    count as the Windows port.
