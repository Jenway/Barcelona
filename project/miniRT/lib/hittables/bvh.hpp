#pragma once

#include "aabb.hpp"
#include "hittable.hpp"
#include <memory>
#include <vector>

class BVHNode : public Hittable {
public:
    BVHNode();
    BVHNode(std::vector<std::shared_ptr<Hittable>>& src_objects, size_t start, size_t end);

    [[nodiscard]] std::optional<HitRecord> hit(const Ray& r, double t_min, double t_max) const override;
    bool intersects_any(const Ray& r, double t_min, double t_max) const;
    [[nodiscard]] AABB bounding_box() const override;

    std::shared_ptr<Hittable> left;
    std::shared_ptr<Hittable> right;
    AABB box;
};

// BVH 构造函数辅助函数
std::shared_ptr<Hittable> bvh_node_from_objects(
    std::vector<std::shared_ptr<Hittable>>& src_objects,
    size_t start, size_t end);
