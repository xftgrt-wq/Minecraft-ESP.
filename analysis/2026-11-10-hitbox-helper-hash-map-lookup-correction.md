# 2026-11-10 — Re-evaluation of the Hitbox helper storage lookup

## Exact binary

The analysis uses the uploaded ARM64 Minecraft library:
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- GNU Build ID: `2e318db12824cadb2618754ab7c82fa96fb30659`

## Important correction

The call at `0x9e3cd78` to `0x10d67694` should not be described as a paged ECS-storage lookup. Re-reading the callee shows a bucket-count / bucket-chain lookup consistent with a hash table:

- It reads a bucket-count-like value from `[x0+0x08]`.
- It reads the key payload from `[x1]`.
- It computes a bucket index (using a fast path for a power-of-two bucket count and a modulo path otherwise).
- It follows a bucket entry and compares key metadata/payload before returning the mapped value.
- The routine has map-node traversal and collision handling, rather than the paged-array indexing pattern seen in the ECS entity stores elsewhere.

Therefore the previous report's phrase “paged, generation-aware lookup” was too strong for this specific helper. The entity key may itself encode generation information, but this function is behaving as a hash-map lookup; that does not prove the map is an ECS paged component store.

## Caller dataflow

At `0x9e3cd60`:
1. `x25` is used as an object with a vtable.
2. The caller dispatches through vtable byte offset `+0x368`.
3. The returned pointer is advanced by `0x48`.
4. That subobject and the entity-key address `x22+0x50` are passed to the hash lookup at `0x10d67694`.
5. If found, the mapped object is read at offsets `+0xe8` and `+0xf0` to form the three-float input to Hitbox helper `0xb58ecd0`.

The object `x25` is obtained earlier in the same entity-processing loop from an entity-associated pointer table, and a byte at `x25+0x251` is checked before this branch. This ties the operation to an entity-associated object, but does not identify the concrete C++ class or the meaning of the byte flag.

## What can be concluded

- The lookup path is a hash map associated with an object returned by a virtual call on the current entity-associated object.
- The lookup key is the entity key stored at `x22+0x50`.
- The mapped object contributes three floats from `+0xe8/+0xf0` to the Hitbox helper input.
- The concrete mapped type and coordinate semantics remain unidentified.

## What cannot be concluded

- The virtual method at vtable offset `+0x368` is not yet resolved to a concrete implementation or class declaration.
- The map's exact C++ key/value types are not yet proven.
- The three floats are not proven to be world coordinates, render interpolation coordinates, or camera-relative values.
- The path is not a safe general-purpose entity iterator and does not establish coverage of every player or dropped item.

## Next static target

Resolve the dynamic type/vtable of `x25` and the implementation behind slot `+0x368`; then identify the map value type by tracing other consumers of the returned object and its `+0xe8/+0xf0` fields. Do not infer a component name from offsets alone.

## Runtime status

No hook or raw game-memory access was added. The current installed module remains the HUD/Mod Menu diagnostic prototype; it has no live entity feed and no verified 3D ESP boxes.
