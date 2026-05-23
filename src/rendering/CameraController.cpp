#include "CameraController.hpp"
#include "raymath.h"
#include "imgui.h"
#include <cmath>

void CameraController::update(Camera3D& camera) {
    if (!ImGui::GetIO().WantCaptureMouse) {
        // Calculate vector from target to camera (camera-relative position)
        Vector3 direction = Vector3Subtract(camera.position, camera.target);
        float radius = Vector3Length(direction);
        if (radius <= 0.0f) return;
        
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
}
