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
    Texture2D earthTexture = LoadTexture(texturePath.c_str());
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
    size_t groundTrackPoints
) {
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

    // Update the ground track.
    groundTrack.push_back(worldPos);
    if (groundTrack.size() > groundTrackPoints) 
        groundTrack.erase(
            groundTrack.begin(), 
            groundTrack.begin() + (groundTrack.size() - groundTrackPoints)
        );
    
}

void Visualizer::render(const Camera3D& camera, const KeplerianElements& elements) {
    BeginMode3D(camera);
        
        rlPushMatrix();
            rlRotatef(90.0f, 1.0f, 0.0f, 0.0f);
            DrawGrid(100, 5.0f);
        rlPopMatrix();
        
        DrawModel(earthModel, Vector3{ 0.0f, 0.0f, 0.0f }, 1.0f, WHITE);

        DrawSphere(
            toRaylibVector(spacecraftPosition),
            0.02f,
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

            for (const auto& pt : groundTrack) {
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