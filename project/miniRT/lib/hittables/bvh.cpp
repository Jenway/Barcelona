#include "bvh.hpp"

#include <algorithm>
#include <random>

// 定义 surrounding_box 函数
AABB surrounding_box(const AABB& box0, const AABB& box1)
{
    Vec3 small(fmin(box0.minimum.x(), box1.minimum.x()),
        fmin(box0.minimum.y(), box1.minimum.y()),
        fmin(box0.minimum.z(), box1.minimum.z()));

    Vec3 big(fmax(box0.maximum.x(), box1.maximum.x()),
        fmax(box0.maximum.y(), box1.maximum.y()),
        fmax(box0.maximum.z(), box1.maximum.z()));

    return { small, big };
}

BVHNode::BVHNode() = default;

BVHNode::BVHNode(std::vector<std::shared_ptr<Hittable>>& objects, size_t start, size_t end)
{
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 2);
    int axis = dist(gen); // 线程安全的随机轴选择

    auto comparator = [&](const std::shared_ptr<Hittable>& a, const std::shared_ptr<Hittable>& b) {
        AABB box_a = a->bounding_box();
        AABB box_b = b->bounding_box();
        return box_a.minimum[axis] < box_b.minimum[axis];
    };

    size_t object_span = end - start;

    if (object_span == 1) {
        left = right = objects[start];
    } else if (object_span == 2) {
        if (comparator(objects[start], objects[start + 1])) {
            left = objects[start];
            right = objects[start + 1];
        } else {
            left = objects[start + 1];
            right = objects[start];
        }
    } else {
        // 使用 nth_element 来做中位数划分（通常比完全排序更快）
        size_t mid = start + (object_span / 2);
        std::nth_element(objects.begin() + static_cast<std::ptrdiff_t>(start),
            objects.begin() + static_cast<std::ptrdiff_t>(mid),
            objects.begin() + static_cast<std::ptrdiff_t>(end),
            comparator);

        left = bvh_node_from_objects(objects, start, mid);
        right = bvh_node_from_objects(objects, mid, end);
    }

    AABB box_left = left->bounding_box();
    AABB box_right = right->bounding_box();

    box = surrounding_box(box_left, box_right);
}

std::optional<HitRecord> BVHNode::hit(const Ray& r, double t_min, double t_max) const
{
    if (!box.hit(r, t_min, t_max))
        return std::nullopt;

    std::optional<HitRecord> hit_left_rec;
    std::optional<HitRecord> hit_right_rec;

    // 先检测左子树
    hit_left_rec = left->hit(r, t_min, t_max);

    // 如果左子树有命中，则用左子树的交点 t 作为新的 t_max 来检测右子树
    // 这样可以确保如果右子树有交点，它也必须比左子树的交点更近
    // 如果左子树没有命中，则 t_max 保持不变，右子树在整个 [t_min, t_max] 范围内搜索
    double current_t_max_for_right = t_max;
    if (hit_left_rec) {
        current_t_max_for_right = hit_left_rec->t;
    }
    hit_right_rec = right->hit(r, t_min, current_t_max_for_right);

    // 根据哪个命中存在且更近来返回
    if (hit_right_rec) {
        return hit_right_rec; // 如果右子树命中，它一定是更近的（因为我们用 left_hit->t 限制了它）
    }
    if (hit_left_rec) {
        return hit_left_rec; // 如果右子树没命中，但左子树命中，则返回左子树的命中
    }
    return std::nullopt; // 两个都没有命中
}

bool BVHNode::intersects_any(const Ray& r, double t_min, double t_max) const
{
    if (!box.hit(r, t_min, t_max))
        return false;

    if (left && left->intersects_any(r, t_min, t_max))
        return true;
    if (right && right->intersects_any(r, t_min, t_max))
        return true;
    return false;
}

AABB BVHNode::bounding_box() const
{
    return box;
}

std::shared_ptr<Hittable> bvh_node_from_objects(
    std::vector<std::shared_ptr<Hittable>>& src_objects,
    size_t start, size_t end)
{
    return std::make_shared<BVHNode>(src_objects, start, end);
}
