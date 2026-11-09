#include "esp_geometry.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

int main() {
    const EspAabbEdge* edges = esp_aabb_edges();
    assert(edges != nullptr);
    for (int i = 0; i < 12; ++i) {
        assert(edges[i].a < 8 && edges[i].b < 8);
        assert(edges[i].a != edges[i].b);
    }

    const float identity[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    const EspAabb bounds{-0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f};
    EspScreenPoint points[8]{};
    EspScreenSegment segments[12]{};

    assert(esp_project_aabb(&bounds, identity, 100.0f, 100.0f, points) == 8);
    for (const auto& point : points) {
        assert(point.valid == 1);
        assert(point.x >= 25.0f && point.x <= 75.0f);
        assert(point.y >= 25.0f && point.y <= 75.0f);
    }
    assert(std::fabs(points[0].x - 25.0f) < 1.0e-5f);
    assert(std::fabs(points[0].y - 75.0f) < 1.0e-5f);
    assert(esp_project_aabb_edges(&bounds, identity, 100.0f, 100.0f, segments) == 12);

    const float negative_w[16] = {
        -1, 0, 0, 0,
         0,-1, 0, 0,
         0, 0,-1, 0,
         0, 0, 0,-1
    };
    assert(esp_project_aabb(&bounds, negative_w, 100.0f, 100.0f, points) == 0);
    assert(esp_project_aabb_edges(&bounds, negative_w, 100.0f, 100.0f, segments) == 0);

    // w=z: four vertical box edges cross the camera plane. Their visible
    // fragments survive, while the four rear-face edges are rejected.
    const float crossing_w[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 1,
        0, 0, 0, 0
    };
    const EspAabb crossing_box{-0.5f, -0.5f, -1.0f, 0.5f, 0.5f, 1.0f};
    assert(esp_project_aabb(&crossing_box, crossing_w, 100.0f, 100.0f, points) == 4);
    assert(esp_project_aabb_edges(&crossing_box, crossing_w, 100.0f, 100.0f, segments) == 8);

    // A box crossing the left clip plane keeps its intersecting edges, but
    // fully off-screen edges are discarded.
    const EspAabb side_crossing{-2.0f, -0.5f, -0.5f, 0.0f, 0.5f, 0.5f};
    assert(esp_project_aabb_edges(&side_crossing, identity, 100.0f, 100.0f, segments) == 8);
    for (const auto& segment : segments) {
        if (!segment.valid) continue;
        assert(segment.x1 >= 0.0f && segment.x1 <= 100.0f);
        assert(segment.x2 >= 0.0f && segment.x2 <= 100.0f);
        assert(segment.y1 >= 0.0f && segment.y1 <= 100.0f);
        assert(segment.y2 >= 0.0f && segment.y2 <= 100.0f);
    }

    const EspAabb outside_box{2.0f, -0.5f, -0.5f, 3.0f, 0.5f, 0.5f};
    assert(esp_project_aabb_edges(&outside_box, identity, 100.0f, 100.0f, segments) == 0);
    // OpenGL far plane: visible face plus four edges clipped at z=w.
    const EspAabb far_crossing{-0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 2.0f};
    assert(esp_project_aabb_edges(&far_crossing, identity, 100.0f, 100.0f, segments) == 8);
    const EspAabb beyond_far{ -0.5f, -0.5f, 2.0f, 0.5f, 0.5f, 3.0f };
    assert(esp_project_aabb_edges(&beyond_far, identity, 100.0f, 100.0f, segments) == 0);

    const EspAabb invalid_bounds{1, 0, 0, 0, 1, 1};
    assert(esp_project_aabb(&invalid_bounds, identity, 100.0f, 100.0f, points) == 0);
    assert(esp_project_aabb_edges(&invalid_bounds, identity, 100.0f, 100.0f, segments) == 0);
    assert(esp_project_aabb(nullptr, identity, 100.0f, 100.0f, points) == 0);
    assert(esp_project_aabb_edges(&bounds, identity, 100.0f, 100.0f, nullptr) == 0);
    assert(esp_project_aabb(&bounds, identity, 0.0f, 100.0f, points) == 0);
    const EspAabb nonfinite_bounds{
        -std::numeric_limits<float>::infinity(), -0.5f, -0.5f,
        0.5f, 0.5f, 0.5f
    };
    assert(esp_project_aabb(&nonfinite_bounds, identity, 100.0f, 100.0f, points) == 0);
    float nonfinite_matrix[16] = {};
    nonfinite_matrix[0] = std::numeric_limits<float>::quiet_NaN();
    assert(esp_project_aabb_edges(&bounds, nonfinite_matrix, 100.0f, 100.0f, segments) == 0);

    std::cout << "AABB projection and frustum clipping tests passed\n";
    return 0;
}
