# Trace HitboxComponent payload into its actor-bound processing helper

Date: 2026-11-10

## Binary and scope

Exact target: uploaded ARM64 `libminecraftpe.so`, GNU Build ID `2e318db12824cadb2618754ab7c82fa96fb30659`.

This report follows the callsite at `0x9e3e394` and callee `0x9e3cb38`. The callsite is in the same CameraAimAssist-related code region that resolves the HitboxComponent storage.

## Hitbox storage lookup and slot

At `0x9e3e25c–0x9e3e270`, the caller requests type key `0x5eb4e8fb` from the context at `x21 + 0x28). The returned storage wrapper is retained in `x22`.

The per-entity slot lookup at `0x9e3e2e8–0x9e3e380`:
- obtains the sparse/entity-key mapping;
- validates the packed entity key against the sparse table;
- obtains the component page;
- calculates the slot address using a `0x18` stride;
- passes that slot address as `x1` to helper `0x9e3cb38`.

This confirms the helper receives the concrete HitboxComponent storage slot for the validated entity key.

## What the helper does with the component slot

At helper entry `0x9e3cb38`:
- `x2` is saved as `x19`, the context/registry-like object;
- after resolving ActorOwnerComponent and the actor record, `x1` is saved as `x20` at `0x9e3cc7c`;
- the helper then loads two pointers from `[x20]` at `0x9e3ccec`, treating them as a begin/end range.

The range is traversed in `0x90`-byte increments. For each entry, the code reads:
- byte flag at entry `+0x88`;
- pointer-like fields around `+0x28), `+0x30), `+0x38), `+0x40), and `+0x48);
- inline/indirect data around `+0x50) and `+0x78).

It compares fields and conditionally calls helpers that obtain additional geometry-like values. One path calls `0x10d67694` using a pointer at entry `+0x50`; if non-null, it reads a double at result `+0xe8` and a float at `+0xf0`. Another path calls `0x105967e8` with an output object and then uses the returned values to populate a temporary 3D/float structure.

The helper finally calls `0xb58ecd0` with the context, a per-entry pointer at `entry + 0x28`, and two local geometry/value structures. The result is then checked against another entity-indexed storage path before a pointer is appended to a local vector-like collection.

## ActorOwnerComponent validation

Before processing the HitboxComponent range, the helper resolves ActorOwnerComponent using type key `0x85b93800`, validates the packed entity key through sparse/page tables, and reads an actor-owned flag at actor-record offset `+0x251`. This supports the conclusion that the HitboxComponent data is processed in an actor-associated context, rather than as an isolated geometry array.

The exact ActorOwnerComponent field semantics and the actor-record type are not recovered here.

## What is established vs. not established

Established by instruction flow and RTTI:
- the callsite resolves a HitboxComponent storage slot for a validated entity key;
- the HitboxComponent slot begins with a begin/end-like pair of pointers;
- the referenced range is traversed with a 0x90-byte record stride;
- records carry flags and several pointer/inline-data fields;
- the helper performs actor-associated and geometry-like processing on those records.

Not established:
- that the 0x90-byte records are AABBs or SubBBs;
- the exact field meanings or world-space coordinate convention;
- the AABBShapeComponent storage key or its retrieval path;
- whether this helper produces the bounds used by the target-selection candidate builder;
- a safe external iteration or render-thread snapshot API.

## Next target

Trace the output of `0xb58ecd0` and the producer of the HitboxComponent begin/end range. Compare the 0x90-byte record data with the AABBShapeComponent and SubBBsComponent consumers. Do not reinterpret the 0x90-byte records as bounds until their producer and consumer agree on field semantics.

No hook or runtime offset is justified by this report. Keep internal hooks disabled.
