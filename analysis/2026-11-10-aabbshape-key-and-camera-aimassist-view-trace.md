# 2026-11-10 — AABBShapeComponent key and CameraAimAssist filtered-view trace

## Exact target

The original uploaded Android ARM64 `libminecraftpe.so` was re-materialized and rechecked:
- Size: 349,243,744 bytes
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- GNU Build ID: `2e318db12824cadb2618754ab7c82fa96fb30659`

All addresses below are virtual addresses in this exact ELF.

## New finding: exact AABBShapeComponent type key

The engine uses 32-bit FNV-1a hashes for the component keys examined in this path. This is cross-checked against known names and constants:
- `SubBBsComponent` -> `0x0b0822d1`
- `SynchedActorDataComponent` -> `0xe642016b`
- `ActorOwnerComponent` -> `0x85b93800`
- `ActorRemovedFlagComponent` -> `0x4434e84f`
- `LocalPlayerComponent` -> `0xacb47b38`

The same hash of the exact RTTI name `AABBShapeComponent` is `0xbac1b3cf`. That key is materialized and passed to the context storage lookup at `0x9e7ad9c–0x9e7ada8`.

This fills the previous gap: the AABBShapeComponent storage key is now identified from the exact target binary, not guessed from a similarly named component.

## Filtered-view construction

At `0x9e7ad60`, the function obtains the context pointer from `[x19]`, then performs storage lookups through the context at `x26 + 0x28`:

| Callsite | Key | Resolved component storage |
|---|---:|---|
| `0x9e7ada8` | `0xbac1b3cf` | AABBShapeComponent |
| `0x9e7adbc` | `0x0b0822d1` | SubBBsComponent |
| `0x9e7add0` | `0xe642016b` | SynchedActorDataComponent |
| `0x9e7ade4` | `0x85b93800` | ActorOwnerComponent |
| `0x9e7adf8` | `0x4434e84f` | ActorRemovedFlagComponent |
| `0x9e7ae0c` | `0xacb47b38` | LocalPlayerComponent |

The function compares the available storage counts and writes the storage pointers plus a selected iteration count into a view-like descriptor. The caller at `0x9e7f0e0` invokes this builder and then enters a filtered iteration path. The adjacent caller at `0x9e7f9b4` builds a second related view.

The iteration code performs paged storage lookups and generation checks on packed entity keys. This is consistent with a typed, filtered entity view, not a flat pointer array.

## CameraAimAssist association

The same code region contains the FNV-1a key `0x4c68a08d` for `CameraAimAssistResetEntityExcludedStatusFlagComponent`. The helper at `0x9e7a814` performs a context storage lookup using that key and resolves the entity-indexed component. The component identity matches the CameraAimAssist strict-system contract recovered from RTTI at `0x2991c86` / `0x2991fba`.

This materially strengthens the association between the filtered-view path and the CameraAimAssist system. It is still a private engine path with system-specific filters and state, not a public unrestricted iterator.

## What is established

1. The exact AABBShapeComponent key is `0xbac1b3cf`.
2. A context-based path at `0x9e7ad60` resolves AABBShapeComponent and SubBBsComponent storage instances alongside actor ownership/removal/local-player filtering components.
3. A nearby helper resolves CameraAimAssist-specific state through the same general context/storage pattern.
4. The caller builds and iterates a filtered view using paged, generation-checked entity storage.

## What is not established

- The value layout of the 0x20-byte AABBShapeComponent slot.
- Which fields represent local bounds versus world-space bounds.
- Whether the filtered view includes every relevant player, mob, or dropped-item entity.
- A stable way for a mod to obtain this private context.
- The callback ABI, context lifetime, render-thread synchronization, or safe handoff to the HUD overlay.

The storage vtable's previously identified 0x20-byte slot stride remains a storage fact only; it does not by itself describe the component's semantic fields.

## Next target

Trace the AABBShapeComponent slot reads in the CameraAimAssist filtered iteration and compare them with the SubBBs/Hitbox bounds path. In particular, establish whether the AABB slot contains two 3D vectors or a shape/transform reference, and determine the exact coordinate space before considering projection.

Do not install a hook or dereference these private objects from the mod yet. No ABI or lifetime is validated. The runtime module remains HUD-only; no live AABB feed or in-game 3D boxes have been implemented.
