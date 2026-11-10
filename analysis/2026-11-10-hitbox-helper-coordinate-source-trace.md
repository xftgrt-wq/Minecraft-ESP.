# 2026-11-10 — Hitbox bounds helper coordinate-source trace

## Exact target

Uploaded Android ARM64 `libminecraftpe.so`:
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- GNU Build ID: `2e318db12824cadb2618754ab7c82fa96fb30659`

## Callsite argument provenance

At callsite `0x9e3cdd8`, the caller prepares a three-float vector at `sp+0x30`:
- a 64-bit pair is loaded from `[x0+0xe8]` at `0x9e3cd80`;
- the third float is loaded from `[x0+0xf0]`;
- those values are stored as a 3-float vector at `sp+0x30`.

The call is then made with:
- `x0 = x19`: context/service object;
- `x1 = x22 + 0x28`: pointer into the current HitboxComponent entry;
- `x2 = sp+0x30`: the three-float input vector just described;
- `x3 = sp+0x40`: zero-initialized output structure.

The return value is retained as an entity key and validated against a paged entity storage immediately afterward. This ties the helper result to the current entity rather than to an unrelated global bounds object.

## Helper behavior relevant to coordinate space

At `0xb58ecd0`, the helper:
1. Saves the context, Hitbox entry pointer, input vector pointer, and two output pointers.
2. Calls a context virtual method at vtable byte offset `+0xd8`, passing a temporary result structure.
3. Validates the returned handle/flags and selects one of two paths.
4. On the resolved-object path, obtains two 3D values through context virtual methods at `+0xf0` and `+0xf8`, then copies them to the two output vectors.
5. On the fallback/additive path, reads two 3D vectors from the Hitbox entry at offsets `+0x00` and `+0x0c`. It obtains corresponding 3D values from the context's `+0xf0` and `+0xf8` methods and adds each pair component-wise before writing the two outputs.

The explicit arithmetic in the fallback path is:
- output A = context value A + Hitbox vector A;
- output B = context value B + Hitbox vector B.

This is consistent with local min/max bounds being translated by a position/origin pair, or with two related coordinate-frame values being composed. It is not evidence of rotation or scale being applied.

## What this narrows down

- The Hitbox helper returns two 3D outputs associated with a validated entity key.
- The fallback path explicitly adds two component vectors to two context-provided vectors.
- The outputs are more promising for world-space AABB construction than the raw SubBBs six-float records, because a transform/position-like context contribution is applied in this path.
- The helper's resolved-object path copies values from context accessors instead of performing the local-vector addition.

## What remains unproven

- The exact semantic identity of the context virtual methods at `+0xd8`, `+0xf0`, and `+0xf8`.
- Whether the two output vectors are guaranteed to be min and max in world space in every branch.
- Whether orientation/scale is encoded upstream or whether the bounds are axis-aligned by construction.
- Whether this helper's entity population covers every player and dropped item.
- The safe owner, lifetime, ABI, and render-thread synchronization needed for an external mod callback.

The caller's input vector from offsets `+0xe8/+0xf0` is a distinct argument; do not conflate it with the two output vectors or assume those raw object offsets are stable API fields.

## Next target

Resolve the context virtual methods' concrete implementations and compare their returned vectors with the caller's position-like input. Then inspect other direct callers to determine whether this helper is shared across actor classes and whether its outputs are consistently used as bounds.

## Runtime status

No hook, patch, or game-memory dereference was added. The installed module remains a HUD/Mod Menu prototype; it still has no live entity feed and does not draw live 3D ESP boxes.
