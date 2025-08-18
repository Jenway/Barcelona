#pragma once
#include "aabb.hpp" // Include AABB definition
#include "hittable.hpp"
#include <cmath>
#include <limits> // For std::numeric_limits
#include <utility>

class Cylinder : public Hittable {
public:
    Vec3 center;
    Vec3 axis;
    double radius;
    double height;
    std::shared_ptr<Material> mat_ptr;

    Cylinder(Vec3 c, const Vec3& a, double r, double h, std::shared_ptr<Material> m)
        : center(std::move(c))
        , axis(a.normalized())
        , radius(r)
        , height(h)
        , mat_ptr(std::move(m))
    {
    }

    // finite cylinder: side surface + caps
    [[nodiscard]] std::optional<HitRecord> hit(const Ray& r, double t_min, double t_max) const override
    {
        // project ray direction and oc onto plane perpendicular to axis
        Vec3 d = r.dir;
        Vec3 oc = r.origin - center;
        // components perpendicular to axis
        Vec3 d_perp = d - axis * d.dot(axis);
        Vec3 oc_perp = oc - axis * oc.dot(axis);

        double a = d_perp.dot(d_perp);
        double b = 2.0 * d_perp.dot(oc_perp);
        double c = oc_perp.dot(oc_perp) - radius * radius;

        double best_t = t_max;
        bool found = false;
        HitRecord bestRec;

        // Solve quadratic for side intersection if a != 0
        if (std::fabs(a) > 1e-12) {
            double disc = b * b - 4 * a * c;
            if (disc >= 0.0) {
                double sq = std::sqrt(disc);
                double t0 = (-b - sq) / (2 * a);
                double t1 = (-b + sq) / (2 * a);
                auto check_t = [&](double t) {
                    if (t <= t_min || t >= best_t)
                        return;
                    Vec3 p = r.at(t);
                    // check projection along axis relative to center
                    double proj = (p - center).dot(axis);
                    if (proj >= -height * 0.5 - 1e-6 && proj <= height * 0.5 + 1e-6) {
                        // compute normal (point - projection onto axis)
                        Vec3 projPoint = center + axis * proj;
                        Vec3 n = (p - projPoint);
                        if (n.squaredNorm() > 1e-12)
                            n.normalize();
                        else
                            n = axis; // fallback
                        if (n.dot(r.dir) > 0)
                            n = -n;
                        best_t = t;
                        bestRec.t = t;
                        bestRec.point = p;
                        bestRec.normal = n;
                        bestRec.material = mat_ptr;
                        found = true;
                    }
                };
                check_t(t0);
                check_t(t1);
            }
        }
        // Check caps (disks) intersections
        auto check_cap = [&](const Vec3& cap_center) {
            double denom = axis.dot(r.dir);
            if (std::fabs(denom) < 1e-12)
                return; // parallel to cap plane
            double t = (cap_center - r.origin).dot(axis) / denom;
            if (t <= t_min || t >= best_t)
                return;
            Vec3 p = r.at(t);
            if ((p - cap_center).squaredNorm() <= radius * radius + 1e-6) {
                Vec3 n = axis;
                // ensure normal points against ray dir
                if (n.dot(r.dir) > 0)
                    n = -n;
                best_t = t;
                bestRec.t = t;
                bestRec.point = p;
                bestRec.normal = n;
                bestRec.material = mat_ptr;
                found = true;
            }
        };
        Vec3 cap_top = center + axis * (height * 0.5);
        Vec3 cap_bottom = center - axis * (height * 0.5);
        check_cap(cap_top);
        check_cap(cap_bottom);

        if (found)
            return bestRec;
        return std::nullopt;
    }

    [[nodiscard]] AABB bounding_box() const override
    {
        // A robust and correct, though not perfectly tight, AABB for an arbitrarily oriented cylinder.
        // It's the AABB of the two end caps expanded by the radius.

        Vec3 half_height_vec = axis * (height * 0.5);
        Vec3 p1 = center - half_height_vec;
        Vec3 p2 = center + half_height_vec;

        // An axis-aligned bounding box that contains the cylinder's axis line
        Vec3 min_seg = p1.cwiseMin(p2);
        Vec3 max_seg = p1.cwiseMax(p2);

        // Expand this box by the radius in all directions.
        Vec3 expansion(radius, radius, radius);
        return { min_seg - expansion, max_seg + expansion };
    }
};
