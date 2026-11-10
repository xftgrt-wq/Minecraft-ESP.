# 2026-11-10 — strict-system registration cluster follow-up

## Binary identity

Target remains the exact uploaded ARM64 ELF:
- Size: 349,243,744 bytes
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- GNU Build ID: `2e318db12824cadb2618754ab7c82fa96fb30659`

## New findings

### Strict-system metadata cluster

The relocation table places the two previously identified strings in a compact read-only metadata cluster:

- `0x145efa98 -> 0x32d9be3`: strict-system descriptor name string
- `0x145efab0 -> 0x32d9ad8`: `StrictTickingSystemFunctionAdapter<&VanillaSystems::ActorMoveSystemImpl::tickUpdateHitboxSystem, ...>` adapter name
- `0x145efad8 -> 0x12b420ec`: associated code pointer
- Nearby entries at `0x145efb18` and `0x145efb30` begin a second structurally similar record, with code pointer `0x12b42cdc`.

The first record is therefore not an isolated string occurrence: its descriptor name, adapter name, and code pointer are adjacent in relocation-backed metadata. This strengthens the interpretation that the cluster describes registered strict-system adapter metadata.

### Code pointer inspection

At `0x12b420ec`, the code is a short wrapper:
- saves the frame,
- takes its first argument into `x1`,
- loads a global object at `0x1473fa48`,
- obtains a virtual function pointer from offset `+0x18`,
- invokes it with the global object and original argument.

This establishes a wrapper-to-global-service dispatch shape, but does **not** identify the service type, its ownership/lifetime, or prove that it is the ECS registry/context.

The adjacent helper routines beginning at `0x12b4211c` and later addresses use indirect virtual calls and stack-canary checks. Their presence alone is insufficient to infer a callable entity iteration API.

### Storage RTTI cluster rechecked

The relocation table around `0x13f71300–0x13f71700` confirms that the StateVector and AABB storage RTTI objects are part of a larger family of EnTT storage-related typeinfo/vtable records. In particular:
- StateVector storage type name: `0x27b9609`
- AABBShape storage type name: `0x27b9863`
- StateVector storage RTTI object: `0x13f713d8`
- AABBShape storage RTTI object: `0x13f71648`

The associated vtable entries point at code around `0x874e8fc` and `0x87448a4` respectively, consistent with storage-specific virtual method tables. They are still type metadata, not live storage instances or global registry pointers.

## What this does and does not prove

Confirmed:
1. The exact target binary contains the strict hitbox-system adapter and descriptor metadata.
2. The metadata is organized in relocation-backed records with associated code pointers.
3. One associated wrapper dispatches through a global service at `0x1473fa48`.
4. The StateVector and AABB storage RTTI objects belong to a broader storage metadata family.

Not established:
- The concrete type or lifecycle of the service at `0x1473fa48`.
- Whether it owns, reaches, or merely dispatches into the ECS strict context.
- A stable world/registry accessor.
- Runtime component layouts, live entity iteration, or AABB storage lifetime.
- A safe render-thread handoff or callable hook ABI.

## Next investigation

Trace the producer/initializer and all relevant references for the global service at `0x1473fa48`, then determine whether its virtual interface reaches the strict-system context or only a scheduler/dispatcher. In parallel, map the neighboring registration records and identify the object that constructs or owns the adapter table. Do not treat the global address as an ECS accessor until its type and lifetime are proven.

Continue to keep all game hooks disabled. No new hook target, offset, or ABI is justified by this pass.
