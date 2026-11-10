# 2026-11-10 — IEntityRegistryOwner _get() call-site provenance follow-up

## Scope

Continued static analysis of the user's ARM64 `libminecraftpe.so` (Build ID `2e318db12824cadb2618754ab7c82fa96fb30659`). No runtime hooks, offsets, or Android binaries were changed.

## Confirmed disassembly

The checked-access signature string for `NonOwnerPointer<IEntityRegistryOwner>::_get() const` at VA `0x23f4709` is materialized inline at multiple sites. Three inspected examples:

- `0x85501fc`: loads `x20 = [x19 + 0x10]`, then checks `[x20 + 0x310]` and later calls through `[x20 + 0x320]`.
- `0x9c41938`: loads `x20 = [x19 + 0x1d0]`, then performs the same `+0x310` validity checks and `+0x320` virtual call.
- `0x1081c028`: `x20` was previously loaded from `[x8 + 0x40]`; it then performs the same checks and virtual call.

The repeated sequence is consistent with an inlined checked `NonOwnerPointer<IEntityRegistryOwner>::_get()` path. It is not a standalone function that can safely be hooked by its signature string.

## Context correction

The first site is inside the large JNI function labelled `Java_com_mojang_minecraftpe_NetworkMonitor_nativeUpdateNetworkStatus`. The third is inside `Java_com_mojang_minecraftpe_AppExitInfoHelper_nativeSendApplicationExitInfo`. The second is in a nearby code region with the same broad JNI-labelled symbol range, but its owning object is loaded from a different parent offset.

These contexts do **not** establish that the pointer at `x20 + 0x310` belongs to the main `Minecraft` object or to the gameplay `StrictEntityContext`. In particular, the three sites use different parent-object provenance:

| Site | Parent source for x20 | Current conclusion |
|---|---|---|
| `0x85501fc` | `[x19 + 0x10]` | owner type unresolved |
| `0x9c41938` | `[x19 + 0x1d0]` | owner type unresolved |
| `0x1081c028` | `[x8 + 0x40]` | owner type unresolved |

The `+0x310` and `+0x320` offsets therefore remain local observations, not validated global Minecraft offsets. No code should use them as runtime offsets.

## What this establishes

- The `IEntityRegistryOwner` checked accessor is genuinely used by native code.
- At least some uses occur in JNI/network/application-exit-related code paths.
- The previous RTTI correction remains in force: `Minecraft -> IEntityRegistryOwner -> Bedrock::EnableNonOwnerReferences` is a single-inheritance RTTI chain; there is no demonstrated secondary base at `+0x10`.
- No inspected call site yet proves a gameplay entity-registry acquisition path.

## Next static-analysis step

Trace the stores and constructor/setup paths that populate the parent objects at the three sites, identify their concrete RTTI/vtable types, and determine whether any parent is the main `Minecraft` instance or a gameplay-owned service. Only after that should the `IEntityRegistryOwner` result be connected to strict ECS context or entity iteration.

## Build/runtime status

No new Android build was produced. The existing HUD bridge and AABB projection code still have no verified live entity/camera data source. **Live ESP boxes remain unimplemented.**