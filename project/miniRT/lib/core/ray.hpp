#pragma once
#include <utility>

#include "vec3.hpp"

struct Ray {
    Vec3 origin;
    Vec3 dir;
    Ray(Vec3 o, const Vec3& d)
        : origin(std::move(o))
        , dir(d.normalized())
    {
    }
    [[nodiscard]] Vec3 at(double t) const { return origin + t * dir; }
};
