# Resolve the bounds-transform helper used by HitboxComponent processing

Date: 2026-11-10

## Exact target

Uploaded ARM64 `libminecraftpe.so`, GNU Build ID `2e318db12824cadb2618754ab7c82fa96fb30659`.

## Callsite and argument roles

At `0x9e3cdd8`, the HitboxComponent consumer calls `0xb58ecd0` with:

- `x0 = x19`: context/service object retained by the caller;
- `x1 = x22 + 0x28`: pointer to a field inside the current 0x90-byte HitboxComponent entry;
- `x2 = sp + 0x30`: first 3D vector input, populated from a geometry query or fallback;
- `x3 = sp + 0x40`: output structure, zero-initialized before the call.

After the call:
- `x0` is saved as `x28` and treated as a packed entity key by the caller's EnTT sparse/page lookup;
- the output structure at `sp + 0x40` is destroyed after the caller consumes it.

This establishes that the helper returns an entity-associated key while also producing a 3D output structure.

## Helper behavior at 0xb58ecd0

The helper saves:
- `x20 = x0` (context/service object);
- `x22 = x1` (entry field pointer);
- `x21 = x2` (first output vector);
- `x19 = x3` (second output vector).

It first calls a virtual method at vtable byte offset `+0xd8` on the context object, passing a local result object. It then validates flags and a returned pointer/handle. Depending on the result flags, it calls additional context virtual methods at byte offsets `+0x1c0`, `+0x1c8`, `+0xf0`, and `+0xf8`.

Two paths are visible:

1. **Resolved-object path:** the helper copies two 3-component values from the returned object into the output vectors.
2. **Fallback/additive path:** it reads two packed 3D values from `x22` at offsets `+0x00` and `+0x0c`, reads a 3D value from the resolved object, adds the corresponding components, and writes the sums to the two output vectors.

The vector arithmetic is explicit:
- two floats are added with `fadd v0.2s`;
- the third component is added with scalar `fadd s1`;
- the resulting 3-component vectors are written to the two output destinations.

This is consistent with resolving a pair of 3D bounds vectors, with a fallback that translates local min/max-like vectors by a resolved origin. The exact semantic names and coordinate space are not proven from this helper alone.

## Cross-reference validation

The helper has 13 direct BL callsites in the executable segment, including:
- `0x9e3cdd8`: the current HitboxComponent consumer;
- `0xb58e218`, `0xb58eb5c`, `0xb594fcc`, `0xb651394`, `0xb65d5ac`, `0xb767904`, and several `0xbd9...` sites.

Other callers also pass a context object, a pointer to a geometry/record field, and two output structures, then use the returned x0 value in entity-key/index validation. This supports the interpretation that the helper is a shared context-based geometry/entity resolution routine, not a one-off routine for the current system.

## What this establishes for ESP

This is the strongest current evidence that the HitboxComponent path can produce entity-associated 3D bounds data:
- it obtains a packed entity key;
- it writes two 3D vectors;
- the fallback path explicitly adds local vector data to a resolved 3D origin;
- the caller validates the returned key against an EnTT-like entity storage path.

It is reasonable to investigate these outputs as candidate min/max bounds, but they must not yet be treated as confirmed world-space AABBs. The exact vector convention, axis order, coordinate space, and relationship to the AABBShapeComponent remain to be verified.

## Next target

Inspect the producer of the 0x90-byte HitboxComponent entries and the context virtual methods used by `0xb58ecd0`. Confirm whether the two output vectors are min/max bounds and whether the returned entity key is the owner entity or a related entity. Then compare the output with the AABBShapeComponent data path.

No runtime hook is justified yet: context ABI, object lifetime, and render-thread synchronization remain unverified.
