# 2026-11-10 — IEntityRegistryOwner secondary-vtable audit

## Exact binary

- ARM64 ELF64 `libminecraftpe.so`
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- GNU Build ID: `2e318db12824cadb2618754ab7c82fa96fb30659`

## What the relocation table confirms

The RTTI/vtable relocation cluster at `0x1443fd88–0x1443ff78` contains the primary `Minecraft` vtable group and several secondary vtable/typeinfo records.

- `0x1443fd88` points to `Minecraft` typeinfo at `0x1443ff38`.
- `0x1443ff28` points to the `IEntityRegistryOwner` type name.
- `0x1443ff48` points to the `IEntityRegistryOwner` typeinfo object at `0x1443ff20`.
- `0x1443ff20` carries an offset-to-top value of `0x10`, consistent with a secondary base subobject located 0x10 bytes before the complete-object address point.
- The adjacent `Minecraft` typeinfo record at `0x1443ff38` names `9Minecraft` and its base-type pointer resolves to `0x1443ff20`.

This reinforces the previous conclusion: the binary's `Minecraft` class implements/derives from `IEntityRegistryOwner`.

## Important limitation in the candidate vtable entry

The relocation at `0x1443ff18` resolves to `0x14054458`. That target lies inside the binary's writable-then-RELRO data segment, not inside the executable `.text` segment. It therefore cannot be treated as a normal callable method address without further understanding of the vtable-group layout/relocation semantics.

I did not cast it to a function pointer, infer a registry accessor signature, or add a runtime call. The other nearby vtable entries include both executable-looking addresses and data/RTTI references, so blindly walking this cluster as a flat function table would be unsafe.

## Constructor-side observations

The routine at `0x1081b404` installs a vtable address derived from `0x1443fd90` into the object at `[x0]`, and then performs substantial member cleanup/initialization. It is consistent with a `Minecraft` object lifecycle routine, but the disassembly excerpt does not identify a registry field or accessor.

## Current status

- `Minecraft : IEntityRegistryOwner`: confirmed by RTTI.
- Exact registry accessor / registry member: not identified.
- Link to gameplay `StrictEntityContext`: not proven.
- Safe entity iteration, object lifetime, and render-thread access: not proven.
- Runtime hooks remain disabled.
- No source code was changed and no Android build was produced.
- Live ESP boxes remain unimplemented.

## Next useful lead

Resolve the secondary-vtable layout and identify the actual `IEntityRegistryOwner` virtual method(s) using complete-object-layout evidence, then cross-reference their implementations against the `EntityRegistryCreation` event and strict-system context. Do not treat data-segment targets as callable methods.
