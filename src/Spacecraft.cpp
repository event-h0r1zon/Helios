#include "helios/Spacecraft.hpp"
#include <iostream>

namespace helios {

    void Spacecraft::log_state() {
        std::cout << "Position: (" << state(0) << ", " << state(1) << "), Orientation: " << state(2) << " rad\n";
        std::cout << "Velocity: (" << state(3) << ", " << state(4) << "), Angular Velocity: " << state(5) << " rad/s\n";
    }

    void Spacecraft::write_csv_header(std::ostream& os) {
        os << "t,x,y,theta,vx,vy,omega,mass\n";
    }

    void Spacecraft::write_state_to_csv(std::ostream& os, double t) const {
        os << t << ','
        << state(0) << ',' << state(1) << ',' << state(2) << ','
        << state(3) << ',' << state(4) << ',' << state(5) << ','
        << mass
        << '\n';
    }
}
