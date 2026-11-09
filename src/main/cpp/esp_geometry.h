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

// A projected edge segment. valid is zero when the edge is fully behind the
// homogeneous near plane or contains invalid coordinates.
struct EspScreenSegment {
    float x1, y1, depth1;
    float x2, y2, depth2;
    std::uint32_t valid;
};

struct EspAabbEdge { std::uint8_t a, b; };

// Returns the 12 canonical box edges, indexed into the eight projected corners.
ESP_API const EspAabbEdge* esp_aabb_edges();

// Matrix is a column-major 4x4 matrix (OpenGL convention), multiplied by
// column vectors: clip = view_projection * vec4(world, 1).
ESP_API int esp_project_aabb(
    const EspAabb* bounds,
    const float* view_projection_16,
    float viewport_width,
    float viewport_height,
    EspScreenPoint* out_points_8);

// Projects all 12 AABB edges and clips edges crossing the w=epsilon plane.
// This preserves visible edge fragments when some box corners are behind camera.
ESP_API int esp_project_aabb_edges(
    const EspAabb* bounds,
    const float* view_projection_16,
    float viewport_width,
    float viewport_height,
    EspScreenSegment* out_segments_12);

}
