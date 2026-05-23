#include "physics/KeplerianSolver.hpp"
#include "Visualizer.hpp"
#include "rlgl.h"
#include "raymath.h"
#include "imgui.h"
#include "physics/Constants.hpp"
#include <cmath>
#include <string>
#include <cstdlib>

static std::string ResolveAssetPath(const std::string& filename) {
    // 1. Check relative to current working directory (for local dev in project root)
    std::string pathCwd = "assets/" + filename;
    if (FileExists(pathCwd.c_str())) {
        return pathCwd;
    }

    // 2. Check relative to executable directory (for local runs from build output)
    std::string exeDir = GetApplicationDirectory();
    std::string pathExe = exeDir + "assets/" + filename;
    if (FileExists(pathExe.c_str())) {
        return pathExe;
    }

    // 3. Check user shared data directory (FHS standard: ~/.local/share/helios/assets/)
    const char* home = std::getenv("HOME");
    if (home != nullptr) {
        std::string pathUserShare = std::string(home) + "/.local/share/helios/assets/" + filename;
        if (FileExists(pathUserShare.c_str())) {
            return pathUserShare;
        }
    }

    // 4. Check global system shared data directory (FHS standard: /usr/local/share/helios/assets/)
    std::string pathSystemShare = "/usr/local/share/helios/assets/" + filename;
    if (FileExists(pathSystemShare.c_str())) {
        return pathSystemShare;
    }

    // Fallback to default relative path
    return "assets/" + filename;
}

Visualizer::Visualizer() {
     // Generate a high-resolution sphere mesh
    Mesh earthMesh = GenMeshSphere(1.0f, 64, 64);
    // Squish the poles along the Z-axis (WGS-84 flattening)
    float flatteningRatio = 1.0f - static_cast<float>(
        Helios::Physics::EARTH_FLATTENING
    ); // b/a
    for (int i = 0; i < earthMesh.vertexCount; i++)
        earthMesh.vertices[i * 3 + 2] *= flatteningRatio;

    // Update the GPU vertex buffer with our squished CPU vertices
    UpdateMeshBuffer(earthMesh, 0, earthMesh.vertices, earthMesh.vertexCount * 3 * sizeof(float), 0);

    // Upload the modified mesh to the GPU and load it into a Model
    earthModel = LoadModelFromMesh(earthMesh);
    earthModel.transform = MatrixRotateX(90.0f * DEG2RAD);
    std::string texturePath = ResolveAssetPath("earth.png");
    earthTexture = LoadTexture(texturePath.c_str());
    earthModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = earthTexture;
}

Vector3 toRaylibVector(const Helios::Math::Vector3D& vec) {
    return Vector3 { 
        float(vec.x / Helios::Physics::EARTH_RADIUS_KM), 
        float(vec.y / Helios::Physics::EARTH_RADIUS_KM), 
        float(vec.z / Helios::Physics::EARTH_RADIUS_KM) 
    };
}

void drawDashedLine3D(Vector3 start, Vector3 end, float dashLength, Color color) {
    Vector3 dir = Vector3Subtract(end, start);
    float totalLength = Vector3Length(dir);
    if (totalLength <= 0.0f) return;
    
    Vector3 dirNorm = Vector3Scale(dir, 1.0f / totalLength);
    float currentDist = 0.0f;
    bool drawSegment = true;
    
    while (currentDist < totalLength) {
        float nextDist = currentDist + dashLength;
        if (nextDist > totalLength) nextDist = totalLength;
        
        if (drawSegment) {
            Vector3 p1 = Vector3Add(start, Vector3Scale(dirNorm, currentDist));
            Vector3 p2 = Vector3Add(start, Vector3Scale(dirNorm, nextDist));
            DrawLine3D(p1, p2, color);
        }
        
        currentDist = nextDist;
        drawSegment = !drawSegment;
    }
}

double deg2rad(double degrees) {
    return degrees * M_PI / 180.0;
}

