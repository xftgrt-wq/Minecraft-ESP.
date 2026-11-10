# 2026-11-10 — CameraAimAssist visitor callback spatial-data trace

## Exact target

Uploaded ARM64 `libminecraftpe.so`:
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- Build ID: `2e318db12824cadb2618754ab7c82fa96fb30659`

## Callback target identified

The filtered-view caller at `0x9e7f0d4` stores code address `0x9e4379c` as the callable target in its visitor object. The generic per-entity visitor helper at `0x9e7c91c` invokes that callable indirectly. This resolves the previously unknown callback target for this particular CameraAimAssist-oriented view path.

## Callback behavior observed

At `0x9e4379c`, the callback:
1. Receives several component/context references and packed entity keys through its argument bundle.
2. Performs generation-checked paged-storage lookups for multiple component types.
3. Reads a float at offset `+0x08` from one resolved component slot.
4. Traverses a separate array with a `0x18`-byte element stride. Each element is read as three-component vector-like data (loads at offsets `+0x00`, `+0x08`, `+0x0c`, and `+0x14`).
5. Performs vector min/max comparisons, squared-distance calculations, and midpoint/interpolation arithmetic.
6. Emits or updates compact `0x34`-byte candidate records through helper `0x9e4db78`.

The arithmetic is consistent with spatial candidate filtering around a cached position/frustum-like input. The callback is not a simple AABB copy routine: it combines component data, performs geometric tests, and constructs candidate records.

## Important caution on AABB interpretation

Although the callback is reached from the view that includes AABBShapeComponent, the observed `+0x08` float read alone does not identify the component's full layout or prove that the component contains six min/max floats. The separate `0x18`-stride vector-like records appear to participate in the spatial tests, but their exact owning component/type is not established by this trace.

Therefore this finding must not be used to hardcode a component layout or world-space bounds offsets.

## What is established

- The concrete visitor callback target is `0x9e4379c`.
- It consumes validated entity/component references from the filtered view.
- It performs vector-based spatial calculations and writes compact candidate records.
- The CameraAimAssist view path is a candidate-selection pipeline, not a generic all-entity ESP feed.

## What remains unknown

- The complete callback argument type mapping.
- The concrete component type for the `+0x08` float read and the `0x18`-stride vector records.
- Whether any of these values are world-space bounds.
- Whether all players and dropped items enter this filtered pipeline.
- A stable owner/lifetime and render-thread-safe access path.

## Next target

Trace the producer and consumer of the `0x18`-stride vector records and the component type behind the `+0x08` scalar. Separately inspect other AABBShapeComponent consumers outside CameraAimAssist, preferably one that materializes a bounding-box object rather than performing target selection. The HitboxComponent bounds helper at `0xb58ecd0` remains a candidate comparison point.

## Runtime status

No runtime hook or game-memory dereference was added. The module remains HUD-only and the live ESP feed is not implemented.
