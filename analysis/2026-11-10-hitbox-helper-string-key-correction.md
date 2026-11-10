# 2026-11-10 — Critical correction: Hitbox helper lookup uses a string key

## Binary identity

Uploaded ARM64 Minecraft library:
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- GNU Build ID: `2e318db12824cadb2618754ab7c82fa96fb30659`

## Re-examining the call

At `0x9e3cd74–0x9e3cd78`, the caller passes:
- `x0 = result of virtual call through x25 vtable slot +0x368, plus 0x48`;
- `x1 = x22 + 0x50`.

The callee `0x10d67694` is not a generic entity-ID lookup. Its key handling strongly matches a C++ string-like key:
- It reads the first 8 bytes from `[x1]`.
- It reads a compact size/representation byte at `[x1+8]`.
- It distinguishes inline/small-string storage from heap-backed string storage using the low representation bit.
- It compares candidate key lengths and then compares key bytes, using `memcmp` for the longer/general case.
- It traverses hash buckets and collision chains, comparing stored key strings.

This is characteristic of a hash map keyed by string (or a string-compatible key wrapper), not a map directly keyed by the entity handle.

## Correction to earlier reports

The earlier description that `x22+0x50` supplies an entity key was incorrect. The argument is string-shaped. Consequently:
- the lookup does not establish a generation-aware entity lookup;
- the returned object cannot yet be described as a per-entity component solely from this call;
- the three floats at mapped-value offsets `+0xe8/+0xf0` must not be labelled entity world position without further proof.

The surrounding loop and later entity-handle validation are separate operations; their proximity does not prove that this string-keyed map lookup is keyed by the entity handle.

## What remains useful

The mapped value, when found, is still passed as a three-float vector into Hitbox helper `0xb58ecd0`. That helper still returns two output vectors through its output structure, and its fallback path adds two vectors from the HitboxComponent payload to two context-derived vectors. But the source and meaning of the input vector remain unresolved.

## Next static target

Identify the string contents produced at `x22+0x50` and determine the concrete type of the map owner returned by vtable slot `+0x368`. Trace other reads of the mapped value's `+0xe8/+0xf0` fields. Only then can this lookup be connected to a specific entity or coordinate system.

## Runtime status

No hooks or raw runtime memory reads were added. The installed module remains the HUD/Mod Menu diagnostic prototype; live ESP boxes are not implemented.