void Visualizer::updateOrbitPoints(const KeplerianElements& elements) {
    orbitPoints.clear();

    int resolution = 360; // Number of points to calculate along the orbit.

    // Transform to 3D coordinates using orbital elements.
    double RAAN = deg2rad(elements.RAAN);
    double inc = deg2rad(elements.i);
    double omega = deg2rad(elements.omega);
    
    Helios::Math::Matrix3x3 rotationRAAN = {
        std::cos(RAAN), -std::sin(RAAN), 0.0,
        std::sin(RAAN), std::cos(RAAN), 0.0,
        0.0, 0.0, 1.0
    };

    Helios::Math::Matrix3x3 rotationInclination = {
        1.0, 0.0, 0.0,
        0.0, std::cos(inc), -std::sin(inc),
        0.0, std::sin(inc), std::cos(inc)
    };

    Helios::Math::Matrix3x3 rotationArgPeriapsis = {
        std::cos(omega), -std::sin(omega), 0.0,
        std::sin(omega), std::cos(omega), 0.0,
        0.0, 0.0, 1.0
    };

    rotation = rotationRAAN * rotationInclination * rotationArgPeriapsis;
    
    double p = elements.a * (1.0 - std::pow(elements.e, 2));

    double r_ascending = p / (1.0 + elements.e * std::cos(omega));
    double r_descending = p / (1.0 - elements.e * std::cos(omega));

    bounds.ascending = Helios::Math::Vector3D(
        r_ascending * std::cos(RAAN),
        r_ascending * std::sin(RAAN),
        0.0
    );

    bounds.descending = Helios::Math::Vector3D(
        -r_descending * std::cos(RAAN),
        -r_descending * std::sin(RAAN),
        0.0
    );

    for (int i = 0; i < resolution; ++i) {
        double theta = (2.0 * M_PI * i) / resolution; // True anomaly
        double p = elements.a * (1 - std::pow(elements.e, 2));
        double r = p / (1 + elements.e * std::cos(theta));

        // Local orbital plane coordinates.
        
        double x_local = r * std::cos(theta);
        double y_local = r * std::sin(theta);
        double z_local = 0;

        Helios::Math::Vector3D localPos = Helios::Math::Vector3D(x_local, y_local, z_local);

        Helios::Math::Vector3D worldPos = rotation * localPos;
        orbitPoints.push_back(worldPos);
    }

}

void Visualizer::update(
    const KeplerianElements& elements, 
    double timeScale,
    size_t groundTrackPoints,
    const Camera3D& camera
) {
    // Handle Esc key to return to 3D mode
    if (show2DMap && IsKeyPressed(KEY_ESCAPE)) {
        show2DMap = false;
    }

    // Hover detection on the Earth sphere (radius 1.0f at origin)
    isEarthHovered = false;
    if (!show2DMap && !ImGui::GetIO().WantCaptureMouse) {
        Ray ray = GetMouseRay(GetMousePosition(), camera);
        RayCollision collision = GetRayCollisionSphere(ray, Vector3{ 0.0f, 0.0f, 0.0f }, 1.0f);
        if (collision.hit) {
            isEarthHovered = true;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                show2DMap = true;
            }
        }
    }

    // Clear ground track when orbital elements change
    if (!hasPreviousElements && !hasPreviousTimeScale) {
        previousElements = elements;
        previousTimeScale = timeScale;
        hasPreviousElements = true;
        hasPreviousTimeScale = true;
    } else if (elements.a != previousElements.a ||
               elements.e != previousElements.e ||
               elements.i != previousElements.i ||
               elements.omega != previousElements.omega ||
               elements.RAAN != previousElements.RAAN ||
               elements.M0 != previousElements.M0 ||
               timeScale != previousTimeScale) {
        groundTrack.clear();
        previousElements = elements;
        previousTimeScale = timeScale;
    }
    
    // Handle earth rotation.
    float dt = GetFrameTime();
    earthRotationAngle += dt * static_cast<float>(timeScale) * static_cast<float>(
        Helios::Physics::EARTH_ROTATION_SPEED
    );
    if (earthRotationAngle > 2.0f * PI) earthRotationAngle -= 2.0f * PI;
    
    earthModel.transform = MatrixRotateZ(earthRotationAngle);

    // Update the orbit points based on the current orbital elements
    updateOrbitPoints(elements);

    // Update the spacecraft's position along the orbit.
    double theta = KeplerianSolver::solve(elements, GetTime() * timeScale);
    double p = elements.a * (1.0 - std::pow(elements.e, 2));
    double r = p / (1.0 + elements.e * std::cos(theta));

    Helios::Math::Vector3D localPos = Helios::Math::Vector3D(r * std::cos(theta), r * std::sin(theta), 0.0);
    Helios::Math::Vector3D worldPos = rotation * localPos;
    spacecraftPosition = worldPos;

    // Convert ECI worldPos to ECEF for rotation-corrected ground track
    double cosAngle = std::cos(earthRotationAngle);
    double sinAngle = std::sin(earthRotationAngle);
    Helios::Math::Vector3D ecefPos = {
        worldPos.x * cosAngle + worldPos.y * sinAngle,
        -worldPos.x * sinAngle + worldPos.y * cosAngle,
        worldPos.z
    };

    // Update the ground track.
    groundTrack.push_back(ecefPos);
    if (groundTrack.size() > groundTrackPoints) 
        groundTrack.erase(
            groundTrack.begin(), 
            groundTrack.begin() + (groundTrack.size() - groundTrackPoints)
        );
    
}

