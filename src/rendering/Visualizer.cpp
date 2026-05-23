#include "physics/KeplerianSolver.hpp"
#include "Visualizer.hpp"
#include "rlgl.h"
#include "raymath.h"
#include "imgui.h"
#include "physics/Constants.hpp"
#include <cmath>

Visualizer::Visualizer() {
    // Position the camera slightly elevated and back from the center
    
    camera.position = Vector3{ 35.0f, 0.0f, 20.0f };
    // Look directly at the center of the orbit (Earth at 0, 0, 0)
    camera.target = Vector3{ 0.0f, 0.0f, 0.0f };

    camera.up = Vector3{ 0.0f, 0.0f, 1.0f };

    // Camera Field of View in degrees
    camera.fovy = 45.0f;

    // Use perspective projection for standard 3D depth perception
    camera.projection = CAMERA_PERSPECTIVE;

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
    Texture2D earthTexture = LoadTexture("assets/earth.png");
    earthModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = earthTexture;
}

Vector3 toRaylibVector(const Vector3D& vec) {
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

    Matrix3x3 rotationRAAN = {
        {
            { std::cos(RAAN), -std::sin(RAAN), 0.0 },
            { std::sin(RAAN), std::cos(RAAN), 0.0 },
            { 0.0, 0.0, 1.0 }
        }
    };

    Matrix3x3 rotationInclination = {
        {
            { 1.0, 0.0, 0.0 },
            { 0.0, std::cos(inc), -std::sin(inc) },
            { 0.0, std::sin(inc), std::cos(inc) }
        }
    };

    Matrix3x3 rotationArgPeriapsis = {
        {
            { std::cos(omega), -std::sin(omega), 0.0 },
            { std::sin(omega), std::cos(omega), 0.0 },
            { 0.0, 0.0, 1.0 }
        }
    };

    Matrix3x3 rotationTotal = rotationRAAN * rotationInclination * rotationArgPeriapsis;
    
    double p = elements.a * (1.0 - std::pow(elements.e, 2));

    double r_ascending = p / (1.0 + elements.e * std::cos(omega));
    double r_descending = p / (1.0 - elements.e * std::cos(omega));

    bounds.ascending = {
        r_ascending * std::cos(RAAN),
        r_ascending * std::sin(RAAN),
        0.0
    };

    bounds.descending = {
        -r_descending * std::cos(RAAN),
        -r_descending * std::sin(RAAN),
        0.0
    };

    for (int i = 0; i < resolution; ++i) {
        double theta = (2.0 * M_PI * i) / resolution; // True anomaly
        double p = elements.a * (1 - std::pow(elements.e, 2));
        double r = p / (1 + elements.e * std::cos(theta));

        // Local orbital plane coordinates.
        
        double x_local = r * std::cos(theta);
        double y_local = r * std::sin(theta);
        double z_local = 0;

        Vector3D localPos = { x_local, y_local, z_local };

        Vector3D worldPos = rotationTotal * localPos;
        orbitPoints.push_back(worldPos);
    }
}

void Visualizer::update(const KeplerianElements& elements) {
    if (!ImGui::GetIO().WantCaptureMouse) {
        // Calculate vector from target to camera (camera-relative position)
        Vector3 direction = Vector3Subtract(camera.position, camera.target);
        float radius = Vector3Length(direction);
        
        // Calculate current spherical angles (Yaw and Pitch)
        // Since camera.up is Z-up (0, 0, 1):
        // Pitch (latitude) is the angle relative to the X-Y equatorial plane
        float pitch = asinf(direction.z / radius);
        // Yaw (longitude) is the angle in the X-Y plane
        float yaw = atan2f(direction.y, direction.x);

        // Handle Orbit Rotation (Only when Right Mouse Button is pressed)
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            Vector2 mouseDelta = GetMouseDelta();
            
            float sensitivity = 0.005f; 
            yaw -= mouseDelta.x * sensitivity;
            pitch += mouseDelta.y * sensitivity;
            
            // Clamp pitch to prevent flipping upside down at the poles (gimbal lock)
            constexpr float maxPitch = 89.0f * DEG2RAD;
            if (pitch > maxPitch) pitch = maxPitch;
            if (pitch < -maxPitch) pitch = -maxPitch;
        }

        // Handle Zooming (Mouse Wheel)
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) {
            float zoomSensitivity = 1.5f;
            radius -= wheel * zoomSensitivity;
            
            // Clamp distance so we don't zoom inside the Earth or too far out
            if (radius < 1.5f) radius = 1.5f;
            if (radius > 100.0f) radius = 100.0f;
        }

        // Recompute the new camera position
        camera.position.x = camera.target.x + radius * cosf(pitch) * cosf(yaw);
        camera.position.y = camera.target.y + radius * cosf(pitch) * sinf(yaw);
        camera.position.z = camera.target.z + radius * sinf(pitch);
    }

    updateOrbitPoints(elements);
}

void Visualizer::render(const KeplerianElements& elements) {
    BeginMode3D(camera);
        
        rlPushMatrix();
            rlRotatef(90.0f, 1.0f, 0.0f, 0.0f);
            DrawGrid(100, 5.0f);
        rlPopMatrix();
        
        DrawModel(earthModel, Vector3{ 0.0f, 0.0f, 0.0f }, 1.0f, WHITE);

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

        if (!orbitPoints.empty()) {
            std::vector<Vector3> projectedPoints;
            projectedPoints.reserve(orbitPoints.size());

            for (const auto& pt : orbitPoints) {
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
            for (size_t i = 0; i < projectedPoints.size() - 1; ++i) {
                DrawLine3D(projectedPoints[i], projectedPoints[i + 1], RED);
            }
            // Close the loop
            DrawLine3D(projectedPoints.back(), projectedPoints.front(), RED);
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