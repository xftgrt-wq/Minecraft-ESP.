# 2026-11-10 — CameraAimAssist strict-system RTTI registration cluster

## Exact binary

The analysis used the previously supplied `libminecraftpe.so`:
- ARM64 ELF64
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- GNU Build ID: `2e318db12824cadb2618754ab7c82fa96fb30659`

## New exact relocation cluster

The dynamic relocation table contains a compact metadata cluster at `0x140605a0–0x14060630`:

- `0x140605a0 -> 0x2991fba`: RTTI name for the `IStrictTickingSystem<StrictExecutionContext<...>>` specialization.
- `0x140605b8 -> 0x2991c86`: RTTI name for `StrictTickingSystemFunctionAdapter<&CameraAimAssistFetchValidEntityTargetSystem::tick, ...>`.
- `0x140605c0 -> 0x14060598`: base-type relationship in the RTTI object graph.
- Nearby entries point to additional RTTI/type metadata and code/data addresses, including `0x1463b890`, `0x875e6f4`, and `0x9e80b18`.

This proves the camera aim-assist adapter and its strict-system interface are represented in the binary's C++ RTTI metadata. It does not, by itself, prove that this cluster is a live scheduler registration table or reveal the runtime owner/context.

## Exact strict-system view contract recovered from the RTTI name

The adapter type string at `0x2991c86` encodes a `View<StrictEntityContext,...>` with:

- Excluded: `ActorRemovedFlagComponent`, `LocalPlayerComponent`.
- Optional reads: `HitboxComponent`, `AABBShapeComponent`, `SubBBsComponent`, `SynchedActorDataComponent`, actor-excluded status, boat/minecart/mob/player flags, and actor ownership.
- Required include: `CameraAimAssistComponent`.
- Additional required reads: `CameraAimAssistTickComponent`, cached frustum, cached position, and entity-target component.
- Optional global read: `CameraAimAssistActorPriorityClientComponent`.
- Entity modifier: `CameraAimAssistEntityTargetComponent`.

The paired strict-system interface type string at `0x2991fba` independently encodes:
- Filter: exclude removed actors and local player.
- Read set: hitbox, AABB shape, sub-BBs, synchronized actor data, actor-kind/ownership flags, camera aim-assist state, cached frustum/position, and reset-exclusion flag.
- Write set: `CameraAimAssistEntityTargetComponent`.
- Add/remove set: `CameraAimAssistEntityExcludedStatusComponent`.
- Global read: `CameraAimAssistActorPriorityClientComponent`.

This is a concrete, detailed example of how the engine declares AABB access through a typed strict-system interface. It is not a supported external query API; the view's filters and component requirements are specific to camera aim-assist behavior.

## Important negative result

Disassembly of nearby generic helper code shows repeated calls through the same process-global dispatch object previously examined. The cross-reference pattern is shared by unrelated subsystems. It remains unsuitable as an ECS context accessor.

The RTTI cluster also contains nearby generic storage and type metadata. None of these static records is a live storage instance or a proven world pointer.

## Updated conclusion and next target

We have now recovered a precise typed AABB-consuming strict-system contract and its RTTI inheritance metadata. The next useful step is to identify the constructor/factory that creates the concrete adapter object and the scheduler/context object that receives it. That requires tracing the data/code references around the RTTI cluster into constructor callsites and lifecycle initialization, not guessing from the metadata addresses.

Still unknown:
- concrete `StrictEntityContext` owner and runtime address;
- adapter instance construction and lifetime;
- a safe route to enumerate player/item entities;
- component memory layouts and synchronization requirements;
- render-thread-safe transfer into the existing AABB projection pipeline.

No hooks, offsets, or runtime accessors were added. Live ESP remains unimplemented; all game hooks stay disabled.
