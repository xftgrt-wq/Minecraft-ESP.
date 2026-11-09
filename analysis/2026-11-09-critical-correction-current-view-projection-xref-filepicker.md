# Critical correction: CurrentViewProjection descriptor xref is inside FilePicker JNI handler — 2026-11-09

Target: uploaded `libminecraftpe.so`, SHA-256 `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`.

## Direct disassembly evidence

Re-disassembly of VA `0x1290c680–0x1290c780` shows the call site previously treated as a camera matrix source:

```asm
0x1290c700: ldr x8, [x21, #0x60]
0x1290c704: adrp x1, 0x14910000
0x1290c708: add x1, x1, #0xa0   // descriptor 0x149100a0
0x1290c70c: add x0, x8, #0x8
0x1290c710: add x2, x21, #0x208
0x1290c714: bl  0x1267ab74
```

The disassembler identifies the enclosing function as:

`Java_com_mojang_minecraftpe_FilePickerManager_nativeDirectoryPickResult`

The same surrounding function accesses file-picker-like dimensions and scalar state at offsets including `x21+0x1e8`, `x21+0x1ea`, `x21+0x1f8`, and `x21+0x1fc`, and makes multiple descriptor-store calls for separate fields.

## Corrected conclusion

The xref from descriptor `0x149100a0` to source field `x21+0x208` occurs inside a JNI method named `FilePickerManager_nativeDirectoryPickResult`. Therefore this call site does **not** establish that `x21` is a LevelRendererCamera object, nor that `x21+0x208` is the live world-camera ViewProjection matrix.

The descriptor's name alone was insufficient to classify this particular use as world rendering. The previous report overstated the result. Treat the descriptor-to-source relation as a generic/UI-side use until a separate world-renderer consumer is proven.

The earlier OpenGL matrix upload cluster was also found in file-picker/UI code. These two observations reinforce that the current leads are not yet a confirmed world-camera path.

## What remains genuinely established

- The string `CurrentViewProjectionMatrixUniform` is present and registered as a descriptor.
- The descriptor pointer is passed to a generic 64-byte data-store helper at this JNI call site.
- The source for this particular call is relative to x21 at `+0x208`.
- The enclosing function is the file-picker result handler, not a proven world renderer.
- ECS adapter metadata identifies relevant component types, but not runtime storage layout or iteration.

## Required next step

Discard this file-picker xref as a world-camera source. Search for other xrefs to descriptor `0x149100a0` and validate each enclosing function by call chain / owner type before interpreting it. Independently trace world-renderer ECS storage and a supported runtime API. Do not hardcode the `+0x208` offset or build a hook from this lead.
