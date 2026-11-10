# 2026-11-10 — Identified the second CameraAimAssist stack object

## Binary identity

- `libminecraftpe.so`
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- GNU Build ID: `2e318db12824cadb2618754ab7c82fa96fb30659`

## New RTTI result

The second vtable address point used by the caller at `0x9e44168` is `0x14060658`.

Its RTTI chain is:

- vtable header/typeinfo pointer at `0x14060650`
- typeinfo object at `0x140606a0`
- type-name pointer at `0x140606a8`
- type-name string at VA `0x299263e`

The type name begins:

```
std::__ndk1::__function::__func<
  ... CameraAimAssistFilterObstructedEntitiesSystem::tickFilterObstructedEntities(...)
  ...
>
```

The associated target-function string at VA `0x2992826` confirms the callable is:

```
CameraAimAssistFilterObstructedEntitiesSystem::tickFilterObstructedEntities
```

This is a concrete identification of the second stack object's role: it is a type-erased callable wrapper for the CameraAimAssist obstruction-filtering system, not an unknown renderer or scheduler object.

## Combined interpretation of the caller

At `0x9e44154–0x9e44170`, the enclosing routine materializes:

1. A local object with vptr `0x140605d8`, the strict-system adapter associated with `CameraAimAssistFetchValidEntityTargetSystem::tick`.
2. A local object with vptr `0x14060658`, now identified as a `std::function` callable targeting `CameraAimAssistFilterObstructedEntitiesSystem::tickFilterObstructedEntities`.

Both are passed to `0x11879140` together with a candidate entity/position context and flags. The caller iterates 0x34-byte records and uses per-record state and position-like fields. This supports the interpretation that the code is a CameraAimAssist candidate/obstruction processing path.

It does **not** make this a public entity iterator or a supported mod callback. The strict adapter and callable are temporary local objects in this particular path.

## What is now established

- The first local object's RTTI is tied to the strict ticking adapter for target fetching.
- The second local object's RTTI is tied to obstruction filtering.
- The enclosing code is part of a CameraAimAssist-specific processing path, rather than an unidentified generic geometry callback.

## What remains unknown

- The exact owning class/function name for the enclosing routine.
- The complete semantics and ABI of helper `0x11879140`.
- The scheduler registration owner and runtime construction/lifetime of the strict execution context.
- A stable, safe, mod-accessible entity view and a render-thread handoff.

No hooks were enabled and no runtime memory access was added. The ESP overlay remains HUD-only; live player/item boxes are not implemented or verified.
