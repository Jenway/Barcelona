#pragma once
#include "hittable.hpp"
#include "vec3.hpp"
#include <memory>
#include <vector>

struct Camera {
    Vec3 origin;
    Vec3 orientation;
    double fov;
};

struct Light {
    Vec3 position;
    double brightness;
    Vec3 color;
};

struct AmbientLight {
    double ratio;
    Vec3 color;
};

struct Scene {
    Camera camera;
    AmbientLight ambient_light;
    std::vector<Light> lights;
    std::shared_ptr<Hittable> world; // Now stores the BVH root

    bool has_camera = false;
    bool has_ambient_light = false;
};
