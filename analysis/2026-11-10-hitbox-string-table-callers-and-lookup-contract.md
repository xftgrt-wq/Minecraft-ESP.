# Hitbox string-table lookup: caller cross-references and contract

## Input binary

- File: `libminecraftpe.so`
- Size: 349,243,744 bytes
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- ELF64, little-endian, AArch64; stripped.

## Lookup routine

The routine at `0x10d67694` is a lookup helper over a bucketed hash table. The call ABI observed at callers is:

- `x0`: table-like object
- `x1`: pointer to a key representation

The helper reads a bucket count from `[x0 + 0x8]`, obtains a hash-like value from `[x1]`, selects a bucket, follows chained entries, and compares key length/content before returning a value pointer or null. The key comparison path reads key metadata around `x1 + 8` and inline/indirect character data. This is consistent with a string-keyed hash map, not a direct entity-ID lookup.

The routine's internal layout is implementation-specific. Do not treat its offsets as a public API or copy them into runtime code without proving object type and lifetime.

## Cross-references

AArch64 direct-call scan of the executable `.text` section found 24 BL call sites:

`0x09e3cd78`, `0x09e9574c`,
`0x0b021248`, `0x0b0212f0`, `0x0b021360`, `0x0b0213cc`, `0x0b021438`, `0x0b021504`, `0x0b021580`, `0x0b0215fc`,
`0x0b58744c`,
`0x0bd1adac`, `0x0bd1ae78`, `0x0bd1aec4`, `0x0bd1af10`, `0x0bd1af5c`, `0x0bd1b000`, `0x0bd1b028`, `0x0bd1b050`,
`0x0bf495a0`,
`0x0c689088`, `0x0c6890a8`, `0x0c689360`, `0x0c689380`.

At `0x0bf495a0`, the caller obtains a context-like object through a virtual call at vtable byte offset `+0x368`, adds `0x48`, and passes that table-like address as `x0`. The key pointer is derived from a nearby caller-owned record. At `0x0c689088` and `0x0c6890a8`, the same lookup is performed twice against records separated by `0x28`; nearby code later compares a float at offset `+0xec` in the returned objects. These call sites show the helper is reused by multiple subsystems and is not, by itself, proof of an ECS entity registry.

At `0x09e3cd78`, the caller passes a key pointer based on `x22 + 0x28` and uses the returned object in a subsequent transform/bounds path. This ties the lookup helper to the previously examined hitbox-related path, but does not establish the semantic type of the key or returned object.

## Conclusions and limits

1. The earlier characterization as a lookup by numeric entity ID is unsupported and should be discarded.
2. The helper behaves as a string-keyed hash lookup. Its exact key schema and map owner remain unidentified.
3. The hitbox caller's key source and returned object's type are still unresolved.
4. The virtual method at `+0x368` and the object at `context + 0x48` are not safe runtime access points until their concrete class, lifetime, synchronization, and call-thread requirements are established.
5. No hooks, raw game-memory reads, or binary changes were made. This is static analysis only.

## Next investigation

Trace the key producer in the hitbox-related caller at `0x09e3cd78`, then compare it with the record/key producers at the other call sites. Identify the owning class for the `+0x368` virtual method from its vtable/RTTI context. Do not assume a hitbox/entity mapping until the key and returned value types are independently established.
