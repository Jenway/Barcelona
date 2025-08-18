#pragma once

#include "scene.hpp"
#include <vector>

/**
 * @brief 渲染给定的场景，并将结果存储在提供的帧缓冲区中。
 *
 * 帧缓冲区假定为行主序，其中 (j*image_width + i) 对应像素 (i, j)。
 * j=0 对应图像的最顶行。
 * 函数会负责调整帧缓冲区的大小以适应 image_width * image_height。
 *
 * @param scene 待渲染的场景对象。
 * @param image_width 图像的宽度。
 * @param image_height 图像的高度。
 * @param[out] framebuffer 存储渲染结果的 std::vector<Vec3>。
 */
void render(const Scene& scene, int image_width, int image_height, std::vector<Vec3>& framebuffer, int ray_max_depth = 5);

/**
 * @brief Renders a single pass (one sample per pixel) of the scene.
 * This is the core function for progressive/interactive rendering.
 *
 * @param scene The scene to render.
 * @param image_width The width of the output image.
 * @param image_height The height of the output image.
 * @param framebuffer A vector to be filled with the color data of the single pass.
 * @param ray_max_depth The maximum recursion depth for rays.
 */
void render_one_sample_pass(const Scene& scene, int image_width, int image_height, std::vector<Vec3>& framebuffer, int ray_max_depth);