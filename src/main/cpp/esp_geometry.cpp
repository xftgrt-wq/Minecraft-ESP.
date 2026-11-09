#include "esp_geometry.h"

#include <cmath>

namespace {
constexpr float kWEpsilon = 1.0e-6f;

struct Vec4 { float x, y, z, w; };

Vec4 transform(const float* m, float x, float y, float z) {
    return {
        m[0] * x + m[4] * y + m[8]  * z + m[12],
        m[1] * x + m[5] * y + m[9]  * z + m[13],
        m[2] * x + m[6] * y + m[10] * z + m[14],
        m[3] * x + m[7] * y + m[11] * z + m[15]
    };
}
}

extern "C" int esp_project_aabb(
    const EspAabb* b,
    const float* m,
    float width,
    float height,
    EspScreenPoint* out) {

    if (!b || !m || !out || !std::isfinite(width) ||
        !std::isfinite(height) || width <= 0.0f || height <= 0.0f ||
        b->min_x > b->max_x || b->min_y > b->max_y ||
        b->min_z > b->max_z) {
        return 0;
    }

    const float corners[8][3] = {
        {b->min_x, b->min_y, b->min_z},
        {b->max_x, b->min_y, b->min_z},
        {b->min_x, b->max_y, b->min_z},
        {b->max_x, b->max_y, b->min_z},
        {b->min_x, b->min_y, b->max_z},
        {b->max_x, b->min_y, b->max_z},
        {b->min_x, b->max_y, b->max_z},
        {b->max_x, b->max_y, b->max_z}
    };

    int valid_count = 0;
    for (int i = 0; i < 8; ++i) {
        const Vec4 clip = transform(m, corners[i][0], corners[i][1], corners[i][2]);
        out[i] = {};
        if (!std::isfinite(clip.x) || !std::isfinite(clip.y) ||
            !std::isfinite(clip.z) || !std::isfinite(clip.w) ||
            clip.w <= kWEpsilon) {
            continue;
        }

        const float inv_w = 1.0f / clip.w;
        const float ndc_x = clip.x * inv_w;
        const float ndc_y = clip.y * inv_w;
        const float ndc_z = clip.z * inv_w;
        out[i].x = (ndc_x + 1.0f) * 0.5f * width;
        out[i].y = (1.0f - ndc_y) * 0.5f * height;
        out[i].depth = ndc_z;
        out[i].valid = 1;
        ++valid_count;
    }
    return valid_count;
}
