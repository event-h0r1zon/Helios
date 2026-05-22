#include "raylib.h"
#include "rlImGui.h"
#include "physics/KeplerianSolver.hpp"
#include "rendering/Visualizer.hpp"
#include "ui/GUIController.hpp"

int main() {

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);

    // Initialization of secreen and GUI.
    InitWindow(1280, 720, "Helios");

    MaximizeWindow();

    SetTargetFPS(60);
    rlImGuiSetup(true);

    KeplerianElements orbit = {
        10,
        0.5,
        0.0,
        0.0,
        0.0
    };
    
    Visualizer visualizer;
    GUIController gui;

    while(!WindowShouldClose()) {
        visualizer.updateOrbitPoints(orbit);

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
