# 2026-11-10 — Code cross-reference for the CameraAimAssist strict adapter vtable

## Binary identity

- File: `libminecraftpe.so`
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- Build ID: `2e318db12824cadb2618754ab7c82fa96fb30659`

## Search method

Scanned the executable AArch64 segment for adjacent `ADRP + ADD` instruction pairs that materialize the address-point VA `0x140605d8`. The scan found three references:

- `0x9e44154`
- `0x9e80bac`
- `0x9e80bc8`

The latter two are the adapter construction/copy helpers already documented. The first is a new in-function use site.

The same scan for the neighboring vtable address point `0x14060658` found:

- `0x9e44168`
- `0x9e80cd0`
- `0x9e80cec`

This indicates that the surrounding routine constructs two related polymorphic stack objects, not merely a single standalone heap object.

## New use-site at 0x9e44154

The code around the new reference does the following:

1. Loads a float from an object at `[x26 + 8]`.
2. Materializes `0x140605d8` and stores it at `[x29 - 0x80]`, creating a local object with the strict-system adapter vptr.
3. Materializes `0x14060658` and stores it at `[sp + 0x80]`, creating a second local polymorphic object.
4. Passes pointers to these local objects, along with entity/object and vector-like arguments, to a helper at `0x11879140`.
5. Later dispatches through the local objects' vtables for cleanup/operation paths.

The helper at `0x11879140` begins by validating three-component floating-point values from its first two arguments. This looks like a geometry/math processing path, but the complete helper semantics and the enclosing caller's owning type have not yet been established.

## Interpretation and limits

This is a real executable code reference to the strict adapter vtable, and it is more useful than the previous RTTI-only evidence: the adapter is materialized in a local execution path and passed into another routine.

However, this does **not** yet prove that the site is the system-registration callsite, nor that the local object provides an externally reusable entity iterator. The two stack objects may be temporary callable/visitor adapters for a geometry operation. Do not infer a stable registry pointer or hook ABI from this xref.

No hooks were enabled. No game-memory reads or writes were added to the mod.

## Next analysis step

Recover the complete enclosing function boundaries and identify the exact type/semantics of the helper at `0x11879140`. Then follow the callback/visitor invocation path to determine whether it receives a strict entity context or only performs a local geometry operation. In parallel, continue tracing the actual scheduler registration owner rather than treating this stack-local use as registration.

## ESP status

The overlay still has no proven live entity feed. The HUD diagnostic works; live player/item boxes remain unimplemented and unverified.
