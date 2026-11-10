# 2026-11-10 — Hitbox helper input-vector provenance

## Binary identity

The uploaded ARM64 Minecraft library was rechecked before analysis:
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- GNU Build ID: `2e318db12824cadb2618754ab7c82fa96fb30659`

## Caller-side trace

At `0x9e3cd60`, the caller invokes a virtual method at byte offset `+0x368` on object `x25`. It then adds `0x48` to the returned pointer and calls `0x10d67694` with the current entity key at `x22+0x50`.

The helper at `0x10d67694` performs a paged, generation-aware lookup using that key and returns a pointer from the selected storage slot. The caller then reads:
- a 64-bit pair from the returned object at offset `+0xe8`;
- a third float at offset `+0xf0`.

Those three float values are assembled into the input vector passed as `x2` to Hitbox helper `0xb58ecd0`.

The same caller passes `x22+0x28` as the HitboxComponent entry and a zero-initialized pair of output vectors. The returned entity key is subsequently checked against another paged storage before use.

## What is established

- The helper input vector is not an arbitrary stack value: it is read from a per-entity storage object returned by a generation-aware lookup.
- The lookup is reached through an object returned by the caller's virtual method at `+0x368`, then a subobject at `+0x48`.
- The three-float vector is distinct from the two output vectors produced by the Hitbox helper.
- The Hitbox helper's fallback branch adds this entity's two HitboxComponent vectors to two context-provided vectors; its resolved-object branch copies context-provided values directly.

## What is not established

- The concrete component type stored at the returned storage slot. The lookup helper itself is generic, and this callsite does not expose a readable component-name string or explicit key constant.
- Whether offsets `+0xe8/+0xf0` are a world position, interpolated position, render origin, or another position-like value.
- Whether the helper's two output vectors are always world-space min/max bounds, including the resolved-object branch.
- Whether the caller's entity set includes every player and dropped item.

## Safety / integration consequence

This narrows the data path to an entity-keyed storage lookup, but it does not yet provide a stable public API, owner lifetime, render-thread guarantee, or validated hook ABI. No private hook or raw game-memory dereference was added.

## Next static target

Identify the storage type behind the `x25 + vtable[0x368] + 0x48` path and find other consumers of its `+0xe8/+0xf0` fields. Cross-check those consumers against known position/interpolation structures before assigning world-space semantics.

## Runtime status

The installed module remains the HUD/Mod Menu diagnostic prototype. It does not yet have a live entity feed and does not draw verified 3D ESP boxes.
