#include "raylib.h"
#include "rlImGui.h"
#include "imgui.h"

int main() {
    // 1. Initialize Raylib Window
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "Helios - Orbital Mechanics Simulator");
    SetTargetFPS(60);

    // 2. Initialize Dear ImGui
    rlImGuiSetup(true); // true enables dark mode by default
    ImGui::GetIO().IniFilename = "build/imgui.ini";
    // 3. Define 3D Camera
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 10.0f, 10.0f }; // Camera looking down at an angle
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };     // Looking at origin
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Y-up
    camera.fovy = 45.0f;                                // Field of view
    camera.projection = CAMERA_PERSPECTIVE;

    // Simulation parameters (adjustable in ImGui)
    float timeStep = 1.0f;
    float planetMass = 100.0f;
    bool isPaused = false;

    // Main Game Loop
    while (!WindowShouldClose()) {
        // Update camera position using mouse controls
        UpdateCamera(&camera, CAMERA_ORBITAL);

        // --- DRAWING ---
        BeginDrawing();
        ClearBackground(DARKGRAY);

        // Draw 3D Scene
        BeginMode3D(camera);
            // Draw a basic space grid on the XZ plane
            DrawGrid(20, 1.0f);
            
            // Draw the central gravitating body (e.g. Earth / Sun)
            DrawSphere((Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, BLUE);
            DrawSphereWires((Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, 16, 16, LIME);
        EndMode3D();

        // Draw HUD / UI
        rlImGuiBegin();
            // Create a panel for controls
            ImGui::Begin("Simulator Controls");
                ImGui::Text("Helios Orbital Mechanics");
                ImGui::Separator();
                
                ImGui::Checkbox("Pause Simulation", &isPaused);
                ImGui::SliderFloat("Time Step", &timeStep, 0.1f, 10.0f, "%.1f x");
                ImGui::SliderFloat("Central Mass", &planetMass, 10.0f, 1000.0f, "%.0f kg");

                ImGui::Separator();
                ImGui::Text("FPS: %d", GetFPS());
                ImGui::Text("Camera Pos: (%.1f, %.1f, %.1f)", camera.position.x, camera.position.y, camera.position.z);
            ImGui::End();
        rlImGuiEnd();

        EndDrawing();
    }

    // Cleanup
    rlImGuiShutdown();
    CloseWindow();

    return 0;
}
