# 2026-11-10 — IEntityRegistryOwner consumer cross-check

## Exact binary

- ARM64 ELF64 `libminecraftpe.so`
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- GNU Build ID: `2e318db12824cadb2618754ab7c82fa96fb30659`

## Cross-check of the new RTTI lead

The `IEntityRegistryOwner` RTTI name at VA `0x2fa91c3` is referenced by a relocation at `0x1443ff28`. The surrounding RTTI records form a run of unrelated interface/type metadata; this is type information, not a live object or vtable-backed global singleton.

The binary contains the exact helper signature string at VA `0x23f4709`:

```
T *Bedrock::NonOwnerPointer<IEntityRegistryOwner>::_get() const [T = IEntityRegistryOwner]
```

This confirms a typed non-owning-pointer accessor specialization exists in the binary. It does not establish where an instance is stored or which owner implementation is returned.

## Concrete named consumer found

A neighboring mangled template/lambda name at VA `0x2aeb71e` identifies:

```
videocapture::details::setupCameraTimeComponentForFixedRateUpdate(
    IEntityRegistryOwner&, Timer&)
```

Related lambda and shared-pointer type names immediately follow it in the same string cluster (`0x2aeb7c5`, `0x2aeb865`). This is a concrete use of the interface by the video-capture subsystem to set up a camera-time component for fixed-rate updates.

This consumer is not evidence that the interface is the world ECS registry owner used by gameplay strict systems. It may be a separate registry/context used by video capture. No bridge from this function to the gameplay `StrictEntityContext` has been demonstrated.

## EntityRegistryCreation string

The exact string at VA `0x2f0e7e0` is `EntityRegistryCreation`. It appears among a contiguous group of lifecycle/event-like names, including `LevelStorageCreation`, `AssetExtraction`, `FinalLevelStorageState`, and `HostMultiplayer`.

The string alone does not establish a callback signature, event producer, or connection to the gameplay registry. Treat it as a named event/category until code references and its owning enum/type are recovered.

## Updated conclusion

Confirmed:
1. `IEntityRegistryOwner` is a real interface type in this exact binary.
2. A typed `NonOwnerPointer<IEntityRegistryOwner>::_get()` specialization exists.
3. At least one explicit named consumer belongs to the video-capture camera-time setup path.
4. `EntityRegistryCreation` appears in a lifecycle/event-name cluster.

Not confirmed:
- The gameplay world's registry owner implementation.
- Whether video-capture's owner shares the same registry as gameplay strict systems.
- A stable runtime pointer or a safe entity iteration API.
- Any render-thread synchronization or component layout.

## Next step

Trace the video-capture helper only far enough to identify the registry abstraction it uses, then separately locate gameplay/world implementers of `IEntityRegistryOwner` and prove whether they converge on the same strict context. Do not use the video-capture owner as a shortcut unless that identity is established.

No runtime code or hooks were changed. No new Android build was produced. Live ESP remains unimplemented.
