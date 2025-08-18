#include "renderer.hpp"

#include "bvh.hpp"
#include "hittable.hpp"
#include "ray.hpp"

#include "constants.hpp"
#include <algorithm>
#include <cmath>
#include <execution>
#include <iostream>
#include <random>
#include <ranges>

using std::ranges::views::iota;

static double random_double()
{
    thread_local static std::mt19937_64 rng([] {
        std::random_device rd;
        return (static_cast<uint64_t>(rd()) << 32) ^ rd();
    }());
    thread_local static std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng);
}
static Vec3 ray_color(const Ray& r, const Scene& scene, int current_depth)
{
    if (current_depth <= 0) {
        return Vec3(0, 0, 0); // 递归深度耗尽 -> 黑
    }

    // 统一使用归一化方向（避免不同分支对 r.dir 的不同假设）
    Vec3 rd = r.dir.normalized();

    std::optional<HitRecord> closest_hit;
    if (scene.world) {
        // 使用一个较小的 t_min（来自常量），避免自相交
        closest_hit = scene.world->hit(r, RTConstants::EPSILON_RAY_HIT, std::numeric_limits<double>::max());
    }

    if (!closest_hit) {
        // 背景色（线性空间）
        double t = 0.5 * (rd.y() + 1.0);
        return (1.0 - t) * Vec3(1.0, 1.0, 1.0) + t * Vec3(0.5, 0.7, 1.0);
    }

    // 解包材质（和你代码风格一致）
    const auto& [diffuse_color, specular_color, shininess, reflectivity] = *(closest_hit->material);

    // 环境光（如果你希望 ambient 不被 tint，可以修改这行）
    Vec3 ambient = scene.ambient_light.ratio * scene.ambient_light.color.cwiseProduct(diffuse_color);

    Vec3 total_diffuse_light(0, 0, 0);
    Vec3 total_specular_light(0, 0, 0);

    // 对每个光源进行简单的点光源计算（无距离衰减；可选：乘以 1/(d*d)）
    for (const auto& light : scene.lights) {
        Vec3 Ldir = (light.position - closest_hit->point);
        double light_distance = Ldir.norm();
        if (light_distance <= 0.0)
            continue;
        Vec3 L = Ldir / light_distance; // normalized

        // 阴影检测：从交点沿法线偏移，再发一条到光源的射线
        Ray shadow_ray(closest_hit->point + closest_hit->normal * RTConstants::EPSILON_SHADOW_OFFSET, L);
        double tmax = std::max(0.0, light_distance - RTConstants::EPSILON_RAY_HIT);
        bool inShadow = false;
        if (scene.world) {
            if (scene.world->intersects_any(shadow_ray, RTConstants::EPSILON_RAY_HIT, tmax))
                inShadow = true;
        }

        if (!inShadow) {
            // 漫反射 (Lambert)
            double lambertian = std::max(0.0, closest_hit->normal.dot(L));
            total_diffuse_light += light.brightness * lambertian * light.color.cwiseProduct(diffuse_color);

            // Blinn-Phong 高光：使用 -rd 作为视向量
            Vec3 V = (-rd).normalized(); // 视向量（从交点指向相机）
            Vec3 H = (L + V).normalized();
            double spec = std::pow(std::max(0.0, closest_hit->normal.dot(H)), shininess);
            total_specular_light += light.brightness * spec * light.color.cwiseProduct(specular_color);
        }
    }

    // 反射（递归）
    Vec3 reflection_color(0, 0, 0);
    if (reflectivity > 0.0) {
        Vec3 in = rd; // 归一化入射方向
        Vec3 reflected_dir = in - 2.0 * in.dot(closest_hit->normal) * closest_hit->normal;
        reflected_dir = reflected_dir.normalized();
        Ray reflected_ray(closest_hit->point + closest_hit->normal * RTConstants::EPSILON_SHADOW_OFFSET, reflected_dir);
        reflection_color = ray_color(reflected_ray, scene, current_depth - 1);
    }

    // 汇总（保持线性）
    Vec3 final_color = ambient + total_diffuse_light + total_specular_light + reflection_color * reflectivity;

    // 不在这里 clamp（保留 HDR 值给后端 tone-mapping）
    return final_color;
}

