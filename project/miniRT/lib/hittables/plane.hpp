#pragma once
#include "aabb.hpp"
#include "hittable.hpp"
#include <cmath>
#include <limits>
#include <utility>

class Plane : public Hittable {
public:
    Vec3 point;
    Vec3 normal;
    std::shared_ptr<Material> mat_ptr;

    Plane(Vec3 p, const Vec3& n, std::shared_ptr<Material> m)
        : point(std::move(p))
        , normal(n.normalized())
        , mat_ptr(std::move(m))
    {
    }

    std::optional<HitRecord> hit(const Ray& r, double t_min, double t_max) const override
    {
        double denom = normal.dot(r.dir);
        if (std::fabs(denom) < 1e-8) // Use a constant here (e.g., RTConstants::EPSILON_PARALLEL_DENOM)
            return std::nullopt;
        double t = (point - r.origin).dot(normal) / denom;
        if (t < t_min || t > t_max)
            return std::nullopt;
        HitRecord rec;
        rec.t = t;
        rec.point = r.at(rec.t);
        Vec3 outward_normal = normal; // Normal is already normalized
        rec.set_face_normal(r, outward_normal); // Set normal correctly
        rec.material = mat_ptr; // Assign material pointer
        return rec;
    }
    bool intersects_any(const Ray& r, double t_min, double t_max) const override
    {
        double denom = normal.dot(r.dir);
        if (std::fabs(denom) < 1e-8)
            return false;
        double t = (point - r.origin).dot(normal) / denom;
        return (t >= t_min && t <= t_max);
    }
    // 平面是无限的，所以返回一个非常大的 AABB
    // 这意味着 BVH 不会有效地加速平面求交，但这是处理无限物体的一种常见方式。
    // 另一种方法是特殊处理无限物体，不将它们放入 BVH。
    AABB bounding_box() const override
    {
        double infinity = std::numeric_limits<double>::infinity();
        return AABB(Vec3(-infinity, -infinity, -infinity), Vec3(infinity, infinity, infinity));
    }
};
