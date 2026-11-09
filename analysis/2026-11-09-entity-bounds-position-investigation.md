# 2026-11-09 — Entity bounds and position component investigation

## Target
Uploaded `libminecraftpe.so` for Minecraft Bedrock Android 1.26.33, ELF64 AArch64.

## Confirmed strings and component types
The binary contains these exact engine component/type names:
- `AABBShapeComponent` at file offset `0x21aa4e8`
- `StateVectorComponent` at file offset `0x218a3ea`
- `ActorDataBoundingBoxComponent` at file offset `0x247a6c9` (pretty-function type string)
- `ActorDataBoundingBoxComponent` at file offset `0x24ca2e0`
- `MinecraftCamera::ActiveCameraComponent` at `0x23adf50`
- `MinecraftCamera::RenderCameraComponent` at `0x24143ea`
- `CurrentViewProjectionMatrixUniform` at `0x21e2297`

The binary also contains descriptive strings for actor bounding-box width and height, plus ECS system metadata showing that `AABBShapeComponent` and `StateVectorComponent` are read together in actor movement/collision systems. A scale-by-age system reads/writes `ActorDataBoundingBoxComponent`, confirming that actor bounds are updated as entity scale changes.

## What this establishes
The build has the underlying data concepts needed for a bounds-based ESP:
1. Actor world position/state is represented by `StateVectorComponent`.
2. Actor shape/bounds are represented by `AABBShapeComponent` and `ActorDataBoundingBoxComponent`.
3. The renderer has a current view-projection matrix uniform, and the camera has active/render camera components.

This is stronger evidence than assuming entity dimensions from fixed player/mob constants: the engine maintains actual per-entity bounds and position components.

## What is not yet established
- Exact in-memory layouts and offsets for these components.
- A safe way to enumerate live entity IDs and retrieve those components from the running client.
- The address/source of the active view-projection matrix at the point an overlay can use it.
- A verified render callback / hook for this exact 1.26.33 build.

String occurrences and ECS template metadata are not themselves runtime accessors or valid hook points. No functional ESP or installable library is produced by this investigation.

## Next required reverse-engineering work
Trace the ECS systems that consume `StateVectorComponent` + `AABBShapeComponent`, identify the concrete component storage access path and actor iteration mechanism, then trace the render pass that uploads `CurrentViewProjectionMatrixUniform`. Only after these are verified can the eight AABB corners be projected and drawn reliably.