void render_one_sample_pass(const Scene& scene, int image_width, int image_height, std::vector<Vec3>& framebuffer, int ray_max_depth)
{
    framebuffer.resize(static_cast<size_t>(image_width) * image_height);

    if (!scene.world) {
        std::cerr << "Warning: Scene has no objects to render.\n";
        return;
    }

    // --- Camera Setup (same as before) ---
    const double aspect_ratio = static_cast<double>(image_width) / image_height;
    Vec3 origin = scene.camera.origin;
    Vec3 look_dir = scene.camera.orientation;
    double fov = scene.camera.fov;
    double theta = fov * M_PI / 180.0;
    double h = std::tan(theta / 2.0);
    double viewport_height = 2.0 * h;
    double viewport_width = viewport_height * aspect_ratio;

    Vec3 w = -look_dir.normalized();
    Vec3 u = Vec3(0, 1, 0).cross(w).normalized();
    // Handle case where look_dir is aligned with world up
    if (u.norm() < 1e-6) {
        u = Vec3(0, 0, 1).cross(w).normalized();
    }
    Vec3 v = w.cross(u);

    Vec3 horizontal = viewport_width * u;
    Vec3 vertical = viewport_height * v;
    double focal_length = 1.0;
    Vec3 lower_left_corner = origin - horizontal / 2.0 - vertical / 2.0 - w * focal_length;

    // --- Main Pixel Loop ---
    // This loop calculates ONE sample for each pixel.
    std::for_each(std::execution::par_unseq,
        iota(0, image_width * image_height).begin(),
        iota(0, image_width * image_height).end(),
        [&](int idx) {
            int j = idx / image_width;
            int i = idx % image_width;

            // KEY CHANGE: Add a random offset for Anti-Aliasing
            // This makes each pass slightly different, averaging to a smooth image.
            double u_param = (static_cast<double>(i) + random_double()) / (image_width - 1);
            double v_param = (static_cast<double>(image_height - 1 - j) + random_double()) / (image_height - 1);

            Vec3 dir = lower_left_corner + u_param * horizontal + v_param * vertical - origin;
            Ray r(origin, dir.normalized());

            // We directly write the result of ONE ray color calculation.
            framebuffer[idx] = ray_color(r, scene, ray_max_depth);
        });
}

// void render(const Scene& scene, int image_width, int image_height, std::vector<Vec3>& framebuffer, int ray_max_depth)
// {
//     framebuffer.resize(static_cast<size_t>(image_width) * image_height);

//     if (!scene.world) {
//         std::cerr << "Warning: Scene has no objects to render (BVH world is null).\n";
//         return;
//     }

//     const double aspect_ratio = static_cast<double>(image_width) / image_height;

//     // 相机设置
//     Vec3 origin = scene.camera.origin;
//     Vec3 look_dir = scene.camera.orientation;
//     double fov = scene.camera.fov;

//     double theta = fov * M_PI / 180.0;
//     double h = std::tan(theta / 2.0);
//     double viewport_height = 2.0 * h;
//     double viewport_width = viewport_height * aspect_ratio;

//     // 相机局部基（u = right, v = up, w = -forward）
//     Vec3 w = -look_dir.normalized();
//     Vec3 world_up(0, 1, 0);
//     if (std::abs(w.dot(world_up)) > 0.999) {
//         world_up = Vec3(0, 0, 1);
//         if (std::abs(w.dot(world_up)) > 0.999) {
//             world_up = Vec3(1, 0, 0);
//         }
//     }
//     Vec3 u = world_up.cross(w).normalized();
//     Vec3 v = w.cross(u);

//     Vec3 horizontal = viewport_width * u;
//     Vec3 vertical = viewport_height * v;

//     // focal_length = 1.0: 视口中心在 origin - w * focal_length
//     double focal_length = 1.0;
//     Vec3 lower_left_corner = origin - horizontal / 2.0 - vertical / 2.0 - w * focal_length;

//     auto t0 = std::chrono::high_resolution_clock::now();

//     std::for_each(std::execution::par_unseq,
//         iota(0, image_width * image_height).begin(),
//         iota(0, image_width * image_height).end(),
//         [&](int idx) {
//             int j = idx / image_width;
//             int i = idx % image_width;

//             double u_param = static_cast<double>(i) / (image_width - 1);
//             double v_param = static_cast<double>(image_height - 1 - j) / (image_height - 1);

//             Vec3 dir = lower_left_corner + u_param * horizontal + v_param * vertical - origin;
//             dir = dir.normalized(); // **重要**：确保方向归一化
//             Ray r(origin, dir);

//             framebuffer[(j * image_width) + i] = ray_color(r, scene, ray_max_depth);
//         });

//     auto t1 = std::chrono::high_resolution_clock::now();
//     double seconds = std::chrono::duration<double>(t1 - t0).count();
//     std::cerr << "\nRender time: " << seconds << " s\n";
// }