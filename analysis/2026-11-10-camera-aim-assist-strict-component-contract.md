# 2026-11-10 — CameraAimAssist strict adapter component contract

## Exact binary

- `libminecraftpe.so`
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- Build ID: `2e318db12824cadb2618754ab7c82fa96fb30659`

## Exact RTTI strings

The binary contains the adapter type name at VA `0x2991c86`:

```
StrictTickingSystemFunctionAdapter<
  &CameraAimAssistFetchValidEntityTargetSystem::tick,
  ViewT<StrictEntityContext, ...>
>
```

The matching strict-system interface specialization begins at VA `0x2991fba`:

```
IStrictTickingSystem<
  StrictExecutionContext<
    Filter<
      Exclude<ActorRemovedFlagComponent, LocalPlayerComponent>,
      Read<
        HitboxComponent,
        AABBShapeComponent,
        SubBBsComponent,
        SynchedActorDataComponent,
        BoatFlagComponent,
        MinecartFlagComponent,
        MobFlagComponent,
        PlayerComponent,
        ActorOwnerComponent,
        CameraAimAssistComponent,
        CameraAimAssistTickComponent,
        CameraAimAssistCachedFrustumComponent,
        CameraAimAssistCachedPositionDataComponent,
        CameraAimAssistResetEntityExcludedStatusFlagComponent
      >,
      Write<CameraAimAssistEntityTargetComponent>,
      AddRemove<CameraAimAssistEntityExcludedStatusComponent>,
      GlobalRead<CameraAimAssistActorPriorityClientComponent>
    >
  >
>
```

The complete mangled strings are in the supplied binary; the formatted type outline above is a readability rendering of those template arguments, not a reconstructed source declaration.

## Implications for the ESP investigation

This confirms a useful, concrete fact: the game already has a strict ticking system whose entity view reads `AABBShapeComponent` and `SubBBsComponent`, and whose filter includes player/mob-related flags. It is a strong candidate for understanding how Bedrock's own systems consume entity bounds.

However, the contract is system-specific. It does not expose an unrestricted entity iterator to a mod, and its callback is not a supported public render hook. Reusing its internal context from an unrelated thread or callback would be unsafe without proving construction, scheduling, ownership, and lifetime.

The small adapter object created by the helper around `0x9e80b4c` is not the ECS registry. The allocator dispatch used in its construction is generic; previous RTTI work identifies that dispatch as `Bedrock::Memory::MallocAllocator`.

## Current boundary

- Known: the adapter's exact system identity and component access contract.
- Known: it consumes a strict execution context supplied by Bedrock's system scheduler.
- Unknown: the scheduler/registration owner callsite and how its context is instantiated at runtime.
- Unknown: a stable mod-accessible entity view, component layouts, and safe render-thread handoff.
- Not established: any valid hook target or ABI.

No hook was enabled and no live entity source was added. The ESP overlay remains HUD-only pending a proven game-data source.