void Visualizer::render(const Camera3D& camera, const KeplerianElements& elements) {
    if (show2DMap) {
        // Render flat 2D Map view
        ClearBackground(BLACK);

        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();

        // Maintain 1:2 aspect ratio for the rotated flat map (width is half of height)
        float mapWidth = (float)screenWidth;
        float mapHeight = mapWidth / 2.0f;
        if (mapHeight > (float)screenHeight) {
            mapHeight = (float)screenHeight;
            mapWidth = mapHeight * 2.0f;
        }

        float mapX = ((float)screenWidth - mapWidth) / 2.0f;
        float mapY = ((float)screenHeight - mapHeight) / 2.0f;

        Rectangle destRec = { mapX + mapWidth, mapY, mapHeight, mapWidth };
        DrawTexturePro(
            earthTexture,
            Rectangle{ 0.0f, 0.0f, (float)earthTexture.width, -(float)earthTexture.height },
            destRec,
            Vector2{ 0.0f, 0.0f },
            90.0f,
            WHITE
        );

        // Draw a nice border around the map
        DrawRectangleLinesEx(Rectangle{ mapX, mapY, mapWidth, mapHeight }, 3.0f, DARKGRAY);

        // Draw projected ground track points
        if (!groundTrack.empty()) {
            std::vector<Vector2> projected2D;
            projected2D.reserve(groundTrack.size());

            std::vector<double> longitudes;
            longitudes.reserve(groundTrack.size());

            for (const auto& ecefPt : groundTrack) {
                double dist = std::sqrt(ecefPt.x * ecefPt.x + ecefPt.y * ecefPt.y + ecefPt.z * ecefPt.z);
                if (dist > 0.0f) {
                    double lon = std::atan2(ecefPt.y, ecefPt.x); // -PI to +PI
                    double lat = std::asin(ecefPt.z / dist);      // -PI/2 to +PI/2

                    // Calculate screen coordinates based on standard equirectangular mapping
                    float x = mapX + mapWidth * static_cast<float>((lon + M_PI) / (2.0 * M_PI));
                    float y = mapY + mapHeight * static_cast<float>((M_PI / 2.0 - lat) / M_PI);

                    projected2D.push_back(Vector2{ x, y });
                    longitudes.push_back(lon);
                }
            }

            // Draw the ground track lines with anti-meridian wrap-around check
            for (size_t i = 0; i < projected2D.size() - 1; ++i) {
                if (std::abs(longitudes[i + 1] - longitudes[i]) < M_PI) {
                    DrawLineEx(projected2D[i], projected2D[i + 1], 2.5f, RED);
                }
            }

            // Draw a marker for the current satellite position
            if (!projected2D.empty()) {
                DrawCircleV(projected2D.back(), 5.0f, GREEN);
                DrawCircleLinesV(projected2D.back(), 7.0f, WHITE);
            }
        }

        // Draw a clean UI Header & "Back to 3D" button
        DrawRectangle(0, 0, screenWidth, 60, ColorAlpha(BLACK, 0.6f));
        DrawText("Earth Ground Track (2D Equirectangular Projection)", 20, 20, 20, RAYWHITE);

        // Back Button
        Rectangle backBtn = { (float)screenWidth - 160.0f, 15.0f, 130.0f, 32.0f };
        bool hovered = CheckCollisionPointRec(GetMousePosition(), backBtn);
        DrawRectangleRec(backBtn, hovered ? RED : DARKGRAY);
        DrawRectangleLinesEx(backBtn, 1.5f, RAYWHITE);
        DrawText("Back to 3D", (int)backBtn.x + 18, (int)backBtn.y + 7, 16, RAYWHITE);

        if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            show2DMap = false;
        }

        return;
    }

    BeginMode3D(camera);
        
        rlPushMatrix();
            rlRotatef(90.0f, 1.0f, 0.0f, 0.0f);
            DrawGrid(100, 5.0f);
        rlPopMatrix();
        
        DrawModel(earthModel, Vector3{ 0.0f, 0.0f, 0.0f }, 1.0f, WHITE);

        if (isEarthHovered) {
            rlDisableDepthMask();
            DrawSphereWires(Vector3{0.0f, 0.0f, 0.0f}, 1.06f, 16, 16, Color{0, 200, 255, 100});
            DrawSphere(Vector3{0.0f, 0.0f, 0.0f}, 1.06f, Color{0, 200, 255, 35});
            rlEnableDepthMask();
        }

        DrawSphere(
            toRaylibVector(spacecraftPosition),
            0.05f,
            GREEN
        );

        // ECI
        DrawCylinderEx(
            Vector3{ 0.0f, 0.0f, 0.0f }, 
            Vector3{ 2.5f, 0.0f, 0.0f }, 
            0.030f, 
            0.030f, 
            8, 
            RED
        ); // X-axis (Vernal Equinox)
        
        DrawCylinderEx(
            Vector3{ 0.0f, 0.0f, 0.0f }, 
            Vector3{ 0.0f, 2.5f, 0.0f }, 
            0.030f, 
            0.030f, 
            8, 
            LIME
        ); // Y-axis (Orthogonal Equator)
        
        DrawCylinderEx(
            Vector3{ 0.0f, 0.0f, 0.0f }, 
            Vector3{ 0.0f, 0.0f, 2.5f }, 
            0.030f, 
            0.030f, 
            8, 
            SKYBLUE
        ); // Z-axis (Polar Axis)

        drawDashedLine3D(
            toRaylibVector(bounds.descending), 
            toRaylibVector(bounds.ascending), 
            0.15f,
            YELLOW
        ); // Line of nodes

        // Draw the ground track.
        if (!groundTrack.empty()) {
            std::vector<Vector3> projectedPoints;
            projectedPoints.reserve(groundTrack.size());

            double cosAngle = std::cos(earthRotationAngle);
            double sinAngle = std::sin(earthRotationAngle);

            for (const auto& ecefPt : groundTrack) {
                // Rotate ECEF point back to ECI
                Helios::Math::Vector3D pt = {
                    ecefPt.x * cosAngle - ecefPt.y * sinAngle,
                    ecefPt.x * sinAngle + ecefPt.y * cosAngle,
                    ecefPt.z
                };
                Vector3 r = toRaylibVector(pt);
                float dist = Vector3Length(r);
                if (dist > 0.0f) {
                    projectedPoints.push_back({
                        (r.x / dist) * 1.001f,
                        (r.y / dist) * 1.001f,
                        (r.z / dist) * 1.001f * (1.0f - static_cast<float>(
                            Helios::Physics::EARTH_FLATTENING
                        )) // Oblateness factor
                    });
                }
            }

            // Draw the projected ground track line (e.g., in RED)
            for (size_t i = 0; i < projectedPoints.size() - 1; ++i) 
                DrawLine3D(projectedPoints[i], projectedPoints[i + 1], RED);
            
        }

        // Draw the orbit path.
        for (int i = 0; i < orbitPoints.size() - 1; ++i)
            DrawLine3D(
                toRaylibVector(orbitPoints[i]),
                toRaylibVector(orbitPoints[i + 1]),
                ORANGE
            );

        // Connect the last point to the first to close the orbit loop.
        if (!orbitPoints.empty()) {
            DrawLine3D(
                toRaylibVector(orbitPoints.back()),
                toRaylibVector(orbitPoints.front()),
                ORANGE
            );
        }
    
    EndMode3D();
}