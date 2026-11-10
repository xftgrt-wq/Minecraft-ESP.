# Correction: provenance of x2 and the 0x34-byte candidate range

Date: 2026-11-10

## Why this correction is necessary

A closer re-read of the full caller disassembly shows that register `x22` is overwritten before the helper call. The earlier report `2026-11-10-camera-aim-assist-x2-range-provenance.md` incorrectly treated the original `x22 = x6` assignment as still live at the callsite.

## Correct register flow

At `0x9e43fb4–0x9e43fbc`:
- `ldp x27, x8, [x6]` reads the two words from the incoming x6 object.
- `mov x22, x6` temporarily copies x6 to x22.
- The code checks whether the two values are equal.

But at `0x9e440dc`:
- `ldp x24, x22, [x29, #0x60]` loads two values from the caller's stack-argument area.
- This overwrites x22 before the call to `0x11879140`.
- At `0x9e44198`, `mov x2, x22` therefore passes the **second stack-passed argument**, not the original incoming x6.

The candidate loop then reloads its begin/end-like pointers from this stack-passed object:
- `ldp x27, x28, [x22]` at `0x9e440bc` and again at `0x9e440d0`.
- It advances x27 by `0x34` per record.
- It reads record fields at offsets `+0x18..+0x30`.

Thus the 0x34-byte candidate range is associated with the second stack-passed argument object. The original x6 object is a separate input used in the initial emptiness check and is not the range passed as x2 at the helper call.

## What remains valid

The per-record observations remain valid for the range reloaded from the stack-passed x22 object:
- three floats around `+0x18..+0x20);
- a 32-bit value at `+0x28) that is decoded and used in indexed table validation;
- a float at `+0x24) written by the processing path;
- a byte at `+0x30) used as a per-record flag.

The packed-handle interpretation remains a hypothesis supported by the bit extraction and table-validation pattern, not a proven C++ entity type.

## Updated conclusion

Do not identify the original x6 argument as the candidate range. The helper's x2 argument is a stack-passed object loaded into x22 at `0x9e440dc`. The next analysis step must identify the caller's stack-argument layout and trace the construction/type of this second stack-passed object. This correction supersedes the x2 provenance statement in the earlier report.

No hook or runtime offset is justified by this path.
