#pragma once
#include "physics/KeplerianSolver.hpp"

class GUIController {
public:
    GUIController() = default;
    
    // Renders the ImGui interface and updates the shared orbit elements state
    void draw(KeplerianElements& elements);
};