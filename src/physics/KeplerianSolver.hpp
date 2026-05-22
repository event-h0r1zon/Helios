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

struct Matrix3x3 {
    double m[3][3];
};

struct Vector3D {
    double x;
    double y;
    double z;
};

Vector3D operator*(const Matrix3x3& mat, const Vector3D& vec);
Matrix3x3 operator*(const Matrix3x3& A, const Matrix3x3& B);

class KeplerianSolver {
    public:
        static double solve(const KeplerianElements& elements, double time);
};