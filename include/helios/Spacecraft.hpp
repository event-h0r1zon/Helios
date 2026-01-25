#pragma once
#include <Eigen/Dense>
#include <iosfwd>

namespace helios {

    using StateVec = Eigen::Matrix<double, 6, 1>;

    class Spacecraft {
        private:
            StateVec state;
            double mass; // kg
            double inertia; // kg*m^2
        public:
            Spacecraft(StateVec init_state, double init_mass) : state(init_state), mass(init_mass) { }
            StateVec get_state() const { return state; }
            void update(StateVec new_state) { state = new_state; }
            void log_state();
            static void write_csv_header(std::ostream& os);
            void write_state_to_csv(std::ostream& os, double t) const;
    };

}