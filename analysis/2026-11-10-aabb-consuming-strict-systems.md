# 2026-11-10 — AABB-consuming strict ECS systems identified

## New result from the exact target binary

A broader scan of null-terminated RTTI/type-name strings found multiple strict-system descriptors that explicitly declare `AABBShapeComponent` as a read component. This is stronger than the earlier isolated hitbox-update metadata: it shows that the game has registered systems whose strict views include AABB data.

Notable examples:

### 1. Camera aim-assist target selection

Adapter type:
`CameraAimAssistFetchValidEntityTargetSystem::tick`

Its strict view metadata includes:
- Excludes `ActorRemovedFlagComponent` and `LocalPlayerComponent`
- Optional reads for `HitboxComponent`, `AABBShapeComponent`, `SubBBsComponent`, `SynchedActorDataComponent`, and actor-kind/ownership flags
- Includes `CameraAimAssistComponent`, `CameraAimAssistTickComponent`, cached-frustum and cached-position components
- Writes `CameraAimAssistEntityTargetComponent`

The paired `IStrictTickingSystem` descriptor independently lists `AABBShapeComponent`, `SubBBsComponent`, and `HitboxComponent` in its read set.

This is a useful proof that AABB data participates in a real entity-targeting system, but it is a camera aim-assist feature, not a public ESP API. Its optional components and filter semantics must be respected; it does not justify iterating storage directly.

### 2. Movement and collision systems

Additional descriptors found:
- `SneakMovementSystem::tickSneakMovementSystem`: reads `AABBShapeComponent`, movement flags, and step-height data; writes movement request and state-vector components.
- `NoClipOrNoBlockMoveFilterSystem`: its adapter signature consumes `AABBShapeComponent`, `StateVectorComponent`, and `SubBBsComponent`.
- `GetCollisionShapeEntityProxy`: a strict execution-context proxy whose read set includes `AABBShapeComponent` and movement-related components.
- Several travel/jump systems also declare AABB reads.

### 3. Bounding-box update system

A separate descriptor writes both:
- `AABBShapeComponent`
- `ActorDataBoundingBoxComponent`

It also updates offsets, depenetration state, and bounding-box update requests. This is a distinct system from the earlier hitbox update system, which writes `HitboxComponent`.

## Why this matters

The binary has at least three distinct bounding-related component roles:
- `HitboxComponent`: written by the actor hitbox update system.
- `AABBShapeComponent`: consumed by many movement, collision, targeting, and camera-related systems.
- `ActorDataBoundingBoxComponent`: written by a bounding-box update system.

They must not be treated as interchangeable layouts.

The most promising next static target is now the adapter/descriptor registration for `CameraAimAssistFetchValidEntityTargetSystem` and the bounding-box update system. These may reveal the strict context construction and component-view machinery more directly than the generic global dispatch object examined in the previous pass.

## Limits

Still not established:
- A stable live `StrictEntityContext` or world-owned registry pointer.
- Runtime component layouts and storage object addresses.
- Safe access timing/lifetime from the mod's render callback.
- A supported entity iteration API in the pinned Preloader SDK.

No hooks were enabled and no runtime offsets were added. The shared global dispatch object at `0x1473fa48` remains rejected as an entity accessor.
