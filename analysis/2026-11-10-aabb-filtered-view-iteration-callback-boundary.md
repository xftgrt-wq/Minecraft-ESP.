# 2026-11-10 — AABB filtered-view iteration and callback boundary

## Binary identity

Exact uploaded Android ARM64 `libminecraftpe.so`:
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- GNU Build ID: `2e318db12824cadb2618754ab7c82fa96fb30659`
- Size: 349,243,744 bytes

## View builder output layout

The builder at `0x9e7ad60` is called at `0x9e7f0e0` with an output descriptor at `sp+0x140`. From the stores in the builder, the descriptor contains six storage pointers at offsets `+0x08` through `+0x30`, plus a metadata pointer/count pair at `+0x38` and a state word at `+0x48`.

The six storage pointers correspond to the keys previously established:
- AABBShapeComponent (`0xbac1b3cf`)
- SubBBsComponent (`0x0b0822d1`)
- SynchedActorDataComponent (`0xe642016b`)
- ActorOwnerComponent (`0x85b93800`)
- ActorRemovedFlagComponent (`0x4434e84f`)
- LocalPlayerComponent (`0xacb47b38`)

The builder compares storage population sizes and records the smallest iteration population / associated storage choice. This is a common filtered-view optimization: iterate the smallest storage and validate entity membership in the other component storages. It does not mean the view enumerates every entity in the world.

## Iteration and callback boundary

The caller checks the selected storage pointer and iterates packed entity keys. The key is validated through sparse/page maps and generation bits before component slots are addressed.

For the selected entity, the code derives component slot addresses using the per-storage strides visible in the loop (including 0x20-byte AABB storage slots, 0x18-byte SubBBs slots, and other component-specific strides). It then calls the helper at `0x9e7c91c`.

The helper builds a temporary callback argument/context from the view and entity's component references, obtains several additional components by type key from the strict context, and invokes a callable through an indirect function pointer. This is a visitor/callback boundary for the filtered view. The function pointer is supplied by the caller's adapter object; it is not a public API and its concrete callback target must be resolved separately.

## What this proves

1. The AABB storage pointer participates in a private, filtered, multi-component entity view.
2. The view is driven by the smallest participating component storage and validates entity membership in the remaining storages.
3. Per-entity component slot addresses are formed before a callback/visitor is invoked.
4. The callback receives a temporary per-entity context assembled from component references and additional strict-context lookups.

## What it does not prove

- The 0x20-byte AABBShapeComponent value's field layout.
- That the AABB data is world-space or directly usable as min/max coordinates.
- That all players and dropped items satisfy this CameraAimAssist-oriented view's component filters.
- The concrete callback target's semantics or ABI.
- A stable context owner, safe lifetime, or render-thread callback.
- A safe external way to call this private view builder or iterator.

In particular, the 0x20-byte slot stride must not be mistaken for a semantic component structure. No field offsets are inferred from stride alone.

## Next static target

Resolve the callable object passed into the `0x9e7c91c` visitor path, then trace the AABB slot reads in that callback. Compare the exact AABB fields with the bounds produced by the HitboxComponent helper at `0xb58ecd0`. Only when both paths establish the same coordinate convention should the AABB-to-HUD bridge be considered for a live feed.

## Runtime status

No code hook or game-memory dereference was added. The installed module remains a HUD/Mod Menu prototype and still reports `WAITING FOR GAME DATA`. Live 3D player/item boxes are not implemented.
