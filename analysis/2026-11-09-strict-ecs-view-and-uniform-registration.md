# 2026-11-09 — Follow-up: strict ECS views and uniform registration

## Exact movement-system signatures

A second pass over the target's printable template-signature strings found the following exact descriptors:

| RoData VA | System / adapter | Component access exposed by the signature |
|---|---|---|
| `0x32d9ad8` | `VanillaSystems::ActorMoveSystemImpl::tickUpdateHitboxSystem` | Includes `MoveRequestComponent`; reads `StateVectorComponent`; writes `HitboxComponent` |
| `0x32d9be3` | `IStrictTickingSystem` descriptor for the same update path | Explicit `Read<StateVectorComponent>` and `Write<HitboxComponent>` |
| `0x32da7f2` | `IStrictTickingSystem` descriptor for `NoClipOrNoBlockMoveFilterSystem` | Reads `OffsetsComponent` and `ActorDataFlagComponent`; writes `AABBShapeComponent`, `StateVectorComponent`, and `SubBBsComponent`; may add `MoveRequestComponent` |
| `0x2c752bd` | `ActorMoveSystemImpl::doActorMoveSystem` adapter | Receives `MoveRequestComponent`, `AABBShapeComponent`, and `SubBBsComponent` |
| `0x292ecbe` | `ScaleByAgeSystem::_tick` adapter | Reads `AgeableComponent`; writes `ActorDataBoundingBoxComponent`, dirty flags, and scale data; may add a bounding-box update request |

The component storage type strings also identify EnTT storage specializations for `StateVectorComponent`, `AABBShapeComponent`, `HitboxComponent`, and `ActorDataBoundingBoxComponent`. These are type/storage identities, not storage object addresses or field offsets.

## Interpretation

This gives a more precise split than treating all box-like components as interchangeable:

- `StateVectorComponent` is the movement state input.
- `HitboxComponent` is refreshed by the dedicated hitbox-update system and is also read by camera aim-assist target selection.
- `AABBShapeComponent` participates in movement/collision and is written by the no-clip/no-block movement filter.
- `ActorDataBoundingBoxComponent` is a separate actor-data component updated by scale-by-age logic.

Thus the strongest geometry lead is now the pair `StateVectorComponent` + `HitboxComponent` for entity hitbox dimensions, with `AABBShapeComponent` as the movement/collision box path. The signatures do not prove that these components share the same coordinate origin or dimensions.

## Matrix and camera uniform references

The code references to `MinecraftCameraPosition`, `MinecraftLastCameraPosition`, `CurrentViewProjectionMatrixUniform`, and `PreviousViewProjectionMatrixUniform` occur in guarded static uniform-descriptor initialization. The camera-position descriptor globals are checked/initialized in the renderer-related initialization path; the view-projection descriptors are registered in a parallel path.

The inspected direct references terminate in guard checks, descriptor construction, and registration helpers. No direct per-frame matrix data pointer or upload callback has yet been identified.

## Next trace targets

1. Trace the registered `tickUpdateHitboxSystem` adapter through its runtime registration record to the concrete function pointer.
2. Trace the EnTT storage instance for `HitboxComponent` and `StateVectorComponent` to establish actual component access and layout.
3. Trace the uniform descriptor registration result into the render command/material binding path to find where the current view-projection matrix is populated.

No guessed offsets or hook addresses are supplied. The type-signature metadata is now concrete enough to narrow the next disassembly pass, but it is not itself a callable accessor.
