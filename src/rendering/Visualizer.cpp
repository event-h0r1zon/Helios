#include "physics/KeplerianSolver.hpp"
#include "Visualizer.hpp"
#include "rlgl.h"
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
}

Vector3 toRaylibVector(const Vector3D& vec) {
    return Vector3 { float(vec.x), float(vec.y), float(vec.z) };
}

double deg2rad(double degrees) {
    return degrees * M_PI / 180.0;
}

void Visualizer::updateOrbitPoints(const KeplerianElements& elements) {
    orbitPoints.clear();
    int resolution = 360; // Number of points to calculate along the orbit.

    for (int i = 0; i < resolution; ++i) {
        double theta = (2.0 * M_PI * i) / resolution; // True anomaly
        double p = elements.a * (1 - std::pow(elements.e, 2));
        double r = p / (1 + elements.e * std::cos(theta));

        // Local orbital plane coordinates.
        
        double x_local = r * std::cos(theta);
        double y_local = r * std::sin(theta);
        double z_local = 0;

        Vector3D localPos = { x_local, y_local, z_local };

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

        Vector3D worldPos = rotationTotal * localPos;

        orbitPoints.push_back(worldPos);
    }
}

void Visualizer::render(const KeplerianElements& elements) {
    BeginMode3D(camera);
        
        rlPushMatrix();
            rlRotatef(90.0f, 1.0f, 0.0f, 0.0f);
            DrawGrid(20, 1.0f);
        rlPopMatrix();
        
        DrawSphere({0, 0, 0}, 1.0f, BLUE);
        DrawSphereWires({0, 0, 0}, 1.0f, 16, 16, LIME);

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