#pragma once
#include <vector>
#include "raylib.h"
#include "physics/KeplerianSolver.hpp"

struct NodeBounds {
    Helios::Math::Vector3D descending;
    Helios::Math::Vector3D ascending;
};

class Visualizer {
    private:
        Helios::Math::Vector3D spacecraftPosition;
        std::vector<Helios::Math::Vector3D> groundTrack; // Ground track projection.
        Model earthModel;
        std::vector<Helios::Math::Vector3D> orbitPoints;
        NodeBounds bounds; // Relevant for the line of nodes.
        Helios::Math::Matrix3x3 rotation;
        float earthRotationAngle = 0.0f;
        bool hasPreviousElements = false;
        bool hasPreviousTimeScale = false;
        KeplerianElements previousElements;
        double previousTimeScale = 0.0;

        Texture2D earthTexture;
        bool show2DMap = false;
        bool isEarthHovered = false;
    public:
        Visualizer();

        void updateOrbitPoints(const KeplerianElements& elements);
        void update(
            const KeplerianElements& elements, 
            double timeScale, 
            size_t groundTrackPoints,
            const Camera3D& camera
        );
        void render(const Camera3D& camera, const KeplerianElements& elements);
};