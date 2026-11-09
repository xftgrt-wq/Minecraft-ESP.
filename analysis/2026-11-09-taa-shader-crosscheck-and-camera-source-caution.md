# TAA shader cross-check — 2026-11-09

## Sources checked

A public RenderDragon shader-code mirror exposes the TAA upscaling shader pair:
- `materials/Upscaling/TAAU.Vertex.glsl`
- `materials/Upscaling/TAAU.Fragment.glsl`

The vertex shader is a fullscreen pass. It transforms its input position only from the pass's 0..1 quad coordinates to clip coordinates:

```glsl
vec4 var_c3366 = vec4(a_position, 1.0);
vec2 var_19dcd = (var_c3366.xy * 2.0) - vec2(1.0);
gl_Position = vec4(var_19dcd.x, var_19dcd.y, var_c3366.z, var_c3366.w);
```

The TAA shader's uniform inventory comment lists `CurrentViewProjectionMatrixUniform` and `CurrentWorldOrigin`, but the visible vertex and fragment source do not actively read these values. The fragment shader performs image-space temporal upscaling using screen coordinates, motion vectors, jitter, render/display resolution and TAA history.

## Implication for this project

This is a meaningful caution: the TAA upscaling pass is a fullscreen post-process, not an entity/world geometry pass. The fact that the engine registers the current view-projection descriptor near TAA state, and the callback copies a 64-byte payload into that descriptor, does not prove that the TAA callback is the best or even a usable place to obtain the live world camera matrix for an ESP overlay.

The shader mirror is not proof of the exact shipped Android 1.26.33 shader binary, and comments can preserve uniforms that are optimized out. Therefore this finding is a reason to avoid assuming TAA callback is the camera source, not proof that the matrix payload is invalid.

## Next direction

Prioritize locating a world-render pass or a camera uniform upload path that demonstrably consumes the matrix for world-space vertex transformation. In parallel, continue target-version-specific ECS tracing for a supported actor/entity view and AABB component layout. Keep the TAA hook disabled until its role, ABI and lifetime are independently established.
