#include "esp_geometry.h"

#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    const float identity[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    const EspAabb bounds{-0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f};
    EspScreenPoint points[8]{};

    assert(esp_project_aabb(&bounds, identity, 100.0f, 100.0f, points) == 8);
    for (const auto& point : points) {
        assert(point.valid == 1);
        assert(point.x >= 25.0f && point.x <= 75.0f);
        assert(point.y >= 25.0f && point.y <= 75.0f);
    }
    assert(std::fabs(points[0].x - 25.0f) < 1.0e-5f);
    assert(std::fabs(points[0].y - 75.0f) < 1.0e-5f);

    const float negative_w[16] = {
        -1, 0, 0, 0,
         0,-1, 0, 0,
         0, 0,-1, 0,
         0, 0, 0,-1
    };
    assert(esp_project_aabb(&bounds, negative_w, 100.0f, 100.0f, points) == 0);

    const EspAabb invalid_bounds{1, 0, 0, 0, 1, 1};
    assert(esp_project_aabb(&invalid_bounds, identity, 100.0f, 100.0f, points) == 0);
    assert(esp_project_aabb(nullptr, identity, 100.0f, 100.0f, points) == 0);
    assert(esp_project_aabb(&bounds, identity, 0.0f, 100.0f, points) == 0);

    std::cout << "AABB projection tests passed\n";
    return 0;
}
