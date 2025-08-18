#pragma once

#include "ray.hpp"
#include "vec3.hpp"

#include <algorithm>
#include <utility>

/**
 * @brief 表示一个轴对齐的边界框 (Axis-Aligned Bounding Box, AABB)。
 *
 * AABB 由其最小点和最大点定义。
 */
class AABB {
public:
    Vec3 minimum;
    Vec3 maximum;

    AABB() = default;
    AABB(Vec3 a, Vec3 b)
        : minimum(std::move(a))
        , maximum(std::move(b))
    {
    }

    /**
     * @brief 检查光线是否与 AABB 相交。
     * @param r 要测试的光线。
     * @param t_min 光线参数的最小值。
     * @param t_max 光线参数的最大值。
     * @return 如果光线与 AABB 相交，则返回 true；否则返回 false。
     */
    [[nodiscard]] bool hit(const Ray& r, double t_min, double t_max) const
    {
        for (int a = 0; a < 3; a++) {
            auto invD = 1.0 / r.dir[a];
            auto t0 = (minimum[a] - r.origin[a]) * invD;
            auto t1 = (maximum[a] - r.origin[a]) * invD;

            if (invD < 0.0F)
                std::swap(t0, t1);

            t_min = std::max(t0, t_min);
            t_max = std::min(t1, t_max);

            if (t_max <= t_min)
                return false;
        }
        return true;
    }
};

/**
 * @brief 计算两个 AABB 的并集。
 * @param box0 第一个 AABB。
 * @param box1 第二个 AABB。
 * @return 包含两个输入 AABB 的最小 AABB。
 */
// Forward declaration for surrounding_box
AABB surrounding_box(const AABB& box0, const AABB& box1);
