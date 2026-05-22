#pragma once
#include <vector>
#include "raylib.h"
#include "physics/KeplerianSolver.hpp"

class Visualizer {
    private:
        Camera3D camera;
        std::vector<Vector3D> orbitPoints;
    public:
        Visualizer();

        void updateOrbitPoints(const KeplerianElements& elements);
        void render(const KeplerianElements& elements);
};