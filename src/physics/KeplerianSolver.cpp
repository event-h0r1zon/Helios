#include "KeplerianSolver.hpp"
#include <cmath>
#include "physics/Constants.hpp"

namespace Helios::Math {

Vector3D operator*(const Matrix3x3& mat, const Vector3D& vec) {
    Vector3D result;
    result.x = mat.m[0][0] * vec.x + mat.m[0][1] * vec.y + mat.m[0][2] * vec.z;
    result.y = mat.m[1][0] * vec.x + mat.m[1][1] * vec.y + mat.m[1][2] * vec.z;
    result.z = mat.m[2][0] * vec.x + mat.m[2][1] * vec.y + mat.m[2][2] * vec.z;
    return result;
}

Matrix3x3 operator*(const Matrix3x3& A, const Matrix3x3& B) {
    Matrix3x3 result;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) 
            result.m[i][j] = 
                A.m[i][0] * B.m[0][j] + 
                A.m[i][1] * B.m[1][j] + 
                A.m[i][2] * B.m[2][j];
    return result;
}

} // namespace Helios::Math

double KeplerianSolver::solve(const KeplerianElements& elements, double time) {
    double n = std::sqrt(Helios::Physics::EARTH_MU / std::pow(elements.a, 3)); // Mean motion
    
    double M0_rad = elements.M0 * M_PI / 180.0; // Convert mean anomaly to radians
    double M = M0_rad + n * time; // Mean anomaly at time t
    M = std::fmod(M, 2.0 * M_PI); // Normalize to [0, 2π]

    if (M < 0) M += 2.0 * M_PI; // Ensure M is positive

    double E = M; // Initial guess for eccentric anomaly
    for (int iter = 0; iter < 100; ++iter) {
        double deltaE = (E - elements.e * std::sin(E) - M) / (1 - elements.e * std::cos(E));
        E -= deltaE;
        if (std::abs(deltaE) < 1e-8) break;
    }

    double theta = 2.0 * std::atan2(std::sqrt(1 + elements.e) * 
        std::sin(E / 2), std::sqrt(1 - elements.e) * std::cos(E / 2));

    return theta;
}