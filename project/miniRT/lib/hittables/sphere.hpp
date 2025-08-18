#pragma once
#include <utility>

#include "aabb.hpp"
#include "hittable.hpp"
class Sphere : public Hittable {
public:
    Vec3 center;
    double radius;
    std::shared_ptr<Material> mat_ptr;

    Sphere(Vec3 c, double r, std::shared_ptr<Material> m)
        : center(std::move(c))
        , radius(r)
        , mat_ptr(std::move(m))
    {
    }

    [[nodiscard]] std::optional<HitRecord> hit(const Ray& r, double t_min, double t_max) const override
    {
        Vec3 oc = r.origin - center;
        double a = r.dir.squaredNorm();
        double half_b = oc.dot(r.dir);
        double c = oc.squaredNorm() - (radius * radius);
        double discriminant = (half_b * half_b) - (a * c);

        if (discriminant < 0) {
            return std::nullopt;
        }

        double sqrtd = std::sqrt(discriminant);

        // Find the nearest root that lies in the acceptable range.
        double root = (-half_b - sqrtd) / a;
        if (root < t_min || t_max < root) {
            root = (-half_b + sqrtd) / a;
            if (root < t_min || t_max < root) {
                return std::nullopt;
            }
        }

        HitRecord rec;
        rec.t = root;
        rec.point = r.at(rec.t);
        Vec3 outward_normal = (rec.point - center) / radius;
        rec.set_face_normal(r, outward_normal); // Set normal correctly
        rec.material = mat_ptr; // Assign material pointer

        return rec;
    }

    bool intersects_any(const Ray& r, double t_min, double t_max) const override
    {
        Vec3 oc = r.origin - center;
        double a = r.dir.squaredNorm();
        double half_b = oc.dot(r.dir);
        double c = oc.squaredNorm() - (radius * radius);
        double discriminant = (half_b * half_b) - (a * c);
        if (discriminant < 0)
            return false;
        double sqrtd = std::sqrt(discriminant);

        double root = (-half_b - sqrtd) / a;
        if (root >= t_min && root <= t_max)
            return true;
        root = (-half_b + sqrtd) / a;
        return root >= t_min && root <= t_max;
    }

    [[nodiscard]] AABB bounding_box() const override
    {
        Vec3 r_vec(radius, radius, radius);
        return { center - r_vec, center + r_vec };
    }
};
