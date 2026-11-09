# CurrentViewProjection source-object and ECS progress checkpoint — 2026-11-09

Target: uploaded Minecraft Bedrock Android ARM64 `libminecraftpe.so`.
SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`.

## What is actually established so far

1. The string `CurrentViewProjectionMatrixUniform` is registered as a descriptor at VA `0x149100a0`.
2. A call site passes that exact descriptor pointer to helper `0x1267ab74`.
3. At that call site, the helper's source pointer is `x21 + 0x208`.
4. The helper copies 64 bytes into the descriptor-keyed data entry and marks it updated.
5. This establishes a descriptor-to-source-field relationship at that call site, but does not yet establish the concrete type or lifetime of x21, nor prove that the field is stable across builds.

## Important limits

The following are not yet verified:
- constructor or vtable identity of the object in x21
- all writers to `x21+0x208`
- matrix convention (row/column major, handedness, clip-space depth, transpose)
- camera-relative/world-relative transform expectations
- a safe runtime interception point
- a usable ECS registry/entity iteration path
- field offsets for `StateVectorComponent`, `HitboxComponent`, or `AABBShapeComponent`

Do not treat `+0x208` as a global offset or as a ready-made hook. It is only a field offset relative to an object whose identity is still unresolved.

## ECS evidence recap

RTTI/adapter metadata confirms systems whose views include:
- `StateVectorComponent`
- `HitboxComponent`
- `AABBShapeComponent`
- `ActorDataBoundingBoxComponent`

The metadata describes component types and read/write access patterns. It does not expose their storage layout or offsets. In particular, `tickUpdateHitboxSystem` metadata shows a system reading state-vector data and writing hitbox data, while other movement systems include AABB and sub-box components.

## Practical implementation status

No implementation can yet safely enumerate live entities and project their bounds. A native library built now would be a loader stub only, not ESP. Do not replace the game's original `libminecraftpe.so` with such a stub.

## Remaining investigation gates

### Camera path
- Resolve x21's constructor and vtable from its incoming call chain.
- Find all writers to `x21+0x208` and identify whether they update once per frame.
- Confirm the 16-float matrix convention using its consumers and math operations.
- Find a renderer-owned call site that can observe the matrix without relying on UI/file-picker paths.

### Entity path
- Identify the runtime owner/registry for the ECS views.
- Find a safe iteration mechanism or supported Preloader/SDK API for this exact game build.
- Establish world position and bounding-box min/max from component accessors or validated layout.
- Distinguish collision AABB, hitbox, and render bounds; they are not interchangeable.

### Build path
- Exact-target SDK/Preloader compatibility for Bedrock 1.26.33 and LeviLauncher 1.5.13 remains unverified.
- Later 1.26.5x/1.26.6x offsets and signatures must not be copied.
- Only after camera and entity interfaces are validated should the render callback and toggle be implemented and tested.

## Next concrete work

Prioritize x21 identity and field writers, then inspect ECS registration/storage access. Keep all conclusions build-specific and label inferred findings separately from confirmed ones.
