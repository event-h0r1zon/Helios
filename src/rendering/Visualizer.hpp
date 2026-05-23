#pragma once
#include <vector>
#include "raylib.h"
#include "physics/KeplerianSolver.hpp"

struct NodeBounds {
    Vector3D descending;
    Vector3D ascending;
};

class Visualizer {
    private:
        Camera3D camera;
        Model earthModel;
        std::vector<Vector3D> orbitPoints;
        NodeBounds bounds; // Relevant for the line of nodes.
    public:
        Visualizer();

        void updateOrbitPoints(const KeplerianElements& elements);
        void update(const KeplerianElements& elements);
        void render(const KeplerianElements& elements);
};