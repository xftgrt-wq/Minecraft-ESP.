# Function-boundary and argument trace for CurrentViewProjection xref — 2026-11-09

## Input binary

- File: libminecraftpe.so
- SHA-256: d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1
- ELF64, AArch64, stripped.

## Confirmed function boundary

The .eh_frame FDE covering the call site gives the actual function range:

```
pc = 0x1290c4a8 .. 0x1290d560
length = 0x10b8
FDE at .eh_frame offset 0x01c847f8
```

The function prologue starts at 0x1290c4a8. At 0x1290c4e4, the code executes:

```asm
mov x21, x2
mov x20, x1
mov x19, x0
```

Therefore the value later used as x21 + 0x208 is derived from the function's third incoming argument (x2). It is not enough to identify the source object type; caller/callback provenance is still required.

At 0x1290c700–0x1290c714, the function:
- reads a pointer from [x21 + 0x60];
- passes [x21 + 0x208] to the generic descriptor store helper;
- uses descriptor address 0x149100a0.

The same function also reads halfword fields at x21+0x1e8 and x21+0x1ea, a float at x21+0x1f8, and another 64-byte block at x21+0x248. This looks like a structured render/uniform state object, but the precise owner type and semantic role are not proven.

## What this resolves

- The call site is inside a real function with a known FDE range.
- The misleading nearest-symbol annotation is not a valid function boundary.
- The source pointer is a field within the third argument object, not a free-standing camera pointer.

## What remains unresolved

- The source-level function name and caller/callback chain.
- Whether the 64-byte field is the world ViewProjection matrix, a UI transform, or another matrix.
- Matrix ordering, handedness, clip-space convention, and update timing.
- A verified runtime hook point.

No matrix offset or hook should be used in the ESP implementation from this evidence alone.

## Next trace

Find the indirect caller/callback that supplies the third argument to 0x1290c4a8, identify the concrete type behind x21, then trace the producer of x21+0x208. Only after that can the descriptor be classified as a world-camera matrix or rejected.