# 2026-11-10 — view-projection shader uniform evidence

## Input verification
- File: `libminecraftpe.so`
- Size: 349,243,744 bytes
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- ELF64, little-endian, AArch64.

## New static findings

### 1. Renderer shader interfaces name the view-projection matrices
Embedded renderer/shader strings include:
- `u_viewProj`
- `u_prevViewProj`
- `u_invViewProj`
- `u_modelViewProj`
- `CurrentViewProjectionMatrixUniform`
- `PreviousViewProjectionMatrixUniform`

A bundled shader source string contains the expression:
`gl_Position = (u_modelViewProj * tmpvar_1);`

This confirms that the renderer's GPU pipeline uses model-view-projection matrices and also has current, previous, and inverse view-projection uniform concepts.

### 2. Camera-position uniform names
The binary also contains:
- `MinecraftCameraPosition`
- `MinecraftLastCameraPosition`

These names indicate camera-position data is part of renderer/shader interfaces. They do not establish the CPU-side object, exact units, origin rebasing, or when values are updated.

### 3. What this does not reveal
String presence alone does not give:
- CPU address or offset of any matrix.
- Matrix storage order or multiplication convention.
- Whether the matrix is row-major or column-major in CPU memory.
- Clip-space depth range, handedness, or viewport transform.
- The correct frame/view when split-screen, third-person, portal, or other render views are active.
- A supported mod callback or safe thread to read the values.

The GPU uniform names are not a safe substitute for a verified CPU-side matrix source.

## Updated conclusion
We now have static evidence for both sides of the intended projection path:
- ECS actor data includes AABB and render-position component types and systems.
- Renderer shader interfaces include current/inverse view-projection and model-view-projection uniforms.

However, the bridge between the ECS snapshot and the active render view is still unresolved. No projection code or runtime hook should be added based only on these strings.

## Next step
Trace the renderer code that populates `CurrentViewProjectionMatrixUniform` / `u_viewProj`, and determine which render-view object owns that data. Then establish whether a supported render callback can access the same view and a synchronized ECS snapshot. Until then, the diagnostic build should remain unchanged.
