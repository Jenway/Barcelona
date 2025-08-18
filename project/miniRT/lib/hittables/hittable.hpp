// hittable.hpp

#pragma once
#include "ray.hpp"
#include <memory> // For std::shared_ptr
#include <optional>
#include <utility>

class AABB;

// --- Step 1: Enhance the Material structure ---
// It's now a standalone concept.
struct Material {
    Vec3 diffuse_color;
    Vec3 specular_color;
    double shininess;
    double reflectivity;

    // A more descriptive constructor
    Material(Vec3 diffuse, Vec3 specular, double shiny, double reflect)
        : diffuse_color(std::move(diffuse))
        , specular_color(std::move(specular))
        , shininess(shiny)
        , reflectivity(reflect)
    {
    }

    // Default material
    Material()
        : diffuse_color(0.8, 0.8, 0.8)
        , specular_color(0.2, 0.2, 0.2)
        , shininess(32.0)
        , reflectivity(0.0)
    {
    }
};

// --- Step 2: Refactor HitRecord ---
// It now stores a reference to the material, not its properties.
// This is more efficient and flexible.
struct HitRecord {
    double t;
    Vec3 point;
    Vec3 normal;
    std::shared_ptr<const Material> material; // Pointer to the material of the hit object
    bool front_face; // To know if the ray hit the outside or inside

    // Helper to ensure normal always points against the ray
    void set_face_normal(const Ray& r, const Vec3& outward_normal)
    {
        front_face = r.dir.dot(outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

// The Hittable interface remains largely the same
class Hittable {
public:
    virtual ~Hittable() = default;
    [[nodiscard]] virtual std::optional<HitRecord> hit(const Ray& r, double t_min, double t_max) const = 0;
    [[nodiscard]] virtual AABB bounding_box() const = 0;
    [[nodiscard]] virtual bool intersects_any(const Ray& r, double t_min, double t_max) const
    {
        return static_cast<bool>(hit(r, t_min, t_max));
    }
};