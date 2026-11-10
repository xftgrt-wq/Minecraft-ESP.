# 2026-11-10 — Call boundary and argument map for the CameraAimAssist candidate-processing path

## Binary identity

- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- GNU Build ID: `2e318db12824cadb2618754ab7c82fa96fb30659`

## Caller site

At VA `0x9e44154–0x9e441a4`, the caller builds two local polymorphic objects and calls `0x11879140`.

The relevant argument setup is:

| Argument | Value at call | Directly established meaning |
|---|---|---|
| x0 | `x19 + 0x0c` | pointer into caller-owned object |
| x1 | `sp + 0x70` | pointer to a 12-byte vector/coordinate value |
| x2 | `x22` | caller-held polymorphic object; its vtable slot +0x10 is invoked by callee |
| x3 | integer converted from float loaded at `[x26+8]` | scalar parameter |
| x4 | `1` | integer flag/parameter |
| x5 | boolean derived from a prior comparison | boolean parameter |
| x6 | `x29 - 0x50` | address of a local object whose first word is a selected vtable pointer |
| x7 | `x29 - 0x80` | address of the local strict-system adapter |
| stack arg 8 | `x23` | additional caller-owned pointer |

The second local object, with vptr `0x14060658`, is stored at `sp+0x80` and passed indirectly through the local context object assembled around `x29-0x50`.

## Callee entry observations at 0x11879140

The callee:

1. Reads three 32-bit float components from x0 at offsets 0, 4, and 8.
2. Reads three float components from x1 at offsets 0, 4, and 8.
3. Checks exponent/mantissa bit patterns to reject NaN/Inf values.
4. On the valid path, loads a function pointer from the vtable of x2 at offset +0x10 and calls it with `blr`.
5. Converts/rounds scalar values and calls that virtual operation before continuing through additional object dispatches.

Thus, this is a geometry-oriented operation accepting two 3D float values and a polymorphic operation/context. It is not itself a raw ECS registry accessor.

## Caller-side collection

The enclosing loop advances a record pointer by `0x34` bytes and checks a byte at offset `0x30`. For active records it reads float values around offsets `0x18–0x20` and constructs a 3D value. This is consistent with a compact spatial-record collection, but the concrete record type and owning class remain unidentified.

## What this adds

- The strict-system adapter is passed as a local argument into a geometry-processing call.
- The second callable object is part of a local context passed through that same call.
- The callee uses a virtual operation from the x2 object; this is the next useful dispatch to identify.
- The caller's 0x34-byte records are spatial-looking but are not yet proven to be entity component storage.

## What is still not proven

No evidence here establishes an unrestricted entity iterator, registry pointer, component layout, scheduler registration point, render callback, or safe cross-thread lifetime. Do not hook `0x11879140` or the caller based on this partial argument map.

## Next step

Identify the dynamic type and vtable slot +0x10 implementation of the x2 object at the callsite. That should clarify what geometry operation is being requested and whether the strict adapter is only a predicate/visitor in this local operation.

## ESP status

No hooks enabled, no live entity source connected. The current module remains HUD-only; player/item boxes have not been implemented or verified in-game.
