# 2026-11-10 — EntityRegistryCreation cross-reference: diagnostic path, not registry acquisition

## Exact binary

- ARM64 ELF64 `libminecraftpe.so`
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- GNU Build ID: `2e318db12824cadb2618754ab7c82fa96fb30659`

## Direct code reference recovered

The string `EntityRegistryCreation` is at VA `0x2f0e7e0`. A code-reference scan of the AArch64 `.text` section found an ADRP/ADD pair at:

- `0xfdc7d20`: `adrp x1, 0x2f0e000`
- `0xfdc7d24`: `add x1, x1, #0x7e0`

The surrounding code is inside the function identified by the ELF disassembler as:

```
Java_com_mojang_minecraftpe_AppExitInfoHelper_nativeSendApplicationExitInfo
```

At this site, the code loads an object from `[x20 + 0x210]`, obtains its vtable, loads slot offset `0x920`, then calls it with:
- x1 = pointer to `EntityRegistryCreation`
- w2 = 0
- w3 = 0

This is not an entity-registry getter and does not pass an `IEntityRegistryOwner*` to an accessor. It is a generic virtual call receiving a string label.

## Strong cross-check: same call pattern for neighboring labels

The same function contains matching call sequences using the same receiver field `[x20 + 0x210]`, the same vtable slot `+0x920`, and zero-valued arguments for other strings in the contiguous label cluster:

| String | Code site |
|---|---:|
| `LevelStorageCreation` | `0xfdc4f68` |
| `PackSourceLoading` | `0xfdc5ea0` |
| `AssetExtraction` | `0xfdc72b4` |
| `FinalLevelStorageState` | `0xfdc7b78` |
| `EntityRegistryCreation` | `0xfdc7d20` |

This makes it unsafe to interpret `EntityRegistryCreation` as proof that the game is creating or exposing the gameplay ECS registry at this call site. The labels are being passed as stage/category identifiers to a shared virtual method in an application-exit-info path.

## Corrected interpretation

- The string is genuinely referenced by executable code.
- Its recovered use is a generic stage-label argument in `AppExitInfoHelper::nativeSendApplicationExitInfo`.
- This cross-reference does **not** identify the registry owner, registry pointer, creation callback, or gameplay `StrictEntityContext`.
- The earlier report's event-name interpretation should remain tentative; this new xref narrows it to a diagnostic/reporting use at this site.

## Next target

Stop treating the `EntityRegistryCreation` string as an accessor lead. Continue from the confirmed `Minecraft : IEntityRegistryOwner` RTTI relationship by recovering the complete secondary-vtable group and finding actual executable function entries/constructor stores for the `IEntityRegistryOwner` subobject. Independently match any candidate method to the EnTT/strict-context registration path before considering runtime integration.

No source code, offsets, or hooks were changed. No Android build was produced. Live ESP boxes remain unimplemented.
