#pragma once

#include "esp_geometry.h"

#include <cstddef>
#include <cstdint>

enum class EspOverlayKind : std::uint8_t {
    Player,
    DroppedItem
};

// Submit projected screen-space segments to LeviLauncher's HUD overlay.
// Player outlines are red; dropped-item outlines are green.
bool esp_overlay_submit_segments(const EspScreenSegment* segments,
                                 std::size_t count,
                                 EspOverlayKind kind,
                                 float thickness = 2.0f);

void esp_overlay_clear();
