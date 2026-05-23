#include "raylib.h"
#include "rlImGui.h"
#include "imgui.h"
#include "physics/KeplerianSolver.hpp"
#include "rendering/Visualizer.hpp"
#include "rendering/CameraController.hpp"
#include "ui/GUIController.hpp"
#include <iostream>
#include <string_view>

constexpr std::string_view HELIOS_VERSION = "0.0.5";

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

    Camera3D camera;
    camera.position = Vector3{ 35.0f, 0.0f, 20.0f };
    camera.target = Vector3{ 0.0f, 0.0f, 0.0f };
    camera.up = Vector3{ 0.0f, 0.0f, 1.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Visualizer visualizer;
    GUIController gui;
    CameraController cameraController;

    while(!WindowShouldClose()) {
        cameraController.update(camera);

        visualizer.update(
            orbit, 
            gui.getTimeScale(),
            gui.getGroundTrackPoints()
        );

        BeginDrawing();
        ClearBackground(DARKGRAY);

        visualizer.render(camera, orbit);

        rlImGuiBegin();
            gui.draw(orbit);
        rlImGuiEnd();

        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();

    return 0;
}
