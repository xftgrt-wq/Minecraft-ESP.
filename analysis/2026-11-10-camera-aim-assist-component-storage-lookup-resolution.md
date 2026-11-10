# Resolve two component-storage lookups in the CameraAimAssist strict context

Date: 2026-11-10

## Exact target and call chain

Target binary: ARM64 `libminecraftpe.so`, Build ID `2e318db12824cadb2618754ab7c82fa96fb30659`.

At `0x9e43f80`, the callback receives a stack-passed context object in argument 8 (`x24)) and a candidate range in argument 9 (`x22`). In the candidate path it calls helper `0x9e80d60` with:
- `x0 = [x24] + 0x28`;
- `x1 = x29 - 0x80`, a local destination object.

The helper at `0x9e80d60` requests two type-keyed storage wrappers from that context using constants:
- `0x85b93800` via `0x97dd950`;
- `0xe642016b` via `0x9e78c08`.

The two helper results are stored as a pair in the local destination object.

## Resolved component types

The lookup helpers allocate type-erased EnTT storage wrappers. Their installed vtables and RTTI resolve the component types:

1. `0x97dd950` installs vtable `0x14023b40`. Its RTTI chain identifies `std::shared_ptr_emplace<entt::basic_storage<ActorOwnerComponent, EntityId>>`. Therefore the lookup key `0x85b93800` resolves to the ActorOwnerComponent storage wrapper.
2. `0x9e78c08` installs vtable `0x14061310`. Its RTTI chain identifies `std::shared_ptr_emplace<entt::basic_storage<SynchedActorDataComponent, EntityId>>`. Therefore the lookup key `0xe642016b` resolves to the SynchedActorDataComponent storage wrapper.

This is not just a name guess: each helper installs a vtable whose RTTI names the concrete `entt::basic_storage` component type.

## What this proves about x24

The object at `[x24] + 0x28` supports type-keyed lookup of EnTT component storage. This strongly identifies the relevant field as a component-storage registry/context, and it confirms the callback is operating through typed entity-component storage rather than only anonymous geometry buffers.

The RTTI for the nearby `CameraAimAssistFetchValidEntityTargetSystem::tick` strict adapter lists both `ActorOwnerComponent` and `SynchedActorDataComponent` among its optional reads, matching the two resolved lookups. This is a strong consistency check for the callback/context interpretation. The exact C++ wrapper type of the stack argument x24 is still not fully recovered, so avoid treating its offsets as reusable outside this callback.

## ESP relevance and limits

This advances the entity-data route: the strict context can resolve concrete component storages by type key. It does **not** yet expose a general iteration API or prove the candidate list covers all players and dropped items. The candidate range remains camera-aim-assist-specific and may be filtered. AABBShapeComponent/HitboxComponent access and camera/render-thread transfer are still unproven.

No hook or runtime offset is authorized by this static finding. Keep the internal hooks disabled until the callback ABI, context lifetime, and render-thread synchronization are established.

## Next static target

Use the same vtable/RTTI resolution technique on the other component lookup helpers used by this callback, especially the AABBShapeComponent, HitboxComponent, SubBBsComponent, and PlayerComponent storages. Then determine whether the candidate record's packed handle can be joined to those stores within this strict context, and whether the context lifetime is safe for an overlay snapshot.
