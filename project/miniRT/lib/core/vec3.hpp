#pragma once
#include <Eigen/Dense>

using Vec3 = Eigen::Vector3d;

inline Vec3 normalize(const Vec3& v)
{
    return v.normalized();
}
