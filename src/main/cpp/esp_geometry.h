#pragma once
#include <cstdint>

#if defined(__GNUC__)
#define ESP_API __attribute__((visibility("default")))
#else
#define ESP_API
#endif

extern "C" {

// World-space axis-aligned bounding box.
struct EspAabb {
    float min_x, min_y, min_z;
    float max_x, max_y, max_z;
};

// Screen-space point. valid is zero when clip-space w is too small/non-positive.
struct EspScreenPoint {
    float x, y;
    float depth;
    std::uint32_t valid;
};

// Matrix is a column-major 4x4 matrix (OpenGL convention), multiplied by
// column vectors: clip = view_projection * vec4(world, 1).
ESP_API int esp_project_aabb(
    const EspAabb* bounds,
    const float* view_projection_16,
    float viewport_width,
    float viewport_height,
    EspScreenPoint* out_points_8);

}
