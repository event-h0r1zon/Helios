#include "KeplerianSolver.hpp"
#include <cmath>

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

double KeplerianSolver::solve(const KeplerianElements& elements, double time) {

    return 0.0;
}