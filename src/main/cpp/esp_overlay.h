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

// Projects entity AABBs with one camera matrix, then submits the combined HUD frame.
// The caller must supply live, frame-consistent game data; this function does not scan the world.
bool esp_overlay_submit_aabbs(const EspAabb* player_bounds,
                               std::size_t player_count,
                               const EspAabb* item_bounds,
                               std::size_t item_count,
                               const float* view_projection_16,
                               float viewport_width,
                               float viewport_height,
                               float thickness = 2.0f);
