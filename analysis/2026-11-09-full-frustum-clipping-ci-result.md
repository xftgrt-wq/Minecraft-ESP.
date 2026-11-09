# Full frustum clipping and verified Android build

Date: 2026-11-09

## Geometry implementation

`esp_project_aabb_edges()` now clips each of the 12 homogeneous AABB edges against:
- the camera half-space `w >= epsilon`;
- left/right clip planes `-w <= x <= w`;
- bottom/top clip planes `-w <= y <= w`;
- OpenGL near/far clip planes `-w <= z <= w`.

The function returns only visible finite segments, including correctly interpolated endpoints where an edge crosses a clip plane. The original corner projection API remains unchanged.

Input validation also rejects non-finite matrix entries and non-finite AABB coordinates.

## Added test cases

- identity transform: all 12 box edges survive;
- negative homogeneous w: no edges survive;
- box crossing the camera plane: clipped front fragments survive;
- box crossing the left clip plane: crossing edges are retained and fully off-screen edges are discarded;
- box entirely outside the right side: no segments survive;
- invalid bounds and null pointers are rejected.

## Verified CI result

GitHub Actions run: https://github.com/xftgrt-wq/Minecraft-ESP./actions/runs/42669487644

Commit: `7b33599717010e9ef3938a61d10ac334dec1777e`

Both jobs completed successfully:
- `host-tests`: compile and execute geometry tests — success;
- `build`: configure Android NDK 27.2.12479018 for `arm64-v8a`, compile shared library, upload artifact — success.

Artifact name: `minecraft-esp-arm64-prototype`.

## Important boundary

The successful Android artifact is still a build prototype. The current `native-lib.cpp` only implements JNI load logging; no game hook, entity enumeration, AABB retrieval, render callback, line drawing, or toggle UI has been implemented. Therefore this CI success proves the native project and geometry module compile for Android ARM64, not that an in-game ESP is ready.
