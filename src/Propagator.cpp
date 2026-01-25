#include "helios/Propagator.hpp"
#include <cmath>

namespace helios {

    void Propagator::step(Spacecraft& sc, double dt) {
        
        // Define the current state.
        StateVec current_state = sc.get_state();

        StateVec k1 = compute_state_change(current_state);
        StateVec k2 = compute_state_change(current_state + 0.5 * dt * k1);
        StateVec k3 = compute_state_change(current_state + 0.5 * dt * k2);
        StateVec k4 = compute_state_change(current_state + dt * k3);

        StateVec new_state = current_state + (dt / 6.0) * (k1 + 2.0 * k2 + 2.0 * k3 + k4);

        sc.update(new_state);
    }

    StateVec Propagator::compute_state_change(const StateVec& state) {
        StateVec delta_state;
        const double mu = 3.986004e14; // Earth's gravitational parameter in m^3/s^2
        
        delta_state(0) = state(3);
        delta_state(1) = state(4);
        delta_state(2) = state(5);
        
        double r = std::sqrt(state(0) * state(0) + state(1) * state(1));
        double r_cu = r * r * r;

        double ax_grav = -mu * state(0) / r_cu;
        double ay_grav = -mu * state(1) / r_cu;

        double ax_thrust = 0.0; // Placeholder for thrust acceleration in x
        double ay_thrust = 0.0; // Placeholder for thrust acceleration in y

        delta_state(3) = ax_grav + ax_thrust;
        delta_state(4) = ay_grav + ay_thrust;
        delta_state(5) = 0.0; // Placeholder for angular acceleration

        return delta_state;
    }

}