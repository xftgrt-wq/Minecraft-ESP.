# Additional RTTI-resolved component lookups in a CameraAimAssist context builder

Date: 2026-11-10

## Exact target

ARM64 `libminecraftpe.so`, Build ID `2e318db12824cadb2618754ab7c82fa96fb30659`.

## Context-builder routine

At `0x9e80e38`, the routine receives a context-like object and queries multiple component storages from the object at `[x28] + 0x28`. It builds a local collection of type-erased storage wrappers and stores the resulting pointers in the context object.

Each lookup helper uses a 32-bit type key, allocates a `shared_ptr_emplace<entt::basic_storage<Component, EntityId>>` wrapper, and installs a vtable whose RTTI names the concrete component type.

## Resolved lookup keys

| Lookup helper | Type key | RTTI-resolved component |
|---|---:|---|
| `0x9e62d1c` | `0x15374807` | `CameraAimAssistComponent` |
| `0x9e69f44` | `0xdb55e891` | `CameraAimAssistCachedDataComponent` |
| `0x9e631d8` | `0xfa8cd650` | `CameraAimAssistTickComponent` |
| `0x9e63434` | `0xa20a51ff` | `CameraAimAssistCachedFrustumComponent` |
| `0x9e78c08` | `0xe642016b` | `SynchedActorDataComponent` |
| `0x9e702ec` | `0xc1627e35` | `CameraAimAssistEntityTargetComponent` |
| `0x873e0f8` | `0xacb47b38` | `LocalPlayerComponent` |

The mappings were established by following each helper's installed vtable to its RTTI object and reading the concrete `shared_ptr_emplace<entt::basic_storage<...>>` name. They are not inferred from the numeric keys alone.

A separate helper `0x9e80d60` resolves two more storages:
- `0x85b93800` -> `ActorOwnerComponent`;
- `0xe642016b` -> `SynchedActorDataComponent`.

## Interpretation

This confirms that the nearby CameraAimAssist code constructs typed EnTT storage views through a type-keyed registry/context. It also gives concrete type-key mappings for several CameraAimAssist components.

The context builder at `0x9e80e38` and the callback at `0x9e43f80` are in the same surrounding code path, but the exact one-to-one relationship between this particular wrapper collection and the callback's complete strict view is not yet proven. Do not assume these seven storages are the complete component set for the FetchValidEntityTarget system.

## ESP relevance

This is useful infrastructure evidence: the game has a type-keyed storage lookup path, and its returned wrappers are RTTI-identifiable. The next step is to find the corresponding storage-wrapper lookup for `AABBShapeComponent`, `HitboxComponent`, and `SubBBsComponent`, then trace how the callback reads their 0x20-byte or other component payloads.

No stable external registry API, safe runtime hook, entity snapshot lifetime, or render-thread transfer is established. Do not hardcode these type keys into the mod or call these helpers from an injected thread.
