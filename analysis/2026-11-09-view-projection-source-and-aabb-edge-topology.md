# Follow-up: ViewProjection uniform source classification and box topology — 2026-11-09

## Matrix classification update

The descriptor initialization sequence around VA 0x1290c4a8..0x1290d560 registers a coherent TAA upscaling uniform set. The descriptor at 0x149100a0 is initialized with the exact name CurrentViewProjectionMatrixUniform. The neighboring descriptor at 0x14910090 is PreviousViewProjectionMatrixUniform.

The update path copies 64 bytes from x21+0x208 into CurrentViewProjectionMatrixUniform and 64 bytes from x21+0x248 into PreviousViewProjectionMatrixUniform. Additional descriptors in the same group are RenderResolution, DisplayResolution, RecipDisplayResolution, SubPixelJitterAndValidHistory, CurrentWorldOrigin, PreviousWorldOrigin, and TAAUpscalingParameters.

This establishes x21+0x208 as the current ViewProjection uniform payload for the TAA upscaling path in this target binary. It is a strong render-camera matrix lead. It does not by itself prove that this is the correct matrix for every render pass or that an ESP can safely hook this update path.

## Geometry module extension

Added a canonical 12-edge topology for the eight projected AABB corners:
- exported esp_aabb_edges() API;
- 12 edge pairs connecting the eight corners;
- unit test coverage for edge count and index validity.

Local Clang C++17 build with -Wall -Wextra -Werror, AddressSanitizer, and UndefinedBehaviorSanitizer passed the projection and edge-topology tests.

## Remaining integration work

1. Identify the concrete type and lifetime of the third argument x21.
2. Trace the producer of x21+0x208 and x21+0x248 and validate matrix convention.
3. Determine whether CurrentWorldOrigin is required to transform entity coordinates into the matrix's coordinate space.
4. Identify a render callback that provides a stable overlay draw opportunity.
5. Identify a safe ECS iteration path and exact entity AABB source for this game build.

No live ECS iterator, entity component offsets, render hook, or installable production ESP library has been verified.