# Helios

An orbital mechanics simulator and visualizer built in C++ using **Raylib** for 3D graphics and **Dear ImGui** (via `rlImGui`) for the user interface.

## Prerequisites

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
