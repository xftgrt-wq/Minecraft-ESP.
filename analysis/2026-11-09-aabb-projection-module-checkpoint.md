# AABB projection module checkpoint — 2026-11-09

## Changes committed

Added:
- `src/main/cpp/esp_geometry.h`
- `src/main/cpp/esp_geometry.cpp`

Updated:
- `CMakeLists.txt` to compile the geometry module.

The exported C function `esp_project_aabb` accepts a world-space AABB, a 4x4 column-major OpenGL-style view-projection matrix, and viewport dimensions. It projects the eight AABB corners to screen coordinates and marks points with invalid/non-positive clip-space W as invalid.

## Scope and limitations

This is a standalone geometry stage only. It does not:
- discover entities or iterate Bedrock ECS storage;
- read HitboxComponent/AABBShapeComponent fields;
- obtain the live camera matrix;
- install hooks or render boxes;
- establish compatibility with Minecraft 1.26.33 / LeviLauncher 1.5.13.

The matrix layout and multiplication convention are explicit assumptions at the API boundary. They must be checked against the actual world renderer before connecting this function to game data. The caller must also handle partially clipped boxes and connect the projected corners into edges.

## Binary availability

The target `libminecraftpe.so` is not mounted in the current execution workspace, so this checkpoint does not claim a new disassembly result or any newly verified target offsets.

## Next required evidence

1. Reattach the exact target `libminecraftpe.so` to continue function-boundary and unwind/FDE analysis.
2. Identify a confirmed runtime ECS access path for entity AABBs.
3. Identify and validate the world renderer's actual view-projection matrix source and matrix convention.
4. Only then connect entity data, projection, and a renderer callback.

No installable ESP build is claimed by this checkpoint.
