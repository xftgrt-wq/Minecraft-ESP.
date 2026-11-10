# Exact-binary follow-up: x2 argument at CameraAimAssist candidate helper

Date: 2026-11-10

## Binary identity

The uploaded `libminecraftpe.so` is byte-for-byte the same binary as the previously analyzed target:

- ELF64, little-endian, AArch64, stripped shared object
- Size: 349,243,744 bytes
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- GNU Build ID: `2e318db12824cadb2618754ab7c82fa96fb30659`

This confirms the prior VA observations apply to this upload.

## Caller-side provenance of x2

At the previously identified callsite `0x9e441a4`, the helper at `0x11879140` receives `x2 = x22`.

In the enclosing function prologue/path:

- `x22` is assigned directly from the function's incoming `x6` at `0x9e43fb8`.
- At `0x9e43fb4`, the caller loads two adjacent 64-bit words from `[x6]` into `x27` and `x8`.
- The loop compares these values as begin/end-like pointers, advances the first pointer by `0x34`, and stops when it reaches the second.
- Records in this loop are 0x34 bytes wide; the caller checks byte `+0x30` and reads float data around `+0x18..+0x20`.

This is evidence that the incoming x6 object participates in a begin/end range over fixed-size records. It does **not** identify the concrete C++ type or establish that the records are ECS entities.

## Cal lee-side use

At `0x11879294–0x118792dc`, the helper:

- loads the first 64-bit word from x2;
- loads a callable pointer from offset `+0x10` relative to that first word;
- invokes it with `blr`.

The raw instructions are therefore consistent with an indirection/adapter object associated with the range, but do not justify naming it a vtable, registry, entity iterator, or ECS accessor. In particular, the prior shorthand “x2 vtable slot +0x10” is too strong: the actual sequence is `ldr x9, [x2]`; `ldr x9, [x9, #0x10]`; `blr x9`.

## Conclusion

The uploaded file validates the exact target binary and permits analysis to resume. The x2 path is still not a safe live-entity source. The next useful static-analysis step is to identify the concrete type/layout represented by the incoming x6 range and the object reached through its first word, then determine whether the 0x34-byte records carry entity identity or only geometric candidates.

No hook, runtime offset, entity registry accessor, or working ESP data feed is established by this finding. Keep game hooks disabled until type, ABI, lifetime, and render-thread ownership are independently verified.
