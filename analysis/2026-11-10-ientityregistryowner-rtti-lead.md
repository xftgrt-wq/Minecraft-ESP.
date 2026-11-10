# 2026-11-10 — IEntityRegistryOwner RTTI lead

## Exact target

Previously supplied `libminecraftpe.so`:
- ELF64, AArch64
- Size: 349,243,744 bytes
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- GNU Build ID: `2e318db12824cadb2618754ab7c82fa96fb30659`

## New static findings

A targeted string/relocation pass found the following exact metadata:

- Mangled RTTI type-name string at VA `0x2fa91c3`: `20IEntityRegistryOwner`.
- A relocation at `0x1443ff28` points to that type-name string, consistent with an RTTI/typeinfo record for `IEntityRegistryOwner`.
- A separate engine string at VA `0x2f0e7e0` reads `EntityRegistryCreation`.
- The binary also contains the helper signature string at VA `0x23f4709`:
  `Bedrock::NonOwnerPointer<IEntityRegistryOwner>::_get() const`.

These are concrete indicators that this build has an entity-registry ownership interface and a named registry-creation lifecycle/event concept.

## Interpretation limits

This does **not** yet prove:
- that the `IEntityRegistryOwner` interface is the owner of the same `StrictEntityContext` used by the previously identified strict systems;
- that `EntityRegistryCreation` is emitted by the client-side world path relevant to this mod;
- that a stable owner instance is accessible from the Preloader mod lifecycle;
- that the interface exposes a safe public accessor or iteration API.

The `NonOwnerPointer` helper is a typed non-owning pointer accessor, not evidence of a global singleton or a usable external pointer by itself.

## Next trace

1. Recover the RTTI object and vtable around the relocation referencing `0x2fa91c3`.
2. Identify concrete implementers/derived types of `IEntityRegistryOwner`.
3. Trace the `EntityRegistryCreation` event's registration and producer/consumer references.
4. Only if those paths converge on the strict-system context, investigate its lifecycle and a safe read-only entity view.

No runtime hook, hard-coded accessor, or guessed offset was added. No Android build was produced. Live ESP remains unimplemented and game hooks remain disabled.
