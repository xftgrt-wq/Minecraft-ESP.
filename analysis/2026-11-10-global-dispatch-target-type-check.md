# 2026-11-10 — global dispatch target identity check

## Exact binary

- Target: previously supplied ARM64 ELF `libminecraftpe.so`
- Size: 349,243,744 bytes
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- GNU Build ID: `2e318db12824cadb2618754ab7c82fa96fb30659`

## Result: the prior global-service lead is not an ECS accessor

The wrapper at VA `0x12b420ec` loads the global pointer slot at `0x1473fa48`, then performs an indirect virtual call through the object vtable at offset `+0x18`.

The relocation table resolves the pointer slot:

- `0x1473fa48 -> 0x14694790`

The relocation-backed vtable/typeinfo cluster around `0x14694700–0x14694850` identifies the object's dynamic type. The RTTI type-name pointer at `0x146947e0` resolves to string VA `0x335e79c`, whose exact mangled name is:

```
N7Bedrock6Memory15MallocAllocatorE
```

Demangled: `Bedrock::Memory::MallocAllocator`.

This is a decisive negative result for the earlier hypothesis: the global object reached by the wrapper is a Bedrock memory allocator, not a StrictEntityContext, ECS registry, or scheduler. The wrapper is therefore an allocator-service dispatch path and must not be repurposed as an entity accessor.

## What is now confirmed

1. The code at `0x12b420ec` uses a global object through its vtable.
2. The global pointer slot resolves to the object/vtable cluster at `0x14694790`.
3. RTTI identifies that object's class as `Bedrock::Memory::MallocAllocator`.
4. This candidate does not provide a route to live ECS entities.

## Next investigation

Discard `0x1473fa48` as an ECS-context candidate. Return to the relocation-backed strict-system adapter records and trace their actual registration/owner construction path, looking for concrete scheduler or StrictEntityContext types rather than generic global services. Keep all runtime hooks disabled; no safe entity accessor or ABI has been established.

## Status

No runtime code or hook was changed. No new Android build was produced. Live ESP boxes remain unimplemented.
