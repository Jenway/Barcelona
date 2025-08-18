// image_utils.cpp
// MODIFIED VERSION

#include "image_utils.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <opencv4/opencv2/opencv.hpp>

// ---------- Configuration (can be changed if needed) ----------
enum ToneMapMode {
    RAW_CLAMP = 0,
    REINHARD = 1,
    EXPONENTIAL = 2
};

static constexpr ToneMapMode TONE_MAP_MODE = EXPONENTIAL;

// --- Helper functions (unchanged) ---
static inline float luminance(const Vec3& c)
{
    return static_cast<float>(0.2126 * c.x() + 0.7152 * c.y() + 0.0722 * c.z());
}

static void compute_luminance_stats(const std::vector<Vec3>& fb, float& minL, float& maxL, float& meanL)
{
    if (fb.empty()) {
        minL = maxL = meanL = 0.0f;
        return;
    }
    minL = std::numeric_limits<float>::infinity();
    maxL = -std::numeric_limits<float>::infinity();
    double acc = 0.0;
    for (const auto& c : fb) {
        float L = luminance(c);
        if (L < minL)
            minL = L;
        if (L > maxL)
            maxL = L;
        acc += L;
    }
    meanL = static_cast<float>(acc / fb.size());
}

// --- Core conversion function (MODIFIED) ---
// It now accepts an exposure parameter.
static cv::Mat convert_framebuffer_to_8bit_bgr(const std::vector<Vec3>& framebuffer, int width, int height, float exposure)
{
    float minL, maxL, meanL;
    compute_luminance_stats(framebuffer, minL, maxL, meanL);
    std::cerr << "[image_utils] Luminance stats -> min: " << minL << "  mean: " << meanL << "  max: " << maxL << "\n";
    std::cerr << "[image_utils] Applying Exposure: " << exposure << "\n";

    cv::Mat float_image_rgb_64f(height, width, CV_64FC3, (void*)framebuffer.data());
    cv::Mat float_image_rgb_32f;
    float_image_rgb_64f.convertTo(float_image_rgb_32f, CV_32FC3);

    cv::Mat image_bgr_float;
    cv::cvtColor(float_image_rgb_32f, image_bgr_float, cv::COLOR_RGB2BGR);

    // --- Tone-mapping and gamma ---
    // REMOVED: const float exposure = EXPOSURE; -- We now use the parameter.
    const float gamma_inv = 1.0f / 2.2f;

    for (int y = 0; y < height; ++y) {
        cv::Vec3f* row = image_bgr_float.ptr<cv::Vec3f>(y);
        for (int x = 0; x < width; ++x) {
            cv::Vec3f px = row[x];
            // Exposure is applied here, using the passed parameter
            px[0] *= exposure;
            px[1] *= exposure;
            px[2] *= exposure;

            if (TONE_MAP_MODE == REINHARD) {
                px[0] = px[0] / (1.0f + px[0]);
                px[1] = px[1] / (1.0f + px[1]);
                px[2] = px[2] / (1.0f + px[2]);
            } else if (TONE_MAP_MODE == EXPONENTIAL) {
                px[0] = 1.0f - std::exp(-px[0]);
                px[1] = 1.0f - std::exp(-px[1]);
                px[2] = 1.0f - std::exp(-px[2]);
            }
            // For all modes (including RAW_CLAMP), clamp to a valid range before gamma.
            px[0] = std::clamp(px[0], 0.0f, 1.0f);
            px[1] = std::clamp(px[1], 0.0f, 1.0f);
            px[2] = std::clamp(px[2], 0.0f, 1.0f);

            // Gamma correction
            px[0] = std::pow(px[0], gamma_inv);
            px[1] = std::pow(px[1], gamma_inv);
            px[2] = std::pow(px[2], gamma_inv);

            row[x] = px;
        }
    }

    cv::Mat final_8bit_bgr;
    image_bgr_float.convertTo(final_8bit_bgr, CV_8UC3, 255.0);
    return final_8bit_bgr;
}

// --- Public functions (MODIFIED) ---

bool write_png(const std::string& filename, const std::vector<Vec3>& framebuffer, int image_width, int image_height, float exposure)
{
    // Pass the exposure parameter down to the core function.
    cv::Mat image_8bit_bgr = convert_framebuffer_to_8bit_bgr(framebuffer, image_width, image_height, exposure);
    return cv::imwrite(filename, image_8bit_bgr);
}

void display_image(const std::string& window_name, const std::vector<Vec3>& framebuffer, int image_width, int image_height, int wait_ms, float exposure)
{
    // Pass the exposure parameter down to the core function.
    cv::Mat image_8bit_bgr = convert_framebuffer_to_8bit_bgr(framebuffer, image_width, image_height, exposure);
    cv::imshow(window_name, image_8bit_bgr);
    cv::waitKey(wait_ms);
}

// write_ppm remains unchanged as it has its own simple pipeline.
void write_ppm(std::ostream& os, const std::vector<Vec3>& framebuffer, int image_width, int image_height)
{
    os << "P3\n"
       << image_width << " " << image_height << "\n255\n";
    for (int j = 0; j < image_height; ++j) {
        for (int i = 0; i < image_width; ++i) {
            const Vec3& col = framebuffer[j * image_width + i];
            int ir = static_cast<int>(255.999 * std::clamp(col.x(), 0.0, 1.0));
            int ig = static_cast<int>(255.999 * std::clamp(col.y(), 0.0, 1.0));
            int ib = static_cast<int>(255.999 * std::clamp(col.z(), 0.0, 1.0));
            os << ir << " " << ig << " " << ib << "\n";
        }
    }
}