# 2026-11-10 — Exact-binary ECS storage RTTI follow-up

## Binary revalidation

The prior Library upload `libminecraftpe.so` was materialized and inspected again:
- Size: 349,243,744 bytes
- ELF64, little-endian, AArch64
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- GNU Build ID from the previous ELF audit: `2e318db12824cadb2618754ab7c82fa96fb30659`

This is the same binary identity as the earlier ECS/camera notes.

## New concrete storage metadata

The binary contains RTTI names for EnTT component-storage instantiations:

- `entt::basic_storage<StateVectorComponent, EntityId, ...>`
  - type-name string at VA `0x27b9609`
  - typeinfo object at `0x13f713d8` (name pointer at `0x13f713e0`)
  - associated vtable address point is consistent with `0x13f71380`
- `entt::basic_storage<AABBShapeComponent, EntityId, ...>`
  - type-name string at VA `0x27b9863`
  - typeinfo object at `0x13f71648` (name pointer at `0x13f71650`)
  - associated vtable address point is consistent with `0x13f71670`

The relocation table also exposes adjacent `shared_ptr_emplace<basic_storage<...>>` RTTI objects. This confirms the engine instantiates separately managed EnTT storage objects for both component types; it is stronger than seeing only component-name strings.

## Hitbox-update adapter metadata

The exact adapter type-name string is at VA `0x32d9ad8`:

`StrictTickingSystemFunctionAdapter<&VanillaSystems::ActorMoveSystemImpl::tickUpdateHitboxSystem, ...>`

Its strict-system descriptor at `0x32d9be3` declares:
- Filter/include: `MoveRequestComponent`
- Read: `StateVectorComponent`
- Write: `HitboxComponent`

The relocation-backed metadata cluster around `0x145efa98–0x145efad8` links this adapter name, descriptor name, and related RTTI objects. It does not directly expose a callable, stable entity-iteration entry point.

## What this adds — and what it does not

Confirmed:
1. The exact binary has EnTT `basic_storage` instantiations for `StateVectorComponent` and `AABBShapeComponent`.
2. The hitbox update adapter has a declared strict-system read/write signature.
3. Storage-level virtual method code is present and relocation-backed.

Not confirmed:
- the live registry/strict-context object address;
- a safe way to acquire the storage instances from the running world;
- component value layouts or field offsets;
- whether a storage object's current address remains stable across world reloads;
- a callable system tick wrapper with proven ABI and safe frame timing.

The storage RTTI/vtable candidates must not be treated as global pointers or as a supported registry accessor. No new runtime hook is justified by this finding.

## Next targeted trace

The next static-analysis target is the owner/registration path for the strict-system adapter metadata around `0x145efa98`, and the owner/creation path for the two storage RTTI/vtable objects. The objective is to establish whether a world-owned strict context or registry accessor is passed through a known lifecycle boundary. In parallel, continue tracing the TAA context producer rather than using the matrix field without an owner/lifetime proof.

No offsets or hook signatures are being added to the runtime module from this audit.
