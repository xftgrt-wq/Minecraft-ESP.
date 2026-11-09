#pragma once

#include "esp_geometry.h"

#include <cstddef>

// Submits one complete overlay frame to LeviLauncher's HUD renderer.
// Player AABB edges are red; dropped-item AABB edges are green.
// Pass screen-space segments returned by esp_project_aabb_edges().
bool esp_overlay_submit_frame(const EspScreenSegment* player_segments,
                              std::size_t player_count,
                              const EspScreenSegment* item_segments,
                              std::size_t item_count,
                              float thickness = 2.0f);

// Clears the module's current HUD draw commands.
void esp_overlay_clear();
