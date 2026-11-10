# 2026-11-10 — Correction: 0x14777e30 is the second word of a global shared_ptr pair

## Result

The earlier description of `0x14777e30` as an independent global pointer slot was too strong. A follow-up AArch64 store trace shows it is addressed as the second 8-byte word of a two-word global object beginning at `0x14777e28`.

Three store sites were identified:

1. `0x854593c`, inside `Java_com_mojang_minecraftpe_NetworkMonitor_nativeUpdateNetworkStatus`:
   - `ADRP x1, 0x14777000`
   - `ADD x1, x1, #0xe28`
   - `STP xzr, xzr, [x1, #8]`
   - This clears the two words at `0x14777e30` and `0x14777e38`.
2. `0x124fbcc0`, inside `Java_com_mojang_minecraftpe_WorldRecovery_nativeComplete`:
   - `x22 = 0x14777e28`
   - `STP xzr, xzr, [x22]`
   - Clears the pair beginning at `0x14777e28`.
3. `0x1252f4dc`, also inside `Java_com_mojang_minecraftpe_WorldRecovery_nativeComplete`:
   - `x8 = 0x14777e28`
   - `STP xzr, xzr, [x8]`
   - Clears the same pair.

The NetworkMonitor routine also loads the two adjacent words separately:
- `LDR x25, [0x14777e28]`
- `LDR x19, [0x14777e30]`

It checks the first word as an object pointer and passes the second word into shared-reference bookkeeping. The two-word layout and the `shared_ptr`-style retain/release sequence strongly indicate a global shared-ownership object (object pointer plus control-block pointer), rather than a raw `IEntityRegistryOwner*` slot.

## Consequence for the ESP investigation

This global is not evidence of a direct gameplay ECS accessor. Its nearby use is in NetworkMonitor and WorldRecovery lifecycle/cleanup code. The earlier interpretation that `0x14777e30` itself was the sought owner pointer is withdrawn.

The checked `NonOwnerPointer<IEntityRegistryOwner>::_get()` sites still need their own parent-object types and field provenance resolved. The NetworkMonitor accessor-looking sequence should not be used as a runtime hook or as an entity-context offset.

## Next step

Resolve the object type stored in the first word at `0x14777e28` by tracing its construction/assignment path, then continue independently with the actual inlined `_get()` sites. No offsets or hooks should be enabled based on this global pair.

## Build/runtime status

No Android build was produced. The current HUD and AABB projection pipeline still lacks verified live entity and camera inputs. Live ESP boxes remain unimplemented.
