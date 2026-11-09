# Additional trace: FDE ownership and direct-call scan — 2026-11-09

## Target

- ELF64 AArch64 shared object
- SHA-256: d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1

## Function and symbol distinction

ELF .symtab reports Java_com_mojang_minecraftpe_FilePickerManager_nativeDirectoryPickResult at VA 0x125444b8 with size 1232 bytes. The FDE at .eh_frame offset 0x01c847f8 separately covers VA 0x1290c4a8..0x1290d560 (length 0x10b8). These are distinct ranges. LLVM's nearest-symbol annotation at the latter address is only a nearest preceding symbol label, not proof that the code belongs to that JNI function.

The FDE prologue saves x19-x28 and x29/x30. At VA 0x1290c4e4, x21 is assigned from incoming x2; x20 from x1; x19 from x0. Thus x21 is the third incoming argument to this FDE-covered routine.

At 0x1290c700, the routine reads [x21+0x60], constructs descriptor address 0x149100a0, and passes [x21+0x208] to helper 0x1267ab74. Other fields read from x21 include 0x1e8, 0x1ea, 0x1f8 and 0x248. The pattern suggests a larger structured state object, but no concrete C++ type is proven.

## Direct-call scan

A scan of AArch64 BL immediate instructions across the target .text section found no direct BL instruction whose destination is exactly 0x1290c4a8. This does not rule out indirect invocation, a callback/function-pointer table, tail-call branches, or a thunk that forwards to this routine. It does mean a simple direct-call xref is not currently available.

## Interpretation

- The descriptor xref is not proven to be world ViewProjection.
- It is also not valid to classify it as FilePicker based solely on objdump's nearest-symbol label.
- The third argument object and its producer/callback must be identified before assigning semantics to offset 0x208.
- No runtime hook, matrix offset, or ECS access offset is approved.

## Geometry test verification

The committed geometry implementation was compiled with Clang C++17, -Wall -Wextra -Werror, AddressSanitizer and UndefinedBehaviorSanitizer. The test executable passed identity projection, viewport mapping, negative clip-W rejection, inverted AABB rejection, null input rejection, and zero-width viewport rejection.

This validates the isolated geometry helper only, not Bedrock's runtime camera convention or actual ESP rendering.

## Next investigation

1. Inspect relocations, init-time registration, and function-pointer tables that may reference the FDE routine.
2. Trace the third argument's allocation/constructor and callback source.
3. Follow descriptors 0x149100a0 and the second block at x21+0x248 to their registration sites.
4. Independently trace a world-renderer draw path and ECS runtime iteration.

No working in-game ESP or installable production library is available.