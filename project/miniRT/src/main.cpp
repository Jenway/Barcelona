// src/main_improved.cpp - 支持上下视角（pitch）与鼠标视角控制
// 改动说明：
//  - CameraController 现在使用 yaw/pitch 两个角度表示朝向（避免万向节问题），并从角度重建 orientation
//  - 添加鼠标相对移动以控制视角（水平控制 yaw，垂直控制 pitch）
//  - 支持用方向键（Up/Down）对 pitch 进行微调
//  - pitch 在 [-PI/2+eps, PI/2-eps] 范围内夹住，避免看直上/直下导致数值不稳定

#include "image_utils.hpp"
#include "parser.hpp"
#include "renderer.hpp"
#include "scene.hpp"

#include <SDL2/SDL.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <exception>
#include <iostream>
#include <numbers>
#include <string>
#include <vector>

constexpr double BASE_MOVE_SPEED = 2.0;
constexpr double BASE_ROTATION_SPEED = 1.5;
constexpr int DEFAULT_WIDTH = 800;
constexpr int DEFAULT_HEIGHT = 450;
static constexpr double PI = std::numbers::pi;
static constexpr int DEFAULT_RAY_MAX_DEPTH = 2;

struct FrameTimer {
    using clock = std::chrono::high_resolution_clock;
    clock::time_point last = clock::now();
    double delta_time = 0.0; // seconds

    int frame_count = 0;
    double acc = 0.0;
    int fps = 0;

    void tick()
    {
        auto now = clock::now();
        delta_time = std::chrono::duration<double>(now - last).count();
        last = now;

        frame_count++;
        acc += delta_time;
        if (acc >= 1.0) {
            fps = frame_count;
            frame_count = 0;
            acc -= 1.0;
        }
    }
};

struct CameraController {
    Vec3 origin;
    Vec3 orientation; // forward unit vector
    Vec3 right;
    Vec3 up;

    // angles in radians
    double yaw = 0.0; // rotation around world Y
    double pitch = 0.0; // rotation around camera right

    CameraController() = default;
    explicit CameraController(const Camera& c)
        : origin(c.origin)
        , yaw(std::atan2(orientation.x(), orientation.z()))
    {
        orientation = c.orientation.normalized();
        // initialize yaw / pitch from orientation
        // pitch = asin(y)
        double oy = std::clamp(orientation.y(), -1.0, 1.0);
        pitch = std::asin(oy);
        // yaw = atan2(x, z) -> so that yaw=0 means facing +Z

        rebuild_from_angles();
    }

    void rebuild_from_angles()
    {
        // clamp pitch slightly away from +/- PI/2
        double eps = 1e-4;
        double max_pitch = (PI / 2.0) - eps;
        pitch = std::clamp(pitch, -max_pitch, max_pitch);

        double cp = std::cos(pitch);
        double sp = std::sin(pitch);
        double cy = std::cos(yaw);
        double sy = std::sin(yaw);

        // orientation: x = cos(pitch)*sin(yaw), y = sin(pitch), z = cos(pitch)*cos(yaw)
        orientation.x() = cp * sy;
        orientation.y() = sp;
        orientation.z() = cp * cy;
        orientation = orientation.normalized();
        update_vectors();
    }

    void update_vectors()
    {
        Vec3 world_up(0, 1, 0);
        right = world_up.cross(orientation).normalized();
        if (right.norm() < 1e-6) {
            right = Vec3(0, 0, 1).cross(orientation).normalized();
        }
        up = orientation.cross(right).normalized();
    }

    void move_forward(double distance) { origin += orientation * distance; }
    void strafe_right(double distance) { origin += right * distance; }
    void move_up(double distance) { origin.y() = origin.y() + distance; }

    void add_yaw(double delta)
    {
        yaw += delta;
        // wrap yaw to avoid overflow
        if (yaw > PI)
            yaw -= 2.0 * PI;
        if (yaw < -PI)
            yaw += 2.0 * PI;
        rebuild_from_angles();
    }

    void add_pitch(double delta)
    {
        pitch += delta;
        // clamp in rebuild
        rebuild_from_angles();
    }
};

