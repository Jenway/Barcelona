// image_utils.hpp

#ifndef IMAGE_UTILS_HPP
#define IMAGE_UTILS_HPP

#include "vec3.hpp" // Or wherever your Vec3 class is defined
#include <iostream>
#include <string>
#include <vector>

// A sensible default exposure value if the user doesn't provide one.
constexpr float DEFAULT_EXPOSURE = 1.0f;

/**
 * @brief Writes the framebuffer to a PNG file, applying exposure, tone mapping, and gamma correction.
 * @param exposure The exposure value to apply before tone mapping.
 */
bool write_png(const std::string& filename, const std::vector<Vec3>& framebuffer, int image_width, int image_height, float exposure = DEFAULT_EXPOSURE);

/**
 * @brief Displays the framebuffer in a window, applying exposure, tone mapping, and gamma correction.
 * @param exposure The exposure value to apply before tone mapping.
 */
void display_image(const std::string& window_name, const std::vector<Vec3>& framebuffer, int image_width, int image_height, int wait_ms, float exposure = DEFAULT_EXPOSURE);

/**
 * @brief Writes the framebuffer to a PPM file (simple clamp, does not use exposure/tone mapping).
 */
void write_ppm(std::ostream& os, const std::vector<Vec3>& framebuffer, int image_width, int image_height);

#endif // IMAGE_UTILS_HPP