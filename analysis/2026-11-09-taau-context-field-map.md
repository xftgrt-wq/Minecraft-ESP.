# TAA upscaling callback context field map

Date: 2026-11-09

Context: ARM64 Bedrock binary Build ID `2e318db12824cadb2618754ab7c82fa96fb30659`.

The callback RTTI ties the function at VA `0x1290c4a8` to the `dragon::framerenderer::modules::taau` render-graph callable and its `TAAContext` argument. The following are observed reads relative to the callback's `x21` context pointer.

| TAAContext offset | Observed operation | Uniform descriptor |
|---|---|---|
| `+0x60` | pointer to uniform/resource store object; helper receives pointer at `+0x8` | multiple |
| `+0x1e8` | 16-bit dimension value | RenderResolution / DisplayResolution / RecipDisplayResolution |
| `+0x1ea` | second 16-bit dimension value | RenderResolution / DisplayResolution / RecipDisplayResolution |
| `+0x1ec` | 8-byte pair copied to temporary payload | SubPixelJitterAndValidHistory |
| `+0x1f4` | byte tested to produce history-valid scalar | SubPixelJitterAndValidHistory |
| `+0x1f8` | float used in resolution ratio and render-size scaling | ResolutionRatiosAndFPEpsilon / RenderResolution |
| `+0x204` | byte converted to boolean-like float | TAAUpscalingParameters |
| `+0x205` | byte converted to boolean-like float | TAAUpscalingParameters |
| `+0x208` | 64-byte block passed to uniform-store helper | CurrentViewProjectionMatrixUniform |
| `+0x248` | 64-byte block passed to uniform-store helper | PreviousViewProjectionMatrixUniform |
| `+0x288` | first 8 bytes of current origin payload | CurrentWorldOrigin |
| `+0x290` | final 4 bytes of current origin payload | CurrentWorldOrigin |
| `+0x29c` | final 4 bytes of previous origin payload | PreviousWorldOrigin |

The two 16-bit dimensions are converted to float for display resolution, multiplied by the float at `+0x1f8` for render resolution, and reciprocated for reciprocal display resolution. The byte at `+0x1f4` controls the valid-history scalar, while the two bytes at `+0x204` and `+0x205` are converted into two boolean-like float values in the TAA parameter payload.

## What this gives the project

This is the strongest camera/render data lead found in the exact uploaded binary so far:
- the current and previous view-projection payloads are directly identified;
- the callback context is associated with the TAA upscaling module;
- the current origin is sourced from the same context object;
- render/display dimensions and history validity are also visible.

## What it does not give

These are private TAA pass context fields, not a supported SDK. Their values and lifetime may be valid only while the TAA pass callback is executing. The main remaining engineering task is to find a supported render callback or prove that this pass context can safely be observed without breaking the renderer. Entity AABB extraction and the correct world-origin transform also remain unresolved.
