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

bool finite(const Vec4& v) {
    return std::isfinite(v.x) && std::isfinite(v.y) &&
           std::isfinite(v.z) && std::isfinite(v.w);
}

bool valid_input(const EspAabb* b, const float* m, float width,
                 float height, const void* out) {
    if (!b || !m || !out || !std::isfinite(width) || !std::isfinite(height) ||
        width <= 0.0f || height <= 0.0f ||
        !std::isfinite(b->min_x) || !std::isfinite(b->min_y) ||
        !std::isfinite(b->min_z) || !std::isfinite(b->max_x) ||
        !std::isfinite(b->max_y) || !std::isfinite(b->max_z) ||
        b->min_x > b->max_x || b->min_y > b->max_y ||
        b->min_z > b->max_z)
        return false;
    for (int i = 0; i < 16; ++i)
        if (!std::isfinite(m[i])) return false;
    return true;
}

void make_corners(const EspAabb& b, Vec4* out, const float* m) {
    const float corners[8][3] = {
        {b.min_x, b.min_y, b.min_z}, {b.max_x, b.min_y, b.min_z},
        {b.min_x, b.max_y, b.min_z}, {b.max_x, b.max_y, b.min_z},
        {b.min_x, b.min_y, b.max_z}, {b.max_x, b.min_y, b.max_z},
        {b.min_x, b.max_y, b.max_z}, {b.max_x, b.max_y, b.max_z}
    };
    for (int i = 0; i < 8; ++i)
        out[i] = transform(m, corners[i][0], corners[i][1], corners[i][2]);
}

EspScreenPoint to_screen(const Vec4& clip, float width, float height) {
    const float inv_w = 1.0f / clip.w;
    const float ndc_x = clip.x * inv_w;
    const float ndc_y = clip.y * inv_w;
    const float ndc_z = clip.z * inv_w;
    return {(ndc_x + 1.0f) * 0.5f * width,
            (1.0f - ndc_y) * 0.5f * height, ndc_z, 1};
}

float interpolate(float a, float b, float t) {
    return a + (b - a) * t;
}

Vec4 interpolate(const Vec4& a, const Vec4& b, float t) {
    return {interpolate(a.x, b.x, t), interpolate(a.y, b.y, t),
            interpolate(a.z, b.z, t), interpolate(a.w, b.w, t)};
}

// Clip a homogeneous line segment to the OpenGL clip volume. The first
// half-space is w >= epsilon; the remaining six are -w <= x,y,z <= w.
bool clip_to_frustum(Vec4& a, Vec4& b) {
    if (!finite(a) || !finite(b)) return false;
    const float fa[7] = {
        a.w - kWEpsilon, a.x + a.w, a.w - a.x,
        a.y + a.w, a.w - a.y, a.z + a.w, a.w - a.z
    };
    const float fb[7] = {
        b.w - kWEpsilon, b.x + b.w, b.w - b.x,
        b.y + b.w, b.w - b.y, b.z + b.w, b.w - b.z
    };
    float t_enter = 0.0f;
    float t_exit = 1.0f;
    for (int i = 0; i < 7; ++i) {
        if (!std::isfinite(fa[i]) || !std::isfinite(fb[i]) ||
            (fa[i] < 0.0f && fb[i] < 0.0f))
            return false;
        if (fa[i] < 0.0f || fb[i] < 0.0f) {
            const float denominator = fa[i] - fb[i];
            if (std::fabs(denominator) <= kWEpsilon) return false;
            const float t = fa[i] / denominator;
            if (fa[i] < 0.0f) t_enter = t > t_enter ? t : t_enter;
            else t_exit = t < t_exit ? t : t_exit;
            if (t_enter > t_exit) return false;
        }
    }
    const Vec4 original_a = a;
    const Vec4 original_b = b;
    a = interpolate(original_a, original_b, t_enter);
    b = interpolate(original_a, original_b, t_exit);
    return finite(a) && finite(b) && a.w > 0.0f && b.w > 0.0f;
}
}

extern "C" const EspAabbEdge* esp_aabb_edges() {
    static constexpr EspAabbEdge edges[12] = {
        {0, 1}, {0, 2}, {1, 3}, {2, 3},
        {4, 5}, {4, 6}, {5, 7}, {6, 7},
        {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };
    return edges;
}

extern "C" int esp_project_aabb(
    const EspAabb* b, const float* m, float width, float height,
    EspScreenPoint* out) {
    if (!valid_input(b, m, width, height, out))
        return 0;

    Vec4 corners[8];
    make_corners(*b, corners, m);
    int valid_count = 0;
    for (int i = 0; i < 8; ++i) {
        out[i] = {};
        if (!finite(corners[i]) || corners[i].w <= kWEpsilon)
            continue;
        out[i] = to_screen(corners[i], width, height);
        if (!std::isfinite(out[i].x) || !std::isfinite(out[i].y) ||
            !std::isfinite(out[i].depth)) {
            out[i] = {};
            continue;
        }
        ++valid_count;
    }
    return valid_count;
}

extern "C" int esp_project_aabb_edges(
    const EspAabb* b, const float* m, float width, float height,
    EspScreenSegment* out) {
    if (!valid_input(b, m, width, height, out))
        return 0;

    Vec4 corners[8];
    make_corners(*b, corners, m);
    const EspAabbEdge* edges = esp_aabb_edges();
    int valid_count = 0;

    for (int i = 0; i < 12; ++i) {
        out[i] = {};
        Vec4 a = corners[edges[i].a];
        Vec4 c = corners[edges[i].b];
        if (!clip_to_frustum(a, c))
            continue;

        const EspScreenPoint p1 = to_screen(a, width, height);
        const EspScreenPoint p2 = to_screen(c, width, height);
        if (!std::isfinite(p1.x) || !std::isfinite(p1.y) ||
            !std::isfinite(p1.depth) || !std::isfinite(p2.x) ||
            !std::isfinite(p2.y) || !std::isfinite(p2.depth))
            continue;

        out[i] = {p1.x, p1.y, p1.depth, p2.x, p2.y, p2.depth, 1};
        ++valid_count;
    }
    return valid_count;
}
