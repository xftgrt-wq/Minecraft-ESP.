# 2026-11-10 — ECS bounds writers and active camera component paths

## Input verification
- `libminecraftpe.so`, 349,243,744 bytes
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- ELF64 AArch64.

## New findings from embedded EnTT system signatures

### 1. Bounding-box update path
A strict ticking-system signature at binary string offset `0x2D20C00` describes a view filtered by:
- `PlayerComponent`
- `MinecartFlagComponent`
- `ShulkerFlagComponent`

The system reads no required components in the signature and writes:
- `AABBShapeComponent`
- `ActorDataBoundingBoxComponent`
- `ActorDataDirtyFlagsComponent`
- `DepenetrationComponent`
- `OffsetsComponent`

It also declares an add/remove operation for `ShouldUpdateBoundingBoxRequestComponent`.

This is direct evidence of a dedicated bounding-box update path for those selected actor categories. It does **not** prove this is the only bounds path, and it does not establish the component memory layout or the meaning of each field.

### 2. Dropped-item actors participate in AABB-aware systems
The liquid-physics system signature at `0x2C445C4` reads:
- `StateVectorComponent`
- `UpdateWaterStateRequestComponent`
- `AABBShapeComponent`
- `SubBBsComponent`
- optional `ItemActorFlagComponent`

Its strict-system interface confirms the same read set and a write set containing `StateVectorComponent` and `UpdateWaterStateRequestComponent`.

This supports the conclusion that item actors can be processed in systems that consume AABB shape data. It does not show that this liquid-physics system enumerates every dropped item or that it is suitable for ESP.

### 3. Camera entity components and camera systems
The binary contains EnTT component type names for:
- `MinecraftCamera::CameraComponent`
- `MinecraftCamera::ActiveCameraComponent`
- `MinecraftCamera::RenderCameraComponent`
- `MinecraftCamera::CurrentInputCameraComponent`
- `MinecraftCamera::DefaultInputCameraComponent`
- `MinecraftCamera::CameraBlendStateComponent`

Camera system signatures show:
- `CameraPortalDistortionSystem` queries `RenderCameraComponent`, reads `CameraBlendStateComponent`, and writes `CameraComponent`.
- `DebugCameraSystem` queries `RenderCameraComponent` and `GameCameraComponent`, reads `DebugCameraComponent`, and writes `CameraComponent`.
- `CameraAspectRatioSystem` writes `CameraComponent` and declares `CameraAPIComponent` as a global read.
- `CameraSetPositionSystem` reads `CameraAdjustedPositionComponent` and writes `CameraComponent`, `CameraTargetComponent`, and `TargetCameraOrientationComponent`.

This identifies a camera-component update path in the same ECS architecture. It does not yet reveal a view/projection matrix field, matrix convention, or a stable external access point.

### 4. Render-position system signature confirmed
The `UpdateRenderPosSystem` signature at `0x2D21839` includes a view that requires `InterpolateMovementNeededComponent` and reads `StateVectorComponent` while writing `RenderPositionComponent`. Its paired strict-system signature appears at `0x2D2191E`.

This is a concrete producer relationship for render position, but no layout or runtime registry pointer is known.

## Practical implications for the ESP investigation
- Player bounds have an identifiable ECS update system.
- Item actors are known to use AABB shape data in at least one ECS system.
- Camera state is represented by dedicated ECS camera components.
- Render positions are produced by a named ECS system.

## Still unresolved — do not guess
- Registry/context address and lifetime.
- Entity iteration API and how the ECS view is acquired.
- Exact component layouts, alignment, and ownership.
- Whether AABB coordinates are local, world, or dimension-relative.
- Camera matrix storage and whether the render camera or active camera is authoritative for the current frame.
- Render-thread synchronization and a safe mod callback.

No native hooks or guessed memory offsets were added. The existing build remains diagnostic-only and does not yet draw live ESP boxes.

## Next static-analysis target
Find the implementation/registration sites for the bounding-box writer and camera render systems, then recover their component access and data-flow contracts. In parallel, identify a renderer-owned view/projection source. Only once both sides are understood should a read-only frame snapshot bridge be considered.