static void handle_input(const Uint8* keystate, CameraController& cam, bool& moved, double dt)
{
    const double move_amount = BASE_MOVE_SPEED * dt;
    const double rot_amount = BASE_ROTATION_SPEED * dt;

    // Keyboard moving
    if (keystate[SDL_SCANCODE_W] != 0U) {
        cam.move_forward(move_amount);
        moved = true;
    }
    if (keystate[SDL_SCANCODE_S] != 0U) {
        cam.move_forward(-move_amount);
        moved = true;
    }
    if (keystate[SDL_SCANCODE_A] != 0U) {
        cam.strafe_right(-move_amount);
        moved = true;
    }
    if (keystate[SDL_SCANCODE_D] != 0U) {
        cam.strafe_right(move_amount);
        moved = true;
    }
    if (keystate[SDL_SCANCODE_SPACE] != 0U) {
        cam.move_up(move_amount);
        moved = true;
    }
    if (keystate[SDL_SCANCODE_LSHIFT] != 0U) {
        cam.move_up(-move_amount);
        moved = true;
    }

    if (keystate[SDL_SCANCODE_Q] != 0U) {
        cam.add_yaw(-rot_amount);
        moved = true;
    }
    if (keystate[SDL_SCANCODE_E] != 0U) {
        cam.add_yaw(rot_amount);
        moved = true;
    }

    if (keystate[SDL_SCANCODE_UP] != 0U) {
        cam.add_pitch(-rot_amount);
        moved = true;
    }
    if (keystate[SDL_SCANCODE_DOWN] != 0U) {
        cam.add_pitch(rot_amount);
        moved = true;
    }
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <scene.rt> [--exposure <value>] [--max-depth <value>]\n";
        std::cerr << "Options:\n";
        std::cerr << "  --exposure <value>    Set exposure value (default: " << DEFAULT_EXPOSURE << ")\n";
        std::cerr << "  --depth <value>   Set maximum ray depth (default: " << DEFAULT_RAY_MAX_DEPTH << ")\n";
        return 1;
    }

    double exposure = DEFAULT_EXPOSURE;
    int max_depth = DEFAULT_RAY_MAX_DEPTH;

    // Parse command line options
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--exposure" && i + 1 < argc) {
            exposure = std::stod(argv[++i]);
        } else if (arg == "--depth" && i + 1 < argc) {
            max_depth = std::stoi(argv[++i]);
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            return 1;
        }
    }

    Scene scene;
    try {
        scene = Parser::parse_file(argv[1]);
    } catch (const std::exception& e) {
        std::cerr << "Error parsing scene file: " << e.what() << "\n";
        return 1;
    }

    CameraController cam(scene.camera);

    const int image_width = DEFAULT_WIDTH;
    const int image_height = DEFAULT_HEIGHT;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    SDL_ShowCursor(SDL_DISABLE);

    Uint32 window_flags = SDL_WINDOW_SHOWN;
    SDL_Window* window = SDL_CreateWindow("MiniRT Interactive", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, image_width, image_height, window_flags);
    if (window == nullptr) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, image_width, image_height);
    if (texture == nullptr) {
        std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << "\n";
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    std::vector<Uint32> pixel_buffer;
    pixel_buffer.resize(static_cast<size_t>(image_width) * image_height);
    std::vector<Vec3> accumulation_buffer;
    accumulation_buffer.resize(static_cast<size_t>(image_width) * image_height, Vec3(0, 0, 0));
    std::vector<Vec3> single_sample_framebuffer;
    single_sample_framebuffer.resize(static_cast<size_t>(image_width) * image_height);

    bool running = true;
    bool cam_moved = true;
    int samples_per_pixel = 0;

    FrameTimer timer;

    // 主循环
    while (running) {
        // 1) 事件处理
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT)
                running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
                running = false;
        }

        // 2) 时间更新
        timer.tick();
        double dt = std::max(1e-6, timer.delta_time);

        // 4) 键盘状态处理
        const Uint8* keystate = SDL_GetKeyboardState(nullptr);
        handle_input(keystate, cam, cam_moved, dt);

        if (cam_moved) {
            std::ranges::fill(accumulation_buffer, Vec3(0, 0, 0));
            samples_per_pixel = 0;
            cam_moved = false;
            scene.camera.origin = cam.origin;
            scene.camera.orientation = cam.orientation;
        }

        // 5) 渲染一帧采样
        samples_per_pixel++;
        if (single_sample_framebuffer.size() != pixel_buffer.size())
            single_sample_framebuffer.resize(pixel_buffer.size());

        render_one_sample_pass(scene, image_width, image_height, single_sample_framebuffer, max_depth);

        for (size_t i = 0; i < accumulation_buffer.size(); ++i) {
            accumulation_buffer[i] += single_sample_framebuffer[i];
        }

        const size_t total = pixel_buffer.size();
#pragma omp parallel for
        for (int64_t i = 0; i < static_cast<int64_t>(total); ++i) {
            Vec3 color = accumulation_buffer[static_cast<size_t>(i)] / static_cast<double>(samples_per_pixel);
            color *= exposure;

            color = { color.x() / (1.0 + color.x()), color.y() / (1.0 + color.y()), color.z() / (1.0 + color.z()) };
            color = { std::pow(color.x(), 1.0 / 2.2), std::pow(color.y(), 1.0 / 2.2), std::pow(color.z(), 1.0 / 2.2) };

            Uint8 r = static_cast<Uint8>(std::clamp(color.x(), 0.0, 1.0) * 255.999);
            Uint8 g = static_cast<Uint8>(std::clamp(color.y(), 0.0, 1.0) * 255.999);
            Uint8 b = static_cast<Uint8>(std::clamp(color.z(), 0.0, 1.0) * 255.999);

            pixel_buffer[static_cast<size_t>(i)] = (255U << 24) | (static_cast<Uint32>(r) << 16) | (static_cast<Uint32>(g) << 8) | static_cast<Uint32>(b);
        }

        // 6) 显示
        SDL_UpdateTexture(texture, nullptr, pixel_buffer.data(), image_width * sizeof(Uint32));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        // 更新标题栏
        std::string title = "MiniRT | Samples: " + std::to_string(samples_per_pixel) + " | FPS: " + std::to_string(timer.fps);
        SDL_SetWindowTitle(window, title.c_str());
    }

    // 清理
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    std::cerr << "Done.\n";
    return 0;
}
