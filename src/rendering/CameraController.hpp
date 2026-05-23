#pragma once
#include "raylib.h"

class CameraController {
public:
    CameraController() = default;

    // Handles user orbiting (RMB drag) and zooming (mouse wheel scroll) updates
    void update(Camera3D& camera);
};
