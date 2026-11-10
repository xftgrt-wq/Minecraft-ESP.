# 2026-11-10 — correction: string-table lookup callers are not proven hitbox/entity paths

## Verified input
- File: `libminecraftpe.so`
- Size: 349,243,744 bytes
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- ELF: AArch64, Android library.

## Important correction to the previous report
The call at `0x09E3CD78` is inside the function whose symbol is `Java_com_mojang_minecraftpe_NetworkMonitor_nativeUpdateNetworkStatus`. Therefore, this call site does **not** establish an entity registry, hitbox handler, or entity-ID lookup. The earlier interpretation that it connected the generic string-table helper to a hitbox/bounds path was too strong and is withdrawn.

Another caller at `0x0BF495A0` is in a function labelled by the disassembler as an aligned-delete operator (`_ZdlPvSt11align_val_t`), which is further evidence that the helper is shared by unrelated code paths.

## What the disassembly at 0x09E3CD78 actually shows
The caller iterates records with a stride of `0x90` bytes. For each record:
- it checks a byte flag at offset `+0x88`;
- it obtains a context-like object through a virtual call at vtable byte offset `+0x368`;
- it passes the address `record + 0x50` as the lookup key to `0x10D67694`;
- if the lookup succeeds, it reads a double at returned-object offset `+0xE8` and a float at `+0xF0`;
- it later passes `record + 0x28` to another helper, in a separate processing path.

These observations describe the caller's data flow only. They do not identify the record's semantic type, nor prove that the returned object contains entity bounds.

## Current conclusion
- `0x10D67694` remains identified as a generic string-keyed hash-table lookup helper.
- Its call sites span unrelated subsystems.
- No verified ECS/entity registry, live entity enumeration API, entity AABB source, or safe camera-projection interface has been identified from this investigation.
- Do not use this helper as an entity lookup or hook it on that assumption.

## Next static-analysis target
Trace the producer/constructor of the `0x90)-byte records in `NetworkMonitor_nativeUpdateNetworkStatus` only to understand this caller's data type and key layout. Separately search for entity iteration and AABB/camera projection through independently verified symbols and call chains. Keep all runtime hooks disabled until ABI, object lifetime, thread context, and coordinate space are established.

## Additional trace of the same caller
The value loaded from `[x25 + 0x1C8]` is passed to `0xE54EBFC`. The PLT entry at that address resolves to `std::__ndk1::__shared_weak_count::lock()`. This indicates the caller promotes a weak shared reference before using the resulting object; it still does not identify that object's domain type. The record iteration is therefore best treated as a generic object-processing path until its producer and concrete types are identified.

The record sequence is reached after a call to `0xE56F558`, which returns the range later traversed as records of stride `0x90`. The exact producer/type remains unresolved.
