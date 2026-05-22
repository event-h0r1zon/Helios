# Helios

An orbital mechanics simulator and visualizer built in C++ using **Raylib** for 3D graphics and **Dear ImGui** (via `rlImGui`) for the user interface.

## Prerequisites

To compile and run this project, you need a C++20 compiler, CMake, and the standard windowing/graphical development libraries.

### Linux / WSL2 (Ubuntu)
Run the following command to install the required build tools and Raylib dependencies:
```bash
sudo apt update && sudo apt install -y \
    build-essential \
    cmake \
    pkg-config \
    git \
    libx11-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev
```

## How to Build

1. **Configure the Project** (downloads and prepares Raylib and Dear ImGui locally):
   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_POLICY_VERSION_MINIMUM=3.5
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
