#pragma once
#include <vector>

struct KeplerianElements {
    double a; // Semi-major axis
    double e; // Eccentricity
    double i; // Inclination
    double omega; // Argument of periapsis
    double RAAN; // Right ascension of the ascending node
    double M0; // Mean anomaly
};

namespace Helios::Math {

struct Vector3D {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    Vector3D() = default;
    Vector3D(double x_val, double y_val, double z_val) : x(x_val), y(y_val), z(z_val) {}
};

struct Matrix3x3 {
    double m[3][3] = {{0.0}};

    Matrix3x3() = default;
    Matrix3x3(double m00, double m01, double m02,
              double m10, double m11, double m12,
              double m20, double m21, double m22) {
        m[0][0] = m00; m[0][1] = m01; m[0][2] = m02;
        m[1][0] = m10; m[1][1] = m11; m[1][2] = m12;
        m[2][0] = m20; m[2][1] = m21; m[2][2] = m22;
    }
};

Vector3D operator*(const Matrix3x3& mat, const Vector3D& vec);
Matrix3x3 operator*(const Matrix3x3& A, const Matrix3x3& B);

} // namespace Helios::Math

class KeplerianSolver {
    public:
        static double solve(const KeplerianElements& elements, double time);
};