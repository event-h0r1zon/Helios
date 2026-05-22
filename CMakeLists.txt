cmake_minimum_required(VERSION 3.24)
project(Helios VERSION 0.1.0 LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable FetchContent dependency manager
include(FetchContent)
# 1. Fetch Raylib
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        6.0
)
FetchContent_MakeAvailable(raylib)

# 2. Fetch Dear ImGui (Official Repository)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.92.7
)
FetchContent_MakeAvailable(imgui)

# 3. Fetch rlImGui (raylib-extras binding)
FetchContent_Declare(
    rlImGui
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
    GIT_TAG        Raylib_6_0
)
FetchContent_MakeAvailable(rlImGui)

add_library(rlImGui STATIC
    # ImGui Core source files
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp

    # rlImGui wrapper source file
    ${rlimgui_SOURCE_DIR}/rlImGui.cpp
)

# Set include directories for both ImGui and rlImGui
target_include_directories(rlImGui PUBLIC
    ${imgui_SOURCE_DIR}
    ${rlimgui_SOURCE_DIR}
)

# Link rlImGui to raylib (needed by rlImGui.cpp)
target_link_libraries(rlImGui PUBLIC raylib)

# ==============================================================================
# 5. Define your executable
# ==============================================================================
add_executable(Helios src/main.cpp)

target_link_libraries(Helios PRIVATE rlImGui)

set_target_properties(Helios PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)