# 2026-11-09 — ECS movement and hitbox system trace (Bedrock 1.26.33 ARM64)

## Target verification

Reopened `/mnt/data/libminecraftpe.so` and confirmed:
- ELF64, little-endian, AArch64 shared object
- Size: 349,243,744 bytes
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`

This matches the binary examined in the previous reports.

## New concrete ECS evidence

The binary contains compiled `StrictTickingSystemFunctionAdapter` type/signature strings that expose the exact component access sets of several systems. These are stronger evidence than isolated component-name strings, although they are still type/signature metadata rather than callable function addresses.

### Hitbox update path

At rodata VA `0x32d9ad8`, the adapter signature identifies:

`VanillaSystems::ActorMoveSystemImpl::tickUpdateHitboxSystem`

Its associated strict-system view includes:
- `MoveRequestComponent` as an included/filter component
- `StateVectorComponent` as a read component
- `HitboxComponent` as a write component

The corresponding strict-system descriptor string at `0x32d9be3` independently lists `StateVectorComponent` under Read and `HitboxComponent` under Write.

This establishes a real engine dataflow relationship: the hitbox component is updated from the actor state-vector component by the movement system. It does **not** establish the in-memory layout of either component or the address of the tick function.

### AABB movement path

At rodata VA `0x2c752bd`, another adapter signature identifies:

`ActorMoveSystemImpl::doActorMoveSystem`

Its function signature consumes `MoveRequestComponent`, `AABBShapeComponent`, and `SubBBsComponent`. This confirms that AABB shape data participates in the actor movement/collision path.

A separate `NoClipOrNoBlockMoveFilterSystem` adapter signature (rodata VA `0x32da636`) shows `AABBShapeComponent` and `StateVectorComponent` in the same movement-system view, with the system writing updated AABB/state-vector data.

### Actor-data bounding-box path

A `ScaleByAgeSystem::_tick` adapter signature at rodata VA `0x292ecbe` reads `AgeableComponent` and `ActorOwnerComponent`, and writes `ActorDataBoundingBoxComponent`, `ActorDataDirtyFlagsComponent`, and `ScaleByAgeComponent`; it can add `ShouldUpdateBoundingBoxRequestComponent`.

This confirms a second bounding-box-related component path, distinct from the movement collision AABB path.

### Independent use of hitbox/AABB data

The `CameraAimAssistFetchValidEntityTargetSystem::tick` adapter signature includes optional `HitboxComponent`, `AABBShapeComponent`, and `SubBBsComponent` reads. This is evidence that these geometry components are also consumed by an entity-targeting system, but it does not provide their layout.

## Camera matrix investigation

The following shader-uniform names exist in the binary:
- `CurrentViewProjectionMatrixUniform` at rodata VA `0x21e2297`
- `PreviousViewProjectionMatrixUniform` at rodata VA `0x246cc8b)
- `MinecraftCameraPosition` at rodata VA `0x24b231d`
- `MinecraftLastCameraPosition` at rodata VA `0x248e9cf`

The current/previous view-projection names are referenced in a guarded static-initialization sequence around `0x1290cec8–0x1290cf70`; camera-position uniform names are similarly referenced in guarded initialization around `0x126dc838–0x126dc9b8`.

The static globals associated with these names have references in the uniform-registration/initialization path. The inspected instructions construct/register uniform descriptors; they do not expose the live matrix values or a verified matrix-upload callback. Therefore these names are useful leads for tracing the renderer's uniform binding, not direct matrix pointers.

## What is now established

1. The target uses an EnTT-style ECS with strict system views/adapters.
2. Actor position state is represented in `StateVectorComponent`.
3. The movement system updates `HitboxComponent` from `StateVectorComponent`.
4. `AABBShapeComponent` is used in actor movement/collision.
5. `ActorDataBoundingBoxComponent` is updated by a scale-by-age system and is a separate data path.
6. The renderer registers current/previous view-projection and camera-position uniforms.

## Still not established

- Runtime addresses or byte layouts of `StateVectorComponent`, `HitboxComponent`, `AABBShapeComponent`, or `ActorDataBoundingBoxComponent`
- The concrete function address for `tickUpdateHitboxSystem` or a callable entity iteration API
- A verified render callback or the live current view-projection matrix source/upload point

No offsets or hook addresses should be inferred from the type-signature strings. The next reverse-engineering step is to trace the registered strict-system adapter's function pointer and the EnTT storage access path, then trace the renderer uniform descriptor into the actual per-frame matrix upload.
