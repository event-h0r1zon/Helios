#pragma once
#include "physics/KeplerianSolver.hpp"

class GUIController {
    private:
        double timeScale = 100.0; // Time scale factor for simulation speed control.
        double groundTrackPoints = 10000.0; // Points to keep in the ground track history.
    public:
        GUIController() = default;

        // Renders the ImGui interface and updates the shared orbit elements state
        void draw(KeplerianElements& elements);
        double getTimeScale() const { return timeScale; }
        size_t getGroundTrackPoints() const { return static_cast<size_t>(groundTrackPoints); }
};