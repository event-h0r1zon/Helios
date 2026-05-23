#include "GUIController.hpp"
#include "imgui.h"
#include "physics/Constants.hpp"

// Helper function to draw a linked slider and input box on the same line
bool DrawLinkedInput(const char* label, double& value, float minVal, float maxVal, const char* format) {
    ImGui::PushID(label); // Prevents ID conflicts in ImGui
    
    ImGui::Text("%s", label);
    
    float val = static_cast<float>(value);
    bool changed = false;

    // Put slider and input field on the same line
    ImGui::SetNextItemWidth(250.0f);
    changed |= ImGui::SliderFloat("##slider", &val, minVal, maxVal, format);
    
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.0f);
    changed |= ImGui::InputFloat("##input", &val, 0.0f, 0.0f, "%.3f");

    if (changed) {
        // Clamp values to prevent physics explosions (e.g. negative semi-major axis)
        if (val < minVal) val = minVal;
        if (val > maxVal) val = maxVal;
        value = static_cast<double>(val);
    }
    
    ImGui::PopID();
    ImGui::Spacing();
    
    return changed;
}

void GUIController::draw(KeplerianElements& elements) {
    // Begin the main control window
    ImGui::Begin("Helios Mission Control");

    if (ImGui::BeginTabBar("ControlTabs")) {
        
        // Tab 1: Keplerian Elements
        if (ImGui::BeginTabItem("Keplerian Elements")) {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "Orbit Geometry Configuration");
            ImGui::Separator();
            ImGui::Spacing();

            // Link sliders and inputs for each orbital parameter
            DrawLinkedInput(
                "Semi-Major Axis (a) [km]", 
                elements.a, 
                6500.0f, 
                100000.0f, 
                "%.1f"
            );
            
            float maxE = 1.0f - static_cast<float>(
                Helios::Physics::EARTH_RADIUS_KM / elements.a
            ); // Prevent orbit from intersecting Earth
            
            DrawLinkedInput(
                "Eccentricity (e)", 
                elements.e, 
                0.0f, 
                maxE, 
                "%.3f"
            );
            
            DrawLinkedInput(
                "Inclination (i) [deg]", 
                elements.i, 
                0.0f, 
                180.0f, 
                "%.1f"
            );

            DrawLinkedInput(
                "Right Ascension of Ascending Node (RAAN) [deg]", 
                elements.RAAN, 
                0.0f, 
                360.0f, 
                "%.1f"
            );

            DrawLinkedInput(
                "Argument of Periapsis (w) [deg]", 
                elements.omega, 
                0.0f, 
                360.0f, 
                "%.1f"
            );

            DrawLinkedInput(
                "Mean Anomaly at Epoch (M0) [deg]", 
                elements.M0, 
                0.0f, 
                360.0f, 
                "%.1f"
            );

            ImGui::EndTabItem();
        }

        // Tab 2: Simulation Controls
        if (ImGui::BeginTabItem("Simulation Controls")) {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "Time Control");
            ImGui::Separator();
            ImGui::Spacing();

            DrawLinkedInput(
                "Time Scale (N/A)", 
                timeScale, 
                1.0f, 
                10000.0f, 
                "%.1f"
            );

            DrawLinkedInput(
                "Ground Track Points", 
                groundTrackPoints, 
                1000.0f, 
                50000.0f, 
                "%.0f"
            );

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}