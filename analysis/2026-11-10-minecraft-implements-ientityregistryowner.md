# 2026-11-10 — Minecraft implements IEntityRegistryOwner

## Exact binary

- ARM64 ELF64 `libminecraftpe.so`
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- GNU Build ID: `2e318db12824cadb2618754ab7c82fa96fb30659`

## Confirmed inheritance chain

The relocation cluster around `0x1443ff20–0x1443ff48` contains two adjacent Itanium RTTI records:

- Typeinfo at `0x1443ff20` has its name pointer at `0x1443ff28`, resolving to `20IEntityRegistryOwner` at `0x2fa91c3`.
- Typeinfo at `0x1443ff38` has its name pointer at `0x1443ff40`, resolving to `9Minecraft` at `0x2fa91b8`.
- The `Minecraft` typeinfo's base-type pointer at `0x1443ff48` resolves exactly to `0x1443ff20`, the `IEntityRegistryOwner` typeinfo.

This is direct RTTI evidence that **Minecraft derives from IEntityRegistryOwner**. It is substantially stronger than the earlier video-capture consumer lead: the interface is part of the main Minecraft class hierarchy.

## Minecraft vtable

A relocation at `0x1443fd88` points to the `Minecraft` typeinfo at `0x1443ff38`. This is the RTTI slot in the associated vtable group; the following function-pointer entries begin at `0x1443fd90`.

The first entries include code at:

- `0x1081b404`
- `0x1081b7e0`
- `0x1081ea38`
- `0x1081ea60`
- `0x1081d72c`
- `0x1081d878`

The first routine writes the `Minecraft` vtable address into the object at `[x0]` and performs member cleanup/initialization work, consistent with a constructor/destructor-family routine. This is not enough to assign reliable semantic names to individual virtual slots.

## What this proves — and what it does not

Proven:
1. The exact game binary contains an `IEntityRegistryOwner` interface.
2. `Minecraft` is an RTTI-confirmed derived class.
3. The main Minecraft object is therefore a concrete candidate owner to investigate for registry access.

Not yet proven:
- Which `Minecraft` field or method exposes the registry.
- Whether the returned registry is the same `StrictEntityContext` used by the gameplay systems identified earlier.
- Whether there is a safe entity iteration API and stable lifetime on the HUD/render thread.
- The ABI and synchronization requirements for a runtime call.

## Next investigation

Trace references to `NonOwnerPointer<IEntityRegistryOwner>::_get()` and the `EntityRegistryCreation` lifecycle event into the `Minecraft` object. Then match any resulting registry/context type against the strict-system component access metadata. Do not add a guessed object offset or runtime hook.

No source code or hooks were changed. No Android build was produced. Live ESP boxes are still not implemented.
