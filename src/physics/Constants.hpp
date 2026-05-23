#pragma once

namespace Helios::Physics {
    // Earth physical constants (WGS-84 standard)
    constexpr double EARTH_RADIUS_KM = 6378.137;           // Equatorial radius (a)
    constexpr double EARTH_POLAR_RADIUS_KM = 6356.7523142; // Polar radius (b)
    constexpr double EARTH_FLATTENING = 1.0 - (EARTH_POLAR_RADIUS_KM / EARTH_RADIUS_KM); // ~0.0033528
    
    // Astrodynamics constants
    constexpr double EARTH_MU = 398600.4418;               // Standard gravitational parameter (km^3/s^2)
    constexpr double EARTH_ROTATION_SPEED = 7.2921159e-5;  // Angular velocity magnitude (rad/s)
    
}
