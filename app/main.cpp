#include <cmath>
#include <iomanip>
#include <iostream>
#include <numbers>
#include <fstream>

#include "helios/Propagator.hpp"
#include "helios/Spacecraft.hpp"

int main() {
  constexpr double mu = 3.986004e14;   // m^3/s^2
  constexpr double r0 = 7000e3;        // m (Earth radius + ~600 km)
  const double v0 = std::sqrt(mu / r0);

  helios::StateVec s0;
  s0 << r0, 0.0, 0.0,   // x, y, theta
        0.0, v0, 0.0;   // vx, vy, omega

  helios::Spacecraft sc{s0, 1000.0};
  helios::Propagator prop;

  std::ofstream csv("spacecraft_log.csv");
  if (!csv.is_open()) {
    std::cerr << "Error: Could not open output file spacecraft_log.csv\n";
    return 1;
  }
  helios::Spacecraft::write_csv_header(csv);

  constexpr double dt = 0.1; // seconds (smaller = more accurate)
  const double period = 2.0 * std::numbers::pi * std::sqrt((r0*r0*r0) / mu);
  const int steps = static_cast<int>(period / dt);

  double t = 0.0;
  for (int i = 0; i <= steps; ++i) {
    sc.write_state_to_csv(csv, t);

    if (i % 60 == 0) { // print every 60 seconds
      const auto s = sc.get_state();
      const double x = s(0), y = s(1);
      const double r = std::sqrt(x*x + y*y);
      sc.log_state();
    }

    prop.step(sc, dt);
    t += dt;
  }
}