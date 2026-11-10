# 2026-11-10 — Corrected IEntityRegistryOwner RTTI chain and _get call-site trace

## Exact binary

- ARM64 ELF64 `libminecraftpe.so`
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- GNU Build ID: `2e318db12824cadb2618754ab7c82fa96fb30659`

## Correction: the supposed +0x10 secondary-base offset was not established

The earlier secondary-vtable report misread the RTTI data around `0x1443ff18`. That address is not a proven virtual function slot. It is the base-type pointer field of the preceding `__si_class_type_info` record.

The relevant records decode as:

- `0x1443ff20`: `__si_class_type_info` for `IEntityRegistryOwner`
  - name pointer at `0x1443ff28` -> `20IEntityRegistryOwner` at `0x2fa91c3`
  - base type pointer at `0x1443ff30` -> `0x1468c308`
- `0x1468c308`: `__class_type_info` for `Bedrock::EnableNonOwnerReferences`
  - name pointer at `0x1468c310` -> `N7Bedrock24EnableNonOwnerReferencesE` at `0x33568d8`
- `0x1443ff38`: `__si_class_type_info` for `Minecraft`
  - name pointer at `0x1443ff40` -> `9Minecraft` at `0x2fa91b8`
  - base type pointer at `0x1443ff48` -> `0x1443ff20` (`IEntityRegistryOwner`)

Therefore the RTTI inheritance chain is:

```
Minecraft
  -> IEntityRegistryOwner
       -> Bedrock::EnableNonOwnerReferences
```

Both relevant records use `__si_class_type_info`, which describes single non-virtual inheritance. This evidence does **not** establish a secondary base at offset +0x10. The previous report's offset-to-top interpretation should not be used.

The only relocation in this cluster that points to the `Minecraft` typeinfo is at `0x1443fd88`, with the associated address point beginning at `0x1443fd90`. This is consistent with a primary vtable carrying `Minecraft` RTTI; it does not reveal a separate `IEntityRegistryOwner` secondary vtable.

## New lead: inlined NonOwnerPointer::_get checks

The helper signature string at VA `0x23f4709` is referenced by many AArch64 code sites. At each inspected site, code forms the exact address using ADRP/ADD and passes it to a shared assertion/reporting routine, together with source-line values such as 314 or 318.

One example is the sequence at `0x85501fc–0x8550288`:

- loads a member pointer from `[x20 + 0x310]`;
- branches to an assertion path if it is null;
- checks the pointed-to object's first byte;
- then obtains another member at `[x20 + 0x320]` and makes a virtual call.

This is consistent with an inlined checked access path involving `NonOwnerPointer<IEntityRegistryOwner>::_get()`, but the enclosing object type and the exact meaning of those fields have not yet been proven. The nearby function is associated by the disassembler with a NetworkMonitor JNI routine; that is not enough to claim it is the gameplay world's owner.

The repeated call-site pattern gives a more actionable next trace than treating the helper signature string as a callable function symbol: identify the owning object at selected call sites, recover the pointer member and its producer/assignment path, and then see whether the resulting `IEntityRegistryOwner*` is the main `Minecraft` object.

## What is now confirmed

1. `Minecraft` derives from `IEntityRegistryOwner` according to exact RTTI.
2. `IEntityRegistryOwner` derives from `Bedrock::EnableNonOwnerReferences`.
3. The previously claimed +0x10 secondary-subobject offset is unsupported and withdrawn.
4. `NonOwnerPointer<IEntityRegistryOwner>::_get()` has many inlined assertion/check call sites.
5. At least one candidate call site checks a pointer member at object offset `0x310`, but the owner object and pointer provenance are not identified.

## What remains unproven

- A registry pointer or registry accessor on `Minecraft`.
- A connection between the checked non-owner pointer and the gameplay `StrictEntityContext`.
- Entity iteration, component access, or stable render-thread lifetime.
- Any safe runtime hook or callable ABI.

No source code, runtime offsets, or hooks were changed. No Android build was produced. Live ESP boxes remain unimplemented.
