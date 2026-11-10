# Stack-argument layout and candidate-context access pattern

Date: 2026-11-10

## Exact target

Rechecked the uploaded ARM64 `libminecraftpe.so` (Build ID `2e318db12824cadb2618754ab7c82fa96fb30659`) using the full instruction window around `0x9e43f80–0x9e444bc`.

## Correct AArch64 stack-argument mapping

The function allocates `0x190` bytes and sets `x29 = sp + 0x130`. Therefore:

- `[x29 + 0x60]` is the first stack-passed argument at the incoming SP (argument 8, counting x0 as argument 0).
- `[x29 + 0x68]` is the next stack-passed argument (argument 9).

At `0x9e440dc`, `ldp x24, x22, [x29, #0x60]` loads:
- `x24`: stack argument 8, a context/container-like object;
- `x22`: stack argument 9, the candidate-range object.

At `0x9e44198`, `x22` is passed as helper argument x2 to `0x11879140`. This confirms the correction in the prior report and makes the ABI mapping explicit.

## Candidate-range object

The range object at x22 is read as two pointers:
- `[x22]` -> begin;
- `[x22 + 8]` -> end.

Each record is `0x34` bytes. The loop accesses:
- float/vector fields around `+0x18..+0x20`;
- output float at `+0x24`;
- packed 32-bit candidate key at `+0x28`;
- per-record byte flag at `+0x30`.

The candidate key at `+0x28` is split into a 7-bit index (bits 11–17), an 11-bit slot (bits 0–10), and upper packed bits. Two separate indexed-table validation paths compare values after XOR with the upper bits. This is consistent with a generation/version-checked entity handle, but the exact type remains unconfirmed.

## Context/container object at x24

The x24 object is used alongside each candidate:
- `x25 = x24 + 8` is retained as a base for indexed pointer lookup.
- `[x24 + 0x28]` is loaded as a mode/count-like value.
- `[x24 + 0x10]` points to a second table-like object, which itself contains begin/end-style pointers at offsets `+8` and `+16`.
- The code validates the candidate index against both table ranges and then indexes 32-bit entries by the 11-bit slot.

This looks like a multi-table context for validating a packed handle, not a simple flat array. It may be related to the strict entity context or component storage used by the camera-aim-assist system, but the disassembly alone does not prove that identity.

## Practical implication for ESP

This is the clearest current lead toward entity identity in the candidate-processing path: a candidate record contains a packed key, and a separate context object validates it against two indexed structures.

It is still not a usable ESP entity feed. The candidate range may be filtered to camera-aim-assist targets rather than all actors/items, and no AABB component extraction or render-thread handoff has been established. Do not treat the x24 offsets as general ECS offsets, and do not hook this routine yet.

## Next target

Identify the concrete type behind x24 and the construction site for the stack-passed candidate range. Prioritize RTTI, nearby adapter/template metadata, and other callsites that access the same two-table shape. Only after the type is identified should we assess whether this path can provide player/item bounds or whether a separate strict-system view is needed.
