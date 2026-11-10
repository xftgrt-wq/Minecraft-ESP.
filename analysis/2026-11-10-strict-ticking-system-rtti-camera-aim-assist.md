# 2026-11-10 — Strict ticking-system RTTI resolved for the CameraAimAssist adapter

## Binary identity

This analysis uses the exact ARM64 library supplied for this project:

- File: `libminecraftpe.so`
- Size: 349,243,744 bytes
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- GNU Build ID: `2e318db12824cadb2618754ab7c82fa96fb30659`

## New result

The RTTI owner of the vtable installed by the helper at VA `0x9e80b4c` is now identified.

The vtable group begins at `0x140605d0`:

- `0x140605d0`: offset-to-top relocation to `0x14060620`
- `0x140605d8`: first virtual entry, `0x9e80b18`
- `0x140605e0`: `0x9e80b1c`
- `0x140605e8`: `0x9e80b4c`
- `0x140605f0`: `0x9e80bc8`
- `0x140605f8`: `0x9e80bd8`
- `0x14060600`: `0x9e80bdc`
- `0x14060608`: `0x9e80c0c`
- `0x14060610`: `0x9e80c14`
- `0x14060618`: `0x9e80c30`

The associated RTTI object is at `0x14060598`. Its type-name relocation at `0x140605a0` resolves to:

```
IStrictTickingSystem<StrictExecutionContext<Filter<...>>>
```

The full mangled type-name string starts at VA `0x2991fba` and includes the component filter/read set. This is a strict ticking-system interface specialization, not an ECS registry accessor.

The neighboring adapter type-name at VA `0x2991c86` resolves to:

```
StrictTickingSystemFunctionAdapter<&CameraAimAssistFetchValidEntityTargetSystem::tick, ...>
```

Its template arguments include a `StrictEntityContext` view and the component access set, including optional `HitboxComponent`, `AABBShapeComponent`, and `SubBBsComponent`. This ties the nearby adapter machinery to the camera aim-assist target-fetch system.

## What this establishes

1. The previously anonymous vtable at `0x140605d8` belongs to an `IStrictTickingSystem` specialization.
2. The adjacent function-adapter metadata is specifically associated with `CameraAimAssistFetchValidEntityTargetSystem::tick`.
3. The helper at `0x9e80b4c` allocates a 16-byte object and installs that strict-system vtable; this is consistent with a small adapter/interface object.
4. The strict-system type's component contract is now better grounded in RTTI than the earlier generic helper interpretation.

## What it does not establish

- It does not reveal a global live ECS registry pointer.
- It does not prove that the adapter's 16-byte object is itself the registry or owns entity storage.
- It does not establish a callable entity iteration API, component memory layout, safe lifetime, render-thread synchronization, or a hook ABI.
- It does not justify enabling a hook at any of these helper addresses.

The global dispatch used by these helpers remains a generic allocator/dispatch path; prior RTTI identified the relevant allocator as `Bedrock::Memory::MallocAllocator`. It must not be re-labelled as an ECS service.

## Next reverse-engineering step

Trace the callsites that create/register the `StrictTickingSystemFunctionAdapter` and identify the owning system-registration container and its lifetime. Then determine whether the strict execution context exposes a stable read-only entity view that can be reached safely from a supported game callback. Until that is proven, the native module remains HUD-only.

## Project status

No live entity feed has been connected to the overlay. No working player/item boxes have been verified in-game. Hooks remain disabled.
