# 2026-11-10 — Bedrock ECS component and render-position discovery

## Input verification
- File: `/mnt/data/libminecraftpe.so`
- Size: 349,243,744 bytes
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- ELF64, little-endian, AArch64.

## Discovery method
Inspected the dynamic symbol table and embedded strings / compiler-generated type signatures. The game implementation's Actor/Player methods are not exposed as usable dynamic symbols in this library; many internal implementation names are present only in strings emitted from templates and diagnostic type names. Therefore, these names are evidence of internal ECS types and system signatures, not directly callable exported APIs.

## Confirmed internal ECS vocabulary
The binary contains EnTT ECS storage/type signatures and diagnostic names for:
- `ActorComponent`
- `PlayerComponent`
- `ItemActorFlagComponent`
- `AABBShapeComponent`
- `ActorDataBoundingBoxComponent`
- `StateVectorComponent`
- `RenderPositionComponent`
- `RenderRotationComponent`
- `LocalSpatialEntityFetcherFactoryComponent`
- `GlobalActorRenderComponent`

This is a meaningful lead: the client build uses an ECS-oriented actor representation, rather than exposing the old-style Actor/Level API as exported dynamic symbols.

## Useful system signatures found in embedded type strings
1. `UpdateRenderPosSystem` is present with a strict ticking-system signature that reads `StateVectorComponent` and writes `RenderPositionComponent`. This establishes a named internal update path for render position, but not the layout or a safe way to access it externally.
2. A collision/movement system signature includes `PlayerComponent` and reads `AABBShapeComponent` plus `ActorOwnerComponent`. It also references a local spatial entity fetcher and block-collision evaluation queue. This confirms that player entities and AABB data participate in the same ECS query path.
3. Several collision/movement system signatures read `AABBShapeComponent`, `DimensionTypeComponent`, `StateVectorComponent`, and movement flags.
4. A liquid-physics system signature reads `AABBShapeComponent` and `ItemActorFlagComponent` as an optional component, confirming that dropped-item actors are represented in the same actor ECS family and can carry AABB shape data.
5. `ActorNormalTick` signatures explicitly exclude multiple actor categories including `ItemActorFlagComponent`; this is further evidence that item actors are distinguished by ECS flags.

## What this does and does not prove
**Proven from the binary's embedded signatures:** ECS actor/player/item component types exist; AABB data is consumed by gameplay systems; render position is maintained by a named system; item actors are represented in the actor ECS.

**Not yet proven:** ECS registry address, entity iteration entry point, component storage offsets, component object layouts, AABB coordinate convention, camera view/projection matrices, render-thread synchronization, or a safe public callback for reading these values.

The presence of `AABBShapeComponent` and `RenderPositionComponent` is not enough to safely dereference guessed offsets or install a hook. No runtime hook was added.

## Next reverse-engineering step
Trace the registration/dispatch path for `UpdateRenderPosSystem` and the client-side actor render system, then identify a verified registry/view access point and the concrete component layouts. Separately identify the camera matrix source and its coordinate convention. Only after those contracts are proven should the ESP data bridge be implemented.
