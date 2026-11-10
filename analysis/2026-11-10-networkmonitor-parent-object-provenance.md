# 2026-11-10 — Follow-up: parent-object provenance at the NetworkMonitor accessor site

## Additional trace

Disassembly around `Java_com_mojang_minecraftpe_NetworkMonitor_nativeUpdateNetworkStatus` at `0x8544c5c` shows the function loading a process-global pointer from address `0x14777e30`:

```asm
adrp x8, 0x14777000
ldr  x19, [x8, #0xe30]   // global slot 0x14777e30
...
ldr  x20, [x19, #0x10]
...
ldr  x8, [x20, #0x310]  // checked IEntityRegistryOwner pointer
```

The global slot is in the binary's zero-initialized data region, so its runtime value is established during initialization and cannot be recovered from the on-disk bytes alone. This confirms that the first inspected accessor site is reached through a process-global NetworkMonitor-related state object, not through a direct statically visible `Minecraft*` argument.

The other two sites remain distinct:
- At `0x9c41938`, `x20` is loaded from the caller's `[x19 + 0x1d0]`; the function receives `x2` as an object and invokes its virtual slot `+0x100` before continuing.
- At `0x1081c028`, `x20` is loaded from `[x8 + 0x40]`, where `x8` came from `[x19 + 0xb0]` in the JNI application-exit helper.

## Assessment

This improves parent-pointer provenance but still does not identify the concrete runtime class of the objects or prove that any is the main `Minecraft` object. The global slot's runtime initialization and the owning class layout are not available from this static snapshot. The `+0x310` field must not be treated as a reusable game offset.

No hook or build change is justified by this evidence. The next useful static lead is to trace initialization/writes to global slot `0x14777e30` and the concrete type behind the object at `[global + 0x10]`, while independently resolving the owner types at the other call sites.

## Runtime status

No new Android build. HUD rendering and AABB projection remain implemented, but live entity and camera data are still not connected. Live ESP boxes are not ready.
