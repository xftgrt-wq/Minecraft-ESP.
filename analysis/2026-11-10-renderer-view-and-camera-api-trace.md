# 2026-11-10 — renderer entry-point and camera API trace

## Input verification
- `/mnt/data/libminecraftpe.so`
- Size: 349,243,744 bytes
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- ELF64 little-endian AArch64.

## New renderer-side evidence

### 1. Actor rendering has a named camera-owned entry point
The binary contains the implementation signature:

```cpp
virtual void LevelRendererCamera::render(
    BaseActorRenderContext &,
    const ViewRenderObject &,
    IClientInstance &
)
```

The source path embedded alongside it is:
`handheld/src-client/common/client/renderer/game/LevelRendererCamera.cpp`.

The method creates multiple command-list tasks, represented by compiler-generated closure names. This indicates actor/world rendering is scheduled through a renderer-owned camera pass and command-list work, rather than being a simple synchronous actor loop.

### 2. View data is threaded through the render pipeline
The renderer symbols also reference:
- `ViewRenderObject`
- `ViewRenderData`
- `ViewSetId`
- `RenderChunkObjectCollection`
- `CommandListQueue`

This establishes a view-oriented render pipeline. It is a promising area to trace for camera matrices, but the presence of these names does not identify the matrix member, its layout, or whether it is directly accessible to a mod.

### 3. Camera API component has concrete accessors
The binary contains both accessors:
- `const ICameraAPI &CameraAPIComponent::get() const`
- `ICameraAPI &CameraAPIComponent::get()`

The ECS camera systems declare `CameraAPIComponent` as an optional global read or global write. This is stronger than a mere type-name occurrence: camera systems are wired to a global camera API component. However, the vtable contract and concrete API methods still need to be recovered before any use can be considered safe.

### 4. Camera/frustum evidence remains limited
A `CameraAimAssistCachedFrustumComponent` and `CameraAimAssistCacheFrustumSystem` exist, but this is an aim-assist cache, not proven to be the renderer's general visibility frustum. Do not treat it as the camera's authoritative view/projection state.

## Updated integration picture
- Actor data: ECS actor components include AABB and render-position types.
- Camera state: ECS camera components and a global `CameraAPIComponent`.
- Renderer: `LevelRendererCamera::render` receives `ViewRenderObject` and submits command-list tasks.

These are three separate verified subsystems. Their safe bridge and frame synchronization are still unknown.

## Explicitly not established
- Exact `ViewRenderObject` or `ViewRenderData` field layout.
- View or projection matrix address, multiplication order, handedness, clip-space convention, or viewport mapping.
- A supported render callback for the mod.
- Registry address, entity iteration, or component storage offsets.
- Whether camera API access is legal from the mod's current callback/thread.

No hooks, pointer offsets, or native-library changes were made. Build #170 remains a menu/HUD diagnostic build only; it does not draw confirmed live ESP boxes.

## Next analysis step
Inspect the demangled camera API accessor and renderer method call graph to identify matrix-producing/consuming functions and the exact view-data handoff. Separately trace ECS context acquisition. Do not implement a projection path until its matrix convention and thread/lifetime rules are established.
