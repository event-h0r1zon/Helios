#include "raylib.h"
#include "rlImGui.h"
#include "imgui.h"
#include "physics/KeplerianSolver.hpp"
#include "rendering/Visualizer.hpp"
#include "ui/GUIController.hpp"
#include <iostream>
#include <string_view>

constexpr std::string_view HELIOS_VERSION = "0.0.2";

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string_view arg = argv[1];
        if (arg == "--version" || arg == "-v") {
            std::cout << HELIOS_VERSION << std::endl;
            return 0;
        }
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);

    // Initialization of secreen and GUI.
    InitWindow(1280, 720, "Helios");

    MaximizeWindow();

    rlImGuiSetup(true);
    ImGui::GetIO().IniFilename = nullptr; // Disable imgui.ini generation

    KeplerianElements orbit = {
        7000.0, // Semi-major axis in km (approx 620 km altitude)
        0.05,   // Eccentricity
        28.5,   // Inclination in degrees
        0.0,    // Argument of periapsis in degrees
        0.0,    // RAAN in degrees
        0.0     // Mean anomaly at epoch
    };

    Visualizer visualizer;
    GUIController gui;

    while(!WindowShouldClose()) {
        visualizer.update(orbit);

        BeginDrawing();
        ClearBackground(DARKGRAY);

        visualizer.render(orbit);

        rlImGuiBegin();
            gui.draw(orbit);
        rlImGuiEnd();

        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();

    return 0;
}
