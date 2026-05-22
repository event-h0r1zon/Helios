# Helios

An orbital mechanics simulator and visualizer built in C++ using **Raylib** for 3D graphics and **Dear ImGui** (via `rlImGui`) for the user interface.

## Installation (Quick Install)

For Linux users, you can install the latest pre-compiled version of Helios directly using our installation script. This script automatically checks your system, downloads the correct binary, and places it in your path:

```bash
curl -fsSL https://raw.githubusercontent.com/event-h0r1zon/Helios/main/install.sh | bash
```

Once installed, you can launch the program from anywhere in your terminal by simply typing:

```bash
helios
```

*(Note: If you install without root permissions, make sure `~/.local/bin` is in your shell's `PATH` variable).*

## Development (Prerequisites)

To compile and run this project, you need a C++20 compiler, CMake, and the standard windowing/graphical development libraries.

## How to Build

1. **Configure the Project** (downloads and prepares Raylib and Dear ImGui locally):
   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Debug
   ```

2. **Compile the Code** (using all CPU cores):
   ```bash
   cmake --build build -j
   ```

## How to Run

After a successful compilation, the executable will be saved in the `build/bin/` folder. You can launch it using:

```bash
./build/bin/Helios
```
