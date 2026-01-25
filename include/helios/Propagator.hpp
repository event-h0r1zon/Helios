#pragma once
#include "Spacecraft.hpp"

namespace helios {
    class Propagator {
        public:
            void step(Spacecraft& sc, double dt);
            StateVec compute_state_change(const StateVec& state);
    };
}