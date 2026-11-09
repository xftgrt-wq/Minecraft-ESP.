# Major finding: TAA upscaling owns Current/Previous ViewProjection uniforms — 2026-11-09

## Binary identity

- Target: Minecraft Bedrock Android ARM64 libminecraftpe.so
- SHA-256: d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1

## Descriptor group identified

At VA 0x1290cf28 the code materializes the string address 0x21e2297 and registers descriptor object at 0x149100a0. The string at 0x21e2297 is exactly CurrentViewProjectionMatrixUniform.

At VA 0x1290cf88 it registers descriptor 0x149100b0 with string RenderResolution. Nearby registrations map the descriptor block as follows:

| Descriptor address | Uniform name |
|---|---|
| 0x14910080 | ResolutionRatiosAndFPEpsilon |
| 0x14910090 | PreviousViewProjectionMatrixUniform |
| 0x149100a0 | CurrentViewProjectionMatrixUniform |
| 0x149100b0 | RenderResolution |
| 0x149100c0 | DisplayResolution |
| 0x149100d0 | RecipDisplayResolution |
| 0x149100e0 | SubPixelJitterAndValidHistory |
| 0x149100f0 | CurrentWorldOrigin |
| 0x14910100 | PreviousWorldOrigin |
| 0x14910110 | TAAUpscalingParameters |

These names are grouped in one initialization/registration routine. The binary also contains the RTTI/schema type name dragon::framerenderer::modules::UpscalingParameters and the pass/resource name TAAUpscaling.

## Source fields

The same routine uses:
- x21 + 0x248 as the source block for PreviousViewProjectionMatrixUniform (descriptor 0x14910090);
- x21 + 0x208 as the source block for CurrentViewProjectionMatrixUniform (descriptor 0x149100a0).

Both are passed to the same descriptor-keyed 64-byte store helper. The 64-byte size is consistent with a 4x4 float matrix. The paired previous/current matrices, current/previous world origins, render/display resolution, jitter, and TAA parameters identify this as temporal-AA/upscaling render state, not an arbitrary UI matrix block.

## Confidence and limits

High confidence: x21+0x208 is the current ViewProjection uniform payload consumed by the TAA upscaling uniform-registration path in this binary.

Not yet proven: that this payload is identical to the matrix used by every world-render pass or is available at a safe hook point for an ESP overlay. Matrix layout, multiplication convention, camera-relative origin handling, and update timing still need runtime validation.

This is a substantial camera-matrix lead, but it is not yet sufficient to hardcode an offset into a production ESP.

## Next steps

1. Identify the producer/type of the third argument x21 and its lifetime.
2. Trace how the TAA upscaling module populates x21+0x208 and x21+0x248.
3. Compare the current matrix uniform's coordinate space with entity AABB coordinates; account for CurrentWorldOrigin.
4. Find a render callback where the matrix and viewport dimensions are simultaneously valid.
5. Continue independent ECS runtime iteration and Hitbox/AABB component access analysis.

No working in-game ESP or installable production library is claimed.